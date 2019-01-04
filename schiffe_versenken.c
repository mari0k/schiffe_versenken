#include <ncurses.h>
//#include <stdlib.h>
//#include <stdio.h>
#include <math.h>
// unistd.h is needed for read() and write() to/from sockets
// alternative use recv() and send() with flag 0
#include <unistd.h>
// includes for tcp sockets
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>


/*
 * game is designed for terminal size 80x24
 */


#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define EMPTY -1
#define SHIP 10
#define SHIP_DAMAGED 20
#define HIT 30
#define MISS 40
#define MISS2 42

const int directionsY[8] = {-1, 0, 1, 0, -1, 1, 1, -1};
const int directionsX[8] = {0, -1, 0, 1, -1, -1, 1, 1};


/*
 * above: some macros and constants
 *
 * below: functions for tcp communication
 */


// read int from buffer
void read_int(unsigned char *buffer, int *pos, int *val) {
    *val = (buffer[*pos + 3] << 24) | (buffer[*pos + 2] << 16) | (buffer[*pos + 1] << 8) | (buffer[*pos]);
    *pos += 4;
}

// write int to Buffer
void write_int(unsigned char *buffer, int *pos, int val) {
    buffer[*pos] = val;
    buffer[*pos + 1] = (val >> 8);
    buffer[*pos + 2] = (val >> 16);
    buffer[*pos + 3] = (val >> 24);
    *pos += 4;
}


// send bombing decision to opponent
void ask_opponent(int socket, void *send_buf, void *recv_buf, int *pos, int y, int x, int *dest) {
    // prepare send
    bzero(send_buf, sizeof(send_buf));
    *pos = 0;
    write_int(send_buf, pos, y);
    write_int(send_buf, pos, x);
    // send
    int n = write(socket, send_buf, *pos);
    if (n < 0) {
        //error("ERROR writing to socket");
    }
    *pos = 0;
    // get response
    bzero(recv_buf, sizeof(recv_buf));
    n = read(socket, recv_buf, 4);
    if(n > 0) {
        // read response
        read_int(recv_buf, pos, dest);
    }
    return;
}

// recv bombing decision from opponent
void wait_opponent(int socket, void *buf, int *pos, int *y, int *x) {
    bzero(buf, sizeof(buf));
    *pos = 0;
    int n = read(socket, buf, 8);
    if(n > 0) {
        // read response
        read_int(buf, pos, y);
        read_int(buf, pos, x);
    }
    return;
}

// reply whether bomb hit or miss to opponent
void reply_opponent(int socket, void *buf, int *pos, int dest) {
    // prepare send
    bzero(buf, sizeof(buf));
    *pos = 0;
    write_int(buf, pos, dest);
    int n = write(socket, buf, *pos);
    if (n < 0) {
        //error("ERROR writing to socket");
    }
    return;
}





/*
 * above: functions for tcp communication
 *
 * below: functions for placing ships and bombing
 */



void do_move(int **area, WINDOW *areaWin, int oldY, int oldX, int newY, int newX, int placing, int ship_id) {
    if (placing) {  // placing contains the shiplength
        // save and display the placement of the ship
        for (int i = 0; i < 4; i++) {   // check all directions
            if (oldY + directionsY[i] * (placing - 1) == newY &&
                oldX + directionsX[i] * (placing - 1) == newX) {
                for (int j = 0; j < placing; j++) {
                    area[oldY + directionsY[i] * j][oldX + directionsX[i] * j] = ship_id;
                    mvwprintw(areaWin, oldY + directionsY[i] * j + 1, (oldX + directionsX[i] * j + 1) * 2, "#");
                }
            }
        }
    }
    // restore old position
    switch (area[oldY][oldX]) {
        case EMPTY:
            mvwprintw(areaWin, oldY + 1, (oldX + 1) * 2, " "); break;
        case 0: case 1: case 2: case 3: case 4:
        case 5: case 6: case 7: case 8: case 9:
            wattron(areaWin, COLOR_PAIR(SHIP));
            mvwprintw(areaWin, oldY + 1, (oldX + 1) * 2, "#"); break;
            wattroff(areaWin, COLOR_PAIR(SHIP));
        case HIT:
            wattron(areaWin, COLOR_PAIR(HIT));
            mvwprintw(areaWin, oldY + 1, (oldX + 1) * 2, "$"); break;
            wattroff(areaWin, COLOR_PAIR(HIT));
        case MISS:
            wattron(areaWin, COLOR_PAIR(MISS));
            mvwprintw(areaWin, oldY + 1, (oldX + 1) * 2, "X"); break;
            wattroff(areaWin, COLOR_PAIR(MISS));
        default:
            break;
    }
    wrefresh(areaWin);
    // highlight new position
    wattron(areaWin, A_REVERSE);
    switch (area[newY][newX]) {
        case EMPTY:
            mvwprintw(areaWin, newY + 1, (newX + 1) * 2, " "); break;
        case 0: case 1: case 2: case 3: case 4:
        case 5: case 6: case 7: case 8: case 9:
            wattron(areaWin, COLOR_PAIR(SHIP));
            mvwprintw(areaWin, newY + 1, (newX + 1) * 2, "#"); break;
            wattroff(areaWin, COLOR_PAIR(SHIP));
        case HIT:
            wattron(areaWin, COLOR_PAIR(HIT));
            mvwprintw(areaWin, newY + 1, (newX + 1) * 2, "$"); break;
            wattroff(areaWin, COLOR_PAIR(HIT));
        case MISS:
            wattron(areaWin, COLOR_PAIR(MISS));
            mvwprintw(areaWin, newY + 1, (newX + 1) * 2, "X"); break;
            wattroff(areaWin, COLOR_PAIR(MISS));
        default:
            break;
    }
    wattroff(areaWin, A_REVERSE);
    wrefresh(areaWin);
    return;
}

int get_move(WINDOW *window, int stepsize, int oldY, int oldX, int *newY, int *newX) {
    while(1) {
        int ch = wgetch(window);
        switch (ch) {
            case KEY_UP:
            case 87:    // W
            case 119:   // w
                *newY = (oldY + 10 - stepsize) % 10; return 0;
            case KEY_LEFT:
            case 65:    // A
            case 97:    // a
                *newX = (oldX + 10 - stepsize) % 10; return 0;
            case KEY_DOWN:
            case 83:    // S
            case 115:   // s
                *newY = (oldY + stepsize) % 10; return 0;
            case KEY_RIGHT:
            case 68:    // D
            case 100:   // d
                *newX = (oldX + stepsize) % 10; return 0;
            case 10:    // ENTER
            case 80:    // P
            case 112:   // p
                return 80;
            case 32:    // SPACE
            case 66:    // B
            case 98:    // b
                return 66;
            case 88:    // X
            case 120:   // x
                return 88;
            case 27:    // ESC
            case 81:    // Q
            case 113:   // q
                return 81;
            default: break;
        }
    }
}

int move_is_feasible(int **area, int oldY, int oldX, int newY, int newX, int placing) {
    if (oldY != newY && oldX != newX) {
        return 0;
    }
    if (oldY == newY && oldX == newX) {
        return 0;
    }
    if (!placing) { // placing contains the shiplength
        return 1;
    }
    // we are in placing mode
    // we need to check all positions the ship would be on
    // and the adjacent positions for other ships
    int matching_direction = -1;
    for (int i = 0; i < 4; i++) {   // check all directions
        if (oldY + directionsY[i] * (placing - 1) == newY &&
            oldX + directionsX[i] * (placing - 1) == newX) {
            for (int j = 0; j < placing; j++) {
                if(area[oldY + directionsY[i] * j][oldX + directionsX[i] * j] != EMPTY) {
                    // ship would be on the same position as another ship
                    return 0;
                }
                for (int l = 0; l < 8; l++) {
                    if (area[MIN(MAX(oldY + directionsY[i] * j + directionsY[l], 0), 9)][MIN(MAX(oldX + directionsX[i] * j + directionsX[l], 0), 9)] != EMPTY) {
                        // ship would be adjacent to another ship
                        return 0;
                    }
                }
            }
            matching_direction = i;
            break;
        }
    }
    // check if ship would be placed in a ring fashion
    // (i.e. leaving at right border and coming in at left border)
    if (matching_direction == -1) {
        return 0;
    }
    // placement of the ship is fine
    return 1;
}



int place_ships(int **area, int *ships, WINDOW *areaWin, WINDOW *infoWin) {
    int move;
    int newY, newX;
    int oldY, oldX;
    oldY = oldX = newY = newX = 0;
    wattron(areaWin, A_REVERSE);
    mvwprintw(areaWin, oldY + 1, (oldX + 1) * 2, " ");
    wattroff(areaWin, A_REVERSE);

    // place ships one by one
    for (int k = 0; k < 10; k++) {
        mvwprintw(infoWin, 1, 1, "placing ship %d of 10: ", k + 1);
        for (int j = 0; j < ships[k]; j++) {
            wprintw(infoWin, "#");
        }
        mvwprintw(infoWin, 2, 3, "select the ships first position");
        wrefresh(infoWin);
        // get feasible position for the first end of the ship
        while (1) {
            newY = oldY;
            newX = oldX;
            move = get_move(areaWin, 1, oldY, oldX, &newY, &newX);
            if (move == 81) {   // user wants to quit game
                return -1;
            }
            if (move == 0 && move_is_feasible(area, oldY, oldX, newY, newX, 0)) {
                do_move(area, areaWin, oldY, oldX, newY, newX, 0, k);
                oldY = newY;
                oldX = newX;
                continue;
            }
            if (move == 80 && oldY == newY && oldX == newX) {
                // anchor ship if there is no adjacent ship
                // and there is at least one feasible direction to place the ship
                int conflict_in_positioning = 0;
                for (int i = 0; i < 8; i++) {
                    if (area[oldY][oldX] != EMPTY) {
                        // ship would be on the same position as another ship
                        conflict_in_positioning = 1;
                        break;
                    }
                    if (area[MIN(MAX(oldY + directionsY[i], 0), 9)][MIN(MAX(oldX + directionsX[i], 0), 9)] != EMPTY) {
                        // ship would be adjacent to another ship
                        conflict_in_positioning = 2;
                        break;
                    }
                }
                if(conflict_in_positioning) {
                    // there was an overlap or an adjacent ship, so skip
                    continue;
                }
                // there is no adjacent ship and no overlap
                // check for a feasible direction now
                int feasible_direction = -1;
                for (int i = 0; i < 4; i++) {
                    if (move_is_feasible(area, oldY, oldX, MIN(MAX(oldY + directionsY[i] * (ships[k] - 1), 0), 9), MIN(MAX(oldX + directionsX[i] * (ships[k] - 1), 0), 9), ships[k])) {
                        // there is a feasible direction to place the ship
                        feasible_direction = i;
                        break;
                    }
                }
                if (feasible_direction != -1) {
                    // there is a feasible direction
                    break;
                }
            }
        }
        // we have a feasible position for the first end of the ship
        // and the user has decided to anchor there
        int anchorY, anchorX;
        anchorY = oldY;
        anchorX = oldX;
        wmove(infoWin, 2, 3);
        wclrtoeol(infoWin);
        box(infoWin, 0, 0);
        mvwprintw(infoWin, 2, 3, "ship has been anchored at (%d, %d)", anchorY, anchorX);
        mvwprintw(infoWin, 3, 3, "now select orientation of the ship");
        wrefresh(infoWin);
        // get feasible position for the second end of the ship
        while(1) {
            oldY = newY;
            oldX = newX;
            newY = anchorY;
            newX = anchorX;
            move = get_move(areaWin, ships[k] - 1, anchorY, anchorX, &newY, &newX);
            if (move == 81) {   // user wants to quit game
                return -1;
            }
            if (move == 88) {   // user wants to re-anchor ship
                k = MIN(MAX(k - 1, -1), 9);
                break;
            }
            if (move == 0 && ((anchorY != newY && anchorX == newX) || (anchorY == newY && anchorX != newX))) {
                // first restore old positions
                if (move_is_feasible(area, anchorY, anchorX, oldY, oldX, ships[k])) {
                    // the old orientation was feasible so we need to do something
                    for (int i = 0; i < 4; i++) {   // find matching direction
                        if (anchorY + directionsY[i] * (ships[k] - 1) == oldY &&
                            anchorX + directionsX[i] * (ships[k] - 1) == oldX) {
                            // direction matches, so restore old placement positions
                            for (int j = 1; j < ships[k]; j++) {
                                mvwprintw(areaWin, anchorY + directionsY[i] * j + 1, (anchorX + directionsX[i] * j + 1) * 2, " ");
                            }
                            break;
                        }
                    }
                }
                if (move_is_feasible(area, anchorY, anchorX, newY, newX, ships[k])) {
                    // ship can be placed in this direction, so highlight new positions
                    wattron(areaWin, A_REVERSE);
                    for (int i = 0; i < 4; i++) {   // find matching direction
                        if (anchorY + directionsY[i] * (ships[k] - 1) == newY &&
                            anchorX + directionsX[i] * (ships[k] - 1) == newX) {
                            // direction matches, so highlight new placement positions
                            for (int j = 1; j < ships[k]; j++) {
                                mvwprintw(areaWin, anchorY + directionsY[i] * j + 1, (anchorX + directionsX[i] * j + 1) * 2, " ");
                            }
                            break;
                        }
                    }
                    wattroff(areaWin, A_REVERSE);
                    oldY = newY;
                    oldX = newX;
                }
            }
            if (move == 80 && ((anchorY != oldY && anchorX == oldX) || (anchorY == oldY && anchorX != oldX))) {
                if (move_is_feasible(area, anchorY, anchorX, oldY, oldX, ships[k])) {
                    do_move(area, areaWin, anchorY, anchorX, oldY, oldX, ships[k], k);
                    break;
                }
            }
            wrefresh(areaWin);
        }

        // erase the information window
        werase(infoWin);
        box(infoWin, 0, 0);
        wrefresh(infoWin);
    }

    mvwprintw(areaWin, oldY + 1, (oldX + 1) * 2, "#");
    wrefresh(areaWin);

    return 0;
}



void update_battlefield(int **area, WINDOW *areaWin, int y, int x, int hit_or_miss, int own_side) {
    if (own_side) {
        if (hit_or_miss) {
            area[y][x] = SHIP_DAMAGED;
            wattron(areaWin, COLOR_PAIR(SHIP_DAMAGED));
            mvwprintw(areaWin, y + 1, (x + 1) * 2, "@");
            wattroff(areaWin, COLOR_PAIR(SHIP_DAMAGED));
        }
        else {
            area[y][x] = MISS;
            wattron(areaWin, COLOR_PAIR(MISS2));
            mvwprintw(areaWin, y + 1, (x + 1) * 2, "x");
            wattroff(areaWin, COLOR_PAIR(MISS2));
        }
    }
    else {
        if (hit_or_miss) {
            area[y][x] = HIT;
            wattron(areaWin, A_REVERSE);
            wattron(areaWin, COLOR_PAIR(HIT));
            mvwprintw(areaWin, y + 1, (x + 1) * 2, "$");
            wattroff(areaWin, COLOR_PAIR(HIT));
            wattroff(areaWin, A_REVERSE);
        }
        else {
            area[y][x] = MISS;
            wattron(areaWin, A_REVERSE);
            wattron(areaWin, COLOR_PAIR(MISS));
            mvwprintw(areaWin, y + 1, (x + 1) * 2, "X");
            wattroff(areaWin, COLOR_PAIR(MISS));
            wattroff(areaWin, A_REVERSE);
        }
    }
    wrefresh(areaWin);
    return;
}

void bomb_impact(int *ships_health, int *ships_length,  int *shipsWin_ships_row,  int *shipsWin_ships_column, int **area, WINDOW *areaWin, WINDOW *shipsWin, WINDOW *infoWin, int destination, int y, int x, int own_bomb) {
    if (destination >= 0 && destination < 10) {
        scroll(infoWin);
        wmove(infoWin, 3, 0);
        wclrtoeol(infoWin);
        switch (own_bomb) {
            case 0: mvwprintw(infoWin, 3, 3, "opponent hit your ship (%d, %d)", y, x); break;
            case 1: mvwprintw(infoWin, 3, 3, "you hit opposing ship"); break;
            default: break;
        }
        if (--ships_health[destination] == 0) {
            if (own_bomb) {
                // you sunk an opposing ship, so remove it from list
                wmove(shipsWin, shipsWin_ships_row[destination], shipsWin_ships_column[destination]);
                for (int i = 0; i < ships_length[destination]; i++) {
                    wprintw(shipsWin, " ");
                }
                wrefresh(shipsWin);
            }
            scroll(infoWin);
            wmove(infoWin, 3, 0);
            wclrtoeol(infoWin);
            switch (own_bomb) {
                case 0: mvwprintw(infoWin, 3, 3, "your ship sunk"); break;
                case 1: mvwprintw(infoWin, 3, 3, "opponents ship sunk"); break;
                default: break;
            }
        }
        update_battlefield(area, areaWin, y, x, 1, (own_bomb + 1) % 2);
    }
    else {
        scroll(infoWin);
        wmove(infoWin, 3, 0);
        wclrtoeol(infoWin);
        switch (own_bomb) {
            case 0: mvwprintw(infoWin, 3, 3, "opponent missed (%d, %d)", y, x); break;
            case 1: mvwprintw(infoWin, 3, 3, "you missed"); break;
            default: break;
        }
        update_battlefield(area, areaWin, y, x, 0, (own_bomb + 1) % 2);
    }
    box(infoWin, 0, 0);
    wrefresh(infoWin);
    return;
}

int bomb_decision(int **area, WINDOW *areaWin, int *y, int *x) {
    int oldY, oldX, newY, newX;
    oldY = newY = *y;
    oldX = newX = *x;
    do_move(area, areaWin, oldY, oldX, newY, newX, 0, 0);
    while(1) {
        newY = oldY;
        newX = oldX;
        int move = get_move(areaWin, 1, oldY, oldX, &newY, &newX);
        if (move == 81) {   // user wants to quit game
            return -1;
        }
        if (move == 0) {    // user wants to move
            do_move(area, areaWin, oldY, oldX, newY, newX, 0, 0);
            oldY = newY;
            oldX = newX;
            continue;
        }
        if (move == 66 && oldY == newY && oldX == newX && area[oldY][oldX] < HIT) {
            // feasible bombing decision
            *y = oldY;
            *x = oldX;
            break;
        }
    }
    return 0;
}

/*
 * above: functions for placing ships and bombing
 *
 * below: main function
 */


int main(int argc, char *argv[]) {

    int port;
    char *server_ip;
    short is_client = 0;
    short is_server = 0;
    short against_computer = 0;

    switch (argc) {
        case 3: port = atoi(argv[2]);
                server_ip = argv[1];
                is_client = 1; break;
        case 2: port = atoi(argv[1]);
                is_server = 1; break;
        default: against_computer = 1; break;
    }

    // something went wrong
    if (is_client + is_server + against_computer != 1) return -1;


    /*
     * above: processing input arguments
     *
     * below: graphical output (most stays unchanged throughout game)
     */


    // initialize ncurses
    initscr();

    // suppress keyboard outputs
    noecho();
    // prevent game crash when using arrow keys
    keypad(stdscr, true);
    // hide cursor
    curs_set(0);

    // enable colors
    start_color();
    // define colors
    init_pair(SHIP, COLOR_WHITE, COLOR_BLACK);
    init_pair(SHIP_DAMAGED, COLOR_RED, COLOR_BLACK);
    init_pair(HIT, COLOR_GREEN, COLOR_BLACK);
    init_pair(MISS, COLOR_WHITE, COLOR_BLACK);
    init_pair(MISS2, COLOR_YELLOW, COLOR_BLACK);


    // ascii art game title
    WINDOW *titleWin = newwin(9, 40, 0, 40);

    mvwprintw(titleWin, 0, 0, "    ___     _    _  __  __          beta");
    mvwprintw(titleWin, 1, 0, "   / __| __| |_ (_)/ _|/ _|___      v0.1");
    mvwprintw(titleWin, 2, 0, "   \\__ \\/ _| \' \\| |  _|  _/ -_)");
    mvwprintw(titleWin, 3, 0, "__ |___/\\__|_||_|_|_| |_| \\___|");
    mvwprintw(titleWin, 4, 0, "\\ \\ / /__ _ _ ___ ___ _ _ | |_____ _ _");
    mvwprintw(titleWin, 5, 0, " \\ V / -_) \'_(_-</ -_) \' \\| / / -_) \' \\");
    mvwprintw(titleWin, 6, 0, "  \\_/\\___|_| /__/\\___|_||_|_\\_\\___|_||_|");

    if (is_client) mvwprintw(titleWin, 8, 0, "Client - server at %s:%d", server_ip, port);
    if (is_server) mvwprintw(titleWin, 8, 0, "Server - port %d", port);
    if (against_computer) mvwprintw(titleWin, 8, 0, "playing against the computer (N.A.)");


    refresh();
    wrefresh(titleWin);


    // ships
    WINDOW *shipsWin = newwin(2, 40, 14, 40);

    mvwprintw(shipsWin, 0, 0, "SHIPS:");
    mvwprintw(shipsWin, 0, 10, "#####");
    mvwprintw(shipsWin, 1, 10, "####");
    mvwprintw(shipsWin, 1, 18, "####");
    mvwprintw(shipsWin, 0, 19, "###");
    mvwprintw(shipsWin, 0, 25, "###");
    mvwprintw(shipsWin, 1, 25, "###");
    mvwprintw(shipsWin, 0, 31, "##");
    mvwprintw(shipsWin, 1, 31, "##");
    mvwprintw(shipsWin, 0, 36, "##");
    mvwprintw(shipsWin, 1, 36, "##");

    refresh();
    wrefresh(shipsWin);


    // controls
    WINDOW *controlsWin = newwin(6, 22, 17, 58);

    mvwprintw(controlsWin, 0, 0, "CONTROLS:");
    mvwprintw(controlsWin, 1, 0, "move  ->  WASD or");
    mvwprintw(controlsWin, 2, 0, "          ARROW KEYS");
    mvwprintw(controlsWin, 3, 0, "bomb  ->  B or SPACE");
    mvwprintw(controlsWin, 4, 0, "place ->  P or ENTER");
    mvwprintw(controlsWin, 5, 0, "quit  ->  Q or ESC");

    refresh();
    wrefresh(controlsWin);


    // legend
    WINDOW *legendWin = newwin(6, 23, 17, 29);

    mvwprintw(legendWin, 0, 0, "LEGEND:");
    wattron(legendWin, A_REVERSE);
    mvwprintw(legendWin, 1, 0, " ");
    wattroff(legendWin, A_REVERSE);
    mvwprintw(legendWin, 1, 3, "->  current position");
    wattron(legendWin, COLOR_PAIR(SHIP));
    mvwprintw(legendWin, 2, 0, "#");
    wattroff(legendWin, COLOR_PAIR(SHIP));
    mvwprintw(legendWin, 2, 3, "->  ship");
    wattron(legendWin, COLOR_PAIR(SHIP_DAMAGED));
    mvwprintw(legendWin, 3, 0, "@");
    wattroff(legendWin, COLOR_PAIR(SHIP_DAMAGED));
    mvwprintw(legendWin, 3, 3, "->  damaged ship");
    wattron(legendWin, COLOR_PAIR(HIT));
    mvwprintw(legendWin, 4, 0, "$");
    wattroff(legendWin, COLOR_PAIR(HIT));
    mvwprintw(legendWin, 4, 3, "->  successful hit");
    wattron(legendWin, COLOR_PAIR(MISS));
    mvwprintw(legendWin, 5, 0, "X");
    wattroff(legendWin, COLOR_PAIR(MISS));
    mvwprintw(legendWin, 5, 3, "->  unsuccessful hit");

    refresh();
    wrefresh(legendWin);



    // information window
    WINDOW *informationWin = newwin(5, 40, 9, 39);

    box(informationWin, 0, 0);
    scrollok(informationWin, 1);

    refresh();
    wrefresh(informationWin);



    // draw Areas
    WINDOW *opponentAreaWin = newwin(12, 23, 0, 0);
    WINDOW *ownAreaWin = newwin(12, 23, 12, 0);

    box(opponentAreaWin, 0, 0);
    box(ownAreaWin, 0, 0);

    mvprintw(0, 24, "OPPONENT");
    mvprintw(12, 24, "YOU");

    refresh();
    wrefresh(opponentAreaWin);
    wrefresh(ownAreaWin);

    // to use arrow keys for moving around
    keypad(ownAreaWin, true);
    keypad(opponentAreaWin, true);



    /*
     * above: graphical output (most stays unchanged througout game)
     *
     * below: tcp socket stuff
     */

     // create socket (this is the same for server and client)
     int sock = socket(AF_INET, SOCK_STREAM, 0);
     if (sock == -1) {
         //perror ("socket()");
         mvwprintw(informationWin, 3, 2, "Error on socket()");
         wrefresh(informationWin);
     }

     struct sockaddr_in server_addr;

     // server stuff
     struct sockaddr_in remote_host;
     socklen_t sin_size = sizeof (remote_host);
     int sock2;

     if (is_server) {
         //bzero((char *) &server_addr, sizeof(server_addr));
         // bind port to socket
         server_addr.sin_family = AF_INET;
         server_addr.sin_port = htons(port);
         server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
         if (bind(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
             //perror ("bind()");
             mvwprintw(informationWin, 3, 2, "Error on bind()");
             wrefresh(informationWin);
         }
         // listen for a client
         if (listen(sock, 5) == -1) {
             //perror ("listen()");
             mvwprintw(informationWin, 3, 2, "Error on listen()");
             wrefresh(informationWin);
         }
         // accept client
         sock2 = accept(sock, (struct sockaddr *) &remote_host, &sin_size);
         if (sock2 == -1) {
             //perror ("accept()");
             mvwprintw(informationWin, 3, 2, "Error on accept()");
             wrefresh(informationWin);
         }
         else {
             wprintw(titleWin, " (client connected)");
             wrefresh(titleWin);
         }
     }

     if (is_client) {
         // set server adress information
         server_addr.sin_family = AF_INET;
         server_addr.sin_port = htons(port);
         server_addr.sin_addr.s_addr = inet_addr(server_ip);
         // connect to server
         if (connect(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
             //perror ("connect()");
             mvwprintw(informationWin, 3, 2, "Error on connect()");
             wrefresh(informationWin);
         }
     }

     int pos;
     unsigned char send_buffer[13];
     unsigned char recv_buffer[13];



    /*
     * above: tcp socket stuff
     *
     * below: data management (and the really interesting code)
     */


    // create 2-dim arrays holding information about battlefields
    int opponentAreaData[100];
    int ownAreaData[100];
    for (int i = 0; i < 100; i++) {
        opponentAreaData[i] = EMPTY;
        ownAreaData[i] = EMPTY;
    }
    int *opponentArea[10];
    int *ownArea[10];
    for (int i = 0; i < 10; i++) {
        opponentArea[i] = &(opponentAreaData[i * 10]);
        ownArea[i] = &(ownAreaData[i * 10]);
    }

    // create several arrays holding information about ships
    int ships_length[10] = { 5, 4, 4, 3, 3, 3, 2, 2, 2, 2 };
    int opponent_ships_health[10] = { 5, 4, 4, 3, 3, 3, 2, 2, 2, 2 };
    int own_ships_health[10] = { 5, 4, 4, 3, 3, 3, 2, 2, 2, 2 };
    int shipsWin_ships_row[10] = { 0, 1, 1, 0, 0, 1, 0, 1, 0, 1 };
    int shipsWin_ships_column[10] = { 10, 10, 18, 19, 25, 25, 31, 31, 36, 36 };

    // player health
    int sum_opponent_ships_health = 30;
    int sum_own_ships_health = 30;


    // place own ships one by one
    if(!against_computer) {
        if(place_ships(ownArea, ships_length, ownAreaWin, informationWin) == -1) {
            // user wants to exit game
            // end ncurses
            endwin();
            return 0;
        }
    }
    else {
        // we are in playing against computer mode
        // this is not yet implemented, so do not place ships
        // only option is to quit
        int ch;
        while(1) {
            ch = getch();
            if (ch == 81 || ch == 113 || ch == 27) {    // Q, q, ESC
                // end ncurses
                endwin();
                return 0;
            }
        }
    }




    // ships have been placed
    // now the bombing phase begins
    int ownY, ownX, opponentY, opponentX;
    ownY = ownX = opponentY = opponentX = 0;
    while(1) {
        // first check if one player has no ships left
        if (sum_opponent_ships_health == 0 && sum_own_ships_health == 0) {    // no ship is alive
            werase(informationWin);
            box(informationWin, 0, 0);
            mvwprintw(informationWin, 2, 2, "all ships sunk -> draw");
            wrefresh(informationWin);
            break;
        }
        if (sum_opponent_ships_health == 0) {    // no opposing ship is alive
            werase(informationWin);
            box(informationWin, 0, 0);
            mvwprintw(informationWin, 2, 2, "all opposing ships sunk -> you win");
            wrefresh(informationWin);
            break;
        }
        if (sum_own_ships_health == 0) {    // no own ship is alive
            werase(informationWin);
            box(informationWin, 0, 0);
            mvwprintw(informationWin, 2, 2, "all your ships sunk -> you lost");
            wrefresh(informationWin);
            break;
        }

        // both player have at least one ship left
        if (is_server) {
            scroll(informationWin);
            wmove(informationWin, 3, 0);
            wclrtoeol(informationWin);
            mvwprintw(informationWin, 3, 1, "choose target location");
            box(informationWin, 0, 0);
            wrefresh(informationWin);
            // decide where to bomb
            if (bomb_decision(opponentArea, opponentAreaWin, &ownY, &ownX) == -1) {
                // user wants to exit game
                // end ncurses
                endwin();
                return 0;
            }
            // ask client if bomb hit a ship
            int destination = -2;
            ask_opponent(sock2, send_buffer, recv_buffer, &pos, ownY, ownX, &destination);
            if (destination >= 0 && destination < 10) sum_opponent_ships_health--;
            // update opponents battlefield area
            bomb_impact(opponent_ships_health, ships_length, shipsWin_ships_row, shipsWin_ships_column, opponentArea, opponentAreaWin, shipsWin, informationWin, destination, ownY, ownX, 1);

            scroll(informationWin);
            wmove(informationWin, 3, 0);
            wclrtoeol(informationWin);
            mvwprintw(informationWin, 3, 1, "opponent chooses target");
            box(informationWin, 0, 0);
            wrefresh(informationWin);
            // wait for clients bombing decision
            wait_opponent(sock2, recv_buffer, &pos, &opponentY, &opponentX);
            // check if this is a hit
            destination = ownArea[opponentY][opponentX];
            if (destination >= 0 && destination < 10) sum_own_ships_health--;
            // respond hit to client
            reply_opponent(sock2, send_buffer, &pos, destination);
            // update own battlefield area
            bomb_impact(own_ships_health, ships_length, shipsWin_ships_row, shipsWin_ships_column, ownArea, ownAreaWin, shipsWin, informationWin, destination, opponentY, opponentX, 0);


            continue;
        }
        if (is_client) {
            scroll(informationWin);
            wmove(informationWin, 3, 0);
            wclrtoeol(informationWin);
            mvwprintw(informationWin, 3, 1, "opponent chooses target");
            box(informationWin, 0, 0);
            wrefresh(informationWin);
            // wait for servers bombing decision
            int destination;
            wait_opponent(sock, recv_buffer, &pos, &opponentY, &opponentX);
            // check if this is a hit
            destination = ownArea[opponentY][opponentX];
            if (destination >= 0 && destination < 10) sum_own_ships_health--;
            // respond hit to server
            reply_opponent(sock, send_buffer, &pos, destination);
            // update own battlefield area
            bomb_impact(own_ships_health, ships_length, shipsWin_ships_row, shipsWin_ships_column, ownArea, ownAreaWin, shipsWin, informationWin, destination, opponentY, opponentX, 0);


            scroll(informationWin);
            wmove(informationWin, 3, 0);
            wclrtoeol(informationWin);
            mvwprintw(informationWin, 3, 1, "choose target location");
            box(informationWin, 0, 0);
            wrefresh(informationWin);
            // decide where to bomb
            if (bomb_decision(opponentArea, opponentAreaWin, &ownY, &ownX) == -1) {
                // user wants to exit game
                // end ncurses
                endwin();
                return 0;
            }
            // ask server if bomb hit a ship
            destination = -2;
            ask_opponent(sock, send_buffer, recv_buffer, &pos, ownY, ownX, &destination);
            if (destination >= 0 && destination < 10) sum_opponent_ships_health--;
            // update opponents battlefield area
            bomb_impact(opponent_ships_health, ships_length, shipsWin_ships_row, shipsWin_ships_column, opponentArea, opponentAreaWin, shipsWin, informationWin, destination, ownY, ownX, 1);


            continue;
        }
        if (against_computer) {
            continue;
        }
    }










    int ch;
    while(1) {
        ch = getch();
        if (ch == 81 || ch == 113 || ch == 27) {    // Q, q, ESC
            break;
        }
    }


    // close cocket
    if (is_server) close(sock2);
    close(sock);

    // end ncurses
    endwin();

    return 0;
}
