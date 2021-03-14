#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>


char* rarityStrArr[] = {
	"common",
	"uncommon",
	"rare",
	"mythic"
};

#define rarityStrArrLen 4

/* horizontal border */
const char *hline = "----------------------------------------------------";

char *zptr = "\0";

