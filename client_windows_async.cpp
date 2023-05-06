#include<iostream>
#include<cstring>
#include<winsock2.h>
#include<Windows.h>
#include<future>

#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable : 4996)

#define STOPSYMBOL "##stop##"
#define servername "ub.arok01.tk"
#define serverport 8920
#define MAXBUFFSIZE 2048

using namespace std;

void recvMS(SOCKET);
void sendMS(SOCKET);

int main()
{
	//DNS解析
	/*struct hostent* server = gethostbyname(servername);
	if (!server)
	{
		cout << "gethostbyname错误" << endl;
		return -1;
	}*/

	WSADATA wsa_data;					//winsock库
	//创建和初始化套接字
	SOCKET client;						//套接字
	struct sockaddr_in remote_addr;		//网络地址
	memset(&remote_addr, 0, sizeof(remote_addr));
	remote_addr.sin_family = AF_INET;
	//remote_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)server->h_addr_list[0]));
	remote_addr.sin_addr.s_addr = inet_addr("107.173.154.85");
	remote_addr.sin_port = htons(serverport);

	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)	//初始化Winsock2.2库
	{
		cout << "WSAStartup初始化失败"<< endl;
		return -1;
	}
		
	if ((client = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)		//创建ipv4+tcp套接字,失败判定为INVALID_SOCKET
	{
		cout << "套接字创建失败" << endl;
		return -1;
	}

	cout << "正在尝试连接到服务器" << servername <<"......" << endl;
	for (int times = 10; times; times--)
	{
		if (connect(client, (struct sockaddr*)&remote_addr, sizeof(struct sockaddr)) == SOCKET_ERROR)	//绑定服务器网络地址,失败判定为SOCKET_ERROR
		{
			cout << "服务器连接失败，将在5秒后重试连接，距离认定失败还有" << times - 1 << "次" << endl;
			Sleep(5000);
		}
		else
		{
			cout << "成功连接到服务器" << endl << "Welcome to chatppt" << endl;
			break;
		}
	}

	//异步通信
	future<void> f_recv = async(launch::async, [client]() {recvMS(client);});
	future<void> f_send = async(launch::async, [client]() {sendMS(client); });
	
	f_recv.wait();
	f_send.wait();
	//结束操作
	closesocket(client);
	WSACleanup();
	return 0;
}

void recvMS(SOCKET client)
{
	int res_NUM = 0;
	int res_len = 0;
	char recvMS[MAXBUFFSIZE];
	recvMS[0] = 0;
	while (1)
	{
		if ((res_len = recv(client, recvMS, MAXBUFFSIZE, 0)) > 0)
		{
			recvMS[res_len] = 0;
			cout << "Server:"<<recvMS << endl;
			if (!strcmp(recvMS, STOPSYMBOL)) break;
			res_NUM++;
		}
	}
	cout <<"CU!"<<endl << "服务端停止连接" << endl << "本次收到的消息条数:" << res_NUM << endl;
	return;
}

void sendMS(SOCKET client)
{
	int snd_NUM = 0;
	char sentMS[MAXBUFFSIZE];
	sentMS[0] = 0;
	while (1)
	{
		cin >> sentMS;
		send(client, sentMS, strlen(sentMS), 0);
		if (!strcmp(sentMS, STOPSYMBOL)) break;
		snd_NUM++;
	}
	cout << "客户端停止连接" << endl << "本次发出的消息条数:" << snd_NUM << endl;
	return;
}