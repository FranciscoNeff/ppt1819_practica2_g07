/*******************************************************
Protocolos de Transporte
Grado en Ingeniería Telemática
Dpto. Ingeníería de Telecomunicación
Univerisdad de Jaén

Fichero: cliente.c
Versión: 2.0
Fecha: 09/2018
Descripción:
	Cliente sencillo TCP para IPv4 e IPv6

Autor: Juan Carlos Cuevas Martínez

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
	struct tm *tm;//ayuda fecha //https://poesiabinaria.net/2012/06/obtener-la-fecha-y-hora-formateada-en-c/
	char fecha[12];
	int address_size = sizeof(server_in4);
	char buffer_in[1024], buffer_out[1024],input[1024];
	int recibidos=0,enviados=0;
	int estado;
	char option;
	int ipversion=AF_INET;//IPv4 por defecto
	char ipdest[256];
	char default_ip4[16]="127.0.0.1"; //IP4 Direccion Loopback 
	char default_ip6[64]="::1"; //IP6 Direccion Loopback 
	char comando[4], line[2048], data[2048] = { NULL };
	char mail[2000] = {NULL};
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	//manenjo de cabeceras
	char date[10], subject[100], to[45], from[45];
	

	//Inicialización Windows sockets - SOLO WINDOWS
	wVersionRequested=MAKEWORD(1,1);
	err=WSAStartup(wVersionRequested,&wsaData);
	if(err!=0)
		return(0);

	if(LOBYTE(wsaData.wVersion)!=1||HIBYTE(wsaData.wVersion)!=1){
		WSACleanup();
		return(0);
	}
	//Fin: Inicialización Windows sockets
	
	printf("**************\r\nCLIENTE SERVIDOR DE CORREO \r\n*************\r\n");
	

	do {

		printf("CLIENTE> ¿Qué versión de IP desea usar? 6 para IPv6, 4 para IPv4 [por defecto] ");
		gets_s(ipdest, sizeof(ipdest));

		if (strcmp(ipdest, "6") == 0) {
			ipversion = AF_INET6;  //IP6 Elige las direcciones ip a usar

		}
		else { //Distinto de 6 se elige la versión 4
			ipversion = AF_INET; //IP4 Elige las direcciones ip a usar
		}

		sockfd = socket(ipversion, SOCK_STREAM, 0); //SOCKET (socket)Crea el descriptor del socket
		if (sockfd == INVALID_SOCKET) {
			printf("CLIENTE> ERROR\r\n");
			exit(-1);
		}
		else {
			printf("CLIENTE> Introduzca la IP del dominio (pulsar enter para IP por defecto): ");
			gets_s(ipdest, sizeof(ipdest));

			//Dirección por defecto según la familia
			if (strcmp(ipdest, "") == 0 && ipversion == AF_INET) //IP4 Se coge la direccion por defecto de ip4
				strcpy_s(ipdest, sizeof(ipdest), default_ip4);

			if (strcmp(ipdest, "") == 0 && ipversion == AF_INET6) //IP6 Se coge la direccion por defecto de ip6
				strcpy_s(ipdest, sizeof(ipdest), default_ip6);

			if (ipversion == AF_INET) {  //IP4
				server_in4.sin_family = AF_INET;
				server_in4.sin_port = htons(TCP_SERVICE_PORT);
				//server_in4.sin_addr.s_addr=inet_addr(ipdest);
				inet_pton(ipversion, ipdest, &server_in4.sin_addr.s_addr);
				server_in = (struct sockaddr*)&server_in4;
				address_size = sizeof(server_in4);
			}

			if (ipversion == AF_INET6) {     //IP6
				memset(&server_in6, 0, sizeof(server_in6));
				server_in6.sin6_family = AF_INET6;
				server_in6.sin6_port = htons(TCP_SERVICE_PORT);
				inet_pton(ipversion, ipdest, &server_in6.sin6_addr);
				server_in = (struct sockaddr*)&server_in6;
				address_size = sizeof(server_in6);
			}

			estado = S_WELC;

			if (connect(sockfd, server_in, address_size) == 0) {  //SOSCKET (connect)Establece/Inicia la conexion
				printf("CLIENTE> CONEXION ESTABLECIDA CON %s:%d\r\n", ipdest, TCP_SERVICE_PORT);

				//Inicio de la máquina de estados
				do {
					switch (estado) {
					case S_WELC:
						//RECIBIR mensaje welcome de argsoft


						break;
					case S_HELO://como al final esto es una conexion fija lo cambiamos a predefinida
						printf("CLIENTE> Introduzca el comando HELO para continuar (enter para salir):  ");
						gets_s(input, sizeof(input));
						if (strlen(input) == 0) {
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", input, CRLF);
							estado = S_QUIT;
						}

						else {
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", input, CRLF);
						}

						break;
					case S_MAILFROM:
						// establece la conexion para escribir el remitente del mensaje
						//problema no verifica el user ahi que añadir VRFY
						printf("CLIENTE> Introduzca el remitente (enter para salir): ");
						gets_s(input, sizeof(input));
						if (strlen(input) == 0) { //Si la cadena esta vacia  se finaliza la conexion
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
							estado = S_QUIT;

						}
						else {
							strcat_s(data, sizeof(data), "from ");
							strcat_s(data, sizeof(data), input, sizeof(input));
							strcat_s(data, sizeof(data), CRLF);//cabecera from en el data
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", MF, input, CRLF);//no reconoce VRFY preguntar
						}

						break;
					case S_RCPT: //hacer un bucle para mandar mas de un destinatario

						printf("CLIENTE> Introduzca el destinatario (enter para salir): ");

						gets_s(input, sizeof(input));
						if (strlen(input) == 0) { //Si la cadena esta vacia  se finaliza la conexion
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
							estado = S_QUIT;
						}
						else {
							//se marca el destinatario
							strcat_s(data, sizeof(data), "to ");
							strcat_s(data, sizeof(data), input, sizeof(input));
							strcat_s(data, sizeof(data), CRLF);//cabecera from en el data
							//va uno a uno
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", RT, input, CRLF);
						}

						break;
					case S_DATA:

						//zona subject
						printf("CLIENTE> Introduzca el asunto (RESET para salir): ");
						gets_s(input, sizeof(input));
						if (strcmp(input, "RESET") == 0) {
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", RS, CRLF);
							estado = S_QUIT;
						}
						else {
							strcat_s(data, sizeof(data), "subject ");
							strcat_s(data, sizeof(data), input, sizeof(input));
							strcat_s(data, sizeof(data), CRLF);//cabecera from en el data
							//sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", data, CRLF);
						}
						//TODO falta la fecha de modo manual
						break;
					case S_MAIL:
						printf("CLIENTE> Escriba el correo (utilice '.' y enter para enviar)( RESET para salir): ");
						gets_s(line, sizeof(line));
						if (strcmp(line, "RESET") == 0) {
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", RS, CRLF);
							estado = S_QUIT;
						}
						else {
							strcat_s(data,sizeof(data), line);
							//strcat_s(data, sizeof(data), CRLF);
							do {
								gets_s(line, sizeof(line));
								strcat_s(data,sizeof(data), line,sizeof(line));
								//strcat_s(data, sizeof(data), "CRLF");
							} while (strcmp(line, ".") != 0 && strlen(data) < 999);//mil caracecteres maximo
							strcat_s(data, sizeof(data), F_MENS);
							
						}
						if(strcmp(line, "RESET") != 0){
							printf("CLIENTE> ¿Si los datos introducidos son correctos pulse cualquier tecla para continuar? ( RESET para salir): ");
						gets_s(line, sizeof(line));
						if (strcmp(line, "RESET") == 0) {
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", RS, CRLF);
							estado = S_QUIT;//comprobar
						}
						else {
							//zona date
					/*	time_t	t = time(NULL);
						tm = localtime(&t);
						strftime(fecha, 10, "%d/%m/%Y", tm);//fomato DD-MM-YYYY
						strcat_s(mail, 5, "date ");
						strcat_s(mail, sizeof(mail), fecha,sizeof(fecha));
						strcat_s(mail, sizeof(mail), CRLF);
						strcat_s(mail, sizeof(mail), data, sizeof(data));*/
							strcat(buffer_out, data);
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s%s", MAIL, CRLF, data );
							//enviados = send(sockfd, buffer_out, (int)strlen(buffer_out), 0); //SOCKET (send)Envia el mensaje
						}
					}
						break;
					}



					//Si todo es correcto envio los datos
					if (estado != S_WELC && estado != S_DATA) {
						enviados = send(sockfd, buffer_out, (int)strlen(buffer_out), 0); //SOCKET (send)Envia el mensaje
						//Se comprueba si hay error
						if (enviados == SOCKET_ERROR) {
							estado = S_QUIT;
							continue;
						}
					}
					if (estado != S_MAIL && estado != S_DATA) {
						recibidos = recv(sockfd, buffer_in, 512, 0); //SOCKET (recv)Recibe el mensaje
						if (recibidos <= 0) {
							DWORD error = GetLastError();
							if (recibidos < 0) {
								//Error al recibir se finaliza la conexion
								printf("CLIENTE> Error %d en la recepción de datos\r\n", error);
								estado = S_QUIT;
							}
							else {
								//Conexion cerrada de forma manual
								printf("CLIENTE> Conexión con el servidor cerrada\r\n");
								estado = S_QUIT;
							}
						}
						else {
							//Se muestra el mensaje
							buffer_in[recibidos] = 0x00;
						}
					}

					//Maquina de estados

					switch (estado) {
					case S_WELC:
						if (strncmp(buffer_in, OK, 1) == 0) {
							printf(buffer_in);
							estado = S_HELO;
						}
						else {
							printf("SERVIDOR> Error en la rececpcion de datos \r\n");
							estado = S_QUIT;
						}
						break;
					case S_HELO:
						if (strncmp(buffer_in, OK, 1) == 0) {
							printf("SERVIDOR> Bienvenido al servidor de correo\n");
							estado = S_MAILFROM;

						}
						else if (strncmp(buffer_in, UNK_COMAND, 3) == 0) {
							estado = S_HELO;
							printf("SERVIDOR> Comando incorrecto \r\n");
						}
						else {
							printf("SERVIDOR> Error en la rececpcion de datos \r\n");
							estado = S_QUIT;
						}
						break;
					case S_MAILFROM:
						//buffer_in[recibidos] = 0x00;
						//printf("%s", &buffer_in);
						if (strncmp(buffer_in, OK, 1) == 0) {
							printf("MAIL FROM: %s\n",&input,sizeof(input));
							estado = S_RCPT;
						}
						
						else {
							printf("SERVIDOR> Error en la rececpcion de datos \r\n");
							estado = S_QUIT;
						}
						break;
					case S_RCPT:
						if (strncmp(buffer_in, OK, 1) == 0) {
							printf("RCPT TO: %s\n", &input, sizeof(input));
							printf("Desea introducir otro destinatario mas, cualquier tecla para continuar?(Enter para salir)\n");
								gets_s(input, sizeof(input));
								if (strlen(input) == 0) { estado = S_DATA; }
								else {
									estado = S_RCPT;
								}
						}
						else if (strncmp(buffer_in, USR_UNK, 3) == 0) {
							estado = S_RCPT;
							printf("SERVIDOR> Usuario incorrecto vuelva a intentarlo \r\n");
						}
						else {
							printf("SERVIDOR> Error en la rececpcion de datos \r\n");
							estado = S_QUIT;
						}
						break;
					case S_DATA:
						estado = S_MAIL;
						break;
					case S_MAIL:
						if (strncmp(buffer_in, OK, 1) == 0) {
							printf("SERVIDOR> Correo enviado correctamente\n");
							printf("CLIENTE> Desea redactar otro correo, cualquier tecla para continuar? (enter para salir)");
							gets_s(input, sizeof(input));
							if (strlen(input) == 0) { estado = S_QUIT; }
							else { estado = S_MAILFROM;
							/*buffer_in[recibidos] = 0x00;
							printf("%s", &buffer_in);
							buffer_in[recibidos] = 0x00;
							buffer_in[recibidos] = 0x00;
							printf("%s", &buffer_in);*/
							//sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", HELO, CRLF);
							}

						}
						else if (strncmp(buffer_in, UNK_COMAND, 3) == 0) {
							estado = S_HELO;
							printf("SERVIDOR> Comando incorrecto \r\n");
						}
						else {
							printf("SERVIDOR> Error en la rececpcion de datos \r\n");
							estado = S_QUIT;
						}
						break;







					}
				
					}while (estado != S_QUIT);
				}

			
			else {
				int error_code = GetLastError();
				printf("CLIENTE> ERROR AL CONECTAR CON %s:%d\r\n", ipdest, TCP_SERVICE_PORT);
			}
			// fin de la conexion de transporte
			closesocket(sockfd); //SOCKET (closesocket) cierra el socket

				}
				printf("-----------------------\r\n\r\nCLIENTE> Volver a conectar (S/N)\r\n");
				option = _getche();

			}while (option != 'n' && option != 'N');

			return(0);
		}
	
