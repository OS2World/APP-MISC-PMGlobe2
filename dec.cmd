/* Process DEC.DAT into a C declaration */
/* 1998.08.15 -- also write a binary resource */
infile ='dec.dat'
outfile='dec.h'
outbin ='dec.brc'
'@echo off'
'erase' outfile '2> nul'
'erase' outbin  '2> nul'

call out ' /* Table (0-366) for the declination calculation     */'
call out ' /* (This allows 366 days, plus interpolation using   */'
call out ' /*  day 367)                                         */'
call out ' /* Values in range: -23500 to +23500 millidegrees.   */'
call out ' static short int dectab[367]={'

days=0
line='   '
do lnum=1 by 1 while lines(infile)>0
  parse value linein(infile) with degs mins .
  c1=left(degs,1)
  if c1='*' then iterate /* comment */
  if c1=' ' then iterate /* blank line */
  if c1='+' then do; sign=+1; iterate; end
  if c1='-' then do; sign=-1; iterate; end
  /* must indeed be degs/mins */
  if mins>59 then say 'Yeeeeeechhh minutes' mins
  mdegs=sign*format((degs+mins/60)*1000,,0)  /* millidegrees */
  if days<>0 then do
    line=line','
    if abs(lastmdegs-mdegs)>500 then
      say "Large interval on line" lnum
    end
  line=line mdegs
  if length(line)>68 then do; call out line; line='  '; end
  call charout outbin, d2c(mdegs,2)
  lastmdegs=mdegs
  days=days+1
  end
call out line'};'
call lineout outfile
call lineout outbin

say days 'days were found in the data file' infile
if days<>367 then say '**** SHOULD HAVE BEEN 367 ****'
/* Two values for interpolation included above */
exit

out: call lineout outfile, arg(1)
return



