/* ------------------------------------------------------------------ */
/* PMGlobe -- a geographical application                              */
/* GLOBECOL -- determine colour tables from logical colour map        */
/* ------------------------------------------------------------------ */
/*                                                                    */
/* Copyright (c) Mike Cowlishaw, 1993-2019.  All rights reserved.     */
/* Parts Copyright (c) IBM, 1993-2009.                                */
/*                                                                    */
/* Permission to use, copy, modify, and distribute this software      */
/* for any non-commercial purpose without fee is hereby granted,      */
/* provided that the above copyright notice and this permission       */
/* notice appear in all copies, and that notice and the date of       */
/* any modifications be added to the software.                        */
/*                                                                    */
/* This software is provided "as is".  No warranties, whether         */
/* express, implied, or statutory, including, but not limited to,     */
/* implied warranties of merchantability and fitness for a            */
/* particular purpose apply to this software.  The author shall       */
/* not, in any circumstances, be liable for special, incidental,      */
/* or consequential damages, for any reason whatsoever.               */
/*                                                                    */
/* ------------------------------------------------------------------ */

#define INCL_GPI
#include "globe.h"

/*====================================================================*/
/* GLOBECOL -- determine colour tables from logical colour map        */
/*====================================================================*/
/* This is called with a logical colour table (as in a bitmap).       */
/*                                                                    */
/*                                                                    */
/*                                                                    */
int nearest(int, int, int *, int *);    /* local subroutine */

int globecol(RGB2 *tab, PCOLSTRUCT c) {
  int i;
  long colours;
  /* if (diag) printf("In GLOBECOL...\n"); */

  /* Find out how many colours are available */
  {/* block */
  long lut[MAXCOLOURS+1];
  colours=GpiQueryRealColors(cps,0L,0L,(long)(MAXCOLOURS+1),lut);
  if (colours>MAXCOLOURS) {colours=MAXCOLOURS;
    if (diag) printf("More than %i colours available\n", MAXCOLOURS);}
   else {if (diag) printf("%li colours available\n",colours);}
  }
  (*c).colours=(int)colours;            /* copy back */

  {/* block */
  /* First find all the pure colour levels and put them into */
  /* local search tables */
  int rs=0, gs=0, bs=0;                 /* counts */
  int cs=0, ys=0, ps=0;                 /* .. (cyan, yellow, pink) */
  int ws=0;                             /* .. (white) */
  /* Each table needs 1KB if MAXCOLOURS=256 with 4-byte INTs... */
  /* (Stack needs to be big enough.) */
  int rtab[MAXCOLOURS], gtab[MAXCOLOURS], btab[MAXCOLOURS]; /* indexes */
  int ctab[MAXCOLOURS], ytab[MAXCOLOURS], ptab[MAXCOLOURS]; /* .. */
  int wtab[MAXCOLOURS];                                     /* .. */
  int rlev[MAXCOLOURS], glev[MAXCOLOURS], blev[MAXCOLOURS]; /* levels */
  int clev[MAXCOLOURS], ylev[MAXCOLOURS], plev[MAXCOLOURS]; /* .. */
  int wlev[MAXCOLOURS];                                     /* .. */
  int hadblack=FALSE;                   /* TRUE when black processed */
  int index;
  for (i=0; i<(int)colours; i++) {
    /* These can't be ELSEd, as black appears in all */
    if (tab[i].bGreen==0) {
      if (tab[i].bBlue==0)
         {
         /* Now check if a black -- if second or subsequent then ignore */
         if (tab[i].bRed ==0) {
           if (hadblack) continue;
            else hadblack=TRUE;
           }
         rtab[rs]=i; rlev[rs]=tab[i].bRed  ; rs++;}
      if (tab[i].bRed ==0)
         {btab[bs]=i; blev[bs]=tab[i].bBlue ; bs++;}
      /* For mixes, allow 'close match' as BGA does not have pure halves*/
      if (labs(tab[i].bBlue-tab[i].bRed)<20)            /* a 'pink' */
         {ptab[ps]=i; plev[ps]=(tab[i].bRed+tab[i].bBlue)/2; ps++;}
      } /* G==0 */

    if (tab[i].bRed==0) {
      if (tab[i].bBlue ==0)
         {gtab[gs]=i; glev[gs]=tab[i].bGreen; gs++;}
      if (labs(tab[i].bBlue-tab[i].bGreen)<20)          /* a cyan */
         {ctab[cs]=i; clev[cs]=(tab[i].bBlue+tab[i].bGreen)/2; cs++;}
      } /* R==0 */

    if (tab[i].bBlue ==0) {
      if (labs(tab[i].bRed-tab[i].bGreen)<20)           /* a yellow */
         {ytab[ys]=i; ylev[ys]=(tab[i].bRed+tab[i].bGreen)/2; ys++;}
      } /* B=0 */

    /* Greys and whites can also appear here */
    if (tab[i].bRed==tab[i].bGreen && tab[i].bRed==tab[i].bBlue)
      {wtab[ws]=i; wlev[ws]=tab[i].bRed  ; ws++;}

    /* if (diag) printf("%03i: %03i %03i %03i\n", i,
        tab[i].bRed, tab[i].bGreen, tab[i].bBlue);   */

    } /* loop counting colours */

  if (diag && test) {
     printf("Tables R,G,B,W have: %i %i %i %i\n", rs, gs, bs, ws);
     printf("Tables C,Y,P have: %i %i %i\n", cs, ys, ps);
     printf("Green table (I, CTAB, CLEV):\n");
     for (i=0; i<gs; i++)
        printf(" %i: %03i %i\n", i, gtab[i], glev[i]);
     /**
     printf("Grey table (I, CTAB, CLEV):\n");
     for (i=0; i<ws; i++)
        printf(" %i: %03i %i\n", i, wtab[i], wlev[i]);
     **/
     }

  /* Copy counts .. excluding Black .. to parameter structure */
  (*c).rsteps=rs-1; (*c).gsteps=gs-1; (*c).bsteps=bs-1;
  (*c).csteps=cs-1; (*c).ysteps=ys-1; (*c).psteps=ps-1;
  (*c).wsteps=ws-1;

  /* Now fill in the seven tables */
  /* This **N lookup is dumb but easy way to do.  We should do */
  /* it by (a) sorting the short tables (b) spreading them     */
  /* along the Xnear tables by computation.                    */
  for (i=0; i<=COLHI; i++) {
    (*c).rnear[i]=nearest((i*255+(COLHI/2))/COLHI, rs, rtab, rlev);
    (*c).gnear[i]=nearest((i*255+(COLHI/2))/COLHI, gs, gtab, glev);
    (*c).bnear[i]=nearest((i*255+(COLHI/2))/COLHI, bs, btab, blev);
    (*c).cnear[i]=nearest((i*255+(COLHI/2))/COLHI, cs, ctab, clev);
    (*c).ynear[i]=nearest((i*255+(COLHI/2))/COLHI, ys, ytab, ylev);
    (*c).pnear[i]=nearest((i*255+(COLHI/2))/COLHI, ps, ptab, plev);
    (*c).wnear[i]=nearest((i*255+(COLHI/2))/COLHI, ws, wtab, wlev);
    }

  /* And finally find the 8 pure colours */
  for (i=0; i<(int)colours; i++) {
    if (tab[i].bRed==255 || tab[i].bRed==0) {
      index=(tab[i].bRed/128)*4;
      if (tab[i].bGreen==255 || tab[i].bGreen==0) {
        index=index+(tab[i].bGreen/128)*2;
        if (tab[i].bBlue==255 || tab[i].bBlue==0) {
          index=index+(tab[i].bBlue/128);
          (*c).pure[index]=i;
          }
        }
      }
    } /* i */

  } /* block */

  return 0;
  } /* globecol */

/* NEAREST: return nearest colour                                 */
/* Could be made much faster if tables sorted, or assumed sorted. */
int nearest(int needle, int size, int ctab[], int lev[]) {
  int closest=0, dist=MAXCOLOURS, n;
  int j;

  for (j=0; j<size; j++) {
    n=abs(needle-lev[j]);
    if (n<dist) {dist=n; closest=ctab[j];}
    }
  return closest;
  } /* nearest */
