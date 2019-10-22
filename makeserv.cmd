/* Rudimentary server */
'@echo off'
call makeews

'cd \wwwroot'
'md pmglobe 2>nul'
'cd pmglobe'
'copy \globe\globe2\globe2.zip'
'copy \globe\globe.gml pmglobe.gml'
'copy \globe\globe2\pmglobe.doc'
'copy \globe\glcomm.gml pmglobex.gml'
'copy \globe\globe2\pmglobex.doc'

