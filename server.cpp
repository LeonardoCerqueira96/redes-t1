#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
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
mutex mtx;

int server_socket;

void echo_message(int except, char *message, vector<int> &clients)
{
	char buffer[BUFFER_SIZE];
	
	strcpy(buffer, message);

	mtx.lock();
	for (int i = 0; i < clients.size(); i++)
	{
		if (clients[i] != except)
			send(clients[i], buffer, BUFFER_SIZE, 0);
	}
	mtx.unlock();

	if (except != server_socket)
		cout << message << endl; 
}

void handle_client(int client_socket, int server_socket, vector<int> &clients)
{
	char buffer[BUFFER_SIZE];
	char client_name[32];
	char message[BUFFER_SIZE];

	int bytes_read;

	// get client's name first
	if ((bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0)) < 0)
	{
		cout << "New connection failed" << endl;
		mtx.lock();
		
		// if the disconnected, remove him from the list
		for (int i = 0; i < clients.size(); i++)
		{
			if (clients[i] == client_socket)
			{
				clients.erase(clients.begin() + i);
				break;
			}
		}
		mtx.unlock();
		return;
	}
	buffer[bytes_read] = '\0';
	strcpy(client_name, buffer);

	// send a welcome message to every client
	strcpy(message, "Welcome, ");
	strcat(message, client_name);
	thread{echo_message, client_socket, message, ref(clients)}.detach();

	// read his messages
	while ((bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0)) >= 0)
	{
		buffer[bytes_read] = '\0';
		
		strcpy(message, "[");
		strcat(message, client_name);
		strcat(message, "]: ");
		strcat(message, buffer);
		
		// echo the messages to every client
		thread{echo_message, client_socket, message, ref(clients)}.detach();
	}

	mtx.lock();

	// If he disconnected, remove him from the clients list
	for (int i = 0; i < clients.size(); i++)
	{
		if (clients[i] == client_socket)
		{
			clients.erase(clients.begin() + i);
			break;
		}
	}
	mtx.unlock();

	// let the other clients know
	strcpy(message, client_name);
	strcat(message, " disconnected");
	thread{echo_message, -1, message, ref(clients)}.detach();
	
	return;
}

int main(int argc, char *argv[])
{
	int client_socket, port;
	int opt = 1;

	struct sockaddr_in server_addr, client_addr;

	vector<int> clients;

	if (argc < 2)
	{
		cout << "You need to provide a port number for the server" << endl;
		exit(1);
	}

	// creating socket
	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
	{
		cout << "Error on create socket" << endl;
		exit(1);
	}
	cout << "Server socket created" << endl;

	// setting option to reuse addresses
	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
	{
		cout << "Error on settting socket options" << endl;
		exit(1);
	}

	// binding the socket to the port
	port = atoi(argv[1]);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	server_addr.sin_port = htons(port);
	client_addr = server_addr;
	if ((bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr))) < 0)
	{
		cout << "Error on binding the socket to the port" << endl;
		exit(1);
	}

	socklen_t addr_len = sizeof(server_addr);


	// set a maximum of 5 pending connections
	if (listen(server_socket, 5) < 0)
	{
		cout << "Error on listen" << endl;
		exit(1);
	}

	int client_count = 0;
	while ((client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len)) >= 0)
	{
		clients.push_back(client_socket);
		thread{handle_client, client_socket, server_socket, ref(clients)}.detach();
	}

	return 0;
}
