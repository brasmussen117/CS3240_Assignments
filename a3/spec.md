# Requirements
## Summary
Your program, which will be invoked as defrag will have to traverse through a directory tree looking for pieces of an mp3, each piece's filename would be made up of an non-negative integer and the file-extension '.mp3' The file contents of those pieces should be concatenated into an output file in the order the numbers imply.

Your program will be given two arguments, a path to the directory tree to explore, and the name of the output file. The starting directory in the traversal will always only contain subdirectories. Your program must explore those subdirectories using asynchronous threads. Remember to compile and link your program with the -pthread flag.

File numbers will always start at 0, each integer only used once, and the order the files are discovered is not necessarily (rarely) in numeric order.

## Intended Implementation
The intended implementation of this program is that there is a dynamically resized global array of file contents shared by all threads. As each thread finds an mp3 file, it will read the file contents and store it in the array in the index of the mp3 piece number. Since the total number of pieces are not known, the array may have to me realloc'd, therefore, it must be protected by a mutex. This is not the only way to implement this program, this way isn't required either, so feel free to do it your own way! 

**Every submission that passes the automated testers will be manually inspected for data-race protection, hardcoded solutions, etc.**

## Example
The directory tree with pieces and the correct output file are given in the starting code. In this example, defrag would be run as: $ ./defrag dirs output.mp3

## See also
* $ man 3 pthread_create
* $ man 3 pthread_join
* $ man 3 pthread_mutex_init
* $ man 3 pthread_mutex_lock
* $ man 3 pthread_mutex_unlock
* $ man 3 pthread_mutex_destroy


***

# Implementation
## Steps
	1.	take 2 args
		a.	path to starting directory
		b.	name of the output file
	2.	traverse the directory tree
		a.	at each level
			i.	look for files with .mp3 extension
				*	when files found put them into the global array
					-	if outside the bounds of current size of array: lock array -> resize -> unlock array
			ii.	look for subdirectories
				*	explore with new asychronous thread
	3.	cat array of fragments, redirct to output file given as 2nd arg

## Notes
*	compile with '-pthread' flag
*	.mp3 files begin numbering with 0
*	each integer filename is only used once
*	the order files are located in dirs is not necessarily in numeric order
