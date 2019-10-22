<!doctype simdoc>

<simdoc status='PMGlobe'>
<body>
<h1>PMGlobe -- an OS/2 Globe -- Commands and Macros
<p>Copyright (c) Mike Cowlishaw, 1993-2019.  All rights reserved.
Parts Copyright (c) IBM, 1993-2009.
All rights reserved.

<h2>PMGlobe commands, and how they are used
<p>
This document lists the PMGlobe commands that are currently implemented.
These may be invoked from a PMGlobe macro (a program written in REXX),
or directly from the command dialog (which can be displayed by pressing
Escape while PMGlobe has the focus).  The command dialog allows editing
of the command, using Insert, Delete, Backspace, and Cursor Left/Right;
it also has a simple 'retrieve' function, controlled by the Cursor
Up/Down keys.
<p>
To use commands from a macro, edit (perhaps with the OS/2 System Editor)
a plain ASCII file such as the TEST.PMG supplied with PMGlobe and
reproduced in simplified form here:
<block>
/* Sample (test) macro */
/* Get the version information and set a new title */
'extract version versionb'
'set title PMGlobe -' version '('versionb'-bit)'
</block>
<block>
/* Mark and label some chosen cities */
"mark  51.5     0   label London"
"mark  37.8  -122.4 label San Francisco"
"mark -33.7   151.3 label Sydney"
</block>
<p>
This is a very simple REXX program that consists almost entirely of
commands (see SPIN.PMG for a more typical macro).  After saving the
file, it can by run by simply bringing up the PMGlobe command dialog,
entering the name of the macro, and pressing Enter.
<p>
The 'Macros' checkbox on the command dialog means that macros will be
searched for before commands.  If it isn't checked, the command "macro
xxxx", where "xxxx" is the name of the macro, will invoke it.
<p>
A macro can be called when PMGlobe is first started by specifying "MACRO
filename" as a parameter to the PMGlobe call, for example:
<block>
"start pmglobe macro mymacro"
</block>
<p>
Macro names as a parameter or on a command may include an explicit drive
and path, and if no extension is given then a default file extension of
".pmg" is added.
<p>
<h>Note:</> all PMGlobe commands should currently be considered
experimental; the syntax and semantics of any command could change in
future versions of PMGlobe (if any).

<h2>General notes
<list ord>
<li>Angles may be specified in decimal form, such as -30 or +45.5, where
for latitude and longitude negative implies south of the equator or west
of Greenwich respectively.  The more usual format is also allowed (for
example: 52&degree;30'10E) -- details below.  In this
document, the character <q>&degree;</> is the degree character, encoded
on IBM PC's as 248 (decimal) or 'F8'x (hexadecimal).
<p>
Angles returned by QUERY or EXTRACT will always be in decimal form.

<li>Times are specified and returned in seconds (with an optional sign
for relative times).

<li>Keywords are shown below in capitals; they may be entered in mixed
or lower case.

<li>Colors may also be in any case, and (with valid alternative
spellings in parentheses) must be one of: White Gray (Grey) Black Red
Green Blue Pink (Magenta) Cyan (Turquoise) Yellow DarkRed DarkGreen
DarkBlue Purple (DarkPink, DarkMagenta) DarkCyan (DarkTurquoise) Brown.
PaleGray (PaleGrey) is also allowed, for the Background color only.  The
dark colors (from DarkRed onwards) cannot be used for Land or Water
coloring.  A QUERY or EXTRACT will return the first shown name for each
color, in uppercase.

<li>The keyword "CENTER" may also be spelled as "CENTRE", and "COLOR" as
(or in) a keyword may be spelled "COLOUR".

<li>It is recommended that long-running macros (especially those that
use the WAIT command) use the SET TITLE command to change the window
title to indicate that the macro is running.  The title should be
restored to its previous value if halted (see FASTSPIN.PMG, for
example).  Good macros will also allow for asynchronous alteration of
viewpoint, etc., due to use of the pull-down menus.

<li>Graphics (drawn with the GLINE command) are shown above any grid
lines, and underneath any markers, clocks, and marks.  Any tracks (drawn
using the Distance Calculator, or using the TRACK option on graphics
commands) are drawn immediately above any graphics.
</list>


<h2>Commands in alphabetical order

<list def>
<lt>CLOSE
<li><p>
Close PMGlobe (as though Close had been selected in the System menu).

<lt>EXTRACT items
<li><p>
Used for obtaining the values of various PMGlobe settings for use with a
REXX macro.
<p>
'Items' is a list of one or more keywords, as given in the QUERY and SET
commands below. The values of the specified items are placed into REXX
variables of the same name.  (See above for a simple example.)

<lt>GERASE [DRAW|TRACK]
<li><p>
Erases either all DRAW graphics or all TRACK graphics.  DRAW is the
default; TRACK is normally only used by PMGlobe, for drawing tracks.

<lt>GLINE lat lon [DRAW|TRACK]
<li><p>
Draws either a DRAW or a TRACK line from the current point to the point
specified, and then makes the specified point the current point. DRAW is
the default, and draws the line in the color set by DRAWCOLOR. TRACK is
normally only used by PMGlobe, for drawing tracks, and draws the line in
the color set by TRACKCOLOR.
<p>
If no current point has been set (by GMOVE) before this command is
used, latitude 0 and longitude 0 are used.

<lt>GMOVE lat lon [DRAW|TRACK]
<li><p>
Sets the current point for either DRAW or TRACK graphics.  DRAW is the
  default, and sets the current point for DRAW graphics.  TRACK is
  normally only used by PMGlobe, and sets the current point for TRACK
  graphics.

<lt>FONT name [SIZE height] [COLOR color] [FACE facename]
<li><p>
Defines a font, with nickname 'name'.  SIZE sets the font size (total
height in points), and FACE specifies an OS/2 outline font (e.g.,
"Helvetica", "Times New Roman", etc.).  If no FACE is specified then the
default system proportional font and size is used, and SIZE is ignored.
The default size for all other fonts is 12 points.  See also the
additional notes below.
<p>
The color for a font is specified using the COLOR keyword and one of the
colors listed above.  The default color is Yellow.
<p>
Fonts may not be redefined.  The default system proportional font is
predefined with the nickname 'Default'.

<lt>MACRO file [argumentstring]
<li><p>
Invoke the specified macro, a REXX program.  The macro should be in the
working directory for the application, or have its path specified.  If
no filename extension is given, one of ".pmg" is added.  The remainder
of the command is passed as the argument to the macro (accessible in
REXX by the PARSE ARG instruction or ARG(1) built-in function).
<p>
Note:  At the highest level, macros run as a single separate thread.
Macros may invoke each other, using the MACRO command, up to a depth of
ten (other than this, only one macro can be running at a time).  All
macros that are running will be halted by Ctrl-Break, or by selecting
'Halt macro' from the menus or the command dialog.

<lt>MARK lat lon DELETE
<li><p>or
<lt>MARK lat lon [MARKER m] [X x-offset] [Y y-offset] [CLOCK t-offset] [LABEL text]
<li><p>
Delete or add a mark from or to the displayed globe.  Latitude and
Longitude (lat lon) are always required.  If DELETE is used, the mark
must have been added before, and is deleted.  If more than one mark have
the same latitude and longitude then the most recently added is deleted.
<p>
If DELETE is not used, then a new mark is added.  The point will be
marked according to the setting of of SET MARKER (default "+") at the
time of the MARK command, unless overridden by the option MARKER.  The
value, m, of the marker can be any of ".", "+", "x", "X", or "OFF" for
various shapes of marker.  "!" may be used for a tiny (single-pel)
marker.
<p>
The color used for a mark is the current drawing color, as set by 'SET
DRAWCOLOR color', at the time of the MARK command.  If none was set
then White is used.
<p>
A mark may have a text associated with it, described by the CLOCK and
LABEL options.  This text is displayed with the start of the baseline at
an offset from the mark as determined by the X and Y options; the
offsets are given in pels, up to 1000, and may be positive or negative,
or CENTER.  The default X and Y offsets (x-offset and y-offset) are +3.
If the text would extend beyond the bounds of the square enclosing the
globe, then it is shifted in order to stay within bounds (if possible).
<p>
If CLOCK is specified, t-offset describes the current GMT offset of the
mark in seconds, and the text begins with the time calculated using
this.  This time is shown in 24-hour or civil format, depending on the
setting of CLOCKCIVIL, and is refreshed according to the REFRESH
setting.  Use the CLOCKDAY setting for an additional
tomorrow/yesterday indication (+/-).
<p>
If LABEL is specified, it must be the last option on the command: the
remainder of the command, after the keyword LABEL and one blank, forms
the label part of the text, which follows any clock and may be up to
100 characters.
<p>
The font used for the text is the current font, as set by 'SET FONT
nickname', at the time of the MARK command.  If none is set then the
default system font is used.
<p>
Markers, clocks, and labels can be included or excluded as a group by
using the MARKERS, CLOCKS, and LABELS settings.  None of these are
shown if the window size is less than or equal to 64 pels in either
dimension (for example, if the globe is minimized to the desktop).

<lt>MESSAGE text
<li><p>
Display text in a message box.  The message box can be moved, and does
not inhibit interaction with the globe or distance calculator.  The
MESSAGE command waits until the "OK" pushbutton on the message box is
selected or the message box is closed.  Commands are currently limited
to 200 characters, so the longest message possible is 192 characters.

<lt>REDRAW [ALL]
<li><p>
Initiates a redrawing of the Globe.  Normally a REDRAW is automatic at
the end of a macro or (if the box on the dialog is checked) after a
command is entered from the command dialog.  The REDRAW command is
provided for long-running macros.  The command does not wait until
redrawing is complete.
<p>
The ALL option requests that all features of the globe are redrawn,
including any grid lines and marks (normally only those features that
have changed since the last draw are drawn).

<lt>QUERY items
<li><p>
Queries the state of various PMGlobe settings.  'items' is a list of one
or more keywords.  Currently the result is displayed using PMprintf; to
see the results the PMprintf console must be active. (PMprintf is an IBM
developer's tool; if you do not have access to this, use the EXTRACT
command in a macro, followed by the MESSAGE command to display values,
etc.)
<p>
Any item that can be set by the SET command (see below) may also be
queried.  The value of any item that may be set or queried may also be
obtained by a macro, using the EXTRACT command.  PMprintf is not needed
in order to use these values in a macro.
<p>
Valid items are:
<list def>
<lt>ACTIVE    <li>ON if the globe window is the active window, else OFF
<lt>CLOCKMARKS<li>number of clocks set using the MARK command
<lt>DAYOFFSET <li>time offset due to daylight savings (in seconds), or "UNKNOWN"
<lt>DIAMETER  <li>globe diameter in pels (0 if unknown)
<lt>FONTS     <li>number of fonts defined using the FONT command
<lt>GMTOFFSET <li>base time offset from GMT (in seconds), or "UNKNOWN"
<lt>MARKS     <li>number of marks set using the MARK command
<lt>SETNAME   <li>'application name' for .INI settings (up to 25 characters)
<lt>SUNLAT    <li>latitude at which sun is overhead (x.xxx), or "UNKNOWN"
<lt>SUNLON    <li>longitude at which sun is overhead (x.xxx), or "UNKNOWN"
(multiply by -4 to get time in minutes from solar noon)
<lt>VERSION   <li>PMGlobe version number
<lt>VERSIONB  <li>Version bits: always 32 for PMGLOBE2.EXE
<lt>ZONEABBREV<li>timezone abbreviation (3 or 4 characters, or "???" if unknown)
</list>

<lt>SET item newvalue
<li><p>
Changes the state of various PMGlobe settings.  Item is one of the
listed keywords.  The value of any item that can be SET can also be
queried using the QUERY command, or obtained by a macro using the
EXTRACT command (see above).
<p>
Flags (items that are either ON or OFF) can also be set to INVERT, which
changes the state of the flag from ON to OFF or vice versa.
<p>
Valid items are:
<list def>
<lt>BACKCOLOR <li>set the color of the background (names as given above)
<lt>CLOCKCIVIL<li>use 12-hour clock format for any clocks (ON/OFF)
<lt>CLOCKDAY  <li>add flag (+ for tomorrow, - for yesterday) to clocks (ON/OFF)
<lt>CLOCKS    <li>display the clocks on marks (ON/OFF)
<lt>CROSSHAIR <li>use cross-hair cursor when over globe (ON/OFF)
<lt>DESKTOP   <li>make PMGlobe the bottommost window (ON/OFF)
<lt>DRAWCOLOR <li>set the color for drawing marks and graphics (names as above)
<lt>ERRORBOX  <li>display command errors in a message box (ON/OFF)
<lt>FONT      <li>sets the current font, by nickname
<lt>GRIDCOLOR <li>set the color of grid lines (names as above)
<lt>GRIDLAT   <li>grid latitude line frequency (0/10/15/30/90)
<lt>GRIDLON   <li>grid longitude line frequency (0/10/15/30)
<lt>GRIDPOLAR <li>show grid polar circles (ON/OFF)
<lt>GRIDTROPIC<li>show grid tropics (ON/OFF)
<lt>IDLEDRAW  <li>draw map at low (IDLE+1) priority (ON/OFF)
<lt>LABELS    <li>display the labels on marks (ON/OFF)
<lt>LANDCOLOR <li>set the color used for land at its brightest (names as above)
<lt>MARGIN    <li>percentage border to allow around globe (0->49.999%)
<lt>MARKER    <li>current default marker (".", "+", "x", "X", or "OFF")
<lt>MARKERS   <li>display the markers on marks (ON/OFF)
<lt>REFRESH   <li>current refresh interval (in seconds, max 24 hours)
<lt>SHADING   <li>'3-D' shading (ON/OFF)
<lt>SHOWDRAW  <li>show (update) picture of globe as it is calculated (ON/OFF)
<lt>STARLIGHT <li>'star' lighting (ON/OFF) -- implies SUNLIGHT ON and SPACE ON
<lt>SUNLIGHT  <li>'sun' lighting (ON/OFF)
<lt>SPACE     <li>'earth from space' lighting (ON/OFF) -- implies SUNLIGHT ON
<lt>TITLE     <li>window and task list title (max 50 characters)
<lt>TRACKCOLOR<li>set the color for drawing tracks (names as above)
<lt>TWILIGHT  <li>degrees of twilight shown (max 18 degrees)
<lt>USEROFFSET<li>time to add to real time (seconds, max +/- one year)
<lt>VIEWLAT   <li>latitude of center of view  (x.xxx)
<lt>VIEWLON   <li>longitude of center of view (x.xxx)
<lt>WATERCOLOR<li>set the color used for lakes and seas (names as above)
</list>
<p>
The following settings are for development use only; the effect of
each is not defined:
<list def>
<lt>DIAG      <li>send diagnostic messages to PMprintf (ON/OFF)
<lt>DIAGMSG   <li>display PM client messages to PMprintf (ON/OFF)
<lt>TEST      <li>controls certain test functions (ON/OFF)
</list>

<lt>WAIT [REDRAW] [time] [+time]
<li><p>
Wait until a redraw is completed, or until a particular time-of-day, or
for a certain length of time from now.  "time" is in hours, with
optional minutes and seconds indicated by colons (for example, +0:20
would mean "wait for twenty minutes").  If a specific time of day is
given and is earlier than the current time of day, then the next day is
assumed.  Multiple times (absolute, relative, or mixed) may be
specified, and WAIT will return when the earliest is reached.  The
maximum wait time is 24 hours (+24).
<p>
If REDRAW is specified, then a REDRAW is started automatically, and WAIT
will return when the redraw is complete or any time condition specified
is met, whichever happens earlier.
<p>
Any WAIT is also ended by Ctrl-Break, or by selecting 'Halt macro'
from the menus or the command dialog.

</list>

<h2>Changing fonts
<p>
The fonts used for any clocks and labels on the MARK command can be
changed by first defining a new font (with the FONT command) and then
making the new font the current font (with the SET FONT command) before
issuing the MARK command.
<p>
The FONT command uses OS/2's facilities for choosing the 'nearest
outline font'; if no good match is found (for example, no font with the
specified face name exists) then the default system bitmap font is used.
<p>
The face name on a font definition should match the OS/2 font name for
an outline font exactly (for example "Helvetica", not "helvetica").  It
can be difficult to find a list of the fonts available; one way you may
be able to find one (if you have a printer installed) is to look at the
font installation panel for the printer driver you are using.  (Click
mouse button 2 on the printer icon, then open Settings. Select the
Printer Driver page, then click button 2 on the driver icon and select
Settings again, then Fonts.)  The following should always be available:
'Courier', 'Helvetica', 'Times New Roman', and 'Symbol Set'.  The first
three of these may be followed by 'Bold', 'Italic', or 'Bold Italic'.
<p>
Example:
<block>
/* ORIGIN.PMG -- Show the origin in a Large font */
'font classic  size 18  color red  face Times New Roman Italic'
'set font classic'
'mark 0 0 y centre label The Origin'
</block>


<h2>Formats allowed for specifying degrees
<p>
A degree value may be specified in in degree-minutes-seconds format, or
in degree.decimal format.
<p>
If the degree value is specifying a latitude or longitude, then it
may be followed by one of the characters N, S, E, W (in either case),
as appropriate.  The default is N (North) for latitude and E (East) for
longitude.  A leading minus sign is allowed in either format, and negates
the direction implied by any N, E, S, or W.  A leading plus sign is also
allowed; it has no effect.
<p>
No blanks are allowed: the seconds, or seconds and minutes, may
be omitted, as may be insignificant leading zeros.  The syntax is
therefore one of:
<block>
[+|-][d]d[o[m]m['[s]s[&quotedbl;]]][x]
[+|-][d]d[.[[d]d]d][x]
</block>
<p>
where the degree indicator in the first format may be either the letter
O (either case) or the true degree character (&degree;, ASCII 248 on IBM PCs
and PS/2s).
<p>
Examples (all specifying a negative angle of 12 degrees 30 minutes):
<block>
12&degree;30'S       -12&degree;30         12&degree;30W        -12.5
12&degree;30'0&quotedbl;S     -12&degree;30'00N    -12.500E        12o30s
</block>

<p>--------
<p>Mike Cowlishaw, IBM UK Laboratories
</simdoc>
