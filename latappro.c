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

 /* Now worry about the latitude.  We base this on a known 0ø of    */
 /* declination: 21:00 on 20 March 1990.                            */
 /* Jan    1, 1990, 00:00 was 631152000 seconds in TIME() format,   */
 /* March 22, 1990, 00:00 was 638064000 seconds in TIME() format,   */
 /* March 20, 1990, 00:00 was 637891200 seconds in TIME() format,   */
 /* 21:00 into that day was therefore at 637966800.                 */
 /* There are 365.24219 mean solar days in a year, hence there are  */
 /* 31556925 mean solar seconds in a year.  We now calculate how    */
 /* far we are into the mean solar year...                          */
 equinsecs=(timesecs-637966800L)%31556925;
 /* We want 0-360ø (0-360000), but must be careful not to overflow  */
 /* This calculation is accurate to 0.1 degrees, gives Deg*1000.    */
 /* LSIN is only accurate to 0.1 degrees, at present, anyway...     */
 equindegs=(3600L*(equinsecs/100L)/315569L)*100L;
 /* Sine of this, times maximum excursion, is degrees of latitude.  */
 if (diag) {
   char ctemp[12];
   putmill(ctemp,equindegs);   printf("Equinox degrees: %s\n", ctemp);
   }
 /* Maximum excursion is tilt of the earth: 23.443ø (23ø26.6') */
 *slat=23443L*lsin(equindegs)/1000L;
