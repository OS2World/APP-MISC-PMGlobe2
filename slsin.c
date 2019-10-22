/*-----------------------------------------------------------------*/
/* SLSIN: super lsin, interpolates between lower/upper bound       */
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
/* LSIN is only accurate to 0.1 degree, so we get values for 0.1   */
/* below and above, and interpolate appropriately                  */
/* (Rarely makes a difference)                                     */
long slsin(long deg) {
 long alow, ahigh;                      /* angles */
 long slow, shigh;                      /* sines */
 long sine, prop;
 if (labs(deg)==360000L) {sine=lsin(deg); return sine;}
 alow=(deg/100L)*100L; ahigh=alow+100L;
 slow=lsin(alow);      shigh=lsin(ahigh);
 if (diag) printf("Alo/hi, Slo/hi: %li %li %li %li\n", alow, ahigh, slow, shigh);
 prop=labs(deg-alow);                   /* % proportion */
 if (diag) printf("Prop: %li\n", prop);
 return (slow*(100L-prop)+shigh*prop)/100L;
 } /* slsin */

