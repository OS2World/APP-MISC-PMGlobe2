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
/* Long ACOS(x)                                      */
/*   X is *1000: -1000 to 1000 (for -1.000 to 1.000) */
/*   returns degrees*1000, i.e, 0->180000            */
/*   ACOS(-1)=180', ACOS(0)=90', ACOS(1)=0'          */
/*   table is degrees *500 i.e, 0->45000             */

extern int diag;
#include <stdio.h>
long lacos(long v) {
  static unsigned int table[1001]={
  45000, 44971, 44943, 44914, 44885, 44857, 44828, 44799, 44771, 44742,
  44714, 44685, 44656, 44628, 44599, 44570, 44542, 44513, 44484, 44456,
  44427, 44398, 44370, 44341, 44312, 44284, 44255, 44226, 44198, 44169,
  44140, 44112, 44083, 44054, 44026, 43997, 43968, 43940, 43911, 43882,
  43854, 43825, 43796, 43768, 43739, 43710, 43682, 43653, 43624, 43596,
  43567, 43538, 43510, 43481, 43452, 43424, 43395, 43366, 43337, 43309,
  43280, 43251, 43223, 43194, 43165, 43137, 43108, 43079, 43050, 43022,
  42993, 42964, 42936, 42907, 42878, 42849, 42821, 42792, 42763, 42734,
  42706, 42677, 42648, 42619, 42591, 42562, 42533, 42504, 42476, 42447,
  42418, 42389, 42361, 42332, 42303, 42274, 42246, 42217, 42188, 42159,
  42130, 42102, 42073, 42044, 42015, 41986, 41958, 41929, 41900, 41871,
  41842, 41814, 41785, 41756, 41727, 41698, 41669, 41641, 41612, 41583,
  41554, 41525, 41496, 41467, 41438, 41410, 41381, 41352, 41323, 41294,
  41265, 41236, 41207, 41179, 41150, 41121, 41092, 41063, 41034, 41005,
  40976, 40947, 40918, 40889, 40860, 40831, 40802, 40773, 40744, 40716,
  40687, 40658, 40629, 40600, 40571, 40542, 40513, 40484, 40455, 40426,
  40397, 40368, 40338, 40309, 40280, 40251, 40222, 40193, 40164, 40135,
  40106, 40077, 40048, 40019, 39990, 39961, 39932, 39902, 39873, 39844,
  39815, 39786, 39757, 39728, 39699, 39669, 39640, 39611, 39582, 39553,
  39524, 39494, 39465, 39436, 39407, 39378, 39348, 39319, 39290, 39261,
  39232, 39202, 39173, 39144, 39115, 39085, 39056, 39027, 38997, 38968,
  38939, 38910, 38880, 38851, 38822, 38792, 38763, 38734, 38704, 38675,
  38645, 38616, 38587, 38557, 38528, 38499, 38469, 38440, 38410, 38381,
  38351, 38322, 38293, 38263, 38234, 38204, 38175, 38145, 38116, 38086,
  38057, 38027, 37998, 37968, 37939, 37909, 37880, 37850, 37820, 37791,
  37761, 37732, 37702, 37672, 37643, 37613, 37584, 37554, 37524, 37495,
  37465, 37435, 37406, 37376, 37346, 37317, 37287, 37257, 37227, 37198,
  37168, 37138, 37108, 37079, 37049, 37019, 36989, 36959, 36930, 36900,
  36870, 36840, 36810, 36780, 36750, 36721, 36691, 36661, 36631, 36601,
  36571, 36541, 36511, 36481, 36451, 36421, 36391, 36361, 36331, 36301,
  36271, 36241, 36211, 36181, 36151, 36121, 36091, 36061, 36031, 36001,
  35970, 35940, 35910, 35880, 35850, 35820, 35789, 35759, 35729, 35699,
  35669, 35638, 35608, 35578, 35547, 35517, 35487, 35457, 35426, 35396,
  35366, 35335, 35305, 35275, 35244, 35214, 35183, 35153, 35122, 35092,
  35062, 35031, 35001, 34970, 34940, 34909, 34879, 34848, 34817, 34787,
  34756, 34726, 34695, 34665, 34634, 34603, 34573, 34542, 34511, 34481,
  34450, 34419, 34388, 34358, 34327, 34296, 34265, 34235, 34204, 34173,
  34142, 34111, 34080, 34050, 34019, 33988, 33957, 33926, 33895, 33864,
  33833, 33802, 33771, 33740, 33709, 33678, 33647, 33616, 33585, 33554,
  33523, 33492, 33460, 33429, 33398, 33367, 33336, 33305, 33273, 33242,
  33211, 33180, 33148, 33117, 33086, 33054, 33023, 32992, 32960, 32929,
  32898, 32866, 32835, 32803, 32772, 32740, 32709, 32677, 32646, 32614,
  32583, 32551, 32520, 32488, 32456, 32425, 32393, 32361, 32330, 32298,
  32266, 32234, 32203, 32171, 32139, 32107, 32076, 32044, 32012, 31980,
  31948, 31916, 31884, 31852, 31820, 31788, 31756, 31724, 31692, 31660,
  31628, 31596, 31564, 31532, 31500, 31468, 31435, 31403, 31371, 31339,
  31306, 31274, 31242, 31210, 31177, 31145, 31113, 31080, 31048, 31015,
  30983, 30950, 30918, 30885, 30853, 30820, 30788, 30755, 30723, 30690,
  30657, 30625, 30592, 30559, 30527, 30494, 30461, 30428, 30395, 30363,
  30330, 30297, 30264, 30231, 30198, 30165, 30132, 30099, 30066, 30033,
  30000, 29967, 29934, 29901, 29868, 29834, 29801, 29768, 29735, 29701,
  29668, 29635, 29601, 29568, 29535, 29501, 29468, 29434, 29401, 29367,
  29334, 29300, 29267, 29233, 29200, 29166, 29132, 29099, 29065, 29031,
  28997, 28963, 28930, 28896, 28862, 28828, 28794, 28760, 28726, 28692,
  28658, 28624, 28590, 28556, 28522, 28488, 28453, 28419, 28385, 28351,
  28316, 28282, 28248, 28213, 28179, 28145, 28110, 28076, 28041, 28007,
  27972, 27938, 27903, 27868, 27834, 27799, 27764, 27729, 27695, 27660,
  27625, 27590, 27555, 27520, 27485, 27450, 27415, 27380, 27345, 27310,
  27275, 27240, 27204, 27169, 27134, 27099, 27063, 27028, 26992, 26957,
  26921, 26886, 26850, 26815, 26779, 26744, 26708, 26672, 26637, 26601,
  26565, 26529, 26493, 26457, 26422, 26386, 26350, 26314, 26277, 26241,
  26205, 26169, 26133, 26097, 26060, 26024, 25988, 25951, 25915, 25878,
  25842, 25805, 25769, 25732, 25696, 25659, 25622, 25585, 25549, 25512,
  25475, 25438, 25401, 25364, 25327, 25290, 25253, 25216, 25179, 25141,
  25104, 25067, 25029, 24992, 24955, 24917, 24880, 24842, 24805, 24767,
  24729, 24691, 24654, 24616, 24578, 24540, 24502, 24464, 24426, 24388,
  24350, 24312, 24274, 24235, 24197, 24159, 24120, 24082, 24044, 24005,
  23966, 23928, 23889, 23850, 23812, 23773, 23734, 23695, 23656, 23617,
  23578, 23539, 23500, 23461, 23421, 23382, 23343, 23303, 23264, 23224,
  23185, 23145, 23106, 23066, 23026, 22986, 22947, 22907, 22867, 22827,
  22786, 22746, 22706, 22666, 22626, 22585, 22545, 22504, 22464, 22423,
  22383, 22342, 22301, 22260, 22219, 22178, 22137, 22096, 22055, 22014,
  21973, 21931, 21890, 21849, 21807, 21766, 21724, 21682, 21641, 21599,
  21557, 21515, 21473, 21431, 21389, 21346, 21304, 21262, 21219, 21177,
  21134, 21092, 21049, 21006, 20963, 20920, 20877, 20834, 20791, 20748,
  20705, 20661, 20618, 20575, 20531, 20487, 20444, 20400, 20356, 20312,
  20268, 20224, 20180, 20135, 20091, 20047, 20002, 19957, 19913, 19868,
  19823, 19778, 19733, 19688, 19643, 19597, 19552, 19507, 19461, 19415,
  19370, 19324, 19278, 19232, 19186, 19140, 19093, 19047, 19001, 18954,
  18907, 18860, 18814, 18767, 18720, 18672, 18625, 18578, 18530, 18483,
  18435, 18387, 18339, 18291, 18243, 18195, 18147, 18098, 18050, 18001,
  17952, 17903, 17854, 17805, 17756, 17706, 17657, 17607, 17557, 17508,
  17458, 17407, 17357, 17307, 17256, 17206, 17155, 17104, 17053, 17002,
  16951, 16899, 16848, 16796, 16744, 16692, 16640, 16588, 16535, 16483,
  16430, 16377, 16324, 16271, 16218, 16164, 16110, 16057, 16003, 15948,
  15894, 15840, 15785, 15730, 15675, 15620, 15565, 15509, 15454, 15398,
  15342, 15285, 15229, 15172, 15116, 15059, 15001, 14944, 14886, 14829,
  14771, 14712, 14654, 14595, 14537, 14478, 14418, 14359, 14299, 14239,
  14179, 14118, 14058, 13997, 13936, 13874, 13813, 13751, 13689, 13626,
  13563, 13500, 13437, 13374, 13310, 13246, 13181, 13117, 13052, 12987,
  12921, 12855, 12789, 12722, 12656, 12588, 12521, 12453, 12385, 12316,
  12247, 12178, 12108, 12038, 11968, 11897, 11826, 11754, 11682, 11610,
  11537, 11464, 11390, 11316, 11241, 11166, 11090, 11014, 10937, 10860,
  10783, 10704, 10626, 10546, 10466, 10386, 10305, 10223, 10141, 10058,
  9974, 9890, 9805, 9719, 9633, 9546, 9458, 9369, 9279, 9189, 9097, 9005,
  8912, 8818, 8723, 8627, 8530, 8432, 8332, 8232, 8130, 8027, 7923, 7817,
  7710, 7602, 7492, 7380, 7267, 7152, 7035, 6916, 6795, 6672, 6547, 6419,
  6289, 6156, 6020, 5881, 5739, 5593, 5444, 5290, 5132, 4968, 4799, 4624,
  4443, 4253, 4055, 3846, 3626, 3392, 3140, 2866, 2563, 2220, 1812, 1281,
  0
  };
  static int warnflag=0;           /* set once warned */
  if (v<0) {if (v>=-1000L)
             return (90000L-(long)table[-v])*2L;}
      else {if (v<= 1000L)
             return (long)(table[v])*2L;}
  if (!warnflag) {
    if (diag) printf("Bad value to LACOS: %li\n",v);
    warnflag=1;
    }
  return 0;
  }; /* lacos */
