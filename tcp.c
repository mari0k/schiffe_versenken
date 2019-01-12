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


char* get_own_ip() {
    char hostbuffer[256];
    struct hostent *host_entry;
    gethostname(hostbuffer, sizeof(hostbuffer));
    host_entry = gethostbyname(hostbuffer);
    return inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
}

int create_socket() {
    #ifdef _WIN32
    	//----------------------
    	// Initialize Winsock.
    	WSADATA wsaData;
    	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    	if (iResult != NO_ERROR) {
            tcp_msg("ERROR on WSAStartup()");
    		return -1;
    	}
    #endif
    // create socket (this is the same for server and client)
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        tcp_msg("ERROR on socket()");
    }
    return sock;
}

void close_socket(int sock) {
    #ifdef _WIN32
		closesocket(sock);
    #else
		close(sock);
    #endif
    return;
}

void clean_up() {
    #ifdef _WIN32
        WSACleanup();
    #endif
    return;
}

void bind_socket_to_port(int sock, int port) {
    struct sockaddr_in server_addr;
    bzero((char *) &server_addr, sizeof(server_addr));
    // bind port to socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        tcp_msg("ERROR on bind()");
    }
    return;
}

int listen_and_accept_client(int sock) {
    struct sockaddr_in client_addr;
    bzero((char *) &client_addr, sizeof(client_addr));
    socklen_t client_size = sizeof (client_addr);
    // listen for a client
    if (listen(sock, 5) == -1) {
        tcp_msg("ERROR on listen()");
        return -1;
    }
    // accept client
    int sock2 = accept(sock, (struct sockaddr *) &client_addr, &client_size);
    if (sock2 == -1) {
        tcp_msg("ERROR on accept()");
    }
    return sock2;
}

int connect_to_server(int sock, char *server_ip, int port) {
    struct sockaddr_in server_addr;
    bzero((char *) &server_addr, sizeof(server_addr));
    // set server adress information
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    // connect to server
    if (connect(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        tcp_msg("ERROR on connect()");
        return -1;
    }
    return 0;
}

int send_name(int sock, char *name) {
    if (send(sock, name, 21, 0) < 0) {
        tcp_msg("ERROR sending name.");
        notify("There was a communication error. This may be due to your opponent leaving the game.");
        return -1;
    }
    return 0;
}

int recv_name(int sock, char *name) {
    if (recv(sock, name, 21, 0) < 0) {
        tcp_msg("ERROR receiving name.");
        notify("There was a communication error. This may be due to your opponent leaving the game.");
        return -1;
    }
    return 0;
}

int send_int(int sock, unsigned char *buf, int num) {
    int pos = 0;
    write_int(buf, &pos, num);
    if (send(sock, buf, pos, 0) < 0) {
        tcp_msg("ERROR sending int.");
        notify("There was a communication error. This may be due to your opponent leaving the game.");
        return -1;
    }
    return 0;
}

int recv_int(int sock, unsigned char *buf, int *num) {
    if (recv(sock, buf, 4, 0) < 0) {
        tcp_msg("ERROR receiving int.");
        notify("There was a communication error. This may be due to your opponent leaving the game.");
        return -1;
    }
    int pos = 0;
    read_int(buf, &pos, num);
    return 0;
}

int send_coord(int sock, unsigned char *buf, struct coord *coord) {
    int pos = 0;
    write_int(buf, &pos, coord->y);
    write_int(buf, &pos, coord->x);
    if (send(sock, buf, pos, 0) < 0) {
        tcp_msg("ERROR sending position.");
        notify("There was a communication error. This may be due to your opponent leaving the game.");
        return -1;
    }
    return 0;
}

int recv_coord(int sock, unsigned char *buf, struct coord *coord) {
    if (recv(sock, buf, 8, 0) < 0) {
        tcp_msg("ERR receiving position.");
        notify("There was a communication error. This may be due to your opponent leaving the game.");
        return -1;
    }
    int pos = 0;
    read_int(buf, &pos, &(coord->y));
    read_int(buf, &pos, &(coord->x));
    return 0;
}
