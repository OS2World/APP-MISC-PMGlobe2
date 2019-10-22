/* ------------------------------------------------------------------ */
/* PMGlobe -- a geographical application                              */
/* GLPAINT -- Paint a rectangle from memory bitmap                    */
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

#define  INCL_GPICONTROL
#define  INCL_GPIPRIMITIVES
#define  INCL_GPIBITMAPS
#define  INCL_GPILCIDS
#define  INCL_DOSSEMAPHORES

#include "globe.h"

/* Variables shared with GLOBER */
extern HEV  sembitmap;             /* protect bitmap */
extern HPS  globeps;               /* HPS for globe bitmap   */

/*====================================================================*/
/* GLPAINT -- Paint a rectangle from memory bitmap                    */
/*====================================================================*/
/* This waits until bitmap exists (if necessary), then copies the     */
/* requested rectangle from the MDC bitmap to the screen.             */
void glpaint(void) {
 POINTL pa[4];                     /* array of points for BitBlt */
 long   x0, y0;                    /* origin of globe drawing */
 long   lrc;                       /* work */
 RECTL  rcl, r;                    /* work */
 HPS    mps;                       /* local copy */

 /* First request consistent bitmap/pointer */
 /* if (diag) printf("Paint requesting Screen bitmap\n"); */
 DosRequestMutexSem(sembitmap, SEM_INDEFINITE_WAIT);
 cps=WinBeginPaint(hwndClient, cps, &rcl);

 /* If no safe shadow bitmap yet, then we fill the window and return. */
 /* If the minimum client size does not match the bitmap DIAMETER,    */
 /* then the bitmap is invalid (e.g., after a WM_SIZE), and we also   */
 /* simply clear the paint rectangle and return.                      */
 /* 'match' here means that the globe must fit within the window.     */
 if (globeps==NULL                                /* no bitmap */
  || lmin(clientx,clienty)<(long)diameter         /* globe too big */
  || diameter==0) {                               /* no globe */
   WinFillRect(cps, &rcl, backcoli);
   if (diag) printf("Painted to fill screen\n");
   }
  else {                      /* copy from shadow bitmap */
   /* Otherwise we'll very probably use the bitmap. */
   mps=globeps;               /* local copy */

   x0=(clientx-diameter)/2;   /* origins (true) */
   y0=(clienty-diameter)/2;
   /* Note: both X0 and Y0 could be non-zero if margin>0 */

   /* First handle rectangles outside of globe-square */
   if (rcl.xLeft<x0) {        /* stripe to left */
     r=rcl;                   /* copy */
     r.xRight =x0;
     rcl.xLeft=x0;
     WinFillRect(cps, &r, backcoli);
     } /* stripe */
   if (rcl.xRight>x0+diameter) {   /* stripe to right */
     r=rcl;                        /* copy */
     r.xLeft   =x0+diameter;
     rcl.xRight=x0+diameter;
     WinFillRect(cps, &r, backcoli);
     } /* stripe */
   if (rcl.yBottom<y0) {           /* stripe at bottom */
     r=rcl;                        /* copy */
     r.yTop     =y0;
     rcl.yBottom=y0;
     WinFillRect(cps, &r, backcoli);
     } /* stripe */
   if (rcl.yTop>y0+diameter) {    /* stripe at top */
     r=rcl;                       /* copy */
     r.yBottom=y0+diameter;
     rcl.yTop =y0+diameter;
     WinFillRect(cps, &r, backcoli);
     } /* stripe */

   /* The rectangle RCL now defines an area wholly within globe square */
   pa[0].x=rcl.xLeft;    pa[0].y=rcl.yBottom;       /* target */
   pa[1].x=rcl.xRight;   pa[1].y=rcl.yTop;          /* .. */
   pa[2].x=rcl.xLeft-x0; pa[2].y=rcl.yBottom-y0;    /* source */
   lrc=GpiBitBlt(cps, mps, 3L, pa, ROP_SRCCOPY, 0L);
   if (lrc==GPI_ERROR) {             /* error, but not fatal */
     printf("Could not BitBlit to screen PS\n");
     }
    /* */else if (diag) printf("Painted to screen\n"); /* */
   } /* build screen from rectangles */

 WinEndPaint(cps);
 DosReleaseMutexSem(sembitmap);    /* done with bitmap and screen */
 return;
 } /* glpaint */
