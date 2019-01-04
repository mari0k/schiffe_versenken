all: schiffe

schiffe: schiffe_versenken.c
	gcc -lncurses schiffe_versenken.c -o schiffe_versenken
