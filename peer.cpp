#include<iostream>
#include<vector>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sstream>
#include<netdb.h>
#include<stdio.h>
#include <pthread.h> 
using namespace std;

int tracker;
char *trackerfile;

void *server(void *p)
{
	int *por = (int *) p;
	int port = *por;
	int s_socket = socket(AF_INET, SOCK_STREAM, 0);

	if(s_socket == -1)
		cout << "Error while opening socket." << endl;

	struct sockaddr_in server_add;
	bzero((char *)&server_add, sizeof(server_add));
	
	server_add.sin_family = AF_INET;
	server_add.sin_addr.s_addr = INADDR_ANY;
	server_add.sin_port = htons(port);

	int err = bind(s_socket, (struct sockaddr *) &server_add, sizeof(server_add));

	if(err < 0)
		cout << "Binding failed."<< endl;

	listen(s_socket, 10);

	int i=0;
	int com_socket[100];
	while(true)
	{
		struct sockaddr_in client_add;
		socklen_t len = sizeof(client_add);
			com_socket[i] = accept(s_socket, (struct sockaddr *)&client_add, &len);

		if(com_socket < 0)
			cout << "Error while accept!" << endl;

		i = (i + 1) % 100;
	}

	close(s_socket);
	pthread_exit(NULL);
}

void get_tracker_info(string &ip, int &port)
{
	ip = "127.0.0.1";
	port = 8001;
}

int connection_establish(string ipa, int port)
{
	char ip[64];
	strcpy(ip, ipa.c_str());
	struct sockaddr_in server_add;
	int c_socket = socket(AF_INET, SOCK_STREAM, 0);

	if(c_socket == -1)
	{
		cout << "Error while opening socket." << endl;
		return -1;
	}

	struct hostent *server = gethostbyname(ip);

	if(server == NULL)
	{
		cout << "No such host exist." << endl;
		close(c_socket);
		return -1;
	}

	bzero((char *)&server_add, sizeof(server_add));
	server_add.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&server_add.sin_addr.s_addr, server->h_length);
	server_add.sin_port = htons(port);

	if(connect(c_socket, (struct sockaddr *) &server_add, sizeof(server_add)) < 0)
	{
		cout << "Error while connecting." << endl;
		close(c_socket);
		return -1;
	}
	return c_socket;
}

void login(string cmd)
{
	//login username password
	char msg[512];
	strcpy(msg, cmd.c_str());
	send(tracker, msg, sizeof(msg), 0);
	recv(tracker, msg, sizeof(msg), 0);
	cout << msg << endl;
}

void create_user(string cmd, string ip, int port)
{
	//create_user username password ip port
	char msg[512];
	char temp[16];
	sprintf(temp, "%d", port);
	strcpy(msg, cmd.c_str());
	strcat(msg, " ");
	strcat(msg, ip.c_str());
	strcat(msg, " ");
	strcat(msg, temp);
	send(tracker, msg, sizeof(msg), 0);
	recv(tracker, msg, sizeof(msg), 0);
	cout << msg << endl;
}

void create_group(string cmd)
{
	char msg[512];
	strcpy(msg, cmd.c_str());
	send(tracker, msg, sizeof(msg), 0);
	recv(tracker, msg, sizeof(msg), 0);
	cout << msg << endl;
}

void join_group(string cmd)
{
	char msg[512];
	strcpy(msg, cmd.c_str());
	send(tracker, msg, sizeof(msg), 0);
	recv(tracker, msg, sizeof(msg), 0);
	cout << msg << endl;
}

void leave_group(string cmd)
{
	char msg[512];
	strcpy(msg, cmd.c_str());
	send(tracker, msg, sizeof(msg), 0);
	recv(tracker, msg, sizeof(msg), 0);
	cout << msg << endl;
}

void list_requests(string cmd)
{
	char msg[512];
	int size;
	strcpy(msg, cmd.c_str());
	send(tracker, msg, sizeof(msg), 0);
	recv(tracker, &size, sizeof(int), 0);
	if(size)
	{
		cout << "List of pending requestes:" << endl;
		while(size-- > 1)
		{
			recv(tracker, msg, sizeof(msg), 0);
			cout << msg << endl;
		}
		recv(tracker, msg, sizeof(msg), 0);
		cout << msg << endl;
	}
	else
	{
		recv(tracker, msg, sizeof(msg), 0);
		cout << msg << endl;
	}
}

void accept_request(string cmd)
{
	char msg[512];
	strcpy(msg, cmd.c_str());
	send(tracker, msg, sizeof(msg), 0);
	recv(tracker, msg, sizeof(msg), 0);
	cout << msg << endl;
}

void list_groups(string cmd)
{
	char msg[512];
	int size;
	strcpy(msg, cmd.c_str());
	send(tracker, msg, sizeof(msg), 0);
	recv(tracker, &size, sizeof(int), 0);
	if(size)
	{
		cout << "List of groups:" << endl;
		while(size-- > 1)
		{
			recv(tracker, msg, sizeof(msg), 0);
			cout << msg << endl;
		}
		recv(tracker, msg, sizeof(msg), 0);
		cout << msg << endl;
	}
	else
	{
		recv(tracker, msg, sizeof(msg), 0);
		cout << msg << endl;
	}
}

void list_files(string cmd)
{
	char msg[512];
	strcpy(msg, cmd.c_str());
	send(tracker, msg, sizeof(msg), 0);
	recv(tracker, msg, sizeof(msg), 0);
	cout << msg << endl;
}

void upload_file(string cmd)
{
	char msg[512];
	strcpy(msg, cmd.c_str());
	send(tracker, msg, sizeof(msg), 0);
	int ln = recv(tracker, msg, sizeof(msg), 0);
	cout << msg << endl;
}

void download_file(string cmd)
{
	char msg[512];
	strcpy(msg, cmd.c_str());
	send(tracker, msg, sizeof(msg), 0);
	int ln = recv(tracker, msg, sizeof(msg), 0);
	cout << msg << endl;
}

void logout(string cmd)
{
	char msg[512];
	strcpy(msg, cmd.c_str());
	send(tracker, msg, sizeof(msg), 0);
	int ln = recv(tracker, msg, sizeof(msg), 0);
	cout << msg << endl;
}

void show_downloads(string cmd)
{
	char msg[512];
	strcpy(msg, cmd.c_str());
	send(tracker, msg, sizeof(msg), 0);
	int ln = recv(tracker, msg, sizeof(msg), 0);
	cout << msg << endl;
}

void stop_share(string cmd)
{
	char msg[512];
	strcpy(msg, cmd.c_str());
	send(tracker, msg, sizeof(msg), 0);
	int ln = recv(tracker, msg, sizeof(msg), 0);
	cout << msg << endl;
}

int main(int argc, char const *argv[])
{
	if(argc != 3)
	{
		cout << "usage: ./peer <IP>:<PORT> <Path of tracker_info.txt>" << endl;
		return 0;
	}
	trackerfile = (char *) malloc(128 * sizeof(char));
	strcpy(trackerfile, argv[2]);
	char *ipp;
	ipp = strtok((char *)argv[1], ":");
	string ip(ipp);
	int port = atoi(strtok(NULL, ":"));
	// pthread_t s;
	// pthread_create(&s, NULL, server, (void *) &port);
 	string commmand, tip;
 	int tport;
 	get_tracker_info(tip, tport);
 	tracker = connection_establish(tip, tport);
 	cout << "successfully connected with tracker." << endl;
 	while(true)
 	{	
 		cout << "Enter commmand:";
 		getline(cin, commmand);
 		stringstream ss(commmand);
 		vector<string> arg;
 		string temp;
 		
 		while(ss >> temp)
 			arg.push_back(temp);

		if(arg[0] == "create_user")
			create_user(commmand, ip, port);
		else if(arg[0] == "login")
			login(commmand);
		else if(arg[0] == "create_group​")
			create_group(commmand);
		else if(arg[0] == "join_group")
			join_group(commmand);
		else if(arg[0] == "leave_group")
		{	
			if(arg.size() != 2)
				cout << "usage: leave_group​ <group_id>" << endl;
			else
				leave_group(commmand);
		}
		else if(arg[0] == "list_requests")
		{	
			if(arg.size() != 2)
				cout << "usage: list_requests <group_id>" << endl;
			else
				list_requests(commmand);
		}
		else if(arg[0] == "accept_request")
		{	
			if(arg.size() != 3)
				cout << "usage: accept_request​ <group_id> <user_id>" << endl;
			else
				accept_request(commmand);
		}
		else if(arg[0] == "list_groups")
		{	
			if(arg.size() != 1)
				cout << "usage: list_groups" << endl;
			else
				list_groups(commmand);
		}
		else if(arg[0] == "list_files")
		{	
			if(arg.size() != 2)
				cout << "usage: list_files​ <group_id>" << endl;
			else
				list_files(commmand);
		}
		else if(arg[0] == "upload_file")
		{	
			if(arg.size() != 3)
				cout << "usage: upload_file​ <file_path> <group_id>" << endl;
			else
				upload_file(commmand);
		}
		else if(arg[0] == "download_file")
		{	
			if(arg.size() != 4)
				cout << "usage: download_file​ <group_id> <file_name> <destination_path>" << endl;
			else
				download_file(commmand);
		}
		else if(arg[0] == "logout")
		{	
			if(arg.size() != 1)
				cout << "usage: logout" << endl;
			else
				logout(commmand);
		}
		else if(arg[0] == "show_downloads")
		{	
			if(arg.size() != 1)
				cout << "usage: Show_downloads" << endl;
			else
				show_downloads(commmand);
		}
		else if(arg[0] == "stop_share")
		{	
			if(arg.size() != 3)
				cout << "usage: stop_share <group_id> <file_name>" << endl;
			else
				stop_share(commmand);
		}
		else
			cout << "Invalid commmand!" << endl;
 	}

	return 0;
}