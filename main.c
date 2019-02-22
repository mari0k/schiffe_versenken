#include <ncurses.h>
#include <stdlib.h> // for atoi();

#include "tcp.h"
#include "ui.h"
#include "structs.h"


/*
 * game is desigend for terminal size 132x43
 */


#define EMPTY 0
#define SHIP 10
#define SHIP_DAMAGED 20
#define HIT 30
#define MISS 40
#define SUNK 50

#define ALTERNATE 1
#define BURST 2
#define UNTILMISS 3


void end_game_with_confirm(int role, int sock, int sock2) {
    // end game only when Q or ESC is pressed
    int ch;
    while(1) {
        ch = getch();
        if (ch == 81 || ch == 113 || ch == 27) {    // Q, q, ESC
            break;
        }
    }
    // close sockets
    switch (role) {
        case 0: close_socket(sock2);
        case 1: close_socket(sock); clean_up();
        default: break;
    }
    // end ncurses
    endwin();
    return;
}

void end_game_without_confirm(int role, int sock, int sock2) {
    // close sockets
    switch (role) {
        case 0: close_socket(sock2);
        case 1: close_socket(sock); clean_up();
        default: break;
    }
    // end ncurses
    endwin();
    return;
}

int is_there_a_winner(WINDOW *window, int opp_health, int own_health) {
    // check if one player has no ships left
    if (opp_health == 0) {    // no opposing ship is alive
        werase(window);
        box(window, 0, 0);
        mvwprintw(window, 4, 12, "all opposing ships sunk -> you win");
        wrefresh(window);
        return 1;
    }
    if (own_health == 0) {    // no own ship is alive
        werase(window);
        box(window, 0, 0);
        mvwprintw(window, 4, 12, "all your ships sunk -> you lost");
        wrefresh(window);
        return 1;
    }
    return 0;
}


int main(int argc, char *argv[]) {

    int mode = ALTERNATE;
    /*
     *  process input arguments
     */
    int port;
    char *server_ip;
    int role = -1;

    switch (argc) {
        case 3: port = atoi(argv[2]);
                server_ip = argv[1];
                role = 1; break;    // Client
        case 2: port = atoi(argv[1]);
                role = 0; break;    // Server
        default: role = 2; break;   // Player vs. AI
    }

    // something went wrong
    if (role < 0 || role > 2) {
        printf("ERROR something went wrong with the input arguments!\n");
        return -1;
    }
    if (role != 2 && (port < 49152 || port > 65535)) {
        printf("ERROR port must be between 49152 and 65535\n");
        return -1;
    }
    if (role == 2) {
        printf("ERROR Playing vs. AI is not yet implemented!\n");
        return -1;
    }


    printf("Please make sure your Terminal size is at least 132x43! Continue (Y/n)?");
    {
        char ch;
        do {
            ch = getchar();
        } while(ch != 10 && ch != 89 && ch != 121 && ch != 78 && ch != 110);
        if (ch == 78 || ch == 110) return -1;
    }



    /*
     *  do ncurses stuff
     */
    // initialize ncurses
    initscr();
    refresh();

    // suppress keyboard outputs
    noecho();
    // prevent game crash when using arrow keys
    keypad(stdscr, true);
    // hide cursor
    curs_set(0);

    // enable colors
    start_color();
    // define colors
    init_pair(SHIP, COLOR_YELLOW, COLOR_BLACK);
    init_pair(SHIP_DAMAGED, COLOR_RED, COLOR_BLACK);
    init_pair(HIT, COLOR_GREEN, COLOR_BLACK);
    init_pair(MISS, COLOR_WHITE, COLOR_BLACK);
    // set background color to black
    bkgd(COLOR_PAIR(MISS));




    // ascii art welcome message
    WINDOW *welcomeWin = newwin(3, 31, 3, 26);
    wbkgd(welcomeWin, COLOR_PAIR(MISS));
    refresh();
    draw_welcome(welcomeWin);

    // ascii art game title
    WINDOW *titleWin = newwin(6, 66, 0, 65);
    wbkgd(titleWin, COLOR_PAIR(MISS));
    refresh();
    draw_title(titleWin);

    // ask the player for his name
    char own_name[22];
    char opp_name[22] = "AI";
    ask_for_name(own_name);

    // print role
    if (role == 0) server_ip = get_own_ip();
    print_role(role, server_ip, port);
    refresh();


    /*
     *  decide game mode (alternating, bursts, until miss, ...)
     */
    if (role == 0 || role == 2) {   // Server or Playing vs. AI
        mode = select_game_mode();
    }



    /*
     *  create sockets and establish connection if playing multiplayer
     */
    int socket, socket_server_only;
    unsigned char buffer[22];
    if (role < 2) {
        if (role == 0) {    // server
            socket_server_only = create_socket();
            bind_socket_to_port(socket_server_only, port);
            socket = listen_and_accept_client(socket_server_only);
            if (socket != -1) tcp_msg("(Client connected)");
            // exchange names
            if (send_name(socket, own_name) < 0) {
                end_game_with_confirm(role, socket, socket_server_only);
                return -1;
            }
            if (recv_name(socket, opp_name) < 0) {
                end_game_with_confirm(role, socket, socket_server_only);
                return -1;
            }
            // tell client the game mode
            if (send_int(socket, buffer, mode) < 0) {
                end_game_with_confirm(role, socket, socket_server_only);
                return -1;
            }
        }
        if (role == 1) {    // client
            socket = create_socket();
            if (connect_to_server(socket, server_ip, port) != -1) tcp_msg("(Connected to Server)");
            // exchange names
            if (recv_name(socket, opp_name) < 0) {
                end_game_with_confirm(role, socket, socket_server_only);
                return -1;
            }
            if (send_name(socket, own_name) < 0) {
                end_game_with_confirm(role, socket, socket_server_only);
                return -1;
            }
            // receive game mode
            if (recv_int(socket, buffer, &mode) < 0) {
                end_game_with_confirm(role, socket, socket_server_only);
                return -1;
            }
        }
    }



    // remove welcome message
    werase(welcomeWin);
    wrefresh(welcomeWin);
    delwin(welcomeWin);

    // print Rules
    WINDOW *rulesWin = newwin(14, 41, 28, 3);
    wbkgd(rulesWin, COLOR_PAIR(MISS));
    refresh();
    print_rules(rulesWin, mode);

    // print Controls
    WINDOW *controlsWin = newwin(7, 24, 25, 100);
    wbkgd(controlsWin, COLOR_PAIR(MISS));
    refresh();
    print_controls(controlsWin);

    // print Legend
    WINDOW *legendWin = newwin(7, 25, 34, 100);
    wbkgd(legendWin, COLOR_PAIR(MISS));
    refresh();
    print_legend(legendWin);

    // print Ships
    WINDOW *shipsWin = newwin(9, 7, 5, 50);
    wbkgd(shipsWin, COLOR_PAIR(MISS));
    refresh();
    print_ships(shipsWin);


    // information window
    WINDOW *informationWin = newwin(10, 66, 9, 65);
    wbkgd(informationWin, COLOR_PAIR(MISS));
    refresh();
    box(informationWin, 0, 0);
    wrefresh(informationWin);
    scrollok(informationWin, 1);




    // draw battlefields
    WINDOW *oppFieldWin = newwin(21, 41, 4, 3);
    WINDOW *ownFieldWin = newwin(21, 41, 22, 51);
    wbkgd(oppFieldWin, COLOR_PAIR(MISS));
    wbkgd(ownFieldWin, COLOR_PAIR(MISS));
    refresh();
    draw_field_labels();
    draw_field_grid(oppFieldWin);
    draw_field_grid(ownFieldWin);

    // to use arrow keys for moving around the battlefields
    keypad(oppFieldWin, true);
    keypad(ownFieldWin, true);

    // print names above the battlefields
    mvprintw(1, 5, opp_name);
    move(2, 4); addch(ACS_S1); addch(ACS_S1);
    for (int i = 0; i < 22; i++) {
        if (opp_name[i] == '\0') break;
        addch(ACS_S1);
    }
    mvprintw(19, 53, own_name);
    move(20, 52); addch(ACS_S1); addch(ACS_S1);
    for (int i = 0; i < 22; i++) {
        if (own_name[i] == '\0') break;
        addch(ACS_S1);
    }
    refresh();


    // create 2-dim arrays holding information about battlefields
    int oppFieldData[100] = { 0 };
    int ownFieldData[100] = { 0 };
    int *oppField[10];
    int *ownField[10];
    for (int i = 0; i < 10; i++) {
        oppField[i] = &(oppFieldData[i * 10]);
        ownField[i] = &(ownFieldData[i * 10]);
    }

    // create array holding the own ships
    struct ship own_ships[7];
    own_ships[0] = (struct ship) { .id = 1, .length = 5, .health = 5 };
    own_ships[1] = (struct ship) { .id = 2, .length = 4, .health = 4 };
    own_ships[2] = (struct ship) { .id = 3, .length = 3, .health = 3 };
    own_ships[3] = (struct ship) { .id = 4, .length = 3, .health = 3 };
    own_ships[4] = (struct ship) { .id = 5, .length = 2, .health = 2 };
    own_ships[5] = (struct ship) { .id = 6, .length = 2, .health = 2 };
    own_ships[6] = (struct ship) { .id = 7, .length = 2, .health = 2 };

    // track the players healths
    int opp_health, own_health;
    opp_health = own_health = 21;

    /*
     *  place ships
     */
    struct move move = { .old = (struct coord) { .y = 0, .x = 0 }, .new = (struct coord) { .y = 0, .x = 0 } };

    // place ships one by one
    for (int k = 0; k < 7; k++) {
        highlight_position(ownFieldWin, &(move.old), ownField, 1);
        int res;
        res = anchor_ship(ownFieldWin, ownField, informationWin, &(own_ships[k]), &move);
        if (res != -1) {
            struct coord anchor = { .y = move.old.y, .x = move.old.x };
            res = place_ship(ownFieldWin, ownField, informationWin, &(own_ships[k]), &anchor);
            // set move to the stern position of the ship
            move.old.y = (own_ships[k]).stern_coord.y;
            move.old.x = (own_ships[k]).stern_coord.x;
            move.new.y = (own_ships[k]).stern_coord.y;
            move.new.x = (own_ships[k]).stern_coord.x;
        }
        if(res == -1) {   // user wants to quit
            end_game_without_confirm(role, socket, socket_server_only);
            return -1;
        }
    }

    // tell opponent, that all ships have been placed
    if (role < 2) {
        mvwprintw(informationWin, 3, 8, "Waiting for %.8s to place all his ships...", opp_name);
        wrefresh(informationWin);
        int opp_msg = 0;
        if (role == 0) {    // server
            if (send_int(socket, buffer, 1) < 0) {
                end_game_with_confirm(role, socket, socket_server_only);
                return -1;
            }
            if (recv_int(socket, buffer, &opp_msg) < 0) {
                end_game_with_confirm(role, socket, socket_server_only);
                return -1;
            }
        }
        if (role == 1) {    // client
            if (recv_int(socket, buffer, &opp_msg) < 0) {
                end_game_with_confirm(role, socket, socket_server_only);
                return -1;
            }
            if (send_int(socket, buffer, 1) < 0) {
                end_game_with_confirm(role, socket, socket_server_only);
                return -1;
            }
        }
        if (opp_msg != 1) { // opponent quit
            wmove(informationWin, 3, 8);
            wclrtoeol(informationWin);
            box(informationWin, 0, 0);
            mvwprintw(informationWin, 3, 8, "%.8s has quit! Press ANY KEY to quit.", opp_name);
            wrefresh(informationWin);
            // end game only when Q or ESC is pressed
            getch();
            end_game_without_confirm(role, socket, socket_server_only);
            return 0;
        }
    }

    // opponent placed all his ships as well
    // determine who will go first
    int own_turn = 3;
    if (role < 2) {
        int own_choice = heads_or_tails(role, opp_name);
        int opp_choice;
        if (role == 0) {    // server
            if (send_int(socket, buffer, own_choice) < 0) {
                end_game_with_confirm(role, socket, socket_server_only);
                return -1;
            }
            if (recv_int(socket, buffer, &opp_choice) < 0) {
                end_game_with_confirm(role, socket, socket_server_only);
                return -1;
            }
            if (own_choice != opp_choice) own_turn = -3;
        }
        if (role == 1) {    // client
            if (recv_int(socket, buffer, &opp_choice) < 0) {
                end_game_with_confirm(role, socket, socket_server_only);
                return -1;
            }
            if (send_int(socket, buffer, own_choice) < 0) {
                end_game_with_confirm(role, socket, socket_server_only);
                return -1;
            }
            if (own_choice == opp_choice) own_turn = -3;
        }
    }


    /*
     *  bomb ships
     */
    werase(informationWin);
    box(informationWin, 0, 0);
    wrefresh(informationWin);
    move.old.y = 0;
    move.old.x = 0;
    move.new.y = 0;
    move.new.x = 0;
    move.type = 0;
    int miss_hit_or_sunk;
    int hit_ship_id;
    struct coord old_opp_bomb = { .y = 0, .x = 0 };
    struct coord new_opp_bomb = { .y = 0, .x = 0 };
    int res;
    while (1) {
        while(own_turn > 0) {
            switch (mode) {
                case ALTERNATE: own_turn = -3; break;
                case BURST: if (own_turn > 1) own_turn--;
                            else own_turn = -3; break;
                default: break;
            }
            print_information(informationWin, 6, "It is YOUR turn! Choose target location.");
            if (mode == BURST) {
                mvwprintw(informationWin, 7, 47, "(bomb %d/3)", (own_turn > 0)?(3 - own_turn):3);
                wrefresh(informationWin);
            }
            // decide where to bomb
            res = decide_bombing_location(oppFieldWin, oppField, informationWin, &move);
            if (res == -1) break;
            // tell the opponent where your bomb is heading to
            if (send_coord(socket, buffer, &(move.old)) < 0) {
                end_game_with_confirm(role, socket, socket_server_only);
                return -1;
            }
            // receive information if bomb hit something
            if (recv_int(socket, buffer, &miss_hit_or_sunk) < 0) {
                end_game_with_confirm(role, socket, socket_server_only);
                return -1;
            }
            if (miss_hit_or_sunk == MISS) {
                oppField[move.old.y][move.old.x] = MISS;
                if (mode == UNTILMISS) own_turn = -3;
                print_information(informationWin, 8, "Your bomb MISSED.");
                highlight_position(oppFieldWin, (struct coord *) &(move.old), oppField, 0);
                break;
            }
            print_information(informationWin, 8, "Your bomb HIT.");
            if (recv_int(socket, buffer, &hit_ship_id) < 0) {
                end_game_with_confirm(role, socket, socket_server_only);
                return -1;
            }
            switch (miss_hit_or_sunk) {
                case SUNK:
                    mvwprintw(shipsWin, hit_ship_id, 1, "     ");
                    wrefresh(shipsWin);
                    print_information(informationWin, 8, "The opposing ship SUNK.");
                    // TODO: mark all positions around sunk ship as MISS
                case HIT:
                    oppField[move.old.y][move.old.x] = HIT;
                    highlight_position(oppFieldWin, (struct coord *) &(move.old), oppField, 0);
                default: break;
            }
            opp_health--;
            if(is_there_a_winner(informationWin, opp_health, own_health)) break;
        }
        if (res == -1) break;
        if(is_there_a_winner(informationWin, opp_health, own_health)) break;
        while(own_turn < 0) {
            switch (mode) {
                case ALTERNATE: own_turn = 3; break;
                case BURST: if (own_turn < -1) own_turn++;
                            else own_turn = 3; break;
                default: break;
            }
            old_opp_bomb.y = new_opp_bomb.y;
            old_opp_bomb.x = new_opp_bomb.x;
            print_information(informationWin, 6, "It is your opponents turn. Waiting for his decision...");
            // receive opponents bombs target location
            if (recv_coord(socket, buffer, &new_opp_bomb) < 0) {
                end_game_with_confirm(role, socket, socket_server_only);
                return -1;
            }
            scroll(informationWin);
            wmove(informationWin, 8, 0);
            wclrtoeol(informationWin);
            mvwprintw(informationWin, 7, 8, "%ss bomb headed towards %c%d.", opp_name, y_name[new_opp_bomb.y], x_name[new_opp_bomb.x]);
            box(informationWin, 0, 0);
            wrefresh(informationWin);
            // check if opponents bomb hit something
            int i = ownField[new_opp_bomb.y][new_opp_bomb.x];
            print_position(ownFieldWin, &old_opp_bomb, ownField, 1);
            switch (i) {
                case EMPTY:
                    print_information(informationWin, 8, "It MISSED.");
                    if (send_int(socket, buffer, MISS) < 0) {
                        end_game_with_confirm(role, socket, socket_server_only);
                        return -1;
                    }
                    ownField[new_opp_bomb.y][new_opp_bomb.x] = MISS;
                    highlight_position(ownFieldWin, &new_opp_bomb, ownField, 1);
                    if (mode == UNTILMISS) own_turn = 3;
                    break;
                case 1: case 2: case 3: case 4: case 5: case 6: case 7:
                    print_information(informationWin, 8, "Your ship got HIT.");
                    if ((own_ships[i - 1]).health-- == 1) {
                        if (send_int(socket, buffer, SUNK) < 0) {
                            end_game_with_confirm(role, socket, socket_server_only);
                            return -1;
                        }
                        print_information(informationWin, 8, "Your ship SUNK.");
                    }
                    else {
                        if (send_int(socket, buffer, HIT) < 0) {
                            end_game_with_confirm(role, socket, socket_server_only);
                            return -1;
                        }
                    }
                    if (send_int(socket, buffer, (own_ships[i]).id) < 0) {
                        end_game_with_confirm(role, socket, socket_server_only);
                        return -1;
                    }
                    ownField[new_opp_bomb.y][new_opp_bomb.x] = SHIP_DAMAGED;
                    highlight_position(ownFieldWin, &new_opp_bomb, ownField, 1);
                    own_health--;
                default: break;
            }
            if(is_there_a_winner(informationWin, opp_health, own_health)) break;
        }
        if(is_there_a_winner(informationWin, opp_health, own_health)) break;
    }



    end_game_with_confirm(role, socket, socket_server_only);

    return 0;
}
