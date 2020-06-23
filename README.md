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

<br/>
There is a unix manpage for led (led.1) with full instructions. 
Move it into your manpages directory.
<br/>
Enjoy!<br/>
<br/>
 -- Kurt
 
