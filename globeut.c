/* ------------------------------------------------------------------ */
/* PMGlobe -- a geographical application                              */
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

/* ------------------------------------------------------------------ */
/* Utilities.  This includes (not all may be in use):                 */
/*                                                                    */
/*   ADDEXT:  Add extension to a filename, if none there              */
/*   FASTRAND Fast random number generator                            */
/*   GETCOL   String name to long int (PM CLR_)                       */
/*   GETDEG   String degrees to long int                              */
/*   GETLONG  String integer to long int                              */
/*   GETMILL  String float to long int *1000                          */
/*   GETTIME  Convert time string to long int                         */
/*   GETWORD  Remove word from string and return                      */
/*   LSQRT    Long SQRT                                               */
/*   PUTCOL   Colour index to string name                             */
/*   PUTMILL  Long int *1000 to string float                          */
/*   STREQ    Caseless compare                                        */
/* ------------------------------------------------------------------ */

/* Next is for PM colours only */
#define  INCL_GPIPRIMITIVES
#include "globe.h"

/* ----- ADDEXT(filename,ext) ------------------------------------- */
/*                                                                  */
/* Adds a dot and the given extension to the filename, if none      */
/* there and there is room.                                         */
/*                                                                  */
/* Returns 0 if none needed, 1 if one added, -1 if no room to add.  */

int addext(char *filename, char *ext)
  {
  int i;
  for (i=0; filename[i]!='\0'; i++) if (filename[i]=='.') return(0);
  if ((strlen(filename)+strlen(ext)+1)>MAXFILE) {
    printf("No room to add default extension .%s\n", ext);
    return(-1);}
  filename[i]='.';            /* add the dot */
  strcpy(&filename[i+1],ext); /* and the extension */
  return(1);
  }  /* addext */

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
       c=(unsigned char)toupper(string[i]);
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

/* ----- GETCOL(string, maxcols) ---------------------------------- */
/* Convert string name to PM CLR_xxxx                               */
/* If maxcols=8  only bright colours allowed                        */
/* If maxcols=9  only bright colours plus black allowed             */
/* If maxcols=15 all colours except Pale Grey allowed               */
/* If maxcols=16 all colours allowed                                */
/* Returns BADLONG if unknown.                                      */
long int getcol(char *s, int maxcols) {
  if (streq(s,"WHITE"        )) return CLR_WHITE    ;
  if (streq(s,"RED"          )) return CLR_RED      ;
  if (streq(s,"GREEN"        )) return CLR_GREEN    ;
  if (streq(s,"BLUE"         )) return CLR_BLUE     ;
  if (streq(s,"PINK"         )) return CLR_PINK     ;
  if (streq(s,"MAGENTA"      )) return CLR_PINK     ;
  if (streq(s,"CYAN"         )) return CLR_CYAN     ;
  if (streq(s,"TURQUOISE"    )) return CLR_CYAN     ;
  if (streq(s,"YELLOW"       )) return CLR_YELLOW   ;
  if (streq(s,"GRAY"         )) return CLR_DARKGRAY ;
  if (streq(s,"GREY"         )) return CLR_DARKGRAY ;
  if (maxcols<=8) return BADLONG;
  if (streq(s,"BLACK"        )) return CLR_BLACK    ;
  if (maxcols<=9) return BADLONG;
  if (streq(s,"DARKRED"      )) return CLR_DARKRED  ;
  if (streq(s,"DARKGREEN"    )) return CLR_DARKGREEN;
  if (streq(s,"DARKBLUE"     )) return CLR_DARKBLUE ;
  if (streq(s,"PURPLE"       )) return CLR_DARKPINK ;
  if (streq(s,"DARKPINK"     )) return CLR_DARKPINK ;
  if (streq(s,"DARKMAGENTA"  )) return CLR_DARKPINK ;
  if (streq(s,"DARKCYAN"     )) return CLR_DARKCYAN ;
  if (streq(s,"DARKTURQUOISE")) return CLR_DARKCYAN ;
  if (streq(s,"BROWN"        )) return CLR_BROWN    ;
  if (maxcols<=15) return BADLONG;
  if (streq(s,"PALEGRAY"     )) return CLR_PALEGRAY ;
  if (streq(s,"PALEGREY"     )) return CLR_PALEGRAY ;
  return BADLONG;
  } /* getcol */

/* ----- PUTCOL(string, long) ------------------------------------- */
/*                                                                  */
void putcol(char *string, long col) {
  switch((int)col) {
    case CLR_DARKRED:    strcpy(string,"DARKRED");   break;
    case CLR_RED:        strcpy(string,"RED");       break;
    case CLR_DARKGREEN:  strcpy(string,"DARKGREEN"); break;
    case CLR_GREEN:      strcpy(string,"GREEN");     break;
    case CLR_DARKBLUE:   strcpy(string,"DARKBLUE");  break;
    case CLR_BLUE:       strcpy(string,"BLUE");      break;
    case CLR_DARKCYAN:   strcpy(string,"DARKCYAN");  break;
    case CLR_CYAN:       strcpy(string,"CYAN");      break;
    case CLR_DARKPINK:   strcpy(string,"PURPLE");    break;
    case CLR_PINK:       strcpy(string,"PINK");      break;
    case CLR_BROWN:      strcpy(string,"BROWN");     break;
    case CLR_YELLOW:     strcpy(string,"YELLOW");    break;
    case CLR_WHITE:      strcpy(string,"WHITE");     break;
    case CLR_BLACK:      strcpy(string,"BLACK");     break;
    case CLR_DARKGRAY:   strcpy(string,"GRAY");      break;
    case CLR_PALEGRAY:   strcpy(string,"PALEGRAY");  break;
    } /* switch */
  return;
  } /* putcol */

/* ----- GETLONG(string) ------------------------------------------ */
/*                                                                  */
/* Replacement for standard ATOL.  This one checks the whole input  */
/* word, and returns BADLONG if an error is found or the number is  */
/* more than 9 digits.                                              */

long int getlong(char *string)
  {long int result=0;
   long int neg=0;
   int i=0, j, k;
   if (string[0]=='+') i=1;                  /* handle sign */
    else if (string[0]=='-') {i=1; neg=1;}

   for (j=i, k=i+9; ; ) {
     if (!isdigit(string[j])) return BADLONG;
     result=result*10+string[j]-'0';
     j++;
     if (string[j]=='\0') break;
     if (j>=k) return BADLONG;
     }
  if (!neg) return(result);
  return(-result);
  } /* Getlong */

/* ----- GETMILL(string) ------------------------------------------ */
/*                                                                  */
/* Takes a string (in the form of a decimal number) and converts    */
/* it to a LONG in which one unit indicates one thousandth of a     */
/* unit of the original number.                                     */
/*                                                                  */
/* For example, if the original string were "12.345" then the       */
/* returned result would be 12345, and if the original string were  */
/* "-12" then -12000 would be returned.                             */
/*                                                                  */
/* Returns BADLONG if there are more than six digits before the     */
/* decimal point or if there are more than three after, or if any   */
/* other error was found (e.g., invalid characters).                */

long int getmill(char *string)
  {long int result=0;              /* accumulator */
   int neg=0;                      /* had a minus sign */
   int haddot=0;                   /* had a decimal point */
   long factor=1;                  /* multiplication factor */
   int i=0;                        /* input offset point */
   int j,k;
   if (string[0]=='+') i=1;        /* handle sign */
    else if (string[0]=='-') {i=1; neg=1;}

   for (j=i, k=i+6; ; ) {          /* allow 6 digits before '.' */
     if (string[j]=='.') {
       if (haddot) return BADLONG;
       haddot=1; k++;}
      else { /* digit expected */
       if (j>=k && !haddot) return BADLONG; /* overflow */
       if (!isdigit(string[j])) return BADLONG;
       result=result*10+(string[j]-'0');
       if (haddot) {               /* are in decimals */
         if (factor>=1000) return BADLONG;  /* >3 decimals */
         factor=factor*10;
         }
       }
     j++;
     if (string[j]=='\0') break;
     }

  result=result*(1000/factor);          /* allow for decimal point */
  if (!neg) return result;              /* .. and sign */
  return -result;
  } /* getmill */

/* ----- PUTMILL(string *, long) ---------------------------------- */
/*                                                                  */
/* Takes a long, representing a number *1000, and converts it into  */
/* a string.  The target string may be up to 12 characters, plus    */
/* a terminator.  Trailing 0's (and decimal point) removed.         */
/*                                                                  */
/* For example, if long integer were -12345 then "-12.345" would be */
/* returned.                                                        */
void putmill(char *string, long num)
  {
  char *c;
  if (num>=0)
    sprintf(string,"%li.%03li",   num/1000L,  num%1000L);
   else
    sprintf(string,"-%li.%03li", -num/1000L, -num%1000L);
  for (c=string+strlen(string)-1; ;c--) {
    if (*c=='.') {*c='\0'; break;}      /* done - an integer */
    if (*c!='0') break;                 /* done - not a trailing 0 */
    *c='\0';                            /* trailing 0 */
    } /* c */
  } /* putmill */

/* ----- GETTIME(string) ------------------------------------------ */
/*                                                                  */
/* Takes a string (in the form of a time) and converts it to a      */
/* LONG in which one unit indicates one second.  A leading "+" or   */
/* "-" is allowed, allowing for time offsets; the result can be     */
/* negative.                                                        */
/*                                                                  */
/* No blanks are allowed: the seconds, or seconds and minutes, can  */
/* be omitted, as can leading 0's.  The syntax is therefore:        */
/*                                                                  */
/*   [+|-][h]h[:[m]m[:[s]s]]                                        */
/*                                                                  */
/* For example, if the original string were "12:30:15" then the     */
/* returned result would be 12*3600+30*60+15, or 45015.             */
/*                                                                  */
/* Returns BADLONG if the hour is >24, or the minutes/seconds are   */
/* greater than 59, or the time is >+/-24:00:00, or other error.    */

long int gettime(char *string)
  {long result;                    /* accumulator */
   int neg=0;                      /* had a minus sign */
   int part[3];                    /* part of the result */
   int i=0;                        /* input offset point */
   int p,k;
   if (string[0]=='+') i=1;        /* handle sign */
    else if (string[0]=='-') {i=1; neg=1;}
   if (!isdigit(string[i])) return BADLONG;

   for (p=0; p<3; p++) part[p]=0;  /* 0's */

   for (p=0; p<3; p++, i++) {
     /* each part must have at least one digit */
     if (!isdigit(string[i])) return BADLONG;
     for (k=0; k<2; k++, i++) {
       if (!isdigit(string[i])) break;
       /* get a digit */
       part[p]=part[p]*10+(string[i]-'0');
       } /* k */
     if (string[i]=='\0') break;                  /* done */
     if (string[i]!=':' || p==2) return BADLONG;  /* yechh */
     }
   if (string[i]!='\0')  return BADLONG;     /* junk on end */
   if (part[1]>59 || part[2]>59) return BADLONG;
   result=(long)(part[2]+60*part[1])+(long)part[0]*3600L;
   if (result>86400L) return BADLONG;
   /* Looks good! */
   if (!neg) return result;             /* .. note sign */
   return -result;
   } /* gettime */

/* ----- GETWORD(string,returnword) ------------------------------- */
/*                                                                  */
/* The first word (plus leading blanks and one delimiter blank)     */
/* is removed from STRING and returned in RETURNWORD.  The          */
/* RETURNWORD will contain no blanks and is limited to MAXTOK       */
/* characters (if the first word is more than that, it will be      */
/* broken at MAXTOK characters).                                    */
/* If no first word found, RETURNWORD[0] will be 0.                 */
/*                                                                  */
/* void getword(char *, char *);                                    */

void getword(char *string, char *fword)
  {
  int i, j;                        /* Work */
  fword[0]='\0';                   /* Default null string */
  if (string[0]=='\0') return;     /* Input string is empty */
  for (i=0; string[i]==' ';) i++;  /* Skip leading blanks */
  if (string[i]!=0) {              /* have some non-blanks */
    for (j=0; j<MAXTOK ; j++) {    /* collect the word */
      fword[j]=string[i];          /* .. */
      i++; if (string[i]==' ') {i++; break;}  /* Skip delim */
      if (string[i]==0) break;     /* End of string */
      } /* collect word */
    j++; fword[j]=0;               /* Add terminator to word */
    } /* Some non blanks */
  /* Remove header (blanks, word, delim) from source string */
  for(j=0; string[i]!=0; i++, j++) string[j]=string[i];
  string[j]=0;                     /* Add terminator to string */
  return;
  } /* Getword */

/* ----- STREQ(string1,string2) ----------------------------------- */
/*                                                                  */
/* Return 1 if strings caseless-compare equal, 0 otherwise          */
/*                                                                  */
/*  int streq(char *, char *);                                      */

int streq(char *char1, char *char2)
  {
  for (;;char1++, char2++) {
    if (*char1==*char2) {if (*char1=='\0') return 1;}
     else if (toupper(*char1)!=toupper(*char2)) return 0;
    } /* forever */
  return 0;              /* for C Set/2 */
  } /* Streq */


/* ------------------------------------------------------------------ */
/* LSQRT -- return the square root of a non-negative LONG integer     */
/* Note: the maximum input value is 2**31-1 (2147483647)              */
/*       hence the maximum possible returned value is 46340           */
/* ------------------------------------------------------------------ */
long lsqrt(long num) {
  long new;
  long old;
  int  in, inew, iold, isub;
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
  static int lastout=300;     /* last result is next time's guess */

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
    /* For the first guess we use the last result */
    /** (Old code in case we need it one day) ....
    for (k=14; k>=0; k--) if (num>ltwopower[k]) break;
    old=power[k+1];  **/      /* first guess */
    old=lastout;
    for (;;) {                /* forever */
      /* if (old==0) printf("SqrtboomL on %li, lastout %i\n", num, lastout);*/
      /* code variation here because of a 16-bit compiler problem */
      #if defined(B16)
      new=((num/old)+old)>>1; /* new guess */
      #else
      /* This generates bad code on IBM C/2 */
      new=(signed)((unsigned)((num/old)+old)>>1L);/* new guess */
      #endif
      if ((new-old)<2
       && (new-old)>-2) {
        lastout=(int)new;
        return new;}  /* Done! */
      old=new;
      }
    } /* using longs */
   else {                     /* use shorts */
    in=(int)num;              /* copy */
    /* For the first guess we use root of a power of two comparable */
    /** (Old code in case we need it one day) ...
    for (k=7; k>=1; k--) if (in>twopower[k]) break;
    iold=power[k+1]; */       /* first guess */
    iold=lastout;
    for (;;) {                /* forever */
      inew=(signed)((unsigned)((in/iold)+iold)>>1);      /* new guess */
      isub=inew-iold;
      if (isub<2 && isub>-2) {
        lastout=inew;
        return (long)inew;}
      iold=inew;
      }
    } /* using shorts */

  return 0;                   /* for C Set/2 */
  } /* lsqrt */


/* ------------------------------------------------------------------ */
/* FASTRAND -- return a random number in the range 0 to N-1           */
/*                                                                    */
/* ------------------------------------------------------------------ */
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