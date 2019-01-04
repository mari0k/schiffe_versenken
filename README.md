# Schiffe Versenken

* this program uses ncurses for graphical output
* compile using 'gcc' with option '-lncurses'

### Multiplayer

* Communication over TCP
* one Player needs to act as a server
'./schiffe_versenken 51234' where '51234' is the Port number (smaller than 65535 and preferably bigger than 49152)
* the other Player acts as a client
'./schiffe_versenken "123.456.789.123" 51234' where '123.456.789.123' is the IP address of the server and '51234' the Port number the server opened

### Singleplayer

not yet implemented
'./schiffe_versenken'


### Note

code is pretty dirty...
