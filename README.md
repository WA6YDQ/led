

##LED ###an easy to use, full-featured multi-platform line editor with a consistant UI
written in ansi c.

I use many different computer systems, and while they all have some sort of line editor
(I don't always use addressable cursor tty's) I got tired of remembering arcane commands
written 50+ years ago. Think DEC's teco. So I wrote LED.

LED has a full-featured set of commands and since it's ansi c it can be easily
ported to all the machines I use (or am likely to use).

To build led:  **cc -o led led.c -Wall -ansi**
Place led in your /usr/local/bin or wherever your executables reside.
There is a man page (led.1). Place this file in your /usr/local/man/man1 directory or
where you keep your local man pages.

LED has enough commands to write meaningful documants/programs/whatever but not
so many that they get in the way.

When LED is started, you will be placed in APPEND mode. Everything you type will
be added to the buffer.

If LED is started with an optional filename then the file (if found) will be
loaded into the buffer and, again, you will be placed in APPEND mode.

To leave APPEND mode press CTRL-B then enter/return on an empty line.
The prompt will change from A> to CMD> and whatever you type in will be interpreted as
a command. To go back to APPEND mode press CTRL-B and the enter/return key on an
empty line.

While in COMMAND mode typing 'help' will display a short list of commands.

**Commands for LED are as follows:**

**new** 
clear the buffer

**load [filename]**
load the contents of a file into the buffer

**save [filename]**
save the buffer to filename

**print [startline] [endline]**
print the buffer to the display/tty 
startline and endline are optional. If used, will display 
only between startline and endline inclusve.
If just the startline is used, print will display starting at line# startline.
While printing, the display will stop at 24 lines and prompt you for more output.
You will see the ---MORE--- line. Either press enter/return to print another
24 lines or press 'q' and the enter/return to stop the listing.

**del [linenumber]**
delete line linenumber

**ins [linenumber]**
start inserting text BEFORE linenumber.
To exit insert mode, press CTRL-B and enter/return at the start of an empty line.

**cut [line number]**
delete the line linenumber saving to cut text in a buffer

**copy [linenumber]**
copy the line linenumber to a buffer

**paste [linenumber]**
paste the cut buffer BEFORE linenumber

**stats**
show various info about the text buffer

**find [string]**
search for 'string' in the text buffer. Shows the line number and line of text.

**sort**
Performs an alphabetic sort of the buffer, replacing the buffer contents with 
the newly sorted contents.

**replace [linenumber] [old string] [new string]**
replace a single whitespace string named 'old string' with 'new string'
'Old string' may be a part of a larger string, but is terminated by either a space
or a newline.

**replaceall [linenumber] [old string] [new string]**
Same as replace, but performs the operation globally starting at linenumber.

**undo**
Undo un-does whatever horrible thing you just did to your buffer.
Example: typing 'new' without saving the contents.
There is only one level of undo.

**quit**
exit the LED program. Does NOT save (or ask to save) the buffer.
If you want your file saved, save it before you type quit.
Undo does not fix quit.

Prompts:
A>  append mode
I> insert mode
CMD> command mode.

See the man page (led.1) for any more information.

The buffer size is limited only by available memory and your operating system.

This code is under the MIT software license as described in the Github
repository where this resides. Basically, do what you want with it. There are no 
guarantees as to it's usefulness and if it breaks it's your problem.

It works for me. I expect it should also work for you.

-- *Kurt*


