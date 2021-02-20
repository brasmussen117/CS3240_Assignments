#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

/* function prototype */
int getMax(int, int);
char *repeat(char *, int);

/* struct for holding the contents of each line */
typedef struct _line
{
    char *first;
    char *second;
    char *third;
} LINE;

int main(int argc, char const *argv[])
{
    /* check for necessary arguments */
    if (argc != 2)
    {
        printf("Usage: ./main <file_name>");
        return -1;
    }
    
    char *buf = NULL; // buffer

    char *stringp = NULL; // copy of buf for tokenizing

    LINE **entries = malloc(2 * sizeof(LINE*)); // var for taking lines

    FILE *input_file = fopen(argv[1], "r"); // file handle; given as argument in command line

    size_t n = 0; // size of result

    ssize_t result = getline(&buf, &n, input_file); // result of getline

    if (result < 0) // check that the file is not empty
    {
        free(buf);
        printf("Error: file emtpy or getline error");
        return 1;
    }
    // free(buf);

    char *free_stringp = NULL;

    // entries[0] = malloc(sizeof(line_t)); // malloc the line at entries[0]
    
    // entries[0]->first = strsep(&stringp, " ");
    // entries[0]->second = strsep(&stringp, " ");
    // entries[0]->third = strsep(&stringp, "\n");

    // free(free_stringp); // free last usage of stringp

    /* set the max values */
    int first_max_len = 0;
    int second_max_len = 0;
    int third_max_len = 0;

    // printf("\nentries[0]: %s | %s | %s\n", entries[0].first, entries[0].second, entries[0].third); // debugging
    // printf("max strlen >>> first: %d, second: %d, third: %d\n\n", first_max_len, second_max_len, third_max_len); // debugging

    /* #Read */
    int count = 0; // loop counter

    while (result > 0)
    {
        stringp = strdup(buf); // duplicate the buf into stringp
        free_stringp = stringp;
		// printf("%d, ", count); // debugging

        entries[count] = malloc(sizeof(LINE)); // allocate the new line

        /* split the sections into entries */        
        entries[count]->first = strsep(&stringp, " ");
        entries[count]->second = strsep(&stringp, " ");
        
        if (count == 0)
        {
            entries[0]->third = strsep(&stringp, "\n");
        } else {
            stringp++; // advance past the beginning single quote
            entries[count]->third = strsep(&stringp, "'");
        }

        /* check/get the max strlen */
        first_max_len = getMax(strlen(entries[count]->first), first_max_len);
        second_max_len = getMax(strlen(entries[count]->second), second_max_len);
        third_max_len = getMax(strlen(entries[count]->third), third_max_len);

        free(free_stringp); // free stringp for next loop

        count++; // increment count

        entries = realloc(entries, count * sizeof(LINE)); // realloc as necessary

        result = getline(&buf, &n, input_file); // get the next line
    }
    
    /* # Print */

    // printf("\nmax strlen >>> first: %d, second: %d, third: %d", first_max_len, second_max_len, third_max_len); // debugging
    // printf("\nentries size: %d\n\n", count); // debugging
    
    int total_width = 2 + first_max_len + 3 + second_max_len + 3 + third_max_len + 2; // calc the total_width

    /* setup the horizontal bar */
    char *horizontal = "-"; // var for the bar
    horizontal = repeat(horizontal, total_width); // creat bar of length total_width
    horizontal = strcat(horizontal, "\n"); // new line
    
    char *first_padding;
    char *second_padding;
    char *third_padding;

    for (size_t i = 0; i < count; i++) // primary print loop
    {
        if ((i == 0) | (i == 1)) // print the border before the first and before the second line
        {
            printf("%s", horizontal); // print the hyphen border line
        }

        first_padding = " "; // zero out the padding
        second_padding = " "; // zero out the padding
        third_padding = " "; // zero out the padding
        
        printf("| "); // left side border and space

        printf("%s", entries[i]->first); // print first text

        /* if first column padding needed, calc and print */
        if (first_max_len != strlen(entries[i]->first)) // check if padding needed
        {
			first_padding = repeat(first_padding, first_max_len - strlen(entries[i]->first));
            printf("%s", first_padding);
            free(first_padding);
        }

        printf(" | "); // print the first/second divider

        /* if second column padding needed, calc and print */
        if (second_max_len != strlen(entries[i]->second)) // check if padding needed
        {
			second_padding = repeat(second_padding, second_max_len - strlen(entries[i]->second));
            printf("%s", second_padding);
            free(second_padding);
        }

        printf("%s", entries[i]->second); // print second text

        printf(" | "); // print the second/third divider

        printf("%s", entries[i]->third); // print third text

        /* if third column padding needed, calc and print */
        if (third_max_len != strlen(entries[i]->third)) // check if padding needed
        {
			third_padding = repeat(third_padding, third_max_len - strlen(entries[i]->third));
            printf("%s", third_padding);
	        free(third_padding);
        }

		printf(" |\n"); // end bar and new line

        if (i == (count - 1)) // print the border after the final line
        {
            printf("%s", horizontal); // print the hyphen border line
        }
    }
    
   
    fclose(input_file); // close the file
    
    // TODO: to be freed
    for (size_t i = 0; i < count; i++) // loop to free each part of entries
    {
        free(entries[i]);
    }
    free(entries); // free array
    
    free(buf);
    // free(stringp);
	free(horizontal);
	// free(first_padding);
	// free(second_padding);
	// free(third_padding);

    return 0;
}

/* check a new value against a previous max value, returns which is max */
int getMax(int entry, int max){
    return entry > max ? entry : max;
}

/* repeat a given string x times
	found on stackoverflow: https://stackoverflow.com/questions/22599556/return-string-of-characters-repeated-x-times
	author: https://stackoverflow.com/users/371974/scorpiozj
 */
char *repeat(char *s, int x)
{
    char *result = calloc(sizeof(s),x + 1);

    while (x > 0) {
        strcat(result, s);
        x --;
    }

    return result;
}

/* Notes:

## Primary loop for reading
1. loop thru file until readline returns end of file
2. split each section of result into entries
    A. first two fields are space delimited
        * read beginning of line until first space into entries[i].first
        * skip the first space
        * read next char until the second space into entries[i].second
        * skip the second space
    B. final field is bounded by single quotes (after the header row, header has no quotes)
        * skip the first quote
        * read the next char until the second single quote into entries[i].third
3. keep track of the length of each section, need max sizes for printing
4. check if entries has maxed out, realloc as necessary
5. loop again, readline

## Primary loop for printing
1. loop until everything from entries is printed
2. print bar of hypens, as wide as max widths for first, second, and third combined plus spaces on either side of each section and the bars

 */


/* # Description:
Write a program that reads a space-delimited table from a file. The table will always have three columns. The number rows will differ bewteen input files and is unbounded, meaning that there is no set maximum length. There will always be at least one row (no empty table).

The first row of the table (refered to as the header row), will have three strings, space delimited. The strings will never contain whitespace. The strings' lengths are unbounded. 

All of the rows after the first will each contain three strings, space delimited, each unbounded. The third string will always be enclosed in single-quotes. The third string may contain whitespace, but will never contain a single-quote.

Once the program has read the table from the file, it needs to print out the data of the table to the terminal. The table should be enclosed the vertical bars and hypens (see examples below). The first and third column should be left-aligned, the second column should be right-aligned.

The program should compile with -std=gnu11, -Werror, and -Wall, all memory should be freed, and valgrind should report no memory errors. Good luck!

# First Example:

* $ cat input1.txt

.#define octal description
S_IRWXU 00700 'user (file owner) has read, write, and execute permission'
S_IRUSR 00400 'user has read permission'
S_IWUSR 00200 'user has write permission'
S_IXUSR 00100 'user has execute permission'
S_IRWXG 00070 'group has read, write, and execute permission'
S_IRGRP 00040 'group has read permission'
S_IWGRP 00020 'group has write permission'
S_IXGRP 00010 'group has execute permission'
S_IRWXO 00007 'others have read, write, and execute permission'
S_IROTH 00004 'others have read permission'
S_IWOTH 00002 'others have write permission'
S_IXOTH 00001 'others have execute permission'

* $ make && ./main input1.txt

-------------------------------------------------------------------------------
| #define | octal | description                                               |
-------------------------------------------------------------------------------
| S_IRWXU | 00700 | user (file owner) has read, write, and execute permission |
| S_IRUSR | 00400 | user has read permission                                  |
| S_IWUSR | 00200 | user has write permission                                 |
| S_IXUSR | 00100 | user has execute permission                               |
| S_IRWXG | 00070 | group has read, write, and execute permission             |
| S_IRGRP | 00040 | group has read permission                                 |
| S_IWGRP | 00020 | group has write permission                                |
| S_IXGRP | 00010 | group has execute permission                              |
| S_IRWXO | 00007 | others have read, write, and execute permission           |
| S_IROTH | 00004 | others have read permission                               |
| S_IWOTH | 00002 | others have write permission                              |
| S_IXOTH | 00001 | others have execute permission                            |
-------------------------------------------------------------------------------
 */