lis : lis.c
	cc -Wall lis.c types.c mpc/mpc.c -o lis -std=c99 -lreadline --debug

clean :
	rm lis
