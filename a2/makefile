testfile1 = ../a1-2_cards_csv/cards.csv
testfile2 = ./helpers/cards.csv
testfile3 = ./s.csv

file = search

input_r = < input.txt
input = $(input_r)

stdout = stdout.txt
stderr = stderr.txt
valout = valout.txt

output = 1> $(stdout) 2> $(stderr)  3> $(valout)

make:
	cc -std=gnu11 -Wall -Werror -o parser parser.c
	cc -std=gnu11 -Wall -Werror -o search search.c

cleanx:
	rm -f ./parser
	rm -f ./search

cleanb:
	rm -f ./cards.bin
	rm -f ./index.bin

debug: $(file).c
	cc -std=gnu11 -Wall -Werror -o $(file) $(file).c -g -no-pie

gdb: $(file)
	gdb $(file)

valgrind:
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --log-fd=3 ./$(file) $(input) $(output)

valgrind_gdb:
	valgrind --tool=memcheck --leak-check=full --vgdb-error=0 ./$(file) $(testfile)
