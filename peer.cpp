#include<iostream>
#include<vector>
#include<unordered_map>
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
#include<algorithm>
#include <openssl/sha.h>
using namespace std;

int tracker;
char *trackerfile;

struct trdarg
{
	string name;
	int port;
	string ip;
	string path;
	vector<int> chunks;
	string dpath;
	string hash;
	int cc;
};

unordered_map<string, string> files;

void *fileshare(void *com_socket)
{
	int socket = *(int *) com_socket;
	char msg[51200];
	bzero(msg, 51200);
	recv(socket, msg, sizeof(msg), 0);
	string commmand(msg);
	stringstream ss(commmand);
	vector<string> arg;
	string temp;
	while(ss >> temp)
 		arg.push_back(temp);

 	if(arg[0] == "get_chunk_details")
 	{
 		strcpy(msg, files[arg[1]].c_str());
		send(socket, msg, strlen(msg), 0);
 	}
 	if(arg[0] == "download")
 	{
		int chnk_no;
		char buf[51200];
		FILE *fp = fopen(arg[1].c_str(),"rb+");		
		recv(socket, &chnk_no, sizeof(int), 0);
		int st = chnk_no * 51200;
		fseek(fp, st, SEEK_SET);
		bzero(buf, 51200);
		int ln = fread(buf, sizeof(char), sizeof(buf), fp);
		send(socket, buf, ln, 0);
		fclose(fp);
 	}
}

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
	int i = 0;
	int com_socket[1000];
	while(true)
	{
		struct sockaddr_in client_add;
		socklen_t len = sizeof(client_add);
		 
		com_socket[i] = accept(s_socket, (struct sockaddr *)&client_add, &len);

		if(com_socket[i] < 0)
			cout << "Error while accept!" << endl;

		pthread_t request_handler;
		pthread_create(&request_handler, NULL, fileshare, &com_socket[i]);
		i = (i + 1) % 1000;
	}

	close(s_socket);
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
	send(tracker, msg, strlen(msg), 0);
	bzero(msg, 512);
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
	send(tracker, msg, strlen(msg), 0);
	bzero(msg, 512);
	recv(tracker, msg, sizeof(msg), 0);
	cout << msg << endl;
}

void create_group(string cmd)
{
	char msg[512];
	strcpy(msg, cmd.c_str());
	send(tracker, msg, strlen(msg), 0);
	bzero(msg, 512);
	recv(tracker, msg, sizeof(msg), 0);
	cout << msg << endl;
}

void join_group(string cmd)
{
	char msg[512];
	strcpy(msg, cmd.c_str());
	send(tracker, msg, strlen(msg), 0);
	bzero(msg, 512);
	recv(tracker, msg, sizeof(msg), 0);
	cout << msg << endl;
}

void leave_group(string cmd)
{
	char msg[512];
	strcpy(msg, cmd.c_str());
	send(tracker, msg, strlen(msg), 0);
	bzero(msg, 512);
	recv(tracker, msg, sizeof(msg), 0);
	cout << msg << endl;
}

void list_requests(string cmd)
{
	char msg[2048];
	strcpy(msg, cmd.c_str());
	send(tracker, msg, strlen(msg), 0);
	bzero(msg, 2048);
	recv(tracker, msg, sizeof(msg), 0);
	cout << msg << endl;
}

void accept_request(string cmd)
{
	char msg[512];
	strcpy(msg, cmd.c_str());
	send(tracker, msg, strlen(msg), 0);
	bzero(msg, 512);
	recv(tracker, msg, sizeof(msg), 0);
	cout << msg << endl;
}

void list_groups(string cmd)
{
	char msg[1024];
	strcpy(msg, cmd.c_str());
	send(tracker, msg, strlen(msg), 0);
	bzero(msg, 1024);
	recv(tracker, msg, sizeof(msg), 0);
	cout << msg << endl;
}

void list_files(string cmd)
{
	char msg[51200];
	strcpy(msg, cmd.c_str());
	send(tracker, msg, strlen(msg), 0);
	bzero(msg, 51200);
	recv(tracker, msg, sizeof(msg), 0);
	cout << msg << endl;
}

void upload_file(string cmd)
{
	//upload_file <filepath> <groupId> <hash> <size>
	stringstream ss(cmd);
	string fpath;
	ss >> fpath >> fpath;
	FILE *fp = fopen(fpath.c_str(), "rb+");
	if(fp == NULL)
	{
		cout << "Invalid file path!" << endl;
		return;
	}
	const unsigned char str[] = "Original String";
  	unsigned char hash[SHA_DIGEST_LENGTH]; // == 20

  	SHA1(str, sizeof(str) - 1, hash);

	fseek(fp, 0, SEEK_END);
	long long size = ftell(fp);
	fclose(fp);
	char temp[32];
	sprintf(temp, "%lld", size);
	char msg[51200];
	strcpy(msg, cmd.c_str());
	strcat(msg, " ");
	strcat(msg, fpath.c_str());
	strcat(msg, "jfjdhajg738sdgjhjafsfkj837rjgjgfd73rjgjhfgjasjdgfjg387rufjdgfhgdajsgfajfd3827rfgjgdsa73");
	strcat(msg, " ");
	strcat(msg, temp);
	send(tracker, msg, strlen(msg), 0);
	bzero(msg, 51200);
	recv(tracker, msg, sizeof(msg), 0);
	if(!strcmp(msg,"File successfully uploaded."))
	{
		long long chunks = size / 51200;
		string list;
		int i = 0;
		list = to_string(i++);
		while(i <= chunks)
		{
			list = list + " " + to_string(i);
			i++;
		}
		files[fpath] = list;
	}
	cout << msg << endl;
}

void *fenner(void *rudoff)
{
	struct trdarg *richard = (struct trdarg *) rudoff;
	char msg[256];
	char chnk[51200];
	int socket = connection_establish(richard->ip, richard->port);
	strcpy(msg, "get_chunk_details ");
	strcat(msg, richard->path.c_str());
	send(socket, msg, strlen(msg), 0);
	bzero(chnk, 51200);
	recv(socket, chnk, sizeof(chnk), 0);

	string temp = chnk;
	stringstream ss(temp);
	int c;
	while(ss >> c)
		richard->chunks.push_back(c);
}

bool cmp(struct trdarg *ow1, struct trdarg *ow2)
{
	if(ow1->chunks.size() > ow2->chunks.size())
		return false;
	else
		return true;
}

void *fc_download(void *rudoff)
{
	struct trdarg *richard = (struct trdarg *) rudoff;
	FILE *fp = fopen(richard->dpath.c_str(), "rb+");
	if(fp == NULL)
	{
		cout << "Invalid destination path!" << endl; 
	}
	int st = richard->cc * 51200;
	fseek(fp, st, SEEK_SET);
	int socket = connection_establish(richard->ip, richard->port);
	if(socket == -1)
	{
		cout << "Error in connection" << endl;
		return NULL;
	}
	char msg[512];
	strcpy(msg, "download ");
	strcat(msg, richard->path.c_str());
	send(socket, &richard->cc, sizeof(int), 0);
	char buf[51200];
	bzero(buf, 51200);
	int ln = recv(socket, buf, sizeof(buf), 0);
	fwrite(buf, sizeof(char), ln, fp);
	cout << richard->cc << " successfully downloaded from " << richard->name << endl;
	fclose(fp);
}

void download_file(string cmd)
{
	char msg[51200];
	int filesize;
	strcpy(msg, cmd.c_str());
	send(tracker, msg, strlen(msg), 0);
	bzero(msg, 51200);
	recv(tracker, msg, sizeof(msg), 0);
	vector<string> o;
	vector<struct trdarg*> owners;
	string h;

	if(strstr(msg, "/"))
	{
		filesize = atoi(strtok(msg,"/"));
		h = strtok(NULL,"/");
		bzero(msg, 51200);	
		recv(tracker, msg, sizeof(msg), 0);
		char *cptr;
		cptr = strtok(msg, "\n");
		while(cptr)
		{
			string temp = cptr;
			o.push_back(temp);
			cptr = strtok(NULL, "\n");
		}
	}
	else
	{
		cout << msg << endl;
		return;
	}

	cout << "Downloading file..." << endl;
	char *fname = strtok((char *)cmd.c_str(), " ");
	fname = strtok(NULL, " ");
	fname = strtok(NULL, " ");
	fname = strtok(NULL, " ");
	for(auto i : o)
	{
		struct trdarg *t = new struct trdarg;
		stringstream ss(i);
 		ss >> t->name;
 		ss >> t->port;
 		ss >> t->ip;
 		ss >> t->path;
 		t->dpath = fname;
 		t->hash = h;
 		owners.push_back(t); 		
	}

	for(int i = 0; i < owners.size(); i++)
	{
		pthread_t steven;
		pthread_create(&steven, NULL, fenner, owners[i]);
		pthread_join(steven, NULL);
	}

	sort(owners.begin(), owners.end(), cmp);

	FILE *fp = fopen(fname, "wb");
	int temp = filesize;
	int ctotal = filesize / 51200;
	char ch[2];
	strcpy(ch, "0");
	while(temp > 0)
	{
		fwrite(ch, sizeof(char), 1, fp);
		temp--;
	}
	fclose(fp);

	vector<int> df(ctotal+1, 0);
	for(int i = 0; i <= ctotal; i++)
	{ 
		int flag = 1;
		if(!df[i])
		{	
			vector<pthread_t> thrd;
			for(int j = 0; j < owners.size(); j++)
			{
				if(owners[j]->chunks.size() > i)
				{
					if(!df[owners[j]->chunks[i]])
					{
						flag = 0;
						df[owners[j]->chunks[i]] = 1;
						owners[j]->cc = owners[j]->chunks[i];
						pthread_t richard;
						pthread_create(&richard,NULL ,fc_download, owners[j]);
						thrd.push_back(richard);
					}
				}
			}
			for(auto i : thrd)
				pthread_join(i, NULL);
		}
		if(flag)
			break;
	}

}

void logout(string cmd)
{
	char msg[512];
	strcpy(msg, cmd.c_str());
	send(tracker, msg, strlen(msg), 0);
	bzero(msg, 512);
	recv(tracker, msg, sizeof(msg), 0);
	cout << msg << endl;
}

void show_downloads(string cmd)
{
	char msg[512];
	strcpy(msg, cmd.c_str());
	send(tracker, msg, strlen(msg), 0);
	bzero(msg, 512);
	recv(tracker, msg, sizeof(msg), 0);
	cout << msg << endl;
}

void stop_share(string cmd)
{
	char msg[512];
	strcpy(msg, cmd.c_str());
	send(tracker, msg, strlen(msg), 0);
	bzero(msg, 512);
	recv(tracker, msg, sizeof(msg), 0);
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
	pthread_t s;
	pthread_create(&s, NULL, server, (void *) &port);
 	string commmand, tip;
 	int tport;
 	get_tracker_info(tip, tport);
 	tracker = connection_establish(tip, tport);
 	if(tracker > 0)
 	{
 		cout << "successfully connected with tracker!" << endl;
 	}

 	while(true)
 	{	
 		cout << "Enter commmand:";
 		getline(cin, commmand);
 		if(commmand.size() == 0)
 			continue;

 		stringstream ss(commmand);
 		vector<string> arg;
 		string temp;
 		while(ss >> temp)
 			arg.push_back(temp);

		if(arg[0] == "create_user")
			create_user(commmand, ip, port);
		else if(arg[0] == "login")
			login(commmand);
		else if(arg[0] == "create_group")
			create_group(commmand);
		else if(arg[0] == "join_group")
			join_group(commmand);
		else if(arg[0] == "leave_group")
			leave_group(commmand);
		else if(arg[0] == "list_requests")
			list_requests(commmand);
		else if(arg[0] == "accept_request")
			accept_request(commmand);
		else if(arg[0] == "list_groups")
			list_groups(commmand);
		else if(arg[0] == "list_files")
			list_files(commmand);
		else if(arg[0] == "upload_file")
			upload_file(commmand);
		else if(arg[0] == "download_file")
			download_file(commmand);
		else if(arg[0] == "logout")
			logout(commmand);
		else if(arg[0] == "show_downloads")
			show_downloads(commmand);
		else if(arg[0] == "stop_share")
			stop_share(commmand);
		else
		{
			char msg[512];
			strcpy(msg, commmand.c_str());
			send(tracker, msg, strlen(msg), 0);
			bzero(msg, 512);
			recv(tracker, msg, sizeof(msg), 0);
			cout << msg << endl;
		}
 	}

	pthread_exit(NULL);
	return 0;
}