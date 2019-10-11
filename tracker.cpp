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
#include <signal.h>
#include <pthread.h> 
#include<unordered_map>
using namespace std;

int s_socket, status = 0;
struct userInfo
{
	string passwd;
	string ip;
	string port;
};

struct groupInfo
{
	string group_id;
	string owner;
	vector<string> members;
	vector<string> join_requests;
};

struct fileInfo
{
	string name;
	string path;
	string hash;
	string size;
	vector<string> groups;
	vector<string> owners;
};

unordered_map<string, struct fileInfo*> files;
unordered_map<string, struct userInfo*> users;
unordered_map<string, int> login_status;
unordered_map<string, struct groupInfo*> groups;

void my_handler(int signum)
{
    if(signum == SIGUSR1)
    	close(s_socket);
}

void create_user(vector<string> arg, int socket)
{
	//create_user username password ip port
	char msg[128];
	if(users.find(arg[1]) != users.end())
	{
		strcpy(msg, "User already exist!");
		send(socket, msg, sizeof(msg), 0);
		return;
	}

	struct userInfo *new_user = (struct userInfo *) malloc(sizeof(struct userInfo));
	new_user->passwd = arg[2];
	new_user->ip = arg[3];
	new_user->port = arg[4];
	users[arg[1]] = new_user;
	strcpy(msg, "Account successfully created!");
	send(socket, msg, sizeof(msg), 0);
}

pair<string,int> login(vector<string> arg, int socket)
{
	//login_user username password
	pair<string, int> temp;
	char msg[512];
	if(users.find(arg[1]) == users.end())
	{
		strcpy(msg, "User doesn't exist!");
		send(socket, msg, sizeof(msg), 0);
		temp.first = "no";
		temp.second = 0;
		return temp;
	}

	if(users[arg[1]]->passwd == arg[2])
	{
		if(login_status[arg[1]])
		{
			strcpy(msg, "User already logged in.");
			send(socket, msg, sizeof(msg), 0);
			temp.first = "no";
			temp.second = 0;
		}	
		else
		{
			strcpy(msg, "Successfully logged in");
			send(socket, msg, sizeof(msg), 0);
			login_status[arg[1]] = 1;
			temp.first = arg[1];
			temp.second = 1;
		}
	}
	else
	{
		strcpy(msg, "Invalid login credentials!");
		send(socket, msg, sizeof(msg), 0);
		temp.first = "no";
		temp.second = 0;
	}
	return temp;
}

void create_group(vector<string> arg, string cu, int socket)
{
	char msg[512];
	if(groups.find(arg[1]) != groups.end())
	{
		strcpy(msg, "Group already exist!");
		send(socket, msg, sizeof(msg), 0);
	}
	else
	{
		struct groupInfo *new_group = (struct groupInfo *) malloc(sizeof(struct groupInfo));
		new_group->group_id = arg[1];
		new_group->owner = cu;
		new_group->members.push_back(cu);
		groups[arg[1]] = new_group;
		strcpy(msg, "Group successfully created.");
		send(socket, msg, sizeof(msg), 0);
	}
}

void join_group(vector<string> arg, string cu, int socket)
{
	char msg[512];
	if(groups.find(arg[1]) != groups.end())
	{
		int flag = 0;
		struct groupInfo *temp = groups[arg[1]];
		for(int i=0; i<temp->members.size(); i++)
		{
			if(temp->members[i] == cu)
			{
				flag = 1;
				break;
			}
		}
		if(flag)
		{
			strcpy(msg, "You are already member of this group.");
			send(socket, msg, sizeof(msg), 0);
		}
		else
		{
			flag = 0;
			for(int i=0; i<temp->join_requests.size(); i++)
			{
				if(temp->join_requests[i] == cu)
				{
					flag = 1;
					break;
				}
			}
			if(flag)
			{
				strcpy(msg, "Your request is already in queue.");
				send(socket, msg, sizeof(msg), 0);
			}
			else
			{
				groups[arg[1]]->join_requests.push_back(cu);
				strcpy(msg, "Group join request successfully sent.");
				send(socket, msg, sizeof(msg), 0);
			}
		}
	}
	else
	{
		strcpy(msg, "Group doesn't exist!");
		send(socket, msg, sizeof(msg), 0);
	}
}

void leave_group(vector<string> arg, string cu, int socket)
{
	char msg[512];
	if(groups.find(arg[1]) != groups.end())
	{
		int flag = -1;
		struct groupInfo *temp = groups[arg[1]];
		for(int i=0; i<temp->members.size(); i++)
		{
			if(temp->members[i] == cu)
			{
				flag = i;
				break;
			}
		}
		if(flag >= 0)
		{
			temp->members.erase(temp->members.begin() + flag);
			strcpy(msg, "You have leaved group ");
			strcpy(msg, arg[1].c_str());
			send(socket, msg, sizeof(msg), 0);
		}
		else
		{
			strcpy(msg, "You are not member of the group.");
			send(socket, msg, sizeof(msg), 0);
		}
	}
	else
	{
		strcpy(msg, "Group doesn't exist!");
		send(socket, msg, sizeof(msg), 0);
	}
}

void list_requests(vector<string> arg, string cu, int socket)
{
	char msg[512];
	int size = 0;
	if(groups.find(arg[1]) != groups.end())
	{
		struct groupInfo *temp = groups[arg[1]];
		if(temp->owner == cu)
		{
			size = temp->join_requests.size();
			send(socket, &size, sizeof(int), 0);
			for(int i=0; i<size; i++)
			{
				strcpy(msg, temp->join_requests[i].c_str());
				send(socket, msg, sizeof(msg), 0);
			}
			if(size == 0)
			{
				strcpy(msg, "There is no pending requests.");
				send(socket, msg, sizeof(msg), 0);
			}
		}
		else
		{
			send(socket, &size, sizeof(int), 0);
			strcpy(msg, "Only group owner can list requests.");
			send(socket, msg, sizeof(msg), 0);
		}
	}
	else
	{
		send(socket, &size, sizeof(int), 0);
		strcpy(msg, "Group doesn't exist!");
		send(socket, msg, sizeof(msg), 0);
	}
}

void accept_request(vector<string> arg, string cu, int socket)
{
	char msg[512];
	if(groups.find(arg[1]) != groups.end())
	{
		int flag = -1;
		struct groupInfo *temp = groups[arg[1]];
		if(temp->owner == cu)
		{
			for(int i=0; i<temp->join_requests.size(); i++)
			{
				if(temp->join_requests[i] == arg[2])
				{
					flag = i;
					break;
				}
			}
			if(flag >= 0)
			{
				temp->join_requests.erase(temp->join_requests.begin() + flag);
				temp->members.push_back(arg[2]);
				strcpy(msg, "Join request accepted.");
				send(socket, msg, sizeof(msg), 0);
			}
			else
			{
				strcpy(msg, "No such join request found!");
				send(socket, msg, sizeof(msg), 0);
			}
		}
		else
		{
			strcpy(msg, "Only group owner can accept join request.");
			send(socket, msg, sizeof(msg), 0);
		}
	}
	else
	{
		strcpy(msg, "Group doesn't exist!");
		send(socket, msg, sizeof(msg), 0);
	}
}

void list_groups(int socket)
{
	char msg[512];
	int size = 0;
	size = groups.size();
	send(socket, &size, sizeof(int), 0);
	for(auto i : groups)
	{
		strcpy(msg, i.first.c_str());
		send(socket, msg, sizeof(msg), 0);
	}
	if(size == 0)
	{
		strcpy(msg, "No group exist!");
		send(socket, msg, sizeof(msg), 0);
	}
}

int logout(string cu, int socket)
{
	char msg[512];
	login_status[cu] = 0;
	strcpy(msg, "successfully logged out.");
	send(socket, msg, sizeof(msg), 0);
	return 0;
}

void upload_file(vector<string> arg, string cu, int socket)
{

}

void *request_handler(void *com_socket)
{
	int socket = *(int *) com_socket;
	char msg[512];
	int logged = 0;
	string curr_user;
	while(true)
	{
		char msg[512];
		recv(socket, msg, sizeof(msg), 0);
		string commmand(msg);
		stringstream ss(commmand);
 		vector<string> arg;
 		string temp;
 		while(ss >> temp)
 			arg.push_back(temp);

		if(arg[0] == "create_user")
		{
			if(arg.size() != 5)
			{
				strcpy(msg, "usage: create_user​ <user_id> <passwd>");
				send(socket, msg, sizeof(msg), 0);
			}
			else
				create_user(arg, socket);
		}
		else if(arg[0] == "login")
		{
			pair<string, int> temp;
			temp.first = "no";
			temp.second = 0;
			if(arg.size() != 3)
			{
				strcpy(msg, "usage: login <user_id> <passwd>");
				send(socket, msg, sizeof(msg), 0);
			}
			else
				temp = login(arg, socket);
			if(temp.second)
			{
				curr_user = temp.first;
				logged = 1;
			}
		}
		else if(logged)
		{
			if(arg[0] == "create_group​")
			{	
				if(arg.size() != 2)
				{
					strcpy(msg, "uasge: create_group​ <group_id>");
					send(socket, msg, sizeof(msg), 0);
				}
				else
					create_group(arg, curr_user, socket);
			}
			else if(arg[0] == "join_group")
			{	
				if(arg.size() != 2)
				{
					strcpy(msg, "usage: join_group​ <group_id>");
					send(socket, msg, sizeof(msg), 0);
				}
				else
					join_group(arg, curr_user, socket);
			}
			else if(arg[0] == "leave_group")
			{	
				if(arg.size() != 2)
				{
					strcpy(msg, "usage: leave_group​ <group_id>");
					send(socket, msg, sizeof(msg), 0);
				}
				else
					leave_group(arg, curr_user, socket);
			}
			else if(arg[0] == "list_requests")
			{	
				if(arg.size() != 2)
				{
					strcpy(msg, "usage: list_requests <group_id>");
					send(socket, msg, sizeof(msg), 0);
				}
				else
					list_requests(arg, curr_user, socket);
			}
			else if(arg[0] == "accept_request")
			{	
				if(arg.size() != 3)
				{
					strcpy(msg, "usage: accept_request​ <group_id> <user_id>");
					send(socket, msg, sizeof(msg), 0);
				}
				else
					accept_request(arg, curr_user, socket);
			}
			else if(arg[0] == "list_groups")
			{	
				if(arg.size() != 1)
				{
					strcpy(msg, "usage: list_groups");
					send(socket, msg, sizeof(msg), 0);
				}
				else
					list_groups(socket);
			}
			// else if(arg[0] == "list_files")
			// {	
			// 	if(arg.size() != 2)
			// 		cout << "usage: list_files​ <group_id>" << endl;
			// 	else
			// 		list_files(commmand);
			// }
			else if(arg[0] == "upload_file")
			{	
				if(arg.size() != 5)
				{
					strcpy(msg, "usage: upload_file​ <file_path> <group_id>");
					send(socket, msg, sizeof(msg), 0);
				}
				else
					upload_file(arg, curr_user, socket);
			}
			// else if(arg[0] == "download_file")
			// {	
			// 	if(arg.size() != 4)
			// 		cout << "usage: download_file​ <group_id> <file_name> <destination_path>" << endl;
			// 	else
			// 		download_file(commmand);
			// }
			else if(arg[0] == "logout")
			{	
				if(arg.size() != 1)
				{
					strcpy(msg, "usage: logout");
					send(socket, msg, sizeof(msg), 0);
				}
				else
					logged = logout(curr_user, socket);
			}
			// else if(arg[0] == "show_downloads")
			// {	
			// 	if(arg.size() != 1)
			// 		cout << "usage: Show_downloads" << endl;
			// 	else
			// 		show_downloads(commmand);
			// }
			// else if(arg[0] == "stop_share")
			// {	
			// 	if(arg.size() != 3)
			// 		cout << "usage: stop_share <group_id> <file_name>" << endl;
			// 	else
			// 		stop_share(commmand);
			// }
			else
				cout << "Invalid commmand!" << endl;
		}
		else
		{
			strcpy(msg, "Please login. If you don't have account? register.");
			send(socket, msg, sizeof(msg), 0);
		}

	}
}

void *server(void *p)
{
	int *por = (int *) p;
	int port = *por;
	s_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	if(s_socket == -1)
	{
		cout << "[-] Error while opening socket." << endl;
		status = 1;
		return NULL;
	}

	signal(SIGUSR1, my_handler);

	struct sockaddr_in server_add;
	bzero((char *)&server_add, sizeof(server_add));
	
	server_add.sin_family = AF_INET;
	server_add.sin_addr.s_addr = INADDR_ANY;
	server_add.sin_port = htons(port);

	int err = bind(s_socket, (struct sockaddr *) &server_add, sizeof(server_add));

	if(err < 0)
	{
		cout << "[-] Binding failed."<< endl;
		status = 1;
		return NULL;
	}

	listen(s_socket, 1000);

	cout << "[+] Tracker successfully started" << endl;
	int i=0;
	int com_socket[1000];
	while(true)
	{
		struct sockaddr_in client_add;
		socklen_t len = sizeof(client_add);
			com_socket[i] = accept(s_socket, (struct sockaddr *)&client_add, &len);

		if(com_socket < 0)
			cout << "[-] Error while accept!" << endl;

		pthread_t client_handler;
		pthread_create(&client_handler, NULL, request_handler, &com_socket[i]);
		i = (i + 1) % 1000;
	}

	close(s_socket);
	pthread_exit((void *) 0);
}


int main(int argc, char const *argv[])
{
	if(argc != 3)
	{
		cout << "usage: ./tracker​ tracker_info.txt ​ tracker_no" << endl;
		return 0;	
	}

	int port;
	if(atoi(argv[2]) == 1)
 		port = 8001;
 	else if(atoi(argv[2]) == 2)
 		port = 8002;
 	else
 	{
 		cout << "Tracker_no should be 1 or 2:" << endl;
 		return 0;
 	}	

	pthread_t s;
	pthread_create(&s, NULL, server, (void *) &port);
	sleep(1);
	if(status)
		return 0;
	string com;
	while(true)
	{
		cout << "Enter command:";
		cin >> com;
		if(com == "quit")
		{
			pthread_kill(s, SIGUSR1);
			cout << "Bye" << endl;
			break;
		}
		else
			cout << "Invalid command!" << endl;
	}
	return 0;
}
