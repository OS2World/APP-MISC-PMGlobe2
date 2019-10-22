/* ------------------------------------------------------------------ */
/* LSQRT -- return the square root of a non-negative LONG integer     */
/* Note: the maximum input value is 2**31-1 (2147483647)              */
/*       hence the maximum possible returned value is 46340           */
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
long lsqrt2(long z)
  {
  int          k;        /* bit we are dealing with */
  unsigned int y;        /* partial (and final) result */
  long         comp;     /* comparison term */
  static long
    twopower[16]=        /* for 2**(k*2), etc. */
      {1L<<0 , 1L<<2 , 1L<<4 , 1L<<6 , 1L<<8 , 1L<<10, 1L<<12, 1L<<14,
       1L<<16, 1L<<18, 1L<<20, 1L<<22, 1L<<24, 1L<<26, 1L<<28, 1L<<30};

  if (z<=0) return 0;    /* for speed and safety */

  /* we start with: */
  y=0;
  /* so that  y**2 <= z < (y+2**(k+1))**2 */
  /* Z will become the remainder of the number as we iterate */

  /* Calculation of COMP within the loop below is quite expensive */
  /* as the shift left involves a K+1 iteration loop.  Therefore  */
  /* we first spin down to find a sensible (late-as-possible)     */
  /* starting point.                                              */
  for (k=15; k>=0; k--) if (z>twopower[k]) break;

  for (k=k; k>=0; k--) {
    comp=((long)y<<(k+1))+twopower[k]; /* 2*y*2**K + 2**(2*k) */
    if (comp<=z) {                     /* it fits */
      z=z-comp;                        /* new remainder */
      y=y+(1U<<k);                     /* y=y+2**k */
      }
    } /* k */
  return (long)y;

  /* Footnote:                                                   */
  /* We used to compute 2**(2*k) by setting an initial variable  */
  /* then shifting right by two each iterate -- but this         */
  /* generated a subroutine call for shift-right! (shift-left is */
  /* OKish, though it does generate a tiny loop).                */
  } /* lsqrt */

