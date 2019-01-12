all: sea_battle

sea_battle: main.o ui.o tcp.o
	gcc -o sea_battle main.o ui.o tcp.o -lncurses


main.o: main.c
	gcc -c main.c

ui.o: ui.c
	gcc -c ui.c

tcp.o: tcp.c
	gcc -c tcp.c
