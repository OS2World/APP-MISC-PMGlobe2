/* ------------------------------------------------------------------ */
/* PMGlobe -- a geographical application                              */
/* GLDO    -- Command executer                                        */
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
/* Common returncodes:                                                */
/*    0 Good                                                          */
/*   -1 Unknown command                                               */
/*   -2 Resource not available                                        */
/*                                                                    */
/* ------------------------------------------------------------------ */
/* Implemented commands:                                              */
/*   GERASE         [TRACK|DRAW]                                      */
/*   GLINE lat lon  [TRACK|DRAW]                                      */
/*   GMOVE lat lon  [TRACK|DRAW]                                      */
/*   MARK lat lon [X x-offset] [Y y-offset]                           */
/*     [MARKER m] [CLOCK offset] [LABEL rest...]                      */
/*   MARK lat lon DELETE                                              */
/*   FONT  name [SIZE n] [COLOR color] [FACE facename]                */
/*   MACRO file parameters...                                         */
/*   MESSAGE text                                                     */
/*   REDRAW [ALL]                                                     */
/*   QUERY various                                                    */
/*   SET   various                                                    */
/*   CLOSE                                                            */
/*   TEST                                                             */
/*   WAIT [REDRAW] [[+]time]                                          */
/*                                                                    */
/* x-offset and y-offset may be CENTRE or CENTER, and max +/-1000     */
/* COLOR may be COLOUR everywhere                                     */
/*                                                                    */
/* Not implemented:                                                   */
/*   WINDOW [SIZE x y] [POSITION x y]                                 */
/*     [MINIMIZE | MAXIMIZE | RESTORE] [SHOW | HIDE]                  */
/* ------------------------------------------------------------------ */

#define  INCL_DOSPROCESS
#define  INCL_GPIPRIMITIVES
#define  INCL_GPILCIDS
#include <time.h>
#include "globe.h"
#include "globepm.h"

/* subroutines */
extern int streq(char *, char *);       /* string compare */
extern int glrexx(char *, char *);      /* run rexx macro */
extern int glquer(char *);              /* query items */
extern int glextr(char *);              /* extract items */
extern int glset (char *);              /* set item */
extern void newdraw(void);              /* force a new draw cycle, please */
extern void distcalc(long, long, long, long, long *, long *); /* distance */
void bad(char *);                       /* error in... */

char *original;                         /* shared pointer */

int gldo(char *comm) {
 char in[MAXCOMM+1];                    /* local copy */
 char verb[MAXTOK+1];                   /* verb */
 char word[MAXTOK+1];                   /* work */
 char buff[MAXTOK+51];                  /* work for msgs */
 int rc;

 strcpy(in,comm);                       /* make local copy */
 original=comm;                         /* .. */

 if (commhalt) {
   if (diag) printf("Halted '%s'\n", in);    /* indicate halt */
   return 0;}

 if (diag) printf(">> '%s'\n", in);     /* trace */
 getword(in, verb);

 /* Graphics and measure commands first -- may be a lot */
 if (streq(verb,"GMOVE")                /* Graphics Move */
  || streq(verb,"GLINE")) {             /* Graphics Line */
   long lat, lon, colour=drawcoli;
   unsigned long size;
   struct drawlist *pd, *plast;         /* pointers */
   struct drawlist **pbase=&drawbase;   /* pointers */

   getword(in, word); lat=getdeg(word,CHECKLAT);
   if (lat==BADLONG || labs(lat)>90000) {
     bad("Invalid latitude"); return 1;}
   getword(in, word); lon=getdeg(word,CHECKLON);
   if (lon==BADLONG || labs(lon)>180000) {
     bad("Invalid longitude"); return 1;}
   getword(in, word); if (word[0]!='\0') {
    if      (streq(word, "TRACK")) {pbase=&trackbase; colour=trackcoli;}
    else if (streq(word,"DRAW" ))  {pbase=&drawbase;  colour=drawcoli; }
    else {bad("Junk"); return 1;}}

   /* Allocate and set display list block */
   size=sizeof(struct drawlist);
   pd=(struct drawlist *)malloc((size_t)size);     /* allocate space */
   if (pd==NULL) {bad("No memory available"); return(-2);}

   pd->next=NULL;                       /* will be end of chain */
   pd->lon=lon; pd->lat=lat;            /* point */
   pd->coli=colour;                     /* current color */
   pd->drawn=FALSE;                     /* flag */
   if (verb[1]=='M'
    || verb[1]=='m') pd->order=DRAW_MOVE;
    else       /*L*/ pd->order=DRAW_LINE;

   /* If non-empty chain then find address of last */
   if (*pbase!=NULL)
     for (plast=*pbase; plast->next!=NULL;) plast=plast->next;
   /* Now add new item to tail of chain */
   DosEnterCritSec();                   /* for drawer */
   if (*pbase==NULL) *pbase=pd;         /* empty list case */
    else plast->next=pd;
   if (pd->order==DRAW_LINE) {draws++;  /* if making ink, bump count */
                                        /* Just a rising edge detector */
     redrawsprite=TRUE;}                /* [B&B] force redraw */
   DosExitCritSec();
   return 0;} /* Gmove/Gline */

 if (streq(verb,"GERASE")) {            /* Graphics Erase */
   /* This erases all entries, including trackings */
   struct drawlist *pd, *pfirst, *pfree;/* pointers */
   struct drawlist **pbase=&drawbase;   /* .. */

   getword(in, word); if (word[0]!='\0') {
    if      (streq(word, "TRACK")) pbase=&trackbase;
    else if (streq(word,"DRAW" ))  pbase=&drawbase;
    else {bad("Junk"); return 1;}}

   /* Unhang whole chain */
   DosEnterCritSec();                   /* for drawer */
   pfirst=*pbase;
   *pbase=NULL;
   redrawsprite=TRUE;                   /* force redraw */
   draws=0;                             /* no draw commands known */
   DosExitCritSec();

   /* Discard old chain now */
   for (pd=pfirst; pd!=NULL; ) {pfree=pd; pd=pd->next; free(pfree);}
   return 0;} /* Gerase */

 if (streq(verb,"MARK")) {              /* command MARK */
   long lat, lon;
   unsigned long size;
   long tx, ty;
   char thismark;
   struct llmark *pll, *plast, *pprev;  /* pointers */
   struct llmark *pbest, *pbestprev;    /* .. */
   char text[MAXTEXT+1];
   long mclock=BADLONG;

   getword(in, word); lat=getdeg(word,CHECKLAT);
   if (lat==BADLONG || labs(lat)>90000) {
     bad("Invalid latitude"); return 1;}
   getword(in, word); lon=getdeg(word,CHECKLON);
   if (lon==BADLONG || labs(lon)>180000) {
     bad("Invalid longitude"); return 1;}

   getword(in, word);
   if (streq(word,"DELETE")) {
     getword(in, word); if (word[0]!='\0') {bad("Junk"); return 1;}
     /* Spin down the chain to find the last entry that matches */
     /* Could improve performance here with a double-linked chain */
     pbest=NULL; pprev=NULL;            /* none so far */
     for (pll=markbase; pll!=NULL; pll=pll->next) {
       if (pll->lat==lat && pll->lon==lon) { /* a match */
         pbest=pll; pbestprev=pprev;}
       pprev=pll;
       } /* pll */
     if (pbest==NULL) {bad("Unmatched lat/lon"); return 2;}
     /* OK, we found a mark to delete */
     DosEnterCritSec();                           /* for drawer */
     /* drop from chain */
     marks--;                                     /* drop counts.. */
     if (pbest->clock!=BADLONG) clockmarks--;     /* .. */
     if (pbestprev==NULL) markbase=pbest->next;   /* head of list case */
      else pbestprev->next=pbest->next;
     redrawsprite=TRUE;                           /* force redraw */
     DosExitCritSec();
     free(pbest);                                 /* done with entry */
     return 0;} /* delete */

   /* ----- Here if "MARK l l ...", not DELETE ----- */
   /* Lat/Lon cannot be defaulted.  Rest are: */
   tx=3; ty=3;                          /* offsets defaults */
   text[0]='\0';                        /* no text yet */
   thismark=curmark;                    /* current mark */
   for (;word[0]!='\0';) {              /* while some options */
     if (streq(word,"LABEL")) {
       if (strlen(in)>MAXTEXT) {bad("Text too long"); return 1;}
       strcpy(text,in);
       break;}                          /* TEXT eats rest */

     else if (streq(word,"CLOCK")) {getword(in,word); mclock=getlong(word);
       if (mclock==BADLONG || labs(mclock)>(24L*60L*60L))
         {bad("Bad clock value"); return 1;}
       }
     else if (streq(word,"X")) {getword(in,word);
       if (streq(word,"CENTER") || streq(word,"CENTRE")) tx=BADSHORT;
        else {tx=getlong(word);
         if (tx==BADLONG || labs(tx)>1000) {bad("Bad X value"); return 1;}}
       }
     else if (streq(word,"Y")) {getword(in,word);
       if (streq(word,"CENTER") || streq(word,"CENTRE")) ty=BADSHORT;
        else {ty=getlong(word);
         if (ty==BADLONG || labs(ty)>1000) {bad("Bad Y value"); return 1;}}
       }
     else if (streq(word,"MARKER")) {
       getword(in,word);
       if (streq(word,"OFF")) thismark=' ';
        else {
         if (strlen(word)!=1) {bad("MARKER must be one character"); return 1;}
         thismark=word[0];}
       }
     else {
       sprintf(buff, "Bad keyword '%s'", word);
       bad(buff); return 1;}
     getword(in, word);                 /* next word please */
     } /* forever */

   /* All options OK .. allocate new mark and initialize */

   /* Now try to allocate a structure to hold the mark.. */
   size=sizeof(struct llmark)+strlen(text);
   pll=(struct llmark *)malloc((size_t)size);     /* allocate space */
   if (pll==NULL) {bad("No memory available"); return(-2);}

   /* Initialize the structure */
   pll->next=NULL;                      /* will be at end of chain */
   pll->lat=lat; pll->lon=lon;          /* Lat/Long */
   pll->drawn=FALSE;                    /* clear flag */
   pll->clock=mclock;                   /* clock offset */
   pll->tx=(short)tx; pll->ty=(short)ty;/* text offsets */
   pll->tfont=curfont;                  /* currents */
   pll->mcoli=drawcoli;                 /* .. */
   pll->mark=thismark;                  /* .. */
   strcpy(pll->text,text);              /* the text string */
   /* If non-empty chain then find address of last */
   if (markbase!=NULL)
     for (plast=markbase; plast->next!=NULL;) plast=plast->next;
   /* Now add new item to tail of chain */
   DosEnterCritSec();                   /* for drawer */
   if (markbase==NULL) markbase=pll;    /* empty list case */
    else plast->next=pll;
   marks++;                             /* It's there: bump counts */
   if (pll->clock!=BADLONG) clockmarks++;
   DosExitCritSec();
   return 0;} /* mark */

 if (streq(verb,"FONT")) {              /* command FONT */
   struct logfont *plf, *plast;         /* work */
   static FATTRS fat;                   /* static so 0's */
   char name[MAXNAME+1];
   char face[MAXTEXT+1];
   long psize, fsize;                   /* font size in points, pels */
   long lrc, lcid, lcol;                /* work */
   unsigned long size;                  /* .. */
   long i;

   name[0]='\0'; face[0]='\0'; psize=12; lcol=CLR_YELLOW;
   if (fonts>=100) {bad("Too many fonts"); return 4;}

   getword(in, word);                   /* get name */
   if (word[0]=='\0') {bad("No name"); return 1;}
   if (strlen(word)>MAXNAME) {bad("Name too long"); return 1;}
   strcpy(name,word);                   /* save */
   for (;;) {
     getword(in, word);
     if (word[0]=='\0') break;          /* got all options */
     if (streq(word,"SIZE")) {
       getword(in, word); psize=getlong(word);
       if (psize==BADLONG || psize<1 || psize>100) {bad("Bad size"); return 1;}
       }
     else if (streq(word,"FACE")) {
       if (strlen(in)>MAXTEXT || strlen(in)>sizeof(fat.szFacename))
         {bad("Face name too long"); return 1;}
       strcpy(face,in);
       /* strip leading and trailing blanks (rare) */
       for (; face[0]==' ';) strcpy(face, &face[1]);
       for (i=(signed)(strlen(face)-1); i>=0 && face[i]==' '; i--) face[i]='\0';
       break;}                          /* FACE eats rest */
     else if (streq(word,"COLOR") || streq(word,"COLOUR")) {
       getword(in, word); lcol=getcol(word, 15);  /* Not pale grey */
       if (lcol==BADLONG) {bad("Bad color"); return 1;}
       }
     else {
       sprintf(buff, "Bad keyword '%s'", word);
       bad(buff); return 1;}
     } /* forever loop */

   /* valid syntax, now some semantics */
   plf=fontbase;                        /* -> start of chain */
   for (i=0; i<fonts; i++) {
     if (streq(plf->fname,name)) {bad("Font already defined"); return 2;}
     plast=plf;                         /* save for later use */
     plf=plf->next;
     }

   /* convert size in points to size in (nearest) pels */
   fsize=(psize*fontresy+36L)/72L;
   if (diag) printf("Font '%s': %li (%li pels) color %li \"%s\"\n",
     name, psize, fsize, lcol, face);

   /* Locate and create the logical font */
   fat.usRecordLength=sizeof(fat);
   strcpy(fat.szFacename, face);
   fat.usCodePage=850;
   fat.lMatch=0;                   /* 0 means find nearest */
   fat.lMaxBaselineExt=fsize;      /* size in pels */
   if (face[0]) fat.fsFontUse=FATTR_FONTUSE_OUTLINE; /* no bitmaps please */
           else fat.fsFontUse=0L;  /* .. unless 'default' */

   lcid=fonts+1;
   /* Now try and create the font against the window PS */
   /* (not against the true bitmap, as it may not yet exist) */
   lrc=GpiCreateLogFont(cps, NULL, lcid, &fat);
   if (diag) printf("RC from GCLF: %li\n", lrc);
   if (lrc==GPI_ERROR) {bad("Unable to find font"); return 3;}
   if (diag) {
     FONTMETRICS ff;                    /* see what we got */
     GpiSetCharSet(cps, lcid);
     GpiQueryFontMetrics(cps,(long)sizeof(FONTMETRICS), &ff);
     printf("> Family: '%s'\n", ff.szFamilyname);
     printf("> Face: '%s'\n", ff.szFacename);
     printf("> Maxa, Maxd: %li %li\n", ff.lMaxAscender, ff.lMaxDescender);
     printf("> Points: %i\n", ff.sNominalPointSize);
     printf("> Type, Defn: %x %x\n", ff.fsType, ff.fsDefn);
     GpiSetCharSet(cps, LCID_DEFAULT);
     }
   GpiDeleteSetId(cps, lcid);           /* and free the LCID */

   /* Looks good: add a new structure */
   size=sizeof(struct logfont);
   plf=(struct logfont *)malloc((size_t)size);    /* allocate space */
   if (plf==NULL) {bad("No memory available"); return(-2);}

   plf->next=NULL;                      /* will be at end of chain */
   plf->flcid=lcid;                     /* logical PM id */
   plf->fsize=fsize;                    /* requested font size */
   plf->fcoli=lcol;                     /* colour */
   strcpy(plf->face, face);             /* set face .. */
   strcpy(plf->fname,name);             /* .. and nickname */

   /* Now add new item to tail of chain */
   DosEnterCritSec();                   /* for drawer */
   if (fontbase==NULL) fontbase=plf;    /* empty list case */
    else plast->next=plf;
   fonts++;                             /* It's there: bump count */
   DosExitCritSec();
   return 0;} /* font */

 if (streq(verb,"EXTRACT"))             /* Extract values */
    {rc=glextr(in); return rc;}

 if (streq(verb,"SET"))                 /* Set a value */
    {rc=glset(in); return rc;}

 if (streq(verb,"Q")
    ||streq(verb,"QUERY"))              /* Query values */
    {rc=glquer(in); return rc;}

 if (streq(verb,"REDRAW")) {            /* command REDRAW */
   getword(in, word);
   if (streq(word,"ALL")) redrawall=TRUE;
    else if (word[0]!='\0') return 2;
   WinPostMsg(hwndClient, UM_REDRAW, NULL, NULL); /* Asynch */
   return 0;} /* redraw */

 if (streq(verb,"MACRO")) {             /* command MACRO file parms */
   char file[MAXFILE+1];                /* filename we allow */
   /* (Currently limited to MAXTOK rather than MAXFILE) */
   getword(in, file); if (!file[0]) return -3;  /* no file! */
   addext(file,"pmg");                  /* default extension */
   rc=glrexx(file, in);                 /* run it, with parms */
   /* glrexx handles error reporting */
   return rc;} /* macro */

 if (streq(verb,"MESSAGE")) {           /* command MESSAGE */
   WinMessageBox(HWND_DESKTOP, HWND_OBJECT,
     in, "PMGlobe message", 0,
     MB_OK | MB_MOVEABLE);
   return 0;} /* message */

 if (streq(verb,"TEST")) {              /* command TEST */
   test=(FLAG)!test;
   WinPostMsg(hwndFrame, UM_TEST, NULL, NULL); /* Asynch */
   return 0;} /* test */

 if (streq(verb,"WAIT")) {              /* command WAIT */
   /* format WAIT [REDRAW] [time] [+time]  (may be >1 times) */
   long timesecs, endsecs;              /* Local seconds since 1 Jan 1970 */
   long secs;                           /* work */
   unsigned long sleep;                 /* sleep time this iteration */
   long abssecs=MAXLONG;                /* absolute time */
   long relsecs=MAXLONG;                /* relative time */
   int  red=FALSE;                      /* wait for redraw */
   int  rel=FALSE;                      /* relative time */
   for (;;) {
     getword(in, word);
     if (word[0]=='\0') break;          /* done all words */
     if (streq(word,"REDRAW")) {red=TRUE; continue;}
     /* time expected */
     if (word[0]=='-') {                /* negative relative time(!) */
       bad("Negative time"); return 2;}
     if (word[0]=='+') {                /* relative time */
       rel=TRUE; strcpy(word,&word[1]);}
     secs=gettime(word);                /* see what we got */
     if (secs==BADLONG) {               /* yecch */
       bad("Invalid time"); return 2;}
     if (rel) relsecs=lmin(relsecs,secs);
         else abssecs=lmin(abssecs,secs);
     } /* for loop */

   /* If we have an absolute time, then compare with TOD and convert */
   /* to relative seconds, then use smaller of this and RELSECS */
   if (abssecs!=MAXLONG) {
     struct tm *stime;
     time(&timesecs);                   /* get Now */
     stime=localtime(&timesecs);        /* .. to structure */
     strcpy(buff, asctime(stime));      /* .. to string */
     buff[19]='\0';                     /* terminate the word */
     secs=gettime(&buff[11]);           /* convert what we got */
     if (secs==BADLONG) secs=abssecs;   /* just in case a nasty */
     secs=abssecs-secs;                 /* now relative */
     if (secs<0) secs=secs+24L*3600L;   /* next day please */
     if (diag) printf("Absolute time now +%li seconds\n", secs);
     relsecs=lmin(relsecs,secs);        /* use near-in time */
     rel=TRUE;                          /* for easy test later */
     } /* absolute */
    else if (rel) time(&timesecs);      /* also need Now */

   /* OK, ready to wait.  If DRAW then start a drawing, to ensure */
   /* DRAWING flag activated */
   if (!red && !rel) return 0;          /* nothing to wait for */
   if (red) newdraw();
   if (rel) endsecs=timesecs+relsecs;   /* when to end */

   /* now loop -- with up to 2.0-second sleeps -- until done */
   if (red) sleep=30L;                  /* initial sleep time if redraw */
    else    sleep=1000L;                /* else start with 1-sec wait */
   for (;;) {
     if (commhalt) {                    /* external halt request */
       if (diag) printf("WAIT cancelled by Halt\n");
       break;}
     if (red && !drawing) break;        /* done */
     if (rel) {
       time(&timesecs);                 /* get new Now */
       if (timesecs>=endsecs) break;}   /* timed out */
     sleep=(unsigned)lmin(sleep*31/30,2000L);     /* new sleep pause */
     DosSleep(sleep);                             /* wait the while... */
     } /* wait loop */
   return 0;} /* wait */

 if (streq(verb,"CLOSE")) {             /* command CLOSE */
   WinPostMsg(hwndClient, WM_CLOSE, NULL, NULL);
   return 0;} /* redraw */

 if (verb[0]=='\0') return 0;           /* let's ignore these */
 errbox("Command '%s' is unknown", comm);
 return -1;
 } /* gldo */

/* ----- "Error in..." routine ----- */
void bad(char *msg) {
 errbox("Error: %s in '%s'", msg, original);
 } /* bad */

