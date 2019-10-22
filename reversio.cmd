/* Get the version number and ensure documents are marked */
in='globe.h'
do 3; line=linein('globe.h'); end; call lineout in
parse var line 'VERSION' '"'vers'"'
if vers='' then do
  say "***** VERSION not found in GLOBE.H, line 3 *****"
  exit 8; end
newline=right("PMGlobe" vers, 73)
temp='$t$.$t$'; '@erase' temp '2>nul'

signal on notready
docs='globe.doc glcomm.doc globe.abs'
count=0
do words(docs)                /* for each doc */
  parse var docs doc docs
  line=linein(doc)
  if words(line)<>2 | length(line)<73 then do
    say "***** First line bad in '"doc"' *****"
    exit 9; end
  call lineout temp, newline
  do while lines(doc)>0
    call lineout temp, linein(doc)
    end
  call lineout doc; call lineout temp
  '@copy' temp doc '>nul'
  if rc<>0 then do
    say "***** Error copying to '"doc"' *****"
    exit 11; end
  '@erase' temp
  count=count+1
  end

say count 'Documents re-versioned as' vers
exit


notready:
  say "***** NOTREADY while processing '"doc"' *****"
  exit 10
