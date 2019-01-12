#include <ncurses.h>

#include "structs.h"


#define EMPTY 0
#define SHIP 10
#define SHIP_DAMAGED 20
#define HIT 30
#define MISS 40

#define ALTERNATE 1
#define BURST 2
#define UNTILMISS 3

const char y_name[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J'};
const int x_name[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};


void draw_welcome(WINDOW *window) {
    wmove(window, 0, 0);
    waddch(window, ACS_TTEE); wprintw(window, " "); waddch(window, ACS_TTEE);
    waddch(window, ACS_ULCORNER); waddch(window, ACS_HLINE); waddch(window, ACS_URCORNER);
    waddch(window, ACS_TTEE); wprintw(window, "  ");
    waddch(window, ACS_ULCORNER); waddch(window, ACS_HLINE); waddch(window, ACS_URCORNER);
    waddch(window, ACS_ULCORNER); waddch(window, ACS_HLINE); waddch(window, ACS_URCORNER);
    waddch(window, ACS_ULCORNER); waddch(window, ACS_TTEE); waddch(window, ACS_URCORNER);
    waddch(window, ACS_ULCORNER); waddch(window, ACS_HLINE); waddch(window, ACS_URCORNER);
    wprintw(window, "    ");
    waddch(window, ACS_ULCORNER); waddch(window, ACS_TTEE); waddch(window, ACS_URCORNER);
    waddch(window, ACS_ULCORNER); waddch(window, ACS_HLINE); waddch(window, ACS_URCORNER);

    wmove(window, 1, 0);
    waddch(window, ACS_VLINE); waddch(window, ACS_VLINE); waddch(window, ACS_VLINE);
    waddch(window, ACS_LTEE); waddch(window, ACS_RTEE); wprintw(window, " ");
    waddch(window, ACS_VLINE); wprintw(window, "  ");
    waddch(window, ACS_VLINE); wprintw(window, "  ");
    waddch(window, ACS_VLINE); wprintw(window, " "); waddch(window, ACS_VLINE);
    waddch(window, ACS_VLINE); waddch(window, ACS_VLINE); waddch(window, ACS_VLINE);
    waddch(window, ACS_LTEE); waddch(window, ACS_RTEE);
    wprintw(window, "      ");
    waddch(window, ACS_VLINE); wprintw(window, " ");
    waddch(window, ACS_VLINE); wprintw(window, " "); waddch(window, ACS_VLINE);

    wmove(window, 2, 0);
    waddch(window, ACS_LLCORNER); waddch(window, ACS_BTEE); waddch(window, ACS_LRCORNER);
    waddch(window, ACS_LLCORNER); waddch(window, ACS_HLINE); waddch(window, ACS_LRCORNER);
    waddch(window, ACS_BTEE); waddch(window, ACS_HLINE); waddch(window, ACS_LRCORNER);
    waddch(window, ACS_LLCORNER); waddch(window, ACS_HLINE); waddch(window, ACS_LRCORNER);
    waddch(window, ACS_LLCORNER); waddch(window, ACS_HLINE); waddch(window, ACS_LRCORNER);
    waddch(window, ACS_BTEE); wprintw(window, " "); waddch(window, ACS_BTEE);
    waddch(window, ACS_LLCORNER); waddch(window, ACS_HLINE); waddch(window, ACS_LRCORNER);
    wprintw(window, "     ");
    waddch(window, ACS_BTEE); wprintw(window, " ");
    waddch(window, ACS_LLCORNER); waddch(window, ACS_HLINE); waddch(window, ACS_LRCORNER);

    wrefresh(window);
    return;
}

void draw_title(WINDOW *window) {
    mvwprintw(window, 0, 0, "  _____                ____        _   _   _          ___    ___");
    mvwprintw(window, 1, 0, " / ____|              |  _ \\      | | | | | |        |__ \\  / _ \\");
    mvwprintw(window, 2, 0, "| (___   ___  __ _    | |_) | __ _| |_| |_| | ___       ) || | | |");
    mvwprintw(window, 3, 0, " \\___ \\ / _ \\/ _` |   |  _ < / _` | __| __| |/ _ \\     / / | | | |");
    mvwprintw(window, 4, 0, " ____) |  __/ (_| |   | |_) | (_| | |_| |_| |  __/    / /_ | |_| |");
    mvwprintw(window, 5, 0, "|_____/ \\___|\\__,_|   |____/ \\__,_|\\__|\\__|_|\\___|   |____(_)___/");
    wrefresh(window);
    return;
}

void draw_field_labels() {
    // opponent
    mvprintw(5, 1, "A");
    mvprintw(7, 1, "B");
    mvprintw(9, 1, "C");
    mvprintw(11, 1, "D");
    mvprintw(13, 1, "E");
    mvprintw(15, 1, "F");
    mvprintw(17, 1, "G");
    mvprintw(19, 1, "H");
    mvprintw(21, 1, "I");
    mvprintw(23, 1, "J");
    mvprintw(3, 5, "1   2   3   4   5   6   7   8   9   0");

    // own
    mvprintw(23, 49, "A");
    mvprintw(25, 49, "B");
    mvprintw(27, 49, "C");
    mvprintw(29, 49, "D");
    mvprintw(31, 49, "E");
    mvprintw(33, 49, "F");
    mvprintw(35, 49, "G");
    mvprintw(37, 49, "H");
    mvprintw(39, 49, "I");
    mvprintw(41, 49, "J");
    mvprintw(21, 53, "1   2   3   4   5   6   7   8   9   0");

    refresh();
    return;
}

void draw_field_grid(WINDOW *window) {
    box(window, 0, 0);
    for (int i = 4; i <= 36; i += 4) {
        wmove(window, 0, i);
        waddch(window, ACS_TTEE);
        wmove(window, 20, i);
        waddch(window, ACS_BTEE);
    }
    for (int j = 2; j <= 18; j += 2) {
        wmove(window, j, 0);
        waddch(window, ACS_LTEE);
        for (int k = 0; k < 39; k++) {
            waddch(window, ACS_HLINE);
        }
        waddch(window, ACS_RTEE);
    }
    for (int i = 4; i <= 36; i += 4) {
        for (int j = 1; j <= 19; j++) {
            wmove(window, j, i);
            if (j % 2 == 0) {
                waddch(window, ACS_PLUS);
            }
            else {
                waddch(window, ACS_VLINE);
            }
        }
    }
    wrefresh(window);
    return;
}

void print_rules(WINDOW *window, int mode) {
    mvwprintw(window, 0, 1, "RULES");
    wmove(window, 1, 0);
    for (int k = 0; k < 7; k++) waddch(window, ACS_S1);
    wmove(window, 3, 2); waddch(window, ACS_BULLET);
    wprintw(window, " ships can\'t touch each other");
    wmove(window, 4, 2); waddch(window, ACS_BULLET);
    wprintw(window, " ships may touch the border");
    wmove(window, 5, 2); waddch(window, ACS_BULLET);
    wprintw(window, " ships can\'t be placed diagonal");
    wmove(window, 6, 2); waddch(window, ACS_BULLET);
    wprintw(window, " each player has 7 ships:");
    mvwprintw(window, 7, 6, "1 battleship (5 boxes)");
    mvwprintw(window, 8, 6, "1 cruiser    (4 boxes)");
    mvwprintw(window, 9, 6, "2 destroyer  (3 boxes)");
    mvwprintw(window, 10, 6, "3 submarines (2 boxes)");
    wmove(window, 11, 2); waddch(window, ACS_BULLET);
    switch (mode) {
        case ALTERNATE: wprintw(window, " each player fires one bomb"); break;
        case BURST: wprintw(window, " each player fires a series of 3 bombs"); break;
        case UNTILMISS: wprintw(window, " each player fires bombs as long");
                        mvwprintw(window, 12, 4, "as his bombs hit opposing ships"); break;
        default: break;
    }
    wrefresh(window);
    return;
}

void print_controls(WINDOW *window) {
    mvwprintw(window, 0, 1, "CONTROLS");
    wmove(window, 1, 0);
    for (int k = 0; k < 10; k++) waddch(window, ACS_S1);
    mvwprintw(window, 2, 2, "move  ->  WASD or");
    mvwprintw(window, 3, 2, "          ARROW KEYS");
    mvwprintw(window, 4, 2, "bomb  ->  B or SPACE");
    mvwprintw(window, 5, 2, "place ->  P or ENTER");
    mvwprintw(window, 6, 2, "quit  ->  Q or ESC");
    wrefresh(window);
    return;
}

void print_legend(WINDOW *window) {
    mvwprintw(window, 0, 1, "LEGEND");
    wmove(window, 1, 0);
    for (int k = 0; k < 8; k++) waddch(window, ACS_S1);
    wattron(window, A_REVERSE);
    mvwprintw(window, 2, 2, " ");
    wattroff(window, A_REVERSE);
    mvwprintw(window, 2, 5, "->  current position");
    wattron(window, COLOR_PAIR(SHIP));
    mvwprintw(window, 3, 2, "#");
    wattroff(window, COLOR_PAIR(SHIP));
    mvwprintw(window, 3, 5, "->  ship");
    wattron(window, COLOR_PAIR(SHIP_DAMAGED));
    mvwprintw(window, 4, 2, "@");
    wattroff(window, COLOR_PAIR(SHIP_DAMAGED));
    mvwprintw(window, 4, 5, "->  damaged ship");
    wattron(window, COLOR_PAIR(HIT));
    mvwprintw(window, 5, 2, "$");
    wattroff(window, COLOR_PAIR(HIT));
    mvwprintw(window, 5, 5, "->  successful hit");
    wattron(window, COLOR_PAIR(MISS));
    mvwprintw(window, 6, 2, "X");
    wattroff(window, COLOR_PAIR(MISS));
    mvwprintw(window, 6, 5, "->  unsuccessful hit");
    wrefresh(window);
    return;
}

void print_ships(WINDOW *window) {
    mvwprintw(window, 0, 1, "SHIPS");
    wmove(window, 1, 0);
    for (int k = 0; k < 7; k++) waddch(window, ACS_S1);
    mvwprintw(window, 2, 1, "#####");
    mvwprintw(window, 3, 1, "####");
    mvwprintw(window, 4, 1, "###");
    mvwprintw(window, 5, 1, "###");
    mvwprintw(window, 6, 1, "##");
    mvwprintw(window, 7, 1, "##");
    mvwprintw(window, 8, 1, "##");
    wrefresh(window);
    return;
}

void print_role(int role, char *server_ip, int port) {
    move(7, 66);
    switch (role) {
        case 0:
            printw("SERVER - address: %s:%d", server_ip, port);
            mvprintw(7, 108, "(Waiting for Client)");
            break;
        case 1:
            printw("CLIENT - server at %s:%d", server_ip, port);
            mvprintw(7, 108, "(Connecting to Server)");
            break;
        case 2:
            printw("Player vs. AI");
            break;
        default: break;
    }
    refresh();
    return;
}

void ask_for_name(char *name) {
    WINDOW *popup = newwin(9, 28, 15, 52);
    wbkgd(popup, COLOR_PAIR(MISS));
    box(popup, 0, 0);

    // allow arrow keys
    keypad(popup, true);

    // draw input field
    wmove(popup, 3, 2);
    waddch(popup, ACS_ULCORNER);
    for(int k = 0; k < 22; k++) waddch(popup, ACS_HLINE);
    waddch(popup, ACS_URCORNER);
    wmove(popup, 4, 2);
    waddch(popup, ACS_VLINE);
    wmove(popup, 4, 25);
    waddch(popup, ACS_VLINE);
    wmove(popup, 5, 2);
    waddch(popup, ACS_LLCORNER);
    for(int k = 0; k < 22; k++) waddch(popup, ACS_HLINE);
    waddch(popup, ACS_LRCORNER);
    mvwprintw(popup, 7, 2, "(Press ENTER to confirm)");

    // ask for name
    mvwprintw(popup, 1, 3, "Please enter your NAME");
    mvwprintw(popup, 2, 2, "(maximum: 21 characters)");
    // move into inputfield
    wmove(popup, 4, 3);
    // enable keyboard input and set cursor visible
    echo();
    curs_set(1);
    wgetstr(popup, name);
    name[21] = '\0';
    // disable keyboard input and set cursor invisible
    noecho();
    curs_set(0);

    // clear popup window
    werase(popup);
    wrefresh(popup);
    delwin(popup);

    return;
}

int select_game_mode() {
    WINDOW *popup = newwin(9, 82, 15, 25);
    wbkgd(popup, COLOR_PAIR(MISS));
    box(popup, 0, 0);

    // allow arrow keys
    keypad(popup, true);

    mvwprintw(popup, 1, 2, "Choose the game mode you want to play.");
    mvwprintw(popup, 3, 15, "(each player fires one bomb)");
    mvwprintw(popup, 4, 15, "(each player fires a series of 3 bombs)");
    mvwprintw(popup, 5, 15, "(each player fires bombs as long as his bombs hit opposing ships)");
    mvwprintw(popup, 7, 2, "(Press ENTER to confirm)");

    int choice = ALTERNATE;
    int ch;

    do {
        if (choice == ALTERNATE) wattron(popup, A_REVERSE);
        mvwprintw(popup, 3, 2, "Alternating");
        wattroff(popup, A_REVERSE);
        if (choice == BURST) wattron(popup, A_REVERSE);
        mvwprintw(popup, 4, 2, "Bursts");
        wattroff(popup, A_REVERSE);
        if (choice == UNTILMISS) wattron(popup, A_REVERSE);
        mvwprintw(popup, 5, 2, "Until miss");
        wattroff(popup, A_REVERSE);
        wrefresh(popup);
        ch = wgetch(popup);
        switch (ch) {
            case KEY_UP: choice = ((choice - 1)<(ALTERNATE))?(ALTERNATE):(choice - 1); break;
            case KEY_DOWN: choice = ((choice + 1)>(UNTILMISS))?(UNTILMISS):(choice + 1); break;
            default: break;
        }
    } while(ch != 10);


    // clear popup window
    werase(popup);
    wrefresh(popup);
    delwin(popup);

    return choice;
}

int heads_or_tails(int role, char *opp_name) {
    WINDOW *popup = newwin(8, 51, 10, 72);
    wbkgd(popup, COLOR_PAIR(MISS));
    box(popup, 0, 0);

    // allow arrow keys
    keypad(popup, true);

    mvwprintw(popup, 1, 2, "Choose either \'Heads\' or \'Tails\'.");
    switch (role) {
        case 0:
            mvwprintw(popup, 2, 2, "You go first if %.13s chooses the SAME.", opp_name);
            break;
        case 1:
            mvwprintw(popup, 2, 2, "You go first if %.12s chooses DIFFERENT.", opp_name);
            break;
        default: break;
    }
    mvwprintw(popup, 6, 13, "(Press ENTER to confirm)");

    int choice = 0;
    int ch;

    do {
        if (choice == 0) {
            wattron(popup, A_REVERSE);
            mvwprintw(popup, 4, 12, "< Heads >");
            wattroff(popup, A_REVERSE);
            mvwprintw(popup, 4, 29, "< Tails >");
        } else {
            mvwprintw(popup, 4, 12, "< Heads >");
            wattron(popup, A_REVERSE);
            mvwprintw(popup, 4, 29, "< Tails >");
            wattroff(popup, A_REVERSE);
        }
        wrefresh(popup);
        ch = wgetch(popup);
        switch (ch) {
            case KEY_LEFT: choice = 0; break;
            case KEY_RIGHT: choice = 1; break;
            default: break;
        }
    } while(ch != 10);


    // clear popup window
    werase(popup);
    wrefresh(popup);
    delwin(popup);

    return choice;
}

void print_position(WINDOW *window, struct coord *coord, int **field, int own_side) {
    if (own_side) {
        switch (field[coord->y][coord->x]) {
            case SHIP_DAMAGED:
                wattron(window, COLOR_PAIR(SHIP_DAMAGED));
                mvwprintw(window, ((coord->y + 1) * 2) - 1, ((coord->x + 1 ) * 4) - 2, "@");
                wattroff(window, COLOR_PAIR(SHIP_DAMAGED));
                break;
            case MISS:
                wattron(window, COLOR_PAIR(MISS));
                mvwprintw(window, ((coord->y + 1) * 2) - 1, ((coord->x + 1 ) * 4) - 2, "x");
                wattroff(window, COLOR_PAIR(MISS));
                break;
            case 1: case 2: case 3: case 4: case 5: case 6: case 7:
                wattron(window, COLOR_PAIR(SHIP));
                mvwprintw(window, ((coord->y + 1) * 2) - 1, ((coord->x + 1 ) * 4) - 2, "#");
                wattroff(window, COLOR_PAIR(SHIP));
                break;
            default:
                mvwprintw(window, ((coord->y + 1) * 2) - 1, ((coord->x + 1 ) * 4) - 2, " ");
                break;
        }
    }
    else {
        switch (field[coord->y][coord->x]) {
            case HIT:
                wattron(window, COLOR_PAIR(HIT));
                mvwprintw(window, ((coord->y + 1) * 2) - 1, ((coord->x + 1 ) * 4) - 2, "$");
                wattroff(window, COLOR_PAIR(HIT));
                break;
            case MISS:
                wattron(window, COLOR_PAIR(MISS));
                mvwprintw(window, ((coord->y + 1) * 2) - 1, ((coord->x + 1 ) * 4) - 2, "X");
                wattroff(window, COLOR_PAIR(MISS));
                break;
            default:
                mvwprintw(window, ((coord->y + 1) * 2) - 1, ((coord->x + 1 ) * 4) - 2, " ");
                break;
        }
    }
    wrefresh(window);
}

void highlight_position(WINDOW *window, struct coord *coord, int **field, int own_side) {
    wattron(window, A_REVERSE);
    print_position(window, coord, field, own_side);
    wattroff(window, A_REVERSE);
    wrefresh(window);
    return;
}

void get_move(WINDOW *window, struct move *move) {
    while(1) {
        int ch = wgetch(window);
        switch (ch) {
            case KEY_UP: case 87: case 119:     // W w
                move->new.y = (move->old.y + 10 - 1) % 10;
                move->type = 0; return;
            case KEY_LEFT: case 65: case 97:    // A a
                move->new.x = (move->old.x + 10 - 1) % 10;
                move->type = 0; return;
            case KEY_DOWN: case 83: case 115:   // S s
                move->new.y = (move->old.y + 1) % 10;
                move->type = 0; return;
            case KEY_RIGHT: case 68: case 100:  // D d
                move->new.x = (move->old.x + 1) % 10;
                move->type = 0; return;
            case 10: case 80: case 112:         // ENTER P p
                move->type = 80; return;
            case 32: case 66: case 98:          // SPACE B b
                move->type = 66; return;
            case 27: case 81: case 113:         // ESC Q q
                move->type = 81; return;
            default: break;
        }
    }
}

int get_alignment(WINDOW *window, struct alignment *align) {
    while(1) {
        int ch = wgetch(window);
        switch (ch) {
            case KEY_UP: case 87: case 119:     // W w
                align->y = -1; align->x = 0; return 0;
            case KEY_LEFT: case 65: case 97:    // A a
                align->y = 0; align->x = -1; return 0;
            case KEY_DOWN: case 83: case 115:   // S s
                align->y = 1; align->x = 0; return 0;
            case KEY_RIGHT: case 68: case 100:  // D d
                align->y = 0; align->x = 1; return 0;
            case 10: case 80: case 112:         // ENTER P p
                return 80;
            case 27: case 81: case 113:         // ESC Q q
                return 81;
            default: break;
        }
    }
}

int is_alignment_feasible(int **field, int ship_length, struct coord *bow_coord, struct alignment *align) {
    // check that the ship would not be positioned out of bounds
    if (bow_coord->y + ((ship_length - 1) * (align->y)) > 9 || bow_coord->y + ((ship_length - 1) * (align->y)) < 0 ||
        bow_coord->x + ((ship_length - 1) * (align->x)) > 9 || bow_coord->x + ((ship_length - 1) * (align->x)) < 0) {
        return 0;
    }
    // check that there are no conflicting ships
    for (int k = -1; k <= 1; k++) {
        for (int l = 2; l <= ship_length; l++) {
            if (field[MIN(MAX(bow_coord->y + (k * (align->x)) + (l * (align->y)), 0), 9)][MIN(MAX(bow_coord->x + (k * (align->y)) + (l * (align->x)), 0), 9)] != EMPTY) {
                return 0;   // there is a conflicting ship
            }
        }
    }
    // alignment is feasible
    return 1;
}

int anchor_ship(WINDOW *fieldWin, int **field, WINDOW *infoWin, struct ship *ship, struct move *move) {
    mvwprintw(infoWin, 1, 2, "placing ship %d of 7: ", ship->id);
    for (int j = 0; j < ship->length; j++) {
        wprintw(infoWin, "#");
    }
    mvwprintw(infoWin, 2, 3, "select the ships bow position");
    wrefresh(infoWin);
    highlight_position(fieldWin, &(move->new), field, 1);
    // get feasible position for the bow of the ship
    while (1) {
        move->new.y = move->old.y;
        move->new.x = move->old.x;
        get_move(fieldWin, move);
        if (move->type == 81) { // user wants to quit game
            return -1;
        }
        if (move->type == 0) {  // just changing position
            print_position(fieldWin, &(move->old), field, 1);
            highlight_position(fieldWin, &(move->new), field, 1);
            move->old.y = move->new.y;
            move->old.x = move->new.x;
            continue;
        }
        if (move->type == 80 && move->old.y == move->new.y && move->old.x == move->new.x) {
            // anchor ship if there is no adjacent ship
            // and there is at least one feasible direction to place the ship
            int conflict_in_positioning = 0;
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    if (field[MIN(MAX(move->old.y + i, 0), 9)][MIN(MAX(move->old.x + j, 0), 9)] != EMPTY) {
                        conflict_in_positioning = 1;
                        break;
                    }
                }
                if (conflict_in_positioning) break;
            }
            if(conflict_in_positioning) { // there was an overlap or an adjacent ship, so skip
                continue;
            }
            // there is no adjacent ship and no overlap
            // check for a feasible direction now
            struct alignment direction = { .y = 0, .x = 0 };
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    if (ABS(i) + ABS(j) == 1) {
                        direction.y = i;
                        direction.x = j;
                        if (is_alignment_feasible(field, ship->length, &(move->old), &direction)) break;
                        direction.y = 0;
                        direction.x = 0;
                    }
                }
                if (direction.y != 0 || direction.x != 0) break;
            }
            if (direction.y != 0 || direction.x != 0) { // there is a feasible direction
                break;
            }
        }
    }
    return 0;
}

int place_ship(WINDOW *fieldWin, int **field, WINDOW *infoWin, struct ship *ship, struct coord *anchor) {
    // we have a feasible position for the bow of the ship
    // and the user has decided to anchor there
    struct alignment old_align = { .y = 0, .x = 0 };
    struct alignment new_align = { .y = 0, .x = 0 };
    int align_type;
    wmove(infoWin, 2, 3);
    wclrtoeol(infoWin);
    box(infoWin, 0, 0);
    mvwprintw(infoWin, 2, 3, "ship has been anchored at %c%d", y_name[anchor->y], x_name[anchor->x]);
    mvwprintw(infoWin, 3, 3, "now select orientation of the ship");
    wrefresh(infoWin);
    // get feasible position for the stern of the ship
    while(1) {
        align_type = get_alignment(fieldWin, &new_align);
        if (align_type == 81) {   // user wants to quit game
            return -1;
        }
        if (align_type == 0 && ABS(new_align.y) + ABS(new_align.x) == 1) {
            // first restore old positions
            if (is_alignment_feasible(field, ship->length, anchor, &old_align) && ABS(old_align.y) + ABS(old_align.x) == 1) {
                // the old orientation was feasible so we need to do something
                struct coord coord = { .y = anchor->y, .x = anchor->x };
                for (int l = 1; l < ship->length; l++) {
                    coord.y = coord.y + old_align.y;
                    coord.x = coord.x + old_align.x;
                    print_position(fieldWin, &coord, field, 1);
                }
            }
            if (is_alignment_feasible(field, ship->length, anchor, &new_align)) {
                // ship can be placed in this direction, so highlight new positions
                struct coord coord = { .y = anchor->y, .x = anchor->x };
                for (int l = 0; l < ship->length; l++) {
                    highlight_position(fieldWin, &coord, field, 1);
                    coord.y = coord.y + new_align.y;
                    coord.x = coord.x + new_align.x;
                }
                old_align.y = new_align.y;
                old_align.x = new_align.x;
            }
        }
        if (align_type == 80 && ABS(new_align.y) + ABS(new_align.x) == 1) {
            if (is_alignment_feasible(field, ship->length, anchor, &new_align)) {
                // user decided to place and the alignment is feasible
                // so place the ship
                struct coord coord = { .y = (anchor->y - new_align.y), .x = (anchor->x - new_align.x) };
                for (int l = 0; l < ship->length; l++) {
                    coord.y = coord.y + new_align.y;
                    coord.x = coord.x + new_align.x;
                    field[coord.y][coord.x] = ship->id;
                    print_position(fieldWin, &coord, field, 1);
                }
                // tell the ship where it is
                ship->bow_coord.y = anchor->y;
                ship->bow_coord.x = anchor->x;
                ship->stern_coord.y = coord.y;
                ship->stern_coord.x = coord.x;
                ship->align.y = new_align.y;
                ship->align.x = new_align.x;
                break;
            }
        }
    }
    // erase the information window
    werase(infoWin);
    box(infoWin, 0, 0);
    wrefresh(infoWin);

    return 0;
}

int decide_bombing_location(WINDOW *fieldWin, int **field, WINDOW *infoWin, struct move *move) {
    highlight_position(fieldWin, &(move->new), field, 0);
    // get feasible position for the bow of the ship
    while (1) {
        move->new.y = move->old.y;
        move->new.x = move->old.x;
        get_move(fieldWin, move);
        if (move->type == 81) { // user wants to quit game
            return -1;
        }
        if (move->type == 0) {  // just changing position
            print_position(fieldWin, &(move->old), field, 0);
            highlight_position(fieldWin, &(move->new), field, 0);
            move->old.y = move->new.y;
            move->old.x = move->new.x;
            continue;
        }
        if (move->type == 66 && move->old.y == move->new.y && move->old.x == move->new.x &&
            field[move->old.y][move->old.x] < HIT) {
            scroll(infoWin);
            wmove(infoWin, 8, 0);
            wclrtoeol(infoWin);
            mvwprintw(infoWin, 7, 8, "Your bomb heads towards %c%d. Waiting for impact...", y_name[move->old.y], x_name[move->old.x]);
            box(infoWin, 0, 0);
            wrefresh(infoWin);
            break;
        }
    }
    return 0;
}

void print_information(WINDOW *window, int x, char *message) {
    scroll(window);
    wmove(window, 8, 0);
    wclrtoeol(window);
    mvwprintw(window, 7, x, message);
    box(window, 0, 0);
    wrefresh(window);
    return;
}

void tcp_msg(char *message) {
    move(7, 108); clrtoeol();
    mvprintw(7, 108, message);
    refresh();
    return;
}

void notify(char *message) {
    WINDOW *notification = newwin(14, 41, 28, 3);
    wbkgd(notification, COLOR_PAIR(MISS));
    refresh();
    mvwprintw(notification, 3, 0, message);
    wrefresh(notification);
    return;
}
