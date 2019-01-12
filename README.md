# Schiffe Versenken

This program uses `ncurses` for graphical output. When compiling make sure you have `ncurses` installed.

### Multiplayer

* Communication over TCP
* one Player needs to act as a server
```
./sea_battle 51234
```
where `51234` is the Port number (between 49152 and 65535)
* the other Player acts as a client
```
./sea_battle "123.456.789.123" 51234
```
where `123.456.789.123` is the IP address of the server and `51234` the Port number the server opened

### Singleplayer

not yet implemented
```
./sea_battle
```


### Note

This is just a fun little project. There is no guarantee that everything will function 100%.
