/* led - line editor for MPE
 *
 * if using valgrind, 
 * compile with cc -o led led.c -g -O0 -Wall
 *
 * else compile with cc -o led led.c -Wall
 *
 * k theis 6/16/2020
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAXLINE
#define MAXLINE 1000      // max line length
#endif

#ifndef LINENO_MAX 
#define LINENO_MAX 99999    // max line number 
#endif

#ifndef LINENO_MIN
#define LINENO_MIN 1        // lowest line number
#endif

/* global variables */
char *buffer, *tbuffer;
int curpos=0, mode=0;
char linein[MAXLINE+1] = {};
int localpos = 0;
char ch[MAXLINE] = {};            // keyboard input 
int RFLAG=0;                // test for tbuffer use at exit

/* define routines */
void print(char *);         // print
void showmode(int);
void clearBuffer(void);     // new
void filesave(char *);      // save
void fileload(char *);      // load
void stats(void);           // stats
void usage(void);           // help
void delete(char *);        // delete a single line
void insert(char *);        // insert a single line
void find(char *);          // find a word
void sort(char *);          // sort the buffer



/* start of main */
int main(int argc, char **argv) {

    /* set up memory for storage */
    buffer = (char*) malloc((MAXLINE * 2) * sizeof(char*));
    if (buffer == NULL) {
        printf("memory error\n");
        exit(1);
    }

    curpos = 0; // curpos is current position in the buffer 

    /* test command line - if argc==2 the treat option as filename to load */
    if (argc == 2) {
        char loadcmd[MAXLINE];
        strcpy(loadcmd,"load ");
        strcat(loadcmd,argv[1]);
        fileload(loadcmd);
    }

    if (argc > 2) {     // show usage and exit 
        printf("Usage: led [filename] \n");
        printf("where filename is a file to load at start\n");
        return 0;
    }

    mode = 1;       // mode=1:input mode   0:edit mode

inputMode:
    showmode(mode);

inputLoop:    /* entry loop */
    localpos = 0;
    printf("A>");       // show append mode cursor
    while ( (linein[localpos++] = fgetc(stdin)) != '\n');

    if (linein[0] == 0x02) {
        mode = 0;  // ^B changes modes
        goto editMode;
    }
        
    // increase buffer size by length of input line
    tbuffer = (char*)realloc(buffer,(curpos+localpos+1));   // add local size to total size
        if (tbuffer == NULL) {
        printf("memory error\n");
        exit(1);
    }
    buffer = tbuffer;       // copy temp to real buffer
    RFLAG = 1;

    for (int n=0; n!= localpos; n++)      // save local line to buffer
        buffer[curpos++] = linein[n];
    
    
    goto inputLoop;

    /* ----------------------------------- */

editMode:   /* command loop */
    showmode(mode);

editLoop:
    /* get a command */
    localpos = 0;
    printf("CMD>");   //show edir mode cursor
    while ( (linein[localpos++] = fgetc(stdin)) != '\n');

    // user hit <cr>
    if (linein[0] == 0x02) {
        mode = 1;   // ^B changes modes
        goto inputMode;
    }
    
    linein[--localpos] = '\0';   // replace <cr> w/\0

    /* test command */
    if ((strncmp(linein,"print",5) == 0) || (strncmp(linein,"list",4) == 0)) {
        print(linein);
        goto editLoop;
    }

    if (strncmp(linein,"new",3) == 0) {
        clearBuffer();
        goto editLoop;
    }

    if (strncmp(linein,"save",4) == 0) {
        filesave(linein);
        goto editLoop;
    }

    if (strncmp(linein,"load",4) == 0) {
        fileload(linein);
        goto editLoop;
    }

    if (strncmp(linein,"stats",5) == 0) {
        stats();
        goto editLoop;
    }

    if (strncmp(linein,"help",4) == 0) {
        usage();
        goto editLoop;
    }

    if (strncmp(linein,"del",3) == 0) {
        delete(linein);
        goto editLoop;
    }

    if (strncmp(linein,"ins",3) == 0) {
        insert(linein);
        goto editLoop;
    }

    if (strncmp(linein,"find",4) == 0) {
        char cmd[MAXLINE] = {}, testword[MAXLINE] = {};
        sscanf(linein,"%s %s",cmd,testword);
        find(testword);
        goto editLoop;
    }

    if (strncmp(linein,"sort",4) == 0) {
        sort(linein);
        goto editLoop;
    }

    if (strncmp(linein,"quit",4) == 0) goto done;

    /* unknown command */
    printf("EH?\n");
    goto editMode;
   
done:    
    /* done */
    free(buffer);   // tbuffer will close automatically
    exit(0);
}
/* --------- end of main ----------- */



void clearBuffer(void) {    // clear the memory buffer
    free(buffer);
    curpos = 0;
    buffer = (char *)malloc(MAXLINE+1);
    if (buffer != NULL) return;
    printf("Memory error\n");
    exit(1);

}


void showmode(int val) {
    if (val == 1) printf("INPUT\n");
    if (val == 0) printf("EDIT\n");
    if ((val < 0) || (val > 1)) printf("error in showmode\n");
    return;
}




void print(char *linein) {  // print the buffer
    int linenum = 1;
    int row=0;
    int pos = 0;
    int n = 0;
    while (1) {
        printf("%04d: ",linenum++);
        for (n=0; n<MAXLINE; n++) {
            printf("%c",buffer[pos+n]);
            if (buffer[pos+n] == '\n')
                break;
        }
        pos += n+1;     // account for \n
        if (pos >= curpos) return;
        row++;
        if (row < 22) continue;
        row = 0;
        printf("\n-- MORE --");
        char ch=fgetc(stdin);
        if (ch == 'q') {
            printf("\n");
            ch = fgetc(stdin);  // get <cr>
            return;
        }
        continue;
    }

    return;
}




void filesave(char *linein) {   // save buffer to file
    FILE *outfile;
    char cmd[MAXLINE] = {}, filename[MAXLINE] = {};

    int localpos = 0;
    if (strlen(linein) == 4) {  // no filename given
        printf("Save to filename? ");
        while ( (linein[localpos++] = fgetc(stdin)) != '\n');
        linein[--localpos] = '\0';
        strcpy(filename,linein);
    }
    else
            sscanf(linein,"%s %s", cmd, filename);
    
    outfile = fopen(filename,"w");
    if (outfile == NULL) {
        printf("Error creating file %s\n",linein);
        return;
    }

    /* save buffer */
    for (int i=0; i!= curpos; i++) 
        fprintf(outfile,"%c",buffer[i]);
    fflush(outfile);
    fclose(outfile);
    printf("Buffer saved\n");
    return;
}






void fileload(char linein[MAXLINE]) {   // load file into buffer
    FILE *infile;
    char ch, cmd[MAXLINE] = {}, filename[MAXLINE] = {};
    int localpos = 0;
    
    if (strlen(linein) == 4) {  // no filename given 
        printf("Load from filename? ");
        while ((linein[localpos++] = fgetc(stdin)) != '\n');
        linein[--localpos] = '\0';
        strcpy(filename,linein);
    }
    else 
        sscanf(linein,"%s %s", cmd, filename);
    infile = fopen(filename,"r");
    if (infile == NULL) {
        printf("File not found: %s\n",linein);
        return;
    }
    localpos = 0;
    while (( ch = fgetc(infile)) != EOF) {
        if (feof(infile)) break;
        linein[localpos++] = ch;
        if (localpos > MAXLINE) {
            printf("Error - reading line of %d chars\n",localpos);
            printf("line is %s\n",linein);
            exit(1);
        }
        // printf("%c",ch);     /* print file as it loads */
        if (ch == '\n') {
            // save buffer
            tbuffer = (char*)realloc(buffer,(curpos+localpos+1));   // add local size to total size
            if (tbuffer == NULL) {
                printf("memory error\n");
                exit(1);
            }
            buffer = tbuffer;       // copy temp to real buffer
            RFLAG = 1;

            for (int n=0; n!= localpos; n++)      // save local line to buffer
                buffer[curpos++] = linein[n];
            localpos = 0;
        }
        continue;
    }
    // EOF reached
    fclose(infile);
    printf("File loaded to buffer\n");
    return;
}



void stats(void) {  /* show buffer stats */
    printf("Buffer size %d bytes\n",curpos-1);
    printf("\n");
    return;
}




void usage(void) {   /* show command list */
    printf("\nLine EDitor Commands: \n");
    printf("pressing ctrl-b  switches modes\n");
    printf("new:                Clear Buffer\n");
    printf("load [filename]:    Load a file into the buffer\n");
    printf("save [filename]:    Save the buffer to a file\n");
    printf("print/list:         Show buffer contents\n");
    printf("del [line number]:  Delete a single line\n");
    printf("ins [line number]:  Insert text BEFORE line number. <cr> by itself stops\n");
    printf("stats:              Show buffer size\n");
    printf("sort:               Sort the buffer\n");
    printf("quit:               Exit the program\n");
    printf("\nA> Append Cursor, I> Insert Cursor, CMD> Command Cursor\n");
    printf("\n");
    return;
}



void delete(char *linein) {     /* delete a line or group of lines */
char cmd[MAXLINE] = {}, linenumber[MAXLINE] = {};
int line = 0, linectr = 0;
int n=0, x=0, startpos=0, endpos=0;

    if (strlen(linein) == 3) {
        printf("format: del [line#]\n");
        return;
    }

    // get line number
    sscanf(linein,"%s %s",cmd,linenumber);
    line = atof(linenumber);
    if ((line <= 0) || (line > 99999)) {
        printf("Invalid line number %d\n",line);
        return;
    }
    
    // got a valid line number in line, now see if it exists
    for (n=0; n<curpos; n++) {
        //if (buffer[n] == '\0') 
        if (buffer[n] == '\n')
            linectr+=1;
    }
    if (line > linectr) {   
        printf("Invalid line number %d\n",line);
            return;
    }

    // line exists - find position and delete line
    linectr = 0;
    startpos = 0;
    endpos = 0;
    for (n=0; n<curpos; n++) {
        //if (buffer[n] == '\0') {
        if (buffer[n] == '\n') {
            linectr++;
            endpos = n;
        }
        if (line == linectr) break;      // match, n points to start of line
        startpos = endpos;
    }
    

    x = startpos;
    while (x < curpos) {
        buffer[x] = buffer[x+(endpos-startpos)];   // shift buffer down by length of deleted line
        x++;
    }
    curpos = x-(endpos-startpos);   // decrease by length of deleted line
    return;
}




void insert(char *linein) {  /* insert a single line */
char cmd[MAXLINE] = {}, linenumber[MAXLINE] = {};
int line = 0, linectr = 0;
int n=0, startpos=0, endpos=0;

    if (strlen(linein) == 3) {
        printf("format: ins [line#]\n");
        return;
    }

    // get line number
    sscanf(linein,"%s %s",cmd,linenumber);
    line = atof(linenumber);
    if ((line <= 0) || (line > 99999)) {
        printf("Invalid line number %d\n",line);
        return;
    }

    // got a valid line number in line, now see if it exists
    for (n=0; n<=curpos; n++) {
        //if (buffer[n] == '\0')
        if (buffer[n] == '\n')
            linectr+=1;
    }
    if (line > linectr) {
        printf("Invalid line number %d\n",line);
            return;
    }
insLoop:
    // line exists - find position for insert function
    linectr = 0;
    startpos = 0;
    endpos = 0;
    for (n=0; n<curpos; n++) {
        //if (buffer[n] == '\0') {
        if (buffer[n] == '\n') {
            linectr++;
            endpos = n+1;
        }
        if (line == linectr) break;     // match, n points to start of line
        startpos = endpos;
    }

    /* now get a line of text from the user */

    localpos = 0;
    printf("I>");
    while ( (linein[localpos++] = fgetc(stdin)) != '\n');
    //linein[localpos++] = '\0';    // tack on \0
    if (linein[0] == 0x02) return;


    // save buffer
    tbuffer = (char*)realloc(buffer,(curpos+localpos+1));   // add local size to total size
        if (tbuffer == NULL) {
        printf("memory error\n");
        exit(1);
    }
    buffer = tbuffer;       // copy temp to real buffer
    RFLAG = 1;

    // shift buffer up
    int newcurpos = curpos + localpos;


    for (int n=0; n!=(curpos-startpos+1); n++) 
        buffer[(newcurpos-n)] = buffer[(curpos-n)];

    // insert new line
    for (int n=0; n != localpos; n++)   // was n < localpos
        buffer[startpos+n] = linein[n];

    curpos = newcurpos;

    /* bump line number for auto insertion until user presses <cr> by itself */
    line += 1;
    goto insLoop;

}


void find(char *linein) { // find a word in the buffer

    int linenum = 1;
    char line[MAXLINE];
    int pos = 0, i=0, n=0;
    
    while (1) {
    
        for (n=0; n<MAXLINE-1; n++)
            line[n] = '\0';         // clear temp buffer
    
        n = 0;
        for (i=pos; i<pos+MAXLINE; i++) {     // load up a line
            line[n++] = buffer[i];
            if (buffer[i] == '\n')
                break;
        }
    
        line[n] = '\0';
        if (strstr(line,linein) != NULL)   // match
            printf("%05d: %s",linenum,line);

        linenum++;
        pos = i+1;      // go past \n at end of line
        if (pos >= curpos) break;   // nothing exists past curpos
        continue;

    }

    printf("\n");
    return;
}


void sort(char *linein) {       // sort the buffer
    int line=0, listline = 0;
    int n=0;
    int pos = 0;
    char templine[MAXLINE];

    for (n=0; n<curpos; n++) 
        if (buffer[n] == '\n') line+=1;     // count line numbers in buffer

    //printf("found %d line numbers\n",line);
    listline = 1;
    char sortlist[line+1][MAXLINE]; // create a location to hold the sorted list
    char locbuf[MAXLINE];
    while (pos < curpos) {
        for (n=0; n<MAXLINE; n++) locbuf[n] = '\0'; // clear the buffer
        for (n=0; n<(pos+MAXLINE); n++) {
            locbuf[n] = buffer[pos+n];
            if (buffer[pos+n] == '\n') break;
        }    
        strcpy(sortlist[listline],locbuf);
        listline++;
        pos = pos+n+1;      // +1 to skip the \n
        if (pos < curpos) continue;
        break;
    }
    // sorted list is populated

    // sort the list
    int count = 1;
    while (count < line+1) {
        for (n=0; n<line; n++) {
            if (strcmp(sortlist[n],sortlist[n+1]) >0 ) {
                strcpy(templine,sortlist[n]);
                strcpy(sortlist[n],sortlist[n+1]);
                strcpy(sortlist[n+1],templine);
                //printf(".");
            } 
        }
        //printf("%d\n",count);
        count++;
    }

    // print the list
    //for (n=1; n!=line+1; n++)
    //    printf("%04d: %s",n,sortlist[n]);

    /* save the sorted list back to the buffer */
    free(buffer);
    curpos = 0;
    buffer = (char*) malloc(line * MAXLINE);
    for (n=0; n < line+1; n++) {
        for (int x=0; x<strlen(sortlist[n]); x++){
            buffer[curpos] = sortlist[n][x];
            curpos++;
        }
    }
    
    return;
}



/* done */
