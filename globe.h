/* PMGlobe -- a geographical application -- primary Include file      */
#define NAME     "PMGlobe"         /* Our name -- also used for .INI  */
#define VERSION  "2.19"            /* >> This must be on line 3 <<    */
#define MACEXT   "pmg"             /* Extension for macros            */
/* ------------------------------------------------------------------ */
/* Copyright (c) Mike Cowlishaw, 1993-2019.  All rights reserved.     */
/* Parts Copyright (c) IBM, 1993-2009.                                */
/* ------------------------------------------------------------------ */

/* Standard includes that all need */
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INCL_WINERRORS
#define INCL_WINTIMER

#if !defined(EXCL_WIN)
#define INCL_WINTYPES
#define INCL_WINWINDOWMGR
#define INCL_WINDIALOGS
#define INCL_WINBUTTONS
#define INCL_WINMENUS
#define INCL_WINLISTBOXES
#define INCL_WININPUT
#define INCL_WINENTRYFIELDS
#define INCL_WINSHELLDATA
#define INCL_WINFRAMEMGR
#define INCL_WINSWITCHLIST
#define INCL_WINSYS
#define INCL_WINMESSAGEMGR
#endif

#include <os2.h>
#include "globepm.h"
#include "dualpath.h"

/* Useful constants */
/* Maximum integer values are reserved for error indication */
/* (0x80000000L is not used as C Set/2 complains) */
#define BADLONG   (signed long)0x80000000L   /* "bad" long int value (error) */
#define MINLONG   (signed long)0x80000001L   /* smallest long int            */
#define MAXLONG   (signed long)0x7FFFFFFFL   /* largest long int             */
#define BADSHORT  (signed short)0x8000       /* "bad" short int value (error)*/
#define MINSHORT  (signed short)0x8001       /* smallest short int           */
#define MAXSHORT  (signed short)0x7FFF       /* largest short int            */
#define MAXTITLE  50               /* longest title text */
#define MAXTOK    50               /* longest token */
#define MAXNAME   10               /* longest font name */
#define MAXREXX   10               /* maximum macro depth */
#define MAXTEXT   100              /* longest text (label) */
#define MAXCOMM   200              /* longest command */
#define MAXFILE   250              /* longest file/path */

/* General types */
typedef unsigned char FLAG;        /* Single flag */

/* Global Settings */
extern char title[MAXTITLE+1];     /* window title */
extern int  diameter;              /* globe diameter (pels, 0 or odd) */
extern long margin;                /* % (*1000) of margin around globe */
extern FLAG sunlight;              /* TRUE if sun-lighting */
extern FLAG spacelight;            /* TRUE if space-lighting (SUN=TRUE) */
extern FLAG starlight;             /* TRUE if star-lighting (SUN, SPACE=TRUE) */
extern long twilight;              /* Degrees (*1000) of twilight */
extern long landcoli;              /* Land colour index */
extern long seacoli;               /* Sea colour index */
extern FLAG mousepos;              /* TRUE if MB2 enabled */
extern FLAG threed;                /* TRUE if 3-D requested */
extern FLAG showdist;              /* TRUE if DistCalc visible */
extern FLAG idledraw;              /* TRUE to draw at IDLE priority */
extern FLAG crosshair;             /* TRUE to use crosshair cursor */
extern FLAG menubar;               /* TRUE if menu bar in use */
extern FLAG titlebar;              /* TRUE if title bar in use */
extern FLAG redrawall;             /* Redraw map and features */
extern FLAG redrawsprite;          /* Redraw all features */
extern FLAG showdraw;              /* Show drawing in progress */
extern FLAG drawing;               /* Drawing in progress */
extern FLAG locklat;               /* Lock to sun latitude (if SUN) */
extern FLAG locklon;               /* Lock to sun longitude (if SUN) */
                                   /* (nyi) */

extern FLAG gridlon10, gridlat10;  /* grid settings */
extern FLAG gridlon15, gridlat15;  /* .. */
extern FLAG gridlon30, gridlat30;  /* .. */
extern FLAG gridarctic, gridtropic;/* .. */
extern FLAG grideq;                /* .. */
extern long gridcoli;              /* colour index */

extern long gmtoffset;             /* GMT offset in seconds (-=West) */
extern long summer;                /* Summertime seconds to add */
extern char zonename[4+1];         /* Time zone name (3 or 4 characters) */
extern long viewlon, viewlat;      /* long/lat of view centre */
extern long viewrot;               /* rotation of view */
extern FLAG desktop;               /* now on desktop */
extern FLAG deskforce;             /* force desktop */
extern FLAG fullforce;             /* force fullscreen */
extern long backcoli;              /* background colour index */
extern long useroffset;            /* user time offset in seconds */
extern long refresh;               /* refresh time in seconds */

/* Graphics, mark, font, and text items */
extern long   drawcoli;            /* current drawing colour index */
extern struct drawlist *drawbase;  /* base of drawing (display) list */
extern long   draws;               /* count of drawing (graphics) items */
extern long   trackcoli;           /* current tracking colour index */
extern struct drawlist *trackbase; /* base of tracking (display) list */
extern long   tracks;              /* count of tracking (graphics) items */

/* Drawing (display) list item structure and orders */
/* Used for both tracks and user-draws */
struct drawlist {
  struct drawlist *next;           /* -> next */
  int  order;                      /* graphics order */
  long lon; long lat;              /* point definition (if used) */
  long coli;                       /* colour index (if used) */
  FLAG drawn;                      /* TRUE once drawn (not used for Tracks) */
  };
#define DRAW_MOVE 1                /* orders */
#define DRAW_LINE 2

/* Logical Font structure */
struct logfont {
  struct logfont *next;            /* -> next */
  long   flcid;                    /* logical PM id of this font */
  long   fsize;                    /* requested size of this font */
  long   fcoli;                    /* colour index for this font */
  char   face[FACESIZE];           /* font face name (no +1) */
  char   fname[MAXNAME+1];         /* font nickname */
  };

/* Lat/Long mark structure */
struct llmark {
  struct llmark *next;             /* -> next */
  long   lat;                      /* Latitude*1000 */
  long   lon;                      /* Longitude*1000 */
  long   tfont;                    /* Font id for the text */
  long   mcoli;                    /* Mark colour index */
  long   clock;                    /* Clock offset (BADLONG if none) */
  short  tx;                       /* Text offset (BADSHORT for centre) */
  short  ty;                       /* .. */
  FLAG   drawn;                    /* TRUE once drawn */
  char   mark;                     /* Mark symbol (may be blank) */
  char   text[1];                  /* Text starts here (1 allows for the */
  };                               /*   terminator) */

extern FLAG   graphics;            /* show graphics */
extern FLAG   markers;             /* show markers */
extern FLAG   labels;              /* show labels */
extern FLAG   clocks;              /* show clocks */
extern FLAG   clockday;            /* show clocks with +/- day flag */
extern FLAG   civiltime;           /* show clocks as 12-hour */
extern char   curmark;             /* current mark symbol (+ x X blank) */
extern long   curfont;             /* current font number */
extern long   marks;               /* count of marks */
extern long   clockmarks;          /* count of marks with clocks */
extern struct llmark *markbase;    /* base of mark chain */
extern long   fonts;               /* count of fonts */
extern struct logfont *fontbase;   /* base of font chain */

extern char commshare[MAXCOMM+1];  /* shared command string */

/* Profile (.INI) settings and savings */
#define KEYSET   "Settings"        /* Our settings in OS2.INI */
#define KEYWIN   "WinState"        /* Window state */
#define KEYPOS   "Position"        /* Restored window size/position */
#define KEYFIX   "Fixed"           /* Fixed point (if any) */
#define TZNAME   "TimeZone"        /* Groupname for OS2SYS.INI */
#define TZACTIVE "Active"          /* Active timezone data OS2SYS.INI */
/* 'Active' holds at least three words separated by exactly one       */
/* blank, and with no leading blanks.                                 */
/*    base-time-offset-from-GMT  winter/summer-time  zone-name        */
/* where the times are [+|-][h]h[:[m]m[:[s]s]]] and zone-name is      */
/* either "???" or "xxx" where xxx are three uppercase alphabetics.   */
/* The total offset from GMT is the sum of the two time offsets.      */
/* Therefore, up to 23 total data characters in the three words,      */
/* including the two separator blanks.                                */
#define TZACTIVELEN 23

/* Settings saver data and constants */
#define MAXININAME (sizeof(NAME)+25)  /* maximum name: PMGlobe.[25] */
#define MAXSETLEN 200              /* maximum setstring length */
#define MAXPOSLEN 60               /* maximum posstring length */
#define MAXWINLEN 7                /* maximum WinState length */
extern char ininame[MAXININAME+1]; /* name to use in .INI file */
extern char initset[MAXSETLEN+1];  /* saves startup conditions */
extern char defset[MAXSETLEN+1];   /* default startup conditions */
extern char initpos[MAXPOSLEN+1];  /* saves startup posstring */
extern char initwin[MAXWINLEN+1];  /* saves startup WinState */
extern SWP  initswp;               /* initial frame size/pos */
extern int  dcposx, dcposy;        /* initial distcalc box position */
extern int  cmposx, cmposy;        /* initial command box position */

/* Global semaphores and control flags -- see GLOBER.C for details */
extern HEV semhavemap     ;        /* Land map is ready */
extern HEV semhavescreen  ;        /* Screen is available */
extern HEV semneeddraw    ;        /* New globe draw is needed */
extern HEV semhavecomm    ;        /* Command runner is available */
extern HEV semruncomm     ;        /* Run command please */
extern FLAG stopdraw      ;        /* Interrupt drawing */

/* Other Shared Variables */
extern HWND hwndFrame;             /* Window/object handles */
extern HWND hwndClient;
extern HWND hwndMenu;
extern HWND hwndMParent;
extern HWND hwndDist;
extern HWND hwndComm;
extern HWND hwndPop;
extern HPOINTER hptrIcon;
extern HAB  hab;
extern HPS  cps;                   /* current picture space */
extern TID  tidg, tidm;            /* thread ids */
extern long clientx, clienty;      /* window size */
extern long screenx, screeny;      /* screen size (pels) */
extern long fontresx, fontresy;    /* screen resolution for fonts (ppi) */
extern FLAG diag;                  /* DIAG verbose flag */
extern FLAG diagmsg;               /* DIAG message trace flag */
extern FLAG test;                  /* TEST specials */
extern FLAG errorbox;              /* show errorboxes */
extern FLAG weactive;              /* globe is active window */
extern FLAG alerted;               /* alert in progress */
extern FLAG commhalt;              /* halt any command or macro */
extern FLAG rexxhalts[MAXREXX];    /* halt flags (per level) */

/* Globe conversions */
extern int pel2ll(long, long, long *, long *);  /* Pel -> Long/Lat */
extern int ll2pel(long, long, int,              /* Long/Lat -> Pel */
                  long *, long *, long *);

/* Landmap dimensions (fixed) -- currently five points per degree */
#define LANDMAPDIMX 1800
#define LANDMAPDIMY 900

/* Maximum radius that we can handle safely without overflow, etc. */
#define MAXRADIUS  511
/* Maximum number of colours that we can make use of */
#define MAXCOLOURS 256

/* Colour tables */
#define COLHI 127
#define COLK  0
#define COLB  1
#define COLG  2
#define COLC  3
#define COLR  4
#define COLM  5
#define COLY  6
#define COLW  7
typedef struct _COLSTRUCT {   /* Colour tables structure */
  int pure[COLW+1];           /* Pure colours: 0-COLW */
  int rnear[COLHI+1];         /* Nearest index for Reds */
  int gnear[COLHI+1];         /* .. Blues */
  int bnear[COLHI+1];         /* .. Greens */
  int ynear[COLHI+1];         /* .. Yellows */
  int pnear[COLHI+1];         /* .. Pinks */
  int cnear[COLHI+1];         /* .. Cyans */
  int wnear[COLHI+1];         /* .. Whites to blacks */
  int rsteps;                 /* Distinct colours in Red.. */
  int gsteps;                 /* (Green).. these counts do */
  int bsteps;                 /* (Blue)..  not include the */
  int ysteps;                 /* (Yellow).. 'colour' black */
  int psteps;                 /* (Pink)..                  */
  int csteps;                 /* (Cyan)..                  */
  int wsteps;                 /* (White)..                 */
  int colours;                /* Total colours available   */
  } COLSTRUCT;
typedef COLSTRUCT *PCOLSTRUCT;/* pointer to same */

/* Utilities, constants for utilities, and common routines */
#define CHECKLAT  -90000L
#define CHECKLON -180000L
extern int  errbox(const unsigned char *, ...);   /* Error in command */
extern int  addext(char *, char *);          /* add file extension */
extern int  streq(char *, char *);           /* string compare */
extern int  gldo(char *);                    /* execute command          */
extern long getcol (char *, int);            /* string to colour index   */
extern long getdeg (unsigned char *, long);  /* string degrees to long   */
extern long getlong(char *);                 /* like ATOL, with checking */
extern long getmill(char *);                 /* get decimal to int*1000  */
extern void putcol (char *, long);           /* colour index to string   */
extern void putmill(char *, long);           /* int*1000 to decimal str  */
extern long gettime(char *);                 /* get time to +/- seconds  */
extern void getword(char *, char *);         /* get first word           */
extern long lcos(long);                      /* Cosine(d*1000) */
extern long lsin(long);                      /* Sine(d*1000) */
extern long lacos(long);                     /* ArcCosine(x*1000) */
extern long lsqrt(long);                     /* Sqrt(x) */
extern int  fastrand(int);                   /* Random number */

/* Useful macros (these in fact work on both Longs and Ints) */
#define lmax(x,y)  (long)((x)>(y) ?    (x) : (y))
#define lmin(x,y)  (long)((x)<(y) ?    (x) : (y))
#define lsign(x)   (long)((x)>0   ?     1  : ((x)<0 ? -1 : 0))
#if defined(B16)
/* labs() is built-in to C Set/2 */
#define labs(x)    (long)((x)<0   ? (-(x)) : (x))
#endif

