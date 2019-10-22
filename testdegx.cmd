/* Test GETDEG */
say "Following should all be good:"
'testdeg 12 12.5 -12.05 12.005'
'testdeg 12o -12ø 12O30 12o30'' 12o30''12 12o30''12\"'
'testdeg lon 145o30E 144o30W -144o3W'
'testdeg lat  45o30N  44o30S  -44o3S'
'testdeg lat  45.3S 44o30''S 44o30''0S 44o30''30s 44o30''30"s'


say "Following should all be bad:"
'testdeg foo +-12 *12 12..1 12.1.2 12.1234'
'testdeg 360.001 o O ø '' \" . - '
'testdeg 12o60 12o0''60 360o00''05'
'testdeg lat 145o30E 144o30W -144o3'
'testdeg 145o30N 144o30W -144o3E 12S'


