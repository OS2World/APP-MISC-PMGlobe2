 /* Table (0-366) for the equation of time adjustment */
 /* (This allows 366 days, plus interpolation using   */
 /*  day 367)                                         */
 /* Values approx: -990 to +990 seconds.              */
 /* ------------------------------------------------- */
 /*                                                   */
 /* Copyright (c) Mike Cowlishaw, 1993-2012.          */
 /* All rights reserved.                              */
 /* Parts Copyright (c) IBM, 1993-2009.               */
 /* --------------------------------------------------*/
 static short int eottab[367]={
    -197, -226, -254, -281, -308, -335, -362, -387, -413, -437, -462, -485
  , -508, -531, -552, -573, -594, -614, -633, -651, -669, -686, -702, -717
  , -732, -746, -759, -771, -782, -793, -802, -811, -819, -827, -833, -839
  , -843, -847, -850, -853, -854, -855, -855, -854, -853, -850, -847, -844
  , -839, -834, -829, -822, -815, -808, -800, -791, -781, -771, -761, -750
  , -738, -726, -714, -700, -687, -673, -658, -644, -629, -613, -597, -581
  , -565, -548, -531, -514, -496, -479, -461, -444, -426, -408, -390, -372
  , -354, -335, -317, -299, -281, -263, -245, -227, -210, -192, -174, -157
  , -140, -123, -106,  -90,  -73,  -57,  -42,  -26,  -12,    3,   17,   31
  ,   45,   58,   70,   83,   94,  106,  116,  127,  136,  146,  154,  163
  ,  171,  178,  185,  191,  197,  202,  206,  210,  214,  217,  219,  221
  ,  222,  223,  223,  222,  221,  219,  217,  214,  211,  207,  202,  197
  ,  191,  185,  179,  172,  164,  156,  148,  139,  130,  121,  111,  101
  ,   90,   80,   68,   57,   45,   34,   21,    9,   -3,  -16,  -29,  -42
  ,  -55,  -68,  -81,  -94, -107, -121, -134, -147, -159, -172, -185, -197
  , -209, -221, -233, -244, -255, -266, -276, -286, -296, -305, -314, -322
  , -330, -338, -345, -351, -358, -363, -368, -373, -377, -381, -384, -386
  , -388, -389, -390, -390, -390, -388, -387, -384, -381, -378, -373, -368
  , -363, -357, -350, -343, -335, -326, -317, -308, -298, -287, -276, -264
  , -252, -239, -226, -212, -198, -184, -168, -153, -137, -120, -103,  -86
  ,  -69,  -51,  -32,  -14,    6,   25,   44,   64,   84,  105,  125,  146
  ,  167,  188,  209,  230,  251,  272,  294,  315,  336,  358,  379,  400
  ,  421,  442,  463,  484,  505,  526,  546,  566,  586,  606,  625,  644
  ,  663,  682,  700,  718,  735,  752,  768,  784,  800,  815,  829,  843
  ,  856,  869,  881,  893,  904,  914,  924,  933,  941,  949,  956,  962
  ,  967,  972,  976,  980,  982,  984,  985,  985,  984,  983,  980,  977
  ,  973,  968,  962,  956,  948,  940,  930,  920,  909,  898,  885,  871
  ,  857,  842,  826,  810,  792,  774,  755,  736,  715,  694,  673,  651
  ,  628,  604,  580,  555,  530,  504,  478,  451,  424,  397,  369,  340
  ,  312,  283,  253,  224,  194,  165,  135,  105,   75,   45,   15,  -14
  ,  -44,  -74, -103, -132, -161, -190, -219};
