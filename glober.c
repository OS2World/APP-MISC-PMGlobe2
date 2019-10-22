/* ------------------------------------------------------------------ */
/* PMGlobe -- a geographical application                              */
/* GLOBER  -- Globe drawing thread                                    */
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

#define  EXCL_WIN
#define  INCL_GPIPRIMITIVES
#define  INCL_GPIBITMAPS
#define  INCL_GPILCIDS
#define  INCL_DOSPROCESS
#define  INCL_DOSSEMAPHORES
#include <malloc.h>
#include <time.h>

/* Time delays for draw/updates (ms) */
#define TIMEROWS  100
#define TIMEMERGE 166

/* handy constant */
#define FULLDAYSECS (24L*60L*60L)

extern long _timezone;

#include "globe.h"
#include "eot.h"
#include "dec.h"

extern unsigned char huge *globemap(void);   /* map-getter */
extern int globecol(RGB2 *, PCOLSTRUCT);     /* colour processor */
void dropbitmaps(void);                      /* drop any old bitmaps */
void setsun(long, long *, long *);           /* set SUN lat, lon */
void domerge(int, int);                      /* merge map+sprites->shadow */
void copynear(int *, int *, PCOLSTRUCT, long);    /* copy NEAR table */
extern void alert(const unsigned char *, ...);    /* Error drawing */
void drawgc(long, long, long, long, long, long,   /* Draw great circle */
            long, long, long, long);

/* Maximum (largest) map indices */
#define TOPMAPX (LANDMAPDIMX-1)
#define TOPMAPY (LANDMAPDIMY-1)

/* These variables are needed by both GLOBER and GLPAINT */
/* SEMBITMAP semaphore controls access to the MDC bitmap, and hence */
/* protects PAINT from bad bitmap, etc. */
extern HEV sembitmap;              /* protect bitmap */
HPS    globeps=NULL;               /* HPS for globe bitmap   */
HPS    spriteps=NULL;              /* HPS for 'sprite' bitmap   */

/* These variables are shared with other external subroutines */
long   drawradius, trueradius;     /* radius*32 */
long   drawvlon, drawvlat;         /* current view long/lat */

/* These variables may be needed by both GLOBER and DOMERGE */
static HDC     globedc=NULL;       /* device contexts */
static HDC     spritedc=NULL;      /* .. */
static HBITMAP globebitmap=NULL;   /* bitmap for globe image */
static HBITMAP spritebitmap=NULL;  /* bitmap for 'sprite' image */
static long    rupx;               /* diameter rounded up for BMAPs */
static unsigned char sback;        /* byte index of sprite transparent */
/* Define our own INFOTABLE as toolkit one allows only 1 RGB entry (!) */
struct BMPTABLE {                  /* Bit-map information table */
    BITMAPINFOHEADER2 head;
    RGB2 rgb[256];
    };
static struct BMPTABLE bmap;       /* used by DOMERGE too */
static BITMAPINFOHEADER2 bmapinfo; /* bit-map information header, 0'd */
static ULONG posted;               /* for V2 semaphores */
static unsigned char huge
                   *flatmap=NULL;  /* drawing image for globe */
static unsigned char *merge=NULL;  /* line for bitmap merge */
#if defined(IDLEBLIP)
static int  slices=0;              /* slices drawn since lo-priority */
#endif

/* Next are global, to keep stack size small */
static int  sz32[MAXRADIUS+1];     /* Save Z32 (lookaside) array */
static long slat[MAXRADIUS+1];     /* Save LAT (lookaside) array */
static int  slxz[MAXRADIUS+1];     /* Save LXZ (lookaside) array */

/*====================================================================*/
/* GLOBER -- draw the globe from the map                              */
/*====================================================================*/

/*----- Overview -----------------------------------------------------*/
/* This globe-drawer routine runs as a separate thread, started early */
/* in the running of the application.  There are two gated sections,  */
/* leading in to a loop which redraws the globe whenever parameters   */
/* change.                                                            */
/*                                                                    */
/*   Start                                                            */
/*     Generate the local landmap bitmap                              */
/*     Signal ready  (SEMHAVEMAP)                                     */
/*   Wait for 'Screen is known' (SEMHAVESCREEN)                       */
/*     Determine screen information, colours, etc.                    */
/*   Start loop                                                       */
/*     Wait until need to redraw (SEMNEEDDRAW)                        */
/*     Copy parameters to safe local copy, and clear inteerupt flag   */
/*     Start to draw                                                  */
/*     Draw                                                           */
/*       (Draw checks the 'interrupt draw' flag; if seen set then     */
/*       stops drawing after current row and showing completed work)  */
/*     end loop                                                       */
/*                                                                    */
/*--------------------------------------------------------------------*/
#define VGASTAR 43                 /* VGA 'starlight' brightness */
#define BGASTAR 63                 /* BGA/XGA value              */

HAB   habGlobe;                    /* our anchor block */
HMQ   hmqGlobe;                    /* .. and message queue */
void glober(void)
 {
 static unsigned int mask[8]={0x80, 0x40, 0x20, 0x10,
                              0x08, 0x04, 0x02, 0x01};
 static int coloured=FALSE;        /* colour processor called */
 static int valstar=VGASTAR;       /* starlight value */

 unsigned char huge *row;          /* work; start of row */
 unsigned char huge *c;            /* work; character within row */
 RECTL rcl;                        /* work */
 int  r;                           /* radius (r*2+1==diameter) */
 long r2x;                         /* radius squared, and a bit */
 long need;                        /* bytes needed for in-store bytemap */
 HBITMAP mbm, sbm, obm;            /* local copies */
 HPS     mps, sps;                 /* .. */
 HDC     mdc, sdc;                 /* memory device contexts */
 SIZEL   size;                     /* work */
 int  newsize;                     /* we are re-sizing globe */
 int  newsprite;                   /* we need all new sprites */
 int  newgeog;                     /* new geography needed */
 int  startup=TRUE;                /* starting up .. first grid */
 int  sun;                         /* showing sunlight */
 int  shade;                       /* shaded (3-D) */
 int  space;                       /* spacelight (SUN will be set) */
 int  star;                        /* starlight (SUN will be set) */
 long twi;                         /* twilight (in degrees*1000) */
 long tz;                          /* total timezone offset (secs) */
 int  glon10, glat10;              /* grid settings */
 int  glon15, glat15;              /* .. */
 int  glon30, glat30;              /* .. */
 int  garctic, gtropic;            /* .. */
 int  geq;                         /* .. */
 long gcoli;                       /* .. colour index */
 long griddeg;                     /* degrees per grid dot */
 int  morelat, morelon;            /* .. grids being added */
 int  lesslat, lesslon;            /* .. grids being removed */
 long gmarks, gdraws;              /* known-about marks, graphics */
 int  gmarkers, glabels, ggraphics;/* known settings */
 int  gclocks, gcivil, gcday;      /* .. */
 int  partback;                    /* background is not yet drawn */
 int  parttext;                    /* text labels to draw */
 int  partgraphics;                /* graphics to draw */
 int  partlat, partlon;            /* grid parts not yet drawn */
 int  partdrawn;                   /* globe is not yet drawn */
 int  somesprite;                  /* some sprite stuff drawn */
 int  restart;                     /* globe restart: rows drawn */
 unsigned long urc;                /* work */
 long lrc;                         /* work */
 long lon, lat;                    /* longitude & latitude */
 long vlon, vlat;                  /* view longitude & latitude */
 long vlatcos, vlatsin;            /* view latitude,  sine & cosine */
 long lightx, lighty, lightz;      /* direction vector of light source */
 long bcoli;                       /* local copy of background colour */
 long lcoli, scoli;                /* local copies and land & sea colours */
 COLSTRUCT col;                    /* colour structure */
 int  collev[MAXCOLOURS];          /* colour levels array */
 int  cr,  cg,  cb;                /* index of colour r, g, b */
 int  cdr, cdg, cdb;               /* index of dark colours r, g, b */
 int  cc,  cy,  cp;                /* index of colour c, y, p */
 int  cdc, cdy, cdp;               /* index of dark colours c, y, p */
 int  cw, cdw;                     /* index of white, dark white */
 int  cblk;                        /* index of colour black */
 long start, stop;                 /* timing */
 long lasttime, newtime;           /* timing */
 int  lasty;                       /* slice recording */
 int  mapx, mapy;                  /* indexes into landmap */
 unsigned char huge *landmap;      /* -> landmap */
 unsigned char huge *pbyte;        /* -> landmap byte */
 int  landmaprupx;                 /* landmap rounded-up-X */
 int  tilt;                        /* globe is tilted */
 int  x32, y32, z32, r32;          /* for optimizations */
 long r2x1024, r2xmy2;             /* .. */
 long x32000;                      /* .. */
 long y32vlatcos, y32vlatsin;      /* .. */
 /* Error diffusion variables */
 int  sr, sg, sb;                  /* error   r g b temporary sums */
 int  er, eg, eb;                  /* error   r g b values */
 int  *pe=NULL;                    /* pointer to whole error array */
 int  *pr, *pg, *pb;               /* pointers to error sub-arrays */
 int  newdiam;                     /* new diameter (iff NEWSIZE) */
/*----- end of dcls --------------------------------------------------*/

 habGlobe=WinInitialize(NULL);     /* so thread can use GPI */
 hmqGlobe=WinCreateMsgQueue(habGlobe,0);
 WinCancelShutdown(hmqGlobe,TRUE); /* allow shutdown */

 /* First we do one-off initialization things.  These need no screen  */
 /* or presentation space.  When complete, let the main thread know.  */
 /* Paint does not have a valid GLOBEPS and so won't use the shadow   */
 /* bitmap */

 landmaprupx=4*((LANDMAPDIMX+31)/32); /* rounded-up landmap x dimension */

 landmap=globemap();               /* allocate and unpack the map */

 DosPostEventSem(semhavemap);      /* tell main thread: we have map */

 /* Now wait until screen and message loop is available */
 DosWaitEventSem(semhavescreen, SEM_INDEFINITE_WAIT);
 /* ... we can now report errors */
 if (landmap==NULL) {
   WinPostMsg(hwndClient, UM_NOMAP, NULL, NULL);
   WinAlarm(HWND_DESKTOP, WA_ERROR);
   WinTerminate(habGlobe);         /* done with anchor block */
   return;}                        /* hopeless -- give up */

 glon10=FALSE;  glat10=FALSE;      /* initialise */
 glon15=FALSE;  glat15=FALSE;
 glon30=FALSE;  glat30=FALSE;  geq=FALSE;
 garctic=FALSE; gtropic=FALSE;
 gcoli=0;
 vlat=BADLONG;  vlon=BADLONG;      /* (unknown) */
 partlat=TRUE;   partlon=TRUE; partdrawn=TRUE;
 parttext=FALSE; partback=FALSE; partgraphics=FALSE;
 newsprite=TRUE; newgeog=TRUE;
 gmarks=0; gmarkers=TRUE; glabels=TRUE; gdraws=0; ggraphics=TRUE;
 gclocks=TRUE; gcivil=civiltime; gcday=clockday;
 bcoli=backcoli; lcoli=landcoli; scoli=seacoli;
 space=FALSE; sun=FALSE; shade=FALSE; star=FALSE; twi=0; tz=0;
 restart=0;                        /* rows drawn */

 /*====== Here starts the main loop =============================*/
 for (;;) {                        /* FOREVER.  (Not indented)   */
 if (stopdraw && diag) printf("Interrupt!\n");

 /* Reset state flags (rather than in protected code) */
 newsize=FALSE;
 morelon=FALSE; lesslon=FALSE;
 morelat=FALSE; lesslat=FALSE;

 /* Now wait until asked to do a new draw */
 DosWaitEventSem(semneeddraw, SEM_INDEFINITE_WAIT);
 DosResetEventSem(semneeddraw, &posted);          /* and set interlock again */

 /* ----- Protected Code ----------------------------------------*/
 /* In this section of code, we inspect and copy the governing   */
 /* parameters.  The main thread is prevented from running while */
 /* we do this.                                                  */
 /*                                                              */
 /* See if resizing .. if so, take note (but we allow a size     */
 /* change from even to odd, of -1, for common squaring case).   */
 DosEnterCritSec();                               /* protect */
 newdiam=(int)lmin(clientx,clienty);              /* max diameter */
 newdiam=(int)lmin(newdiam, MAXRADIUS*2+1);       /* clamp upper bound */
 newdiam=newdiam-(int)(margin*(long)newdiam/100000L)*2;/* less margin */
 if (newdiam==(newdiam/2)*2) newdiam--;           /* to odd below if needed */
 if (newdiam<0) newdiam=0;                        /* clamp */

 if (diameter!=newdiam                            /* new globe size */
  || diameter==0                                  /* no globe yet */
  || globeps==NULL) newsize=TRUE;                 /* .. ditto (B&B) */

 /* Here could apply sun-locked lat/lon */

 /* Take copy of other viewing parameters, as required */
 /* .. take special note of whether we are adding or removing grids */
 if ((glon10!=gridlon10) || (glon15!=gridlon15) || (glon30!=gridlon30))
   {  /* some change to grid longitudes */
   if ((glon10 && !gridlon10)
    || (glon15 && !gridlon15)
    || (glon30 && !(gridlon10 || gridlon15 || gridlon30))) lesslon=TRUE;
    else morelon=TRUE;
   } /* LON change */

 if ((glat10!=gridlat10) || (glat15!=gridlat15) || (glat30!=gridlat30)
  || (garctic!=gridarctic) || (gtropic!=gridtropic) || (geq!=grideq))
   {  /* some change to grid latitudes */
   if ((glat10 && !gridlat10) || (glat15 && !gridlat15)
    || (glat30 && !(gridlat10 || gridlat15 || gridlat30))
    || (geq    && !(gridlat30 || gridlat10 || gridlat15 || grideq))
    || (garctic && !gridarctic)
    || (gtropic && !gridtropic)) lesslat=TRUE;
    else morelat=TRUE;
   } /* LAT change */

 if (gcoli!=gridcoli) {
   if (gridlon10 || gridlon15 || gridlon30) morelon=TRUE;
   if (gridlat10 || gridlat15 || gridlat30
               || gridarctic || gridtropic) morelat=TRUE;
   } /* grid colour change */

 if  (lesslon || lesslat
  || (gmarkers  && !markers)
  || (glabels   && !labels)
  || (ggraphics && !graphics)
  || (gclocks!=clocks) || (gcivil!=civiltime) || (gcday!=clockday)
  || (clockmarks>0 && clocks)      /* clock updates likely */
  || redrawsprite)          {
   /* changing or removing features */
   newsprite=TRUE;                 /* all sprite data needs redraw */
   redrawsprite=FALSE;
   }
  else {                           /* maybe adding features */
   if (morelon) partlon=TRUE;
   if (morelat) partlat=TRUE;
   if ((gmarks  !=marks)           /* adding marks */
    || (gmarkers!=markers)         /* .. etc. */
    || (glabels !=labels)) parttext=TRUE;
   if ((ggraphics!=graphics)       /* adding graphics */
    || (gdraws   !=draws)) partgraphics=TRUE;
   }
 gmarks=marks; gmarkers=markers; glabels=labels; gclocks=clocks;
 gdraws=draws; ggraphics=graphics;
 glon10=gridlon10; glon15=gridlon15; glon30=gridlon30;
 glat10=gridlat10; glat15=gridlat15; glat30=gridlat30;
 garctic=gridarctic; gtropic=gridtropic; geq=grideq;
 gcoli=gridcoli;

 if (bcoli!=backcoli) {            /* background colour changes */
   partback=TRUE;
   bcoli=backcoli;}

 if (newsize || viewlon!=vlon || viewlat!=vlat
  || redrawall) {                  /* total redraw please */
   partdrawn=TRUE;                 /* need new globe.. */
   newgeog=TRUE;                   /* .. */
   restart=0;                      /* .. from the top */
   newsprite=TRUE;                 /* .. and new sprite layer, too */
   vlon=viewlon;  vlat=viewlat;    /* view direction */
   drawvlon=vlon; drawvlat=vlat;   /* make acccessible */
   redrawall=FALSE;
   }
 else if (shade!=threed || sun!=sunlight || twi!=twilight
  || space!=spacelight || star!=starlight
  || lcoli!=landcoli   || scoli!=seacoli
  || tz!=(gmtoffset+summer-useroffset) ) {
   partdrawn=TRUE;                 /* need new picture */
   restart=0;                      /* draw the whole globe */
   }

 sun  =sunlight;                   /* sunlight flag */
 shade=threed;                     /* shading  flag */
 space=spacelight;                 /* space lighting (SUN will be set) */
 star =starlight;                  /* star lighting (SUN & SPACE will be set) */
 twi  =twilight;                   /* twilight (degrees*1000) */
 lcoli=landcoli;                   /* land colour */
 scoli=seacoli;                    /* sea colour */
 tz   =gmtoffset+summer-useroffset;/* actual timezone total offset */
 gcivil=civiltime;                 /* time am/pm or 24 hour */
 gcday =clockday;                  /* day flag */

 if (sun) partdrawn=TRUE;          /* need redraw always if SUN */
 /* (but we risk allowing restart, for quicker response) */

 stopdraw=FALSE;                   /* clear 'interrupt' flag */
 DosExitCritSec();
 /* ----- End of Protected Code --------------------------------- */

 if (diag) {
   printf("More lon/lat, less lon/lat: %i%i %i%i\n",
          morelon, morelat, lesslon, lesslat);
   printf("Newsize/Newsprite/Newgeog: %i%i%i\n",
          newsize, newsprite, newgeog);
   printf("Partback/partlat/lon/text/graphics/drawn: %i%i%i%i%i%i\n",
          partback, partlat, partlon, parttext, partgraphics, partdrawn);
   printf("Sun, Space, 3-D: %i%i%i\n", sun, space, shade);
   }

 if (newsize) {
   /* MAXRADIUS is the maximum ball radius.  We find the largest  */
   /* odd diameter that will fit, subject to a maximum radius.    */
   /* This allows for large globes while ensuring no overflow in  */
   /* later calculations.  Somewhat larger radii are very likely  */
   /* possible, but would require additional analysis.            */
   r=(int)lmin((newdiam-1)/2,MAXRADIUS);/* true radius */
   if (r<=0) {alert("Globe radius would be zero pels");
     diameter=0;                        /* indicate no globe */
     continue;}
   diameter=r+r+1;                      /* actual diameter used */
   /* r2=(long)r*(long)r; */            /* radius squared (not used) */
   r2x=(long)r*(long)(r+1);             /* radius squared and a bit */
   r2x1024=r2x*1024L;                   /* ditto R2X1024 (a square) */
   r32=(int)lsqrt(r2x1024);             /* best accurate R32 */
   drawradius=r32;                      /* copy to shared */
   trueradius=r*32L;                    /* true R32, shared */
   rupx=4L*((diameter+3L)/4L);          /* rounded-up DIAMETER to a */
                                        /* .. multiple of 4 bytes   */
   if (diag) {
     printf("Newdiam, diameter %i %i\n", newdiam, diameter);
     }

   /* Get rid of any existing bitmaps and work areas */
   dropbitmaps();

   if (flatmap!=NULL) {            /* already have work area */
     hfree(flatmap); flatmap=NULL;}
   if (merge!=NULL) {              /* already have merge area */
     free(merge); merge=NULL;}
   if (pe!=NULL) {                 /* have error diffusion array */
     free(pe); pe=NULL;}
   if (diag) printf("New size: all cleared\n");
   } /* new size please */

 /* Make a globe-sized PM bitmap in Memory device context */
 if (globebitmap==NULL) {          /* need to allocate bitmaps */
   POINTL p;
   /* Create device contexts for the bitmaps we need */
   if (globedc==NULL) { /* need a 'memory' device */
     mdc=DevOpenDC(habGlobe, OD_MEMORY,"*", 0L, 0L, NULL); /* Open device */
     if (mdc==DEV_ERROR) {
       lrc=(signed)WinGetLastError(habGlobe);
       alert("Error %lxx trying to open memory device", lrc);
       continue;}
     globedc=mdc;
     }
    else {
     if (diag) printf("Already have GLOBEDC\n");
     mdc=globedc;}
   if (spritedc==NULL) { /* need a 'memory' device */
     sdc=DevOpenDC(habGlobe, OD_MEMORY,"*", 0L, 0L, NULL); /* Open device */
     if (sdc==DEV_ERROR) {
       lrc=(signed)WinGetLastError(habGlobe);
       alert("Error %lxx trying to open memory device 2", lrc);
       continue;}
     spritedc=sdc;
     }
    else {
     if (diag) printf("Already have SPRITEDC\n");
     sdc=spritedc;}

   size.cx=diameter; size.cy=diameter;  /* Bitmap size */
   /* make two presentation spaces */
   mps=GpiCreatePS(habGlobe, mdc, &size, PU_PELS+GPIT_MICRO+GPIA_ASSOC);
   if (mps==GPI_ERROR) {
     lrc=(signed)WinGetLastError(habGlobe);
     alert("Error %lxx trying to create mapper PS", lrc);
     continue;}
   sps=GpiCreatePS(habGlobe, sdc, &size, PU_PELS+GPIT_MICRO+GPIA_ASSOC);
   if (sps==GPI_ERROR) {
     lrc=(signed)WinGetLastError(habGlobe);
     GpiAssociate(mps,(HDC)NULL); GpiDestroyPS(mps);   /* cleanup */
     alert("Error %lxx trying to create sprite PS", lrc);
     continue;}

   /* Create the Bitmaps -- these could also initialise/convert format */
   bmapinfo.cbFix=sizeof(BITMAPINFOHEADER2); /* size of infoheader */
   bmapinfo.cx=(USLONG)diameter;   /* size of bitmap */
   bmapinfo.cy=(USLONG)diameter;
   bmapinfo.cPlanes   = 1L;        /* number of planes */
   bmapinfo.cBitCount = 8L;        /* up to 24-bit, RGB format */
   mbm=GpiCreateBitmap(mps, &bmapinfo, 0L, NULL, NULL);
   if (mbm==GPI_ERROR) {
     lrc=(signed)WinGetLastError(habGlobe);
     GpiAssociate(mps,(HDC)NULL); GpiDestroyPS(mps);   /* cleanup */
     GpiAssociate(sps,(HDC)NULL); GpiDestroyPS(sps);   /* .. */
     alert("Error %lxx trying to create mapper bitmap", lrc);
     continue;}
   /* Select the bitmap "into" the memory device context */
   obm=GpiSetBitmap(mps, mbm);
   if (obm==HBM_ERROR) {
     lrc=(signed)WinGetLastError(habGlobe);
     GpiAssociate(mps,(HDC)NULL); GpiDestroyPS(mps);   /* cleanup */
     GpiAssociate(sps,(HDC)NULL); GpiDestroyPS(sps);   /* .. */
     alert("Error %lxx trying to select mapper bitmap", lrc);
     continue;}
   globebitmap=mbm;
   /* The sprite-plane is only 4-deep */
   bmapinfo.cBitCount = 4L;        /* up to 24-bit, RGB format */
   sbm=GpiCreateBitmap(sps, &bmapinfo, 0L, NULL, NULL);
   bmapinfo.cBitCount = 8L;        /* back to default */
   if (sbm==GPI_ERROR) {
     lrc=(signed)WinGetLastError(habGlobe);
     GpiAssociate(mps,(HDC)NULL); GpiDestroyPS(mps);   /* cleanup */
     GpiAssociate(sps,(HDC)NULL); GpiDestroyPS(sps);   /* .. */
     alert("Error %lxx trying to create sprite bitmap", lrc);
     continue;}
   /* Select the bitmap "into" the memory device context */
   obm=GpiSetBitmap(sps, sbm);
   if (obm==HBM_ERROR) {
     lrc=(signed)WinGetLastError(habGlobe);
     GpiAssociate(mps,(HDC)NULL); GpiDestroyPS(mps);   /* cleanup */
     GpiAssociate(sps,(HDC)NULL); GpiDestroyPS(sps);   /* .. */
     alert("Error %lxx trying to select sprite bitmap", lrc);
     continue;}
   spritebitmap=sbm;
   newsprite=TRUE;                 /* need new sprite background */

   /* clear the shadow bitmap, in case of repaint before drawn */
   GpiSetColor(mps, bcoli);
   p.x=0;          p.y=0;          GpiMove(mps,&p);
   p.x=diameter-1; p.y=diameter-1; GpiBox(mps, DRO_FILL, &p, 0L, 0L);
   globeps=mps;                    /* shadow bitmap now safe */

   /* Now set any visible surround on the screen to background */
   /* We do this as a full screen rectangle write, for simplicity; */
   /* this also makes for smoothness new size drawing from 'scratch' */
   WinQueryWindowRect(hwndClient, &rcl);
   WinFillRect(cps, &rcl, bcoli);
   startup=TRUE;                   /* blank screen startup */

   /* Now copy the bitmap to memory, to give us a colour table and    */
   /* also clear the non-globe area very rapidly.                     */
   if (flatmap==NULL) {            /* we need a new work area */
     /* Allocate working storage to construct the globe picture */
     need=rupx*diameter;           /* total bytes */
     flatmap=(char huge *)halloc(need, (size_t)1);
     if (flatmap==NULL) {alert("Not enough memory for flatmap array\n"); continue;}
     } /* need new work area (flatmap) */
   bmap.head.cbFix=sizeof(BITMAPINFOHEADER2); /* size of infoheader */
   bmap.head.cx=(USLONG)diameter;  /* size of bitmap */
   bmap.head.cy=(USLONG)diameter;  /* .. (not needed for Query) */
   bmap.head.cPlanes   = 1L;       /* number of planes */
   bmap.head.cBitCount = 8L;       /* up to 24-bit, RGB format */
                                   /* colour table is set by call */
   DosRequestMutexSem(sembitmap, SEM_INDEFINITE_WAIT);
   lrc=GpiQueryBitmapBits(mps, 0L, (long)diameter, flatmap, (PBITMAPINFO2)(PVOID)&bmap);
   DosReleaseMutexSem(sembitmap);
   if (lrc==GPI_ALTERROR) {
     lrc=(signed)WinGetLastError(habGlobe);
     alert("Error %li from GpiQueryBitmapBits", lrc);
     continue;}
   /* map is now cleared to all background */
   partback=FALSE;                 /* No need for this */
   partdrawn=TRUE;                 /* B&B */

   if (!coloured) {                /* need the colour analyzer */
     int i, j, rc;
     rc=globecol(bmap.rgb, &col);
     if (rc!=0) printf("Bad RC=%i from GLOBECOL\n", rc);
     /* Set up common levels array (max of R, G, B for each) */
     for (i=0; i<col.colours; i++)
       collev[i]=max((int)bmap.rgb[i].bRed,
        max((int)bmap.rgb[i].bGreen, (int)bmap.rgb[i].bBlue));
     coloured=TRUE;
     cr  =col.rnear[COLHI];
     cdr =col.rnear[COLHI/2];
     cg  =col.gnear[COLHI];
     cdg =col.gnear[COLHI/2];
     cb  =col.bnear[COLHI];
     cdb =col.bnear[COLHI/2];
     cc  =col.cnear[COLHI];
     cdc =col.cnear[COLHI/2];
     cy  =col.ynear[COLHI];
     cdy =col.ynear[COLHI/2];
     cp  =col.pnear[COLHI];
     cdp =col.pnear[COLHI/2];
     cw  =col.wnear[COLHI];        /* white */
     cdw =col.wnear[COLHI/2];      /* grey  */
     cblk=col.wnear[0];            /* black */
     if (diag && test) {
       printf("Cols: %i %i %i %i %i %i %i %i %i %i %i %i %i %i\n",
          cw, cdw, cp, cdp, cy, cdy, cc, cdc, cr, cdr, cg, cdg, cb, cdb);
       j=col.colours;  /* use J */
       printf("%i colours total available\n", j);
       /***
       printf("Yellow-near table:\n");
       for (i=0; i<=COLHI; i++) {
         j=col.ynear[i];
         printf("%03i: %03i %i %i %i\n", i, j, (int)bmap.rgb[j].bRed,
            (int)bmap.rgb[j].bGreen, (int)bmap.rgb[j].bBlue);
         }
        ***/
       }

     /* If not VGA then use brighter starlight */
     if (col.colours>16) valstar=BGASTAR;
     }  /* colours */
   } /* new bitmap */

 /* Allocate the merge array, if none there */
 if (merge==NULL) {
   /* We allocate one linesworth -- could make wider later */
   need=rupx /* *lines */ ;        /* total bytes */
   merge=(char *)malloc((size_t)need);
   if (merge==NULL) {alert("Not enough memory for merge array"); continue;}
   } /* allocating merge array */

 /* If newsprite then (a) clear sprite array */
 /* (b) indicate redraw all features         */
 if (newsprite) {
   POINTL p;
   /* clear the sprite bitmap to background */
   GpiSetColor(sps, CLR_PALEGRAY);
   p.x=0;          p.y=0;          GpiMove(sps,&p);
   p.x=diameter-1; p.y=diameter-1; GpiBox(sps, DRO_FILL, &p, 0L, 0L);
   spriteps=sps;                   /* sprite bitmap now safe */
   /* Determine the index of the sprite transparent colour... */
   /* (BMAP header will be set already) */
   GpiQueryBitmapBits(sps, 0L, 1L, merge, (PBITMAPINFO2)(PVOID)&bmap);
   sback=*merge;                   /* first pel is background colour */
   partlon=TRUE; partlat=TRUE; parttext=TRUE;     /* draw 'em all */
   partgraphics=TRUE;                             /* .. */
   newsprite=FALSE;                /* we're happy */
   }

 if (stopdraw) continue;           /* give up */

 /* Allocate the error array, if none there */
 if (pe==NULL) {
   /* We always allocate it, as even if !shade we may error diffuse */
   int makesize, i;
   /* The size we allocate is the 2*radius+1 for each colour. plus */
   /* two bytes (one at each end) which allow us to ignore end     */
   /* conditions */
   makesize=r+r+3;
   pe=(int *)malloc((size_t)(sizeof(int)*makesize*3));
   if (pe==NULL) {alert("Not enough memory for error array"); continue;}
   /* Clear whole error array to 0 */
   for (i=0; i<(int)makesize*3; i++) *(pe+i)=0;
   /* set up the sub-array pointers.  These are offset by 1, so in  */
   /* effect can be indexed starting at -1, together with a further */
   /* offset of R, so effectively -(r+1) to (r+1)                   */
   pr=pe+r+1; pg=pr+makesize; pb=pg+makesize;
   /* (Note that we include an array for Red, even though initially */
   /* we don't use it) */
   } /* allocating error array */

 if (stopdraw) continue;           /* give up */

 /* --------------------------------------------------------------- */
 /* OK, we now have:                                                */
 /*   GLOBEBITMAP  GLOBEPS  -- screen shadow PM bitmap (8-bit)      */
 /*   SPRITEBITMAP SPRITEPS -- 'features' PM bitmap (4-bit)         */
 /*   FLATMAP               -- map-holder in memory (8-bit)         */
 /*   MERGE                 -- one-line merge working area (8-bit)  */
 /*   PE (PR, PG, PB)       -- error diffusion arrays (ints+edges)  */
 /* --------------------------------------------------------------- */
 DosSetPrty(PRTYS_THREAD, PRTYC_REGULAR, 0, tidg);  /* To normal */

 /* ----- If need partial background, draw it now ----- */
 /* (This code only used when background changes after first draw) */
 if (partback) {                   /* background needed */
   unsigned char huge *cend;       /* work; last character within row */
   unsigned char nback;            /* new CBACK */
   int y;                          /* Y .. */
   long y2;                        /* .. squared */
   int  xf;                        /* first x in row */
   POINTL p;

   /* First determine the index of the new background colour... */
   DosRequestMutexSem(sembitmap, SEM_INDEFINITE_WAIT);
   GpiSetColor(mps, bcoli);
   p.x=0; p.y=0; GpiMove(mps,&p);
   GpiBox(mps, DRO_FILL, &p, 0L, 0L); /* .. at 0,0 again */
   /* Now copy a line of bitmap to memory, to get the index */
   /* (BMAP header will be set already) */
   GpiQueryBitmapBits(globeps, 0L, 1L, merge, (PBITMAPINFO2)(PVOID)&bmap);
   DosReleaseMutexSem(sembitmap);
   nback=*merge;                   /* first pel is background colour */

   /* This loop is related to those below; see notes below for details */
   row=flatmap+rupx*(diameter-1);  /* -> start of top row of globe */
   y2=(long)(r+1)*(long)(r+1);     /* set initial Y2 */
   for (y=r; y>=-r; y--, row=row-rupx) {
     y2=y2-(y+y+1);                /* calculate y**2 */
     xf=(int)lsqrt(r2x-y2);        /* maximum +/- X value */
     /* Left segment... */
     c=row;                        /* first byte to draw within row */
     cend=row+r-xf-1;              /* last  byte to draw within row */
     for (c=c; c<=cend; c++) *c=nback;
     /* Right segment... */
     c=row+r+xf+1;                 /* first byte to draw within row */
     cend=row+diameter-1;          /* last  byte to draw within row */
     for (c=c; c<=cend; c++) *c=nback;
     if (stopdraw) break;          /* give up inner loop */
     } /* y */
   if (stopdraw) continue;         /* give up */
   if (diag) printf("New map background done\n");
   /* Now copy whole square-as-is to the memory bitmap and screen */
   if (startup || !newgeog) domerge((int)diameter-1,0); /* whole screen */
    else domerge((int)diameter-1, (int)(diameter-restart-1));
   /* now invalidate whole to ensure sides/top filled */
   WinQueryWindowRect(hwndClient, &rcl);
   WinInvalidateRect(hwndClient, &rcl, FALSE);
   if (diag) printf("Partial background merged/drawn\n");
   partback=FALSE;                 /* OK, we have good background */
   } /* new partial background */
 if (stopdraw) continue;           /* give up */

 /* ----- Add any grid, etc.  This is slow, so is interruptible ----- */
 /* and may have partial draws, if no map yet on screen.              */
 /* Grid and features are drawn to sprite plane, which is then        */
 /* merged with map as map is drawn.  Partial draws only take place   */
 /* for the very first time (when no map).                            */
 somesprite=FALSE;                 /* none found yet */

 /* Set the grid spacing, and clear Drawn flags, so all markers and */
 /* graphics will be redrawn */
 if (partlon || partlat) {
   struct llmark *pll;                  /* current mark */
   struct drawlist *pd;                 /* current draw */
   if (diameter>=360)       griddeg=1;  /* spacing of grid dots */
    else if (diameter>=180) griddeg=2;
    else                    griddeg=5;

   parttext=TRUE;                  /* may overwrite text .. */
   partgraphics=TRUE;              /* .. and graphics */
   for (pll=markbase; pll!=NULL; pll=pll->next) pll->drawn=FALSE;
   for (pd =drawbase; pd !=NULL; pd =pd ->next) pd ->drawn=FALSE;

   if (diag) start=(signed long)WinGetCurrentTime(habGlobe);

   if (partlon                     /* meridians only if needed */
     && r>=32) {                   /* .. and not tiny */
     long latstep, lonstep;        /* for grid drawing */
     long x, y, z;                 /* .. */
     int  rc;                      /* work */
     POINTL at;
     int features;
     features=FALSE;
     lasttime=start;
     if (glon10 || glon15 || glon30) {  /* draw meridians */
       if (glon10)       lonstep=10000; /* every 10 degrees */
        else if (glon15) lonstep=15000; /* every 15 degrees */
        else             lonstep=30000; /* .. or 30 */
       latstep=1000L*griddeg;           /* how often a dot */
       GpiSetColor(sps, gcoli);         /* colour for grid */
       for (lat=90000L; lat>=-90000L; lat=lat-latstep) {
         if (stopdraw) break;
         features=TRUE;                 /* something to draw */
         /* Draw from top for best visual effect */
         for (lon=-180000L; lon<180000L; lon=lon+lonstep) {
           rc=ll2pel(lon, lat, FALSE, &x, &y, &z);
           if (rc==0) {
             at.x=x+r; at.y=y+r; GpiMove(sps, &at); GpiLine(sps, &at);}
           } /* lon */
         } /* lat */
       if (diag) printf("Meridians drawn\n");
       } /* Longitude lines */

     if (!stopdraw && features) {
       if (startup) domerge((int)diameter-1,0); /* whole screen */
       somesprite=TRUE;                 /* have something to draw */
       partlon=FALSE;                   /* OK, these look good */
       } /* not interrupted */
     } /* need meridians */
   if (stopdraw) continue;              /* give up */

   if (partlat                          /* parallels only if needed */
     && r>=32) {                        /* .. and not tiny */
     long latstep, lonstep;             /* for grid drawing */
     long x, y, z;                      /* .. */
     int  rc;                           /* work */
     POINTL at;
     long latlist[4];
     int lats, l, features;
     long latdiv;
     lasttime=start;
     features=FALSE;
     GpiSetColor(sps, gcoli);                /* colour for grid */
     if (glat10 || glat15 || glat30 || geq) {/* draw parallels */
       if (glat10)       latstep=10000L;     /* every 10 degrees */
        else if (glat15) latstep=15000L;     /* every 15 degrees */
        else if (glat30) latstep=30000L;     /* every 30 degrees */
                    else latstep=90000L;     /* 90's */
       for (lat=90000L; lat>=-90000L; lat=lat-latstep) {
         if (stopdraw) break;
         features=TRUE;
         latdiv=lcos(lat); if (latdiv==0) latdiv=1L;
         lonstep=1000000L*griddeg/latdiv;    /* how often a dot */
         for (lon=-180000L; lon<180000L; lon=lon+lonstep) {
           rc=ll2pel(lon, lat, FALSE, &x, &y, &z);
           if (rc==0) {
             at.x=x+r; at.y=y+r; GpiMove(sps, &at); GpiLine(sps, &at);}
           } /* lat */

         } /* lon */
       if (diag) printf("Parallels drawn\n");
       } /* Latitude lines */

     /* Add tropics and artics, if required */
     lats=0;
     if (garctic) {
       features=TRUE;
       latlist[0]=-66500L; latlist[1]=66500L;
       lats=2;}
     if (gtropic) {
       features=TRUE;
       latlist[lats]=-23443L; latlist[lats+1]=23443L;
       lats=lats+2;}
     for (l=0; l<lats; l++) {
       if (stopdraw) break;
       lat=latlist[l];
       latdiv=lcos(lat); if (latdiv==0) latdiv=1L;
       lonstep=2500000L*griddeg/latdiv;      /* rarer dots */
       for (lon=-180000L; lon<180000L; lon=lon+lonstep) {
         rc=ll2pel(lon, lat, FALSE, &x, &y, &z);
         if (rc==0) {
             at.x=x+r; at.y=y+r; GpiMove(sps, &at); GpiLine(sps, &at);}
         } /* lat */
       /* Don't worry about timed redraw, because these lines fast */
       } /* lon */

     if (!stopdraw && features) {
       if (startup) domerge((int)diameter-1,0);   /* whole screen */
       somesprite=TRUE;                 /* have something to draw */
       partlat=FALSE;                   /* OK, these look good */
       if (diag && lats>0) printf("Circles drawn\n");
       } /* not interrupted */
     } /* need parallels */

   if (stopdraw) continue;           /* give up */
   if (diag) {
     stop=(signed long)WinGetCurrentTime(habGlobe);
     printf("Grid draw time: %lims\n", (long)stop-start);
     }
   } /* Drawing part grids */

 /* ----- Add graphics --------------------------------------------- */
 if (partgraphics                  /* something to draw */
  && (drawbase!=NULL ||
      trackbase!=NULL)
  && diameter>64                   /* and not tiny (active minimized) */
  && (ggraphics)) {                /* and something to show */
   struct drawlist *pd;            /* work */
   struct llmark   *pll;           /* current mark */
   long clat=0, clon=0;            /* current point */
   long curx32, cury32, curz32;    /* current point, in pels*32 */
   long tox32, toy32, toz32;       /* 'to' pel coordinates, in pels*32 */
   struct drawlist *drawlists[2];  /* lists to process */
   int list;                       /* counter */

   parttext=TRUE;                  /* may overwrite text .. */
   for (pll=markbase; pll!=NULL; pll=pll->next) pll->drawn=FALSE;
                                   /* (May be a repeat, but low-cost) */

   if (diag) start=(signed long)WinGetCurrentTime(habGlobe);
   somesprite=TRUE;                /* have something to merge */
   ll2pel(clat, clon, TRUE, &curx32, &cury32, &curz32);/* Initial point */

   drawlists[0]=drawbase;          /* Two lists to process */
   drawlists[1]=trackbase;
   for (list=0; list<2; list++) {  /* each graphic list */
     for (pd=drawlists[list]; pd!=NULL; pd=pd->next) { /* each drawlist item */
       /* New point coordinates in Pel terms */
       ll2pel(pd->lon, pd->lat, TRUE, &tox32, &toy32, &toz32);
       if (pd->order==DRAW_LINE
        && (!pd->drawn || list>0)) {    /* DRAWN only valid for DRAWbase */
         pd->drawn=TRUE;           /* optimize -- assumes new at end? */
         GpiSetColor(spriteps, pd->coli); /* at this colour */
         drawgc(curx32, cury32, curz32, tox32, toy32, toz32,
                (pd->lon+clon)/2, (pd->lat+clat)/2, (long)r, (long)r32);
         } /* draw great circle */
       clat=pd->lat; clon=pd->lon;
       curx32=tox32; cury32=toy32; curz32=toz32;
       } /* item in list */
     } /* each list */

   if (!stopdraw) {
     if (startup) domerge((int)diameter-1,0); /* whole screen */
     partgraphics=FALSE;           /* OK, looks good */
     if (diag) printf("Graphics drawn\n");
     } /* not interrupted */

   if (diag) {
     stop=(signed long)WinGetCurrentTime(habGlobe);
     printf("Graphics draw time: %lims\n", (long)stop-start);
     }
   } /* Graphics to draw */

 /* ----- Add labels and markers ----------------------------------- */
 if (parttext && markbase!=NULL    /* something to add */
  && diameter>64                   /* and not tiny (active minimized) */
  && (glabels || gmarkers || gclocks)){ /* and something to show */
   #define MAXLAB (MAXTEXT+8)
   POINTL at;
   POINTL xs[MAXLAB+1];            /* up to N-char clock-labels */
   char   text[MAXLAB+1];          /* .. */
   FONTMETRICS fm;                 /* metrics structure */
   static FATTRS fat;              /* static so 0's */
   SIZEF cbox;                     /* character box */
   long len;                       /* text length */
   long lcid, fcoli;               /* logical font ID, color index */
   long timenow;                   /* this moment (seconds) */
   long x, y, z;                   /* pel coordinates */
   struct llmark *pll;             /* current mark */
   int havetext;                   /* have label or clock on this mark */
   struct tm *timeinfo;            /* work */
   int rc;

   sps=spriteps;                   /* local copy */
   somesprite=TRUE;                /* have something to draw */

   /* Now set up the fonts (likely we will use all)... */
   if (glabels || gclocks) {       /* maybe text, so set fonts */
     struct logfont *plf;          /* work */
     long i;
     fat.usRecordLength=sizeof(fat);
     fat.usCodePage=850;
     fat.lMatch=0;                 /* 0 means find nearest */
     plf=fontbase;                 /* -> start of chain */
     for (i=1; i<=fonts; i++) {
       strcpy(fat.szFacename, plf->face);
       fat.lMaxBaselineExt=plf->fsize; /* size */
       lrc=GpiCreateLogFont(sps, NULL, i, &fat);
       /* no action if error .. will revert to default font */
       plf=plf->next;}
     } /* labels */

   /* Get time in minutes, mod 24 hours, if maybe clocks */
   if (gclocks && gmtoffset!=BADLONG) {
     time(&timenow);                    /* get time in secs */
     /* And now retro-calculate true GMT -- see SETSUN() for details */
     timeinfo=localtime(&timenow); timenow=timenow-_timezone;
     if (timeinfo->tm_isdst>0) timenow=timenow+3600; /* now true local time */
     timenow=timenow%FULLDAYSECS;}      /* mod 24 hours */

   for (pll=markbase; pll!=NULL; pll=pll->next) {  /* each mark */
     if (pll->drawn) continue;     /* optimization */
     if (glabels && gmarkers && gclocks) pll->drawn=TRUE;

     /* lat/lon --> pel coordinates */
     rc=ll2pel(pll->lon, pll->lat, FALSE, &x, &y, &z);

     if (rc==0) {                       /* visible */
       x=x+r; y=y+r;                    /* into bitmap */
       len=(signed)strlen(pll->text);   /* string length */
       if ((len>0 && glabels) ||        /* have a label.. */
        (pll->clock!=BADLONG            /* .. or a clock */
         && gclocks && gmtoffset!=BADLONG)) havetext=TRUE;
        else havetext=FALSE;

       if (havetext) {                  /* will need font information */
         struct logfont *plf;           /* work */
         long i;
         if (pll->tfont==0) lcid=LCID_DEFAULT;
          else {                        /* defined font */
           lcid=pll->tfont;
           plf=fontbase;                /* -> start of chain */
           for (i=1; i<lcid; i++) plf=plf->next;
           /* plf now points to font for this mark's label */
           cbox.cx=MAKEFIXED(plf->fsize,0); cbox.cy=cbox.cx;
           fcoli=plf->fcoli;
           /* if (diag) printf("Using logical font '%s' size %li\n",
                             plf->fname, plf->fsize); */
           }
         } /* need font information */

       if (gmarkers) {                  /* draw mark, if any */
         GpiSetColor(sps,pll->mcoli);   /* at this colour */
         switch (pll->mark) {
           case '+':
             at.x=x-2; at.y=y;   GpiMove(sps, &at);
             at.x=x+2;           GpiLine(sps, &at);
             at.x=x;   at.y=y-2; GpiMove(sps, &at);
                       at.y=y+2; GpiLine(sps, &at);
             break;
           case '.':
             at.x=x-1; at.y=y;   GpiMove(sps, &at);
             at.x=x+1;           GpiLine(sps, &at);
             at.x=x;   at.y=y-1; GpiMove(sps, &at);
                       at.y=y+1; GpiLine(sps, &at);
             break;
           case 'x':
             at.x=x-2; at.y=y-2; GpiMove(sps, &at);
             at.x=x+2; at.y=y+2; GpiLine(sps, &at);
             at.x=x-2; at.y=y+2; GpiMove(sps, &at);
             at.x=x+2; at.y=y-2; GpiLine(sps, &at);
             break;
           case 'X':
             at.x=x-4; at.y=y-4; GpiMove(sps, &at);
             at.x=x+4; at.y=y+4; GpiLine(sps, &at);
             at.x=x-4; at.y=y+4; GpiMove(sps, &at);
             at.x=x+4; at.y=y-4; GpiLine(sps, &at);
             break;
           case '!':             /* special dot */
           case '\'':            /* (old form) */
             at.x=x  ; at.y=y;   GpiMove(sps, &at);
                                 GpiLine(sps, &at);
             break;
           default: break;    /* blank or unknown */
           } /* Have mark */
         } /* markers */

       /* write text and/or clock, if any */
       if (havetext) {
         long slen, shi;                /* string length and height */
         char ampm[3+1], *dayflag;      /* am/pm, plus flag */
         char layout[31];               /* for format layout */
         long there, hh, mm;

         /* First create the text string */
         if (pll->clock!=BADLONG && gclocks && gmtoffset!=BADLONG) {
           /* A clock to add */
           dayflag="";
           there=timenow+pll->clock     /* adjusted local time */
             -(tz%FULLDAYSECS);         /* .. */
           if (there<0)                 {there=there+FULLDAYSECS; dayflag="-";}
            else if (there>FULLDAYSECS) {there=there-FULLDAYSECS; dayflag="+";}
           if (!gcday) dayflag="";      /* undo flag */
           /* Time there now in seconds, 0-24 hours */
           there=there/60;              /* to minutes */
           mm=there%60;                 /* total minutes */
           hh=there/60;                 /* total hours */
           if (!gcivil) {
             strcpy(ampm,dayflag);      /* use just flag if 24-hour */
             strcpy(layout,"%02li:%02li%s ");
             } /* not civil */
            else {                      /* civil time please */
             sprintf(ampm,"am%s", dayflag);
             if (hh==0) hh=12;          /* midnight to 1am */
              else if (hh>=12) {
                ampm[0]='p';            /* noon to midnight */
                if (hh>12) hh=hh-12;    /* 1pm to midnight */
                } /* pm */
             strcpy(layout,"%li:%02li%s ");
             } /* civil */
           sprintf(text, layout, hh, mm, ampm);
           /* And add any text */
           if (glabels) strcpy(&text[strlen(text)], pll->text);
           /* if (diag) printf("New text '%s'\n", text); */
           len=(signed)strlen(text);         /* update */
           } /* Have a clock */
          else strcpy(text, pll->text);      /* (LEN is OK) */
           /* no clock or text wouldn't get here */

         GpiSetCharSet(sps, lcid);
         GpiSetColor(sps, fcoli);            /* font colour */
         GpiSetCharMode(sps, CM_MODE1);      /* font mode */
         if (lcid!=LCID_DEFAULT) {
           /* if (diag) printf("Using logical font #%li\n", lcid); */
           /* and in case we got an outline font... */
           urc=GpiSetCharBox(sps, &cbox);         /* size it */
           if (diag && urc==0) printf("GSCB failed\n");
           }

         /* Find its size and other details */
         GpiQueryFontMetrics(sps, (long)sizeof(fm), &fm);
         at.x=0; at.y=0;
         GpiQueryCharStringPosAt(sps, &at, 0L, len, text, NULL, xs);
         /* final entry in XS array is new current pos */
         slen=xs[len].x;                          /* string length */
         shi =fm.lMaxAscender+fm.lMaxDescender;   /* height (approx) */

         /* Now decide the real start coordinates */
         if (pll->tx==BADSHORT) at.x=x-(slen/2);  /* centre request */
          else at.x=x+pll->tx;                    /* offset */
         if (pll->ty==BADSHORT)                   /* centre request */
           at.y=y+fm.lMaxDescender-(shi/2);
          else at.y=y+pll->ty;                    /* offset */

         /* Shift left or right if would be clipped */
         if ((at.x+slen)>diameter)
           at.x=diameter-slen-1;        /* need shift left */
         if (at.x<1) at.x=1;            /* also clamp to left */

         /* Similar watchits for Y */
         if (at.y+fm.lMaxAscender>=diameter) at.y=y-fm.lMaxAscender-1;
         if (at.y<fm.lMaxDescender)          at.y=fm.lMaxDescender;

         lrc=GpiCharStringAt(sps, &at, len, text);
         if (lrc==GPI_ERROR) {
           if (diag) printf("Could not GCSA\n");
           }
         } /* have text */
       } /* mark point is visible */
     } /* each mark loop */

   if (glabels || gclocks) {            /* maybe text, so release fonts */
     struct logfont *plf;               /* work */
     long i;
     GpiSetCharSet(sps, LCID_DEFAULT);
     plf=fontbase;                      /* -> start of chain */
     for (i=1; i<=fonts; i++) {
       GpiDeleteSetId(sps, i);          /* drop font */
       plf=plf->next;}
     } /* labels */

   /* All marks now in the bitmap .. copy to memory version so not lost */

   if (!stopdraw) {
     if (startup) domerge((int)diameter-1,0); /* whole screen */
     parttext=FALSE;               /* OK, looks good */
     if (diag) printf("Marks drawn\n");
     } /* not interrupted */
   } /* text */

 if (startup) {
   /* can no longer be in 'startup' (draw grids) special */
   startup=FALSE;                       /* no longer in startup */
   if (stopdraw) continue;              /* give up */
   somesprite=FALSE;                    /* no new merge needed */
   }

 if (somesprite) {                      /* merge what we have to screen */
   if (!newgeog) domerge((int)diameter-1,0); /* whole screen */
    else domerge((int)diameter-1, (int)(diameter-restart-1));
   }
 if (stopdraw) continue;                /* give up */

 /* From now we can run at reduced priority, if requested */
 if (idledraw) DosSetPrty(PRTYS_THREAD, PRTYC_IDLETIME, +1, tidg);

 /* ----- begin drawing the globe ---------------------------------- */
 if (stopdraw) continue;           /* give up */

 /* Now, if the map changed, then draw it now.  (I.e., not needed if */
 /* only the grid lines changed, for example.)                       */
 /* (This logic option not indented.)                                */
 if (partdrawn) {

 int valueland[256+256+1];         /* Lightcos->Value lookup for Land */
 int valuesea[256+256+1];          /* Lightcos->Value lookup for Sea */
 int vland, vsea;                  /* brightness of this pel */
 int valindex;                     /* index into valueland/sea */
 int index;                        /* colour index */
 int landnear[COLHI+1];            /* nearest colour indexes for land */
 int seanear[COLHI+1];             /* nearest colour indexes for sea */
 int landsteps, seasteps;          /* steps in selected array */
 int randr, randg, randb, randw;   /* range of noise to add to RGB */
 int  x,  y  /*,  z*/;             /* globe x,y,z (0,0 is centre) */
 long x2, y2 /*, z2*/;             /* .. squared */
 int  xf;                          /* first x in row */
 long yprime /*, zprime*/;         /* transformed Y, Z */
 long yprime1000, zprime1000;      /* .. */
 int  lxz;                         /* length in X-Z plane*32 */


 /* First get the light vector (where the light is) */
 if (sun) {                        /* lighting: set up coordinates */
   long slat1000, slon1000;
   /* get latest Sun Latitude/Longitude */
   setsun(tz, &slat1000, &slon1000);
   if (diag) {
     char clat[12], clon[12];
     putmill(clat,slat1000); putmill(clon,slon1000);
     printf("Sun latitude/longitude: %s %s\n", clat, clon);
     }
   /* We need the X, Y, Z in screen coordinates, with high       */
   /* resolution.  LL2PEL can provide this...                    */
   ll2pel(slon1000, slat1000, TRUE, &lightx, &lighty, &lightz);
   /* printf("Light vector(1000): %li %li %li\n",
     lightx*1000L/(r*32L), lighty*1000L/(r*32L),
     lightz*1000L/(r*32L)); */
   } /* sun */
  else if (shade) {                /* need vector for artifical light */
   lightx=-(57L*32L*r)/100;        /* somewhere top-left */
   lighty= (67L*32L*r)/100;        /* somewhere top-left */
   lightz=lsqrt(r2x1024-lightx*lightx-lighty*lighty);
   /* printf("Artifical light vector(1000): %li %li %li\n",
     lightx*1000L/(r*32L), lighty*1000L/(r*32L), lightz*1000L/(r*32L));
     */
   } /* sun calculations */

 /* Copy the appropriate 'nearest colour index' arrays */
 copynear(landnear, &landsteps, &col, lcoli);
 copynear(seanear,  &seasteps, &col, scoli);

 /* Set random spread (at least 1, in case of large number colours) */
 randr=max(221/col.rsteps,1);      /* (R not used) */
 randg=max(221/landsteps,1);
 randb=max(221/seasteps,1);
 /* Note: on VGA, PALEGRAY adds a rogue and unbalanced grey step, but   */
 /* this helps a smoother Starlight, so we let sleeping levels lie.     */
 /* randw=max(221/(col.wsteps-1),1); */ /* -1 allows for rogue PALEGRAY */
 randw=max(221/col.wsteps,1);

 if (diag && test) printf("RandRGBW: %i %i %i %i\n", randr, randg, randb, randw);

 /* Now set up the values arrays.  These arrays are used to map      */
 /* LIGHTCOS/4 (in range -256 to 256) to a colour value (0-255).     */
 /* There are two arrays, one for sea and one for land.              */
 /* Each has three zones:                                            */
 /*   zmin:   below this, all values are set to vmin                 */
 /*   zmax:   above this, all values are set to vmax                 */
 /*   remainder is shaded zone: values are shaded for shade range    */
 /*   vlow->vhigh over span zlow->zhigh                              */
 /*   (vlow has two possible values: vlowland and vlowsea, for nice  */
 /*   starlight)                                                     */
 {int i, j;
 long term, maxterm;
 int  vlow, vhigh, vmin, vmax;
 int  zlow, zhigh, zmin, zmax;
 int  vlowland,   vlowsea;
 long vrangeland, vrangesea, zrange;

 if (!sun) {                       /* artificial light: 3-D or flat */
   vlow= 35; vhigh=255; vmin=  35; vmax=255;
   zlow=-256; zhigh=256;
   if (shade) {zmin=-256; zmax= 256;}   /* whole is shaded */
    else      {zmin=-256; zmax=-257;}   /* whole is 255 */
   } /* artificial light */
  else { /* sun */
   term   =lcos(twi+90000L);       /* selected terminator (if used) */
   maxterm=lcos(108000L);          /* 90+18ø */

   if (space) {                    /* shading is for earth in space */
     /* (Sunlight is implied) */
     vlow=  0; vhigh=255; vmin=   0; vmax=255;}
    else  {                        /* sunlight not in space */
     vlow=127; vhigh=255; vmin= 127; vmax=255;
     if (shade) vmin=63;
     }

   if (!shade) {                   /* not 3-D, so narrow shade band */
     if (vlow==0) vlow=30; else vlow=160;
     vhigh=200;
     }

   zlow=(int)(maxterm*256L/1000L); /* shading starts here */
   if (shade) zhigh=256;           /* shade to sun-centre */
    else      zhigh=-1;            /* shade only on dark side */
   zmin=(int)(term*256L/1000L);    /* cutoff at chosen terminator */
   zmax=zhigh;
   } /* sun */

 /***
 if (diag) printf("V low, high, min max: %i %i, %i %i\n",
     vlow, vhigh, vmin, vmax);
 if (diag) printf("Z low, high, min max: %i %i, %i %i\n",
     zlow, zhigh, zmin, zmax);
 ***/

 /* adjust VLOW for land if starlight */
 /*** if (star && vlow<VALSTAR) vlowland=VALSTAR; else ***/
 vlowland=vlow;
 vlowsea=vlow;                     /* always */

 /* now fill in the tables */
 zrange=zhigh-zlow;                /* 0->512 */
 vrangeland=vhigh-vlowland;        /* 0->255 */
 vrangesea =vhigh-vlowsea ;        /* 0->255 */

 for (i=-256; i<=256; i++) {
   j=i+256;                        /* index into array */
   if       (i>zmax) {valueland[j]=vmax; valuesea[j]=vmax;}
    else if (i<zmin) {valueland[j]=vmin; valuesea[j]=vmin;}
    else { /* shaded from zlow->zhigh with value ranges vlow->vhigh */
     /* distance from base is I-ZLOW */
     valueland[j]=(int)((vrangeland*(long)(i-zlow))/zrange)+vlowland;
     valuesea [j]=(int)((vrangesea *(long)(i-zlow))/zrange)+vlowsea;
     } /* shaded value */
   } /* values loop */
 } /* block */

 /* ----- Start the drawing proper ----- */
 start=(signed long)WinGetCurrentTime(habGlobe);
 if (diag) printf("Starting globe draw\n");

 if (vlat==0) tilt=FALSE;          /* we are vertical */
  else {                           /* we are tilted */
   tilt=TRUE;
   vlatcos=lcos(vlat);             /* we will need these... */
   vlatsin=lsin(vlat);
   }

 /* There are two complicating issues in this loop.  First, we have  */
 /* to be careful to get sufficient accuracy: to achieve that, we    */
 /* hold most values multiplied by 32.                               */
 /* Second, multiplications are quite expensive (horrendous, for 16- */
 /* bit) so we avoid by preserving multiplied versions of numbers    */
 /* whereever possible, calculating squares by addition, etc.        */
 /* We also manually move calculations out of the loop as certain    */
 /* compilers don't seem to be able to do that very well.            */
 lasty=r-restart+1;                /* 'last row copied to screen' */
 lasttime=start;                   /* when last slice drawn */
 row=flatmap+rupx*(r+r-restart);   /* -> start of top row of globe */
 /* Y**2 and X**2 are calculated (as in sequence) by addition only, */
 /* using (x+1)**2 == x**2+2x+1 (and similar for decreasing Y)      */
 zprime1000=0;                     /* in case VLAT==0 */
 y32=(r-restart)*32;               /* ditto Y32, save * in inner loop */
                                   /* (Note use true multiple of Y) */
 y2=(long)(r-restart+1);           /* set initial Y2 */
 y2=y2*y2;                         /* .. */
 vland=255; vsea=255;              /* in case neither SUN nor 3-D */

 /* ----- start processing the globe, by Y (rows) from top ----- */
 for (y=r-restart; y>=-r; y--, y32=y32-32, row=row-rupx) {
   y2=y2-(y+y+1);                  /* calculate y**2 */
   r2xmy2=r2x-y2;                  /* oft-used */
   er=0; eg=0; eb=0;               /* reset linear errors */

   xf=(int)lsqrt(r2xmy2);          /* maximum +/- X value */
   c=row+r-xf;                     /* first byte to draw within row */
   x=-xf-1; x2=(long)x*(long)x;    /* set initial X2 */
   if (!tilt) {                    /* have no tilt */
     /* If there is no tilt, then the length in X-Z plane (and the */
     /* latitude) are constant for the whole whole, so we can do the */
     /* calculation now, rather than for each pel of the drawing. */
     lxz=(int)lsqrt(r2xmy2*1024L); /* length in X-Z plane, for LLON */
     /* The latitude is a constant, too */
     lat=90000L-lacos(y*1000L/r);  /* .. latitude corresponding */
     /* Also, we are always looking at 'front' of earth, so just */
     /* use preset ZPRIME1000 (it is only tested for negative) */
     } /* no tilt */
    else {                         /* tilt, do some pre-calculations */
     #if !defined(B16)
     y32=y32+lsqrt(0);             /* workaround for optimization problem */
     #endif
     y32vlatcos=(long)y32*vlatcos; /* useful constant multiples */
     y32vlatsin=(long)y32*vlatsin; /* .. */
     } /* tilt */

   /* ----- start processing the row, by X ----- */
   x32   =32*-xf;                  /* set initial X*32      */
   /* note form in line above is 'required' at present */
   x32000=(long)x32*1000L;         /* set initial X*32*1000 */
   for (x=-xf; x<=xf; x++, x32=x32+32, x32000=x32000+32000L, c++) {
     /* Here only if we are inside circle */

     /* If tilted, or in sunlight, or 3-D then need Z32 etc... */
     if (tilt || sun || shade) {
       /* We will need Z32 for these paths */
       if (x<=0) {                 /* calculate and save, if left */
         x2=x2+(x+x-1);            /* globe coordinate, squared */
         /* Note: X', Y', Z', and LXZ are all *32 for accuracy      */
         /*       (As are X32, Y32, Z32, and R32)                   */
         z32=(int)lsqrt((r2xmy2-x2)*1024L); /* calculate Z (slow..) */
         /*              r2x-y2-x2                                  */
         sz32[-x]=z32;
         }
        else z32=sz32[x];          /* right half, so use saved value */

       /* determine light value, if not fixed... */
       if (sun || shade) {
         int lightcos;             /* cos(lightvector:landvector) */
         /* Lightx,y,z are *32 [lighty*y32 could be pre-calculated] */
         lightcos=(int)((lightx*(long)x32 + lighty*(long)y32
                       + lightz*(long)z32)/r2x);
         /* LIGHTCOS here is range -1024->1024, not the usual 0->1000 */

         if (abs(lightcos)>1024) { /* be careful: nasties seen */
           if (diag) {
             static int oor=0;
             oor++;
             if (oor<=10) printf("Out of range LIGHTCOS: %i\n", lightcos);
             if (oor==10) printf("*** 10+ Out of range LIGHTCOS ***\n");
             if (oor<5) {
               printf("x, y: %i %i\n", x, y);
               printf("x32, y32, z32: %i %i %i\n", x32, y32, z32);
               printf("r, r2, r2x: %i %li %li\n", r, (long)r*(long)r, r2x);
               printf("lx, ly, lz: %li %li %li\n", lightx, lighty, lightz);
               }
             }
           if (lightcos> 1024) lightcos= 1024;
           if (lightcos<-1024) lightcos=-1024;
           }
         valindex=256+lightcos/4;  /* is now in range 0->512 */
         vland=valueland[valindex]; vsea=valuesea[valindex];
         /* Values are now in range 0->255 */
         } /* sunshine or 3-D */

       if (tilt) {                 /* tilted */
         /* If tilted, we have to calculate Y', the height on the Y    */
         /* axis of the actual (rather than seen) globe.  From this we */
         /* get the real latitude (if needed), and also LXZ which is   */
         /* needed to calculate longitude.                             */
         /* Apply rotation about X, using multiplier for accuracy.     */
         /* We save the '*1000' result as often useful below.          */
         /* The X', Y', Z' values are all *32                          */
         zprime1000=vlatcos*z32-y32vlatsin;
         /* zprime=zprime1000/1000L; */      /* ZPRIME not used */

         /* Now, the LATitude (and LXZ) is symmetrical in X, so we */
         /* only calculate for left half, and load from lookaside for */
         /* right half... */
         if (x<=0) {                         /* calculate and save */
           yprime1000=vlatsin*z32+y32vlatcos;
           yprime=yprime1000/1000L;
           lxz=(int)lsqrt(r2x1024-yprime*yprime);   /* length in X-Z */

           /* This gives Y' from which we can calculate the latitude, */
           /* unless it is too close to R, in which case we use LXZ,  */
           /* the distance in X-Z plane for better accuracy. */
           if (labs(lxz)<(int)labs(yprime)) {/* use LXZ' for best results */
             lat=lacos((long)lxz*1000L/r32);
             if (yprime<0) lat=-lat;
             }
            else                             /* use Y' for best result */
             lat=90000L-lacos(yprime1000/r32);
           slxz[-x]=lxz;
           slat[-x]=lat;
           } /* left half */
          else {                             /* right half - use saved */
           lxz=slxz[x];
           lat=slat[x];
           }
         } /* tilted */

       } /* tilted or in sunshine */

     /* No need to do any more, if a feature dot (e.g. grid line) */
     /* Quick exit is also possible if dark side of the earth and */
     /*   not starlight                                           */
     if (vsea<=0 && space && !star) {*c=(unsigned char)cblk; continue;}

     /* ----- Now the longitude calculation ----- */
     /* (Some of this could be lookaside, too) */
     if (lxz==0) lon=0;                  /* North/South pole */
      else {                             /* elsewhere */
       /* Use either X or Zprime to calculate angle, depending on best */
       /* accuracy... */
       /* Except that if non-tilt view, we risk using X only */
       if (vlat!=0 &&
        labs(zprime1000)<labs(x32000)) { /* use Z' for best results */
         lon=lacos(zprime1000/lxz);      /* simple: 0 is front centre */
         if (x<0) lon=-lon;              /* eastern hemisphere */
         }
        else {                           /* near LON=0 or LON=180, use X */
         lon=90000L-lacos(x32000/lxz);
         if (zprime1000<0) {             /* if "back of globe" */
           lon=180000L-lon;
           if (lon>180000L) lon=lon-360000L;
           }
         } /* using X */
       } /* not at pole */

     /* Apply any longitude rotation (+/- VLON, about poles) */
     if (vlon!=0) {
       lon=lon+vlon;
       if       (lon>  180000L) lon=lon-360000L;
        else if (lon< -180000L) lon=lon+360000L;
       }
     /* pull the appropriate character out of the landmap */
     mapx=(int)(((lon+180000L)          /* range 0-360000 */
                *TOPMAPX)/360000L);     /* into pels */
     mapy=(int)(((lat+90000L)           /* range 0-180000 */
                *TOPMAPY)/180000L);     /* into pels */

     /* Find out if bit at this coordinate is set */
     /* First get the byte in which bit is stored */
     /* Note: need (long) to avoid compiler silly */
     pbyte=landmap + ((long)mapy*landmaprupx+mapx/8);

     if (*pbyte & mask[mapx%8]) {       /* On Land */
       /* Strategy here is a mite tricky. */
       /* If dark side: always use black or starlight diffusion. */
       /* else calculate error-adjusted value.  If less than VALSTAR */
       /* (and starlight) then use grey diffusion.  Else green scale. */
       if (vland>0) {                   /* definitely bright */
         if (lcoli==CLR_DARKGRAY) vland=vland/2;  /* special */
         if (vland==255) index=landnear[COLHI];   /* easy fast path */
          else if (vland==127 && !shade)          /* !shade case of ring */
           index=landnear[COLHI/2];               /* for smooth dark side*/
          else { /* landvalue > 0 */    /* error diffusion needed... */
           eg=eg+fastrand(randg)-randg/2; /* add random noise to level */
           vland=vland+eg+*(pg+x);
           if (vland>255) vland=255;    /* clamp */
            else if (vland<0) vland=0;
           /* now calculate and diffuse the error */
           index=landnear[vland/(255/COLHI)];/* get the index */
           eg=vland-collev[index];      /* error to spread */
           *(pg+x)=eg*5/16;             /* error 'below'          */
           sg=*(pg+x);                  /* sum (used-so-far)      */
           *(pg+x-1)=eg*3/16;           /* error 'below to left'  */
           sg=*(pg+x-1)+sg;             /* sum (used-so-far)      */
           *(pg+x+1)=eg*1/16;           /* error 'below to right' */
           sg=*(pg+x+1)+sg;             /* sum (used-so-far)      */
           eg=eg-sg;                    /* remainder (about 7/16) */
           } /* not full, half, or <=0 */
         } /* vland>0 */
        else index=cblk;                /* <=0 is dark */
       if (index==cblk && star) {       /* use grey... */
         /* We use the spare 'red' error array for now */
         er=er+fastrand(randw)-randw/2; /* add random noise to level */
         vland=valstar;               /* fixed grey */
         vland=vland+er+*(pr+x);
         if (vland>255) vland=255;    /* clamp */
          else if (vland<0) vland=0;
         /* now calculate and diffuse the error */
         index=col.wnear[vland/(255/COLHI)];    /* get the index */
         er=vland-collev[index];      /* error to spread */
         *(pr+x)=er*5/16;             /* error 'below'          */
         sr=*(pr+x);                  /* sum (used-so-far)      */
         *(pr+x-1)=er*3/16;           /* error 'below to left'  */
         sr=*(pr+x-1)+sr;             /* sum (used-so-far)      */
         *(pr+x+1)=er*1/16;           /* error 'below to right' */
         sr=*(pr+x+1)+sr;             /* sum (used-so-far)      */
         er=er-sr;                    /* remainder (about 7/16) */
         } /* starlight land */
       } /* on land */

      else {                            /* At Sea */
       if (scoli==CLR_DARKGRAY) vsea=vsea/2; /* special */
       if (vsea==255) index=seanear[COLHI];  /* easy fast path */
        else if (vsea==127 && !shade)        /* !shade case of ring */
         index=seanear[COLHI/2];             /* for smooth dark side*/
        else if (vsea<=0)   index=cblk;      /* .. */
        else {                          /* error diffusion needed... */
         eb=eb+fastrand(randb)-randb/2; /* add random noise to level */
         vsea=vsea+eb+*(pb+x);
         if (vsea>255) vsea=255;        /* clamp */
          else if (vsea<0) vsea=0;
         /* now calculate and diffuse the error */
         index=seanear[vsea/(255/COLHI)];    /* get the index */
         eb=vsea-collev[index];         /* error to spread */
         *(pb+x)=eb*5/16;               /* error 'below'          */
         sb=*(pb+x);                    /* sum (used-so-far)      */
         *(pb+x-1)=eb*3/16;             /* error 'below to left'  */
         sb=*(pb+x-1)+sb;               /* sum (used-so-far)      */
         *(pb+x+1)=eb*1/16;             /* error 'below to right' */
         sb=*(pb+x+1)+sb;               /* sum (used-so-far)      */
         eb=eb-sb;                      /* remainder (about 7/16) */
         } /* not full, half, or 0 */
       } /* at sea */
     *c=(unsigned char)index;           /* set the pel */
     /* Note: error diffusion errors persist across grid lines and */
     /* across land or sea (for the other colour(s)) */
     } /* x */

   /* Now, if suitably long delay since last copy, draw/copy any */
   /* completed rows to the bitmap, and then echo to screen */
   /* also do it if forced: just did the last row, or stopdraw */
   if (stopdraw || y==-r) {             /* interrupt or globe completed */
     /* if (diag) printf("Final swathe\n"); */
     domerge(lasty+r-1, y+r);           /* merge map+sprites -> shadow */
     if (stopdraw) break;               /* interrupt now! */
     restart=0;                         /* where to restart */
     } /* done draw */
    else if (showdraw) {                /* maybe in-progress update */
     newtime=(signed long)WinGetCurrentTime(habGlobe);
     if ((newtime-lasttime)>TIMEROWS    /* time for an update */
       || newtime<lasttime) {           /* timer wrapped */
       lasttime=newtime;
       domerge(lasty+r-1, y+r);         /* merge map+sprites -> shadow */
       if (stopdraw) break;             /* interrupt now! */
       restart=r-y;                     /* where to restart */
       lasty=y;                         /* last row drawn */
       } /* timeout */
     } /* showdraw */
   } /* y */
 } /* logic deciding whether to draw */

 if (stopdraw) continue;                /* give up, now */

 stop=(signed long)WinGetCurrentTime(habGlobe);
 if (diag) printf("Globe draw time: %lims\n", (long)(stop-start));

 /* and from now on geography is consistent (all globe) and safe */
 newgeog=FALSE;                    /* good geography */
 partdrawn=FALSE;                  /* we have a good piccie */
 restart=0;                        /* next from top */
 WinPostMsg(hwndClient, UM_DRAWN, NULL, NULL);
 } /* ===== end of main ('forever') loop ========================== */

 WinTerminate(habGlobe);           /* done with anchor block */
 return;
 } /* glober */

/*-----------------------------------------------------------------*/
/* COPYNEAR: copy NEAR table into land/sea near array              */
/* Arguments:                                                      */
/*   land or sea NEAR array                                        */
/*   land or sea STEPS count                                       */
/*   COL structure                                                 */
/*   selected colour (CLR_...)                                     */
/*-----------------------------------------------------------------*/
/* (There are cleverer ways to code this function.) */
void copynear(int *nea, int *steps, PCOLSTRUCT c, long clr) {
 int i;
 if      (clr==CLR_BLACK)    {for (i=0; i<COLHI+1; i++) nea[i]=(*c).pure[0];
                              *steps=1;}
 else if (clr==CLR_RED)      {for (i=0; i<COLHI+1; i++) nea[i]=(*c).rnear[i];
                              *steps=(*c).rsteps;}
 else if (clr==CLR_GREEN)    {for (i=0; i<COLHI+1; i++) nea[i]=(*c).gnear[i];
                              *steps=(*c).gsteps;}
 else if (clr==CLR_BLUE)     {for (i=0; i<COLHI+1; i++) nea[i]=(*c).bnear[i];
                              *steps=(*c).bsteps;}
 else if (clr==CLR_CYAN)     {for (i=0; i<COLHI+1; i++) nea[i]=(*c).cnear[i];
                              *steps=(*c).csteps;}
 else if (clr==CLR_PINK)     {for (i=0; i<COLHI+1; i++) nea[i]=(*c).pnear[i];
                              *steps=(*c).psteps;}
 else if (clr==CLR_YELLOW)   {for (i=0; i<COLHI+1; i++) nea[i]=(*c).ynear[i];
                              *steps=(*c).ysteps;}
 else if (clr==CLR_WHITE)    {for (i=0; i<COLHI+1; i++) nea[i]=(*c).wnear[i];
                              *steps=(*c).wsteps;}
 else if (clr==CLR_DARKGRAY) {for (i=0; i<COLHI+1; i++) nea[i]=(*c).wnear[i];
                              *steps=(*c).wsteps;}
 return;} /* copynear */

/*-----------------------------------------------------------------*/
/* SETSUN: set sun latitude and longitude                          */
/* Arguments:                                                      */
/*   GMT total offset in seconds to use for this math.             */
/*   latitude (long *) to set                                      */
/*   longitude (long *) to set                                     */
/*-----------------------------------------------------------------*/
void setsun(long offset, long *slat, long *slon) {
 long timesecs;                    /* Seconds since 1 Jan 1970 */
 long daysecs;                     /* Seconds since civil midnight */
 long noonsecs;                    /* Seconds since/to solar noon */
 long yearsecs;                    /* Seconds since 1 January 00:00 */
 long inteot;                      /* Interpolated equation of time */
 long intdec;                      /* Interpolated declination */
 int  daynum;                      /* Day of the year, 0-365 */
 struct tm *timeinfo;              /* work */

 /* protect against invalid timezone */
 if (gmtoffset==BADLONG) return;   /* no change */

 time(&timesecs);                  /* Get Now */
 /* if (diag) printf("Raw timesecs: %li\n", timesecs); */

 /* This gives a 'GMT', from default or TZ= setting.  Only if EST/PST     */
 /* is this safe and correct.  Hence we rely on our own TimeZone setting, */
 /* and must convert the result of TIME() to wall-clock time.             */
 timeinfo=localtime(&timesecs);
 if (diag && (timeinfo->tm_isdst>0)) printf("Daylight savings in effect\n");
 /* correct back for timezone and daylight savings hour */

 timesecs=timesecs-_timezone;
 /* Next line needed as _timezone variable allows for base offset only */
 if (timeinfo->tm_isdst>0) timesecs=timesecs+3600;
 /* timesecs is now true local time */

 /* and now apply the requested (trusted) timezone and user offset.. */
 timesecs=timesecs-offset;         /* effect timezone */
 /* if (diag) printf("Timesecs (without EOT): %li\n", timesecs); */

 /* This gives us mean solar time.  We need apparent solar time,    */
 /* which differs from the mean by the Equation of time.            */
 /* EOTTAB table gives us the difference, in seconds, for each day  */
 /* of the year -- this is within half a minute over one day, which */
 /* is sufficiently accurate for our purposes, especially as we     */
 /* linearly interpolate (this gives results to within a few        */
 /* seconds at the fastest change time of year, near the equinoxes) */
 /* DECTAB similarly gives us the declination, in millidegrees.     */
 /* Both tables are based on an origin of Jan 1, 1990, 00:00.       */
 /* Jan 1, 1990, 00:00 was 631152000 seconds in TIME() format, and  */
 /* we can work out how many days we are into the current solar     */
 /* year from that.                                                 */
 /* Table lookup is based on civil time, not solar time.            */
 /* There are 365.24219 mean solar days in a year, hence there are  */
 /* 31556925 mean solar seconds in a year.  We now calculate how    */
 /* far we are into the mean solar year...                          */
 yearsecs=(timesecs-631152000L)%31556925;
 /* Solar day number (0-365) is this divided by seconds in a day:   */
 daynum=(int)(yearsecs/FULLDAYSECS);

 daysecs=yearsecs%FULLDAYSECS;     /* seconds into DAYNUM solar day */
                                   /* (also used for declination */
                                   /* calculation below) */
 /* if (diag) printf("Solar day number/secs, EOT, EOT+1: %i %li %i %i\n",
                   daynum, daysecs, eottab[daynum], eottab[daynum+1]); */
 /* Adjustment... */
 inteot=(eottab[daynum+1]-eottab[daynum])*daysecs/FULLDAYSECS;
 inteot=eottab[daynum]+inteot;     /* .. plus midnight base */
 if (diag) printf("Interpolated EOT: %li\n", inteot);

 timesecs=timesecs+inteot;         /* is now apparent solar time */

 noonsecs=(timesecs+12L*3600L)%FULLDAYSECS;   /* time of solar noon */
 if (noonsecs>12L*3600L) noonsecs=noonsecs-FULLDAYSECS; /* relative */
 if (diag) printf("Seconds since apparent solar noon: %li\n", noonsecs);

 /* Noonsecs now -24*3600 to 24*3600.  Convert to longitude         */
 *slon=-noonsecs*1800L/(12L*36L);   /* careful: else overflow...    */

 /* Now worry about the latitude.  We used to calculate this; now   */
 /* look it up from a table and interpolate, just like EOT is       */
 /* handled.                                                        */
 /* if (diag) printf("Solar day number/secs, DEC, DEC+1: %i %li %i %i\n",
                   daynum, daysecs, dectab[daynum], dectab[daynum+1]); */
 /* Adjustment... */
 intdec=(dectab[daynum+1]-dectab[daynum])*daysecs/FULLDAYSECS;
 intdec=dectab[daynum]+intdec;     /* .. plus midnight base */
 /* if (diag) printf("Interpolated DEC: %li\n", intdec); */

 /* Interpolated declination is the sun's latitude, exactly */
 *slat=intdec;
 return;}  /* setsun */

/*-----------------------------------------------------------------*/
/* DOMERGE: sprite+map -> shadow                                   */
/*-----------------------------------------------------------------*/
/* Arguments are start row/end row, in bitmap-space from top down. */
/* BOTR can be larger than TOPR if none to do.                     */
/* Currently runs to completion .. could probably be interruptible */
void domerge(int topr, int botr) {
 long lrc;                         /* work */
 HPS  mps;                         /* .. */
 HPS  sps;                         /* .. */
 long srowy, highy;
 char huge *srow;                  /* -> flatmap row */
 char *mrow;                       /* -> merge row */
 POINTL  pa[4];                    /* array of points for capture */
 long lasttime, newtime;           /* last time */

 mps=globeps; sps=spriteps;        /* local copies */
 lasttime=(signed long)WinGetCurrentTime(habGlobe);

 /* Now merge the requested rows to the shadow bitmap.        */
 /* This is actually a multi-stage operation; for each line:  */
 /*   1. Copy sprite line to merge                            */
 /*   2. Merge in 'real' data where sprite=background colour  */
 /*   3. Copy line to shadow bitmap                           */
 /* When all done, BitBlt shadow swathe to screen             */
 bmap.head.cbFix=sizeof(BITMAPINFOHEADER2); /* size of infoheader */
 bmap.head.cx=(USLONG)diameter;    /* size of bitmap */
 bmap.head.cy=(USLONG)diameter;    /* .. (not needed for Query) */
 bmap.head.cPlanes   = 1L;         /* number of planes */
 bmap.head.cBitCount = 8L;         /* up to 24-bit, RGB format */
 highy=topr;                       /* next row to copy to screen */
 for (srowy=topr; srowy>=botr; srowy--) {    /* each line */
   lrc=GpiQueryBitmapBits(sps, srowy, 1L, merge, (PBITMAPINFO2)(PVOID)&bmap);
   if (lrc==GPI_ALTERROR) {
     lrc=(signed)WinGetLastError(habGlobe);
     alert("Error %lxx from GpiQueryBitmapBits on row %li",
         lrc, srowy);
     stopdraw=TRUE; break;}
   /* got line into memory .. merge in map/image data */
   srow=flatmap+rupx*srowy;        /* -> start of actual row of globe */
   for (mrow=merge; mrow<merge+diameter; srow++, mrow++) {
     if (*mrow==sback) *mrow=*srow;
     }
   /* now copy merged row to shadow bitmap */
   DosRequestMutexSem(sembitmap, SEM_INDEFINITE_WAIT);
   lrc=GpiSetBitmapBits(mps, srowy, 1L, merge, (PBITMAPINFO2)(PVOID)&bmap);
   DosReleaseMutexSem(sembitmap);  /* done with bitmap */
   if (lrc==GPI_ALTERROR) {
     lrc=(signed)WinGetLastError(habGlobe);
     alert("Error %lxx from GpiSetBitmapBits on row %li",
        lrc, srowy);
     stopdraw=TRUE; break;}
   newtime=(signed long)WinGetCurrentTime(habGlobe);
   if (srowy==botr                           /* bottom row */
     || (showdraw && (                       /* show updates */
            (newtime-lasttime)>TIMEMERGE     /* .. time for an update */
          || newtime<lasttime))              /* .. or timer wrapped */
     ){
     lasttime=newtime;
     /* copy to the screen, centred in rectangle */
     pa[0].x=(clientx-diameter)/2;pa[0].y=(clienty-diameter)/2+srowy; /* target */
     pa[1].x=pa[0].x+diameter;    pa[1].y=pa[0].y-srowy+highy+1;      /* .. */
     pa[2].x=0;                   pa[2].y=srowy;                      /* source */
     DosRequestMutexSem(sembitmap, SEM_INDEFINITE_WAIT);
     lrc=GpiBitBlt(cps, mps, 3L, pa, ROP_SRCCOPY, 0L);
     DosReleaseMutexSem(sembitmap);     /* done with bitmap */
     if (lrc==GPI_ERROR) {
       /* if (diag) printf("Could not BitBlit row %i to screen PS\n", srowyy); */
       stopdraw=TRUE;    /* panic */
       }
     highy=srowy-1;                     /* where next swathe starts */
     #if defined(IDLEBLIP)
     /* Now, every so often drop to true idle priority to stop PM */
     /* lockup, as it seems background tasks run at IDLE */
     slices++;                /* slices since last draw */
     if (slices>=11) {slices=0;
       /* Go to true idle priority for a moment so PM doesn't lock up */
       if (/* idledraw && */ !stopdraw)
         DosSetPrty(PRTYS_THREAD, PRTYC_IDLETIME,  0, tidg);
       /* And back to higher priority, as requested */
       if (idledraw) DosSetPrty(PRTYS_THREAD, PRTYC_IDLETIME, +1, tidg);
                else DosSetPrty(PRTYS_THREAD, PRTYC_REGULAR,   0, tidg);
       } /* priority tweak */
     #endif
     } /* copy to screen */
   if (stopdraw) return;           /* give up */
   } /* srow */
 } /*domerge */

/*-----------------------------------------------------------------*/
/* DRAWGC: draw great circle (colour already set)                  */
/*-----------------------------------------------------------------*/
/* Arguments are start and end in X,Y,Z (pels*32)                  */
/* Then Mid-longitude and Mid-latitude (in case of extremis)       */
/* Then Radius and accurate-Radius32                               */
void drawgc(long curx32, long cury32, long curz32,
            long  tox32, long  toy32, long toz32,
            long midlon, long midlat, long r,     long r32) {
  long lx32, ly32, lz32;           /* lengths in pels*32 */
  long cx32, cy32, cz32;           /* chord pel in pels*32 */
  long x0, y0, z0;                 /* Segment start */
  long x1, y1, z1;                 /* Segment end */
  long x32, y32, z32;              /* work */
  long i, n, len;
  long s, segs, omitted;           /* segments to draw */
  POINTL at, last;
  last.x=-1; last.y=-1;            /* impossible 'last' point */

  /***
  if (diag) printf("  (%li %li  %li) to (%li %li %li)\n",
                    curx32, cury32, curz32, tox32, toy32, toz32);
  ***/

  /* Draw a line of points from one place to the other */
  /* We do it as a series of segments for accuracy and to minimise */
  /* wasted calculations, yet allowing us to extrapolate from chord */
  len=lsqrt((tox32-curx32)*(tox32-curx32)
           +(toy32-cury32)*(toy32-cury32)
           +(toz32-curz32)*(toz32-curz32));
  /* Len is now length of the chord between points (in pels*32) */
  segs=len*3/r32;             /* Up to 6 segments */
  if (segs%2==1) segs++;      /* .. and an even number */
  if (segs==0) segs=1;        /* .. or one, if very small */
  omitted=0;                  /* Number ignored */
  x1=curx32; y1=cury32; z1=curz32; /* setup */
  for (s=1; s<=segs; s++) {   /* S=1 TO SEGS: for each segment */
    x0=x1; y0=y1; z0=z1;      /* start from last end */
    x1=curx32+(tox32-curx32)*s/segs;    /* end of segment */
    y1=cury32+(toy32-cury32)*s/segs;
    z1=curz32+(toz32-curz32)*s/segs;
    /* X1, Y1, Z1 now a place on the original chord */
    /* Extend to a point on the surface by simply normalizing */
    /* to length R32 */
    len=lsqrt(x1*x1+y1*y1+z1*z1);       /* Length to chord */
    if (len<=5) {                       /* going through centre */
      /* we can't calculate a point on surface, so choose an */
      /* arbitrary one (averaged Lat/Lon) */
      ll2pel(midlon, midlat, TRUE, &x1, &y1, &z1);
      /* if (diag) printf("Arbitrary line point, Segs=%li\n", segs); */
      /* then carry on for segment calculation */
      } /* Free choice */
     else if ((x0/16L)==(x1/16L) && (y0/16L)==(y1/16L)
           && (z0/16L)==(z1/16L)) {
      /* Not a useful/significant segment */
      x1=x0; y1=y0; z1=z0;    /* ignore this segment */
      /* if (diag) printf("Segment ignored\n"); */
      omitted++;
      continue;               /* .. */
      }
     else {
      x1=x1*r32/len;          /* OK, extend to surface */
      y1=y1*r32/len;
      z1=z1*r32/len;
      } /* normal extension */

    /* Segment is now from X0,Y0,Z0 to X1,Y1,Z1 */
    lx32=x1-x0;               /* calculate lengths in pels*32 */
    ly32=y1-y0;               /* .. */
    lz32=z1-z0;               /* .. */
    len=lsqrt(lx32*lx32+ly32*ly32+lz32*lz32);
    /* Len is now length of chord in segment */
    n=(len*(omitted+1))/23;   /* number of points needed */
                              /* (Empirical, to just close gaps) */
    if (n==0) n=1;            /* ensure never 0 */
    for (i=0; i<=n; i++) {    /* draw N+1 points */
      cx32=x0+(lx32*i/n);     /* re-calculate each step */
      cy32=y0+(ly32*i/n);
      cz32=z0+(lz32*i/n);
      /* CX32, CY32, and CZ32 are points on the chord, in pels*32 */
      /* Extend to a point on the surface by simply normalizing */
      /* to length R32 */
      len=lsqrt(cx32*cx32+cy32*cy32+cz32*cz32);     /* Length to chord */
      if (len==0) {           /* going through centre(!) */
        /* (This should never happen, with segments) */
        if (diag) printf("Invalid LEN in segment\n");
        continue;}            /* Ignore */
      x32=cx32*r32/len;
      y32=cy32*r32/len;
      z32=cz32*r32/len;

      if (z32>=0) {           /* point is visible and valid */
        at.x=(x32+16L)/32L+r; /* bitmap pel */
        at.y=(y32+16L)/32L+r; /* .. */
        if (at.x!=last.x || at.y!=last.y) {
          GpiMove(spriteps, &at);       /* make a dot */
          GpiLine(spriteps, &at);       /* .. */
          last.x=at.x; last.y=at.y;/* and remember for next time */
          } /* new dot */
        } /* visible */
      } /* I loop */
    } /* S loop */
  return;} /* draw great circle line */

/*-----------------------------------------------------------------*/
/* DROPBITMAPS: Get rid of any existing bitmaps and work areas     */
/*-----------------------------------------------------------------*/
void dropbitmaps(void) {
  HBITMAP sbm, obm;           /* local copies */
  ULONG urc;
  DosRequestMutexSem(sembitmap, SEM_INDEFINITE_WAIT);
  if (globebitmap!=NULL) {
    obm=GpiSetBitmap(globeps,(HBITMAP)NULL);
    if (obm==HBM_ERROR) printf("GSB failed: %lxx\n", WinGetLastError(habGlobe));
    urc=GpiDeleteBitmap(globebitmap);
    if (!urc) printf("GDB1 failed with: %li\n", urc);
     else { /* delete worked */
      GpiAssociate(globeps,(HDC)NULL);
      globebitmap=NULL;}
    } /* globebitmap to free */
  if (spritebitmap!=NULL){
    sbm=GpiSetBitmap(globeps,(HBITMAP)NULL);
    if (sbm==HBM_ERROR) printf("GSB2 failed: %lxx\n", WinGetLastError(habGlobe));
    urc=GpiDeleteBitmap(spritebitmap);
    if (urc) { /* delete worked */
      GpiAssociate(spriteps,(HDC)NULL);
      spritebitmap=NULL;}
    } /* spritebitmap to free */
  if (globeps!=NULL)  {GpiDestroyPS(globeps);  globeps=NULL;}
  if (spriteps!=NULL) {GpiDestroyPS(spriteps); spriteps=NULL;}
  if (globedc!=NULL)  {DevCloseDC(globedc);    globedc=NULL;}
  if (spritedc!=NULL) {DevCloseDC(spritedc);   spritedc=NULL;}
  DosReleaseMutexSem(sembitmap);  /* indicate bitmap now free */
  return;} /* dropbitmaps */
