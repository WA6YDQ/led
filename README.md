# Led
Simple (but useful) line editor in c

I use many computer systems. Each has their own way of entering text. I'm tired 
of having to remember commands from line editors written over the last 
50+ years and wanted a simple-to-use interface. So I wrote this.

Also I use terminals that don't have cursor control ie. tty's. 

Compile is straight c: <br/>
  cc -o led led.c -Wall
  
Usage is simple: <br/>
led [filename]   filename is optional. If used, led will load 
filename and put you in append mode (note the A> prompt). 
Otherwise led will start in the append mode (noted by the A> prompt).

There are 3 modes: <br/>
  A> append     Append typed text to the end of the end of the buffer. <br/>
  I> insert     Insert text BEFORE the entered line number. Continue entering <br/>
                until ctrl-b <enter> is pressed. <br/>
  CMD>          Command mode <br/>
  
  To switch between modes, press ctrl-b and <enter>. <br/>
  When starting you will be in the append mode. <br/>
  
Help is available by typing help<enter> in command mode. <br/>

----------------------------
CMD>help

Line EDitor Commands: 
pressing ctrl-b  switches modes
new:                Clear Buffer
load [filename]:    Load a file into the buffer
save [filename]:    Save the buffer to a file
print/list:         Show buffer contents
del [line number]:  Delete a single line
ins [line number]:  Insert text BEFORE line number. ctrl-b<enter> stops insert mode.
stats:              Show buffer size
sort:               Sort the buffer
quit:               Exit the program (does NOT automatically save the buffer).
A> Append Cursor, I> Insert Cursor, CMD> Command Cursor
CMD>

--------------------------

Printing will display 22 lines before showing -- MORE  --
To continue, press <enter>
To abort printing and return to command mode press q<enter>

Line numbers are displayed (but not part of the buffer) during print.
The del and ins commands require line numbers. Nothing else does.

Enjoy!

 -- Kurt
 
