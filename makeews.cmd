/* This will make a GLOBE2.ZIP file in the GLOBE2 directory */
/* We explicitly make our own list here */
'@md globe2 2> nul'
'cd globe2'
'@echo y | erase *.* 1>nul 2>nul'
'@copy \globe\globe.abs    pmglobe.abs'
'@copy \globe\globe.doc    pmglobe.doc'
'@copy \globe\glcomm.doc   pmglobex.doc'
'@copy \globe\globe32.exe  pmglobe2.exe'
'@copy \globe\test.pmg     test.pmg'
'@copy \globe\spin.pmg     spin.pmg'
'@copy \globe\fastspin.pmg fastspin.pmg'
'@copy \globe\os2ews.asi   license.txt'
'zip globe2.zip *.*'
/* 'zip2exe globe2.zip' */

