/* ------------------------------------------------------------------ */
/* PMGlobe -- a geographical application                              */
/* GLCLIENT -- the main WinProc                                       */
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

#include <process.h>
#include <time.h>
#define  INCL_GPIPRIMITIVES
#define  INCL_DOSPROCESS
#define  INCL_DOSSEMAPHORES
#define  INCL_WINPOINTERS
#define  INCL_WINDIALOGS
#define  INCL_WINBUTTONS
#define  INCL_WINENTRYFIELDS
#define  INCL_WINMENUS
#define  INCL_WINSWITCHLIST
#define  INCL_WININPUT
#define  INCL_WINSHELLDATA
#define  INCL_WINWINDOWMGR
#define  INCL_WINFRAMEMGR
#define  INCL_WINMESSAGEMGR
#define  INCL_WINSTDFILE
#define  EXCL_WIN   /* we did them ourself */
#include "globe.h"


/* Subroutines */
 extern  void ll2str(long, long, char *, char *); /* formatter */
 extern  void glpaint(void);                      /* the globe repainter */
 extern  MRESULT EXPENTRY gltimez(HWND, USLONG, MPARAM, MPARAM);
 MRESULT EXPENTRY setfileicon(HWND, USLONG, MPARAM, MPARAM);

 void timer(FLAG, FLAG);        /* start/stop the fast/slow timer */
 /* Options for timer -- must not be changed */
 #define START 1
 #define STOP  0
 #define FAST  1
 #define SLOW  0
 void onclient(void);         /* mouse is going over client */
 void offclient(void);        /* mouse is leaving client */
 void onglobe(void);          /* mouse is going over globe */
 void offglobe(void);         /* mouse is leaving globe */
 void distvis(FLAG);          /* make DISTCALC visible or not */
 void showll(long, long);     /* show mouse lat/lon, if changed */
 void todesk(void);           /* go to desktop */
 void fromdesk(void);         /* leave desktop */
 void deskcheck(void);        /* ensure on desktop */
 void makefull(void);         /* make full-screen */
 void makequart(void);        /* make quarter-screen */
 void newdraw(void);          /* start a new draw cycle, please */

 extern void enable(int, FLAG);    /* en(/dis)able a control from ID */
 extern void check(int,  FLAG);    /* (un)check a menu item */
 extern void newview(long, long);  /* set new view */
 extern void newback(long);        /* set new background colour */
 extern void newgrid(long);        /* set new grid colour */
 extern void newland(long);        /* set new land colour */
 extern void newsea(long);         /* set new sea colour */
 extern void newtwi(long);         /* set new twilight degrees */
 extern void newref(long);         /* set new refresh seconds */
 extern void distcalc(long, long, long, long, long *, long *); /* distance */
 extern void set2menu(void);       /* convert settings to menu checks etc. */
 extern void set2str(char *);      /* convert settings to a string */
 extern void str2set(char *);      /* convert string to settings */
 extern void str2pos(char *);      /* convert string to window positions */
 extern void saveset(void);        /* save settings in .INI */
 extern void savepos(void);        /* save position in .INI */
 extern void setmenubar(FLAG);     /* show/hide menu bar */
 extern void settitlebar(FLAG);    /* show/hide title bar */


/* Windowing globals */
int  idref;                        /* current checked REF */
int  idview;                       /* current checked VIEW */
int  idcback;                      /* current checked background */
int  idtwi;                        /* current checked twilight */
int  idcgrid;                      /* current checked grid colour */
int  idcland;                      /* current checked land colour */
int  idcsea;                       /* current checked sea  colour */

int  timeslow=FALSE;               /* set if slow timer running */
int  timefast=FALSE;               /* set if fast timer running */
long nexttime=0;                   /* next refresh time, per TIME() */

/* Settings saver globals */
char initset[MAXSETLEN+1];         /* saves startup conditions */
char defset[MAXSETLEN+1];          /* default startup conditions */
char initpos[MAXPOSLEN+1];         /* saves startup posstring */
char initwin[MAXWINLEN+1];         /* saves startup WinState */
SWP  initswp;                      /* initial frame size/pos */
int  dcposx, dcposy;               /* initial distcalc box position */
int  cmposx, cmposy;               /* initial command box position */

/* Static windowing globals */
/* Next sets how often we check the timer, in ms (max 64K-1) */
/* (Also defines minimum delay, for 'immediate' refresh) */
#define DELAY        50U           /* slow timer, mseconds per second */
#define DELAYFAST   500U           /* fast timer, fixed delay */

/* local/housekeeping shared variables */
static SIZEL size0={0,0};          /* useful */
static HWND  hwndDesk=NULL;        /* desktop handle */
static int   overglobe=FALSE;      /* set if mouse over globe */
static int   overclient=FALSE;     /* set if mouse over client */
static int   windowset=FALSE;      /* set if window ready to go */
static int   startup=FALSE;        /* set if running startup macro */
static int   canhalt=FALSE;        /* set if Halt Command enabled */
static SWP   defswp;               /* default (PM-set) frame size/pos */
static HPOINTER hptrGlobe;         /* our pointer handle */
static HPOINTER hptrSystem;        /* system pointer handle */

/* distance calculator variables */
static int   dcpoints=0;           /* number points displayed */
static int   dcfixed=FALSE;        /* TRUE if first point fixed */
static long  dclat[2], dclon[2];   /* recorded points Lat/Long */
static long  dckm, dcmiles;        /* last measurement */
static long  dctotkm=0;            /* track distance accumulators.. */
static long  dctotmiles=0;         /* .. */

static long oldlat=BADLONG;        /* optimization for lat/lon update */
static long oldlon=BADLONG;

static ULONG posted;               /* for V2 semaphores */

/*====================================================================*/
/* The Client Window Procedure                                        */
/*====================================================================*/
MRESULT EXPENTRY _loadds glclient(
  HWND hwnd, USLONG msg, MPARAM mp1, MPARAM mp2)
 {
 int /* source,*/ id;              /* message parts */
 MRESULT mres;                     /* message result temporary */
 char comm[MAXCOMM+1];             /* work for commands */

 if (diagmsg) printf("Msg: %04x\n", msg);

 switch(msg)
  {
  case WM_TIMER: {
    /* We assume (SHORT1FROMMP(mp1)==TIMERID) as only one timer */
    long now, lat, lon;
    POINTL pp;
    SWP f, c;
    int rc;

    if (timeslow) {                /* slow timer running */
      time(&now);                  /* what time is it? */
      if (now>nexttime) {          /* time to do a new draw */
        timer(STOP,SLOW);          /* we don't need the slow timer */
        newdraw();                 /* and do a redraw */
        } /* end of pause */
      } /* refresh timer */

    if (timefast) {
      HWND hwndtemp;
      if (!overclient) {   /* running for Desktop */
        /* ensure we are at the bottom of the dungheap */
        if (!desktop && diag) printf("**Fast timer without mouse/desktop\n");
        deskcheck();
        } /* desktop */
       else { /* Timer is running for client detection */
        /* This is needed so:                                       */
        /* (a) we can regularly update the distance calculator (if  */
        /*     visible), even if mouse moves off the client window, */
        /*     or the globe changes under the mouse;                */
        /* (b) we can update the cursor to/from cross-hair, even if */
        /*     the globe changes under the mouse.  (In theory we do */
        /*     not need to do this if not crosshair cursor, but it  */
        /*     is expected that most people will want this -- and   */
        /*     we may well do our own non-crosshair, next time.)    */
        /* First (sigh) see if the mouse is still in our window */
        WinQueryPointerPos(HWND_DESKTOP, &pp);
        hwndtemp=WinWindowFromPoint(HWND_DESKTOP, &pp, FALSE);
        if (hwndtemp!=hwndFrame) offclient();     /* not even on frame */
         else {                                   /* more to calculate */
          /* First calculate mouse position in client coordinates */
          WinQueryWindowPos(hwndFrame,  &f);      /* query frame */
          WinQueryWindowPos(hwndClient, &c);      /* query client */
          c.x=f.x+c.x; c.y=f.y+c.y;               /* => desktop coordinates */
          pp.x=pp.x-c.x; pp.y=pp.y-c.y;           /* => client coordinates */
          if (pp.x>=0 && pp.y>=0 && pp.x<c.cx && pp.y<c.cy){/* on client */
            /* OK, pointer is on client: now see if on globe */
            /* We need to recalculate lat/lon, in case */
            /* globe has changed underneath us. */
            rc=pel2ll(pp.x, pp.y, &lon, &lat);
            if (rc>=0) {
              onglobe();                          /* on the globe */
              if (showdist) showll(lat, lon);}    /* (update lat/lon) */
             else {
              offglobe();                         /* now off globe */
              } /* off globe */
            } /* on client */
           else offclient();                      /* limbo */
          } /* on frame */
        } /* timer running for distance calculator */
      } /* fast (onclient/Desktop) timer */
    break;}

  case WM_MOUSEMOVE: {             /* Mouse movement */
    long x, y;
    long lon, lat;
    int rc;
    if (desktop) break;            /* not interesting nowadays */

    if (!overclient) onclient();   /* this is news */

    x=(long)mp1s1;                 /* mouse point X, Y in pels */
    y=(long)mp1s2;
    /* convert to LONG*1000, LAT*1000 */
    rc=pel2ll(x, y, &lon, &lat);
    if (rc<0) {                    /* not on globe */
      offglobe();
      break;}                      /* (Will default to system pointer) */
    /* OK, we're on the globe */
    onglobe();                     /* use our pointer (always) */
    if (showdist) showll(lat, lon);/* show the lat/lon */
    return MFALSE;}                /* (Return MFALSE if set our own ptr) */

  case WM_BUTTON1DBLCLK: {
    char basestr[2];               /* work */
    char buff[25];                 /* .. */
    char clat[12], clon[12];       /* .. */
    if (desktop || !weactive) break;    /* not interesting */
    dcfixed=TRUE;                  /* Now fixed (base) point */
    if (dcpoints==2) {             /* Item2-> Item1, clear item2 */
      char buffo[15], buffa[15];
      WinQueryDlgItemText(hwndDist, ID_DC2LON, sizeof(buffo), buffo);
      WinQueryDlgItemText(hwndDist, ID_DC2LAT, sizeof(buffa), buffa);
      WinSetDlgItemText(  hwndDist, ID_DC1LON, buffo);
      WinSetDlgItemText(  hwndDist, ID_DC1LAT, buffa);
      WinSetDlgItemText(  hwndDist, ID_DC2LON, "");
      WinSetDlgItemText(  hwndDist, ID_DC2LAT, "");
      dclon[0]=dclon[1]; dclat[0]=dclat[1]; dclon[1]=0; dclat[1]=0;
      dcpoints=1;
      /* (And no tracking/distance yet) */
      WinSetDlgItemText(  hwndDist, ID_DCDTEXT, "");
      WinSetDlgItemText(  hwndDist, ID_DCDKM,   "");
      WinSetDlgItemText(  hwndDist, ID_DCDMILES,"");
      WinEnableControl (  hwndDist, ID_DCTRACK, FALSE);
      WinSendMsg(WinWindowFromID(hwndDist, ID_DCCANCEL),
               BM_SETDEFAULT, (MPARAM)TRUE, NULL);
      WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwndDist, ID_DCCANCEL));
      }
    /* Indicate base set */
    basestr[0]='\x1A';   /* wee right arra' */
    basestr[1]='\0';
    WinSetDlgItemText(hwndDist, ID_DC1TEXT, basestr);
    /* ... and record in the .INI file (x.xxx form) */
    putmill(clat,dclat[0]); putmill(clon,dclon[0]);
    sprintf(buff,"%s %s", clat, clon);
    if (diag) printf("Writing fixed, '%s' to .INI\n", buff);
    PrfWriteProfileString(HINI_USERPROFILE, ininame, KEYFIX, buff);
    break;}

  case WM_BUTTON2DBLCLK: {
    /* no special op */
    break;}

  /* Button going down */
  /* MB . On . Act   << Mouse button, On globe, were active ... action */
  /*  1    0    0    Pop up                                            */
  /*  1    0    1    Beep                                              */
  /*  1    1    0    Pop up                                            */
  /*  1    1    1    Show distance calculator                          */
  /*  2    0    0    Pop Up Menu                                       */
  /*  2    0    1    Pop Up Menu                                       */
  /*  2    1    0    Pop up                                            */
  /*  2    1    1    Change View                                       */

  case WM_BUTTON1DOWN:
  case WM_BUTTON2DOWN:{            /* mouse button going down */
    long lon, lat;
    long km, miles;                /* work */
    long x, y /* , z */;
    int  i, rc; USLONG fid;
    FLAG isonglobe, wereactive;    /* flags */
    unsigned long popflags;        /* for popup */
    unsigned long urc;
    char buffo[15], buffa[15];
    char buff[31];                 /* work */

    /* If on desktop, then decide whether to stay that way, etc. */
    if (desktop) {
      static char leavetext[]="Would you like to remove PMGlobe from \
the desktop?";
      /* Message box could activate (pop up) window */
      timer(STOP,FAST);            /* stop timer */
      urc=WinMessageBox(HWND_DESKTOP, HWND_OBJECT, leavetext,
          "PMGlobe leaving desktop",    /* title */
          0, MB_DEFBUTTON1 | MB_OKCANCEL | MB_SYSTEMMODAL | MB_ICONQUESTION);
      if       (urc==MBID_CANCEL) timer(START,FAST);   /* just restart timer */
       else if (urc==MBID_OK)     fromdesk();          /* leave desktop */
      break;} /* desktop */

    /* Not desktop ... make active, then see if on the globe... */
    wereactive=weactive;
    WinSetActiveWindow(HWND_DESKTOP, hwnd);          /* pop up */

    x=(long)mp1s1;                      /* click point X, Y in pels */
    y=(long)mp1s2;
    /* convert to LONG*1000, LAT*1000 */
    rc=pel2ll(x,y,&lon,&lat);
    if (rc>=0) isonglobe=TRUE; else isonglobe=FALSE;

    if (msg==WM_BUTTON1DOWN) {
      if (!wereactive) return MTRUE;         /* no more to do for MB1 */
      if (!isonglobe) {
        WinAlarm(HWND_DESKTOP, WA_WARNING);  /* off globe MB1 -> beep */
        return MTRUE;}                       /* .. and quit */
      /* Here if MB1, on globe, when active */
      /* If no calculator then show it */
      /* .. otherwise ensure it has the focus */
      if (!showdist) distvis(TRUE);
       else {    /* Focus is a 'best guess' -- could do better */
        if (dcpoints==2) fid=ID_DCTRACK; else fid=ID_DCCANCEL;
        WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwndDist, fid));
        }

      /* Decide where we will put the information */
      if (dcfixed || dcpoints>0) {           /* put into slot 2 */
        i=1;
        if (!dcfixed && dcpoints==2) {
          dclon[0]=dclon[1]; dclat[0]=dclat[1];     /* shift */
          ll2str(dclon[0], dclat[0], buffo, buffa); /* convert to strings */
          WinSetDlgItemText(hwndDist, ID_DC1LON, buffo);
          WinSetDlgItemText(hwndDist, ID_DC1LAT, buffa);
          }
        }
       else i=0;
      dcpoints=i+1;
      dclon[i]=lon; dclat[i]=lat;       /* copy */
      ll2str(lon, lat, buffo, buffa);   /* convert to strings */
      if (i==0) {
        WinSetDlgItemText(hwndDist, ID_DC1LON, buffo);
        WinSetDlgItemText(hwndDist, ID_DC1LAT, buffa);
        }
       else {
        WinSetDlgItemText(hwndDist, ID_DC2LON, buffo);
        WinSetDlgItemText(hwndDist, ID_DC2LAT, buffa);
        }
      if (i!=1) return MTRUE;           /* Only one point; no more to do */

      /* Have more than one point: Track is now allowed */
      WinEnableControl(hwndDist, ID_DCTRACK, TRUE);

      /* Prepare for distance calculation and results... */
      distcalc(dclat[0], dclon[0], dclat[1], dclon[1], &km, &miles);

      WinSetDlgItemText(hwndDist, ID_DCDTEXT,  "Distance apart:");
      sprintf(buff, "%li miles", miles);
      WinSetDlgItemText(hwndDist, ID_DCDMILES, buff);
      sprintf(buff, "%li km", km);
      WinSetDlgItemText(hwndDist, ID_DCDKM, buff);
      dckm=km; dcmiles=miles;           /* save for Track accumulator */
      return MTRUE;}                    /* MB1 done */

    /* Here if MB2 down, active or not */
    if (isonglobe) {
      if (!wereactive)                      /* inactive & on globe */
        WinAlarm(HWND_DESKTOP, WA_WARNING); /* warn */
       else {                               /* active and on globe */
        if (mousepos) newview(lat, lon);}
      return MTRUE;}                        /* .. and done */

    /* Here for MB2 down off globe */
    popflags=PU_HCONSTRAIN   | PU_VCONSTRAIN
           | PU_MOUSEBUTTON1 | PU_MOUSEBUTTON2 | PU_KEYBOARD;
    /* offsets put bottom item under mouse (usually) */
    WinPopupMenu(hwndClient, hwndClient, hwndPop,
                 x-8, y-8, 0, popflags);
    return MTRUE;}

  case WM_COMMAND: {   /* Asynchronous controls (Buttons+Menus) */
    int i; char buff[MAXCOMM+1];        /* work */
    /* source=COMMANDMSG(&msg)->source; */
    id=COMMANDMSG(&msg)->cmd;

    switch (id) {
      case ID_SPACE:                    /* spacelight */
      case ID_STAR:                     /* starlight */
      case ID_SUN: {                    /* sunlight */
        if (!sunlight && gmtoffset==BADLONG) { /* Need timezone */
          if (WinMessageBox(HWND_DESKTOP, hwndFrame,
            "PMGlobe needs to know your timezone before it can display sunlight.\n\n\
Would you like to set it now?",
            "Need timezone...",  /* title */
            0, MB_DEFBUTTON2 | MB_YESNO | MB_ICONQUESTION)
           == MBID_YES)
            WinPostMsg(hwnd, UM_TIMEZONE, MPFROMSHORT(id), NULL);
          break;}

        /* To get here we have a good timezone */
        if       (id==ID_STAR)  gldo("set starlight invert");
         else if (id==ID_SPACE) gldo("set space invert");
         else  /* id==ID_SUN */ {
          gldo("set sunlight invert");
          if (!sunlight) timer(STOP,SLOW);   /* in case running */
          } /* ID_SUN */
        newdraw();
        break;}

      case ID_3D: {                     /* shaded */
        gldo("set shading invert"); newdraw(); break;}

      case ID_IDLEDRAW: {               /* draw at IDLE */
        gldo("set idledraw invert"); break;}

      case ID_CROSSHAIR: {              /* use crosshair cursor */
        gldo("set crosshair invert"); break;}

      case ID_MENUBAR: {                /* flip menubar */
        setmenubar((FLAG)!menubar);
        break;}

      case ID_TITLEBAR: {               /* flip titlebar */
        settitlebar((FLAG)!titlebar);
        break;}

      /* New land colour */
      case ID_CLRED:      newland(CLR_RED);      newdraw(); break;
      case ID_CLGREEN:    newland(CLR_GREEN);    newdraw(); break;
      case ID_CLBLUE:     newland(CLR_BLUE);     newdraw(); break;
      case ID_CLCYAN:     newland(CLR_CYAN);     newdraw(); break;
      case ID_CLPINK:     newland(CLR_PINK);     newdraw(); break;
      case ID_CLYELLOW:   newland(CLR_YELLOW);   newdraw(); break;
      case ID_CLGRAY:     newland(CLR_DARKGRAY); newdraw(); break;
      case ID_CLWHITE:    newland(CLR_WHITE);    newdraw(); break;
      case ID_CLBLACK:    newland(CLR_BLACK);    newdraw(); break;

      /* New sea colour */
      case ID_CSRED:      newsea(CLR_RED);       newdraw(); break;
      case ID_CSGREEN:    newsea(CLR_GREEN);     newdraw(); break;
      case ID_CSBLUE:     newsea(CLR_BLUE);      newdraw(); break;
      case ID_CSCYAN:     newsea(CLR_CYAN);      newdraw(); break;
      case ID_CSPINK:     newsea(CLR_PINK);      newdraw(); break;
      case ID_CSYELLOW:   newsea(CLR_YELLOW);    newdraw(); break;
      case ID_CSGRAY:     newsea(CLR_DARKGRAY);  newdraw(); break;
      case ID_CSWHITE:    newsea(CLR_WHITE);     newdraw(); break;
      case ID_CSBLACK:    newsea(CLR_BLACK);     newdraw(); break;

      /* New grid colour */
      case ID_CGRED:      newgrid(CLR_RED);      newdraw(); break;
      case ID_CGDARKRED:  newgrid(CLR_DARKRED);  newdraw(); break;
      case ID_CGGREEN:    newgrid(CLR_GREEN);    newdraw(); break;
      case ID_CGDARKGREEN:newgrid(CLR_DARKGREEN);newdraw(); break;
      case ID_CGBLUE:     newgrid(CLR_BLUE);     newdraw(); break;
      case ID_CGDARKBLUE: newgrid(CLR_DARKBLUE); newdraw(); break;
      case ID_CGCYAN:     newgrid(CLR_CYAN);     newdraw(); break;
      case ID_CGDARKCYAN: newgrid(CLR_DARKCYAN); newdraw(); break;
      case ID_CGPINK:     newgrid(CLR_PINK);     newdraw(); break;
      case ID_CGPURPLE:   newgrid(CLR_DARKPINK); newdraw(); break;
      case ID_CGYELLOW:   newgrid(CLR_YELLOW);   newdraw(); break;
      case ID_CGBROWN:    newgrid(CLR_BROWN);    newdraw(); break;
      case ID_CGGRAY:     newgrid(CLR_DARKGRAY); newdraw(); break;
      case ID_CGWHITE:    newgrid(CLR_WHITE);    newdraw(); break;
      case ID_CGBLACK:    newgrid(CLR_BLACK);    newdraw(); break;

      /* New background colour */
      case ID_CBRED:      newback(CLR_RED);      newdraw(); break;
      case ID_CBDARKRED:  newback(CLR_DARKRED);  newdraw(); break;
      case ID_CBGREEN:    newback(CLR_GREEN);    newdraw(); break;
      case ID_CBDARKGREEN:newback(CLR_DARKGREEN);newdraw(); break;
      case ID_CBBLUE:     newback(CLR_BLUE);     newdraw(); break;
      case ID_CBDARKBLUE: newback(CLR_DARKBLUE); newdraw(); break;
      case ID_CBCYAN:     newback(CLR_CYAN);     newdraw(); break;
      case ID_CBDARKCYAN: newback(CLR_DARKCYAN); newdraw(); break;
      case ID_CBPINK:     newback(CLR_PINK);     newdraw(); break;
      case ID_CBPURPLE:   newback(CLR_DARKPINK); newdraw(); break;
      case ID_CBYELLOW:   newback(CLR_YELLOW);   newdraw(); break;
      case ID_CBBROWN:    newback(CLR_BROWN);    newdraw(); break;
      case ID_CBGRAY:     newback(CLR_DARKGRAY); newdraw(); break;
      case ID_CBPALEGRAY: newback(CLR_PALEGRAY); newdraw(); break;
      case ID_CBWHITE:    newback(CLR_WHITE);    newdraw(); break;
      case ID_CBBLACK:    newback(CLR_BLACK);    newdraw(); break;

      /* New twilight setting */
      case ID_TWI0   : newtwi(    0L); newdraw(); break;
      case ID_TWIZ50 : newtwi(  833L); newdraw(); break;
      case ID_TWI5   : newtwi( 5000L); newdraw(); break;
      case ID_TWI6   : newtwi( 6000L); newdraw(); break;
      case ID_TWI12  : newtwi(12000L); newdraw(); break;
      case ID_TWI18  : newtwi(18000L); newdraw(); break;

      /* Allow mouse button 2 */
      case ID_POSMB2: check(id, mousepos=(FLAG)!mousepos); break;

      /* Change grid settings */
      case ID_GRIDLAT10: if (gridlat10) i=0; else i=10;
           sprintf(buff,"set gridlat %i",i);
           gldo(buff); newdraw(); break;
      case ID_GRIDLAT15: if (gridlat15) i=0; else i=15;
           sprintf(buff,"set gridlat %i",i);
           gldo(buff); newdraw(); break;
      case ID_GRIDLAT30: if (gridlat30) i=0; else i=30;
           sprintf(buff,"set gridlat %i",i);
           gldo(buff); newdraw(); break;
      case ID_GRIDEQ: if (grideq) i=0; else i=90;
           sprintf(buff,"set gridlat %i",i);
           gldo(buff); newdraw(); break;
      case ID_GRIDLON10: if (gridlon10) i=0; else i=10;
           sprintf(buff,"set gridlon %i",i);
           gldo(buff); newdraw(); break;
      case ID_GRIDLON15: if (gridlon15) i=0; else i=15;
           sprintf(buff,"set gridlon %i",i);
           gldo(buff); newdraw(); break;
      case ID_GRIDLON30: if (gridlon30) i=0; else i=30;
           sprintf(buff,"set gridlon %i",i);
           gldo(buff); newdraw(); break;
      case ID_GRIDARCTIC: gldo("set gridpolar invert" ); newdraw(); break;
      case ID_GRIDTROPIC: gldo("set gridtropic invert"); newdraw(); break;
      case ID_GRIDALL:
           gldo("set gridlat 10");    gldo("set gridlon 10");
           gldo("set gridpolar on");  gldo("set gridtropic on");
           newdraw(); break;
      case ID_GRIDNONE:
           gldo("set gridlat 0");     gldo("set gridlon 0");
           gldo("set gridpolar off"); gldo("set gridtropic off");
           newdraw(); break;

      /* Modify position (view) */
      case ID_POS0  :  newview(     0L,     0L); break;
      case ID_POSNP :  newview(+90000L,     0L); break;
      case ID_POSSP :  newview(-90000L,     0L); break;
      case ID_POS90W:  newview(     0L,-90000L); break;
      case ID_POS90E:  newview(     0L,+90000L); break;
      case ID_POS180:  newview(     0L,180000L); break;
      /* Snap to equator or prime meridian */
      case ID_POSEQ :  newview(     0L,viewlon); break;
      case ID_POSGM :  newview(viewlat,     0L); break;

      /* Refresh interval settings */
      case ID_REF0 : newref( 0L    ); break; /* 0 means immediate */
      case ID_REF1 : newref( 1L*60L); break;
      case ID_REF5 : newref( 5L*60L); break;
      case ID_REF10: newref(10L*60L); break;
      case ID_REF15: newref(15L*60L); break;
      case ID_REF20: newref(20L*60L); break;
      case ID_REF30: newref(30L*60L); break;
      case ID_REF60: newref(60L*60L); break;

      case ID_DESKTOP: {           /* Desktop status */
        gldo("set desktop invert"); break;}

      case ID_FULL:           /* Make fullscreen */
        makefull();
        break;
      case ID_QUARTER:        /* Make quarter screen */
        makequart();
        break;

      case ID_SQUARE: {       /* Make window square */
        long newx, newy;      /* desired X and Y */
        int  adjx, adjy;      /* adjustments */
        long d;               /* work */
        SWP swp;              /* .. */

        d=lmin(clientx,clienty);
        d=lmin((d-1)/2,MAXRADIUS)*2+1;            /* best diameter */
        newx=d; newy=d;

        /* Standard piece of code */
        WinQueryWindowPos(hwndClient, &swp);      /* query client */
        adjx=(int)(newx-swp.cx); adjy=(int)(newy-swp.cy);
        if (adjx!=0 || adjy!=0) {                 /* adjustment needed */
          WinQueryWindowPos(hwndFrame, &swp);     /* query frame */
          WinSetWindowPos(hwndFrame, 0,           /* posts the WM_SIZE */
            swp.x, swp.y-adjy,                    /* down if bigger */
            swp.cx+adjx, swp.cy+adjy,             /* new dimensions */
            SWP_SIZE | SWP_MOVE);                 /* flags */
          }
        /* end of standard code */
        break;}

      case ID_QUIT:
        WinPostMsg(hwnd, WM_CLOSE, NULL, NULL);   /* do a close */
        break;

      case ID_MINIMIZE:                           /* do a minimize */
        WinSetWindowPos(hwndFrame, 0, 0,0,0,0, SWP_MINIMIZE);
        break;
      case ID_MAXIMIZE:                           /* do a maximize */
        WinSetWindowPos(hwndFrame, 0, 0,0,0,0, SWP_MAXIMIZE);
        break;
      case ID_RESTORE:                            /* do a restore */
        WinSetWindowPos(hwndFrame, 0, 0,0,0,0, SWP_RESTORE);
        break;

      /* Help processes .. passed on to outer message processor */
      case ID_HELPABOUT: case ID_HELPOPTS:  case ID_HELPSTAN:
      case ID_HELPVIEWS: case ID_HELPREF:   case ID_HELPINTRO:
      case ID_HELPGRID:  case ID_HELPLIGHT: case ID_HELPTWI:
      case ID_HELPBACK:  case ID_HELPMACRO: case ID_HELPCOLOUR:
        WinPostMsg(hwnd, (unsigned)(WM_USER+id), NULL, NULL);
        break;

      case ID_SHOWDC: {            /* Make DISTCALC visible or not */
        distvis((FLAG)!showdist); break;}

      case ID_SAVESET: {           /* Save settings */
        saveset(); break;}

      case ID_SAVEPOS: {           /* Save position */
        savepos(); break;}

      case ID_RESTSET: {           /* Restore settings */
        str2set(initset); set2menu();
        newdraw(); break;}

      case ID_DEFSET:  {           /* Default settings */
        str2set(defset);  set2menu();
        newdraw(); break;}

      case ID_RESTPOS: {           /* Restore position */
        str2pos(initpos);
        WinSetWindowPos(hwndFrame, 0,        /* position frame */
          initswp.x, initswp.y, initswp.cx, initswp.cy,
          SWP_SIZE | SWP_MOVE | SWP_ACTIVATE | SWP_SHOW); /* flags */
        WinSetWindowPos(hwndDist,  0,        /* position distcalc */
          dcposx, dcposy, 0, 0, SWP_MOVE);
        WinSetWindowPos(hwndComm,  0,        /* position command */
          cmposx, cmposy, 0, 0, SWP_MOVE);
        newdraw();
        break;}

      case ID_COMMDLG:                       /* surface the command line */
        WinSetFocus(HWND_DESKTOP,
          WinWindowFromID(hwndComm, ID_COMMDLGNAME));
        WinShowWindow(hwndComm, TRUE);       /* Make visible */
        break;

      case ID_HALTMAC:                       /* stop running macro */
        if (diag) printf("Got ID_HALTMAC\n");
        WinPostMsg(hwnd, UM_COMMHALT, NULL, NULL); /* pass on */
        break;

      case ID_RUNMAC: {                      /* find and run macro */
        static FILEDLG filedlg;              /* static to avoid memset() */
        char ftitle[MAXTITLE];
        char fstart[6];

        sprintf(ftitle,"Run %s macro...", NAME);
        sprintf(fstart,"*.%s", MACEXT);

        /* Find a file and run it */
        filedlg.cbSize=sizeof(FILEDLG);      /* start initialize... */
        filedlg.fl=FDS_CENTER | FDS_OPEN_DIALOG;
        filedlg.pszTitle=ftitle;
        filedlg.pszOKButton="Run";
        strcpy(filedlg.szFullFile, fstart);       /* where to look */
        filedlg.pfnDlgProc=(PFNWP)setfileicon;    /* filter */

        /* Use the standard file dialog to get the file */
        if (!WinFileDlg(HWND_DESKTOP, hwndClient, (PFILEDLG)&filedlg))
          {printf("File dialog failed to start\n"); break;}
        if (filedlg.lReturn!=DID_OK) break;       /* "Run" not selected */
        /* OK, we have a selected file to run */
        if (strlen(filedlg.szFullFile)>(MAXCOMM-6))
           filedlg.szFullFile[MAXCOMM-6] ='\0';/* truncate if too long */

        if (diag) printf("About to run '%s'\n", filedlg.szFullFile);
        /* Tell command dialogue that command is running, after hiding */
        WinShowWindow(hwndComm, FALSE);      /* hide the command dialog */
        WinSendMsg(hwndComm, UM_COMMDO, NULL, NULL);
        sprintf(commshare,"macro %s", filedlg.szFullFile);  /* add command */
        WinPostMsg(hwndClient, UM_COMMDO, NULL, NULL);      /* launch */
        break;} /* run macro */

      case ID_TIMEZONE:
        WinPostMsg(hwnd, UM_TIMEZONE, NULL, NULL); /* pass on */
        break;
      } /* ID switch */
    return MFALSE;} /* WM_COMMAND */

  /* Help processor -- common path */
  case WM_USER+ID_HELPABOUT: case WM_USER+ID_HELPINTRO:
  case WM_USER+ID_HELPOPTS:  case WM_USER+ID_HELPREF:
  case WM_USER+ID_HELPSTAN:  case WM_USER+ID_HELPVIEWS:
  case WM_USER+ID_HELPGRID:  case WM_USER+ID_HELPLIGHT:
  case WM_USER+ID_HELPTWI:   case WM_USER+ID_HELPBACK:
  case WM_USER+ID_HELPMACRO: case WM_USER+ID_HELPCOLOUR:
  case WM_HELP: {
    int hid;                  /* Help message ID */
    char htitle[257];         /* work */
    char htext[257];          /* .. */
    if (msg==WM_HELP) hid=ID_HELPABOUT;      /* system help request */
                 else hid=(signed)msg-(signed)WM_USER; /* all other helps */
    strcpy(htitle,"?"); strcpy(htext,"?");   /* just in case not found */
    WinLoadString(hab, NULL, (unsigned)hid, sizeof(htitle), htitle);
    /* Special case for the 'about' panel -- fill in name and version */
    if (hid==ID_HELPABOUT) {
      /* single-version method: */
      /* sprintf(htext, htitle, NAME, VERSION); */  /* edit to buffer.. */
      /* strcpy(htitle,htext); */                   /* .. and copy back */
      #if defined(B16)
      sprintf(htitle,"%s %s (16-bit)", NAME, VERSION);
      #else
      sprintf(htitle,"%s %s (32-bit)", NAME, VERSION);
      #endif
      }
    WinLoadString(hab, NULL, (unsigned)(hid+1), sizeof(htext) , htext);
    WinMessageBox(HWND_DESKTOP, hwndFrame, htext, htitle,
      0, MB_OK+MB_MOVEABLE+MB_INFORMATION);
    return MFALSE;}

  case UM_TEST: {                       /* Experimental */
    return MFALSE;}

  case UM_TODESK: {           /* Move to desktop */
    todesk();
    return MFALSE;}
  case UM_FROMDESK: {         /* Move from desktop */
    fromdesk();
    return MFALSE;}

  case UM_DRAWN: {            /* Landmap or globe completed */
    /* (Not sent if drawing was interrupted) */
    if (refresh>0 && (sunlight || clockmarks>0)) {
      long now;
      time(&now);
      nexttime=(now/refresh)*refresh + refresh;  /* Next boundary */
      if (diag) printf("Refresh set for +%lis\n", nexttime-now);
      timer(START,SLOW);      /* start up slow timer */
      }
    drawing=FALSE;            /* indicate drawing done */
    return MFALSE;}

  case WM_MINMAXFRAME: {
    USLONG flags; HPOINTER hptr;
    #if defined(B16)
    flags=((SWP *)PVOIDFROMMP(mp1))->fs;
    #else
    flags=((SWP *)PVOIDFROMMP(mp1))->fl;
    #endif
    hptr=hptrIcon;                                /* default show icon */
    if ((flags & (unsigned)SWP_MINIMIZE) || (flags & (unsigned)SWP_MAXIMIZE)) {
      /* Minimizing or maximizing .. hide dialog/items until requested */
      if (diag) printf("Min/maximizing...\n");
      enable(ID_SAVEPOS, FALSE);                  /* disable menu option */
      WinShowWindow(hwndComm, FALSE);             /* disable commands */
      if (showdist) distvis(FALSE);               /* .. and calculator */
      if (flags & (unsigned)SWP_MINIMIZE) hptr=NULLHANDLE;  /* no icon */
      }
     else enable(ID_SAVEPOS, TRUE);               /* restore menu */

    WinSendMsg(hwndFrame,  WM_SETICON, MPFROMP(hptr), MPVOID);
    return MFALSE;}                               /* (do defaults) */

  case WM_SIZE: {
    SWP client;
    int x, y;
    x=mp2s1; y=mp2s2;         /* X, Y are new size in pels */
    if (x>0 && y>0 && windowset) {      /* not minimizing to size 0 */
      long oldsize, newsize;
      oldsize=lmin(clientx,clienty);
      /* Get true size of client window (actually are told these) */
      WinQueryWindowPos(hwndClient, &client);
      DosEnterCritSec(); /* ensure consistent */
      clientx=client.cx; clienty=client.cy;
      DosExitCritSec();
      /* Now redraw, unless no size change, or this is a squaring-off */
      /* of single even pel */
      newsize=lmin(clientx, clienty);
      if (newsize!=oldsize) {     /* could be size change  */
        if (newsize!=oldsize-1                       /* >1 */
         || oldsize!=(oldsize/2)*2) newdraw();       /* was odd */
        }
      } /* not minimizing */
    break;}

  case WM_ACTIVATE:
    if (mp1) weactive=TRUE;
     else {
      weactive=FALSE;
      if (overclient) offclient(); /* for efficiency */
      }
    break;

  case WM_SETFOCUS:
    /* if (mp2) { } */             /* unused */
    break;

  case WM_CONTROLPOINTER:          /* Move out of window .. to border */
  case WM_MENUSELECT:              /* .. or to F10-selected menu */
    if (overclient) offclient();   /* for efficiency */
    break;

  case WM_PAINT:
    glpaint();
    /* If we are idling in desktop mode then.. */
    if (desktop && timefast) deskcheck(); /* quick response pushdown */
    return MFALSE;

  case UM_CREATED: {
    USLONG opts=0; HPOINTER hptr;

    /* Set title... */
    sprintf(comm,"set title %s", NAME);
    gldo(comm);                    /* set default title */
    gldo("font Default");          /* create default font */
    gldo("set font default");      /* .. and select */

    /* Set window positions, etc. */
    windowset=TRUE;                          /* WM_SIZE now OK */
    /* First position the main window */
    if (fullforce) makefull();
     else {
      /* Be sure to include initial state */
      if (streq(initwin,"MIN")) {
        opts=SWP_MINIMIZE;
        hptr=NULLHANDLE;                     /* no icon */
        /* Don't set icon here -- upsets 2.0 Minimized window viewer */
        }
       else {
        if (streq(initwin,"MAX")) opts=SWP_MAXIMIZE;
        hptr=hptrIcon;                       /* show icon */
        WinSendMsg(hwndFrame, WM_SETICON, MPFROMP(hptr), MPVOID);
        }
      WinSetWindowPos(hwndFrame, 0,          /* position frame */
        initswp.x, initswp.y, initswp.cx, initswp.cy,
        opts | SWP_SIZE | SWP_MOVE | SWP_SHOW);   /* flags */
      } /* not to desk */
    if (desktop) todesk();
     else WinSetActiveWindow(HWND_DESKTOP, hwndClient);

    WinSetWindowPos(hwndDist,  0,       /* position distcalc */
      dcposx, dcposy, 0, 0, SWP_MOVE);
    WinSetWindowPos(hwndComm,  0,       /* position command */
      cmposx, cmposy, 0, 0, SWP_MOVE);

    newdraw();                          /* always start a draw now */

    if (commshare[0]!='\0') {           /* have a macro to run */
      startup=TRUE;                     /* indicate */
      enable(ID_RUNMAC, FALSE);         /* disable drop-down item */
      WinPostMsg(hwnd, UM_COMMDO, NULL, NULL);
      }
     else WinPostMsg(hwndComm, UM_COMMDONE, NULL, NULL); /* release */
    return MFALSE;}

  case WM_CREATE: {
    HDC hdc;
    RECTL rcl;
    long caps[CAPS_DEVICE_FONT_SIM+1];  /* for device capabilities */
    char pos[MAXPOSLEN+1];
    char word[MAXTOK+1];
    unsigned long urc;

    /* Prepare the graphics environment */
    hdc=WinOpenWindowDC(hwnd);
    cps=GpiCreatePS(hab, hdc, &size0,
      PU_PELS                      /* Units pels */
      | GPIT_MICRO | GPIA_ASSOC);  /* Micro ps, associated */

    /* Find out font pels/inch for this device */
    if (!DevQueryCaps(hdc, 0L, CAPS_DEVICE_FONT_SIM+1L, caps))
      printf("DevQueryCaps failed\n");
    if (diag) {
      printf("Screen width, height: %li %li\n",
        caps[CAPS_WIDTH],                 caps[CAPS_HEIGHT]);
      printf("X, Y resolution ppm: %li %li\n",
        caps[CAPS_HORIZONTAL_RESOLUTION], caps[CAPS_VERTICAL_RESOLUTION]);
      printf("Font X, Y res ppi: %li %li\n",
        caps[CAPS_HORIZONTAL_FONT_RES],   caps[CAPS_VERTICAL_FONT_RES]);
      }
    screenx=caps[CAPS_WIDTH]; screeny=caps[CAPS_HEIGHT];
    fontresx=caps[CAPS_HORIZONTAL_FONT_RES];
    fontresy=caps[CAPS_VERTICAL_FONT_RES];

    /* Find the handles of the frame, menu window, and desktop */
    hwndFrame=WinQueryWindow(hwnd, QW_PARENT);
    hwndMenu=WinWindowFromID(hwndFrame, FID_MENU);
    hwndMParent=WinQueryWindow(hwndMenu, QW_PARENT);
    hwndDesk=WinQueryDesktopWindow(hab,NULL);

    /* Get pointer handles */
    hptrSystem=WinQuerySysPointer(HWND_DESKTOP, SPTR_ARROW, FALSE);
    hptrGlobe=WinLoadPointer(HWND_DESKTOP, NULL, ID_GLOBEPTR);
    hptrIcon=WinLoadPointer(HWND_DESKTOP, NULL, ID_FRAMERC);

    /* Get popup menus */
    hwndPop=WinLoadMenu(hwnd, NULLHANDLE, ID_FRAMERC);

    /* Send icon to permanent frames */
    if (hptrIcon==NULL) printf("** Could not find Icon\n");
     else { /* set it to known frames */
      WinSendMsg(hwndFrame, WM_SETICON, MPFROMP(hptrIcon), MPVOID);
      WinSendMsg(hwndDist,  WM_SETICON, MPFROMP(hptrIcon), MPVOID);
      WinSendMsg(hwndComm,  WM_SETICON, MPFROMP(hptrIcon), MPVOID);
      /* nothing useful to be done if one fails */ }

    /* No macro/command running yet... */
    enable(ID_HALTMAC, FALSE);
    WinShowWindow(WinWindowFromID(hwndComm, ID_COMMDLGHALT), FALSE);

    /* Determine the PM-set default frame window size and position.  */
    /* Save this for later use, and also copy for initial default */
    WinQueryTaskSizePos(hab,0,&defswp);
    initswp=defswp;

    /* Get any initial position and size from user profile */
    pos[0]='\0';                   /* case of error (B&B) */
    dcposx=8; dcposy=8; cmposx=8; cmposy=8;  /* case unset */
    urc=PrfQueryProfileString(HINI_USERPROFILE, ininame, KEYPOS,
        "", pos, (long)MAXPOSLEN); /* returns stringlen+1 */
    if (urc>1 && urc<=MAXPOSLEN+1) { /* care: QPrf seems a bit tricksy */
      /* We have a plausible string .. process it to positions */
      str2pos(pos);
      strcpy(initpos, pos);
      }
    urc=PrfQueryProfileString(HINI_USERPROFILE, ininame, KEYWIN,
        "", pos, (long)MAXWINLEN+1); /* returns stringlen+1 */
    if (urc>1 && urc<=MAXWINLEN+1) { /* care: QPrf seems a bit tricksy */
      /* We have a plausible string .. save it */
      strcpy(initwin, pos);
      }
    /* (Otherwise use the PM default, already in INITSWP.) */
    /* Now calculate the client window size , as it's not known yet. */
    /* We have an SWP, which we need in a RECTL... */
    rcl.xLeft  =initswp.x; rcl.xRight=initswp.x+initswp.cx;
    rcl.yBottom=initswp.y; rcl.yTop  =initswp.y+initswp.cy;
    /* Call conversion routine to get client size .. */
    WinCalcFrameRect(hwndFrame,&rcl,TRUE);
    /* .. and finally remove the base offset to get sizes */
    clientx=rcl.xRight-rcl.xLeft;
    clienty=rcl.yTop  -rcl.yBottom;

    /* Get any initial 'fixed' point from user profile */
    pos[0]='\0';                   /* case of error (B&B) */
    urc=PrfQueryProfileString(HINI_USERPROFILE, ininame, KEYFIX,
        "", pos, (long)MAXPOSLEN); /* returns stringlen+1 */
    if (urc>1 && urc<=MAXPOSLEN+1) { /* care: QPrf seems a bit tricksy */
      long lat, lon;
      /* We have a plausible string .. process it to fixed point */
      getword(pos,word);  lat=getmill(word);
      if (labs(lat)>90000L) lat=BADLONG;
      getword(pos,word);  lon=getmill(word);
      if (labs(lon)>180000L) lon=BADLONG;
      if (lat!=BADLONG && lon!=BADLONG) {
        /* Note: similar code to above, but not shared in order */
        /* to make this fast as possible */
        char buffa[15], buffo[15], basestr[2];
        dcpoints=1;                     /* now 1 point */
        dclon[0]=lon; dclat[0]=lat;     /* copy */
        dcfixed=TRUE;                   /* have a fixed */
        ll2str(dclon[0], dclat[0], buffo, buffa); /* convert to strings */
        WinSetDlgItemText(hwndDist, ID_DC1LON, buffo);
        WinSetDlgItemText(hwndDist, ID_DC1LAT, buffa);
        basestr[0]='\x1A';   /* wee right arra' */
        basestr[1]='\0';
        WinSetDlgItemText(hwndDist, ID_DC1TEXT, basestr);
        } /* OK fixed */
      } /* looking for fixed point */

    /* See if any system timezone set, if so, process and change */
    /* our variables. */
    { /* block */
    char set[TZACTIVELEN+1];       /* receiver */
    char tzword[TZACTIVELEN+1];    /* work */
    long base, season;             /* base and seasonal offsets */
    set[0]='\0';                   /* case of error (Belt&Braces) */
    urc=PrfQueryProfileString(HINI_SYSTEMPROFILE, TZNAME, TZACTIVE,
        "", set, (long)TZACTIVELEN+1); /* returns stringlen? */
    if (urc>1 && urc<=TZACTIVELEN+1) { /* care: QPrf seems a bit tricksy */
      /* we have a candidate string */
      getword(set,tzword);  base=gettime(tzword);
      getword(set,tzword);  season=gettime(tzword);
      getword(set,tzword);  /* zone */
      if (base!=BADLONG && season!=BADLONG
         && (strlen(tzword)==3 || strlen(tzword)==4)) { /* OK! */
        gmtoffset=base; summer=season;  /* set globals */
        strcpy(zonename,tzword);        /* .. */
        if (diag) printf("Got %li %li '%s' from OS2SYS.INI\n",
          base, season, zonename);
        }
       else printf("Invalid TimeZone.Active in OS2SYS.INI\n");
      }
     else if (diag) printf("No TimeZone in OS2SYS.INI\n");
    } /* block */

    /* Set up default/user settings */
    viewlon =0; viewlat=0; viewrot=0;   /* LONG/LAT/ROT of view */
    sunlight=FALSE;                     /* plain as default */
    threed  =FALSE;                     /* 3-D is luxury */
    spacelight=FALSE;                   /* space is a bit harsh */
    starlight=FALSE;                    /* starlight is better, though */
    mousepos=TRUE;                      /* allow manipulation */
    desktop =FALSE;                     /* not desktop */
    showdist=FALSE;                     /* not startup */
    refresh=5*60;                       /* 5 minutes */
    twilight=0;                         /* no twilight */
    idtwi=ID_TWI0;                      /* .. */
    gridcoli=CLR_DARKRED;               /* grid colour index */
    idcgrid=ID_CGDARKRED;               /* .. */
    backcoli=CLR_DARKGRAY;              /* background colour index */
    idcback=ID_CBGRAY;                  /* .. */
    landcoli=CLR_GREEN;                 /* land colour index */
    idcland=ID_CLGREEN;                 /* .. */
    seacoli=CLR_BLUE;                   /* sea colour index */
    idcsea=ID_CSBLUE;                   /* .. */
                                        /* (mark colour set earlier) */
    /* grid settings */
    gridlon10=FALSE; gridlon15=FALSE; gridlon30=TRUE;
    gridlat10=FALSE; gridlat15=FALSE; gridlat30=TRUE;
    gridarctic=TRUE; gridtropic=TRUE; grideq=FALSE;

    set2str(defset);               /* save default settings */
    if (diag) printf("Default settings: %s\n", defset );

    /* Get any initial settings from user profile */
    {char set[MAXSETLEN+1];
    set[0]='\0';                   /* case of error (B&B) */
    urc=PrfQueryProfileString(HINI_USERPROFILE, ininame, KEYSET,
        "", set, (long)MAXSETLEN); /* returns stringlen+1 */
    if (urc>1 && urc<=MAXSETLEN+1) { /* care: QPrf seems a bit tricksy */
      /* We have a plausible string .. process it to settings */
      str2set(set);                /* update the actual settings */
      }

    /* Override desktop/size settings if requested */
    if (deskforce) desktop=TRUE;
    if (fullforce) {
      WinQueryWindowRect(HWND_DESKTOP,&rcl); /* Get desktop size */
      clientx=rcl.xRight-rcl.xLeft;
      clienty=rcl.yTop-rcl.yBottom;
      } /* force desktop and full screen */
    } /* block */

    /* Now safe for the globe thread to proceed... */
    DosPostEventSem(semhavescreen);          /* tell globe thread */

    /* Record settings as initials */
    set2str(initset);                        /* save actual start settings */
    if (diag) printf("Initial settings: %s\n", initset);
    /* Check menu items as required */
    set2menu();                    /* reflect in menus */

    /* Defer some stuff for a while */
    WinPostMsg(hwnd, UM_CREATED, NULL, NULL);
    break;}  /* WM_CREATE */

  case WM_CLOSE:
  case WM_SAVEAPPLICATION: {
    savepos();                /* save window positions */
    saveset();                /* save settings */
    break;}

  case UM_COMMDO:    {        /* Command start */
    int i;
    /* Halt is only enabled during user macro/command run, so we */
    /* don't stop the Client (and others) running commands. */
    /* Ensure halt is cleared (B&B) */
    commhalt=FALSE; for (i=0; i<MAXREXX; i++) rexxhalts[i]=FALSE;
    canhalt=TRUE;                            /* halt allowed */

    /* If we don't have a real command, quit now */
    if (commshare[0]=='\0') {           /* have no command to run */
      WinPostMsg(hwndComm, UM_COMMDONE, NULL, NULL);   /* release */
      break;}

    enable(ID_RUNMAC, FALSE);                /* disable drop-down item */

    /* Tell the command thread to proceed, and wait for ack... */
    DosResetEventSem(semhavecomm, &posted);  /* engage lock */
    DosPostEventSem(semruncomm);             /* release macro thread */
    DosWaitEventSem(semhavecomm, SEM_INDEFINITE_WAIT); /* almost immediate */
    break;}

  case UM_COMMDONE: {         /* Command done: [rc], NULL */
    long rc; int i;
    if (startup) {
      if (diag) printf("Startup macro command done\n");
      rc=(long)mp1l;
      if (rc>=0) newdraw();   /* not a failure .. assume changes made */
      startup=FALSE;
      }
    /* clear any halt that we had */
    canhalt=FALSE;                      /* halt no longer allowed */
    commhalt=FALSE; for (i=0; i<MAXREXX; i++) rexxhalts[i]=FALSE;
    commshare[0]='\0';                  /* show processed */
    /* Restore user interaction in dialogue box */
    WinPostMsg(hwndComm, UM_COMMDONE, NULL, NULL);
    enable(ID_RUNMAC, TRUE);            /* .. and drop-down */
    break;}

  case UM_MACROBEG:                     /* begin running macro */
    WinSetDlgItemText(hwndComm, ID_COMMDLGTEXT, "Running macro...");
    enable(ID_HALTMAC, TRUE);
    WinShowWindow(WinWindowFromID(hwndComm, ID_COMMDLGHALT), TRUE);
    break;
  case UM_MACROEND:                      /* end running macro */
    WinShowWindow(WinWindowFromID(hwndComm, ID_COMMDLGHALT), FALSE);
    enable(ID_HALTMAC, FALSE);
    WinSetDlgItemText(hwndComm, ID_COMMDLGTEXT, "");
    break;

  case UM_COMMHALT: {                   /* halt running commands/macro */
    int i;
    if (!canhalt) break;                /* not enabled */
    if (diag) printf("Command halt\n");
    commhalt=TRUE; for (i=0; i<MAXREXX; i++) rexxhalts[i]=TRUE;
    break;}

  case UM_REDRAW:                       /* new draw request */
    newdraw();
    return MFALSE;

  case UM_ALERT: {                      /* MP1 is descriptive message */
    char buff[255];
    sprintf(buff, "Sorry, unable to draw the globe.  [%s.]\n\n\
Try a different window size?",
    (char *)PVOIDFROMMP(mp1));
    if (diag) printf("Error drawing globe: %s\n",      /* there, too */
                      (char *)PVOIDFROMMP(mp1));
    WinMessageBox(HWND_DESKTOP, hwndFrame, buff,
      "Error drawing globe...", 0, MB_CANCEL | MB_ICONEXCLAMATION);
    alerted=FALSE;                      /* allow another alert */
    return MFALSE;}

  case UM_NOMAP:
    WinMessageBox(HWND_DESKTOP, hwndFrame,
      "Sorry, could not create world bitmap",
      "Ending Globe...", 0, MB_CANCEL | MB_ICONEXCLAMATION);
    WinPostMsg(hwnd, WM_QUIT, NULL, NULL);
    return MFALSE;

  case UM_DCTRACK: {                    /* Track button request */
    char clat[12], clon[12];            /* work */
    char buff[51];                      /* .. */
    if (dcpoints!=2) {                  /* Wrong number of points(!) */
      printf("TRACK when not 2(!)\n");
      return MFALSE;}                   /* ignore */

    /* First update the track distance accumulator, and display */
    dctotmiles=dctotmiles+dcmiles; dctotkm=dctotkm+dckm;
    WinSetDlgItemText(hwndDist, ID_DCTTEXT,  "Total track:");
    if (dctotmiles>1000000) {dctotmiles=10000001; strcpy(buff,"lots of miles");}
     else sprintf(buff, "%li miles", dctotmiles);
    WinSetDlgItemText(hwndDist, ID_DCTMILES, buff);
    if (dctotkm>1000000) {dctotkm=10000001; strcpy(buff,"lots of km");}
     else sprintf(buff, "%li km", dctotkm);
    WinSetDlgItemText(hwndDist, ID_DCTKM, buff);

    /* Make the track button default/focus (for easier multitrack)... */
    WinSendMsg(WinWindowFromID(hwndDist, ID_DCTRACK),
               BM_SETDEFAULT, (MPARAM)TRUE, NULL);
    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwndDist, ID_DCTRACK));

    /* OK, now add new track line */
    putmill(clat,dclat[0]); putmill(clon,dclon[0]);
    sprintf(buff,"gmove %s %s track", clat, clon);
    gldo(buff);
    putmill(clat,dclat[1]); putmill(clon,dclon[1]);
    sprintf(buff,"gline %s %s track", clat, clon);
    gldo(buff);
    newdraw();                          /* now do a redraw */
    return MFALSE;}

  case UM_DCRESET: {                    /* Reset points and track request */
    dcfixed=FALSE; dcpoints=0;
    dctotkm=0; dctotmiles=0;
    WinSetDlgItemText(hwndDist, ID_DC1TEXT, "");
    WinSetDlgItemText(hwndDist, ID_DC1LON, "");
    WinSetDlgItemText(hwndDist, ID_DC1LAT, "");
    WinSetDlgItemText(hwndDist, ID_DC2TEXT, "");
    WinSetDlgItemText(hwndDist, ID_DC2LON, "");
    WinSetDlgItemText(hwndDist, ID_DC2LAT, "");
    WinSetDlgItemText(hwndDist, ID_DCDTEXT, "");
    WinSetDlgItemText(hwndDist, ID_DCDKM  , "");
    WinSetDlgItemText(hwndDist, ID_DCDMILES, "");
    WinSetDlgItemText(hwndDist, ID_DCTTEXT, "");
    WinSetDlgItemText(hwndDist, ID_DCTKM  , "");
    WinSetDlgItemText(hwndDist, ID_DCTMILES, "");
    WinEnableControl (hwndDist, ID_DCTRACK, FALSE);
    WinSendMsg(WinWindowFromID(hwndDist, ID_DCCANCEL),
               BM_SETDEFAULT, (MPARAM)TRUE, NULL);
    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwndDist, ID_DCCANCEL));
    PrfWriteProfileString(HINI_USERPROFILE, ininame, KEYFIX, NULL);
    if (trackbase!=NULL) {
      gldo("gerase track");             /* remove any track */
      newdraw();}                       /* .. and do a redraw */
    return MFALSE;}

  case UM_DCCANCEL: {                   /* Cancel (hide) DISTCALC */
    distvis(FALSE);
    return MFALSE;}

  /* Run the TIMEZONE dialogue.  */
  /* MP1 is ID_SUN, ID_SPACE, or ID_STAR if this triggered via these */
  case UM_TIMEZONE: {
    HWND hwndTz;         /* handle */
    char hours[8];       /* hours in character form */
    int  rc;
    unsigned long urc;
    int  hh, mm;
    int  sumhalves;      /* summertime half hours */
    hwndTz=WinLoadDlg(HWND_DESKTOP, hwndFrame, gltimez,
          NULL, (USLONG)ID_TZDLG, NULL);
    WinSendDlgItemMsg(hwndTz, ID_TZHOURS,
          EM_SETTEXTLIMIT, MPFROMSHORT(6), NULL); /* up to 6 chars */
    WinSendDlgItemMsg(hwndTz, ID_TZZONE,
          EM_SETTEXTLIMIT, MPFROMSHORT(4), NULL); /* up to 4 chars */
    WinSetDlgItemText(hwndTz, ID_TZZONE, zonename);

    if (summer==BADLONG) sumhalves=0;
                    else sumhalves=(int)(summer/1800L);
    WinSendDlgItemMsg(hwndTz, (unsigned)(ID_TZDAY0+sumhalves),
          BM_SETCHECK, MPFROMSHORT(1), NULL);     /* initial radio */
    if (gmtoffset==BADLONG) {hh=0; mm=0;}
     else {
      hh=(int)(gmtoffset/3600L);             /* hours.. */
      mm=abs((int)(gmtoffset%3600L))/60;     /* .. minutes */
      }
    sprintf(hours,"%+i:%02i", hh, mm);
    WinSetDlgItemText(hwndTz, ID_TZHOURS, hours);
    /* Tell dialogue all data ready */
    WinSendMsg(hwndTz, UM_CREATED, NULL, NULL);

    urc=WinProcessDlg(hwndTz);          /* wait for completion */
    /* if (diag) printf("TZ URC=%li\n", urc); */

    if (urc==ID_TZOK) {                 /* OK, use the new data */
      char buff[30];
      long oldoff, oldsum;              /* old values */
      long newoff, newsum;              /* new values */
      oldoff=gmtoffset; oldsum=summer;  /* remember oldies */
      WinQueryDlgItemText(hwndTz, ID_TZZONE, sizeof(zonename), zonename);
      if (strlen(zonename)!=3 && strlen(zonename)!=4) strcpy(zonename,"???");
       else strupr(zonename);
      WinQueryDlgItemText(hwndTz, ID_TZHOURS, sizeof(hours), hours);
      newoff=gettime(hours);            /* only get here if OK */
      rc=(int)WinSendDlgItemMsg(hwndTz, ID_TZDAY0,
            BM_QUERYCHECKINDEX, NULL, NULL);     /* test radio */
      newsum=rc*30L;                    /* index is 0, 1, 2.. halves */
      /* NEWSUMmer is now in minutes */
      /* Looks good .. write to OS2SYS.INI... */
      sprintf(buff,"%s %+li:%02li %s", hours, newsum/60L, newsum%60L, zonename);
      if (diag) printf("New timezone: %s\n", buff);
      PrfWriteProfileString(HINI_SYSTEMPROFILE, TZNAME, TZACTIVE, buff);
      /* can't do anything with an error, so ignore */
      newsum=newsum*60L;                /* into seconds, ready for use */
      /* Copy pair or values consistently */
      DosEnterCritSec();
      summer=newsum; gmtoffset=newoff;
      DosExitCritSec();

      /* If message was result of SUN/SPACE/STAR request, set SUN, etc., now */
      if (mp1s1!=0) {
        gldo("set sunlight on");
        if (mp1s1!=ID_SUN)  gldo("set space on");
        if (mp1s1==ID_STAR) gldo("set starlight on");
        }
      if (oldoff!=gmtoffset || oldsum!=summer) newdraw();
      }

    WinDestroyWindow(hwndTz);
    return MFALSE;}

  default: break;
  } /* switch(msg) */

 mres=MDEFAULT;          /* Dropthrough returns default */
 return mres;            /* Intermediate for beta C Set/2 */
 } /* glclient */

/* ------------------------------------------------------------------ */
/* TIMER -- start or stop the fast or slow timer (protected)          */
/* ------------------------------------------------------------------ */
/* First argument is TRUE to start timer, FALSE to stop it            */
/* Second argument is TRUE for fast (mouse) timing, FALSE for slow,   */
/* and applies for stopping, too.  i.e., a STOP of fast timer may     */
/* just slow it down if slow timer was started.                       */
void timer(FLAG flag, FLAG fast)
  {
  unsigned int delay;

  if (flag) {    /* start request */
    if (fast) {                         /* fast timer request */
      if (timefast) return;             /* already running */
      if (timeslow) WinStopTimer(hab, hwndClient, TIMERID);
      delay=DELAYFAST; timefast=TRUE;}  /* fixed fast delay */
     else {                             /* slow request */
      if (timeslow) return;             /* already running */
      timeslow=TRUE;
      if (timefast) return;             /* timer already running fast */
      delay=(unsigned int)
        lmin((refresh+1)*(long)DELAY,10000L); /* variable delay, max 10s */
      }
    }                                   /* drop through to start it */
   else { /* stop request */
    if (fast) {                         /* stop fast please */
      if (!timefast) return;            /* nothing to do */
      timefast=FALSE;                   /* stop the fast timer */
      }
     else {                             /* stop slow please */
      if (!timeslow) return;            /* nothing to do */
      timeslow=FALSE;                   /* stop the fast timer */
      if (timefast) return;             /* fast is running, so no more */
      }
    /* either stopping or slowing */
    WinStopTimer(hab, hwndClient, TIMERID);
    if (!timeslow) {
      if (diag) printf("Stop timer\n");
      return;} /* both stopped */
    delay=(unsigned int)
       lmin((refresh+1)*(long)DELAY,10000L);  /* variable delay, max 10s */
    }

  if (diag) printf("Start timer %i\n", delay);
  WinStartTimer(hab, hwndClient, TIMERID, delay);
  return;
  } /* timer */

/* ------------------------------------------------------------------ */
/* DISTVIS -- make DISTCALC visible or not                            */
/* ------------------------------------------------------------------ */
void distvis(FLAG flag) {
  showdist=flag;
  check(ID_SHOWDC, showdist);
  /* Now activate the right window, etc. */
  if (showdist) {
    WinShowWindow(hwndDist, TRUE);                /* Display window */
    WinSendMsg(WinWindowFromID(hwndDist, ID_DCCANCEL),
               BM_SETDEFAULT, (MPARAM)TRUE, NULL);
    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwndDist, ID_DCCANCEL));
    }
   else {
    WinShowWindow(hwndDist, FALSE);               /* Hide window */
    WinSetActiveWindow(HWND_DESKTOP, hwndClient);
    }
  } /* distvis */

/* ------------------------------------------------------------------ */
/* SHOWLL -- show mouse lat/lon, if changed                           */
/* ------------------------------------------------------------------ */
void showll(long lat, long lon) {
    char buffo[15], buffa[15];
    if (lat==oldlat && lon==oldlon) return;  /* no change */
    ll2str(lon, lat, buffo, buffa);          /* always do both */
    WinSetDlgItemText(hwndDist, ID_DCMLON, buffo);
    WinSetDlgItemText(hwndDist, ID_DCMLAT, buffa);
    oldlat=lat; oldlon=lon;                  /* record done */
    return;} /* showll */

/* ------------------------------------------------------------------ */
/* ONCLIENT -- mouse going over window                                */
/* ------------------------------------------------------------------ */
void onclient(void) {
  if (overclient) return;
  overclient=TRUE;
  timer(START,FAST);                    /* start fast timer */
  return;} /* onclient */

/* ------------------------------------------------------------------ */
/* OFFCLIENT -- mouse is leaving window                               */
/* ------------------------------------------------------------------ */
void offclient(void) {
  if (!overclient) return;
  timer(STOP,FAST);                     /* stop the fast timer */
  overclient=FALSE;
  offglobe();                           /* and must be leaving globe */
  return;} /* offclient */

/* ------------------------------------------------------------------ */
/* ONGLOBE -- mouse is entering globe                                 */
/* ------------------------------------------------------------------ */
void onglobe(void) {
  HPOINTER hptr;
  if (crosshair) hptr=hptrGlobe; else hptr=hptrSystem;
  /* Always set pointer */
  WinSetPointer(HWND_DESKTOP, hptr);    /* use selected pointer */
  if (overglobe) return;
  overglobe=TRUE;
  return;} /* onglobe */

/* ------------------------------------------------------------------ */
/* OFFGLOBE -- mouse is leaving globe                                 */
/* ------------------------------------------------------------------ */
void offglobe(void) {
  /* (Setting pointer is needed, for when globe size changes but */
  /* mouse doesn't move.) */
  WinSetPointer(HWND_DESKTOP, hptrSystem);   /* use system pointer */
  if (!overglobe) return;
  overglobe=FALSE;
  WinSetDlgItemText(hwndDist, ID_DCMLAT, "");/* clear distcalc */
  WinSetDlgItemText(hwndDist, ID_DCMLON, "");
  oldlat=BADLONG; oldlon=BADLONG;            /* force new display */
  } /* offglobe */

/* ------------------------------------------------------------------ */
/* TODESK -- place on top of background                               */
/* ------------------------------------------------------------------ */
void todesk(void)
  {
  /* disable normal interactions while on desktop */
  enable(ID_OPTIONS, FALSE); enable(ID_VIEW,    FALSE);
  enable(ID_LIGHT,   FALSE); enable(ID_HELP,    FALSE);
  if (onclient) offclient();                 /* disable client timer */
  distvis(FALSE);                            /* hide calculator */
  WinShowWindow(hwndComm, FALSE);            /* .. and command dialog */
  /* Now put behind all siblings, and show */
  WinSetWindowPos(hwndFrame, HWND_BOTTOM,    /* posts the WM_SIZE */
    0,0,0,0, SWP_SHOW | SWP_ZORDER | SWP_DEACTIVATE);
  timer(START,FAST);                         /* start timer */
  } /* todesk */

/* ------------------------------------------------------------------ */
/* FROMDESK -- leave background, ensuring visible                     */
/* ------------------------------------------------------------------ */
void fromdesk(void)
  {
  timer(STOP,FAST);                          /* Stop desktop timer */
  if (onclient) offclient();                 /* .. B&B */
  check(ID_DESKTOP, desktop=FALSE);          /* no more */
  /* enable normal interactions again */
  enable(ID_OPTIONS, TRUE); enable(ID_VIEW,    TRUE);
  enable(ID_LIGHT,   TRUE); enable(ID_HELP,    TRUE);
  /* If it's bigger than the screen, Reposition the window -- to */
  /* default PM -- and activate */
  if (clientx>=screenx && clienty>=screeny)
    WinSetWindowPos(hwndFrame, 0,            /* position frame */
      defswp.x, defswp.y, defswp.cx, defswp.cy,
      SWP_SIZE | SWP_MOVE | SWP_ACTIVATE | SWP_SHOW);  /* flags */
   /* otherwise simply pop it up */
   else WinSetActiveWindow(HWND_DESKTOP, hwndFrame);   /* pop up */
  } /* fromdesk */

/* ------------------------------------------------------------------ */
/* DESKCHECK -- check on desk: if not, then pushdown                  */
/* ------------------------------------------------------------------ */
/* Also deactivate, if need be                                        */
void deskcheck(void) {
  HWND hwndtemp;
  FLAG bottom=TRUE;      /* Assume at bottom */

  hwndtemp=WinQueryWindow(hwndFrame, QW_NEXT);    /* what's below? */
  if (hwndtemp!=HWND_DESKTOP
   && hwndtemp!=hwndDesk
   && hwndtemp!=(HWND)0  /* get these even though not doc */
    ) {
    /**
    hwndtemp=WinQueryWindow(hwndFrame,QW_PREV,NULL);
    if (diag) printf("Pushdown: PREV is %li\n", (long)hwndtemp);
    **/
    bottom=FALSE;        /* We're not there */
    }
  if (weactive || !bottom)
    WinSetWindowPos(hwndFrame, HWND_BOTTOM, 0,0,0,0,
                    SWP_ZORDER | SWP_DEACTIVATE);
  } /* deskcheck */

/* ------------------------------------------------------------------ */
/* MAKEFULL -- make desktop sized                                     */
/* ------------------------------------------------------------------ */
void makefull(void)
  {
  RECTL rcl;
  distvis(FALSE);                            /* hide calculator */
  WinQueryWindowRect(HWND_DESKTOP,&rcl);     /* Get desktop size */
  /* If that is to be client, then frame is... */
  WinCalcFrameRect(hwndFrame,&rcl,FALSE);    /* Get new window coords */
  /* Now set the new window position and size .. full screen */
  WinSetWindowPos(hwndFrame, NULL,           /* posts the WM_SIZE */
    (short)rcl.xLeft,
    (short)rcl.yBottom,
    (short)rcl.xRight-(short)rcl.xLeft,
    (short)rcl.yTop-(short)rcl.yBottom,
    SWP_SIZE | SWP_MOVE | SWP_SHOW);         /* flags */
  WinSendMsg(hwndFrame, WM_SETICON, MPFROMP(hptrIcon), MPVOID);
  } /* makefull */

/* ------------------------------------------------------------------ */
/* MAKEQUART -- make quarter-desktop sized                            */
/* ------------------------------------------------------------------ */
void makequart(void)
  {
  RECTL rcl; long x, y;
  distvis(FALSE);                            /* hide calculator */
  WinQueryWindowRect(HWND_DESKTOP,&rcl);     /* Get desktop size */
  /* calculate centre quarter adjustments, and adjust rectangle */
  x=(rcl.xRight-rcl.xLeft)/4; y=(rcl.yTop-rcl.yBottom)/4;
  rcl.xLeft=rcl.xLeft+x; rcl.xRight=rcl.xRight-x;
  rcl.yBottom=rcl.yBottom+y; rcl.yTop=rcl.yTop-y;
  /* If that is to be client, then frame is... */
  WinCalcFrameRect(hwndFrame,&rcl,FALSE);    /* Get new window coords */
  /* Now set the new window position and size .. full screen */
  WinSetWindowPos(hwndFrame, NULL,           /* posts the WM_SIZE */
    (short)rcl.xLeft,
    (short)rcl.yBottom,
    (short)rcl.xRight-(short)rcl.xLeft,
    (short)rcl.yTop-(short)rcl.yBottom,
    SWP_SIZE | SWP_MOVE | SWP_SHOW);         /* flags */
  } /* makequart */

/* ------------------------------------------------------------------ */
/* SETFILEICON -- filter for WinFileDlg to set icon                   */
/* ------------------------------------------------------------------ */
MRESULT APIENTRY setfileicon(HWND hwnd, USLONG msg, MPARAM mp1, MPARAM mp2){
   if (msg==WM_INITDLG) {          /* startup */
     WinSendMsg(hwnd,  WM_SETICON, MPFROMP(hptrIcon), MPVOID);
     /* if (diag) printf("Setfileicon Init %lxx %lxx\n"); */
     }
   return WinDefFileDlgProc(hwnd, msg, mp1, mp2);
   } /* setfileicon */

static char c[]=" Copyright (c) IBM Corporation, 1991, 1992, 1993 -- mfc ";

/* ------------------------------------------------------------------ */
/* NEWDRAW -- start a new drawing cycle                               */
/* ------------------------------------------------------------------ */
void newdraw(void) {
  stopdraw=TRUE;              /* halt any in progress */
  timer(STOP,SLOW);           /* cancel any slow timing in progress */
  drawing=TRUE;               /* indicate incomplete draw in progress */
  /* Ensure drawing thread high for rapid response to STOPDRAW request */
  DosSetPrty(PRTYS_THREAD, PRTYC_REGULAR, +1, tidg);   /* boost glober */
  DosPostEventSem(semneeddraw);                  /* let GLOBER proceed */
  /* (Glober does atomic copy of critical data) */
  return;} /* newdraw */
