#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

/* required enum and struct */

typedef enum rarity
{
	common,
	uncommon,
	rare,
	mythic
} RARITY;

typedef struct card
{
	unsigned int id;
	char* name;
	char* cost;
	unsigned int converted_cost;
	char* type;
	char* text;
	char* stats;
	RARITY rarity;
} CARD;

/* useful funcitons/values */

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