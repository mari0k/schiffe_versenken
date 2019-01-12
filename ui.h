#ifndef UI_H
#define UI_H

// ui.h
#include <ncurses.h>

#include "structs.h"

const char y_name[10];
const int x_name[10];



void draw_welcome(WINDOW *window);

void draw_title(WINDOW *window);

void draw_field_labels();

void draw_field_grid(WINDOW *window);

void print_rules(WINDOW *window, int mode);

void print_controls(WINDOW *window);

void print_legend(WINDOW *window);

void print_ships(WINDOW *window);

void print_role(int role, char *server_ip, int port);

void ask_for_name(char *name);

int select_game_mode();

int heads_or_tails(int role, char *opp_name);

void print_position(WINDOW *window, struct coord *coord, int **field, int own_side);

void highlight_position(WINDOW *window, struct coord *coord, int **field, int own_side);

void get_move(WINDOW *window, struct move *move);

int get_alignment(WINDOW *window, struct alignment *align);

int is_alignment_feasible(int **field, int ship_length, struct coord *bow_coord, struct alignment *align);

int anchor_ship(WINDOW *fieldWin, int **field, WINDOW *infoWin, struct ship *ship, struct move *move);

int place_ship(WINDOW *fieldWin, int **field, WINDOW *infoWin, struct ship *ship, struct coord *anchor);

int decide_bombing_location(WINDOW *fieldWin, int **field, WINDOW *infoWin, struct move *move);

void print_information(WINDOW *window, int x, char *message);

void tcp_msg(char *message);

void notify(char *message);


#endif
