# Make file for TESTDEG

common=
all=testdeg.obj
comp=cl -c -Gs -Zalpd -Alfu -nologo -W3

# Make the executable file
testdeg.exe: $(all)
  link /NOD /CODEVIEW /NOLOGO $(all),$@,$*, llibce doscalls /stack:5000;
# Next line needed for a Family app
# bind nn.exe d:\toolkt11\lib\api.lib c:\os2\doscalls.lib
  family testdeg.exe
  winable testdeg.exe

testdeg.obj:  $(common) testdeg.c
  $(comp)  $*.c /DOS2

# Utilities
# streq.obj:  streq.c
#   $(comp)  $*.c



