

##LED##  an easy to use, full-featured multi-platform line editor with a consistant UI
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
so many that they get in the way. A lot of led was written with led.

When LED is started, you will be placed in APPEND mode. Everything you type will
be added to the buffer.

If LED is started with an optional filename then the file (if found) will be
loaded into the buffer and, again, you will be placed in APPEND mode.

Commands are in the form of a period followed by a single letter. ie .l, .q
and need to be the at the start of an empty line.

**Commands for LED are as follows:**

**.q** 
Exit immediately, do not save the buffer

**.e**
Exit with a prompt for a filename to save to

**.s [filename]**
Save the buffer to filename. If no filename is given you will be prompted 
for one.

**.p [startline] [endline]**
print the buffer to the display/tty 
startline and endline are optional. If used, will display 
only between startline and endline inclusve.
If just the startline is used, print will display starting at line# startline.
While printing, the display will stop at 24 lines and prompt you for more output.
You will see the ---MORE--- line. Either press enter/return to print another
24 lines or press 'q' and the enter/return to stop the listing.

**.d [linenumber]**
delete line linenumber

**.i [linenumber]**
start inserting text BEFORE linenumber.
To exit insert mode, press .q and enter/return at the start of an empty line.

**.x [line number]**
delete the line linenumber saving to cut text in a buffer

**.c [linenumber]**
copy the line linenumber to a buffer

**.p [linenumber]**
paste the cut buffer BEFORE linenumber

**.f [string]**
search for 'string' in the text buffer. Shows the line number and line of text.

**.r [linenumber] [old string] [new string]**
replace a single whitespace bounded string named 'old string' with 'new string'
'Old string' may be a part of a larger string, but is terminated by either a space
or a newline.

**.ra [linenumber] [old string] [new string]**
Same as replace, but performs the operation globally starting at linenumber.

**.undo**
Undo un-does whatever horrible thing you just did to your buffer.
Example: typing 'new' without saving the contents.
There is only one level of undo.

**.os**
Drop to a command prompt (/bin/sh on *nix systems). This is set by a #define 
in the beginning of the source file.

Prompts:
>  append mode
insert> insert mode

See the man page (led.1) for any more information.

The buffer size is limited only by available memory and your operating system.

This code is under the MIT software license as described in the Github
repository where this resides. You are free to share it, use it and modify it
to your needs. There are no guarantees as to it's usefullnes and function.

-- *Kurt*


