# DESCRIPTION
Now for a challenge: write your own make!

Make makes files and keeps them up-to-date. When invoked, make tries to 'make' every file referred to through the command-line arguments. If no files are given on the command-line, make will default to attempting to build the 'target' of the first 'recipe' in it's input file.

**What's a recipe?** A recipe provides information as to which prerequisite files a target requires in order to be built as well as which commands need to be executed to make the target from the prerequisite files. The typical syntax for a recipe and commands follows. Targets might not have any prerequisites at all, and recipes might have any commands at all.

```makefile
target: dependency.c other_dep.txt
    cat other_dep.txt | grep content
    cc -g -o target dependency.c
    ...

# this is a comment

other_dep.txt:
    echo "other dep file content" > other_dep.txt

# more generally...

path/to/target: path/to/first/prerequisite path/to/second/prerequisite
	command1 argument1 argument2 argument3
	command1 arguments | command2 arguments
	command1 arguments | command2 arguments | command3 arguments
	command argument1 argument2 argument3 < path/to/input/file
	command argument1 argument2 argument3 > path/to/output/file
	command arguments < path/to/input/file > path/to/output/file
```

Notice the hard-tab (\t) proceeding each command. Newlines separate commands, each sub-command can have any number of arguments. Each command can have any number of sub-commands. Vertical bars between two sub-commands indicate that the content written to the stdout of the first should be read from as the content of stdin next. (see man 2 pipe) This operation can be chained or repeated. The first sub-command could also have a redirect-in operator <. The last sub-command could also have a redirect-out operator >.

Most of the time any path, command, or arguments ends on whitespace or colon (':'). However, if paths, commands, or arguments are proceeded with a double-quote, the contents are escaped until the next unescaped double-quote. Whitespace, colons, vertical bars, everything besides a double-quote. and a null terminator could be escaped in double quotes. What if I wanted to write a recipe that builds a target whose filename is a single double quote, and whose prerequisites are a file whose name is a literal hard-tab, and another file whose name is a literal newline? I hear you cry. Well fear not, because make supports escaping quotes in quotes with "\"", tabs with "\t", and newlines with "\n". Whew!

Prerequisites must be built from left to right

Comments can also exist in the input file, if the line starts with a '#', the rest of the line should be ignored by the the program.

Your mission, if you choose to accept it, is to implement your own program, called fake, that imitates the behavior of make. Instead of reading from makefile, it will read from fakefile.

<hr>

# Implementation
## First Steps
1. Automatically open a file named Fakefile when the program (named fake) is executed.
2. Parse this file line by line looking for:
	* Comments to ignore (any line that starts with a #)
	* Rule targets and prerequisites (any line that doesn't start with a # or \t)
	* "Recipe" actions (lines under rule targets and prerequisites that begin with \t)
3. Complete "recipes" are separated by a blank line.
4. Annoyingly throw errors if the file is formatted incorrectly, __do not be lenient!__
5. There are __no__ "implicit rules", everything must be included in the Fakefile.

## Parsing Recipes
1. For each recipe, check if the __target__ is missing in the current directory or if any dependencies are more recent than the file named __target__
	* If this is true, perform each action under the target and prerequisites
	* If false, the target is up to date, nothing needs to be done
2. Because some targets can depend on other targets that come later in the file, the entire Fakefile must be read first and a dependency tree created. This data structure is up to you (but a stack should be sufficient ...)

## Final Steps
1. Execute each command in the recipe using fork() and exec() family of functions.
2. You must parse the command line, look for | (pipe) characters to use the pipe() function, or </> (input/output redirection) to redirect a file as input or output, respectively.
3. If a recipe fails (exit status was non-zero), then stop executing recipes. Show stderr!
4. When executed, if the user provides a target name, run that target instead of the first (default) target.

<hr>

# Reading
Stephen's __Chapters 3, 7, 8__:

* __3.12__ dup2()
* __8.3__ fork()
* __8.6__ wait() and waitpid()
* __8.10__ exec()
* __15.2__ pipe()

<hr>

# Viewing
## How to Implement a Stack in C - Jacob Sorber
[![How to Implement a Stack in C - Jacob Sorber](http://img.youtube.com/vi/A4sRhuGkRb0/0.jpg)](http://www.youtube.com/watch?v=A4sRhuGkRb0)


<hr>

# Starter Code
``` C
/*
** pipex.c - multipipes support
*/

#include <error.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
 * loop over commands by sharing
 * pipes.
 */
static void
pipeline(char ***cmd)
{
	// array to hold pipe fds
	int fd[2];
	// to hold created child process id
	pid_t pid;
	// this holds the previous iteration read end fd
	int fdd = 0;
	// array to hold all pids from created child processes
	pid_t pids[4];
	// count number of loop iterations
	int i = 0;

	while (*cmd != NULL) {
		// if this IS NOT the final command
		if (*(cmd + 1) != NULL) {
			// make a pipe
			pipe(fd);
		}
		// split this process into two
		if ((pid = fork()) == -1) {
			// was there an error?
			perror("fork");
			exit(1);
		}

		/* CHILD PROCESS */
		else if (pid == 0) {
			// previously created read end pipe is now stdin
			// (or this is already stdin)
			dup2(fdd, 0);
			// close fdd as it has been made stdin
			if (fdd > 0) {
				int err = close(fdd);
				if (err) {
					perror("close()");
				}
			}
			// if this IS NOT the final command, also make write
			// end of current pipe the stdout
			if (*(cmd + 1) != NULL) {
				dup2(fd[1], 1);
			}
			// close pipe file descriptors in child as we now
			// have a new stdin and stdout
			close(fd[0]);
			close(fd[1]);
			// execute the program!
			execvp((*cmd)[0], *cmd);
			// never execute this (hopefully)
			exit(1);
		/* PARENT PROCESS */
		} else {
			// close fdd, not needed
			if (fdd > 0) {
				close(fdd);
			}
			// save the pid
			pids[i++] = pid;
			// close the write end of the pipe as well
			close(fd[1]);
			// save the read end of current pipe
			fdd = fd[0];
			// go to next command
			cmd++;
		}
	}

	// wait on pids here, similar to how pthread_join() is used
	for (int j = (i - 1); j > -1; --j) {
		waitpid(pids[j], NULL, 0);
	}
}

/*
 * Compute multi-pipeline based
 * on a command list.
 */
int main(int argc, char *argv[])
{
	char *ls[] = {"ls", "-al", NULL};
	char *rev[] = {"rev", NULL};
	char *nl[] = {"nl", NULL};
	char *cat[] = {"cat", "-e", NULL};
	char **cmd[] = {ls, rev, nl, cat, NULL};

	/*
	char *yes[] = {"yes", NULL};
	char *cat[] = {"cat", NULL};
	char *head[] = {"head", NULL};
	char **cmd[] = {yes, cat, head, NULL};
	*/

	pipeline(cmd);
	return (0);
}
```