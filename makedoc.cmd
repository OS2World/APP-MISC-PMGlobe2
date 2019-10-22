/* Format a monospace document from .GML files */
docs='globe glcomm'
homedir='globe'

do while docs<>''
  parse var docs doc docs
  call tryformat doc
  end

/* Now fixup degree characters */
say 'Translating GLCOMM...'
file='glcomm.doc'
temp='glcomm.$$$'
do while lines(file)>0
  call lineout temp, translate(linein(file), 'f8'x, '@')
  end
call lineout file
call lineout temp
'@copy' temp file
'@erase' temp

exit

/* ----- Format a document, if necessary ----- */
tryformat: procedure expose homedir
parse arg name .
infile =name'.gml'
outfile=name'.doc'

numeric digits 20
if filedate(outfile)>=filedate(infile) then return
/* need reformat */
tempfile='$temp$.doc'
headfile='$head$.doc'

'@echo off'
'copy' infile '\svp >nul'
'cd \svp'
/* Generate special header line */
'erase' headfile             '2>nul'
call lineout headfile, right('XXX x.xx', 75)
call lineout headfile
say 'SGML parsing' infile'...'
'svp -k -l -r -m1' infile
if rc<>0 then say '*** SVP RC was' rc '***'
say 'MO formatting...'
'mo' infile '-f'tempfile '2> mo.err'
if rc<>0 then say '*** MO RC was' rc '***'
'type mo.err'
'copy' headfile '+' tempfile '\'homedir'\'outfile
'erase' infile
'erase' tempfile
'erase' headfile
'cd \'homedir
return

/* Filedate as all-numbers */
filedate: procedure
  arg file
  parse value stream(file, 'c', 'query datetime') with date time .
  if time='' then return 0
  parse var date mon'-'day'-'year     /* USA form */
  parse var time h':'m':'s
  value=year''mon''day''h''m''s
  /* say 'FD of' file 'is' value */
  return value

