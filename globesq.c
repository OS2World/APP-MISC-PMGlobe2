/* ------------------------------------------------------------------ */
/* PMGlobe -- a geographical application                              */
/* GLOBESQ -- SET and QUERY commands                                  */
/*   GLSET  -- set     command                                        */
/*   GLQUER -- query   command                                        */
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
/*                                                                    */
/* The actual querying is moved to a subroutines, which are also      */
/* called by the EXTRACT command.                                     */
/* GLEXTR, the extract command, is with other Rexx stuff, in GLOBEC.C */

#define  INCL_DOSPROCESS
#include "globe.h"
#include <time.h>

#define MAXTIMEOFFSET 3600L*24L*366L

/* Subroutine and external definitions */
char * qvalue(char *);                  /* query item value */
char * qflag(FLAG);                     /* query state of a flag */
int    junk(char *);                    /* test for junk in string */
int    onoff(char *, FLAG *);           /* set item ON/OFF/INVERT */
int    setdeg(char *, long *, long);    /* set item to degrees */
extern void newview(long, long);        /* set new view */
extern void check(int, FLAG);           /* (un)check a menu item */
extern void setsun(long,long *, long *);/* set SUN lat, lon */
extern void newref(long);               /* set new refresh seconds */
extern void newtwi(long);               /* set new twilight degrees */
extern void newback(long);              /* set new background colour */
extern void newgrid(long);              /* set new grid colour */
extern void newland(long);              /* set new land colour */
extern void newsea(long);               /* set new sea colour */

extern FLAG diagmsg;                    /* message trace flag */

/* ------------------------------------------------------------------ */
/* GLSET: Globe SET a value                                           */
/* ------------------------------------------------------------------ */
static char item[MAXTOK+1];             /* item to be set */
                                        /* (shared for messages) */
int glset(char inoptions[]) {
  char in[MAXCOMM+1];                   /* copy of options */
  char word[MAXTOK+1];                  /* work */
  long lval;
  int rc;

  strcpy(in,inoptions);                 /* safe working copy */

  getword(in,item);                     /* get item */
  strupr(item);                         /* save time, and for msgs */
  if (item[0]=='\0') {
    errbox("No item given.  Try 'SET item value'");
    return 2;}
  if (streq(item,"MARKERS"))   {rc=onoff(in,&markers);   return rc;}
  if (streq(item,"GRAPHICS"))  {rc=onoff(in,&graphics);  return rc;}
  if (streq(item,"CLOCKS"))    {rc=onoff(in,&clocks);    return rc;}
  if (streq(item,"CLOCKCIVIL")){rc=onoff(in,&civiltime); return rc;}
  if (streq(item,"CLOCKDAY"))  {rc=onoff(in,&clockday);  return rc;}
  if (streq(item,"LABELS"))    {rc=onoff(in,&labels);    return rc;}
  if (streq(item,"DIAG"))      {rc=onoff(in,&diag);      return rc;}
  if (streq(item,"DIAGMSG"))   {rc=onoff(in,&diagmsg);   return rc;}
  if (streq(item,"ERRORBOX"))  {rc=onoff(in,&errorbox);  return rc;}
  if (streq(item,"SHOWDRAW"))  {rc=onoff(in,&showdraw);  return rc;}

  if (streq(item,"MARKER"))  {
    getword(in,word);                   /* get value */
    if (junk(in)) return 4;
    if (streq(word,"OFF"))   {curmark=' '; return 0;}
    if (word[1]=='\0') {     /* looks hopeful */
      if (word[0]=='.' || word[0]=='+'
       || word[0]=='x' || word[0]=='X') {curmark=word[0]; return 0;}
      }
    errbox("'%s is not valid for SET %s", word, item);
    return 1;}

  if (streq(item,"MARGIN")) {      /* Margin (% * 100) */
    getword(in,word); if (junk(in)) return 4; lval=getmill(word);
    if (lval!=BADLONG && lval>=0 && lval<50000) { /* is good */
      margin=lval; return 0;}
    errbox("'%s' is not valid for SET %s", word, item);
    return 3;}

  if (streq(item,"USEROFFSET")) {  /* User time offset */
    getword(in,word); if (junk(in)) return 4; lval=getlong(word);
    if (lval!=BADLONG && labs(lval)<=MAXTIMEOFFSET) {  /* is good */
      useroffset=lval; return 0;}
    errbox("'%s' is not valid for SET %s", word, item);
    return 3;}

  if (streq(item,"VIEWLAT")) {     /* View latitude */
    rc=setdeg(in,&lval,CHECKLAT); if (rc!=0) return rc;
    newview(lval, viewlon);
    return 0;} /* viewlat */
  if (streq(item,"VIEWLON")) {     /* View longitude */
    rc=setdeg(in,&lval,CHECKLON); if (rc!=0) return rc;
    newview(viewlat, lval);
    return 0;} /* viewlon */

  if (streq(item,"TWILIGHT")) {    /* Twilight degrees */
    rc=setdeg(in,&lval,18000L); if (rc!=0) return rc;
    if (lval<0) {
      errbox("Degrees may not be negative in SET %s", item);
      return 3;}
    newtwi(lval);
    return 0;} /* twilight */

  if (streq(item,"SUNLIGHT")) {
    if (gmtoffset==BADLONG) return -1;  /* protect */
    rc=onoff(in,&sunlight); if (rc!=0) return rc;
    check(ID_SUN, sunlight);
    if (!sunlight) {
      check(ID_SPACE, spacelight=FALSE);
      check(ID_STAR,  starlight =FALSE);
      }
    return 0;} /* sunlight */

  if (streq(item,"SPACE")) {
    if (gmtoffset==BADLONG) return -1;  /* protect */
    rc=onoff(in,&spacelight); if (rc!=0) return rc;
    check(ID_SPACE, spacelight);
    if (spacelight) check(ID_SUN,  sunlight=TRUE);
     else           check(ID_STAR, starlight=FALSE);
    return 0;} /* space */

  if (streq(item,"STARLIGHT")) {
    if (gmtoffset==BADLONG) return -1;  /* protect */
    rc=onoff(in,&starlight); if (rc!=0) return rc;
    check(ID_STAR, starlight);
    if (starlight) {check(ID_SUN,   sunlight  =TRUE);
                    check(ID_SPACE, spacelight=TRUE);}
    return 0;} /* starlight */

  if (streq(item,"CROSSHAIR")) {
    rc=onoff(in, &crosshair); if (rc!=0) return rc;
    check(ID_CROSSHAIR, crosshair);     /* tell menu */
    return 0;} /* CrossHair */

  if (streq(item,"IDLEDRAW")) {
    USLONG newprio;
    SLONG boost;
    rc=onoff(in,&idledraw); if (rc!=0) return rc;
    check(ID_IDLEDRAW, idledraw);       /* tell menu */
    if (idledraw) {newprio=PRTYC_IDLETIME; boost=1;}
             else {newprio=PRTYC_REGULAR;  boost=0;}
    DosSetPrty(PRTYS_THREAD, newprio, boost, tidg);   /* prio+boost */
    return 0;} /* idledraw */

  if (streq(item,"DESKTOP")) {
    USHORT way;
    rc=onoff(in,&desktop); if (rc!=0) return rc;
    if (desktop) way=UM_TODESK; else way=UM_FROMDESK;
    WinPostMsg(hwndClient, way, NULL, NULL); /* Asynch */
    return 0;} /* desktop */

  if (streq(item,"GRIDPOLAR")) {
    rc=onoff(in,&gridarctic); if (rc!=0) return rc;
    check(ID_GRIDARCTIC,gridarctic); return 0;}
  if (streq(item,"GRIDTROPIC")) {
    rc=onoff(in,&gridtropic); if (rc!=0) return rc;
    check(ID_GRIDTROPIC,gridtropic); return 0;}
  if (streq(item,"GRIDLON")) {
    getword(in,word); if (junk(in)) return 4; lval=getlong(word);
    if (lval==0 || lval==10 || lval==15 || lval==30) { /* is good */
      gridlon10=FALSE; gridlon15=FALSE; gridlon30=FALSE;
      if      (lval==10) gridlon10=TRUE;
      else if (lval==15) gridlon15=TRUE;
      else if (lval==30) gridlon30=TRUE;
      check(ID_GRIDLON10, gridlon10); check(ID_GRIDLON15, gridlon15);
      check(ID_GRIDLON30, gridlon30);
      return 0;}
    errbox("'%s' is not valid for SET %s", word, item);
    return 3;}
  if (streq(item,"GRIDLAT")) {
    getword(in,word); if (junk(in)) return 4; lval=getlong(word);
    if (lval==0 || lval==10 || lval==15 || lval==30 || lval==90) { /* is good */
      gridlat10=FALSE; gridlat15=FALSE; gridlat30=FALSE; grideq=FALSE;
      if      (lval==10) gridlat10=TRUE;
      else if (lval==15) gridlat15=TRUE;
      else if (lval==30) gridlat30=TRUE;
      else if (lval==90) grideq   =TRUE;
      check(ID_GRIDLAT10, gridlat10); check(ID_GRIDLAT15, gridlat15);
      check(ID_GRIDLAT30, gridlat30); check(ID_GRIDEQ   , grideq);
      return 0;}
    errbox("'%s' is not valid for SET %s", word, item);
    return 3;}

  if (streq(item,"GRIDCOLOR")  || streq(item,"GRIDCOLOUR")
   || streq(item,"BACKCOLOR")  || streq(item,"BACKCOLOUR")
   || streq(item,"LANDCOLOR")  || streq(item,"LANDCOLOUR")
   || streq(item,"WATERCOLOR") || streq(item,"WATERCOLOUR")
   || streq(item,"DRAWCOLOR")  || streq(item,"DRAWCOLOUR")
   || streq(item,"TRACKCOLOR") || streq(item,"TRACKCOLOUR")) {
    int cols;                           /* Colours allowed */
    if (item[0]=='B') cols=16;          /* Background */
    else if (item[0]=='L') cols=9;      /* Land */
    else if (item[0]=='W') cols=9;      /* Water */
    else cols=15;                       /* Draw, Track, or Grid */
    getword(in,word); if (junk(in)) return 4; lval=getcol(word, cols);
    if (lval==BADLONG) {
      errbox("'%s' is not valid for SET %s", word, item);
      return 3;}
    if       (item[0]=='G') newgrid(lval);
     else if (item[0]=='B') newback(lval);
     else if (item[0]=='L') newland(lval);
     else if (item[0]=='W') newsea(lval);
     else if (item[0]=='T') trackcoli=lval;
     else                   drawcoli=lval;
    return 0;}

  if (streq(item,"SHADING")) {
    rc=onoff(in,&threed); if (rc!=0) return rc;
    check(ID_3D, threed); return 0;}

  if (streq(item,"REFRESH")) {
    getword(in,word); if (junk(in)) return 4;
    lval=getlong(word);
    if (lval==BADLONG || lval<0 || lval>86400) {
      errbox("'%s' is not a valid time for SET %s", word, item);
      return 3;}
    newref(lval);
    return 0;} /* refresh */

  if (streq(item,"FONT")) {
    struct logfont *plf;                /* work */
    long i;
    getword(in,word);                   /* get fontname */
    if (junk(in)) return 4;
    if (word[0]=='\0') {curfont=0; return 0;}  /* to test default */
    plf=fontbase;                       /* -> start of chain */
    for (i=1; i<=fonts; i++) {
      if (streq(plf->fname,word)) {     /* found it */
        curfont=i; return 0;}
      plf=plf->next;
      }
    errbox("Font '%s' not found for SET %s", word, item);
    return 1;} /* font */

  if (streq(item,"TITLE")) {
    static SWCNTRL swc;            /* static so all nulls */
    static HSWITCH hswitch=NULL;   /* handle */
    unsigned int flagw;
    in[MAXTITLE]='\0';             /* truncate in case too long */
    strcpy(title, in);             /* copy */
    /* Set Frame title */
    flagw=WinSetWindowText(hwndFrame, title);
    if (!flagw && diag) printf("Bad WSetWText\n");
    /* Set task list entry */
    swc.hwnd=hwndFrame; strcpy(swc.szSwtitle, title);
    if (hswitch) WinRemoveSwitchEntry(hswitch);
    hswitch=WinAddSwitchEntry(&swc);
    if (!hswitch && diag) {printf("Bad add switch\n");
      /** lrc=(signed)WinGetLastError(hab);
      printf("Error '%08x'x trying to set switch", lrc); **/
      }
    return 0;} /* title */

  if (streq(item,"TEST")) {
    rc=onoff(in,&test); if (rc!=0)return rc;
    return 0;} /* test */

  /* unknown */
  errbox("'%s' is an unknown item for SET\n", item);
  return 1;
  } /* glset */

/* ------------------------------------------------------------------ */
/* GLQUER: Globe QUERY command                                        */
/* ------------------------------------------------------------------ */
int glquer(char inoptions[]) {
  char *pchar;                          /* working */
  char options[MAXCOMM+1];              /* copy of options */
  char qitem[MAXTOK+1];                 /* item to be queried */

  strcpy(options,inoptions);            /* safe working copy */

  getword(options,qitem);               /* get item */
  if (qitem[0]=='\0') {
    errbox("No item given.  Try 'QUERY item [item]...'");
    return 2;}

  for (; qitem[0]!='\0';) {
    strupr(qitem);                      /* save time, and for msgs */
    pchar=qvalue(qitem);                /* pchar is ->char */
    if (pchar==NULL) return 1;          /* error */
    printf("%s %s\n", qitem, pchar);
    getword(options,qitem);             /* get next item */
    } /* do while items */

  return 0;
  } /* glquer */

/* ------------------------------------------------------------------ */
/* QVALUE: Subroutine to return a string value for an item            */
/*         Used by QUERY and EXTRACT                                  */
/* ------------------------------------------------------------------ */
/* Returns NULL if error */
/* Static working buffer */
static char workbuf[30];                /* for conversions, etc. */
char *qvalue(char item[]) {
  char *p;                              /* for temporary results */

  if (streq(item,"SUNLIGHT"))    {p=qflag(sunlight);             return p;}
  if (streq(item,"SPACE"))       {p=qflag(spacelight);           return p;}
  if (streq(item,"STARLIGHT"))   {p=qflag(starlight);            return p;}
  if (streq(item,"SHADING"))     {p=qflag(threed);               return p;}

  if (streq(item,"GRIDPOLAR"))   {p=qflag(gridarctic);           return p;}
  if (streq(item,"GRIDTROPIC"))  {p=qflag(gridtropic);           return p;}
  /* other grid settings below */

  if (streq(item,"DESKTOP"))     {p=qflag(desktop);              return p;}
  if (streq(item,"MARKERS"))     {p=qflag(markers);              return p;}
  if (streq(item,"GRAPHICS"))    {p=qflag(graphics);             return p;}
  if (streq(item,"LABELS"))      {p=qflag(labels);               return p;}
  if (streq(item,"CLOCKS"))      {p=qflag(clocks);               return p;}
  if (streq(item,"CLOCKCIVIL"))  {p=qflag(civiltime);            return p;}
  if (streq(item,"CLOCKDAY"))    {p=qflag(clockday);             return p;}
  if (streq(item,"MARKS"))       {p=_ltoa(marks,workbuf,10);     return p;}
  if (streq(item,"CLOCKMARKS"))  {p=_ltoa(clockmarks,workbuf,10);return p;}
  if (streq(item,"FONTS"))       {p=_ltoa(fonts,workbuf,10);     return p;}
  if (streq(item,"ACTIVE"))      {p=qflag(weactive);             return p;}
  if (streq(item,"DIAMETER"))    {p=_itoa(diameter,workbuf,10);  return p;}
  if (streq(item,"SETNAME"))     return ininame;
  if (streq(item,"VERSION"))     return VERSION;
  if (streq(item,"MARKER"))      {workbuf[0]=curmark; workbuf[1]='\0';
                                 return workbuf;}
  if (streq(item,"REFRESH"))     {p=_ltoa(refresh,workbuf,10);   return p;}
  if (streq(item,"TITLE"))       return title;

  if (streq(item,"DIAG"))        {p=qflag(diag);                 return p;}
  if (streq(item,"DIAGMSG"))     {p=qflag(diagmsg);              return p;}
  if (streq(item,"TEST"))        {p=qflag(test);                 return p;}
  if (streq(item,"ERRORBOX"))    {p=qflag(errorbox);             return p;}
  if (streq(item,"SHOWDRAW"))    {p=qflag(showdraw);             return p;}
  if (streq(item,"IDLEDRAW"))    {p=qflag(idledraw);             return p;}
  if (streq(item,"CROSSHAIR"))   {p=qflag(crosshair);            return p;}

  if (streq(item,"MARGIN"))      {putmill(workbuf, margin);   return workbuf;}
  if (streq(item,"TWILIGHT"))    {putmill(workbuf, twilight); return workbuf;}
  if (streq(item,"GRIDCOLOR"))   {putcol (workbuf, gridcoli); return workbuf;}
  if (streq(item,"GRIDCOLOUR"))  {putcol (workbuf, gridcoli); return workbuf;}
  if (streq(item,"BACKCOLOR"))   {putcol (workbuf, backcoli); return workbuf;}
  if (streq(item,"BACKCOLOUR"))  {putcol (workbuf, backcoli); return workbuf;}
  if (streq(item,"DRAWCOLOR"))   {putcol (workbuf, drawcoli); return workbuf;}
  if (streq(item,"DRAWCOLOUR"))  {putcol (workbuf, drawcoli); return workbuf;}
  if (streq(item,"TRACKCOLOR"))  {putcol (workbuf, trackcoli);return workbuf;}
  if (streq(item,"TRACKCOLOUR")) {putcol (workbuf, trackcoli);return workbuf;}
  if (streq(item,"LANDCOLOR"))   {putcol (workbuf, landcoli); return workbuf;}
  if (streq(item,"LANDCOLOUR"))  {putcol (workbuf, landcoli); return workbuf;}
  if (streq(item,"WATERCOLOR"))  {putcol (workbuf, seacoli);  return workbuf;}
  if (streq(item,"WATERCOLOUR")) {putcol (workbuf, seacoli);  return workbuf;}

  /* ----- Those that need a little extra processing ----- */

  if (streq(item,"SUNLAT"))      {
    long slat1000, slon1000;
    setsun(gmtoffset+summer-useroffset, &slat1000, &slon1000);
    if (slat1000==BADLONG) return "UNKNOWN";
    putmill(workbuf, slat1000);
    return workbuf;}
  if (streq(item,"SUNLON"))      {
    long slat1000, slon1000;
    setsun(gmtoffset+summer-useroffset, &slat1000, &slon1000);
    if (slon1000==BADLONG) return "UNKNOWN";
    putmill(workbuf, slon1000);
    return workbuf;}

  if (streq(item,"VIEWLON")) {
    putmill(workbuf, viewlon);
    return workbuf;}
  if (streq(item,"VIEWLAT")) {
    putmill(workbuf, viewlat);
    return workbuf;}

 if (streq(item,"GRIDLAT")) {int i=0;
    if      (gridlat10) i=10;
    else if (gridlat15) i=15;
    else if (gridlat30) i=30;
    else if (grideq   ) i=90;
    sprintf(workbuf,"%i", i); return workbuf;}
 if (streq(item,"GRIDLON")) {int i=0;
    if      (gridlon10) i=10;
    else if (gridlon15) i=15;
    else if (gridlon30) i=30;
    sprintf(workbuf,"%i", i); return workbuf;}

  if (streq(item,"USEROFFSET"))  {p=_ltoa(useroffset,workbuf,10); return p;}

  if (streq(item,"DAYOFFSET"))   {
    if (summer==BADLONG) return "UNKNOWN";
    p=_ltoa(summer,workbuf,10); return p;}
  if (streq(item,"GMTOFFSET"))   {
    if (gmtoffset==BADLONG) return "UNKNOWN";
    p=_ltoa(gmtoffset,workbuf,10); return p;}
  if (streq(item,"ZONEABBREV"))  {p=zonename; return p;}

  if (streq(item,"FONT")) {
    struct logfont *plf;                /* work */
    long i;
    if (curfont==0) return "";          /* test default */
    plf=fontbase;                       /* -> start of chain */
    for (i=1; i<curfont; i++) plf=plf->next;
    return plf->fname;} /* font */

  if (streq(item,"VERSIONB"))
    #if defined(B16)
    return "16";
    #else
    return "32";
    #endif

  /* unknown */
  errbox("'%s' is an unknown item for QUERY", item);
  return NULL;
  } /* qvalue */


/* ----- QFLAG: Returns pointer to ON or OFF for a flag ----- */
/* (Could be a macro) */
char *qflag(FLAG flag) {if (flag) return "ON"; else return "OFF";}

/* ----- Complain if string contains any junk ----- */
/* returns TRUE if string is non-blank, else false */
int junk(char *string) {
  char *c;                         /* work */
  for (c=string; ; c++) {
    if (*c=='\0') return FALSE;    /* looks good */
    if (*c!=' ')  break;           /* found junk */
    }
  errbox("'%s' junk found on SET %s", string, item);
  return TRUE;
  } /* junk */

/* ----- Get and check a 'set xxx degrees' item ----- */
/* Sets var if OK, else returns non-0 RC and sets var to BADLONG */
/* We have: remaining STRING, var DEG to set, and CHECK    */
/* CHECK follows the GETDEG check convention. */
int setdeg(char *string, long *deg, long check) {
  char word[MAXTOK+1];
  getword(string,word);
  if (junk(string)) return 4;                /* junk on end */
  *deg=getdeg(word, check);
  if (*deg!=BADLONG) return 0;
  errbox("'%s' is not valid degrees for SET %s", word, item);
  return 3;
  } /* setdeg */

/* ----- Process a flag for ON/OFF/INVERT ----- */
/* We have: remaining STRING, flag to set/reset */
/* Sets flag if OK, else returns non-0 RC */
int onoff(char *string, FLAG *flag) {
  char word[MAXTOK+1];
  getword(string,word);
  if (junk(string)) return 4;           /* junk on end */
  if (streq(word,"ON"))     {*flag=TRUE;  return 0;}
  if (streq(word,"OFF"))    {*flag=FALSE; return 0;}
  if (streq(word,"INVERT")) {*flag=(FLAG)!*flag; return 0;}
  errbox("'%s' should be ON, OFF, or INVERT for SET %s", word, item);
  return 3;
  } /* onoff */
