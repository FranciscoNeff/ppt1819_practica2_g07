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
#include <time.h>
#include "protocol.h"

#pragma comment(lib, "Ws2_32.lib")

int main(int *argc, char *argv[])
{
	SOCKET sockfd;
	struct sockaddr *server_in = NULL;
	struct sockaddr_in server_in4;
	struct sockaddr_in6 server_in6;
	struct in_addr address;
	//ayuda fecha //https://poesiabinaria.net/2012/06/obtener-la-fecha-y-hora-formateada-en-c/
	//http://www.holamundo.es/lenguaje/c/articulos/fecha-hora-c.html
	char fecha[64];
	int address_size = sizeof(server_in4);
	char buffer_in[1024], buffer_out[1024], input[1024];
	int recibidos = 0, enviados = 0;
	int estado;
	char option;
	int ipversion = AF_INET;//IPv4 por defecto
	char ipdest[256];
	char ipdestl[256];
	char default_ip4[16] = "127.0.0.1"; //IP4 Direccion Loopback 
	char default_ip6[64] = "::1"; //IP6 Direccion Loopback 
	char comando[4], line[2048], data[2048] = { NULL };
	char mail[2000] = { NULL };
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	//manenjo de cabeceras



	//Inicializaci�n Windows sockets - SOLO WINDOWS
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
		return(0);

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return(0);
	}
	//Fin: Inicializaci�n Windows sockets

	printf("**************\r\nCLIENTE SERVIDOR DE CORREO \r\n*************\r\n");


	do {

		printf("CLIENTE> �Qu� versi�n de IP desea usar? 6 para IPv6, 4 para IPv4 [por defecto] ");
		gets_s(ipdest, sizeof(ipdest));

		if (strcmp(ipdest, "6") == 0) {
			ipversion = AF_INET6;  //IP6 Elige las direcciones ip a usar

		}
		else { //Distinto de 6 se elige la versi�n 4
			ipversion = AF_INET; //IP4 Elige las direcciones ip a usar
		}

		sockfd = socket(ipversion, SOCK_STREAM, 0); //SOCKET (socket)Crea el descriptor del socket
		if (sockfd == INVALID_SOCKET) {
			printf("CLIENTE> ERROR\r\n");
			exit(-1);
		}
		else {
			printf("CLIENTE> Introduzca la IP o el dominio (pulsar enter para IP por defecto): ");
			gets_s(ipdest, sizeof(ipdest));
			strcpy_s(ipdestl, sizeof(ipdestl), ipdest);
			//para el dominio aqui
			struct hostent *host;
			host = gethostbyname(ipdest);
			if (host != NULL) {//Resolucion del dominio si es valida se lanza el cliente si no abortamos
				memcpy(&address, host->h_addr_list[0], 4);
				strcpy_s(ipdest, sizeof(ipdest), inet_ntoa(address));
				estado = S_WELC;
			}
			else { printf("Dominio mal introducido"); estado = S_QUIT; }


			//Direcci�n por defecto seg�n la familia
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
			

			if (connect(sockfd, server_in, address_size) == 0) {  //SOSCKET (connect)Establece/Inicia la conexion
				printf("CLIENTE> CONEXION ESTABLECIDA CON %s:%d\r\n", ipdest, TCP_SERVICE_PORT);

				//Inicio de la m�quina de estados
				do {

					switch (estado) {
					case S_WELC:
						//RECIBIR mensaje welcome de argsoft
						break;
					case S_HELO:
						sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", HELO, host, CRLF);
						printf("CLIENTE> Introduzca el comando Host para continuar (enter para salir):  ");
						gets_s(input, sizeof(input));
						if (strlen(input) == 0) {
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
							estado = S_QUIT;
						}

						else {//mandamos HELO y el host
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s",HELO, input, CRLF);
						}

						break;
					case S_MAILFROM:
						printf("CLIENTE> Introduzca el remitente (enter para salir): ");
						gets_s(input, sizeof(input));
						if (strlen(input) == 0) { //Si la cadena esta vacia  se finaliza la conexion
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
							estado = S_QUIT;

						}
						else {
							fflush(data);
							strcat_s(data, sizeof(data), "From: ");
							strcat_s(data, sizeof(data), input, sizeof(input));
							strcat_s(data, sizeof(data), CRLF);//cabecera From:  en el data
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s%s", MF, input, CRLF);
						}
						break;
					case S_RCPT:

						printf("CLIENTE> Introduzca el destinatario (enter para salir): ");

						gets_s(input, sizeof(input));
						if (strlen(input) == 0) { //Si la cadena esta vacia  se finaliza la conexion
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
							estado = S_QUIT;
						}
						else {
							//se marca el destinatario
							strcat_s(data, sizeof(data), "To: ");
							strcat_s(data, sizeof(data), input, sizeof(input));
							strcat_s(data, sizeof(data), CRLF);//cabecera To: en el data
							//como pueden ser mas de un destinatario los va incluyendo uno a nuno
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", RT, input, CRLF);
						}

						break;
					case S_DATA:
						printf("CLIENTE> Introduzca el RESET para salir(cualquier tecla para continuar): ");
						gets_s(input, sizeof(input));
						if (strcmp(input, "RESET") == 0) {
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", RS, CRLF);
							estado = S_RSET;
						}
						else {
							//zona subject
							printf("CLIENTE> Introduzca asunto: ");
							gets_s(input, sizeof(input));
							strcat_s(data, sizeof(data), "Subject: ");
							strcat_s(data, sizeof(data), input, sizeof(input));
							strcat_s(data, sizeof(data), CRLF);//cabecera Subject: en el data	
							//zone date
							time_t t = time(NULL);
							struct tm *date = localtime(&t);
							strftime(fecha, sizeof(fecha), "%a, %d %b %Y %H:%M:%S %z", date);
							//EJEMPLO DE FORMATO PARA EL DATE
							//Date: Fri, 21 Nov 1997 09:55:06 -0600
							strcat_s(data, sizeof(data), "Date: ");
							strcat_s(data, sizeof(data), fecha, sizeof(fecha));
							strcat_s(data, sizeof(data), CRLF);//cabecera Date: en el data	
							strcat_s(data, sizeof(data), CRLF);//Doble salto de linea para las cabeceras
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s%s", DATA, CRLF, data);
							printf("CLIENTE> Escriba el correo (utilice '.' y enter para enviar): ");
						}

						//TODO falta la fecha de modo manual
						break;
					case S_MAIL:
						gets_s(line, sizeof(line));
						sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", line,"\n");
						break;

					}//HAY mi madre el bicho del corchete

						//Si todo es correcto envio los datos
						if (estado != S_WELC ) {
							enviados = send(sockfd, buffer_out, (int)strlen(buffer_out), 0); //SOCKET (send)Envia el mensaje
							//Se comprueba si hay error
							if (enviados == SOCKET_ERROR) {
								estado = S_QUIT;
								continue;
							}
						}

						if (estado != S_QUIT && estado!=S_MAIL) {
							recibidos = recv(sockfd, buffer_in, 512, 0); //SOCKET (recv)Recibe el mensaje
							if (recibidos <= 0) {
								DWORD error = GetLastError();
								if (recibidos < 0) {
									//Error al recibir se finaliza la conexion
									printf("CLIENTE> Error %d en la recepci�n de datos\r\n", error);
									estado = S_QUIT;
								}
								else {
									//Conexion cerrada de forma manual
									printf("CLIENTE> Conexi�n con el servidor cerrada\r\n");
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
								printf("SERVIDOR> ");
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
							if (strncmp(buffer_in, OK, 1) == 0 || strncmp(buffer_in, "3", 1) == 0) {
								printf("MAIL FROM: %s\n", &input, sizeof(input));
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
							//el reset te manda a mailfrom
							break;
						case S_DATA:
							if (strncmp(buffer_in, "3", 1) == 0) {
								estado = S_MAIL;
							}
							else { printf("SERVIDOR> Ha ocurrido un error"); estado = S_QUIT; }
							break;
						case S_MAIL:
							if (strcmp(line, ".") != 0) {
								estado = S_MAIL;
							}
							else {
								estado = S_RESPONSE;
								sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", mail, F_MENS);
							}
							break;
						case S_RESPONSE:
								if (strncmp(buffer_in, OK, 1) == 0) {
									printf("SERVIDOR> Correo enviado correctamente\n");
									printf("CLIENTE> Desea redactar otro correo, cualquier tecla para continuar? (enter para salir)");
									gets_s(input, sizeof(input));
									if (strlen(input) == 0) { estado = S_QUIT; }
									else {
										estado = S_MAILFROM;
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
						case S_RSET:
							if (strncmp(buffer_in, OK, 1) == 0) {
								estado = S_MAILFROM;
							}
							else {
								printf("SERVIDOR> Error en la rececpcion de datos \r\n");
								estado = S_QUIT;
							}
							break;


						}	
					} while (estado != S_QUIT);
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

