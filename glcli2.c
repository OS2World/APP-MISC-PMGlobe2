/* ------------------------------------------------------------------ */
/* PMGlobe -- a geographical application                              */
/* GLCLI2  -- the main WinProc, shared subroutines                    */
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

#include <time.h>
#define  INCL_GPICONTROL
#define  INCL_GPIPRIMITIVES
#define  INCL_GPIBITMAPS
#define  INCL_DOSPROCESS
#define  INCL_DOSSEMAPHORES
#define  INCL_WINPOINTERS
#define  INCL_WINDIALOGS
#define  INCL_WINBUTTONS
#define  INCL_WINENTRYFIELDS
#define  INCL_WINMENUS
#define  INCL_WINSWITCHLIST
#define  INCL_WINWINDOWMGR
#define  INCL_WININPUT
#define  INCL_WINSHELLDATA
#define  INCL_WINFRAMEMGR
#define  EXCL_WIN   /* we did them ourself */
#include "globe.h"

/* Subroutines */
 void enable(int, FLAG);      /* en(/dis)able a control from ID */
 void check (int, FLAG);      /* (un)check a menu item */
 void newview(long, long);    /* set new view */
 void newback(long);          /* set new background colour */
 void newgrid(long);          /* set new grid colour */
 void newland(long);          /* set new land colour */
 void newsea(long);           /* set new sea colour */
 void newtwi(long);           /* set new twilight degrees */
 void newref(long);           /* set new refresh seconds */
 long slacos(long, long);     /* super interpolated LACOS */
 void set2menu(void);         /* convert settings to menu checks etc. */
 void setmenubar(FLAG);       /* show/hide menu bar */
 void settitlebar(FLAG);      /* show/hide title bar */

 extern void newdraw(void);   /* start a new draw cycle, please */
 extern void timer(int, int); /* start/stop the fast/slow timer */
 /* Options for timer -- must not be changed */
 #define START 1
 #define STOP  0
 #define FAST  1
 #define SLOW  0

/* Windowing globals */
extern int  idref;            /* current checked REF */
extern int  idview;           /* current checked VIEW */
extern int  idcback;          /* current checked background */
extern int  idtwi;            /* current checked twilight */
extern int  idcgrid;          /* current checked grid colour */
extern int  idcland;          /* current checked land colour */
extern int  idcsea;           /* current checked sea  colour */
extern FLAG timeslow;         /* set if slow timer running */
extern FLAG timefast;         /* set if fast timer running */
extern long nexttime;         /* next refresh time, per TIME() */

/* ------------------------------------------------------------------ */
/* SET2STR -- make a settings string from internal flags              */
/* Argument is string, length >= MAXSETLEN                            */
/* ------------------------------------------------------------------ */
/* Format of each word (flags are one character/bit, '0'/'1')         */
/*    1: View Latitude (format as OK for GETMILL)                     */
/*    2: View Longitude (")                                           */
/*    3: View rotation (currently always 0)                           */
/*    4: Presentation flags                                           */
/*       a) Allow MB2                                                 */
/*       b) Desktop                                                   */
/*       c) Idledraw                                                  */
/*       d) Crosshair                                                 */
/*       e) Menubar                                                   */
/*       f) Titlebar                                                  */
/*    5: Grid flags:                                                  */
/*       a) Grid Lat 10                                               */
/*       b) Grid Lat 15                                               */
/*       c) Grid Lat 30                                               */
/*       d) Grid Lon 10                                               */
/*       e) Grid Lon 15                                               */
/*       f) Grid Lon 30                                               */
/*       g) Grid Arctic circles                                       */
/*       h) Grid Tropics                                              */
/*       i) Grid Equator                                              */
/*    6: Lighting flags                                               */
/*       a) Sunlight                                                  */
/*       b) 3-D                                                       */
/*       c) Spacelight                                                */
/*       d) Starlight                                                 */
/*    7: Refresh interval (seconds, 1=immediate)                      */
/*    8: Background colour name                                       */
/*    9: Grid colour name                                             */
/*   10: Twilight degrees (format as OK for getmill)                  */
/*   11: Land colour name                                             */
/*   12: Sea colour name                                              */
/*                                                                    */
void set2str(char *set)
  {
  char clat[10];  char clon[10];   char crot[10];
  char cpflag[7]; char cgflag[10]; char clflag[5];
  char cref[10];  char cback[14];  char cgrid[14];
  char ctwi[10];  char cland[10];  char csea[10];
  /* Set up the words */
  putmill(clat,viewlat); putmill(clon,viewlon);
  strcpy(crot,"+0");
  sprintf(cpflag,"%1i%1i%1i%1i%1i%1i",
    mousepos, desktop, idledraw, crosshair, menubar, titlebar);
  sprintf(cgflag,"%1i%1i%1i%1i%1i%1i%1i%1i%1i",
    gridlat10, gridlat15, gridlat30,
    gridlon10, gridlon15, gridlon30,
    gridarctic, gridtropic, grideq);
  sprintf(clflag,"%1i%1i%1i%1i", sunlight, threed, spacelight, starlight);
  sprintf(cref,"%li", refresh);
  putcol(cback,backcoli); putcol(cgrid,gridcoli);
  putmill(ctwi,twilight);
  putcol(cland,landcoli); putcol(csea,seacoli);

  /* >>> Note: Additions may require update to MAXSETLEN */

  /* And assemble into a single string */
  sprintf(set,"%s %s %s %s %s %s %s %s %s %s %s %s",
     clat, clon, crot, cpflag, cgflag, clflag, cref, cback, cgrid,
     ctwi, cland, csea);
  } /* set2str */

/* ------------------------------------------------------------------ */
/* STR2SET -- set internal flags from a settings string               */
/* Argument is string of settings                                     */
/* ------------------------------------------------------------------ */
/* See SET2STR for format of saved string                             */
/* If anything ain't right, then we just ignore it (i.e., accept the  */
/* current settings) -- except that with flags, we set to FALSE, if   */
/* the wordcharacter is there but is not '1'.                         */
/* These are set atomically to ensure consistency                     */
void str2set(char *input)
  {
  char set[MAXSETLEN+1];      /* local copy */
  char word[MAXTOK+1];
  long lnum;
  if (input[0]=='\0') return; /* save a few cycles if silly */
  strcpy(set,input);          /* make local copy */

  /* From now on, no interrupt/return allowed */
  DosEnterCritSec();
  getword(set,word);  lnum=getmill(word);
  if (lnum!=BADLONG && labs(lnum)<= 90000L) viewlat=lnum;
  getword(set,word);  lnum=getmill(word);
  if (lnum!=BADLONG && labs(lnum)<=180000L) viewlon=lnum;
  getword(set,word);  lnum=getmill(word);
  if (lnum!=BADLONG && labs(lnum)<=180000L) viewrot=lnum;

  getword(set,word); if (strlen(word)>=1 && strlen(word)<=6) {
    if (word[0]=='1')   mousepos=TRUE;  else mousepos=FALSE;
    if (strlen(word)>1) {
      if (word[1]=='1') desktop=TRUE;   else desktop=FALSE;}
    if (strlen(word)>2) {
      if (word[2]=='1') idledraw=TRUE;  else idledraw=FALSE;}
    if (strlen(word)>3) {
      if (word[3]=='1') crosshair=TRUE; else crosshair=FALSE;}
    if (strlen(word)>4) {
      if (word[4]=='1') menubar=TRUE;   else menubar=FALSE;}
    if (strlen(word)>5) {
      if (word[5]=='1') titlebar=TRUE;  else titlebar=FALSE;}
    /* if (diag) printf("menubar read %d\n", menubar); */
    if (diag) printf("titlebar read %d\n", titlebar);
    }
  getword(set,word);
  if (strlen(word)==6) {  /* old form -- remove 1992/06 */
    if (word[0]=='1') gridlat10=TRUE; else gridlat10=FALSE;
    if (word[1]=='1') gridlat30=TRUE; else gridlat30=FALSE;
    if (word[2]=='1') gridlon10=TRUE; else gridlon10=FALSE;
    if (word[3]=='1') gridlon30=TRUE; else gridlon30=FALSE;
    if (word[4]=='1') gridarctic=TRUE; else gridarctic=FALSE;
    if (word[5]=='1') gridtropic=TRUE; else gridtropic=FALSE;
    /* Fixup .. old form allowed both 10/30 to be on */
    if (gridlat10) gridlat30=FALSE; if (gridlon10) gridlon30=FALSE;
    }
  if (strlen(word)>=8) {  /* new form */
    if (word[0]=='1') gridlat10=TRUE; else gridlat10=FALSE;
    if (word[1]=='1') gridlat15=TRUE; else gridlat15=FALSE;
    if (word[2]=='1') gridlat30=TRUE; else gridlat30=FALSE;
    if (word[3]=='1') gridlon10=TRUE; else gridlon10=FALSE;
    if (word[4]=='1') gridlon15=TRUE; else gridlon15=FALSE;
    if (word[5]=='1') gridlon30=TRUE; else gridlon30=FALSE;
    if (word[6]=='1') gridarctic=TRUE; else gridarctic=FALSE;
    if (word[7]=='1') gridtropic=TRUE; else gridtropic=FALSE;
    /* Next is safe even if only 8-long word */
    if (word[8]=='1') grideq    =TRUE; else grideq    =FALSE;
    /* note we could check and enforce mutually exclusive 10/15/30 */
    }
  getword(set,word); if (strlen(word)>=2 && strlen(word)<=4) {
    if (word[0]=='1') sunlight=TRUE; else sunlight=FALSE;
    if (word[1]=='1') threed  =TRUE; else threed  =FALSE;
    /* Next one is OK even if length was 2 */
    if (word[2]=='1') spacelight=TRUE; else spacelight=FALSE;
    if (strlen(word)>=4 && word[3]=='1') starlight=TRUE; else starlight=FALSE;

    /* Now some implied settings */
    if (starlight) spacelight=TRUE; if (spacelight) sunlight=TRUE;
    if (!sunlight) {spacelight=FALSE; starlight=FALSE;}
    }
  getword(set,word);  lnum=getlong(word);    /* up to 24 hours */
  if (lnum!=BADLONG && lnum>=0 && lnum<=24L*3600L) refresh=lnum;
  getword(set,word);  lnum=getlong(word);
  if (lnum!=BADLONG && lnum>=0 && lnum<=100) {
    /* Old style -- just a "white" number */
    if      (lnum<10) backcoli=CLR_BLACK;
    else if (lnum<70) backcoli=CLR_DARKGRAY;
    else if (lnum<90) backcoli=CLR_PALEGRAY;
    else              backcoli=CLR_WHITE;
    }
   else {lnum=getcol(word, 16); if (lnum!=BADLONG) backcoli=lnum;}
  getword(set,word);
  lnum=getlong(&word[1]);
  if (lnum!=BADLONG && lnum>=0 && lnum<=100) {
    /* old style */
    if      (streq(word,"R100")) gridcoli=CLR_RED;
    else if (streq(word,"R50" )) gridcoli=CLR_DARKRED;
    else if (streq(word,"C100")) gridcoli=CLR_CYAN;
    else if (streq(word,"C50" )) gridcoli=CLR_DARKCYAN;
    else if (streq(word,"Y100")) gridcoli=CLR_YELLOW;
    else if (streq(word,"Y50" )) gridcoli=CLR_BROWN;
    else if (streq(word,"M100")) gridcoli=CLR_PINK;
    else if (streq(word,"M50" )) gridcoli=CLR_DARKPINK;
    } /* OK lnum */
   else {lnum=getcol(word, 15); if (lnum!=BADLONG) gridcoli=lnum;}
  getword(set,word);  lnum=getmill(word);
  if (lnum!=BADLONG && labs(lnum)<= 90000L) twilight=lnum;
  getword(set,word); lnum=getcol(word, 9); if (lnum!=BADLONG) landcoli=lnum;
  getword(set,word); lnum=getcol(word, 9); if (lnum!=BADLONG) seacoli=lnum;
  DosExitCritSec();
  } /* str2set */

/* ------------------------------------------------------------------ */
/* SAVESET -- save settings in user profile                           */
/* ------------------------------------------------------------------ */
void saveset(void)
  {
  char set[MAXSETLEN+1];
  set2str(set);
  if (PrfWriteProfileString(HINI_USERPROFILE, ininame, KEYSET, set)) {
    /* Can't do much if this fails (message to user notalotofhelp) */
    if (diag) printf("Set string '%s' saved\n", set);
    strcpy(initset,set);      /* Is now safe setting */
    }
  } /* saveset */

/* ------------------------------------------------------------------ */
/* STR2POS -- set internal position (window settings) from string     */
/* Argument is string of settings                                     */
/* ------------------------------------------------------------------ */
void str2pos(char *input) {
  char pos[MAXPOSLEN+1];      /* local copy */
  char word[MAXTOK+1];
  long lnum;
  strcpy(pos,input);          /* make local copy */
  getword(pos,word);  lnum=getlong(word);
  if (lnum!=BADLONG && labs(lnum)<32000L) initswp.x=(int)lnum;
  getword(pos,word);  lnum=getlong(word);
  if (lnum!=BADLONG && labs(lnum)<32000L) initswp.y=(int)lnum;
  getword(pos,word);  lnum=getlong(word);
  if (lnum!=BADLONG && labs(lnum)<32000L) initswp.cx=(int)lnum;
  getword(pos,word);  lnum=getlong(word);
  if (lnum!=BADLONG && labs(lnum)<32000L) initswp.cy=(int)lnum;
  getword(pos,word);  lnum=getlong(word);
  if (lnum!=BADLONG && labs(lnum)<32000L) dcposx=(int)lnum;
  getword(pos,word);  lnum=getlong(word);
  if (lnum!=BADLONG && labs(lnum)<32000L) dcposy=(int)lnum;
  getword(pos,word);  lnum=getlong(word);
  if (lnum!=BADLONG && labs(lnum)<32000L) cmposx=(int)lnum;
  getword(pos,word);  lnum=getlong(word);
  if (lnum!=BADLONG && labs(lnum)<32000L) cmposy=(int)lnum;
  } /* str2pos */

/* ------------------------------------------------------------------ */
/* SAVEPOS -- save position and size in user profile                  */
/* ------------------------------------------------------------------ */
/* (Unless maximized or minimized...) */
void savepos(void) {
  char pos[MAXPOSLEN+1];
  char win[MAXWINLEN+1];
  SWP swp, swpdc, swpcm;
  USLONG flags;                         /* copy from swp option flags */
  WinQueryWindowPos(hwndFrame, &swp);   /* query frame */
  #if defined(B16)
  flags=swp.fs;                         /* copy of options */
  #else
  flags=swp.fl;
  #endif                                /* copy of options */
  /* First the window state */
  if       (flags & (unsigned)SWP_MINIMIZE) strcpy(win,"MIN");
   else if (flags & (unsigned)SWP_MAXIMIZE) strcpy(win,"MAX");
   else strcpy(win,"RESTORE");
  if (!streq(initwin,win)) {            /* it changed */
    if (PrfWriteProfileString(HINI_USERPROFILE, ininame, KEYWIN, win)) {
      /* Can't do much if this fails (message to user notalotofhelp) */
      if (diag) printf("Window State string '%s' saved\n", win);
      /* if (diag) printf("Strings were '%s' and '%s'\n", initwin, win); */
      win[0]='\0';                      /* B&B */
      }
    strcpy(initwin,win);                /* remember */
    } /* changed */
  if ((flags & (unsigned)SWP_MINIMIZE) || (flags & (unsigned)SWP_MAXIMIZE)) {
    if (diag) printf("Max/min: position not saved\n");
    return;}

  WinQueryWindowPos(hwndDist,  &swpdc); /* .. and distcalc */
  WinQueryWindowPos(hwndComm,  &swpcm); /* .. and command */
  sprintf(pos,"%i %i %i %i %i %i %i %i",
    swp.x, swp.y, swp.cx, swp.cy, swpdc.x, swpdc.y, swpcm.x, swpcm.y);
  /* >>> Note: Additions may require update to MAXPOSLEN */

  if (!streq(initpos,pos)) {            /* it changed */
    if (PrfWriteProfileString(HINI_USERPROFILE, ininame, KEYPOS, pos)) {
      /* Can't do much if this fails (message to user notalotofhelp) */
      if (diag) printf("Position string '%s' saved\n", pos);
      pos[0]='\0';                      /* B&B */
      }
    strcpy(initpos,pos);                /* remember */
    } /* changed */
  } /* savepos */


/* ------------------------------------------------------------------ */
/* ENABLE -- enable or disable a control from ID                      */
/* ------------------------------------------------------------------ */
/* Call with ID of item to enable/disable, and TRUE/FALSE             */
void enable(int id, FLAG flag)
  {
  SHORT mask;                 /* work */
  if (flag) mask=~MIA_DISABLED;
       else mask= MIA_DISABLED;
  WinSendMsg(hwndMenu, MM_SETITEMATTR, MPFROM2SHORT(id, TRUE),
             MPFROM2SHORT(MIA_DISABLED, mask)); /* Must be SEND */
  WinSendMsg(hwndPop,  MM_SETITEMATTR, MPFROM2SHORT(id, TRUE),
             MPFROM2SHORT(MIA_DISABLED, mask)); /* Must be SEND */
  } /* enable */

/* ------------------------------------------------------------------ */
/* CHECK -- check or uncheck a menu item                              */
/* ------------------------------------------------------------------ */
/* Call with ID of menu item to check/uncheck, and TRUE/FALSE         */
void check(int id, FLAG flag)
  {
  SHORT mask;                 /* work */
  if (flag) mask= MIA_CHECKED;
       else mask=~MIA_CHECKED;
  WinSendMsg(hwndMenu, MM_SETITEMATTR, MPFROM2SHORT(id, TRUE),
             MPFROM2SHORT(MIA_CHECKED, mask)); /* Must be SEND */
  WinSendMsg(hwndPop,  MM_SETITEMATTR, MPFROM2SHORT(id, TRUE),
             MPFROM2SHORT(MIA_CHECKED, mask)); /* Must be SEND */
  } /* check */

/* ------------------------------------------------------------------ */
/* NEWREF -- set a new refresh delay                                  */
/* ------------------------------------------------------------------ */
void newref(long seconds) {
  refresh=seconds;                 /* seconds */
  if (idref) {
    check(idref, FALSE);           /* uncheck old */
    idref=0;}
  if      (seconds<= 1L    ) idref=ID_REF0 ;
  else if (seconds== 1L*60L) idref=ID_REF1 ;
  else if (seconds== 5L*60L) idref=ID_REF5 ;
  else if (seconds==10L*60L) idref=ID_REF10;
  else if (seconds==15L*60L) idref=ID_REF15;
  else if (seconds==20L*60L) idref=ID_REF20;
  else if (seconds==30L*60L) idref=ID_REF30;
  else if (seconds==60L*60L) idref=ID_REF60;
  if (idref) check(idref, TRUE);   /* check new */
  if (refresh<=0) refresh=1L;      /* immediate transformed to 1 sec */
  if (timeslow) {                  /* timer already running, so.. */
    long now;
    time(&now);                    /* .. update next refresh time */
    nexttime=(now/refresh)*refresh + refresh;  /* Next boundary */
    if (diag) printf("Refresh set for +%lis\n", nexttime-now);
    timer(STOP, SLOW);             /* stop and start timer for best */
    timer(START,SLOW);             /*   delay granularity */
    }
  } /* newref */

/* ------------------------------------------------------------------ */
/* NEWLAND -- set a new land colour                                   */
/* ------------------------------------------------------------------ */
void newland(long col) {
  /* Uncheck any current standard colour, then check any valid.  */
  /* (We always do this, for safety)                             */
  if (idcland) {check(idcland, FALSE); idcland=0;}/* uncheck old */
  switch((int)col) {
    case CLR_RED:        idcland=ID_CLRED;       break;
    case CLR_GREEN:      idcland=ID_CLGREEN;     break;
    case CLR_BLUE:       idcland=ID_CLBLUE;      break;
    case CLR_CYAN:       idcland=ID_CLCYAN;      break;
    case CLR_PINK:       idcland=ID_CLPINK;      break;
    case CLR_YELLOW:     idcland=ID_CLYELLOW;    break;
    case CLR_WHITE:      idcland=ID_CLWHITE;     break;
    case CLR_DARKGRAY:   idcland=ID_CLGRAY;      break;
    case CLR_BLACK:      idcland=ID_CLBLACK;     break;
    } /* switch */
  landcoli=col;
  if (idcland) check(idcland,TRUE);
  } /* newland */

/* ------------------------------------------------------------------ */
/* NEWSEA  -- set a new sea colour                                    */
/* ------------------------------------------------------------------ */
void newsea(long col) {
  /* Uncheck any current standard colour, then check any valid.  */
  /* (We always do this, for safety)                             */
  if (idcsea) {check(idcsea, FALSE); idcsea=0;}/* uncheck old */
  switch((int)col) {
    case CLR_RED:        idcsea=ID_CSRED;       break;
    case CLR_GREEN:      idcsea=ID_CSGREEN;     break;
    case CLR_BLUE:       idcsea=ID_CSBLUE;      break;
    case CLR_CYAN:       idcsea=ID_CSCYAN;      break;
    case CLR_PINK:       idcsea=ID_CSPINK;      break;
    case CLR_YELLOW:     idcsea=ID_CSYELLOW;    break;
    case CLR_WHITE:      idcsea=ID_CSWHITE;     break;
    case CLR_DARKGRAY:   idcsea=ID_CSGRAY;      break;
    case CLR_BLACK:      idcsea=ID_CSBLACK;     break;
    } /* switch */
  seacoli=col;
  if (idcsea) check(idcsea,TRUE);
  } /* newsea */

/* ------------------------------------------------------------------ */
/* NEWGRID -- set a new grid colour                                   */
/* ------------------------------------------------------------------ */
void newgrid(long col) {
  /* Uncheck any current standard colour, then check any valid.  */
  /* (We always do this, for safety)                             */
  if (idcgrid) {check(idcgrid, FALSE); idcgrid=0;}/* uncheck old */
  switch((int)col) {
    case CLR_DARKRED:    idcgrid=ID_CGDARKRED;   break;
    case CLR_RED:        idcgrid=ID_CGRED;       break;
    case CLR_DARKGREEN:  idcgrid=ID_CGDARKGREEN; break;
    case CLR_GREEN:      idcgrid=ID_CGGREEN;     break;
    case CLR_DARKBLUE:   idcgrid=ID_CGDARKBLUE;  break;
    case CLR_BLUE:       idcgrid=ID_CGBLUE;      break;
    case CLR_DARKCYAN:   idcgrid=ID_CGDARKCYAN;  break;
    case CLR_CYAN:       idcgrid=ID_CGCYAN;      break;
    case CLR_DARKPINK:   idcgrid=ID_CGPURPLE;    break;
    case CLR_PINK:       idcgrid=ID_CGPINK;      break;
    case CLR_BROWN:      idcgrid=ID_CGBROWN;     break;
    case CLR_YELLOW:     idcgrid=ID_CGYELLOW;    break;
    case CLR_WHITE:      idcgrid=ID_CGWHITE;     break;
    case CLR_BLACK:      idcgrid=ID_CGBLACK;     break;
    case CLR_DARKGRAY:   idcgrid=ID_CGGRAY;      break;
    } /* switch */
  gridcoli=col;
  if (idcgrid) check(idcgrid,TRUE);
  } /* newgrid */

/* ------------------------------------------------------------------ */
/* NEWBACK -- set a new background colour                             */
/* ------------------------------------------------------------------ */
void newback(long col) {
  /* Uncheck any current standard colour, then check any valid.  */
  /* (We always do this, for safety and for startup case)        */
  if (idcback) {check(idcback, FALSE); idcback=0;}   /* uncheck old */
  switch((int)col) {
    case CLR_DARKRED:    idcback=ID_CBDARKRED;   break;
    case CLR_RED:        idcback=ID_CBRED;       break;
    case CLR_DARKGREEN:  idcback=ID_CBDARKGREEN; break;
    case CLR_GREEN:      idcback=ID_CBGREEN;     break;
    case CLR_DARKBLUE:   idcback=ID_CBDARKBLUE;  break;
    case CLR_BLUE:       idcback=ID_CBBLUE;      break;
    case CLR_DARKCYAN:   idcback=ID_CBDARKCYAN;  break;
    case CLR_CYAN:       idcback=ID_CBCYAN;      break;
    case CLR_DARKPINK:   idcback=ID_CBPURPLE;    break;
    case CLR_PINK:       idcback=ID_CBPINK;      break;
    case CLR_BROWN:      idcback=ID_CBBROWN;     break;
    case CLR_YELLOW:     idcback=ID_CBYELLOW;    break;
    case CLR_WHITE:      idcback=ID_CBWHITE;     break;
    case CLR_BLACK:      idcback=ID_CBBLACK;     break;
    case CLR_DARKGRAY:   idcback=ID_CBGRAY;      break;
    case CLR_PALEGRAY:   idcback=ID_CBPALEGRAY;  break;
    } /* switch */
  backcoli=col;               /* A PM CLR_xx */
  if (idcback) check(idcback,TRUE);
  } /* newback */

/* ------------------------------------------------------------------ */
/* NEWTWI -- set new twilight degrees                                 */
/* ------------------------------------------------------------------ */
void newtwi(long deg) {
  twilight=deg;
  if (idtwi) {
    check(idtwi, FALSE);           /* uncheck old */
    idtwi=0;}                      /* none selected */
  if      (deg==    0L) idtwi=ID_TWI0  ;
  else if (deg==  833L) idtwi=ID_TWIZ50;
  else if (deg== 5000L) idtwi=ID_TWI5  ;
  else if (deg== 6000L) idtwi=ID_TWI6  ;
  else if (deg==12000L) idtwi=ID_TWI12 ;
  else if (deg==18000L) idtwi=ID_TWI18 ;
  if (idtwi) check(idtwi,TRUE);
  } /* newtwi */

/* ------------------------------------------------------------------ */
/* NEWVIEW -- set a new view                                          */
/* ------------------------------------------------------------------ */
void newview(long lat, long lon) {
  if (lon!=viewlon || lat!=viewlat) {
    DosEnterCritSec();             /* ensure atomic */
    viewlon=lon; viewlat=lat;
    DosExitCritSec();
    newdraw();
    }

  /* Uncheck any current standard view, then check any valid.    */
  /* (We always do this, for safety and for startup case)        */
  if (idview) {check(idview, FALSE); idview=0;}   /* uncheck old */
  if (lon==0L)  {
    if      (lat==     0L) idview=ID_POS0;
    else if (lat==+90000L) idview=ID_POSNP;
    else if (lat==-90000L) idview=ID_POSSP;}
  if (lat==0L)  {
    if      (lon==-90000L) idview=ID_POS90W;
    else if (lon==+90000L) idview=ID_POS90E;
    else if (lon==180000L) idview=ID_POS180;}
  if (idview) check(idview,TRUE);
  } /* newview */

/* ------------------------------------------------------------------ */
/* DISTCALC -- calculate distances between two points                 */
/* ------------------------------------------------------------------ */
/* Takes two lat/lon pairs; sets two results                          */
void distcalc(long lat0, long lon0, long lat1, long lon1,
              long *km, long *miles) {
  extern long trueradius /*, drawradius*/;   /* R32's from glober */
  long x0, y0, z0;                           /* work */
  long x1, y1, z1;
  long x,  y,  z;
  long angle, dist;
  int rc;

  /* We need the X, Y, Zs in screen coordinates, with high      */
  /* resolution.  LL2PEL can provide this...                    */
  rc=ll2pel(lon0, lat0, TRUE, &x0, &y0, &z0);
  if (rc!=0) printf("*** Bad RC from LL2PEL\n");  /* yech */
  rc=ll2pel(lon1, lat1, TRUE, &x1, &y1, &z1);
  if (rc!=0) printf("*** Bad RC from LL2PEL\n");  /* yech */
  /* Calculate separations */
  x=x0-x1; y=y0-y1; z=z0-z1;
  /* dist=lsqrt(x*x+y*y+z*z);         This would be chord length */
  /* Dist is the chord length in pels*32, i.e., max=MAXRADIUS*64 */
  dist=(x*x+y*y+z*z)/4;                 /* (chord length/2)**2     */
  /* For best accuracy when using LACOS, we use this directly    */
  /* (and so get 90-halfangle) when DIST is small, and calculate */
  /* the centre-earth to centre-chord distance in other cases    */
  /* (which gives us the halfangle directly).                    */
  if (dist<(trueradius*trueradius/2L)) {/* circa < (R*0.707)**2  */
    dist=lsqrt(dist);                   /* chord/2 */
    /* 90-halfangle of chord */
    angle=90000L-slacos(dist,trueradius);
    }
   else { /* wider angle */
    /* centre-earth to centre-chord.. */
    dist=lsqrt(trueradius*trueradius-dist);
    angle=slacos(dist,trueradius);      /* half-angle of chord */
    }

  /* Now we have the angle we can calculate the distance.  */
  /* Circumference at equator:   24901.83 miles            */
  /* Circumference at poles:     24818.51 miles            */
  /* Mean Circumference:         24860.17 miles            */
  *miles=(2486L*angle+9000L)/18000L;    /* careful of overflow */
  /* 1.609344 km in a mile; mean circumference: 40008.57km */
  /* However, we use the 40000.00 figure, to match km      */
  /* definition of 10000 km from pole to equator.          */
  *km=(4000L*angle+9000L)/18000L;       /* careful, again */
  /* km=miles*16093L/10000L; */         /* alternative */
  return;} /* distcalc */

/* ------------------------------------------------------------------ */
/* SLACOS  -- super LACOS (linear interpolated result)                */
/* ------------------------------------------------------------------ */
/* OK up to a denom of 160000+, i.e., TRUERADIUS (MAXRADIUS*32)       */
long slacos(long num, long denom) {
  long alow, ahigh;
  long prop;
  alow =lacos(num*1000L/denom);         /* lower bound argument */
  ahigh=lacos((num*1000L/denom)+1);     /* upper ditto */
  prop=(num*100000L/denom)%100L;        /* proportion of high part, % */
  return (alow*(100L-prop)+ahigh*prop)/100L;
  } /* slacos */

/* ------------------------------------------------------------------ */
/* SET2MENU -- set menu checkmarks, etc., from settings               */
/* ------------------------------------------------------------------ */
void set2menu(void) {
  newview(viewlat, viewlon);            /* whichever view */
  check(ID_SUN   , sunlight);           /* sunlighting */
  check(ID_SPACE , spacelight);         /* spacelighting */
  check(ID_STAR  , starlight);          /* starlighting */
  check(ID_3D,     threed  );           /* shading */
  check(ID_POSMB2, mousepos);           /* allow MB2 */
  check(ID_SHOWDC, showdist);           /* distance calculator */
  check(ID_DESKTOP,desktop );           /* desktop 'mode' */
  check(ID_IDLEDRAW, idledraw);         /* IDLE priority for draw */
  check(ID_CROSSHAIR, crosshair);       /* crosshair cursor */
  check(ID_MENUBAR, menubar);           /* menu bar showing */
  check(ID_TITLEBAR, titlebar);         /* title bar showing */
  setmenubar(menubar);                  /* menu bar visible */
  settitlebar(titlebar);                /* title bar visible */
  newref(refresh);                      /* interval */
  newtwi(twilight);                     /* twilight */
  newback(backcoli);                    /* background colour */
  newgrid(gridcoli);                    /* grid colour */
  newland(landcoli);                    /* land colour */
  newsea(seacoli);                      /* sea colour */
  /* grid settings */
  check(ID_GRIDLON10, gridlon10);
  check(ID_GRIDLON15, gridlon15);
  check(ID_GRIDLON30, gridlon30);
  check(ID_GRIDLAT10, gridlat10);
  check(ID_GRIDLAT15, gridlat15);
  check(ID_GRIDLAT30, gridlat30);
  check(ID_GRIDEQ,    grideq   );
  check(ID_GRIDARCTIC,gridarctic);
  check(ID_GRIDTROPIC,gridtropic);
  } /* set2menu */

/* ------------------------------------------------------------------ */
/* SETMENUBAR -- turn menu bar on or off                              */
/* ------------------------------------------------------------------ */
/* Changes state of menu bar to match MENUBAR flag */
/* [Only this routine should change MENUBAR flag] */
void setmenubar(FLAG new) {
  HWND mparent;
  /* Always do it, e.g., for start-up case */
  if (new) mparent=hwndMParent;         /* turn on */
   else    mparent=HWND_OBJECT;         /* turn off */
  WinSetParent(hwndMenu, mparent, FALSE);
  /* if (test) WinSetParent(hwndTitle, mparent, FALSE); */

  WinSendMsg(hwndFrame, WM_UPDATEFRAME, MPFROMLONG(FCF_MENU), NULL);
  menubar=new;                          /* assume worked */
  check(ID_MENUBAR, menubar);           /* tick to match */
  } /* setmenubar */

/* ------------------------------------------------------------------ */
/* SETTITLEBAR -- turn title bar on or off                            */
/* ------------------------------------------------------------------ */
/* Changes state of title bar to match TITLEBAR flag */
/* [Only this routine should change TITLEBAR flag] */
void settitlebar(FLAG new) {
  unsigned long flags=FCF_MINMAX | FCF_TITLEBAR | FCF_SYSMENU;
  FLAG there;
  if (WinWindowFromID(hwndFrame, FID_SYSMENU)==NULLHANDLE) there=FALSE;
   else there=TRUE;
  /* if (diag) printf("*** In SetTB; there, new: %d %d\n", there, new); */
  if (there==new) return;               /* nowt to do */
  if (new) {                            /* show titlebar, etc. */
    FRAMECDATA fcd;
    char tcomm[MAXTITLE+10+1];          /* work */
    memset(&fcd, 0, sizeof(FRAMECDATA));
    fcd.cb=sizeof(FRAMECDATA);
    fcd.flCreateFlags=flags;
    fcd.idResources=ID_FRAMERC;
    WinCreateFrameControls(hwndFrame, &fcd, "mfc");
    /* That should have set title, but didn't */
    sprintf(tcomm,"set title %s", title);
    gldo(tcomm);                        /* set current title */
    } /* show */
   else {                               /* hide titlebar, etc. */
    WinDestroyWindow(WinWindowFromID(hwndFrame, FID_SYSMENU));
    WinDestroyWindow(WinWindowFromID(hwndFrame, FID_TITLEBAR));
    WinDestroyWindow(WinWindowFromID(hwndFrame, FID_MINMAX));
    } /* hide */

  WinSendMsg(hwndFrame, WM_UPDATEFRAME, MPFROMLONG(flags), MPVOID);
  titlebar=new;                         /* assume worked */
  check(ID_TITLEBAR, titlebar);         /* tick to match */
  } /* settitlebar */

