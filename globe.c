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
/* Command syntax:                                                    */
/*                                                                    */
/*   [PM]GLOBE [options]                                              */
/*                                                                    */
/* Where options are variable, currently may be (in any order):       */
/*                                                                    */
/*   MACRO nnn  -- run macro on startup                               */
/*   NAME  xxx  -- name (stem: PMGlobe.) to be used for .INI file     */
/*   DESKTOP    -- force start on desktop                             */
/*   FULLSCREEN -- force start full-screen                            */
/*   DIAG       -- turn on diagnostic messages (seen with PMPRINTF)   */
/*   DIAGMSG    -- add client message tracing                         */
/*   TEST       -- special tests enable                               */

#include <time.h>
#define  INCL_GPI
#define  INCL_DOSPROCESS
#define  INCL_DOSSEMAPHORES
#include "globe.h"
#if defined(B16)
#include <process.h>
#endif

/* Static application (e.g., geographical) globals */
 char title[MAXTITLE+1]=NAME;      /* window title */
 char ininame[MAXININAME+1];       /* name to use in .INI file */

 FLAG sunlight;                    /* TRUE if sun-lighting */
 FLAG spacelight;                  /* TRUE if space colours (SUN=TRUE) */
 FLAG starlight;                   /* TRUE if star-lighting (SUN, SPACE=TRUE) */
 long twilight;                    /* Degrees (*1000) of twilight */
 long landcoli;                    /* Land colour index */
 long seacoli;                     /* Sea colour index */
 FLAG locklat=FALSE;               /* Lock to sun latitude (if SUN) */
 FLAG locklon=FALSE;               /* Lock to sun longitude (if SUN) */
 FLAG mousepos;                    /* allow position by mouse */
 FLAG threed;                      /* TRUE if 3-D shading */
 FLAG showdist;                    /* set if DISTCALC visible */
 FLAG gridlon10, gridlat10;        /* grid settings */
 FLAG gridlon15, gridlat15;        /* .. */
 FLAG gridlon30, gridlat30;        /* .. */
 FLAG gridarctic, gridtropic;      /* .. */
 FLAG grideq;                      /* .. */
 long gridcoli;                    /* .. and colour index */

 long viewlon=0, viewlat=0;        /* long/lat of view centre */
 long viewrot=0;                   /* rotation of view */
 long useroffset=0;                /* user time offset in seconds */

 long gmtoffset=BADLONG;           /* GMT offset in seconds (-=West) */
 long summer   =BADLONG;           /* Summertime seconds to add */
 char zonename[4+1]="???";         /* Timezone name */

 FLAG weactive=FALSE;              /* globe is active window */
 FLAG desktop=FALSE;               /* act as desktop */
 FLAG fullforce=FALSE;             /* force fullscreen */
 FLAG deskforce=FALSE;             /* force desktop */
 long backcoli;                    /* background colour index */
 long refresh=300;                 /* refresh interval, in seconds */

 int  diameter=0;                  /* globe diameter (pels, 0 or odd) */
 long margin=0;                    /* % (*1000) of margin around globe */
 long clientx=0,  clienty=0;       /* client window size */
 long screenx=0,  screeny=0;       /* screen size (pels) */
 long fontresx=0, fontresy=0;      /* screen resolution for fonts (ppi) */

 /* drawing, label, font, and mark subsystem */
 long          drawcoli;           /* current drawing colour index */
 long          trackcoli;          /* current tracking colour index */
 FLAG          graphics=TRUE;      /* show graphics */
 FLAG          markers=TRUE;       /* show markers */
 FLAG          labels=TRUE;        /* show labels */
 FLAG          clocks=TRUE;        /* show clocks */
 FLAG          clockday=TRUE;      /* show clocks with +/- day flag */
 FLAG          civiltime=FALSE;    /* show clocks as 12-hour */
 char          curmark='+';        /* current mark symbol (+ x X blank) */
 long          curfont=0;          /* current font number */
 long          marks=0;            /* count of marks */
 long          clockmarks=0;       /* count of marks with clocks */
 long          fonts=0;            /* count of fonts */
 long          draws=0;            /* count of drawing (graphics) items */
 long          tracks=0;           /* count of tracking (graphics) items */
 struct llmark *markbase;          /* base of mark chain */
 struct logfont *fontbase;         /* base of font chain */
 struct drawlist *drawbase=NULL;   /* base of drawing display list */
 struct drawlist *trackbase;       /* base of tracking display list */

 long   trackcoli;                 /* current tracking colour index */
 long   tracks;                    /* count of tracking (graphics) items */

 char commshare[MAXCOMM+1]="";     /* shared command string */

 FLAG test=FALSE;                  /* TEST flag */
 FLAG diag=FALSE;                  /* DIAG flag */
 FLAG diagmsg=FALSE;               /* show PM Client Messages */
 FLAG errorbox=TRUE;               /* show errorboxes */
 FLAG idledraw=TRUE;               /* draw at IDLE priority */
 FLAG showdraw=TRUE;               /* show drawing in progress */
 FLAG crosshair=TRUE;              /* use crosshair cursor */
 FLAG menubar=TRUE;                /* TRUE if menu bar in use */
 FLAG titlebar=TRUE;               /* TRUE if title bar in use */
 FLAG redrawall=TRUE;              /* redraw map and features */
 FLAG redrawsprite=FALSE;          /* redraw all features */
 FLAG drawing=FALSE;               /* drawing in progress */
 FLAG commhalt=FALSE;              /* halt any command or macro */
 FLAG alerted=FALSE;               /* alert system in use */
 FLAG rexxhalts[MAXREXX];          /* halt flags (per level) */

/* Shared Handles... */
 HWND hwndFrame   =NULL;
 HWND hwndClient  =NULL;
 HWND hwndMenu    =NULL;
 HWND hwndMParent =NULL;
 HWND hwndDist    =NULL;
 HWND hwndComm    =NULL;
 HWND hwndPop     =NULL;
 HPOINTER hptrIcon=NULL;
 HAB  hab;
 HPS  cps;                         /* current picture space */
 TID  tidg, tidm;                  /* glober, macros thread ids */

/* Global semaphores and control flags -- see GLOBER.C for details */
 HEV sembitmap    =0;              /* Mutex: protect bitmap */
 HEV semhavemap   =0;              /* Land map is ready */
 HEV semhavescreen=0;              /* Screen is available */
 HEV semneeddraw  =0;              /* New globe draw is needed */
 HEV semhavecomm  =0;              /* Command runner is available */
 HEV semruncomm   =0;              /* Run command please */
 FLAG stopdraw    =0;              /* Interrupt drawing */

/* Subroutine and stack declarations */
 int     cdecl    main(int, char **);
 MRESULT EXPENTRY glclient(HWND, USLONG, MPARAM, MPARAM);
 MRESULT EXPENTRY gldist  (HWND, USLONG, MPARAM, MPARAM);
 MRESULT EXPENTRY glcomm  (HWND, USLONG, MPARAM, MPARAM);
 extern  void     glober(void *);       /* the globe thread */
 extern  void     globec(void *);       /* the command thread */
 extern  void     dropbitmaps(void);    /* drop any old bitmaps */
 #if defined(B16)
 static  CHAR     gstack[16300];        /* stack for globe thread */
 static  CHAR     mstack[4096];         /* stack for macro thread */
 #endif

/*====================================================================*/
/* The 'main program' (starter)                                       */
/*====================================================================*/
int cdecl main(int argc, char **argv)
 {
 char **parm;                 /* parameter character/string */
 HMQ hmq;
 QMSG qmsg;
 int i, j, k;
 ULONG flCreateFlags= FCF_TITLEBAR | FCF_SYSMENU | FCF_SIZEBORDER
  | FCF_MINMAX | FCF_SHELLPOSITION | FCF_ACCELTABLE | FCF_MENU;
  /*| FCF_TASKLIST;*/
 static CHAR szClientClass[]="Client Window";

 /* Global initials not done above */
 markbase=NULL;                    /* no marks yet */
 fontbase=NULL;                    /* no fonts yet */
 drawcoli =CLR_WHITE;              /* adjustable colours */
 trackcoli=CLR_DARKCYAN;           /* .. */
 gridcoli =BADLONG;                /* (set in GLCLIENT) */
 backcoli =BADLONG;                /* .. */
 strcpy(ininame, NAME);            /* Default .INI name */

 /* ----- Check the arguments and save startup macro name ----- */
 for (i=0, parm=argv; i<argc ; parm++, i++) {
   if (i==0) continue;        /* own name */
   printf("%s: '%s' requested", NAME, *parm);
   if      (streq(*parm,"MACRO"))  {
     parm++; i++;
     if (i>=argc) printf(" -- without name");
      else {   /* set up initial 'macro' command */
       /* use a loop rather than STRCPY in case too long */
       j=(signed)strlen(*parm); if (j>MAXCOMM-10) j=MAXCOMM-10;
       strcpy(commshare,"macro ");
       for (k=0; k<j; k++) commshare[k+6]=(*parm)[k];
       commshare[k+6]='\0';
       addext(&(commshare[6]),MACEXT);
       printf(" -- file '%s'", &(commshare[6]));
       }
     } /* 'macro' */
    else if (streq(*parm,"NAME"))  {
     parm++; i++;
     if (i>=argc) printf(" -- without name");
      else {   /* set up name string */
       int maxtail=MAXININAME-sizeof(NAME);
       /* use a loop rather than STRCPY in case too long */
       j=(signed)strlen(*parm); if (j>maxtail) j=maxtail;
       ininame[sizeof(NAME)-1]='.';     /* start the tail */
       for (k=0; k<j; k++) ininame[k+sizeof(NAME)]=(*parm)[k];
       ininame[k+sizeof(NAME)]='\0';  /* terminate */
       printf(" -- as '%s'", ininame);
       }
     } /* 'name' */
   else if (streq(*parm,"DESKTOP"))    deskforce=TRUE;
   else if (streq(*parm,"FULLSCREEN")) fullforce=TRUE;
   else if (streq(*parm,"DIAG"))       diag=TRUE;
   else if (streq(*parm,"DIAGMSG"))    diagmsg=TRUE;
   else if (streq(*parm,"TEST"))       test=TRUE;
   else printf(" -- ignored");
   printf("\n");
   }

 /* ----- Initialize PM and interlocks ----- */
 hab=WinInitialize(NULL);
 hmq=WinCreateMsgQueue(hab,0);

 /* Create and initialize semaphores */
 DosCreateEventSem(NULL, &semhavemap,    0L, FALSE); /* FALSE=blocked */
 DosCreateEventSem(NULL, &semhavescreen, 0L, FALSE); /* FALSE=blocked */
 DosCreateEventSem(NULL, &semneeddraw,   0L, FALSE); /* FALSE=blocked */
 DosCreateEventSem(NULL, &semhavecomm,   0L, FALSE); /* FALSE=blocked */
 DosCreateEventSem(NULL, &semruncomm,    0L, FALSE); /* FALSE=blocked */
 DosCreateMutexSem(NULL, &sembitmap,     0L, FALSE); /* FALSE=unowned */

 /* ----- Start up the globe constructor and macro threads ----- */
 #if defined(B16)
 tidg=_beginthread((void far *)glober, (void far *)gstack, sizeof(gstack), NULL);
 tidm=_beginthread((void far *)globec, (void far *)mstack, sizeof(mstack), NULL);
 #else
 tidg=(TID)_beginthread(glober, NULL, 4096U*8U, NULL);
 tidm=(TID)_beginthread(globec, NULL, 4096U,    NULL);
 #endif

 if (diag) WinAlarm(HWND_DESKTOP, WA_NOTE);

 WinRegisterClass(hab, szClientClass, glclient, CS_SIZEREDRAW, 0);

 /* Test commands, etc. */
 /* if (test) { } */

 /* ----- Create the hidden modeless dialogue ----- */
 /* (Must be there before main) */
 /*hwndDist=WinLoadDlg(HWND_DESKTOP, hwndFrame,     */
 hwndDist=WinLoadDlg(HWND_DESKTOP, HWND_OBJECT,
          gldist, NULL, ID_DISTCALC, NULL);
 hwndComm=WinLoadDlg(HWND_DESKTOP, HWND_OBJECT,
          glcomm, NULL, ID_COMMDLG, NULL);

 /* Ensure threads are ready before continuing */
 DosWaitEventSem(semhavemap,  SEM_INDEFINITE_WAIT);
 DosWaitEventSem(semhavecomm, SEM_INDEFINITE_WAIT);

 /* ----- Start the main window ----- */
 hwndFrame=WinCreateStdWindow(
   HWND_DESKTOP, 0L, &flCreateFlags,
   szClientClass, "",
   0L, NULL, ID_FRAMERC, &hwndClient);

 while (WinGetMsg(hab, &qmsg, NULL, 0, 0)) WinDispatchMsg(hab,&qmsg);

 if (diag) WinAlarm(HWND_DESKTOP, WA_NOTE);

 WinDestroyWindow(hwndClient);
 WinDestroyWindow(hwndFrame);
 WinDestroyWindow(hwndDist);
 WinDestroyMsgQueue(hmq);
 dropbitmaps();                  /* Bitmap resources */
 WinTerminate(hab);
 return 0;
 } /* main */

