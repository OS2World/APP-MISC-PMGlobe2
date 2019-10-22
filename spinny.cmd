/* Set up a spinning globe object on the desktop */
/* Run this from the directory that contains PMGLOBE2.EXE */

  path=directory()         /* PMGlobe directory */
  exen='PMGLOBE2'          /* .EXE name */
  name="Rotating Globe"    /* Object name */

  If RxFuncQuery('SysLoadFuncs') Then Do
    Call RxFuncAdd 'SysLoadFuncs', 'REXXUTIL', 'SysLoadFuncs'
    Call SysLoadFuncs
    End
  Call SysCreateObject "WPProgram", name, "<WP_DESKTOP>",,
        "EXENAME="path"\"exen".EXE;"||,
        "PARAMETERS=macro fastspin name spin;"||,
        "PROGTYPE=PM;"||,
        "MINWIN=DESKTOP;"||,
        "STARTUPDIR="path";",,
        "F"
  If result<>1 then Say "Unable to create the object '"name"'."
   else say "The object '"name"' has been created on desktop."
