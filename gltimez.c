/* ------------------------------------------------------------------ */
/* PMGlobe -- a geographical application                              */
/* GLTIMEZ -- WinProc for the TimeZone dialogue                       */
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

#include "globe.h"

/* Subroutines */
/* (none) */

/* Time zone data: TZITEMS of time zones */
/* (Later: Include from a constructed file) */
#define TZITEMS  17
static char tzones[TZITEMS][5]={  /* ten on a line */
  "ADT", "AST", "BST", "CDT","CEST", "CET", "CST", "EDT", "EST", "GMT",
  "HKT", "HST", "JST", "MDT", "MST", "PDT", "PST"};
static int  tzoffs[TZITEMS]={     /* January offsets (minutes) */
  -9*60, -9*60,     0, -6*60,  1*60,  1*60, -6*60, -5*60, -5*60,     0,
  +8*60,-10*60, +9*60, -7*60, -7*60, -8*60, -8*60};
static int  tzsums[TZITEMS]={     /* summertimes (minutes) */
     60,     0,    60,    60,    60,     0,     0,    60,     0,     0,
      0,     0,     0,    60,     0,    60,     0};

/*====================================================================*/
/* The TimeZone Window Procedure                                      */
/*====================================================================*/
MRESULT EXPENTRY _loadds gltimez(
        HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2) {
 static long seloffs;         /* selected/initial offset in secs */
 static int  selsums;         /* selected/initial summertime in mins */
 static char selzone[5];      /* selected zone name */
 char   hours[8];             /* work buffer */
 MRESULT res;

 switch(msg)
  {

  case WM_CONTROL: {   /* Checkboxes etc. */
    int item;
    int hh, mm;
    if (mp1s1==ID_TZLIST &&
      (mp1s2==LN_SELECT ||
       mp1s2==LN_ENTER)) {         /* listbox select or ENTER */
      item=(int)WinSendDlgItemMsg(hwnd, ID_TZLIST,
            LM_QUERYSELECTION, NULL, NULL);       /* what did we get? */
      seloffs=tzoffs[item]*60L;                   /* save for checks */
      selsums=tzsums[item];                       /* .. */
      strcpy(selzone,tzones[item]);               /* .. */
      WinSendDlgItemMsg(hwnd, (unsigned)ID_TZDAY0+(tzsums[item]/30),
          (unsigned)BM_SETCHECK, MPFROMSHORT(1), NULL);     /* new radio */
      hh=tzoffs[item]/60;          /* hours.. */
      mm=tzoffs[item]%60;          /* .. minutes */
      sprintf(hours,"%+i:%02i", hh, mm);
      WinSetDlgItemText(hwnd, ID_TZHOURS, hours);
      WinSetDlgItemText(hwnd, ID_TZZONE,  tzones[item]);
      if (mp1s2==LN_ENTER) {       /* listbox ENTER -- press OK, too */
        WinPostMsg(hwnd,WM_COMMAND,MPFROMSHORT(ID_TZOK),NULL);
        }
      } /* list box clicked */
    return MFALSE;}

  case WM_COMMAND: {   /* Asynchronous controls (Buttons+Menus) */
    long result;
    char buff[256];
    if (mp1s1==ID_TZCANCEL) WinDismissDlg(hwnd, mp1s1);
     else {
      if (diag) printf("TZDlg, got: %i\n", mp1s1);
      /* Must be "OK".. */
      WinQueryDlgItemText(hwnd, ID_TZHOURS, sizeof(hours), hours);
      result=gettime(hours);
      if (result==BADLONG) {
        sprintf(buff,"'%s' is not a valid time offset.\n\n\
Please try another: it can be as simple as '-7' for seven hours west \
of Greenwich.  (Or select Cancel if you do not want to change the \
time zone.)", hours);
        if (WinMessageBox(HWND_DESKTOP, hwnd,
             buff, NULL,  /* title='error' */
             0, MB_DEFBUTTON1 | MB_RETRYCANCEL | MB_ICONEXCLAMATION
             | MB_MOVEABLE)
           == MBID_CANCEL) WinDismissDlg(hwnd, ID_TZCANCEL);
        /* here if retry -- ensure focus is on entryfield (B&B) */
        WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, ID_TZHOURS));
        }
       else {              /* looking good .. see if ZONE name is valid */
        int radio;         /* current radio button selected */
        char name[4];      /* current zone name */
        radio=(int)WinSendDlgItemMsg(hwnd, ID_TZDAY0,
                   BM_QUERYCHECKINDEX, NULL, NULL);
        WinQueryDlgItemText(hwnd, ID_TZZONE, sizeof(name), name);
        /* Invalidate name if same as selected, but offset or */
        /* daylight have been changed */
        if (streq(name,selzone)) {
          if (result!=seloffs                /* hours changed */
           || radio !=(selsums/30))          /* radio button changed */
             WinSetDlgItemText(hwnd, ID_TZZONE, "???");    /* invalidate */
          } /* zone match */
        WinDismissDlg(hwnd, ID_TZOK);
        }
      } /* OK */
    return MFALSE;}

  case UM_CREATED: {     /* Data ready. */
    /* record the initial settings */
    WinQueryDlgItemText(hwnd, ID_TZHOURS, sizeof(hours), hours);
    seloffs=gettime(hours);        /* (Will be valid) */
    selsums=(int)WinSendDlgItemMsg(hwnd, ID_TZDAY0,
      BM_QUERYCHECKINDEX, NULL, NULL)*30;
    break;}

  case WM_INITDLG: {     /* Initialize. */
    int i;

    /* Put icon into the frame */
    WinSendMsg(hwnd,  WM_SETICON, MPFROMP(hptrIcon), MPVOID);

    /* Add the time zones to the list box */
    for (i=0; i<TZITEMS; i++) {
      WinSendDlgItemMsg(hwnd, ID_TZLIST,
        LM_INSERTITEM, MPFROMSHORT(i), MPFROMP(tzones[i]));
      } /* i */
    break;}

  default:
    break;
  }
 res=WinDefDlgProc(hwnd, msg, mp1,mp2); return res;
 }  /* gltimez */
