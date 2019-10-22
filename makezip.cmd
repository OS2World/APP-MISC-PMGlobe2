/* This will make a PMGLOBE.ZIP file */
/* We explicitly make our own list here */
'@md globezip 2> nul'
'cd globezip'
'@copy \globe\globe.doc    pmglobe.doc'
'@copy \globe\glcomm.doc   pmglobex.doc'
'@copy \globe\globe.exe    pmglobe.exe'
'@copy \globe\globe32.exe  pmglobe2.exe'
'@copy \globe\test.pmg     test.pmg'
'@copy \globe\testspin.pmg testspin.pmg'
'zip pmglobez.zip *.*'
'zip2exe pmglobez.zip'



