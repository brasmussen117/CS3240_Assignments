#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>

#include "parser.h"
#include "builder.h"
#include "stack.h"

/* defs */
#define MAXLINE 50 // max input length
#define MAXBUF 1024 // max buffer length
#define ISTERMINAL ((isatty(STDIN_FILENO) == 1) ? true : false) // if tty == 1, then stdin is from terminal
#define ISREDIRECT ((isatty(STDIN_FILENO) == 0) ? true : false) // if tty == 0, then stdin has been redirected

/* #region fake structs -------------------------------------------- */

typedef struct recipe
{
	char *target;
	char **prereqs;
	char **actions;
} RECIPE;

/* #endregion fake structs */

