make:
	cc -std=gnu11 -Wall -Werror -o parser parser.c

clean:
	rm -f ./parser
	rm -f ./s_cardbin

debug: parser.c
	cc -std=gnu11 -Wall -Werror -o parser parser.c -g

gdb: parser
	gdb parser

valgrind:
	valgrind --tool=memcheck --leak-check=full ./parser ../a1-2_cards_csv/s.csv

valgrind_gdb:
	valgrind --tool=memcheck --leak-check=full --vgdb-error=0 ./parser ../a1-2_cards_csv/s.csv
