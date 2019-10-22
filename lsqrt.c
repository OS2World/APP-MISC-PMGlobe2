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
long lsqrt(num)
  long num;
  {
  long new;
  long old;
  int  in, inew, iold, isub, k;
  static long
    ltwopower[16]=            /* for starting guess */
      {1L<<0 , 1L<<2 , 1L<<4 , 1L<<6 , 1L<<8 , 1L<<10, 1L<<12, 1L<<14,
       1L<<16, 1L<<18, 1L<<20, 1L<<22, 1L<<24, 1L<<26, 1L<<28, 1L<<30};
  static unsigned int
    power[16]=                /* also for starting guess */
      {1U<<0 , 1U<<1 , 1U<<2 , 1U<<3 , 1U<<4 , 1U<<5 , 1U<<6 , 1U<<7 ,
       1U<<8 , 1U<<9 , 1U<<10, 1U<<11, 1U<<12, 1U<<13, 1U<<14, 1U<<15};
  static unsigned int
    twopower[8]=              /* for quicker path */
      {1U<<0 , 1U<<2 , 1U<<4 , 1U<<6 , 1U<<8 , 1U<<10, 1U<<12, 1U<<14};

  if (num<9) {                /* special case low values for safety */
    if (num<=0) return 0;     /* special case + safety */
    if (num< 4) return 1;     /* 1, 2, 3 */
    return 2;}                /* 4-8 */

  /* If definitely safe, do it in (maybe short) integers, for speed */
  /* Otherwise have to use LONGs                                    */
  /* Note: first guess has dramatic performance impact -- a value   */
  /* 1, or N/16, both doubled total GLOBE draw time.                */
  /* N/1024+1, and 32,000 both worked well.  We now (Nov 91) use a  */
  /* dynamically calculated first guess, which is (just) better.    */
  /* Note also table lookup faster than multiple shifts, on 16-bit. */
  if (num>32767) {            /* too big */
    /* For the first guess we use root of a power of two comparable */
    for (k=14; k>=0; k--) if (num>ltwopower[k]) break;
    old=power[k+1];           /* first guess */
    for (;;) {                /* forever */
      new=((num/old)+old)>>1; /* new guess */
      if ((new-old)<2
       && (new-old)>-2) return new;  /* Done! */
      old=new;
      }
    } /* using longs */
   else {                     /* use shorts */
    in=(int)num;              /* copy */
    /* For the first guess we use root of a power of two comparable */
    for (k=7; k>=1; k--) if (in>twopower[k]) break;
    iold=power[k+1];          /* first guess */
    for (;;) {                /* forever */
      inew=((in/iold)+iold)>>1;/* new guess */
      isub=inew-iold;
      if (isub<2 && isub>-2) return (long)inew;  /* Done! */
      iold=inew;
      }
    } /* using shorts */
  } /* lsqrt */

