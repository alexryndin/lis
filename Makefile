lis : parser.c
	cc -Wall parser.c mpc/mpc.c -o parser -std=c99 -lreadline

clean :
	rm parsing
