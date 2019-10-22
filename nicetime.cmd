
/* TOTIME(seconds) -- convert winter GMT offset to current */
/* Answer is returned in +/-hh:mm:ss format */
totime: procedure expose dayoffset
  arg hours .                             /* (Unchecked) */
  if dayoffset='UNKNOWN' then dayoffset=0 /* just in case */
  secs=hours*3600+dayoffset               /* Adjust and convert to seconds */
  ss=secs//60; secs=secs%60; mm=secs//60; time=secs%60   /* split */
  if mm+ss<>0 then time=time':'right(abs(mm),2,0)
  if ss<>0    then time=time':'right(abs(ss),2,0)
  return time

