BINARIES?=asuna

asuna: asuna.c 
	cc -g3 asuna.c -o asuna

my_app: my_app.c	my_lib.c
	cc -c my_app.c -o my_app.o && cc -c my_lib.c -o my_lib.o && cc my_app.o my_lib.o -o my_app  

clean: 
	rm -f $(BINARIES)