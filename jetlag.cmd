/* Program to analyse jetlag effects.                                */
/* This program uses a modification of Buley's algorithm for medical */
/* estimation of required rest time for jet lag compensation.        */
/* Author: Mike Cowlishaw, October 1983                              */

say 'This program is intended as a guide to travellers who cross time zones.'
say 'It will give you a "severity" rating which will warn you how badly you may'
say 'be affected by jet-lag, and will also tell you how much rest you should'
say 'take after arrival before making important decisions, etc.'
say
call gettime 'Please enter your departure time',
             '(local, 24 hour clock, eg 17:30):'
dep=result
call gettime 'Please enter your arrival time',
             '(local, 24 hour clock, eg 9:00):'
arr=result
call getnum 'How many time zones will you cross? (i.e. hours to change watch by):'
zones=result
say 'Are you flying from West -> East?  (Please answer Y for Yes or N for No):'
pull dir +1
if dir='Y' then way=-1
 else if dir='N' then way=1
 else do
  say "(OK, I'll assume No.)"; way=1; end
say
/* Now we have valid DEP, ARR, ZONES, WAY */
eff=arr+(zones*way)          /* effective arrival time */
if eff<=dep then eff=eff+24  /* past midnight */
flight=eff-dep               /* flight time in hours */
if flight<zones/2 then flight=flight+24  /* for extreme journeys */
/* Now get the coefficients */
call init
/* Calculate coefficients for a given time */
tt=dep
hh=trunc(tt); nhh=hh+1; dd=tt-hh
low=word(co.hh,1); high=word(co.nhh,1)
cdep=low+(high-low)*dd
tt=arr
hh=trunc(tt); nhh=hh+1; dd=tt-hh
low=word(co.hh,2); high=word(co.nhh,2)
carr=low+(high-low)*dd

/* Now the magic formula */
rest=(flight/2 + (zones-4) + cdep + carr) *2.4  /* hours */
if rest<0 then rest=0

severity=min(trunc(rest*10/50),9)

trace off
/* hi='1de8'x; lo='1d60'x; 'DEPTH'; if rc<24 then */
do; hi=' '; lo=' '; end
say '       Calculated flight time is'hi||format(flight,,1)||lo'hours.'
say '           The jet-lag effect is'hi||adj.severity'.'lo
say 'You should rest for a minimum of'hi||format(rest,,1)||lo,
    ||'hours before any important decisions.'
say
exit

/* Get a time value into decimal */
gettime: procedure
 parse arg message
 do forever
   say message
   pull answer .
   if answer='QUIT' then exit
   if answer='' then say 'Please try again (or type Quit to exit).'
   parse var answer hh':'mm .
   if datatype(hh)^='NUM' then iterate
   if datatype(mm)^='NUM' then iterate
   if mm<0 | mm>59 | hh<0 | hh>23 then iterate
   return hh+(mm/60)
   end

/* Get a number of timezones - note non-integers are OK */
getnum: procedure
 parse arg message
 do forever
   say message
   pull ans .
   if answer='QUIT' then exit
   if answer='' then say 'Please try again (or type QUIT to exit).'
   if datatype(ans)^='NUM' then iterate
   if ans<0 | ans>23 then do
     say 'Number of time zones must be 0 -> 23'; iterate; end
   return ans
   end

/* Initialise coefficients array (Buley, modified by MFC 1983) */
init:
/*     Dept.  Arr.  */
co.0 ='4     1.5     '
co.1 ='3.9   2.5     '
co.2 ='3.8   3       '
co.3 ='3.7   3       '
co.4 ='3.6   3       '
co.5 ='3.5   3       '
co.6 ='3     3.2     '
co.7 ='2     3.5     '
co.8 ='1     4       '
co.9 ='0     4       '
co.10='0     4       '
co.11='0     3.5     '
co.12='0.5   2.5     '
co.13='0.9   2       '
co.14='1     2       '
co.15='1     2       '
co.16='1     2       '
co.17='2     1.5     '
co.18='2.7   0.5     '
co.19='3     0       '
co.20='3     0       '
co.21='3     0       '
co.22='3.5   0.5     '
co.23='4     1.5     '
co.24=co.0  /* Provide wrap */

/* Severity ratings */
adj.0='negligible'
adj.1='small'
adj.2='minor'
adj.3='fairly bad'
adj.4='bad'
adj.5='fairly severe'
adj.6='severe'
adj.7='very severe'
adj.8='extremely severe'
adj.9='as bad as it gets'
return

