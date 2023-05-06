#include<stdio.h>
#include<iostream>
#include<cstring>
#include<sys/fcntl.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<errno.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<string>
#include<future>
#include<chrono>
using namespace std;

#define STOPSYMBOL "##stop##"
#define MAXBUFFSIZE 2048
#define serverport 8920

void recvMS(int);
void sendMS(int);

int main(void)
{
    //创建socket
    int server;
    if((server=socket(AF_INET,SOCK_STREAM,0))==-1)   //failure->-1 success->0
    {
        cout<<"server socket创建失败"<<endl;
        return -1;
    }

    //网络地址与bind()
    struct sockaddr_in local_addr;
    memset(&local_addr,0,sizeof(local_addr));
    local_addr.sin_family=AF_INET;
    local_addr.sin_addr.s_addr=htonl(INADDR_ANY);  //0.0.0.0本机
    local_addr.sin_port=htons(serverport);
    if(bind(server,(struct sockaddr *)&local_addr,sizeof(local_addr))!=0)
    {
        cout<<"server网络地址绑定失败"<<endl;
        return -1;
    }

    //listen()
    if(listen(server,5)!=0)
    {
        cout<<"server设置监听模式失败"<<endl;
        return -1;
    }

    //创建新的连接套接字和accept
    int client;
    int sockaddr_len=sizeof(struct sockaddr_in);
    struct sockaddr_in client_addr;
    cout<<"等待客户端连接..."<<endl;
    client=accept(server,(struct sockaddr*)&client_addr,(socklen_t *)&sockaddr_len);
    if(client==-1)
    {
        cout<<"客户端连接错误"<<endl;
        return -1;
    }
    cout<<"已连接到："<<inet_ntoa(client_addr.sin_addr)<<endl;
    
    //异步通信
    future<void> f_recv = async(launch::async, [client](){recvMS(client);});
    future<void> f_send = async(launch::async, [client](){sendMS(client);});
	
	// f_recv.wait();
	// f_send.wait();

    //任意线程结束(寄了)
    while (f_recv.wait_for(chrono::milliseconds(1)) != future_status::ready
           && f_send.wait_for(chrono::milliseconds(1)) != future_status::ready) {
        // Do nothing, just keep waiting
    }
    
    close(server); 
    close(client);
}

void recvMS(int client)
{
    char recvbuff[MAXBUFFSIZE];
    recvbuff[0]=0;
    int recvlen=0;
    int recvnum=0;
    while(1)
    {
        recvlen=recv(client,recvbuff,MAXBUFFSIZE,0);
        if(recvlen>0)
        {
            recvbuff[recvlen]=0;
            cout<<"Client:"<<recvbuff<<endl;
            if(!strcmp(recvbuff,STOPSYMBOL))
            {
                cout<<"客户端已停止连接"<<endl<<"本次收到的消息条数"<<recvnum<<endl;
                return;
            }
            recvnum++;
        }
    }
}

void sendMS(int client)
{
    char sendbuff[MAXBUFFSIZE];
    sendbuff[0]=0;
    int sendlen=0;
    int sendnum=0;
    while(1)
    {
        //cout<<"You:";
        cin>>sendbuff;
        send(client,sendbuff,strlen(sendbuff),0);
        if(!strcmp(sendbuff,STOPSYMBOL))
        {
           cout<<"服务已停止连接"<<endl<<"本次收到的消息条数"<<sendnum<<endl;
           return;
        }
        sendnum++;
    }
}