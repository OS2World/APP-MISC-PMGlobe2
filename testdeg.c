/* Test GETDEG subroutine.  Use "lon" or "lat" for special tests */
/* --------------------------------------------------------------- */
/*                                                                 */
/* Copyright (c) Mike Cowlishaw, 1993-2012.  All rights reserved.  */
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#define BADLONG 1000000000L
#define TRUE 1
#define FALSE 0
long getdeg(unsigned char *, long);

int main(int argc, char **argv)
 {
 char **parm;                 /* parameter character/string */
 int i;
 long lrc;
 long check=360000L;

 for (i=0, parm=argv; i<argc ; parm++, i++) {
   if (i==0) continue;        /* own name */
   if (strcmp(*parm,"lon")==0) {check=-180000L; continue;}
   if (strcmp(*parm,"lat")==0) {check= -90000L; continue;}

   printf("'%s' requested: ", *parm);
   lrc=getdeg(*parm, check);
   if (lrc==BADLONG) printf("bad"); else printf("%li", lrc);
   printf("\n");
   }
 return 0;} /* main */

/* ----- GETDEG(string, check) ------------------------------------ */
/*                                                                  */
/* Takes a string (in the form of degree) and converts it to a      */
/* LONG in which one unit indicates one thousandth of a degree.     */
/*                                                                  */
/* CHECK is a long that indicates the maximum degree (x1000) value  */
/* allowed: for example a value of 90000L means that degrees that   */
/* are outside the range -90 through +90 degrees would be reported  */
/* as an error.  CHECK may be up to 360000L.                        */
/*                                                                  */
/* Two special values of CHECK are known: -90000L means that the    */
/* string is expected to be a Latitude.  It may then be ended by    */
/* an uppercase or lowercase 'N' or 'S'.  If an S then the          */
/* resulting degree will be assumed negative.                       */
/* Similarly -180000L means that a Longitude is expected, and the   */
/* string may be ended by an 'E' or 'W', with 'W' being negative.   */
/*                                                                  */
/* The degree may be in degree-minutes-seconds format, or in        */
/* in degree.decimal format.  A leading minus sign is allowed in    */
/* either format, and negates the direction implied by any N, E,    */
/* S, or W.  A leading plus sign is also allowed; it has no effect. */
/*                                                                  */
/* No blanks are allowed: the seconds, or seconds and minutes, can  */
/* be omitted, as can leading 0's.  The syntax is therefore one of: */
/*                                                                  */
/*  [+|-][d]d[o[m]m['[s]s["]]][x]                                   */
/*  [+|-][d]d[.[[d]d]d][x]                                          */
/*                                                                  */
/* where the degree indicator can be either an o (either case) or   */
/* the true degree character (ø).                                   */
/*                                                                  */
/* For example, if the original string were "12o30'W" then the      */
/* returned result would be -12500, as would be the result from     */
/* the strings "-12.5", "-12ø30'00"E" or "-12.500N", with the       */
/* appropriate CHECK value.                                         */
/*                                                                  */
/* Returns BADLONG if the degree is out of range, or the minutes or */
/* seconds are greater than 59, or other error.                     */

long int getdeg(unsigned char *string, long check)
  {long result;                    /* accumulator */
   int neg=FALSE;                  /* needs a minus sign */
   int dec=-1;                     /* decimal format (if >=0) */
   int lat=FALSE;                  /* latitude  expected */
   int lon=FALSE;                  /* longitude expected */
   int part[3];                    /* part of the result */
   int i=0;                        /* input offset point */
   int p,k;
   unsigned char c;

   if (check<0) {                  /* lat or lon */
     if (check==-90000L) lat=TRUE;
      else if (check==-180000L) lon=TRUE;
      else return BADLONG;
     check=-check;
     } /* lat/lon */
   for (p=0; p<3; p++) part[p]=0;  /* clear to 0's */

   /* handle leading sign */
   if (string[0]=='-') {neg=TRUE; i++;}
   else if (string[0]=='+') i++;

   if (!isdigit(string[i])) return BADLONG;
   for (p=0; p<3; p++) {
     /* each part must have at least one digit, and <=3 digits */
     for (k=0; k<3; k++, i++) {
       if (!isdigit(string[i])) break;
       /* get a digit */
       part[p]=part[p]*10+(string[i]-'0');
       if (dec>0) dec--;           /* digits of decimal to go */
       } /* k */
     /* now check clean terminator */
     c=string[i];
     switch(p) {
       case 0:
         if (c=='.')                     {i++; dec=3;  break;}
         if (c=='ø' || c=='o' || c=='O') {i++;         break;}
         break;
       case 1:
         if (c=='\'' && dec<0) i++;
         break;
       case 2:
         if (c=='"' && dec<0) i++;
         break;
       }
     if (string[i]=='\0') break;                  /* done */
     if (isdigit(string[i]))
       {if (dec==0 || p==2) return BADLONG;}
      else {                       /* not a digit */
       if (string[i+1]!='\0') return BADLONG;     /* N,E not last */
       c=toupper(string[i]);
       switch(c) {
         case 'N':
           if (!lat) return BADLONG;
           break;
         case 'S':
           if (!lat) return BADLONG;
           neg=!neg;          /* effectively a trailing '-' */
           break;
         case 'E':
           if (!lon) return BADLONG;
           break;
         case 'W':
           if (!lon) return BADLONG;
           neg=!neg;          /* effectively a trailing '-' */
           break;
         default:
           return BADLONG;
         } /* switch */
       break;                           /* no more to do */
       } /* not a digit */
     } /* P */
   if (dec>=0) {
     /* apply remaining powers of ten */
     for (k=0; k<dec; k++) part[1]=part[1]*10;
     result=(long)part[0]*1000L+part[1];
     } /* dec */
    else {
     if (part[1]>59 || part[2]>59) return BADLONG;
     result=(1000L*(long)(part[2]+60*part[1]))/3600L+(long)part[0]*1000L;
     }
   if (result>check) return BADLONG;         /* out of range */
   /* Looks good! */
   if (!neg) return result;             /* .. note sign */
   return -result;
   } /* getdeg */

