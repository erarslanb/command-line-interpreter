all: bilshell
cost: bilshell.c
	gcc -o bilshell bilshell.c
clean:
	rm -fr bilshell bilshell.o *~
