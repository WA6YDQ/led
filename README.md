# Led
Simple (but useful) line editor in c <br/>
<br/>
I use many computer systems. Each has their own way of entering text. I'm tired 
of having to remember commands from line editors written over the last 
50+ years and wanted a simple-to-use interface. So I wrote this.
<br/>
Also I use terminals that don't have cursor control ie. tty's. 
<br/>
Compile is straight c: <br/>
  cc -o led led.c -Wall
<br/>  
Usage is simple: <br/>
led [filename]   filename is optional. If used, led will load 
filename and put you in append mode (note the A> prompt). <br/>
Otherwise led will start in the append mode (noted by the A> prompt).
<br/>
There are 3 modes: <br/>
  A> append     Append typed text to the end of the end of the buffer. <br/>
  I> insert     Insert text BEFORE the entered line number. Continue entering <br/>
                until ctrl-b and enter is pressed. <br/>
  CMD>          Command mode <br/>
  
  To switch between modes, press ctrl-b and enter. <br/>
  When starting you will be in the append mode. <br/>
  

Printing will display 22 lines before showing -- MORE  --<br/>
To continue, press enter<br/>
To abort printing and return to command mode press q and enter<br/>
<br/>
Line numbers are displayed (but not part of the buffer) during print.<br/>
The del and ins commands require line numbers. Nothing else does. Print takes <br/>
an optional line number to start at.<br/>
<br/>
There is a unix manpage for led (led.1) with fill instructions. 
Move it into your manpages directory.
<br/>
Enjoy!<br/>
<br/>
 -- Kurt
 
