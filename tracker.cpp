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
	vector<string> gr;
	vector<string> fl;
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
	string size;
	vector<string> groups;
	vector<string> owners;
	vector<string> paths;
};

unordered_map<string, struct fileInfo*> files;
unordered_map<string, struct userInfo*> users;
unordered_map<string, int> login_status;
unordered_map<string, struct groupInfo*> groups;
unordered_map<string, string> filemap;

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
		send(socket, msg, strlen(msg), 0);
		return;
	}

	struct userInfo *new_user = new struct userInfo;
	new_user->passwd = arg[2];
	new_user->ip = arg[3];
	new_user->port = arg[4];
	users[arg[1]] = new_user;
	strcpy(msg, "Account successfully created!");
	send(socket, msg, strlen(msg), 0);
}

pair<string,int> login(vector<string> arg, int socket)
{
	//login_user username password
	pair<string, int> temp;
	char msg[512];
	if(users.find(arg[1]) == users.end())
	{
		strcpy(msg, "User doesn't exist!");
		send(socket, msg, strlen(msg), 0);
		temp.first = "no";
		temp.second = 0;
		return temp;
	}

	if(users[arg[1]]->passwd == arg[2])
	{
		if(login_status[arg[1]])
		{
			strcpy(msg, "User already logged in.");
			send(socket, msg, strlen(msg), 0);
			temp.first = "no";
			temp.second = 0;
		}	
		else
		{
			strcpy(msg, "Successfully logged in");
			send(socket, msg, strlen(msg), 0);
			login_status[arg[1]] = 1;
			temp.first = arg[1];
			temp.second = 1;
		}
	}
	else
	{
		strcpy(msg, "Invalid login credentials!");
		send(socket, msg, strlen(msg), 0);
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
		send(socket, msg, strlen(msg), 0);
	}
	else
	{
		struct groupInfo *new_group = new struct groupInfo[1];
		new_group->group_id = arg[1];
		new_group->owner = cu;
		new_group->members.push_back(cu);
		groups[arg[1]] = new_group;


		struct userInfo *u = users[cu];
		u->gr.push_back(arg[1]);
		strcpy(msg, "Group successfully created.");
		send(socket, msg, strlen(msg), 0);
	}
}

void join_group(vector<string> arg, string cu, int socket)
{
	char msg[512];
	if(groups.find(arg[1]) != groups.end())
	{
		int flag = 0;
		struct groupInfo *temp = groups[arg[1]];
		for(int i=0; i<temp->members.size(); i++)  if(temp->members[i] == cu) { flag = 1; break; }
		
		if(flag)
		{
			strcpy(msg, "You are already member of this group.");
			send(socket, msg, strlen(msg), 0);
		}
		else
		{
			flag = 0;
			for(int i=0; i<temp->join_requests.size(); i++) if(temp->join_requests[i] == cu) { flag = 1; break; }

			if(flag)
			{
				strcpy(msg, "Your request is already in queue.");
				send(socket, msg, strlen(msg), 0);
			}
			else
			{
				groups[arg[1]]->join_requests.push_back(cu);
				strcpy(msg, "Group join request successfully sent.");
				send(socket, msg, strlen(msg), 0);
			}
		}
	}
	else
	{
		strcpy(msg, "Group doesn't exist!");
		send(socket, msg, strlen(msg), 0);
	}
}

void leave_group(vector<string> arg, string cu, int socket)
{
	char msg[512];
	if(groups.find(arg[1]) != groups.end())
	{
		int flag = -1;
		struct groupInfo *temp = groups[arg[1]];
		for(int i=0; i<temp->members.size(); i++) if(temp->members[i] == cu) { flag = i; break; }
		if(flag >= 0)
		{
			if(cu != temp->owner)
			{
				for(auto f : users[cu]->fl)
				{
					struct fileInfo *fi = files[filemap[f]];
					if(fi->owners.size() == 1)
					{
						filemap.erase(f);
						files.erase(filemap[f]);
						free(fi);	
					}
					else
					{
						for(int i = 0; i < fi->owners.size(); i++)
						{
							if(fi->owners[i] == cu && fi->groups[i] == arg[1])
							{
								fi->owners.erase(fi->owners.begin() + i);
								fi->groups.erase(fi->groups.begin() + i);
								fi->paths.erase(fi->paths.begin() + i);

								int cnt = 0;
								for(int j = 0; j < fi->owners.size(); j++)
								{
									int k;
									for(k = fi->paths[j].size() - 1; k >= 0; i--)  if(fi->paths[j][k] == '/')  break;
									string temp = fi->paths[j].substr(k+1);
									if(temp == f)
									{
										cnt = 1;
										break;
									}			
								}
								if(!cnt)
									filemap.erase(f);
							}	
						}
					}
				}

				temp->members.erase(temp->members.begin() + flag);
				strcpy(msg, "You have leaved group ");
				strcat(msg, arg[1].c_str());
				send(socket, msg, strlen(msg), 0);
			}
			else
			{
				strcpy(msg, "Group owner cann't leave group!");
				send(socket, msg, strlen(msg), 0);
			}
		}
		else
		{
			strcpy(msg, "You are not member of the group.");
			send(socket, msg, strlen(msg), 0);
		}
	}
	else
	{
		strcpy(msg, "Group doesn't exist!");
		send(socket, msg, strlen(msg), 0);
	}
}

void list_requests(vector<string> arg, string cu, int socket)
{
	char msg[2048];
	int size = 0;
	if(groups.find(arg[1]) != groups.end())
	{
		struct groupInfo *temp = groups[arg[1]];
		if(temp->owner == cu)
		{
			size = temp->join_requests.size();
			strcpy(msg, "Join request list:");
			for(int i=0; i<size; i++)
			{
				strcat(msg,"\n");
				strcat(msg, temp->join_requests[i].c_str());
			}
			if(size == 0)
			{
				strcpy(msg, "There is no pending requests.");
				send(socket, msg, strlen(msg), 0);
			}
			else
				send(socket, msg, strlen(msg), 0);
		}
		else
		{
			strcpy(msg, "Only group owner can list requests.");
			send(socket, msg, strlen(msg), 0);
		}
	}
	else
	{
		strcpy(msg, "Group doesn't exist!");
		send(socket, msg, strlen(msg), 0);
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
			for(int i=0; i<temp->join_requests.size(); i++)  if(temp->join_requests[i] == arg[2]) { flag = i; break; }

			if(flag >= 0)
			{
				temp->join_requests.erase(temp->join_requests.begin() + flag);
				temp->members.push_back(arg[2]);
				struct userInfo *u = users[arg[2]];
				u->gr.push_back(arg[1]);
				strcpy(msg, "Join request accepted.");
				send(socket, msg, strlen(msg), 0);
			}
			else
			{
				strcpy(msg, "No such join request found!");
				send(socket, msg, strlen(msg), 0);
			}
		}
		else
		{
			strcpy(msg, "Only group owner can accept join request.");
			send(socket, msg, strlen(msg), 0);
		}
	}
	else
	{
		strcpy(msg, "Group doesn't exist!");
		send(socket, msg, strlen(msg), 0);
	}
}

void list_groups(int socket)
{
	char msg[1024];
	int size = 0;
	size = groups.size();
	bzero(msg, 1024);
	strcpy(msg, "Group list:");
	for(auto i : groups)
	{
		strcat(msg, "\n");
		strcat(msg, i.first.c_str());
	}
	if(size == 0)
	{
		strcpy(msg, "No group exist!");
		send(socket, msg, strlen(msg), 0);
	}
	else
		send(socket, msg, strlen(msg), 0);
}

int logout(string cu, int socket)
{
	char msg[512];
	login_status[cu] = 0;
	strcpy(msg, "successfully logged out.");
	send(socket, msg, strlen(msg), 0);
	return 0;
}

void upload_file(vector<string> arg, string cu, int socket)
{
	//upload_file <filepath> <groupId> <hash> <size>
	char msg[512];
	if(groups.find(arg[2]) != groups.end())
	{
		int flag = 0;
		struct userInfo *u = users[cu];
		for(int i=0; i<u->gr.size(); i++)  if(arg[2] == u->gr[i]) {  flag = 1; break; }
		
		if(flag)
		{
			int i;
			for(i = arg[1].size() - 1; i >= 0; i--)  if(arg[1][i] == '/')  break;
			string temp = arg[1].substr(i+1);
			if(files.find(arg[3]) == files.end())
			{
				struct fileInfo *ff = new fileInfo;
			
				ff->size = arg[4];
				ff->owners.push_back(cu);
				ff->groups.push_back(arg[2]);
				ff->paths.push_back(arg[1]);
				files[arg[3]] = ff;
				users[cu]->fl.push_back(temp);
				filemap[temp] = arg[3];
				strcpy(msg, "File successfully uploaded.");
				send(socket, msg, strlen(msg), 0);
			}
			else
			{
				if(filemap.find(temp) == filemap.end())
				{
					filemap[temp] = arg[3];
					struct fileInfo *f = files[arg[3]];
					f->owners.push_back(cu);
					f->groups.push_back(arg[2]);
					f->paths.push_back(arg[1]);
					users[cu]->fl.push_back(temp);
					strcpy(msg, "File successfully uploaded.");
					send(socket, msg, strlen(msg), 0);
				}
				else
				{
					int flag = 0;
					struct fileInfo *f = files[arg[3]];
					for(int i=0; i<f->paths.size(); i++)  if(arg[1] == f->paths[i] && f->owners[i] == cu) {  flag = i; break; }
					if(flag)
					{
						strcpy(msg, "File already exist!");
						send(socket, msg, strlen(msg), 0);
					}
					else
					{
						f->owners.push_back(cu);
						f->groups.push_back(arg[2]);
						f->paths.push_back(arg[1]);
						strcpy(msg, "File successfully uploaded.");
						send(socket, msg, strlen(msg), 0);
					}
				}
			}	
		}
		else
		{
			strcpy(msg, "You are not part of this group!");
			send(socket, msg, strlen(msg), 0);
		}
	}
	else
	{
		strcpy(msg, "Group doesn't exist!");
		send(socket, msg, strlen(msg), 0);
	}
}

void list_files(vector<string> arg, int socket)
{
	char msg[51200];
	char temp[512];
	if(groups.find(arg[1]) != groups.end())
	{
		if(filemap.size())
		{
			int cnt = 0;
			strcpy(msg, "File list:");
			for(auto i : filemap)
			{
				int flag = 0;
				struct fileInfo *f = files[i.second];
				strcpy(temp, i.first.c_str());
				strcat(temp, " ");

				for(int j = 0; j < f->groups.size(); j++)
				{
					if(f->groups[j] == arg[1] && login_status[f->owners[j]])
					{
						strcat(temp, f->owners[j].c_str());
						strcat(temp, " ");
						flag++;
					}
				}
				if(flag)
				{
					strcat(msg,"\n");
					strcat(msg,temp);
					cnt++;
				}
			}
			if(cnt)
				send(socket, msg, strlen(msg), 0);
			else
			{
				strcpy(msg, "No files exist!");
				send(socket, msg, strlen(msg), 0);
			}
		}
		else
		{
			strcpy(msg, "No files exist!");
			send(socket, msg, strlen(msg), 0);
		}
	}
	else
	{
		strcpy(msg, "Group doesn't exist!");
		send(socket, msg, strlen(msg), 0);
	}	
}

void download_file(vector<string> arg, string cu, int socket)
{
	// download_file​ <group_id> <file_name> <destination_path>
	char msg[51200];
	if(groups.find(arg[1]) != groups.end())
	{
		if(filemap.find(arg[2]) != filemap.end())
		{
			int flag = 0;
			struct userInfo *u = users[cu];
			for(int i=0; i<u->gr.size(); i++)  if(arg[1] == u->gr[i]) {  flag = 1; break; }

			if(flag)
			{
				string temp;
				struct fileInfo *f = files[filemap[arg[2]]];
				temp = f->size + "/" + filemap[arg[2]];
				strcpy(msg, temp.c_str());
				send(socket, msg, strlen(msg), 0);
				sleep(0.1);
				strcpy(msg, "");
				for(int i = 0; i < f->owners.size(); i++)
				{
					if(arg[1] == f->groups[i] && login_status[f->owners[i]])
					{
						temp = f->owners[i] + " " + users[f->owners[i]]->port + " " + users[f->owners[i]]->ip + " " + f->paths[i] + "\n";
						strcat(msg, temp.c_str());
					}
				}
				send(socket, msg, strlen(msg), 0);
			}
			else
			{
				strcpy(msg, "You are not part of this group!");
				send(socket, msg, strlen(msg), 0);
			}
		}
		else
		{
			strcpy(msg, "File doesn't exist!");
			send(socket, msg, strlen(msg), 0);
		}
	}
	else
	{
		strcpy(msg, "Group doesn't exist!");
		send(socket, msg, strlen(msg), 0);
	}
}

void *request_handler(void *com_socket)
{
	int socket = *(int *) com_socket;
	int logged = 0;
	string curr_user;
	char msg[51200];
	while(true)
	{
		bzero(msg, 51200);
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
				send(socket, msg, strlen(msg), 0);
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
				send(socket, msg, strlen(msg), 0);
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
			if(arg[0] == "create_group")
			{	
				if(arg.size() != 2)
				{
					strcpy(msg, "uasge: create_group​ <group_id>");
					send(socket, msg, strlen(msg), 0);
				}
				else
					create_group(arg, curr_user, socket);
			}
			else if(arg[0] == "join_group")
			{	
				if(arg.size() != 2)
				{
					strcpy(msg, "usage: join_group​ <group_id>");
					send(socket, msg, strlen(msg), 0);
				}
				else
					join_group(arg, curr_user, socket);
			}
			else if(arg[0] == "leave_group")
			{	
				if(arg.size() != 2)
				{
					strcpy(msg, "usage: leave_group​ <group_id>");
					send(socket, msg, strlen(msg), 0);
				}
				else
					leave_group(arg, curr_user, socket);
			}
			else if(arg[0] == "list_requests")
			{	
				if(arg.size() != 2)
				{
					strcpy(msg, "usage: list_requests <group_id>");
					send(socket, msg, strlen(msg), 0);
				}
				else
					list_requests(arg, curr_user, socket);
			}
			else if(arg[0] == "accept_request")
			{	
				if(arg.size() != 3)
				{
					strcpy(msg, "usage: accept_request​ <group_id> <user_id>");
					send(socket, msg, strlen(msg), 0);
				}
				else
					accept_request(arg, curr_user, socket);
			}
			else if(arg[0] == "list_groups")
			{	
				if(arg.size() != 1)
				{
					strcpy(msg, "usage: list_groups");
					send(socket, msg, strlen(msg), 0);
				}
				else
					list_groups(socket);
			}
			else if(arg[0] == "list_files")
			{	
				if(arg.size() != 2)
				{
					strcpy(msg, "usage: list_files​ <group_id>");
					send(socket, msg, strlen(msg), 0);
				}
				else
					list_files(arg, socket);
			}
			else if(arg[0] == "upload_file")
			{	
				if(arg.size() != 5)
				{
					strcpy(msg, "usage: upload_file​ <file_path> <group_id>");
					send(socket, msg, strlen(msg), 0);
				}
				else
					upload_file(arg, curr_user, socket);
			}
			else if(arg[0] == "download_file")
			{	
				if(arg.size() != 4)
				{
					strcpy(msg, "usage: download_file​ <group_id> <file_name> <destination_path>");
					send(socket, msg, strlen(msg), 0);
				}
				else
					download_file(arg, curr_user, socket);
			}
			else if(arg[0] == "logout")
			{	
				if(arg.size() != 1)
				{
					strcpy(msg, "usage: logout");
					send(socket, msg, strlen(msg), 0);
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
			{
				strcpy(msg, "Invalid commmand!");
				send(socket, msg, strlen(msg), 0);
			}
		}
		else
		{
			strcpy(msg, "Please login. If you don't have account? register.");
			send(socket, msg, strlen(msg), 0);
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
