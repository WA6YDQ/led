/* nled.c - version 3 of led
 * k theis 4/2021
 *
 * This is a simple line editor. It is compiled
 * with c99 via: gcc -std=gnu99 -O2 -march=native -Wall -pedantic
 * use .help to show avail commands.
 *
*/ 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#define BUFSIZE 32768
#define LINELENGTH 80

void list(char *);
void save(char *);
void load(char *);
void del(char *);
void ins(char *);
void find(char *);
void help(void);
void replace (char *);
void size(void);
void copy(char *);
void paste(char *);
int getlinestart(int);
void upper(void);
void lower(void);

int position = 0;		// current char position in buffer
char *buffer = NULL;		// text buffer
char line[LINELENGTH+1]; 	// temp storage
char clipboard[LINELENGTH+1];	// cut/paste
_Bool isSaved=1;		// set when saved, cleared when buffer is modified

int main (int argc, char **argv) {

	int linesize = 0;
	buffer = malloc(BUFSIZE*sizeof(char*));
	if (buffer == NULL) {
		fprintf(stderr,"Out of Memory\n");
		exit(1);
	}
	memset(buffer,0,BUFSIZE);

	if (argc > 1) { 		// start by loading a file
		load(argv[1]);
	}

	// show cursor, get line. Save line if not a command
	while (1) {
		printf("append> ");
		memset(line,0,LINELENGTH);
		if (fgets(line,LINELENGTH,stdin) == NULL)
			perror("");

		/* -------- test commands -------- */

		if (strncmp(line,".q",2)==0) {
			printf("quitting w/o saving buffer\n");
			free(buffer);
			exit(0);
		}
		
		if (strncmp(line,".exit",5)==0) {
			if (!isSaved) {
				printf("Buffer modified. Exit anyway (y/n)? ");
				memset(line,0,LINELENGTH);
				if(fgets(line,LINELENGTH,stdin)==NULL)
					perror("");
				if (line[0] != 'y') continue;
			}
			free(buffer);
			exit(0);
		}

		if (strncmp(line,".list",5)==0) {
			list(line);
			continue;
		}
		
		if (strncmp(line,".save",5)==0) {
			char *buf = strchr(line,' ');
			if (buf == NULL) {
		            printf("usage: .save [filename]\n");
			    continue;
			}
			buf += 1;
			save(buf);
			continue;
		}
		
		if (strncmp(line,".load",5)==0) {
			char *buf = strchr(line,' ');
			if (buf == NULL) {
			    printf("usage: .load [filename\n");
			    continue;
			}
			buf += 1;
			load(buf);
			continue;
		}
		
		if (strncmp(line,".new",4)==0) {
			memset(buffer,0,BUFSIZE);
			position = 0;
			printf("Buffer Cleared\n");
			continue;
		}
	
		if (strncmp(line,".stat",5)==0) {
			size();
			continue;
		}

		if (strncmp(line,".upper",6)==0) {
			upper();
			continue;
		}

		if (strncmp(line,".lower",6)==0) {
			lower();
			continue;
		}

		if (strncmp(line,".i",2)==0) {
			ins(line);
			continue;
		}

		if (strncmp(line,".d",2)==0) {
			del(line);
			continue;
		}

		if (strncmp(line,".f",2)==0) {
			find(line);
			continue;
		}
	
		if (strncmp(line,".r",2)==0) {
			replace(line);
			continue;
		}

		if (strncmp(line,".c",2)==0) {
			copy(line);
			continue;
		}

		if (strncmp(line,".p",2)==0) {
			paste(line);
			continue;
		}
	

		if (strncmp(line,".help",5)==0) {
			help();
			continue;
		}


		/* -------- end of commands -------- */

		// enter line into buffer
		linesize = strlen(line);
		for (int n=0; n<linesize; n++) buffer[position+n]=line[n];
		position += linesize;
		isSaved=0;
		continue;
	}

	return 0;
}

// find a string, show line it's on
void find(char *buf) {
	char testline[LINELENGTH]; int n=0, cnt=0, linenum=1;
	char *tmp=strchr(buf,' ');
	if (tmp == NULL) {
		fprintf(stderr,"nothing to find\n");
		return;
	}
	tmp++; // point to start of search string
	char findstr[LINELENGTH]; sscanf(tmp,"%s",findstr);
	n=0;
findloop:
	cnt=0; memset(testline,0,LINELENGTH);
	while (n < position) {
		testline[cnt] = buffer[n];
		if (testline[cnt]=='\n' || testline[cnt]=='\0') break;
		cnt++; n++;
	}
	testline[cnt] = '\0';
	// search the line
	tmp = strstr(testline,findstr);
	if (tmp != NULL) 	// line contains the string
		printf("%04d: %s\n",linenum,testline);
	linenum++; n++;
	if (n < position) goto findloop;
	return;
}

// replace single (.r) or all (.ra) strings in the buffer
void replace(char *buf) {
	char cmd[5], oldstring[LINELENGTH], newstring[LINELENGTH];
	char *tmp = strchr(buf,' ');
	char *lineptr;
	if (tmp == NULL) {
		fprintf(stderr,"nothing to find\n");
		return;
	}
	sscanf(buf,"%s %s %s",cmd,oldstring,newstring);
replaceloop:
	lineptr = strstr(buffer,oldstring);
	if (lineptr == NULL) return;
	if (lineptr != NULL) {
		char *dest = lineptr;
		char *src = dest+strlen(oldstring); 
		//int bufsize = position-(lineptr-&buffer[0]);
		int bufsize = &buffer[position]-dest;
		memmove(dest,src,bufsize);		// delete oldstring
		position -= strlen(oldstring);	// reset position
		src = dest+strlen(newstring);
		memmove(src,dest,position-strlen(newstring));		// make room for new string
		for (int n=0; n<strlen(newstring); n++) buffer[lineptr-&buffer[0]+n] = newstring[n];
		position += strlen(newstring);
		if (strncmp(cmd,".ra",3)==0) goto replaceloop;
		isSaved=0;
		return;
	}
}

void del(char *line) {	// delete a line
	char *buf = strchr(line,' ');
	if (buf == NULL) {
		printf("usage: .d [line number]\n");
		return;
	}
	int linenum = atoi(buf);
	int n=getlinestart(linenum);	// n holds start of line in buffer
	char *startpos = &buffer[n];
	if (startpos==NULL) {
		printf("bad line\n");
		return;
	}
	char *endpos = strchr(startpos,'\n');
	if (endpos==NULL) {
		printf("bad line\n");
		return;
	}
	endpos+=1;		// skip the ending \n
	// delete the line
	memmove(startpos,endpos,&buffer[position]-endpos);
	position -= (endpos - startpos);
	printf("line %d deleted\n",linenum);
	isSaved=0;
	return;
}

void ins(char *gline) {	// insert a line, repeat until .q entered
	char *buf = strchr(gline,' ');
	if (buf == NULL) {
		printf("usage: .i [line number]\n");
		return;
	}
	int linenum = atoi(buf);
	int n=getlinestart(linenum);
insloop:
	// insert begins at n
	printf("ins> ");
	if (fgets(line,LINELENGTH,stdin)==NULL)
		perror("");
	if (strncmp(line,".q",2)==0) return;
	char *src=&buffer[n];
	char *dest=src+strlen(line);
	memmove(dest,src,position-n);	// shift up
	position+=strlen(line);
	memcpy(&buffer[n],line,strlen(line));	// insert line
	n+=strlen(line);
	isSaved=0;
	goto insloop;
}




void copy(char *line) {		// copy a line to buffer
	int n;
	char *buf = strchr(line,' ');
	if (buf == NULL) {
		printf("usage: .c [line number]\n");
		return;
	}
	memset(clipboard,0,LINELENGTH);	// clear prior
	buf += 1;		// point to line number
	int linenum=atoi(buf);
	n = getlinestart(linenum);  // n points to start of line
	int cnt=0;
	while (n<position) {
		clipboard[cnt] = buffer[n];
		if (buffer[n]=='\n') break;
		cnt++; n++;
	}
	// clipboard holds wanted line
	printf("line %d copied\n",linenum);
	return;
}

void paste(char *line) {		// paste the clipboard to the buffer
	int n;
	char *buf = strchr(line,' ');
	if (buf == NULL) {
		printf("usage: .p [line number]\n");
		return;
	}
	int linenum = atoi(buf);
	n = getlinestart(linenum);	// n points to line start
	// move buffer up
	memmove(&buffer[n+strlen(clipboard)],&buffer[n],position-n);
	position += strlen(clipboard);
	// insert clipboard
	memcpy(&buffer[n],clipboard,strlen(clipboard));
	printf("buffer pasted before line %d\n",linenum);
	isSaved=0;
	return;
}


int getlinestart(int linenum){
	int n, ln=1;
	for (n=0; n<position; n++) {
        if (ln == linenum) return n;
        if (buffer[n] == '\n')
            ln++;
    }
    if (ln != linenum) {
        printf("line %d not found\n",linenum);
        return -1;
    }
    // n points to start of line
	return -1;
}

// display buffer contents
void list(char options[20]) {
	int linenum = 1;
	char *buf = strchr(options,' ');
	if (buf == NULL) {		// show all lines
		printf("%04d: ",linenum);
		for (int n=0; n<position; n++) {
			printf("%c",buffer[n]);
			if (buffer[n] == '\n') {
				linenum++;
				printf("%04d: ",linenum);
			}
		}
		printf("\n");
		return;
	}
	// show selected lines
	char cstart[10], cend[10];
	memset(cstart,0,10); memset(cend,0,10);
	int istart=0, iend=0; int n=0;
	buf += 1;	// point past ' '
	while (*buf != '-') 
		cstart[n++] = *(buf++);
	n=0; buf++;		// point past -
	while (*buf != '\n' && *buf != '\0')
		cend[n++] = *(buf++);
	istart = atoi(cstart);
	iend = atoi(cend);
	// set proper values
	if (istart < 1) istart=1; 
	if (iend == 0) iend=9999; 
	if (iend < istart) iend=istart;
	// show lines from istart thru iend
	int linecnt=1; int PRINTFLAG=0;
	if ((istart <= linecnt) && (linecnt <= iend)) {
		PRINTFLAG=1;
		printf("%04d: ",linecnt);
	}
	for (n=0; n<position; n++) {
		if (buffer[n] == '\n') { 
            linecnt++;
			if ((istart <= linecnt) && (linecnt <= iend)) 
				PRINTFLAG=1;
			else
				PRINTFLAG=0;
			if (PRINTFLAG && buffer[n]=='\n')
				printf("\n%04d: ",linecnt);    // show line number
			continue;
		}
		if (PRINTFLAG) printf("%c",buffer[n]);
	}
	printf("\n");
	return;
}

// save the buffer to a file
void save(char line[LINELENGTH+1]) {
	char filename[LINELENGTH+1];
	if (strlen(line) < 2) {
	    printf("usage: save [filename]\n");
	    return;
	}
	strcpy(filename,line);
	if (filename[strlen(filename)-1] == '\n') filename[strlen(filename)-1]='\0';
	printf("filename [%s]\n",filename);
	FILE *fd = fopen(filename,"w");
	if (fd == NULL) {
		fprintf(stderr,"failed to create file %s\n",filename);
		return;
	}
	for (int n=0; n<position; n++)
		fprintf(fd,"%c",buffer[n]);
	fclose(fd);
	printf("Buffer saved to %s\n",filename);
	isSaved = 1;
	return;
}

// load a text file into the buffer
void load(char line[LINELENGTH+1]) {
	char filename[LINELENGTH+1]; char ch;
        if (strlen(line) < 2) {
	    printf("usage: load [filename]\n");
	    return;
        }
	strcpy(filename,line);
        if (filename[strlen(filename)-1] == '\n') filename[strlen(filename)-1]='\0';
	FILE *fd = fopen(filename,"r");
	if (fd == NULL) {
		fprintf(stderr,"failed to open file %s\n",filename);
		return;
	}
	// clear buffer before loading
	position = 0;
	memset(buffer,0,BUFSIZE);
	while (1) {
		ch = fgetc(fd);
		if (feof(fd)) break;
		buffer[position++] = ch;
	}
	fclose(fd);
	printf("read in %d characters\n",position);
	isSaved=0;
	return;
}

void upper(void) {	// convert buffer EXCEPT between "" to UPPER CASE
	char ch; int QUOTEFLAG=0;
	for (int n=0; n<position; n++) {
		ch = buffer[n];
		if (ch=='"') QUOTEFLAG = abs(QUOTEFLAG-1);
		if (!QUOTEFLAG) 
			buffer[n] = toupper(ch);
	}
	printf("Conversion done.\n");
	isSaved=0;
	return;
}

void lower(void) {	// convert buffer EXCEPT quoted to lower case
	char ch; int QUOTEFLAG=0;
    for (int n=0; n<position; n++) {
        ch = buffer[n];
        if (ch=='"') QUOTEFLAG = abs(QUOTEFLAG-1);
        if (!QUOTEFLAG)
            buffer[n] = tolower(ch);
    }
    printf("Conversion done.\n");
    isSaved=0;
    return;
}



// show memory (used/total)
void size(void) {
	printf("available memory %d of %d characters\n",BUFSIZE-position,BUFSIZE);
	printf("buffer is %d characters\n",position);
	printf("clipboard: %s",clipboard);
	return;
}


void help(void) {
	printf("Max Line Length is %d characters\n",LINELENGTH);
	printf("exit led	.q, .exit\n");
	printf("show this	.help\n");
	printf("file load	.load [filename]\n");
	printf("file save	.save [filename]\n");
	printf("clr buffer	.new\n");
	printf("list buffer	.list [start-end]\n");
	printf("stats		.stat\n");
	printf("insert		.i [line number]\n");
	printf("delete		.d [line number]\n");
	printf("find		.f [search string]\n");
	printf("replace		.r [old string] [new string]\n");
	printf("replace all	.ra [old string] [new string]\n");
	printf("copy		.c [line number]\n");
	printf("paste		.p [line number]\n");
	printf("cnvrt UPPER	.upper\n");
	printf("cnvrt lower	.lower\n");
	return;
}

