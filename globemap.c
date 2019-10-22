/* ------------------------------------------------------------------ */
/* PMGlobe -- a geographical application                              */
/* GLOBEMAP: Allocate and create a bitmap world map                   */
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

/* ---------------------------------------------------------------- */
/* This currently assumes that 16-bit compatibility is required;    */
/* the size of the bitmap must be <= 64K.  This restriction is      */
/* deliberate, in order to get best performance during drawing.     */
/*                                                                  */
/* The map itself is run-length-encoded in a static array (details  */
/* of the code below).  Storage is allocated to hold the final      */
/* result, and the bitmap is then generated from the RLE data.      */
/* The result is the address of the bitmap (or NULL if an error).   */
/* It is the responsibility of the caller to free(..) the storage,  */
/* if and when necessary.                                           */
/*                                                                  */
/* The byte codes in the RLE array are:                             */
/*                                                                  */
/*    0-249  length of run of current bit; flips bit after          */
/*      250  run of 250, do not flip bit after                      */
/*      251  [reserved]                                             */
/*      252  end line, next line starts with 0 (not used)           */
/*      253  end line, next line starts with 1 (not used)           */
/*      254  [reserved]                                             */
/*      255  end of data                                            */
/*                                                                  */
/* ---------------------------------------------------------------- */

#include <malloc.h>
#include "globe.h"
#include "landmap.h"          /* the RLE bitmap and its dimensions */

extern HAB habGlobe;          /* anchor block for timing */

/* Proc starts here */
unsigned char huge *globemap(void) {
 long size;                   /* total size of bitmap (bytes) */
 unsigned char huge *map;     /* base of map */
 unsigned char huge *m;       /* next char in map */
 unsigned char huge *s;       /* next char in source */
 long start, stop, msecs;     /* timing */
 int left=0;                  /* un-set bits in current byte (0-7) */
 unsigned int bit=0;          /* current bit */
 unsigned int byte=0;         /* byte-worth of current bit */
 int sc;                      /* source count */
 int i, bytes;                /* work */
 long rupx;                   /* rounded-up X */

 if (landmapz>1) {printf("GLOBE can only handle one LANDMAP plane\n");
   return(NULL);}
 if (landmapx%8) {printf("GLOBE maps must be a multiple of 8 wide\n");
   return(NULL);}
 if (landmapx!=LANDMAPDIMX || landmapy!=LANDMAPDIMY) {
   printf("GLOBE map is not expected size\n");
   return(NULL);}

 /* Determine the real size required: each line is rounded up to */
 /* a multiple of 4 bytes...                                     */
 rupx=4*((landmapx+31)/32);   /* rounded-up landmapx */

 size=rupx*(long)landmapy;
 /** Restriction removed 27 Dec 1991 **
 if (size>256L*256L) {printf("GLOBE can only handle LANDMAPs < 64K\n");
   return(NULL);} **/

 map=(unsigned char huge *)halloc(size, (size_t)1); /* allocate map storage */
 if (map==NULL) {
   printf("No memory for %li bytes of world bitmap -- sorry\n", size);
   return(NULL);}
  else if (diag) printf("%li bytes allocated for map\n", size);

 /* Looks plausible -- now unpack the map */
 /* (We time it, to see if further work needed here) */
 if (diag) start=(signed)WinGetCurrentTime(habGlobe);
 s=landmap;                        /* -> first source */
 m=map;                            /* -> first output */
 for (;;s++) {                     /* for each byte in RLE */

/**
if ((s-landmap)>22300L==0 && diag) printf("s %li\n",(long)(s-landmap));
**/

   sc=(int)*s;                     /* copy to local INT */
   if (sc<=250) {                  /* have run, maybe flip bit */
     if (sc<left) {                /* won't fill current byte */
       for (i=0; i<sc; i++) *m=(unsigned char)((unsigned)(*m<<1U)|bit);
       left=left-sc;
       }  /* fit in one */
     else /* whole bytes already in target, or source will complete */ {
       if (left>0) {               /* fill part byte first */
         for (i=0; i<left; i++) *m=(unsigned char)((unsigned)(*m<<1U)|bit);
         sc=sc-left; left=0;
         m++;}
       /* left is now 0, *m ready for full bytes */
       bytes=sc/8;
       if (bytes>0) {
         for (i=0; i<bytes; i++, m++) *m=(unsigned char)byte;
         sc=sc-bytes*8;}
       /* may be part of a byte left */
       if (sc>0) {
         for (i=0; i<sc; i++) *m=(unsigned char)((unsigned)(*m<<1U)|bit);
         left=8-sc;
         } /* final part-byte */
       } /* a run of more than part byte */
     if (*s!=250) {                /* flip bit unless was 250 */
       bit =bit ^0x01U;
       byte=byte^0xffU;
       } /* flip */
     } /* run */
   else break;                     /* unknown or EOF */
   } /* m-loop */

 if (!diag) return map;

 stop=(signed)WinGetCurrentTime(habGlobe);
 msecs=stop-start;
 if (msecs<0) printf("Timer wrapped\n");
         else printf("Unpack took %lims\n", msecs);
 return map;
 } /* globemap */
