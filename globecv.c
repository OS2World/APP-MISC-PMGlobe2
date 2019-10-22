/* ------------------------------------------------------------------ */
/* PMGlobe -- a geographical application                              */
/* Geographical conversion utilities                                  */
/*   PEL2LL -- convert a screen pel X,Y to Longitudude and Latitude   */
/*   LL2PEL -- Longitudude and Latitude to a bitmap pel X,Y,visible   */
/*   LL2STR -- format longitude and latitude to strings               */
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

/*====================================================================*/
/* PEL2LL -- convert screen pel coordinate to Long/Lat                */
/*====================================================================*/
/* This is expected to be called from Main thread, hence uses the     */
/* main thread shared variables and uses screen (not bitmap) X,Y.     */
/*                                                                    */
/* It takes:                                                          */
/*   X, Y (Screen pels, in range 0->(CLIENTX-1), 0->(CLIENTY-1)       */
/* and sets:                                                          */
/*   LLON, LLAT (longitutude/latitude *1000, e.g. 180000)             */
/* Returns:                                                           */
/*   0 if OK                                                          */
/*  -1 if X, Y are outside the globe's square                         */
/*  -2 if X, Y are outside the globe circle                           */
/*                                                                    */
/* These algorithms are also in the globe drawer.  They are           */
/* duplicated there for speed, because the calculations can be        */
/* distributed.                                                       */
/*                                                                    */
int pel2ll(long x, long y, long *plon, long *plat)
 {
 long diam;                        /* globe diameter */
 long r;                           /* radius (r*2<=diam) */
 long r2, r2x;                     /* radius squared, and a bit */
 long /* d, */ d2;                 /* distance from centre; squared */
 long x2, y2;                      /* squares of X and Y */
 long z;                           /* z coordinate */
 long yprime;                      /* transformed Y */
 long zprime;                      /* transformed Z */
 long lxz;                         /* length in X-Z plane */
 long lon, lat;                    /* local long/lat for calculation */
 long vlatcos, vlatsin;            /* cosine and sine of tilt */

 if (diameter==0) return -2;       /* no globe yet */
 diam=diameter;                    /* local copy */
 r=(diam-1)/2; r2=r*r;             /* radius, radius squared */
 r2x=r*(r+1);                      /* radius squared and a bit */

 /* De-centre and see if in square */
 x=x-(clientx-diam)/2;             /* de-centre */
 if (x>=diam || x<0) return -1;    /* outside of square */
 y=y-(clienty-diam)/2;             /* de-centre */
 if (y>=diam || y<0) return -1;    /* outside of square */
 /* Now relative to centre of circle... */
 x=x-r; y=y-r;

 /* Now see if in circle */
 x2=x*x; y2=y*y;                   /* squares */
 d2=x2+y2;                         /* distance from centre, squared */
 if (d2>r2x) return -2;            /* we are outside circle */

 /* OK, a valid point, with X and Y in range -R to R */
 vlatcos=lcos(viewlat); vlatsin=lsin(viewlat);

 /* Now make all linear measurements multiplied by 32, for better */
 /* calculation accuracy (we hope) */
 x=x*32; x2=x2*32*32;
 y=y*32; y2=y2*32*32;
 r=r*32; r2=r2*32*32;
         r2x=r2x*32*32;

 /* r2x=r2; */  /* temp experiment */

 z=lsqrt(r2x-x2-y2);         /* calculate Z (slow..) */
 yprime=(z*vlatsin+y*vlatcos)/1000L; /* apply rotation about X */
 zprime=(z*vlatcos-y*vlatsin)/1000L; /* apply rotation about X */
 lxz=lsqrt(r2x-yprime*yprime);       /* length in X-Z' plane */
 /* This gives Y' from which we can calculate the latitude, */
 /* unless it is too close to R, in which case we use LXZ, the */
 /* distance in X-Z plane for better accuracy. */
 if (labs(lxz)<labs(yprime)) {    /* use LXZ' for best results */
   lat=lacos(lxz*1000L/r);
   if (yprime<0) lat=-lat;
   }
  else lat=90000L-lacos(yprime*1000L/r);

 /* printf("X, Zprime, lxz: %li %li %li\n", x, zprime, lxz); */

 if (lxz==0) lon=0;                  /* North/South pole */
  else {                             /* elsewhere */
   /* Use either X or Zprime to calculate angle, depending on best */
   /* accuracy... */
   if (labs(zprime)<labs(x)) {       /* use Z' for best results */
     lon=lacos(zprime*1000L/lxz);    /* simple: 0 is front centre */
     if (x<0) lon=-lon;              /* eastern hemisphere */
     }
    else {                           /* near LON=0 or LON=180, use X */
     lon=90000L-lacos(x*1000L/lxz);
     if (zprime<0) {                 /* if "back of globe" */
       lon=180000L-lon;
       if (lon>180000L) lon=lon-360000L;
       }
     } /* using X */
   } /* not at pole */

 /* Modify longitude by VIEW rotation */
 lon=lon+viewlon;
 if       (lon>  180000L) lon=lon-360000L;
  else if (lon< -180000L) lon=lon+360000L;

 /* Copy back to arguments */
 *plon=lon; *plat=lat;
 return 0;                         /* Success */
 } /* pel2ll */

/*====================================================================*/
/* LL2PEL -- convert Long/Lat to bitmap X,Y (and indicate if visible) */
/*====================================================================*/
/* This is expected to be called from Draw thread, hence uses the     */
/* draw thread shared variables and uses bitmap (not screen) X,Y.     */
/*                                                                    */
/* It takes:                                                          */
/*   LLON, LLAT (longitutude/latitude *1000, e.g. 180000)             */
/*   HIRES      Flag.  If TRUE, reults will be hi-res (pels*32)       */
/* and sets:                                                          */
/*   X, Y, Z (Bitmap pels, in range -R to R, -R to R                  */
/* Returns:                                                           */
/*   0 if OK, and given Long/Lat is visible                           */
/*   1 if OK, but given Long/Lat is not visible                       */
/*  -1 for invalid Long/Lat                                           */
/*                                                                    */
extern long trueradius, drawradius, drawvlon, drawvlat;   /* shared */

int ll2pel(long lon, long lat, int hires, long *px, long *py, long *pz)
 {
 long r32;                         /* radius*32 [r*2<=diam] */
 long x32, y32, z32;               /* bitmap coordinates *32 */
 long temp;                        /* work */
 long vlon, vlat;                  /* View long/lat */
 long vloncos, vlonsin;            /* cosine and sine of longitude */
 long vlatcos, vlatsin;            /* cosine and sine of latitude */

 /* if (diag && (labs(lon)>180000L || labs(lat)>90000L)) {
      printf("Bad L/L to LL2PEL: %li %li\n", lon, lat);
      return -1;} */

 r32 =trueradius;                  /* copy from Draw thread */
 /* drawradius is RUPXish: R-and-a-bit *32, and so not used */
 vlon=drawvlon;                    /* .. */
 vlat=drawvlat;                    /* .. */

 /* We need the X, Y, Z in bitmap coordinates.  To calculate   */
 /* this, we start with vector at 0,0, then rotate this by the */
 /* requested latitude and longitude                           */
 /* We then apply the world (view) longitude and latitude.     */
 /* The order of these operations is not optional.             */
 x32=0;
 y32=0;
 z32=r32;

 /* First apply our position.  Rotate (tilt) about X */
 if (lat!=0) {
   vlatcos=lcos(lat);
   vlatsin=lsin(lat);
   /* Here we would do the following, but X32 and Y32 are 0...    */
   /*   temp=(z32*vlatsin+y32*vlatcos)/1000L;                     */
   /*   z32 =(z32*vlatcos-y32*vlatsin)/1000L;                     */
   /*   y32 =temp;                                                */
   y32=(z32*vlatsin)/1000L;
   z32=(z32*vlatcos)/1000L;
   } /* LAT nonzero */

 /* The two longitude calculations can be merged, as LCOS/LSIN  */
 /* accept +/-360ø */
 /* Rotate about Y */

 /* Again, X32 is 0, so we would have done this, but simplified */
 /*                                                             */
 /*   vloncos=lcos(lon-vlon);                                   */
 /*   vlonsin=lsin(lon-vlon);                                   */
 /*   temp=(z32*vlonsin+x32*vloncos)/1000L;                     */
 /*   z32 =(z32*vloncos-x32*vlonsin)/1000L;                     */
 /*   x32 =temp;                                                */
 if ((lon-vlon)!=0) {
   vloncos=lcos(lon-vlon);
   vlonsin=lsin(lon-vlon);
   x32=(z32*vlonsin)/1000L;
   z32=(z32*vloncos)/1000L;
   }

 /* Finally, rotate (tilt) the globe about X, to allow for view */
 if (vlat!=0) {
   vlatcos=lcos(-vlat);
   vlatsin=lsin(-vlat);
   temp=(z32*vlatsin+y32*vlatcos)/1000L;
   z32 =(z32*vlatcos-y32*vlatsin)/1000L;
   y32 =temp;
   }

 /* Set the result x,y, high or low resolution */
 if (hires) {*px=x32; *py=y32; *pz=z32;}
  else {
   /* round to nearest pel */
   if (x32>=0) *px=(x32+16L)/32L; else *px=(x32-16L)/32L;
   if (y32>=0) *py=(y32+16L)/32L; else *py=(y32-16L)/32L;
   if (z32>=0) *pz=(z32+16L)/32L; else *pz=(z32-16L)/32L;
   }

 if (z32<0) return 1;    /* not visible */
 return 0;               /* visible */
 } /* ll2pel */


/*====================================================================*/
/* LL2STR -- format longitude and latitude to strings                 */
/*====================================================================*/
/* Formats a longitude and latitude into two strings.                 */
/*                                                                    */
/* It takes:                                                          */
/*   LLON, LLAT (longitutude/latitude *1000, e.g. 180000)             */
/* And sets:                                                          */
/*   SLON, SLAT: string-formatted versions of the same.               */
/*                                                                    */
/* Longest string is: "180ø 00' W" (10+1 bytes), but allow an extra   */
/* four in case add seconds later.                                    */
void ll2str(long llon, long llat, char *slon, char *slat) {
 char odir, adir;        /* N, S, E, W */
 long lonmins, latmins;  /* minutes, <1000 */
 if (llon<0) {odir='W'; llon=-llon;} else odir='E';
 if (llat<0) {adir='S'; llat=-llat;} else adir='N';
 /* Both are positive, now */
 lonmins=(llon%1000L)*60L/1000L;
 sprintf(slon, "%liø %02li' %c", llon/1000L, lonmins, odir);
 latmins=(llat%1000L)*60L/1000L;
 sprintf(slat, "%liø %02li' %c", llat/1000L, latmins, adir);
 } /* ll2str */

