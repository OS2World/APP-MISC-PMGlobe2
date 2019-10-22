/* ------------------------------------------------------------------ */
/* PMGlobe -- a geographical application                              */
/* GLDIST  -- WinProc for the Distance Calculator dialogue            */
/* GLCOMM  -- WinProc for the command dialogue                        */
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

#include "globe.h"

/* Subroutines */
extern int streq(char *, char *);       /* string compare */

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
 struct history {                       /* command history list */
   struct history *hnext;               /* -> next in chain */
   struct history *hprev;               /* -> previous in chain */
   char hstring[2];                     /* input string >= 1 chars */
   };
 long int size;                         /* work */
 static struct history *recent=NULL;    /* -> most recent command */
 static struct history *ph=NULL;        /* -> current history point */
 struct history *phh;                   /* work */

 switch (msg) {
   case WM_CONTROL:
     switch(mp1s1) {
       case ID_COMMDLGNAME:        /* the name entry field */
         if (mp1s2==EN_CHANGE) {};
         break;
       }
     return (MRESULT)FALSE;

   case WM_COMMAND:                /* Button: OK/Cancel */
     if (mp1s1==DID_CANCEL) WinDismissDlg(hwnd, TRUE);
      else { /* Please do command */
       /* get it from the dialog */
       WinQueryDlgItemText(hwnd, ID_COMMDLGNAME,
                           sizeof(workcomm), workcomm);

       /* --- Add command to history list --- */
       /* allocate storage for the entry */
       size=sizeof(struct history)+strlen(workcomm);
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
       ph=recent;             /* set current history pointer */

       /* --- execute the command --- */
       gldo(workcomm);        /* do the command */
       /* clear the entryfield */
       WinSetDlgItemText(hwnd, ID_COMMDLGNAME, "");
       /* and set focus back to the entryfield */
       WinPostMsg(hwnd, UM_REFOCUS, NULL, NULL);
       WinPostMsg(hwndClient, UM_COMMDONE, NULL, NULL);
       }
     return (MRESULT)FALSE;

   case UM_REFOCUS:                /* re-focus to command entryfield */
     WinSetFocus(HWND_DESKTOP,
        WinWindowFromID(hwnd, ID_COMMDLGNAME));
     return (MRESULT)FALSE;

   case WM_CHAR: {                 /* Key */
     USHORT vk;
     if (LOUSHORT(mp1) & KC_VIRTUALKEY) if (!(LOUSHORT(mp1) & KC_KEYUP))
       { /* VK going down */
       vk=HIUSHORT(mp2);
       if (vk==VK_UP || vk==VK_DOWN) {
         if (diag) printf("Up-down %i\n", vk);
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
           WinPostMsg(hwnd, UM_REFOCUS, NULL, NULL);
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
     /* Allow full-length commands */
     WinSendDlgItemMsg(hwnd, ID_COMMDLGNAME,
           EM_SETTEXTLIMIT, MPFROMSHORT(MAXCOMM), NULL);
     return (MRESULT)FALSE;}

   } /* switch */
 return(WinDefDlgProc(hwnd, msg, mp1, mp2));
 } /* command dialog */
