#define MAXLINE 135

/* set file colors */
void red () {
  printf("\033[1;31m");
}

void yellow () {   // executable files
  printf("\033[1;33m");
}

void blue () {     // directory
  printf("\033[1;34m");
}

void white () {    // files
  printf("\033[0m");
}

void magenta () {
  printf("\033[0;35m");
}

void cyan () {
  printf("\033[0;36m");
}

void green () {
  printf("\033[0;32m");
}

void redBold () {
    printf("\033[1;31m");
}

void greenBold () {
    printf("\033[1;32m");
}

void yellowBold () {
    printf("\033[1;33m");
}

void blueBold () {
    printf("\033[1;34m");
}

void magentaBold () {
    printf("\033[1;35m");
}
void cyanBold () {
    printf("\033[1;36m");
}


