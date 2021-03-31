testfile = ../a1-2_cards_csv/cards.csv

file = search

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
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --log-fd=3 ./$(file) < input.txt 1> stdout 2> stderr 3> valout

valgrind_gdb:
	valgrind --tool=memcheck --leak-check=full --vgdb-error=0 ./$(file) $(testfile)
