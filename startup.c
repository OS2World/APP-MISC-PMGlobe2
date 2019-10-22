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
       /* special case 'progress' display */
       if (startup) {
         /* Now, if suitably long delay since draw, redraw the bitmap */
         newtime=(signed long)WinGetCurrentTime(habGlobe);
         if ((newtime-lasttime)>TIMEGRID  /* time for an update */
            || newtime<lasttime) {        /* timer wrapped */
           redraw();
           features=FALSE;                /* all drawn so far */
           lasttime=(signed long)WinGetCurrentTime(habGlobe); /* exclude draw time */
           } /* timed redraw */
         } /* startup */