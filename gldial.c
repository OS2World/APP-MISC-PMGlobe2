/* ------------------------------------------------------------------ */
/* PMGlobe -- a geographical application                              */
/* GLDIAL  -- WinProcs for dialogue boxes:                            */
/*   GLDIST  -- WinProc for the Distance Calculator dialogue          */
/*   GLCOMM  -- WinProc for the command dialogue                      */
/*   ERRBOX  -- Error reporting for commands                          */
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

#include <stdarg.h>
#include "globe.h"

/* Subroutines */

/*====================================================================*/
/* The Distance Calculator Window Procedure                           */
/*====================================================================*/
MRESULT EXPENTRY _loadds gldist(
        HWND hwnd, USLONG msg, MPARAM mp1, MPARAM mp2) {
 MRESULT res;

 /* printf("DC got MSG=%i\n", msg); */

 switch(msg)
  {
  case WM_COMMAND: {   /* Asynchronous controls (Buttons+Menus) */
    int id;
    id=mp1s1;
    if      (id==ID_DCCANCEL) WinPostMsg(hwndClient, UM_DCCANCEL, NULL, NULL);
    else if (id==ID_DCRESET)  WinPostMsg(hwndClient, UM_DCRESET,  NULL, NULL);
    else if (id==ID_DCTRACK)  WinPostMsg(hwndClient, UM_DCTRACK,  NULL, NULL);
    return MFALSE;}

  case WM_ACTIVATE:
    /* Tell client if we are going active .. counts */
    if (mp1) WinPostMsg(hwndClient, msg, mp1, mp2);
    break;

  case WM_INITDLG: {     /* Initialize. */
    /* Nothing to do at the moment */
    break;}

  case WM_CLOSE: {       /* Closing. */
    /* Tell main window so it can update menu check, and refocus */
    WinPostMsg(hwndClient, UM_DCCANCEL, NULL, NULL);
    break;}

  default:
    break;
  }
 res=WinDefDlgProc(hwnd, msg, mp1,mp2);
 return res;
 }  /* gldist */

/* ------------------------------------------------------------------ */
/* GLCOMM -- the WinProc for the command dialog (shell)               */
/* Copyright (c) IBM Corporation, 1991, 1992                          */
/* ------------------------------------------------------------------ */
MRESULT EXPENTRY glcomm( HWND hwnd, USLONG msg, MPARAM mp1, MPARAM mp2)
 {
 char workcomm[MAXCOMM+1];              /* working command string */
 char workcomm2[MAXCOMM+1];             /* working command string */
 char file[MAXTOK+5];                   /* working verb, + extension */
 struct history {                       /* command history list */
   struct history *hnext;               /* -> next in chain */
   struct history *hprev;               /* -> previous in chain */
   char hstring[2];                     /* input string >= 1 chars */
   };
 FILE *f;                               /* work */
 long int size;                         /* work */
 int rc;
 static HWND   hwndOK=NULL;             /* handle of OK button */
 static HWND   hwndText=NULL;           /* handle of entry text */
 static struct history *recent=NULL;    /* -> most recent command */
 static struct history *ph=NULL;        /* -> current history point */
 static unsigned int drawcomm=TRUE;     /* initial checkbox state */
 static unsigned int drawmac =TRUE;     /* initial checkbox state */
 struct history *phh;                   /* work */
 MRESULT mres;                          /* .. */

 switch (msg) {
   case WM_CONTROL:
     switch(mp1s1) {
       case ID_COMMDLGMAC:         /* the macros checkbox */
         rc=SHORT1FROMMR(WinSendDlgItemMsg(hwnd, ID_COMMDLGMAC,
                         BM_QUERYCHECK, NULL, NULL));
         if (rc==1) drawmac=TRUE; else drawmac=FALSE;
         WinPostMsg(hwnd, UM_COMMFOCUS, NULL, NULL); /* reset focus */
         break;
       case ID_COMMDLGDRAW:        /* the redraw checkbox */
         rc=SHORT1FROMMR(WinSendDlgItemMsg(hwnd, ID_COMMDLGDRAW,
                         BM_QUERYCHECK, NULL, NULL));
         if (rc==1) drawcomm=TRUE; else drawcomm=FALSE;
         WinPostMsg(hwnd, UM_COMMFOCUS, NULL, NULL); /* reset focus */
         break;
       case ID_COMMDLGNAME:        /* the name entry field */
         /* if (mp1s2==EN_CHANGE) {}; */
         break;
       }
     return (MRESULT)FALSE;

   case WM_COMMAND:                /* Button: OK/Cancel */
     if (mp1s1==DID_CANCEL) WinDismissDlg(hwnd, TRUE);
      else if (mp1s1==ID_COMMDLGHALT) {
       WinPostMsg(hwnd, UM_COMMFOCUS, NULL, NULL);
       WinPostMsg(hwndClient, UM_COMMHALT, NULL, NULL); /* halt */
       }
      else { /* Please do command */
       /* get it from the dialog */
       WinQueryDlgItemText(hwnd, ID_COMMDLGNAME, sizeof(workcomm), workcomm);

       /* --- Add command to history list --- */
       /* allocate storage for the entry */
       size=(long)sizeof(struct history)+(long)strlen(workcomm);
       ph=(struct history *)malloc((size_t)size);
       if (ph==NULL) printf("Out of memory for command history\n");
        else {
         strcpy(ph->hstring,workcomm);        /* copy the command */
         ph->hnext=NULL;
         ph->hprev=recent;                    /* ok if null */
         if (recent!=NULL) recent->hnext=ph;
         recent=ph;
         /* prune any earlier in history that match */
         for (ph=ph->hprev; ph!=NULL;) {
           if (streq(workcomm,ph->hstring)) {
             /* close up chain */
             (ph->hnext)->hprev=ph->hprev;
             if (ph->hprev!=NULL) (ph->hprev)->hnext=ph->hnext;
             phh=ph;          /* save ptr */
             ph=ph->hprev;    /* ready for next */
             free(phh);       /* and drop the storage */
             } /* prune */
            else ph=ph->hprev;
           } /* prune search */
         /* +++ prune if too many would go here +++ */
         } /* history allocated */
       ph=recent;                       /* set current history pointer */

       /* --- execute the command --- */
       strcpy(commshare, workcomm);     /* copy to shared */
       if (drawmac) {                   /* try for a macro first */
         strcpy(workcomm2,workcomm);    /* make throwaway copy */
         getword(workcomm2,file);       /* isolate verb */
         addext(file,MACEXT);           /* .. add extension if needed */
         f=fopen(file,"r");             /* see if we can open it */
         if (f!=NULL) {                 /* test it -- non-0 if exists */
           fclose(f);                   /* close it again */
           /* make command into a MACRO command -- this will add 6 characters */
           workcomm[MAXCOMM-6]='\0';    /* truncate if too long */
           sprintf(commshare,"macro %s", workcomm);    /* add 'MACRO ' */
           }
         } /* drawmac */
       WinSendMsg(hwnd, UM_COMMDO, NULL, NULL);        /* prevent entry */
       WinPostMsg(hwnd, UM_COMMFOCUS, NULL, NULL);
       WinPostMsg(hwndClient, UM_COMMDO, NULL, NULL);  /* launch */
       }
     return (MRESULT)FALSE;

   case UM_COMMDO:                 /* command is starting */
     WinEnableWindow(hwndOK,   FALSE);  /* disable OK button */
     WinEnableWindow(hwndText, FALSE);  /* .. and entry field */
     WinSetDlgItemText(hwnd, ID_COMMDLGTEXT, "Running command...");
     return (MRESULT)FALSE;

   case UM_COMMDONE:               /* command has completed */
     /* clear the entryfield, renew prompt, and refocus */
     WinSetDlgItemText(hwnd, ID_COMMDLGNAME, "");
     WinSetDlgItemText(hwnd, ID_COMMDLGTEXT, "Please enter a command:");
     WinEnableWindow(hwndOK,   TRUE);   /* enable OK button */
     WinEnableWindow(hwndText, TRUE);   /* .. and entry field */
     /* If asked, tell main window to initiate a redraw */
     if (drawcomm) WinPostMsg(hwndClient, UM_REDRAW, NULL, NULL);
     return (MRESULT)FALSE;

   case UM_COMMFOCUS:              /* command focus */
     WinSetFocus(HWND_DESKTOP, hwndText);
     /* if (diag) printf("CommDial Focussed\n"); */
     return (MRESULT)FALSE;

   case WM_CHAR: {                 /* Key */
     USHORT vk;
     if (LOUSHORT(mp1) & (unsigned)KC_VIRTUALKEY)
      if (!(LOUSHORT(mp1) & (unsigned)KC_KEYUP))
       { /* VK going down */
       vk=HIUSHORT(mp2);
       if (vk==VK_UP || vk==VK_DOWN) {
         /* if (diag) printf("Up-down %i\n", vk); */
         if (recent!=NULL) {  /* have some history */
           if (vk==VK_DOWN) { /* move down before we display */
             ph=ph->hnext; /* move down */
             if (ph==NULL) for (ph=recent; ph->hprev!=NULL;)
               ph=ph->hprev; /* wrap to start of history */
             }
           strcpy(workcomm, ph->hstring);  /* so far */
           /* set the entryfield */
           WinSetDlgItemText(hwnd, ID_COMMDLGNAME, workcomm);
           /* and set focus back to the entryfield */
           WinPostMsg(hwnd, UM_COMMFOCUS, NULL, NULL);
           /* Now prepare PH for next time, if moving back */
           if (vk==VK_UP) {
             ph=ph->hprev; /* move up */
             if (ph==NULL) ph=recent; /* wrap */
             }
           } /* have some history */
         return (MRESULT)FALSE;}
       } /* vk down */
     break;  /* default response, please */
     }

   case WM_INITDLG: {              /* Startup */
     hwndOK  =WinWindowFromID(hwnd, DID_OK);           /* get OK handle */
     hwndText=WinWindowFromID(hwnd, ID_COMMDLGNAME);   /* get text handle */
     /* Allow full-length commands */
     WinSendMsg(hwndText, EM_SETTEXTLIMIT, MPFROMSHORT(MAXCOMM), NULL);
     /* .. initially with any startup macro command */
     WinSetDlgItemText(hwnd, ID_COMMDLGNAME, commshare);
     WinEnableWindow(hwndOK,   FALSE);  /* disable OK button initially */
     WinEnableWindow(hwndText, FALSE);  /* .. and entry field */
     /* and set checkbox(es) */
     if (drawcomm) WinSendDlgItemMsg(hwnd, ID_COMMDLGDRAW,
           BM_SETCHECK, MPFROMSHORT(TRUE), NULL);
     if (drawmac)  WinSendDlgItemMsg(hwnd, ID_COMMDLGMAC,
           BM_SETCHECK, MPFROMSHORT(TRUE), NULL);
     return (MRESULT)FALSE;}

   } /* switch */
 mres=WinDefDlgProc(hwnd, msg, mp1, mp2);
 return mres;
 } /* command dialog */

/* ------------------------------------------------------------------ */
/* ERRBOX -- Error reporting for commands                             */
/* Copyright (c) IBM Corporation, 1991, 1992                          */
/* ------------------------------------------------------------------ */
#define MAXERRLEN MAXCOMM+100
int errbox(const unsigned char *f, ...) {
  static UCHAR buffer[MAXERRLEN+1];/* formatting area */
  long count;
  va_list argptr;                  /* -> variable argument list */
  va_start(argptr, f);             /* get pointer to argument list */
  count=vsprintf(buffer, f, argptr);    /* print to the buffer */
  va_end(argptr);                  /* done with variable arguments */
  if (count<0) return (int)(count-1000);     /* ouch! */
  /* If overrun, we are probably "dead", but truncate and carry on */
  if (count>MAXERRLEN) count=MAXERRLEN;
  buffer[count]='\0';              /* ensure terminated */

  /* Now use PRINTF or WINMESSAGEBOX, depending on user setting */
  if (!errorbox) printf("=> %s\n",buffer);
   else {  /* Message box please, owned by whoever */
    WinMessageBox(HWND_DESKTOP, HWND_OBJECT,
        buffer, "PMGlobe command result", 0,
        MB_CANCEL | MB_MOVEABLE | MB_ICONEXCLAMATION);
    }
  /* ignore any errors */
  return 0;
  } /* errbox */

/* ------------------------------------------------------------------ */
/* ALERT -- Error trying to draw globe                                */
/* Copyright (c) IBM Corporation, 1992                                */
/* ------------------------------------------------------------------ */
void alert(const unsigned char *f, ...) {
  static UCHAR buffer[MAXERRLEN+1];/* formatting area */
  long count;
  va_list argptr;                  /* -> variable argument list */

  if (alerted) return;             /* alert already in progress */
  va_start(argptr, f);             /* get pointer to argument list */
  count=vsprintf(buffer, f, argptr);    /* print to the buffer */
  va_end(argptr);                  /* done with variable arguments */
  if (count<0) return;             /* ouch! best ignore */
  alerted=TRUE;
  /* If overrun, we are probably "dead", but truncate and carry on */
  if (count>MAXERRLEN) count=MAXERRLEN;
  buffer[count]='\0';              /* ensure terminated */

  /* Now use PRINTF and send Msg to Client for WINMESSAGEBOX */
  if (diag) printf("=> %s\n",buffer);
  WinPostMsg(hwndClient, UM_ALERT, MPFROMP(buffer), NULL);
  /* when this is done, GLCLIENT will clear ALERTED */
  /* ignore any errors */
  return;
  } /* alert */
