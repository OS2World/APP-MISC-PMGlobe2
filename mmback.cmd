cd \makemap
move \makemap\geodata3.dat \temp >nul
saveram *.* makemap.ram
move \temp\geodata3.dat \makemap >nul
copy makemap.ram a:
erase makemap.ram >nul
