cc = cc
CFLAGS = -std=gnu11 -Wall -Werror -pthread -o

file = defrag

testdir = ./starters/dirs
testout = ./debug/myout.mp3

input_r = < debug/input.txt
input = $(input_r)

stdout = ./debug/stdout.txt
stderr = ./debug/stderr.txt
valout = ./debug/valout.txt

output = 1> $(stdout) 2> $(stderr)

make:
	$(cc) $(CFLAGS) $(file) $(file).c

clean:
	rm -f $(file)
	rm -f $(testout)

debug: clean
	$(cc) -g -no-pie -fsanitize=thread $(CFLAGS) $(file) $(file).c

gdb: debug
	gdb $(file)

valgrind: debug
	rm -f $(stdout) $(stderr) $(valout)
	valgrind --tool=memcheck --leak-check=full --log-file="$(valout)" ./$(file) $(testdir) $(testout) $(output)

valgrind_gdb:
	valgrind --tool=memcheck --leak-check=full --vgdb-error=0 ./$(file) $(input)
