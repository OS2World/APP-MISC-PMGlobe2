@rem Globe backup-to-diskette A:
@cd \globe
@move *.obj \temp        2>nul
@move *.o32 \temp        2>nul
@move *.map \temp        2>nul
@erase *.e16             2>nul
@erase *.e32             2>nul
 saveram *.* globe.ram
 copy        globe.ram a:
@erase       globe.ram    >nul
@move \temp\*.obj  .     2>nul
@move \temp\*.o32  .     2>nul
@move \temp\*.map  .     2>nul
