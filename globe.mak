# Make file for PMGlobe, 16-bit or 32-bit...

# ----- 16/32-bit dependent names -----
!IFNDEF B16
name=globe32
prtf=printf32
obj =obj
err =err
bits=32
!ELSE
name=globe
prtf=printf
obj =o16
err =e16
bits=16
!ENDIF

# ----- Name lists (need both for 16-bit) -----
mods =globe glober glpaint globemap globeut globecv gldo \
 glclient glcli2 gltimez globecol gldial globec globesq

objs =globe.$(obj) glober.$(obj) globemap.$(obj) globeut.$(obj) globecv.$(obj) \
 gldo.$(obj) glpaint.$(obj) glclient.$(obj) glcli2.$(obj) gltimez.$(obj) \
 globecol.$(obj) gldial.$(obj) globec.$(obj) globesq.$(obj)

utils =$(prtf) lacos  lcos
utilso=$(prtf).$(obj) lacos.$(obj) lcos.$(obj)

# ----- 16/32-bit dependent (application independent) -----
!IFNDEF B16
# 32-bit run
# Compile and link commands and options
#   add /Fa for assembler listing
comp=icc /c /Gm /O+ /Q /J /Kabgop /Fi /Si
stacksize=24576
links=$(objs) $(utilso)
libs=rexx
link=link386
linkopts=/NOI /NOL /MAP /PMTYPE:PM /ALIGN:4 /EXEPACK /BASE:0x10000 /STACK:$(stacksize)
!ELSE
# 16-bit run
# Compile and link commands and options
#   add /Fa for assembler listing
# add -Fc where required for output (code) listing
# add -Olta where required for optimization
comp=cl -c -G2s -W3 -Zdpel -Oltai -Alfu -nologo -DB16 -If:\ibmc2\include\mt
links=$(mods) $(utils)
# note 'def' file reference added here
libs=llibcmt rexx os2, globe
linkopts=/NOD /CODEVIEW /NOLOGO
link=link
!ENDIF

# ----- Control, link, and start -----
$(name).exe: $(objs) $(utilso) $(name).res globe.gml
  set $(link)=$(linkopts)
  $(link) $(links), $(name).exe, $(name).map, $(libs); | tee link.$(err)
  rc $(name).res $(name).exe
  start $(name) test diag name Test macro test 2> run$(bits).log
  makedoc.cmd
  reversio.cmd
  dir *.$(err)

# ----- Compiles -----
globe.$(obj):    globe.c globe.h globepm.h
  $(comp) /Fo$*.$(obj)     $*.c | tee $*.$(err)

glclient.$(obj): glclient.c globe.h globepm.h
  $(comp) /Fo$*.$(obj)     $*.c | tee $*.$(err)

glcli2.$(obj):   glcli2.c globe.h globepm.h
  $(comp) /Fo$*.$(obj)     $*.c | tee $*.$(err)

gldo.$(obj):     gldo.c globe.h globepm.h
  $(comp) /Fo$*.$(obj)     $*.c | tee $*.$(err)

gldial.$(obj):   gldial.c globe.h globepm.h
  $(comp) /Fo$*.$(obj)     $*.c | tee $*.$(err)

gltimez.$(obj):  gltimez.c globe.h globepm.h
  $(comp) /Fo$*.$(obj)     $*.c | tee $*.$(err)

glpaint.$(obj):  glpaint.c globe.h globepm.h
  $(comp) /Fo$*.$(obj)     $*.c | tee $*.$(err)

globec.$(obj):  globec.c globe.h globepm.h
  $(comp) /Fo$*.$(obj)     $*.c | tee $*.$(err)

globesq.$(obj):  globesq.c globe.h globepm.h
  $(comp) /Fo$*.$(obj)     $*.c | tee $*.$(err)

glober.$(obj):   glober.c globe.h globepm.h eot.h dec.h
  $(comp) /Fo$*.$(obj)     $*.c | tee $*.$(err)

globemap.$(obj): globemap.c globe.h landmap.h
  $(comp) /Fo$*.$(obj)     $*.c | tee $*.$(err)

globecol.$(obj): globecol.c globe.h
  $(comp) /Fo$*.$(obj)     $*.c | tee $*.$(err)

globeut.$(obj):  globeut.c globe.h
  $(comp) /Fo$*.$(obj)     $*.c | tee $*.$(err)

globecv.$(obj):  globecv.c globe.h
  $(comp) /Fo$*.$(obj)     $*.c | tee $*.$(err)

# Utilities don't have pre-reqs
$(prtf).$(obj): $(prtf).c
  $(comp) /Fo$*.$(obj)     $*.c | tee $*.$(err)

lacos.$(obj): lacos.c
  $(comp) /Fo$*.$(obj)     $*.c | tee $*.$(err)

lcos.$(obj): lcos.c
  $(comp) /Fo$*.$(obj)     $*.c | tee $*.$(err)

$(name).res: globe.rc globe.h globepm.h globe.ico globe.ptr
  rc -r globe.rc $(name).res | tee globerc.$(err)



