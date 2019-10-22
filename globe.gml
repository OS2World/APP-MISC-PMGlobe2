<!doctype simdoc>
<simdoc status='PMGlobe'>
<body>
<h1>PMGlobe -- an OS/2 Presentation Manager World Globe
<p>Copyright (c) Mike Cowlishaw, 1993-2019.  All rights reserved.
Parts Copyright (c) IBM, 1993-2009.
All rights reserved.

<h2>Introduction
<p>
PMGlobe is a program which displays the Earth as a globe using OS/2
Presentation Manager.  You can choose to view the globe from any
direction, or select one of a number of 'standard' views.
<p>
In addition to simply displaying a picture of the world, PMGlobe will
also let you light the globe as though by sunlight -- so you can see
at a glance those areas of the globe where the sun has risen, and where
it is night.  Additional options add shading to the globe for a
three-dimensional effect, and let you measure and track distances
between two points on the globe.  Other features are described in detail
below.
<p>
With the command interface included with PMGlobe, you can add markers,
labels, and clocks to the globe and have more control over its actions.
If you wish, you can use REXX (the scripting language included with
OS/2) as a macro language for PMGlobe -- see PMGLOBEX.DOC for a
description of the PMGlobe commands available; all of them can be used
in a REXX macro.


<h2>Installation
<p>
PMGlobe is a 32-bit application, and therefore requires OS/2 Version
2.  Only the file PMGLOBE2.EXE is needed, though you may wish to
experiment with the sample macros (TEST.PMG, SPIN.PMG, and FASTSPIN.PMG)
or your own macros.  If using macros then they should be in the current
working directory for PMGlobe.
<p>
It is recommended that PMGlobe be started from a program object
contained in a folder or on the desktop.  One way of setting this up is:
<list ord>
<li>Copy a 'Program' object template from the OS/2 Templates folder to
your desired target folder.
<li>On the new object, click mouse button 2, select the [->] button on
'Open', then select 'Settings'.  This should open the settings
notebook.
<li>On the 'Program' page enter the path and file name of PMGLOBE2.EXE,
and, for 'Working Directory', the directory in which you have placed
the sample macros.
<li>On the 'Window' page (optional), choose the setting "Minimize window
to desktop" if you would like the globe to appear on the desktop as an
icon and continue to be updated, when minimized.
<li>On the 'General' page, change the title to whatever you wish (e.g.,
PMGlobe), and un-check the 'Template' check box.
<li>Close the settings notebook.  Clicking on the Globe item should then
start PMGlobe.
</list>

<h2>Using the Mouse with PMGlobe
<p>
When PMGlobe is the active window, mouse button 1 is used to select a
geographical location on the globe, and mouse button 2 can be used to
select a new point of view or pop up a menu of choices.  Specifically:
<list>
<li>Move the mouse pointer to any visible point on the globe, so that the
cross-hair cursor appears, and click mouse button 1.  This will cause
the 'PMGlobe Position/Distance Calculator' window to appear (if not
already visible).  You can move the calculator anywhere on the screen,
just like any other window.  Then:
<list ord>
<li>The current position (Latitude and Longitude) of the mouse is
displayed if the mouse is over the globe.

<li>A single click of mouse button 1 records the current point on the
distance calculator.  Up to two points can be recorded: if two are
shown then the distance between them is also displayed, in
kilometres and miles.  (The distance shown is the 'Great Circle
Distance', that is, the shortest distance between the points when
moving over the surface of the globe.)

<li>A double click of mouse button 1 also records the current point
on the distance calculator, and makes it a 'fixed' (or 'base')
point.  This keeps that point visible on the calculator while you
select any number of other points.  You can set a new fixed point by
double-clicking mouse button 1 at another position.

<li>If two points are shown, the 'Track' button will be enabled.
Pressing this will add the distance between the points to the 'Total
Track' distance (which is then shown on the calculator), and also
displays the new track (the line joining the two points) on the
surface of the globe.
</list>
You can clear all selected points, including the fixed point, by
pressing the 'Reset' button on the calculator.  This also resets the
total track distance to zero, and erases any tracks displayed on the
globe.
<p>
The 'Cancel' button hides the calculator.

<li>If the mouse pointer is over the window background (off the globe),
then clicking mouse button 2 will show a popup menu of options in the
usual OS/2 manner.  This duplicates the menu bar choices; you can remove
the menu bar using the 'options' menu.
<p>
To change the view of the globe, move the mouse pointer over a point on
the globe and click mouse button 2.  The globe will then be redrawn with
that point at the center of the view.
<p>
Once you have found your preferred view, view selection by mouse button
2 can be disabled from the 'Views' menu, if you wish, to avoid
accidental change of the view.
</list>

<h2>Menu options
<p>
The PMGlobe menu options can be selected from the Action Bar in the
usual way, and fall into three groups (in addition to the Help panel
index):
<list def>
<lt>'Options'<li>gives access to various miscellaneous settings
<lt>'Views'<li>is used to select what is seen, and from where
<lt>'Lighting'<li>is used to choose the lighting effects and colors.
</list>
Each group (and each submenu in each group) has a summary 'Help' panel
as its first selection.  The other selections are as follows:
<h3> 'Options' -- miscellaneous settings
<list def>

<lt>'Run macro':<li>displays an OS/2 file dialog to let you select a
macro that is to be run.  (Macros are programs, written in REXX and
having a file extension of <q>.pmg</q>, that let you do more complicated
things than are possible using just menu selections--see PMGLOBEX.DOC
for more details.)
<p>
For example, if you installed all the files that came with PMGlobe,
clicking on the file TEST.PMG would run that macro and show various
cities and clocks on the globe.  You might like to use the system editor
to look at the macros supplied, to see how they work.

<lt>'Halt macro':<li>is enabled when a macro is running.  If selected,
any macro that is running is halted.  The Ctrl-Break key combination has
the same effect.

<lt>'Make window square':<li>makes the window fit the globe on all edges,
if possible (it may not be possible on some narrow or low windows).
This is an instantaneous action; the window size and position is not
saved until you select 'Save windows' (see below), or PMGlobe is
closed, or OS/2 is shut down.

<lt>'Make full screen':<li>enlarges the window to fill the screen entirely,
so that the title and action bars and the frame are just off the
screen and so not visible.  You can still use OS/2 key combinations
to move the window, select action bar items, etc.  For example: F10
followed by "O" and then "Q", will pull down the 'Options' menu and
then make the window a quarter of the size of the screen.

This is an instantaneous action; the window size and position is not
saved until you select 'Save windows' (see below), or PMGlobe is
closed or shut down.

<lt>'Make quarter screen':<li>makes the window be centred and fill a quarter
of the screen (useful as a quick recovery from Make Full Screen).
This is an instantaneous action; the window size and position is not
saved until you select 'Save windows' (see below), or PMGlobe is
closed, or OS/2 is shut down.

<lt>'Make desktop':<li>makes PMGlobe become (and remain) the bottom window;
that is, puts it behind all other windows.  Click anywhere visible
to bring PMGlobe to the foreground (after confirmation).  This does
not change the size of the window.  See below for more details.

<lt>'Show position and distance calculator':<li>when selected, this
makes the position and distance calculator appear (it will also appear
automatically if you click on the window with mouse button 1). See above
for information on how to use the calculator.

<lt>'Cross-hair cursor':<li>when selected, this requests that a cross-hair
cursor be used when the mouse pointer is over the globe itself.
If not selected, the standard desktop pointer is used.

<lt>'Draw at low priority':<li>requests that map drawing be carried out at
lower than normal priority, to minimize impact on other applications.
This is the default; if PMGlobe appears to halt drawing when (for
example) a communications program is running in a DOS session, try
de-selecting this option.

<lt>'Use title bar':<li>requests that the globe title bar (with system
menu button and the minimize and maximize buttons) be shown.
The menu of choices, including Close and Minimize options, is always
available by clicking mouse button 2 on the window background (off the
globe), even if the menu bar and title bar are not shown.

<lt>'Use menu bar':<li>requests that the menu bar be shown below the
globe title bar.  This is the default.  The menu of choices is always
available by clicking mouse button 2 on the window background (off the
globe), even if the menu bar is not shown.

<lt>'Set timezone':<li>this pops up a dialog that lets you set the time zone
information.  You only need to do this for a 'sunlight' view, and
even then only need to do it once.  If no other program updates the
current time zone information then you may also have to change it
whenever you change the clock on your computer for daylight saving
(summer or winter) time.  PMGlobe assumes that your computer clock
is set to local time; the timezone dialog describes your local time
in terms of GMT (Greenwich Mean Time), and hence gives PMGlobe the
information that it needs to calculate the sun's position.

<lt>'Set refresh time':<li>this lets you choose how long PMGlobe will wait
after completing a view of the globe before it redraws it.  This
takes effect for the 'Sunlight' view (which needs regular updating
to give a useful picture) or if any clocks may be displayed.  The
refresh takes place shortly after the wall-clock time that is a
multiple of the selected refresh interval (for example, if 'One
Hour' is selected, the refresh should take place on the hour).

<lt>'Save settings':<li>this saves the settings you have selected; they will
then be used automatically when PMGlobe is next started.  See below
for details of which settings are saved; these are also saved
automatically when OS/2 is shut down or PMGlobe is closed.

<lt>'Restore settings':<li>restores settings to those last saved.

<lt>'Reset settings to defaults':<li>sets settings to the defaults used when
PMGlobe was first run.

<lt>'Save windows':<li>this saves the position and size of the main PMGlobe
window, and the position of the distance calculator.  These will
be used automatically when PMGlobe is next started, and are also
saved automatically when OS/2 is shut down or PMGlobe is closed.

<lt>'Restore windows':<li>restores the position and size of the windows to
those last saved.

<lt>'Minimize':<li>minimizes PMGlobe.
If PMGlobe was started from a program object (see above) with the
'minimize to desktop' setting selected, then the globe will continue to
be displayed and updated in its minimized form.

<lt>'Maximize', 'Restore':<li>same as from the system menu.  These are
useful if you have elected not to use a title bar.

<lt>'Close':<li>leaves PMGlobe.  The settings, window positions, etc., are
saved automatically (see below).
</list>

<h3>'Views' -- where and what you see
<list def>
<lt>'Standard views':<li>this gives you a choice of standard views (Europe &
Africa, Americas, India & Asia, Pacific, and the two Poles).  Use
mouse button 2 (see above) to select any point to be placed at the
center of the view.

<lt>'Grid line choices':<li>lets you choose which grid lines (if any) are to
be shown, and the color (one of fifteen) of the grid lines.

<lt>'Snap to equator':<li>this leaves the Longitude of the center of view
unchanged and sets the Latitude to zero (the equator).  Globe
drawing is faster when the Latitude of the center of view is zero
than when globe is tilted.

<lt>'Snap to Greenwich':<li>sets the Longitude of the center of to zero (the
Prime Meridian) while leaving the Latitude unchanged.  This has no
effect on drawing time.

<lt>'Allow mouse button 2':<li>if selected, lets you rotate and tilt the
globe using Mouse Button 2 (see above).  It can be de-selected to
prevent accidental movement of the direction of view.
</list>

<h3>'Lighting' -- how the globe is seen
<list def>
<lt>'Sunlight':<li>asks for "sun lighting" of the globe.  This lets you see
at a glance which parts of the world are in daylight, and which are
in the dark.  This lighting is (of course) time dependent, so
PMGlobe needs to know the time zone you are in, and will ask you for
it if it is not already set.  If Sunlight is selected, the globe
will be redrawn at regular intervals (as selected via 'Set refresh
time', under 'Options').  See below for notes on the accuracy of the
time and sunlight information.

<lt>'Sunlight from space':<li>is similar to 'Sunlight', except that it is
more realistic -- you cannot see details on the dark side of the
earth except where lit by twilight. For best realism, choose a black
background, 3-D, astronomical twilight, and no grid lines (or
perhaps low-key grid lines, such as GREY).  This setting implies
(sets) 'Sunlight'.

<lt>'Sunlight and starlight':<li>shows the 'Sunlight from space' view, with
the land masses on the "dark side" of the earth made visible, too.
(If you cannot see them, turn up the brightness on your display.)

<lt>'3-D':<li>adds shading, to give the globe a "three-dimensional"
appearance.  On a VGA screen there are only two shades of green and
blue available, so this will give a rather grainy and mottled
appearance (especially if a Sunlight option is selected), so
this setting may not give an acceptable display.  On a BGA (8514),
or better screen and adapter, much smoother shading is possible.

<lt>'Twilight settings':<li>lets you select how much of the "dark side" of
the globe is to be shown when sun lighting is in effect.  When
theoretical sunrise/sunset is selected, the globe is lit as though
by a point source at the center of the sun: exactly half of the
globe is lit.  For the meaning of the other settings, see the
detailed notes below.

<lt>'Land color':<li>lets you select one of nine colors for land masses and
islands.  The default is Green, though Yellow and White are good
alternatives.

<lt>'Water color':<li>lets you select one of nine colors for lakes and seas.
The default is Blue; Gray is an alternative.

<lt>'Background color':<li>lets you select one of sixteen background colors.
A black background gives a nice 'deep space background' effect,
though may be a bit too much of a contrast for some tastes; you may
prefer Dark Blue.
</list>

<h2>Date and time limits and accuracy
<p>
The data used for presenting the globe are loaded in compressed form (a
little less than 22 Kilobytes).  The coastline and lake data were
compiled from a number of different sources, with some manual editing to
improve the representation in critical areas such as narrow straits and
isthmuses.  The accuracy of the coastline and other data is not
guaranteed in any way, but is believed to be within 40km (25miles) in
the worst case (near the equator) and significantly better in the
East-West direction in Northern and Southern latitudes.
<p>
When the distance calculator is used, the mouse position can only be
determined to the nearest pel (picture element) on the screen.  The
longitude and latitude is then reported as being at the center of that
pel.  Distance calculations between the two positions thus reported are
then calculated from those coordinates and should be accurate to the
nearest unit (km or mile), or 0.2%, whichever is greater.  The 0.2%
limit is a consequence of assumption used in the calculation that the
earth is spherical (which it is not).
<p>
Time and sun position calculations are only important when one of the
'Sunlight' options is selected.  In this case, PMGlobe needs to know the
current date and time-of-day (taken from your computer's clock -- make
sure it is set correctly).  It also needs to know which time zone you
are in: if not already set it will ask you to set it (you may also need
to change it if your computer clock is changed for daylight saving,
summer, or winter time).
<p>
From the current time and time zone information, PMGlobe can determine
apparent solar time (which is as much as 16 minutes different from the
Civil time used for clocks) and hence the sun's position.  This is then
used to display the globe as though lit by the sun: the light/dark
dividing line thus shows where the sun is rising or setting.
<p>
The various calculations done should give an accuracy of sun position
that leads to a sunset or sunrise indication that is correct to within
twenty seconds of time.  Actual sunset or sunrise times will be a little
different because of atmospheric effects, which vary with the time of
year, the weather, and latitude.  However, PMGlobe does give a useful
indication of sunrise and sunset, and of course lets you see at a glance
which parts of the globe are in night or daylight.
<p>
Celestial events such as equinoxes, and solstices especially, are very
dependent on the earth's movements; equinoxes should be correct within
some tens of minutes, solstices should be correct within about ten
hours.
<p>
Note that results will be incorrect if you set your computer's date to
be earlier than 1 January 1990.  Also, the formulae used to calculate
the sun's position may prove to be inaccurate at some time in the
future, because the earth's movement is not entirely predictable.


<h2>Twilight settings
<p>
The sun is not a point source of light, and the atmosphere scatters its
light, too.  Therefore, sunlight is seen on the "dark side" of the earth
for some time before the sun rises and for some time after it sets.  You
can select to see all, part, or none of this twilight zone on the globe.
<p>
Twilight, when shown, is always shown shaded.  On a VGA (standard)
screen it can appear mottled or granular, because there are only two
shades of each color available to PMGlobe.
<p>
The twilight settings provided only have an effect when 'Sunlight' is
selected, and are:
<list def>
<lt>'Theoretical sunrise/set':<li>this lights the earth as though by a point
source at the center of the sun; the sunrise/set line is shown as
though light ceased when the center of the sun crossed the horizon,
and so no twilight can be seen.

<lt>'Ideal sunrise/set':<li>this is a practical and generally accepted
definition (fifty minutes of angle after the theoretical
sunrise/set) which allows for the diameter of the sun and common
atmospheric effects.  The line seen on the globe joins the points
at which the disc of the sun will have just disappeared or be just
about to appear, and encloses a very narrow band of twilight.

<lt>'Civil twilight':<li>the boundary marks the end of "civil twilight" -- a
convenient point used for legal purposes (for example, when it is
too dark to carry out certain tasks).  Please note that PMGlobe's
rendering of this line must not be used for any critical or legal
decisions; errors and bugs are always possible, and the definition
used by PMGlobe (six degrees after the theoretical sunrise/set) may
not match the legal definition used in your country.

<lt>'Naval twilight':<li>the boundary indicates the points where it is dark
for all practical purposes: the center of the sun is twelve degrees
below the horizon.

<lt>'Astronomical twilight':<li>there is no boundary; light fades to zero,
where it is perfectly dark: no effects from sunlight can be
detected.  At this point, the center of the sun is eighteen degrees
below the horizon.
</list>

<h2>Performance considerations
<p>
PMGlobe is designed as a <q>32-bit</> application, with heavy use of long
(32-bit) integers.  All computation is done using integers, so a math
coprocessor is not required.
<p>
In general, the simpler the image presented the faster it is drawn.
Selecting sunlight, 3-D, and twilight all slow down the drawing (but
'sunlight in space' is usually faster than 'sunlight' alone).
<p>
Non-equatorial (tilted) views take significantly longer to draw than
equatorial views (hence the 'Snap to equator' option).
<p>
As a rough guide, the simplest (flat lighting, equatorial view, no
sunlight) image with a diameter of 400 picture elements should take
about 4-6 seconds to draw on a 25MHz 386 PC or PS/2.  The time taken
is inversely proportional to the speed of the processor and proportional
to the square of the image diameter.  The most complicated drawing (3-D
shading, non-equatorial view, sunlight) might take about four times
longer.

<h2>Saved settings
<p>
The following settings are saved (in the system file "OS2.INI") when
'Save settings' is selected.  They are also automatically saved when
PMGlobe is closed or if OS/2 is shut down while PMGlobe is running:
<block>
Latitude and Longitude of the center point of the view
Grid (Meridians and Parallels) selections and color
Lighting selections (Sunlight, Sunlight from space, Starlight, 3-D)
Twilight selection (degrees)
Land, Water, and Background colors
Refresh (re-draw) interval.
Whether 'Desktop' is selected (globe will be bottom window)
Whether 'Draw at low priority' is selected
Whether mouse button two is active ('Allow mouse button 2')
</block>
<p>
'Restore settings' restores all of these to the last saved values.
'Reset settings to defaults' sets all of these to the PMGlobe defaults.
<p>
The following settings are saved, also in OS2.INI, when 'Save windows'
is selected, OS/2 is shut down, or PMGlobe is closed (unless the PMGlobe
window is minimized or maximized):
<block>
The position and size of the main (globe) window
The position of the 'Position/distance calculator' window.
The position of the 'Command dialog' window.
</block>
<p>
'Restore windows' restores these to the values used when PMGlobe was
last started.
<p>
The following setting is saved in OS2.INI when Mouse button 1 is
double-clicked on the globe:
<block>
The latitude and longitude of the current 'fixed point'.
</block>
<p>
This is cleared when 'Reset' is selected in the Position/distance
calculator.
<p>
By default, all the settings are saved in the OS2.INI file under the
application name "PMGlobe".  You can extend this name (and hence control
different instances of PMGlobe and its settings) by using the NAME
parameter when starting PMGlobe.  This parameter constructs the name
used by adding the word you give (which should be from 1 to 25
characters long, with no blanks) to the stem "PMGlobe.".
<p>
For example, if you run PMGlobe with the command:
<block>
"start pmglobe name Fred"
</block>
then the entries in the OS2.INI file will be stored under the name
"PMGlobe.Fred".  Note that the case of each letter is significant.


<h2>The 'Make Desktop' setting
<p>
The Desktop setting requests that PMGlobe will keep itself at the bottom
of all the windows on the screen. Under OS/2 2.x the desktop icons are
part of the desktop, and so PMGlobe cannot get "underneath" them;
selecting both 'Make Desktop' and 'Make full screen' is therefore not
advised as it would hide the desktop icons (unless you have moved them
off the desktop and into a different folder).
<p>
Once on the desktop, the usual PMGlobe mouse functions are suspended.
Clicking on PMGlobe will ask for confirmation before leaving the Desktop
state.  If leaving the desktop and the globe and background window fills
or is larger than the screen then PMGlobe will revert to the
Presentation Manager default size and position when it leaves the
desktop.
<p>
PMGlobe will remember the Desktop state (and window positions) when
Shutdown by OS/2.  It can also be forced to start up on the Desktop by
specifying the parameter 'DESKTOP' when starting PMGlobe, for example:
"start pmglobe desktop".  To fill the screen, add the parameter
'FULLSCREEN' on startup.
<p>
Do not start two applications (such as certain "wallpaper" programs, or
two instances of PMGlobe) that both attempt to remain on the desktop.


<h2>Time Zone interface details
<p>
Personal computer operating systems have a variety of ways of holding
timezone information (some compilers even build in a default
geographical location!), but to date no standard mechanism has been
defined.
<p>
PMGlobe introduces a new mechanism for holding timezone information.
The current timezone offset, daylight savings offset, and timezone name
is held in the system file "OS2SYS.INI" in a general format that any
application can use and which can easily be accessed by application
programs (for example, by C or REXX programs).
<p>
Specifically, the information is held as three words in the OS2SYS.INI
file, under the name "TimeZone" with key "Active".  This string holds at
least three words separated by exactly one blank, and with no leading
blanks.   The third word will be followed by either a blank (to allow
future extension) or the C 'end of string' null character.
<p>
The three words are:
<block>
base-time-offset-from-GMT  summer-time-offset  zone-abbreviation
</block>
where the two times have the format [+|-][h]h[:[m]m[:[s]s]]] and the
zone-abbreviation is either "???" (indicating unknown) or "xxx", where
xxx are three or four uppercase alphabetics.  For example:
<block>
+0:00 +1:00 BST
</block>
<p>
The total offset from GMT is the sum of the two time offsets.

<p><p>--------
<p>Mike Cowlishaw, IBM UK Laboratories
</simdoc>
