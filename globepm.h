/* ------------------------------------------------------------------ */
/* PMGlobe -- a geographical application -- PM-related includes       */
/* ------------------------------------------------------------------ */
/* Copyright (c) Mike Cowlishaw, 1993-2019.  All rights reserved.     */
/* Parts Copyright (c) IBM, 1993-2009.                                */

/* ID's for the application */
/* SOme of these may not be used */
#define ID_OPTIONS  100
#define ID_RUNMAC    101
#define ID_HALTMAC   102

#define ID_SQUARE    111
#define ID_FULL      112
#define ID_QUARTER   113
#define ID_DESKTOP   114

#define ID_SHOWDC    121
#define ID_CROSSHAIR 122
#define ID_IDLEDRAW  123

#define ID_TIMEZONE  131
     /* ID_REFRESH */

#define ID_MENUBAR   141
#define ID_TITLEBAR  142
#define ID_SAVESET   143
#define ID_RESTSET   144
#define ID_DEFSET    145
#define ID_SAVEPOS   146
#define ID_RESTPOS   147
#define ID_MINIMIZE  148
#define ID_MAXIMIZE  149
#define ID_RESTORE   150
#define ID_QUIT      151

/* Refresh IDs */
#define ID_REFRESH   170
#define ID_REF0       171
#define ID_REF1       172
#define ID_REF5       173
#define ID_REF10      174
#define ID_REF15      175
#define ID_REF20      176
#define ID_REF30      177
#define ID_REF60      178

/* Views */
#define ID_VIEW     200
#define ID_POSMB2    201
#define ID_POSEQ     202
#define ID_POSGM     203
#define ID_POSITION  210
#define ID_POSNP      211
#define ID_POSSP      212
#define ID_POS90W     213
#define ID_POS90E     214
#define ID_POS180     215
#define ID_POS0       216
#define ID_GRID      220
#define ID_GRIDLON10  221
#define ID_GRIDLON15  222
#define ID_GRIDLON30  223
#define ID_GRIDLAT10  224
#define ID_GRIDLAT15  225
#define ID_GRIDLAT30  226
#define ID_GRIDEQ     227
#define ID_GRIDARCTIC 228
#define ID_GRIDTROPIC 229
#define ID_GRIDNONE  230
#define ID_GRIDALL   231
/* Colours for grid have any numbers */
#define ID_CGRID     240
#define ID_CGWHITE     241
#define ID_CGBLACK     242
#define ID_CGRED       243
#define ID_CGDARKRED   244
#define ID_CGGREEN     245
#define ID_CGDARKGREEN 246
#define ID_CGBLUE      247
#define ID_CGDARKBLUE  248
#define ID_CGCYAN      249
#define ID_CGDARKCYAN  250
#define ID_CGYELLOW    251
#define ID_CGBROWN     252
#define ID_CGPINK      253
#define ID_CGPURPLE    254
#define ID_CGGRAY      255

/* Lighting */
#define ID_LIGHT    300
#define ID_SUN       310
#define ID_3D        320
#define ID_SPACE     330
#define ID_STAR      340
/* Twilight range can have any numbers, we use approx 350+deg */
#define ID_TWILIGHT  350
#define ID_TWI0       351
#define ID_TWIZ50     352
#define ID_TWI5       355
#define ID_TWI6       356
#define ID_TWI12      362
#define ID_TWI18      368
/* Land and Sea colours have any numbers */
#define ID_CLAND     370
#define ID_CLWHITE     371
#define ID_CLRED       372
#define ID_CLGREEN     373
#define ID_CLBLUE      374
#define ID_CLCYAN      375
#define ID_CLYELLOW    376
#define ID_CLPINK      377
#define ID_CLGRAY      378
#define ID_CLBLACK     379
#define ID_CSEA      380
#define ID_CSWHITE     381
#define ID_CSRED       382
#define ID_CSGREEN     383
#define ID_CSBLUE      384
#define ID_CSCYAN      385
#define ID_CSYELLOW    386
#define ID_CSPINK      387
#define ID_CSGRAY      388
#define ID_CSBLACK     389
/* Background colours have any numbers */
#define ID_CBACK     400
#define ID_CBWHITE     401
#define ID_CBBLACK     402
#define ID_CBRED       403
#define ID_CBDARKRED   404
#define ID_CBGREEN     405
#define ID_CBDARKGREEN 406
#define ID_CBBLUE      407
#define ID_CBDARKBLUE  408
#define ID_CBCYAN      409
#define ID_CBDARKCYAN  410
#define ID_CBYELLOW    421
#define ID_CBBROWN     422
#define ID_CBPINK      423
#define ID_CBPURPLE    424
#define ID_CBGRAY      425
#define ID_CBPALEGRAY  426

/* Macro ID's */
#define ID_MACRO     490
#define ID_MACROHALT  491

/* Help ID's should start at a multiple of 16 (for PM).  */
/* Each has two strings in the string table, the first   */
/* being the title string and the second being the text. */
#define ID_HELP      500
#define ID_HELPABOUT  512
#define ID_HELPOPTS   514
#define ID_HELPLIGHT  516
#define ID_HELPSTAN   518
#define ID_HELPVIEWS  520
#define ID_HELPREF    522
#define ID_HELPGRID   524
#define ID_HELPINTRO  526
#define ID_HELPTWI    528
#define ID_HELPBACK   530
#define ID_HELPMACRO  532
#define ID_HELPCOLOUR 534
/* etc... */

/* ID's for the timezone dialogue. */
#define ID_TZDLG     700
#define ID_TZLIST     701
#define ID_TZTEXT1    702
#define ID_TZHOURS    703
#define ID_TZTEXT2    704
#define ID_TZZONE     705
#define ID_TZTEXT3    706
/* Next four increment by 1 for each half hour  (DAY3 not shown) */
#define ID_TZDAY0     710
#define ID_TZDAY1     711
#define ID_TZDAY2     712
#define ID_TZDAY3     713
#define ID_TZDAY4     714
/* Timezone buttons */
#define ID_TZOK       720
#define ID_TZCANCEL   721

/* ID's for the distance calculator dialogue */
#define ID_DISTCALC   800
#define ID_DCBLURB     811
/* Mouse.. */
#define ID_DCMGROUP   820
#define ID_DCMTEXT     821
#define ID_DCMLON      822
#define ID_DCMLAT      823
/* Selected points */
#define ID_DCPGROUP   830
#define ID_DC1TEXT     831
#define ID_DC1LON      832
#define ID_DC1LAT      833
#define ID_DC2TEXT     841
#define ID_DC2LON      842
#define ID_DC2LAT      843
/* Distances */
#define ID_DCDTEXT     851
#define ID_DCDKM       852
#define ID_DCDMILES    853
#define ID_DCTTEXT     854
#define ID_DCTKM       855
#define ID_DCTMILES    856
/* Buttons */
#define ID_DCRESET     861
#define ID_DCTRACK     862
#define ID_DCCANCEL    863

/* ID's for the command line dialogue */
#define ID_COMMDLG     900
#define ID_COMMDLGNAME   901
#define ID_COMMDLGDRAW   902
#define ID_COMMDLGMAC    903
#define ID_COMMDLGTEXT   904
#define ID_COMMDLGHALT   905

#define ID_FRAMERC  2000
#define ID_GLOBEPTR 2001

/* ----- User messages ----- */
#define UM_CREATED   WM_USER+0     /* created */
#define UM_DRAWN     WM_USER+1     /* landmap ready */
#define UM_GLOBE     WM_USER+2     /* draw a globe */
#define UM_NOMAP     WM_USER+3     /* map could not be built */
#define UM_TIMEZONE  WM_USER+4     /* run timezone dialogue */
#define UM_ALERT     WM_USER+5     /* insufficient resources */
#define UM_REDRAW    WM_USER+6     /* redraw globe please */
#define UM_DCRESET   WM_USER+10    /* distance calculator Reset */
#define UM_DCTRACK   WM_USER+11    /* .. TRACK  button */
#define UM_DCCANCEL  WM_USER+12    /* .. CANCEL button */
#define UM_COMMDO    WM_USER+20    /* command run start */
#define UM_COMMDONE  WM_USER+21    /* command run completed */
#define UM_COMMHALT  WM_USER+22    /* command halt */
#define UM_COMMFOCUS WM_USER+23    /* command refocus */
#define UM_MACROBEG  WM_USER+30    /* start macro/command */
#define UM_MACROEND  WM_USER+31    /* end macro/command */
#define UM_TODESK    WM_USER+40    /* move to desk */
#define UM_FROMDESK  WM_USER+41    /* up from desk, please */
#define UM_TEST      WM_USER+42    /* test message */
/* Reserved: all id's that match WM_USER+ID_HELPxxxx (i.e., +5xx) */

/* ----- Timer ID ----- */
#define TIMERID     TID_USERMAX-1  /* "Must be below USERMAX" */

/* Useful macros */
#define mp1l  LONGFROMMP(mp1)
#define mp2l  LONGFROMMP(mp2)
#define mp1s1 SHORT1FROMMP(mp1)
#define mp1s2 SHORT2FROMMP(mp1)
#define mp2s1 SHORT1FROMMP(mp2)
#define mp2s2 SHORT2FROMMP(mp2)
#define MFALSE   (MRESULT)FALSE
#define MTRUE    (MRESULT)TRUE
#define MDEFAULT WinDefWindowProc(hwnd, msg, mp1, mp2);

