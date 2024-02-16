#include<io.h>
#include<stdio.h>
#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library
int main(int argc , char *argv[])
{

WSADATA wsa ;
SOCKET s , new_socket;
struct sockaddr_in server , client;
int c, cnt;
char msg [100];
char client_reply[2000];
int recv_size;

printf("\nInitialising Winsock...");
if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
{
    printf("Failed. Error Code : %d",WSAGetLastError());
    return 1;
}

printf("Initialised.\n");


if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
{
    printf("Could not create socket : %d" , WSAGetLastError());
}
printf("Socket created.\n");
server.sin_family = AF_INET;
server.sin_addr.s_addr = inet_addr ("127.0.0.1");
server.sin_port = htons( 9999 );



if( bind(s ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR)
{
    printf("Bind failed with error code : %d" , WSAGetLastError());
}
puts("Bind done");
listen(s , 3);


puts("Waiting for incoming connections...");

c = sizeof(struct sockaddr_in);
new_socket = accept(s , (struct sockaddr *)&client, &c);
if (new_socket == INVALID_SOCKET)
{
    printf("accept failed with error code : %d" , WSAGetLastError());
}

puts("Connection accepted");
int packet=0,ack=0;
while(1){
if((recv_size = recv(new_socket , client_reply , 2000 , 0)) == SOCKET_ERROR)
    puts("recv failed");
else { 
client_reply[recv_size] = '\0';
if (strcmp(client_reply, "hi") == 0) { // Compare strings using strcmp()
            
            
            printf("Retransmitted packet %d\n", packet);
            continue;
        }
 else{       
packet++;
ack++;
puts(client_reply);

printf ("Packet sent: %d",packet);
printf ("ACK recieved: %d",ack);
 }
//gets (msg);
printf("\n");
send(new_socket , msg , strlen(msg) , 0);
}
}
return 0;
}