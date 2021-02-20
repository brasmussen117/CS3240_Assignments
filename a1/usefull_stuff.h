#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

/* check a new value against a previous max value, returns which is max */
int getMax(int entry, int max){
    return entry > max ? entry : max;
}

/* repeat a given string x times
	* NOTE memory must be freed
	found on stackoverflow: https://stackoverflow.com/questions/22599556/return-string-of-characters-repeated-x-times
	author: https://stackoverflow.com/users/371974/scorpiozj
 */
char *repeat(char *s, int x)
{
    char *result = malloc(sizeof(s) * x + 1);

    while (x > 0) {
        strcat(result, s);
        x--;
    }

    return result;
}