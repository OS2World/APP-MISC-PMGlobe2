/* Build 16- or 32- bit EXE */
/* Call with argument '16' or '32' to force a specific MAKE. */
/* If any other (or no) argument is given, will build for    */
/* current operating system level.                           */

name16='globe'           /* .mak make file name(s) */
name32='globe'

arg bits .
if bits<>16 & bits<>32 then do
  do queued(); pull; end
  '@VER | RXQUEUE'
  pull; pull ver
  parse var ver ' IS ' num .
  if num>=2 then bits=32; else bits=16
  end

call time 'R'
if bits=32 then do
  say '32-bit build...'
  '@erase *.e32 2> nul'
  '@set include=F:\TOOLKT21\CPLUS\OS2H;F:\TOOLKT21\ASM\OS2INC;F:\IBMCPP\INCLUDE'
  '@set lib=F:\TOOLKT21\OS2LIB;F:\IBMCPP\LIB'
  '@nmake /f' name32'.mak /c'
  end
 else do
  say '16-bit build...'
  '@erase *.e16 2> nul'
  '@set include=F:\IBMC2\INCLUDE;F:\TOOLKT13\C\INCLUDE'
  '@set lib=F:\IBMC2\LIB;C:\OS2;F:\TOOLKT13\LIB'
  '@make /f' name16'.mak /c "B16=True"'
  end
say 'Build time:' format(time('E')/60,,1) 'minutes'

exit rc
