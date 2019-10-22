/* ----------------------------------------------------------------- */
/* PRINTF: diverts PRINTF calls to an OS/2 Named Queue               */
/* ----------------------------------------------------------------- */
/*                                                                   */
/* Copyright (c) Mike Cowlishaw, 1993-2019.  All rights reserved.    */
/* Parts Copyright (c) IBM, 1993-2009.                               */
/*                                                                   */
/* Permission to use, copy, modify, and distribute this software     */
/* for any non-commercial purpose without fee is hereby granted,     */
/* provided that the above copyright notice and this permission      */
/* notice appear in all copies, and that notice and the date of      */
/* any modifications be added to the software.                       */
/*                                                                   */
/* This software is provided "as is".  No warranties, whether        */
/* express, implied, or statutory, including, but not limited to,    */
/* implied warranties of merchantability and fitness for a           */
/* particular purpose apply to this software.  The author shall      */
/* not, in any circumstances, be liable for special, incidental,     */
/* or consequential damages, for any reason whatsoever.              */
/*                                                                   */
/* ----------------------------------------------------------------- */
/*                                                                   */
/* This routine, when linked into an .EXE instead of the usual C     */
/* runtime, sends the edited result string to a named queue (if      */
/* it exists).  If the queue does not exist, then all printf data    */
/* are discarded (ignored).                                          */
/*                                                                   */
/* The result string is accumulated until a line feed (LF) character */
/* is received; the whole line is then sent to the queue.  Lines are */
/* automatically broken at a set (tailorable) length, if necessary.  */
/*                                                                   */
/* This routine may be tailored by altering the #defines at the      */
/* top:                                                              */
/*                                                                   */
/*   PRINTFID      - An ID string that is prefixed to each line of   */
/*                   data before being sent to the queue.  This      */
/*                   can be any string, or the null string.          */
/*   PRINTFMAXLEN  - Maximum length of string that can be formatted  */
/*                   in a single call.                               */
/*                   Results are unpredictable if this length is     */
/*                   exceeded.  Default is 250.                      */
/*   PRINTFLINELEN - Maximum length of a line that will be sent.     */
/*                   This excludes the prefix an its blank.  If the  */
/*                   calls to printf cause a line to be generated    */
/*                   that is longer than this, the line will be      */
/*                   broken at this point.                           */
/*   PRINTFTHREADS - Maximum number of threads expected.  This may   */
/*                   need to be increased if the process limitation  */
/*                   is removed, or you can save a little storage    */
/*                   by decreasing it.  PRINTFs from threads larger  */
/*                   than this number are ignored.                   */
/*   PRINTFQNAME   - The name of the public queue that the result    */
/*                   is to be sent to.  Normally '\QUEUES\PMPRINTF'. */
/*                   Note that the \QUEUES\ part is required.        */
/*                                                                   */
/* Returns:                                                          */
/*   n: Count of data characters, if successfully received           */
/*   0: If no queue existed (i.e., no server)                        */
/*  <0: An error occurred (e.g., out of memory)                      */
/*                                                                   */
/* Restrictions:                                                     */
/*   1. Total length of data (length of PRINTFID, + PRINTFMAXLEN)    */
/*      must be less than 32K-1.                                     */
/*   2. This has only been tested under Large model compilation.     */
/*      It may need modification for other models.                   */
/*   3. This version uses a static array to point to the per-thread  */
/*      data.  The code could be made read-only by hanging this      */
/*      array (and the other static information) off a system-owned  */
/*      anchor of some kind.                                         */
/*   4. To use PRINTF within other than the main thread in a         */
/*      program, that thread must be started with _beginthread       */
/*      (not DosCreateThread).  This restriction is a consequence of */
/*      the use of C library routines (sprintf) in PRINTF <sigh>.    */
/*   5. If the last PRINTF done by a thread does not end in '\n'     */
/*      then the final part-line may be lost or appear later.        */
/*                                                                   */
/* Protocol:                                                         */
/*   PRINTF writes its data to the named queue using the following   */
/*   protocol:                                                       */
/*     Request -- Holds the Selector of the shared memory segment    */
/*                valid in the receiver's address space.  The        */
/*                shared memory segment hold the character data      */
/*                (a 0-terminated string) starting at offset 0.      */
/*     Length  -- The length of the data, including terminator.      */
/*                A negative length indicates a BELL in the data.    */
/*     Address -- Timestamp (when queue was written) in C long       */
/*                integer format (as returned by time()).            */
/*                This may be 0L if not required.                    */
/*                                                                   */
/* Notes:                                                            */
/*   1. PMPRINTF uses a queue and shared memory messages because:    */
/*        (a) It makes collection at the receiving end very easy.    */
/*        (b) I wanted to experiment with queues and shared memory.  */
/*      This make not be the most cost-effective method.             */
/*   2. Typical IBM C/2 compiler invocation:                         */
/*        cc -nologo -G2s -W3 -Zpe printf.c;                         */
/*      For multi-thread (and link with LLIBCMT):                    */
/*        cl -c -nologo -G2s -W3 -Zpel Alfw printf.c                 */
/*   3. PRINTF sends the timestamp across the queue as a GMT long    */
/*      integer, the result from a call to the C function time().    */
/*      This will only be correct if the environment variable TZ has */
/*      been set (e.g., TZ=EST5EDT), or you are in the same time     */
/*      zone as the default for your compiler (e.g., +5 for IBM C/2, */
/*      +8 for Microsoft C6).  Hence, if PRINTF is called by an MS   */
/*      C6 program and TZ has not been set, then timestamps logged   */
/*      will be wrong by 3 hours, because PMPRINTF currently uses    */
/*      the IBM C/2 runtime library.  (The MS compiler applies a     */
/*      default +8 or +7 hour shift to generate GMT, then when       */
/*      received, the IBM compiler will apply a default -5 or -4     */
/*      hour shift to generate the local time recorded in the log.)  */
/*      For more information, see the tzset() function description   */
/*      in the IBM C/2 manual.                                       */


/* ----- Customization variables ----- */
#define PRINTFID      ""
#define PRINTFMAXLEN  250
#define PRINTFLINELEN 100
#define PRINTFTHREADS  54
#define PRINTFQNAME   "\\QUEUES\\PMPRINTF"

/* ----- Includes and externals ----- */
#include <stdlib.h>                /* standard C functions */
#include <stddef.h>                /* (note NOT stdio.h, to avoid a */
#include <string.h>                /*   warning of non-matching */
#include <time.h>                  /*   PRINTF prototype) */
#include <stdarg.h>                /* .. */
#define INCL_DOS                   /* Operating system definitions */
#include <os2.h>                   /* For OS/2 functions */
/* Next line defines sprintf (from mt\stdio.h) */
extern int far _CDECL vsprintf(char far *, const char far *, va_list);

/* ----- Local defines ----- */
#define PRINTFIDSIZE sizeof(PRINTFID)
#define PRINTFMAXBUF PRINTFIDSIZE+PRINTFLINELEN

/* ----- Per-thread output buffer and current indices into line ---- */
struct perthread {
  UCHAR  line[PRINTFMAXBUF];       /* accumulator */
  SHORT  lineindex;                /* where next char */
  SHORT  tidemark;                 /* rightmost char */
  SHORT  bell;                     /* TRUE if line has bell */
  };

/* ----- Local static variables ----- */
static USHORT ourpid=0;            /* our process ID */
static USHORT servepid=0;          /* process IDs of the server */
static HQUEUE qhandle=0;           /* handle for the queue */
static struct perthread *tps[PRINTFTHREADS+1]; /* -> per-thread data */

/* ----- Local subroutine ----- */
int _CDECL printf_(struct perthread *);

/* ----------------------------------------------------------------- */
/* The "printf" function.  Note this has a variable number of        */
/* arguments.                                                        */
/* ----------------------------------------------------------------- */
int printf(f)
  char f[];
  {
  USHORT  rc;                      /* returncode */
  PIDINFO pids;                    /* process/thread id structure */
  TID     ourtid;                  /* thread ID */
  struct perthread *tp;            /* pointer to per-thread data */

  rc=DosOpenQueue(&servepid, &qhandle, PRINTFQNAME);  /* Open the Q */
  /* Non-0 RC means Q does not exist or cannot be opened */
  if (rc==343) return 0;           /* queue does not exist, so quit */
  if (rc!=0) return -1;            /* report any other error */

  /* First determine our thread ID (and hence get access to the      */
  /* correct per-thread data.  If the per-thread data has not been   */
  /* allocated, then allocate it now.  It is never freed, once       */
  /* allocated, as PRINTF is not notified of end-of-thread.          */
  DosGetPID(&pids);                /* get process/thread info */
  ourtid=pids.tid;                 /* .. and copy TID */
  if (ourtid>PRINTFTHREADS)        /* too many threads .. */
    return 0;                      /* .. so quit, quietly */
  tp=tps[ourtid];                  /* copy to local pointer */
  if (tp==NULL) {                  /* uninitialized (NULL=0) */
    /* allocate a per-thread structure */
    tp=(struct perthread *)malloc(sizeof(struct perthread));
    if (tp==NULL) return -1;       /* out of memory -- return error */
    tps[ourtid]=tp;                /* save for future calls */
    strcpy(tp->line,PRINTFID);     /* initialize: line.. */
    tp->lineindex=PRINTFIDSIZE-1;  /* ..where next char */
    tp->tidemark =PRINTFIDSIZE-2;  /* ..rightmost char */
    tp->bell=FALSE;                /* ..if line has bell */
    if (ourpid==0) ourpid=pids.pid;/* save PID for all to use */
    }

  { /* Block for declarations -- only needed if queue exists, etc. */
    SHORT count;                   /* count of characters formatted */
    SHORT formlen;                 /* length of formatted data */
    UCHAR buffer[PRINTFMAXLEN+1];  /* formatting area */
    SHORT i, newind;               /* work */
    UCHAR ch;                      /* .. */
    va_list argptr;                /* -> variable argument list */

    va_start(argptr, f);           /* get pointer to arg list */
    count=vsprintf(buffer, f, argptr);
    va_end(argptr);                /* done with variable arguments */

    if (count>PRINTFMAXLEN) {
      /* Disaster -- we are probably "dead", but just in case we */
      /* are not, carry on with truncated data. */
      count=PRINTFMAXLEN;
      }
    buffer[count]='\0';            /* ensure terminated */
    formlen=count+1;               /* length of whole string + \0 */
    /* OK, ready to go with the data now in BUFFER                    */
    /* We copy from the formatted string to the output (line) buffer, */
    /* taking note of certain control characters and sending a line   */
    /* the queue whenever we see a LF control, or when the line       */
    /* fills (causing a forced break).                                */
    for (i=0; ch=buffer[i]; i++) {
      switch(ch) {
        case '\r':                 /* carriage return */
          tp->lineindex=PRINTFIDSIZE-1; /* back to start of line */
          break;
        case '\n':                 /* new line */
        case '\f':                 /* form feed */
          rc=printf_(tp);          /* print a line */
          if (rc!=0) return rc;    /* error */
          break;
        case '\t':                 /* tab */
          newind=tp->lineindex-PRINTFIDSIZE+1;   /* offset into data */
          newind=tp->lineindex+5-newind%5;    /* new index requested */
          if (newind>=PRINTFMAXBUF) newind=PRINTFMAXBUF;    /* clamp */
          for (; tp->lineindex<newind; tp->lineindex++) {
            if (tp->lineindex>tp->tidemark) {  /* beyond current end */
              tp->line[tp->lineindex]=' ';              /* add space */
              tp->tidemark=tp->lineindex;
              }
            }
          break;
        case '\v':                 /* vertical tab */
          /* ignore it */
          break;
        case '\b':                 /* backspace */
          tp->lineindex=max(tp->lineindex-1,PRINTFIDSIZE);
          break;
        case '\a':                 /* alert (bell) */
          tp->bell=TRUE;
          break;
        default:                   /* ordinary character */
          tp->line[tp->lineindex]=ch;
          if (tp->lineindex>tp->tidemark)  /* is rightmost.. */
            tp->tidemark=tp->lineindex;
          tp->lineindex++;                 /* step for next */
        } /* switch */
      if (tp->lineindex>=PRINTFMAXBUF) {
        rc=printf_(tp);            /* print a line */
        if (rc!=0) return rc;      /* error */
        }

      } /* copy loop */
    return count;                  /* all formatted data processed */
    } /* block */
  } /* printf */

/* ----- printf_(tp) -- Local subroutine to send a line ------------ */
/* A line has been completed (or overflowed): write it to the queue. */
int _CDECL printf_(tp)
  struct perthread *tp;            /* pointer to per-thread data */
  {
  USHORT  rc;                      /* returncode */
  PSZ     pszTo, pszFrom;          /* character pointers */
  SEL     oursel, servesel;        /* selectors for shared segment */
  USHORT  size;                    /* total size of output data */
  long    timenow;                 /* holds current time */

  tp->line[tp->tidemark+1]='\0';   /* add terminator */
  size=tp->tidemark+2;             /* total length of data */

  /* Get some shared memory that can be given away */
  rc=DosAllocSeg(size, &oursel, SEG_GIVEABLE);
  if (rc!=0) return -2;            /* error */
  pszTo=MAKEP(oursel, 0);          /* make far ptr from selector */
  pszFrom=&(tp->line[0]);          /* pointer to source */
  while (*pszTo++=*pszFrom++);     /* copy the string to segment */

  if (ourpid==servepid) servesel=oursel; /* no giveaway needed */
   else {
    rc=DosGiveSeg(oursel, servepid, &servesel);
    if (rc!=0) return -3;}         /* error */

  /* Write the selector, size, and timestamp to the queue */
  if (tp->bell) size=-size;        /* BELL passed by negation */
  time(&timenow);                  /* optional - else use 0 */
  rc=DosWriteQueue(qhandle,        /* handle */
                   servesel,       /* 'request' (selector) */
                   size,           /* 'length'  (length/bell) */
            (PBYTE)timenow,        /* 'address' (timestamp) */
                   0);             /* priority (FIFO if enabled) */
  if (rc!=0) return -4;            /* error */
  if (ourpid!=servepid) {          /* if given away.. */
    rc=DosFreeSeg(oursel);         /* .. *we* are done with it */
    if (rc!=0) return -5;}         /* error */
  /* Reset the line buffer and indices */
  tp->lineindex=PRINTFIDSIZE-1;    /* where next char */
  tp->tidemark =PRINTFIDSIZE-2;    /* rightmost char */
  tp->bell     =FALSE;             /* true if line has bell */
  return 0;                        /* success! */
  } /* printf_ */
