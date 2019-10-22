/* ------------------------------------------------------------------ */
/* FASTRAND -- return a random number in the range 0 to N-1           */
/*                                                                    */
/* --------------------------------------------------------------- */
/*                                                                 */
/* Copyright (c) Mike Cowlishaw, 1993-2019.  All rights reserved.  */
/* Parts Copyright (c) IBM, 1993-2009.                             */
/*                                                                 */
/* Permission to use, copy, modify, and distribute this software   */
/* for any non-commercial purpose without fee is hereby granted,   */
/* provided that the above copyright notice and this permission    */
/* notice appear in all copies, and that notice and the date of    */
/* any modifications be added to the software.                     */
/*                                                                 */
/* This software is provided "as is".  No warranties, whether      */
/* express, implied, or statutory, including, but not limited to,  */
/* implied warranties of merchantability and fitness for a         */
/* particular purpose apply to this software.  The author shall    */
/* not, in any circumstances, be liable for special, incidental,   */
/* or consequential damages, for any reason whatsoever.            */
/*                                                                 */
/* --------------------------------------------------------------- */
/*  This uses the conventional  X(n+1)=mod((A*X(n) + C),M) as used    */
/*  in the REXX function package, but without some refinements.       */
/*                                                                    */
/*  The values for A and M are from the table in D.E.Knuth's          */
/*    book, "Seminumerical Algorithms", Addison-Wesley, 1981,         */
/*    page 102 (section 3.3.2).                                       */
/*    These particular values are attributed to Lavaux and            */
/*    Janssens.                                                       */
/*  C is chosen as 1                                                  */
/*  M is 2**32                                                        */
/*  A is 1664525      (RNDFACT)                                       */
/*                                                                    */
int fastrand(int n)
  {
  static unsigned long seed=621476127L;      /* arbitrary start value */

  seed=seed*1664525L+1;                  /* don't care about overflow */
  return (int)(seed%n);
  } /* fastrand */
