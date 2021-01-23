#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

struct line
{
    char *first;
    char *second;
    char *third;
};

int main(int argc, char const *argv[])
{
    // check for necessary arguments
    if (argc < 2)
    {
        printf("Usage: ./a0 <file_name>");
        return -1;
    }
    
    char *buf = NULL;// buffer

    struct line entries[100]; // setup var for taking lines, start with 100 lines
    
    // open the file given as argument in command line
    FILE *input_file = fopen(argv[1], "r");

    size_t n = 0; // size of result

    ssize_t result = getline(&buf, &n, input_file); // read the first line

    if (result < 0) // check that the file is not empty
    {
        printf("Error: file emtpy or getline error");
        return 1;
    }

    // put the header into entries
    char *stringp = strdup(buf);
    entries[0].first = strsep(&stringp, " ");
    entries[0].second = strsep(&stringp, " ");
    entries[0].third = strsep(&stringp, "\n");

    printf("entries 0: %s | %s | %s\n", entries[0].first, entries[0].second, entries[0].third);

    // primary loop
    // while (result < 0)
    // {
        
    // }
    
    // printf("buf: %s\n", buf); // debugging
    
    fclose(input_file);
    // free();
        // TODO: to be freed
        // buf
        // entries
        // n
        // result
        // strings: stringp, 

    return 0;
}

/* Notes:

## TODO: Primary loop for reading
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

#define octal description
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