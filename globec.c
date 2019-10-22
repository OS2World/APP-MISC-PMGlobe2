/* ------------------------------------------------------------------ */
/* PMGlobe -- a geographical application                              */
/* GLOBEC  -- Command runner thread                                   */
/* GLREXX  -- Invoke a single macro                                   */
/* GLEXTR  -- EXTRACT command                                         */
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

#define  INCL_DOSPROCESS
#define  INCL_DOSSEMAPHORES
#include "globe.h"

/*====================================================================*/
/* GLOBEC -- thread for independent commands                          */
/*====================================================================*/

/*----- Overview -----------------------------------------------------*/
/* This command-runner routine runs as a separate thread, started     */
/* early in the running of the application.  It signals when it is    */
/* ready, then loops waiting on a semaphore for a command to run.     */
/*                                                                    */
/*   Start                                                            */
/*     Signal ready  (SEMHAVECOMM)                                    */
/*   Start loop                                                       */
/*     Wait until need to run a macro (SEMRUNCOMM)                    */
/*     Copy COMMSHARE to safe local copy                              */
/*     Signal ready  (SEMHAVECOMM)                                    */
/*     Clear pending 'halt' (we are at user level)                    */
/*     Run the command (GLDO)                                         */
/*     Post message UM_COMMDONE (ready for another) to Client         */
/*     end loop                                                       */
/*                                                                    */
/*--------------------------------------------------------------------*/

HAB    habComm;                    /* our anchor block .. */
HMQ    hmqComm;                    /* .. and queue */
static ULONG posted;               /* for semaphores */
void globec(void)
 {
 char command[MAXCOMM+1];          /* whole */
 long rc;

 /*----- end of dcls -------------------------------------------------*/

 habComm=WinInitialize(NULL);      /* so thread can use GPI */
 hmqComm=WinCreateMsgQueue(habComm,0);
 WinCancelShutdown(hmqComm,TRUE);  /* allow shutdown */

 DosPostEventSem(semhavecomm);     /* tell main thread: we're ready */

 /*====== Here starts the main loop =============================*/
 for (;;) {                        /* FOREVER */
   DosWaitEventSem(semruncomm, SEM_INDEFINITE_WAIT);
   DosResetEventSem(semruncomm, &posted); /* lock for next time */

   /* ----- Protected Code ----------------------------------------*/
   /* In this section of code, we copy the command to local        */
   /* storage.  The main thread is prevented from running while    */
   /* we do this.                                                  */
   strcpy(command, commshare);
   DosPostEventSem(semhavecomm);       /* Let main thread continue */
   /* ----- End of Protected Code -------------------------------- */

   rc=gldo(command);
   if (rc==0) /* Looks good: carry on and be responsive */
     command[0]='\0'; /* be clean */
    else { /* oops -- maybe trace */
     if (diag) printf("RC=%li from '%s'\n", rc, command);
     } /* bad RC */
   WinPostMsg(hwndClient, UM_COMMDONE, MPFROMLONG(rc), NULL);
   } /* ===== end of main ('forever') loop ======================== */

 /* Here only if disaster/break */
 WinTerminate(habComm);            /* done with anchor block */
 return;
 } /* globec */


/*====================================================================*/
/* GLREXX -- execute MACRO command (invoke REXX)                      */
/*====================================================================*/
/*                                                                    */
/* This code sets up subcommand handler, invokes a REXX macro,        */
/* and also provides an exit for SAY output.  The subcommand          */
/* handler accepts subcommands and any incoming subcommand is         */
/* passed on to the GLDO command processor.                           */
/*                                                                    */
/* GLREXX command arguments:                                          */
/*                                                                    */
/*   name of the macro                                                */
/*   argument string                                                  */
/*                                                                    */
/* Returns:  return code from the macro or REXX interpreter           */
/*                                                                    */

#define INCL_REXXSAA
#include <rexxsaa.h>               /* needed for REXXSAA() etc. */

SLONG APIENTRY glcommand(PRXSTRING, PUSHORT, PRXSTRING);
SLONG APIENTRY glsio(SLONG, SLONG, PRXSTRING);
SLONG APIENTRY glhlt(SLONG, SLONG, PUSLONG);
void       rxcopy(char *,RXSTRING);/* copy a REXX string */
static int registered=FALSE;       /* set if subcommand registered  */
static int rexxdepth=0;            /* call nesting */
/* System exits that we handle */
static RXSYSEXIT exitlist[] ={
  {"GLSIO",RXSIO}, {"GLHLT",RXHLT}, {NULL,RXENDLST}};

/* ----- Subroutine starts here ----- */
int glrexx(char *filename, char *options)
  {

  RXSTRING arg;                    /* argument string for REXX   */
  RXSTRING rexxretval;             /* return value from REXX     */
  int rc;                          /* returncodes */
  SHORT rexxrc=0;
  #if defined(B16)
  SCBLOCK  scb;                    /* subcommand block */
  #else
  static unsigned char UserArea[8];/* work */
  ULONG urc;
  #endif

  if (!registered) {               /* first call... */
    #if defined(B16)
    /* Set up subcommand block, and register the SUBCOM handler */
    scb.scbname="GLOBE";
    scb.scbaddr=(PFN)glcommand;
    scb.scbdll_name=(PSZ)0;        /* EXE, not DLL */
    rc=RxSubcomRegister(&scb);
    if (rc!=0 && diag) printf("RSR failed, RC=%li\n", rc);
    #else
    urc=RexxRegisterSubcomExe("GLOBE", (PFN)glcommand, UserArea);
    if (urc!=0 && diag) printf("RRSE failed, RC=%li\n", urc);
    #endif
    #if defined(B16)
    /* Set up subcommand block, and register the Exit handlers */
    scb.scbname="GLSIO";
    scb.scbaddr=(PFN)glsio;
    scb.scbdll_name=(PSZ)0;        /* EXE, not DLL */
    rc=RxExitRegister(&scb);
    if (rc!=0 && diag) printf("RER failed, RC=%li\n", rc);
    scb.scbname="GLHLT";
    scb.scbaddr=(PFN)glhlt;
    rc=RxExitRegister(&scb);
    if (rc!=0 && diag) printf("RER2 failed, RC=%li\n", rc);
    #else
    urc=RexxRegisterExitExe("GLSIO", (PFN)glsio, UserArea);
    if (urc!=0 && diag) printf("RREE failed, RC=%li\n", urc);
    urc=RexxRegisterExitExe("GLHLT", (PFN)glhlt, UserArea);
    if (urc!=0 && diag) printf("RREE2 failed, RC=%li\n", urc);
    #endif
    registered=TRUE;
    }

  if (rexxdepth>=MAXREXX) return -2;    /* too deep */
  if (rexxdepth==0) WinPostMsg(hwndClient, UM_MACROBEG, NULL, NULL);

  rexxdepth++;                          /* bump counter */

  if (diag) printf("Calling macro '%s' with '%s'...\n", filename, options);
  MAKERXSTRING(rexxretval,0,0);    /* empty result string */

  arg.strptr=options;              /* argument string */
  arg.strlength=strlen(options);

  rc=RexxStart((SLONG)      1,     /* call the REXX interpreter  */
               (PRXSTRING)  &arg,
               (PSZ)        filename,
               (PRXSTRING)  NULL,
               (PSZ)        "GLOBE",
               (SLONG)      RXCOMMAND,
               (PRXSYSEXIT) exitlist,
               (PSHORT)     &rexxrc,
               (PRXSTRING)  &rexxretval );

  if (rc==0) {
    if (rexxretval.strlength>0) {rc=rexxrc;
      DosFreeMem(rexxretval.strptr);}        /* drop the result */
    }
   else /* rc!=0 */ {
    /* RC may be positive or negative depending on OS/2 release <sigh> */
    rc=abs(rc);
    if (rc==3) errbox("Could not find the macro file '%s'\n", filename);
    else if (rc==4) {   /* 4 means halt without handle */
      if (rexxdepth==1) errbox("Macro '%s' halted\n", filename);
      }
    else errbox("Sorry, REXX error %i running macro '%s' with arguments '%s'",
                 rc, filename, options);
    }

  rexxdepth--;                     /* unbump counter */
  if (rexxdepth==0) WinPostMsg(hwndClient, UM_MACROEND, NULL, NULL);
  return rc;
  } /* glrexx */

/* ----- The I/O system exit handler ----- */
/* This re-directs SAY and TRACE output to PRINTF */
/* Function should only be RXSIO */
/* Sub-function indicates which particular I/O is going on */
/* Control block is sub-function dependent, but the ones we want are */
/* both RXSTRING.                                                    */
SLONG APIENTRY glsio(SLONG function, SLONG subfunc, PRXSTRING pdata)
  {
  char buffer[MAXCOMM+1];

  if (function!=RXSIO) return -1;       /* oopsie */
  if (subfunc!=RXSIOSAY                 /* if not SAY or TRACE .. */
   && subfunc!=RXSIOTRC) return 1;      /* standard handling please */
  if (pdata->strlength>MAXCOMM) return -1;   /* too long */
  /* Copy string to buffer and add terminator */
  rxcopy(buffer, *pdata);
  printf("%s\n", buffer);               /* present it */
  return 0;  /* good */
  } /* rexxsio */

/* ----- The Halt system exit handler ----- */
/* Sub-function indicates which particular Halt exit */
SLONG APIENTRY glhlt(SLONG function, SLONG subfunc, PUSLONG haltflag)
  {
  if (function!=RXHLT)   return -1;     /* oopsie */
  if (subfunc==RXHLTTST) {
    *haltflag=(unsigned)rexxhalts[rexxdepth-1];
    if (diag && *haltflag) printf("Raising Halt at level %li\n", rexxdepth-1);
    }
   else if (subfunc==RXHLTCLR) {
    commhalt=FALSE;                     /* primary request handled */
    rexxhalts[rexxdepth-1]=FALSE;       /* and this level done too */
    if (diag) printf("Clearing Halt at level %li\n", rexxdepth-1);
    }
  return 0;  /* good */
  } /* rexxhlt */

/* ----- The "GLOBE" subcommand handler ----- */
SLONG APIENTRY glcommand(
  PRXSTRING command, PUSHORT flags, PRXSTRING retstr)
  {

  char buffer[MAXCOMM+1];
  int rc;

  *flags=RXSUBCOM_OK;        /* Normally OK */

  if (command->strlength>MAXCOMM) {
    printf("Command string too long to handle\n");
    *flags=RXSUBCOM_FAILURE;
    return -2;}

  /* Copy string to buffer and add terminator (may not be needed in V2) */
  rxcopy(buffer,*command);
  /* if (diag) printf("Subcommand: '%s'\n", buffer); */
  /* execute command */
  rc=gldo(buffer);

  /* MAKERXSTRING(*retstr,0,0); */ /* make empty string */
  retstr->strlength=1L;            /* set up 0 for 0 RC */
  strcpy(retstr->strptr,"0");
  return rc;
  }

/* ----- RXCOPY - copy RXSTRING to 0-terminated string ----- */
/* Up to a maximum of MAXCOMM are copied: any extra is ignored */
/* (Buffers, as usual, should be MAXCOMM+1) */
void rxcopy(char *target, RXSTRING source) {
  int i, m;         /* stepper and maximum */
  char *t, *s;      /* work */
  if (source.strlength>MAXCOMM) m=MAXCOMM;
   else m=(int)source.strlength;
  t=&target[0]; s=source.strptr;
  /* Copy the string, translating nulls to blanks */
  for (i=0; i<m; i++, s++, t++) if (*s) *t=*s; else *t=' ';
  *t='\0';          /* add terminator */
  return;
  } /* rxcopy */


/* ------------------------------------------------------------------ */
/* GLEXTR: Globe EXTRACT command                                      */
/* ------------------------------------------------------------------ */
extern char * qvalue(char *);           /* query item value */
int glextr(char inoptions[])
  {
  char *pchar;                          /* working */
  char options[MAXCOMM+1];              /* copy of options */
  char item[MAXTOK+1];                  /* item to be SET */
  SHVBLOCK shv;                         /* shared variable block */
  int rc;

  strcpy(options,inoptions);            /* safe working copy */

  getword(options,item);                /* get item */
  if (item[0]=='\0') {
    printf("No item given.  Try 'EXTRACT item [item]...'\n");
    return 2;}

  /* Set up REXX SHV block constants */
  shv.shvnext=NULL;                     /* only one block */
  shv.shvcode=RXSHV_SET;                /* direct set */

  for (; item[0]!='\0';) {
    strupr(item);                       /* save time, and for name */
    pchar=qvalue(item);                 /* pchar is ->char */
    if (pchar==NULL) return 1;          /* error */
    /* Now (attempt to) set the REXX variable */
    /* Add name/value to SHVBLOCK */
    MAKERXSTRING(shv.shvname, item, strlen(item));
    MAKERXSTRING(shv.shvvalue,pchar,strlen(pchar));
    /* One or both of these is needed, too <sigh> */
    shv.shvnamelen=strlen(item);
    shv.shvvaluelen=strlen(pchar);
    rc=(int)RexxVariablePool(&shv);     /* Set the variable */
    if (rc!=0 && rc!=RXSHV_NEWV) {      /* error? */
      printf("%s: return code from RxVar was %i\n", NAME, rc);
      return rc;}
    getword(options,item);              /* get next item */
    } /* while items */

  return 0;
  } /* glextr */

