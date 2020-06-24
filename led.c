/* led - simple line editor
 *
 * if using valgrind, compile with:
 *      cc -o led led.c -g -O0 -Wall -ansi
 * to use valgrind:
 *      valgrind --log-file=led.val --leak-check=full --show-reachable=yes ./led
 * else compile with 
 *      cc -o led led.c -Wall -ansi
 *
 * k theis 6/24/2020
 * version 0.70 added undo:  undo the last buffer changing operation
 * version 0.66 replace format changed: replace [starting line number][start][end]
 * version 0.65 added replace/replaceall commands 
 * version 0.61 changed print routine
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAXLINE
#define MAXLINE 1000      /* max line length */
#endif

#ifndef LINENO_MAX 
#define LINENO_MAX 99999    /* max line number */
#endif

#ifndef LINENO_MIN
#define LINENO_MIN 1        /* lowest line number */
#endif

/* global variables */
char *buffer, *tbuffer, *undobuffer, cutbuffer[MAXLINE]={'\n'};
int curpos=0, mode=0, undocurpos = 0;
char linein[MAXLINE+1] = {};
int localpos = 0;
char ch[MAXLINE] = {};            /* keyboard input */ 
int RFLAG=0;                /* test for tbuffer use at exit */

/* define routines */
void print(char *);         /* print */
void showmode(int);
void clearBuffer(void);     /* new */
void filesave(char *);      /* save */
void fileload(char *);      /* load */
void stats(void);           /* stats */
void usage(void);           /* help */
void delete(char *);        /* delete a single line */
void paste(char *);         /* paste the cut buffer to the buffer */
void insert(char *);        /* insert a single line */
void find(char *);          /* find a word */
void sort(char *);          /* sort the buffer  */
void repl(char *);          /* global replace  */
void undo_save(void);       /* save buffer and pointers for later undo */
void undo(void);            /* undo last operation */

/* start of main */
int main(int argc, char **argv) {
int n=0;

    /* set up memory for storage */
    buffer = (char*) malloc((MAXLINE * 2) * sizeof(char*));
    if (buffer == NULL) {
        printf("memory error in main\n");
        exit(1);
    }
    undobuffer = (char *)malloc((MAXLINE * 2) * sizeof(char *));
    if (undobuffer == NULL) {
        printf("memory error in main\n");
        exit(1);
    }

    curpos = 0; /* curpos is current position in the buffer */

    /* test command line - if argc==2 treat the option as filename to load */
    if (argc == 2) {
        char loadcmd[MAXLINE];
        strcpy(loadcmd,"load ");
        strcat(loadcmd,argv[1]);
        fileload(loadcmd);
    }

    if (argc > 2) {     /* show usage and exit */
        printf("Usage: led [filename] \n");
        printf("where filename is a file to load at start\n");
        return 0;
    }

    mode = 1;       /* mode=1:input mode   0:edit mode */

inputMode:
    showmode(mode);

inputLoop:    /* entry loop */
    localpos = 0;
    printf("A>");       /* show append mode cursor */
    while ( (linein[localpos++] = fgetc(stdin)) != '\n');

    if (linein[0] == 0x02) {
        mode = 0;  /* ^B changes modes */
        goto editMode;
    }
        
    /* increase buffer size by length of input line */
    tbuffer = (char*)realloc(buffer,(curpos+localpos+1));   /* add local size to total size */
        if (tbuffer == NULL) {
        printf("memory error\n");
        exit(1);
    }
    buffer = tbuffer;       /* copy temp to real buffer */
    RFLAG = 1;

    for (n=0; n!= localpos; n++)      /* save local line to buffer */
        buffer[curpos++] = linein[n];
    
    
    goto inputLoop;

    /* ----------------------------------- */

editMode:   /* command loop */
    showmode(mode);

editLoop:
    /* get a command */
    localpos = 0;
    printf("CMD>");   /*  show command prompt  */
    for (n=0; n<MAXLINE; n++) linein[n]='\0';   /* clear buffer  */
    /* while ( (linein[localpos++] = fgetc(stdin)) != '\n'); */  /* alt form */
    fgets(linein,sizeof(linein)-1,stdin);
    localpos = strlen(linein);

    /* user hit ctrl-B/enter at pos 0 */
    if (linein[0] == 0x02) {
        mode = 1;   /* ^B changes modes  */
        goto inputMode;
    }
    
    linein[--localpos] = '\0';   /* replace <cr> w/\0  */

    /* test command */
    if ((strncmp(linein,"print",5) == 0) || (strncmp(linein,"list",4) == 0)) {
        print(linein);
        goto editLoop;
    }

    if (strncmp(linein,"new",3) == 0) {
        undo_save();    /* save buffer and pointers for undo command  */
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
        undo_save();
        delete(linein);
        goto editLoop;
    }
    
    if (strncmp(linein,"cut",3) == 0) {
        undo_save();
        delete(linein);
        goto editLoop;
    }

    if (strncmp(linein,"copy",4) == 0) {
        undo_save();
        delete(linein);
        goto editLoop;
    }
    
    if (strncmp(linein,"paste",5) == 0) {
        undo_save();
        paste(linein);
        goto editLoop;
    }

    if (strncmp(linein,"ins",3) == 0) {
        undo_save();
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
        undo_save();
        sort(linein);
        goto editLoop;
    }

    if (strncmp(linein,"replace",7) == 0) {
        undo_save();
        repl(linein);
        goto editLoop;
    }

    if (strncmp(linein,"replaceall",10) == 0) {
        undo_save();
        repl(linein);
        goto editLoop;
    }

    if (strncmp(linein,"undo",4) == 0) {
        undo();
        goto editLoop;
    }

    if (strncmp(linein,"quit",4) == 0) goto done;

    /* unknown command */
    printf("EH?\n");
    goto editMode;
   
done:    
    /* done */
    free(buffer);
    free(undobuffer);
    exit(0);
}
/* --------- end of main ----------- */



void clearBuffer(void) {    /* clear the memory buffer */
    free(buffer);
    curpos = 0;
    buffer = (char*) malloc((MAXLINE * 2) * sizeof(char*));
    if (buffer != NULL) {
        buffer[curpos]='\0';
        return;
    }
    free(undobuffer);
    printf("Memory error in clearBuffer\n");
    exit(1);

}



void showmode(int val) {
    if (val == 1) printf("INPUT\n");
    if (val == 0) printf("EDIT\n");
    if ((val < 0) || (val > 1)) printf("error in showmode\n");
    return;
}



/* print a line - format: print/list [start number][end number] */
/* [start number] defaults to 0, [end number] defaults to 99999 */
void print(char *linein) {    /* display buffer */

    /* get line #'s to print */
    char cmd[MAXLINE]={}, linelow[MAXLINE]={}, linehi[MAXLINE]={};
    sscanf(linein,"%s %s %s",cmd,linelow,linehi);
    int startline=atoi(linelow);
    if (startline == 0) startline = LINENO_MIN;
    int endline=atoi(linehi);
    if (endline == 0) endline = LINENO_MAX;
    
    char *data = &buffer[0];
    int line = 1;   /* current line # */
    int row = 1;    /* row counter */

    int n=0;
    char chbuf[MAXLINE];
    for (n=0; n<MAXLINE; n++) chbuf[n] = '\0';
    while ((data - &buffer[0]) < curpos-1) {    /* getting a line of data at a time */
        for (n=0; n<MAXLINE; n++) {             /* makes it easier to print ranges */
            chbuf[n] = *data++;                 /* of line numbers */
            if (chbuf[n] == '\n') break;        /* and have formatting look good */
        }
        /* got a \n */
        if ((line >= startline) && (line <= endline))
            printf("%04d: %s",line,chbuf);
        if (line > endline) {
            printf("\n");
            return;
        }
        for (n=0; n<MAXLINE; n++) chbuf[n] = '\0';
        line++;
        if ((line >= startline) && (line <= endline)) 
            row++;
        else
            row = 1;                /* don't count rows if we're not printing them */
        if (row < 25) continue;
        row = 1;
        printf("\n--- MORE ---");
        char ch = fgetc(stdin);
        if (ch == 'q') {
            printf("\n");
            ch = fgetc(stdin);
            return;
        }
        else 
            while (ch != '\n') ch = fgetc(stdin);   /* ignore all but <cr> */
    }
    return;

}






void filesave(char *linein) {   /* save buffer to file */
    FILE *outfile;
    char cmd[MAXLINE] = {}, filename[MAXLINE] = {};
    int i=0;

    int localpos = 0;
    if (strlen(linein) == 4) {  /* no filename given */
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
    for (i=0; i!= curpos; i++) 
        fprintf(outfile,"%c",buffer[i]);
    fflush(outfile);
    fclose(outfile);
    printf("Buffer saved\n");
    return;
}






void fileload(char linein[MAXLINE]) {   /* load file into buffer */
    FILE *infile;
    char ch, cmd[MAXLINE] = {}, filename[MAXLINE] = {};
    int localpos = 0;
    int n=0;

    if (strlen(linein) == 4) {  /* no filename given  */
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
        /* printf("%c",ch); */     /* print file as it loads */
        if (ch == '\n') {
            /* save buffer */
            tbuffer = (char*)realloc(buffer,(curpos+localpos+1));   /* add local size to total size */
            if (tbuffer == NULL) {
                printf("memory error in fileload\n");
                exit(1);
            }
            buffer = tbuffer;       /* copy temp to real buffer */
            RFLAG = 1;

            for (n=0; n!= localpos; n++)      /* save local line to buffer */
                buffer[curpos++] = linein[n];
            localpos = 0;
        }
        continue;
    }
    /* EOF reached */
    fclose(infile);
    printf("File loaded to buffer\n");
    return;
}



void stats(void) {  /* show buffer stats */
    int line=0, ct=0, n=0;
    printf("Buffer size %d bytes\n",curpos);
    for (ct=0; ct<curpos; ct++) 
        if (buffer[ct]=='\n') line++;
    printf("%d lines\n",line);
    printf("Contents of the cut buffer:\n");
    for (n=0; n<MAXLINE; n++) {
        printf("%c",cutbuffer[n]);
        if (cutbuffer[n] == '\n') break;
    }
    printf("\n");

    return;
}




void usage(void) {   /* show command list */
    printf("\nLine EDitor Commands: \n");
    printf("Pressing ctrl-b then enter on an empty line switches modes\n\n");
    printf("new                     Clear the buffer.\n");
    printf("load [filename]         Load a named file into end of the buffer.\n");
    printf("save [filename]         Save the buffer to a named file.\n");
    printf("print [start][end]      Show buffer contents. start/end are optional line #'s.\n");
    printf("print by itself prints all the lines.\n");
    printf("at --MORE-- press <cr> for next, q<cr> to stop listing.\n");
    printf("del [line number]       Delete a single line.\n");
    printf("ins [line number]       Insert text BEFORE line number. ctrl-b then enter stops insert.\n");
    printf("cut [line number]       Delete a line, saving it to the cut buffer for later use.\n");
    printf("copy [line number]      Copy a line to the cut buffer for later use.\n");
    printf("paste [line number]     Paste the cut buffer BEFORE line number.\n");
    printf("stats                   Show buffer stats.\n");
    printf("find [string]           Find string in the buffer. Prints the line# & entire line.\n");
    printf("sort                    Alphabetic sort of the buffer. Destructive to original.\n");
    printf("replace [line#][old][new] & replaceall [line#][old][new]\n");
    printf("Replaces 'old' with 'new' starting at line 'line#'. replace does this once.\n");
    printf("replaceall does the same but from line# until the end of the buffer\n");
    printf("undo                    Undo the last buffer changing operation\n");
    printf("quit                    Exit the program (no automatic save).\n");
    printf("\nPrompts: A> Append, I> Insert, CMD> Command\n");
    printf("\n");
    return;
}



void delete(char *linein) {  /* delete, copy or cut a line */
    char cmd[MAXLINE]={}, linenumber[MAXLINE]={};
    int line=0; int linectr=0;
    int n=0, indx=0, startpos = 0, endpos = 0, dist = 0;

    /* get the line number */
    sscanf(linein,"%s %s",cmd,linenumber);
    line = atof(linenumber);
    if ((line < LINENO_MIN) || (line > LINENO_MAX)) {
        printf("Invalid line number %d\n",line);
        return;
    }

    startpos = 0;
    for (n=0; n<curpos; n++) {
        if (buffer[n] == '\n') {
            linectr++;
            endpos = n;
            if (line == linectr) 
                goto del1;
            startpos = endpos+1;    /* skip the \n */
        }
    }
    if (line != linectr) {
        if (n==curpos) {
            printf("line %d not found\n",line);
            return;
        }
    }

/* match */    
del1:
    if (line == linectr) {
        /* get distance from start of line to end of line */
        dist = (endpos-startpos)+1;

        /* save the line for later paste or undo */
        for (indx=0; indx < dist; indx++)
            cutbuffer[indx] = buffer[startpos+indx];
        cutbuffer[indx]='\0';

        /* check if copy command */
        if (strcmp(cmd,"copy") ==0) return;

        /* delete the line, shift the buffer down by dist */
        for (n=endpos+1; n<curpos; n++) 
            buffer[n-dist] = buffer[n];
        /* reset the curpos pointer */
        curpos = curpos - dist;
        return;
    }
    printf("underfined error in delete\n");
    return;
}


void paste(char *linein) {  /* paste the cutbuffer into the buffer */
char cmd[MAXLINE] = {}, linenumber[MAXLINE] = {};
int line = 0, linectr = 0;
int n = 0, startpos = 0, endpos = 0, dist = 0;

    /* get the line number */
    sscanf(linein,"%s %s",cmd,linenumber);
    line = atof(linenumber);
    if ((line <= 0) || (line > 99999)) {
        printf("Invalid line number %d\n",line);
        return;
    }

    /* find where in the buffer it starts */
    for (n=0; n<curpos; n++) {
        if (buffer[n] == '\n') {
            linectr++;
            endpos = n+1;
        }
        if (line == linectr) break;
        startpos = endpos;
    }

    if (n >= curpos) {
        printf("line number %d not found\n",line);
        return;
    }

    /* curpos, endpos contain the contents of line */

    /* find the length of the cut buffer */
    for (n=0; n<MAXLINE; n++) 
        if (cutbuffer[n] == '\n') break;    /* length of cutbuffer */
    if (n == MAXLINE) {
        printf("cut buffer is empty/corrupt\n");
        return;
    }
    dist = n;
    
    tbuffer = (char *)realloc(buffer,curpos+dist+1);
    if (tbuffer == NULL) {
        printf("memory error in paste\n");
        exit(1);
    }
    buffer = tbuffer;
    RFLAG = 1;

    /* shift buffer up by size dist */
    int newcurpos = curpos + dist+1;
    for (n=0; n != (curpos-startpos+1); n++) 
        buffer[newcurpos-n] = buffer[curpos-n];

    /* insert cutbuffer */
    for (n=0; n!=dist+1; n++)
        buffer[startpos+n] = cutbuffer[n];

    curpos = newcurpos;

    return;
}


void insert(char *linein) {  /* insert a group of lines. Exit w/ctrl-b */
char cmd[MAXLINE] = {}, linenumber[MAXLINE] = {};
int line = 0, linectr = 0;
int n=0, startpos=0, endpos=0;

    if (strlen(linein) == 3) {
        printf("format: ins [line#]\n");
        return;
    }

    /* get line number */
    sscanf(linein,"%s %s",cmd,linenumber);
    line = atof(linenumber);
    if ((line <= 0) || (line > 99999)) {
        printf("Invalid line number %d\n",line);
        return;
    }

    /* got a valid line number in line, now see if it exists */
    for (n=0; n<=curpos; n++) {
        if (buffer[n] == '\n')
            linectr+=1;
    }
    if (line > linectr) {
        printf("Invalid line number %d\n",line);
            return;
    }
insLoop:
    /* line exists - find position for insert function */
    linectr = 0;
    startpos = 0;
    endpos = 0;
    for (n=0; n<curpos; n++) {
        if (buffer[n] == '\n') {
            linectr++;
            endpos = n+1;
        }
        if (line == linectr) break;     /* match, n points to start of line */
        startpos = endpos;
    }

    /* now get a line of text from the user */

    localpos = 0;
    printf("I>");
    while ( (linein[localpos++] = fgetc(stdin)) != '\n');
    if (linein[0] == 0x02) return;


    /* save buffer */
    tbuffer = (char*)realloc(buffer,(curpos+localpos+1));   /* add local size to total size */
        if (tbuffer == NULL) {
        printf("memory error in insert\n");
        exit(1);
    }
    buffer = tbuffer;       /* copy temp to real buffer */
    RFLAG = 1;

    /* shift buffer up */
    int newcurpos = curpos + localpos;


    for (n=0; n!=(curpos-startpos+1); n++) 
        buffer[(newcurpos-n)] = buffer[(curpos-n)];

    /* insert new line */
    for (n=0; n != localpos; n++)   
        buffer[startpos+n] = linein[n];

    curpos = newcurpos;

    /* bump line number for auto insertion until user presses ctrl-b & enter */
    line += 1;
    goto insLoop;

}


void find(char *linein) { /* find a word in the buffer */

    int linenum = 1;
    char line[MAXLINE];
    int pos = 0, i=0, n=0;
    
    while (1) {
    
        for (n=0; n<MAXLINE-1; n++)
            line[n] = '\0';         /* clear temp buffer */
    
        n = 0;
        for (i=pos; i<pos+MAXLINE; i++) {     /* load up a line */
            line[n++] = buffer[i];
            if (buffer[i] == '\n')
                break;
        }
    
        line[n] = '\0';
        if (strstr(line,linein) != NULL)   /* match */
            printf("%05d: %s",linenum,line);

        linenum++;
        pos = i+1;      /* go past \n at end of line */
        if (pos >= curpos) break;   /* nothing exists past curpos */
        continue;

    }

    printf("\n");
    return;
}


void sort(char *linein) {       /* sort the buffer */
    int line=0, listline = 0;
    int n=0, x=0;
    int pos = 0;
    char templine[MAXLINE];

    for (n=0; n<curpos; n++) 
        if (buffer[n] == '\n') line+=1;     /* count line numbers in buffer */

    listline = 1;
    char sortlist[line+1][MAXLINE]; /* create a location to hold the sorted list */
    char locbuf[MAXLINE];
    while (pos < curpos) {
        for (n=0; n<MAXLINE; n++) locbuf[n] = '\0'; /* clear the buffer */
        for (n=0; n<(pos+MAXLINE); n++) {
            locbuf[n] = buffer[pos+n];
            if (buffer[pos+n] == '\n') break;
        }    
        strcpy(sortlist[listline],locbuf);
        listline++;
        pos = pos+n+1;      /* +1 to skip the \n */
        if (pos < curpos) continue;
        break;
    }
    /* sorted list is populated */

    /* sort the list */
    int count = 1;
    while (count < line+1) {
        for (n=0; n<line; n++) {
            if (strcmp(sortlist[n],sortlist[n+1]) >0 ) {
                strcpy(templine,sortlist[n]);
                strcpy(sortlist[n],sortlist[n+1]);
                strcpy(sortlist[n+1],templine);
            } 
        }
        count++;
    }

    /* save the sorted list back to the buffer */
    free(buffer);
    curpos = 0;
    buffer = (char*) malloc(((line * MAXLINE) + 1) * sizeof(char*));
    for (n=0; n < line+1; n++) {
        for (x=0; x<strlen(sortlist[n]); x++){
            buffer[curpos] = sortlist[n][x];
            curpos++;
        }
    }
    
    return;
}


/* replace a string (replace=once, replaceall=global) */
/* format: replace/replaceall [start line] [orig string] [replacement string] */
void repl(char *linein) {
    char cmd[MAXLINE]={}, linenum[MAXLINE]={}, orig[MAXLINE]={}, replace[MAXLINE]={};
    sscanf(linein,"%s %s %s %s",cmd,linenum,orig,replace);
    int distance = 0, startpos = 0, endpos = 0, line = 0;
    int newcurpos = 0, lineptr = 0;
    int FLAG = 0, n=0;

    /* error checking */
    line = atoi(linenum);
    if ((line < LINENO_MIN) || (line > LINENO_MAX)) {
        printf("invalid line number %d\n",line);
        return;
    }

    if ((strlen(orig)==0) || (strlen(replace)==0)) {
        printf("Format: replace/replaceall [original string] [replacement string]\n");
        return;
    }

    /* find starting position (lineptr) given a line number in line */
    for (n=0; n<curpos; n++) {
        if (n == curpos-1) {    /* line not found */
            printf("line %d not found\n",line);
            return;
        }
        if (line==1) {  /* special case */
            lineptr=0;
            break;
        }
        if (buffer[n] == '\n') {
            lineptr++;
            if (lineptr == line-1) {    /* want to start at beginning of line #, not after */
                lineptr = n+1;
                break;
            }     
        }
    }


replaceBegin:       /* loop back to here for global */
    /* search for instance */
    distance = 0; startpos = 0; endpos = 0;
    char *data = &buffer[lineptr];      /* was 0 */
    char *p = strstr(data,(char *)orig);
    if (p == NULL) {
        if (!FLAG) printf("string %s not found\n",orig);
        return;
    }
    /* get start position of string */
    startpos = (p - &buffer[0]);
    
    /* make sure the beginning is either a space or \n */
    /* (some test here) */
   
    /* found start, now find end of string */
    while (p++) {
        if (*p != ' ') break; 
        if (*p != '\n') break;
    }
    /* strings end either w/a newline or space. */

    endpos = (p - &buffer[0]); 
    distance = (endpos - startpos) + 1;
   
    /* old string length = new string length */
    if (strlen(orig) == strlen(replace)) {
        for (n=0; n<strlen(replace); n++)
            buffer[startpos+n] = replace[n];
        FLAG=1;
        if (strcmp(cmd,"replace") == 0) return;
        if (strcmp(cmd,"replaceall") == 0) goto replaceBegin;
    }
    

    /* old string length > new string length */ 
    if (strlen(replace) < strlen(orig)) {
       for (n=0; n<strlen(replace); n++)
           buffer[startpos+n] = replace[n];

    /* shift down the difference */   
        endpos = startpos + strlen(orig);
        distance = strlen(orig) - strlen(replace);
        for (n=endpos; n<curpos; n++)
            buffer[n-distance] = buffer[n];
        curpos = curpos - distance;
        FLAG=1;
        if (strcmp(cmd,"replace") == 0) return;
        if (strcmp(cmd,"replaceall") == 0) goto replaceBegin;
    }


    /* new string length > old string length */
    if (strlen(replace) > strlen(orig)) {
        distance = (strlen(replace) - strlen(orig));
        tbuffer = (char *)realloc(buffer,(curpos+distance+1));
        if (tbuffer == NULL) {
            printf("memory error in replace\n");
            exit(1);
        }
        buffer = tbuffer;
        RFLAG = 1;
        /* move buffer contents up by distance */
        newcurpos = curpos + distance;
        for (n=0; n != (curpos-endpos+1); n++)
            buffer[newcurpos-n] = buffer[curpos-n];
        curpos = newcurpos;
        /* now replace the string */
        for (n=0; n!= strlen(replace); n++)
            buffer[startpos+n] = replace[n];
        FLAG = 1;
        if (strcmp(cmd,"replace") == 0) return;
        if (strcmp(cmd,"replaceall") == 0) goto replaceBegin;
    }

    /* should never get here */
    printf("unknown error in replace\n");
    return;
}


void undo_save(void) {  /* save buffer and pointers  */
    tbuffer = (char *)realloc(undobuffer,curpos);   /* copy buffer to undo buffer */
    if (undobuffer == NULL) {
        printf("memory error in undo_save\n");
        exit(1);
    }
    undobuffer = tbuffer;
    memcpy(undobuffer,buffer,curpos);
    undocurpos = curpos;
    return;
}


void undo(void) {   /* undo last operation by restoring buffer and pointers */
    curpos = undocurpos;
    free(buffer);
    buffer = (char *)malloc(curpos);
    if (buffer == NULL) {
        printf("memory error in undo\n");
        exit(1);
    }
    memcpy(buffer,undobuffer,curpos);
    return;
}



/* done */
