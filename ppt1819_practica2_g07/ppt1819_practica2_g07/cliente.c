/*******************************************************
Protocolos de Transporte
Grado en Ingenier�a Telem�tica
Dpto. Ingen�er�a de Telecomunicaci�n
Univerisdad de Ja�n

Fichero: cliente.c
Versi�n: 2.0
Fecha: 09/2018
Descripci�n:
	Cliente sencillo TCP para IPv4 e IPv6

Autor: Juan Carlos Cuevas Mart�nez

*******************************************************/
#include <stdio.h>
#include <ws2tcpip.h>//Necesaria para las funciones IPv6
#include <conio.h>
#include "protocol.h"

#pragma comment(lib, "Ws2_32.lib")

int main(int *argc, char *argv[])
{
	SOCKET sockfd;
	struct sockaddr *server_in=NULL;
	struct sockaddr_in server_in4;
	struct sockaddr_in6 server_in6;
	int address_size = sizeof(server_in4);
	char buffer_in[1024], buffer_out[1024],input[1024];
	int recibidos=0,enviados=0;
	int estado=S_HELO;
	char option;
	int ipversion=AF_INET;//IPv4 por defecto
	char ipdest[256];
	char default_ip4[16]="127.0.0.1"; //IP4 Direccion Loopback 
	char default_ip6[64]="::1"; //IP6 Direccion Loopback 

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
   
	//Inicializaci�n Windows sockets - SOLO WINDOWS
	wVersionRequested=MAKEWORD(1,1);
	err=WSAStartup(wVersionRequested,&wsaData);
	if(err!=0)
		return(0);

	if(LOBYTE(wsaData.wVersion)!=1||HIBYTE(wsaData.wVersion)!=1){
		WSACleanup();
		return(0);
	}
	//Fin: Inicializaci�n Windows sockets
	
	printf("**************\r\nCLIENTE TCP SENCILLO SOBRE IPv4 o IPv6\r\n*************\r\n");
	

	do{

		printf("CLIENTE> �Qu� versi�n de IP desea usar? 6 para IPv6, 4 para IPv4 [por defecto] ");
		gets_s(ipdest, sizeof(ipdest));

		if (strcmp(ipdest, "6") == 0) {
			ipversion = AF_INET6;  //IP6 Elige las direcciones ip a usar

		}
		else { //Distinto de 6 se elige la versi�n 4
			ipversion = AF_INET; //IP4 Elige las direcciones ip a usar
		}

		sockfd=socket(ipversion,SOCK_STREAM,0); //SOCKET (socket)Crea el descriptor del socket
		if(sockfd==INVALID_SOCKET){
			printf("CLIENTE> ERROR\r\n");
			exit(-1);
		}
		else{
			printf("CLIENTE> Introduzca la IP destino (pulsar enter para IP por defecto): ");
			gets_s(ipdest,sizeof(ipdest));

			//Direcci�n por defecto seg�n la familia
			if(strcmp(ipdest,"")==0 && ipversion==AF_INET) //IP4 Se coge la direccion por defecto de ip4
				strcpy_s(ipdest,sizeof(ipdest),default_ip4);

			if(strcmp(ipdest,"")==0 && ipversion==AF_INET6) //IP6 Se coge la direccion por defecto de ip6
				strcpy_s(ipdest, sizeof(ipdest),default_ip6);
			 
			if(ipversion==AF_INET){  //IP4
				server_in4.sin_family=AF_INET;
				server_in4.sin_port=htons(TCP_SERVICE_PORT);
				//server_in4.sin_addr.s_addr=inet_addr(ipdest);
				inet_pton(ipversion,ipdest,&server_in4.sin_addr.s_addr);
				server_in=(struct sockaddr*)&server_in4;
				address_size = sizeof(server_in4);
			}

			if(ipversion==AF_INET6){     //IP6
				memset(&server_in6, 0, sizeof(server_in6));
				server_in6.sin6_family=AF_INET6;
				server_in6.sin6_port=htons(TCP_SERVICE_PORT);
				inet_pton(ipversion,ipdest,&server_in6.sin6_addr);
				server_in=(struct sockaddr*)&server_in6;
				address_size = sizeof(server_in6);
			}

			estado=S_HELO;

			if(connect(sockfd, server_in, address_size)==0){  //SOSCKET (connect)Establece/Inicia la conexion
				printf("CLIENTE> CONEXION ESTABLECIDA CON %s:%d\r\n",ipdest,TCP_SERVICE_PORT);
			
				//Inicio de la m�quina de estados
				do{
					switch(estado){
					case S_HELO:
						// Se recibe el mensaje de bienvenida
						break;
					case S_USER:
						// establece la conexion de aplicacion 
						printf("CLIENTE> Introduzca el usuario (enter para salir): ");
						gets_s(input,sizeof(input));
						if(strlen(input)==0){ //Si la cadena esta vacia  se finaliza la conexion
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);
							estado=S_QUIT;
						}
						else
							//se verifica el usuario
						sprintf_s (buffer_out, sizeof(buffer_out), "%s %s%s",SC,input,CRLF);
						break;
					case S_PASS:
						printf("CLIENTE> Introduzca la clave (enter para salir): ");
						gets_s(input, sizeof(input));
						if(strlen(input)==0){ //Si la cadena esta vacia  se finaliza la conexion
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);
							estado=S_QUIT;
						}
						else
							//se verifica el pass
							sprintf_s (buffer_out, sizeof(buffer_out), "%s %s%s",PW,input,CRLF);
						break;
					case S_DATA:
						/*printf("CLIENTE> Introduzca datos (enter o QUIT para salir): ");
						gets_s(input, sizeof(input));
						if(strlen(input)==0){ //Si la cadena esta vacia  se finaliza la conexion
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);
							estado=S_QUIT;
						}
						else
							
							sprintf_s (buffer_out, sizeof(buffer_out), "%s %s%s",ECHO,input,CRLF);*/
						//Cliente SUMA
						printf("CLIENTE> Introduzca SUM NUM1 NUM2 para realizar la suma (enter o QUIT para salir): ");
						fflush(stdin); fflush(stdout);//Limpieza de buffer
						gets_s(input, sizeof(input));
						if (strlen(input) == 0) { //Si la cadena esta vacia  se finaliza la conexion
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
							estado = S_QUIT;
						}
						else

							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", SUM, input, CRLF);
						//No funciona siempre da comando erroneo
							break;
				
					}
					//Si todo es correcto envio los datos
					if(estado!=S_HELO){
						enviados=send(sockfd,buffer_out,(int)strlen(buffer_out),0); //SOCKET (send)Envia el mensaje
						//Se comprueba si hay error
						if(enviados==SOCKET_ERROR){
							 estado=S_QUIT;
							 continue;
						}
					}
						
					recibidos=recv(sockfd,buffer_in,512,0); //SOCKET (recv)Recibe el mensaje
					if(recibidos<=0){
						DWORD error=GetLastError();
						if(recibidos<0){
							//Error al recibir se finaliza la conexion
							printf("CLIENTE> Error %d en la recepci�n de datos\r\n",error);
							estado=S_QUIT;
						}
						else{
							//Conexion cerrada de forma manual
							printf("CLIENTE> Conexi�n con el servidor cerrada\r\n");
							estado=S_QUIT;
						}
					}else{
						//Se muestra el mensaje
						buffer_in[recibidos]=0x00;
						printf(buffer_in);
						if(estado!=S_DATA && strncmp(buffer_in,OK,2)==0) 
							estado++;  
					}

				}while(estado!=S_QUIT);		
			}
			else{
				int error_code=GetLastError();
				printf("CLIENTE> ERROR AL CONECTAR CON %s:%d\r\n",ipdest,TCP_SERVICE_PORT);
			}		
			// fin de la conexion de transporte
			closesocket(sockfd); //SOCKET (closesocket) cierra el socket
			
		}	
		printf("-----------------------\r\n\r\nCLIENTE> Volver a conectar (S/N)\r\n");
		option=_getche();

	}while(option!='n' && option!='N');

	return(0);
}
