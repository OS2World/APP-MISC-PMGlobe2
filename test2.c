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
#define BADLONG   0x80000000L
#define NEGLONG   0xFFFFFFFFL

#include <stdio.h>

int main(void) {
 long result;

 if (BADLONG<0) printf("badlong<0  %li\n",BADLONG);
           else printf("badlong>=0 %li\n",BADLONG);
 if (NEGLONG<0) printf("neglong<0  %li\n",NEGLONG);
           else printf("neglong>=0 %li\n",NEGLONG);
 return 0;
 }

