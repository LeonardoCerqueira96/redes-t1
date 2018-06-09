#include <iostream>
#include <thread>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

using namespace std;

const int BUFFER_SIZE = 1024;
bool server_up;

void read_from_server(int client_socket)
{
	int bytes_read;
	char buffer[BUFFER_SIZE];

	while ((bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0)
	{
		buffer[bytes_read] = '\0';
		cout << buffer << endl;
	}
	server_up = false;

	return;
}

int main(int argc, char *argv[])
{
	int client_socket, port;
	char ip[16], client_name[32], message[BUFFER_SIZE];


	struct sockaddr_in serv_addr;

	if (argc < 4)
	{
		cout << "You need to provide more information" << endl;
		cout << "1- Port number" << endl;
		cout << "2- Server IP address" << endl;
		cout << "3- Client name" << endl;
		exit(1);
	}
	port = atoi(argv[1]);
	strcpy(ip, argv[2]);
	strcpy(client_name, argv[3]);
	
	// creating socket
	if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cout << "Error on creating socket" << endl;
		exit(1);
	}
	cout << "Socket created" << endl;

	// setting up struct for connection
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0)
	{
		cout << "Invalid IP address" << endl;
		exit(1);
	}

	// connecting to server
	if (connect(client_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
	{
		cout << "Connection failed" << endl;
		exit(1);
	}
	cout << "Connected to server" << endl;
	server_up = true;
	
	send(client_socket, client_name, strlen(client_name), 0);
	thread{read_from_server, client_socket}.detach();

	while (server_up)
	{
		cin.getline(message, BUFFER_SIZE);
		send(client_socket, message, strlen(message), 0);
	}
	
	cout << "The server is down, quitting..." << endl;
	
	return 0;
}

