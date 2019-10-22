/* Process EOT.DAT into a C declaration */
/* 1998.08.15 -- also write a binary resource */
infile='eot.dat'
outfile='eot.h'
outbin ='eot.brc'
'@echo off'
'erase' outfile '2> nul'
'erase' outbin  '2> nul'

call out ' /* Table (0-366) for the equation of time adjustment */'
call out ' /* (This allows 366 days, plus interpolation using   */'
call out ' /*  day 367)                                         */'
call out ' /* Values approx: -990 to +990 seconds.              */'
call out ' static short int eottab[367]={'

days=0
line='   '
do while lines(infile)>0
  parse value linein(infile) with mins secs .
  c1=left(mins,1)
  if c1='*' then iterate /* comment */
  if c1=' ' then iterate /* blank line */
  if c1='+' then do; sign=+1; iterate; end
  if c1='-' then do; sign=-1; iterate; end
  /* must indeed be mins/secs */
  if secs>59 then say 'Yeeeeeechhh seconds' secs
  secs=sign*(secs+mins*60) /* total secs */
  if days<>0 then line=line','
  line=line right(secs,4)
  if length(line)>68 then do; call out line; line='  '; end
  call charout outbin, d2c(secs,2)
  days=days+1
  end
call out line'};'
call lineout outfile
call lineout outbin

say days 'days were found in the data file.'
if days<>367 then say '**** SHOULD HAVE BEEN 367 ****'
/* Two values for interpolation included above */
exit

out: call lineout outfile, arg(1)
return



