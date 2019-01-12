#ifndef TCP_H
#define TCP_H

// tcp.h
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // for gethostname(); and gethostbyname();
#endif

#include <stdlib.h>
#include <string.h>

#include "ui.h"
#include "structs.h"


void read_int(unsigned char *buffer, int *pos, int *val);

void write_int(unsigned char *buffer, int *pos, int val);

char* get_own_ip();

int create_socket();

void close_socket(int sock);

void clean_up();

void bind_socket_to_port(int sock, int port);

int listen_and_accept_client(int sock);

int connect_to_server(int sock, char *server_ip, int port);

int send_name(int sock, char *name);

int recv_name(int sock, char *name);

int send_int(int sock, unsigned char *buf, int num);

int recv_int(int sock, unsigned char *buf, int *num);

int send_coord(int sock, unsigned char *buf, struct coord *coord);

int recv_coord(int sock, unsigned char *buf, struct coord *coord);


#endif
