#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

int sockfd;//客户端socket
char* IP = "127.0.0.1";//服务器的IP
short PORT = 10222;//服务器服务端口
typedef struct sockaddr SA;
char name[30];

void init(){
	sockfd = socket(PF_INET,SOCK_STREAM,0);
	struct sockaddr_in addr;
	addr.sin_family = PF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = inet_addr(IP);
	if (connect(sockfd,(SA*)&addr,sizeof(addr)) == -1){
		perror("无法连接到服务器");
		exit(-1);
	}
	printf("客户端启动成功\n");
}

void start(){
	pthread_t id;
	void* recv_thread(void*);
	pthread_create(&id,0,recv_thread,0);
	char buf2[100] = {};
	send(sockfd,name,sizeof(name),0);
	sprintf(buf2,"%s进入了聊天室",name);
	send(sockfd,buf2,strlen(buf2),0);
	while(1){
		char buf[100] = {};
		//	scanf("%s",buf);
		fgets(buf,sizeof(buf),stdin);
		if(buf[strlen(buf)-1] == '\n')
			buf[strlen(buf)-1] = '\0';
		char msg[131] = {};
		// sprintf(msg,"%s:%s",name,buf);
		//send(sockfd,msg,strlen(msg),0);
		if (strcmp(buf,"/bye") == 0){
			memset(buf2,0,sizeof(buf2));
			sprintf(buf2,"%s退出了聊天室",name);
			send(sockfd,buf2,strlen(buf2),0);
			break;
		}
		else if(strcmp(buf,"/help") == 0){
			printf("/members : 查看線上人數\n");
			printf("/sendto : 與指定對方傳訊息\n");
			printf("/file : 與指定對方傳檔案\n");
			printf("/bye : 離線\n");
		}
		else if(strcmp(buf,"/members") == 0){
			memset(buf2,0,sizeof(buf2));
			printf("Online members\n");
			send(sockfd,"/members",strlen("/members"),0);
		}
		else if(strcmp(buf,"/sendto") == 0){
			memset(buf2,0,sizeof(buf2));
			send(sockfd,"/sendto",strlen("/sendto"),0);
			printf("Sent to: ");
			memset(buf,0,sizeof(buf));
			scanf("%s",buf);
			send(sockfd,buf,strlen(buf),0);
			printf("Message: ");
			memset(buf,0,sizeof(buf));
			fgets(buf,sizeof(buf),stdin);
			if(buf[strlen(buf)-1] == '\n')
				buf[strlen(buf)-1] = '\0';
			send(sockfd,buf,strlen(buf),0);
		}	
		else if(strcmp(buf,"/file") == 0){
			memset(buf2,0,sizeof(buf2));
			memset(buf,0,sizeof(buf));
			send(sockfd,"/file",strlen("/file"),0);
			FILE *ftp;
			printf("Sent to: ");
			memset(buf,0,sizeof(buf));
			scanf("%s",buf);
			send(sockfd,buf,strlen(buf),0);
			printf("File name: ");
			memset(buf,0,sizeof(buf));
			scanf("%s",buf);
			send(sockfd,buf,strlen(buf),0);
			ftp = fopen(buf,"r");
			if(ftp != NULL){ 
				while(fgets(buf,sizeof(buf),ftp)!=NULL){
					printf("%s",buf);
					send(sockfd,buf,strlen(buf),0);
					sleep(1);
				}
				send(sockfd,"end",strlen("end"),0);
				printf("Sending file Finished!\n");
				fclose(ftp);
			}
			else printf("The file doesn't exist\n");
		}
		else if(strcmp(buf,"Y") == 0){
			send(sockfd,"Y",strlen("Y"),0);
		}
		else if(strcmp(buf,"n") == 0){
			printf("The file is cancelled\n");
			send(sockfd,"n",strlen("n"),0);

		}
		else if(strcmp(buf,"\0") == 0){
			continue;
		}
		else{
			sprintf(msg,"%s:%s",name,buf);
			send(sockfd,msg,strlen(msg),0);
		}

	}
	close(sockfd);
}

void* recv_thread(void* p){
	while(1){
		char buf3[100] = {};
		if (recv(sockfd,buf3,sizeof(buf3),0) <= 0){
			exit(1);
		}
		if(strcmp(buf3,"Recieve file[Y/n]") == 0){
			printf("%s\n",buf3);
			fflush(stdout);
			memset(buf3,0,sizeof(buf3));
			while(1){
				if(recv(sockfd,buf3,sizeof(buf3),0)>0)
					break;
			}
			if(strcmp(buf3,"start") == 0){
				FILE *ftp;
				memset(buf3,0,sizeof(buf3));
				while(1){
					if(recv(sockfd,buf3,sizeof(buf3),0)>0)
						break;
				}
				ftp = fopen(buf3,"w");
				while(1){
					if(recv(sockfd,buf3,sizeof(buf3),0)>0){
						if(strcmp(buf3,"end") == 0) break;
						printf("%s",buf3);
						fflush(stdout);
						fprintf(ftp,"%s",buf3);
						memset(buf3,0,sizeof(buf3));
					}
				}
				fclose(ftp);
				printf("Recieve completely\n");
			}
		}
		else	printf("%s\n",buf3);
	}
}

int main(){
	init();
	printf("请输入您的名字：");
	scanf("%s",name);
	start();
	return 0;
}
