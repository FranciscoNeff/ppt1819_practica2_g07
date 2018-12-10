#ifndef protocolostpte_practicas_headerfile
#define protocolostpte_practicas_headerfile
#endif

// COMANDOS DE APLICACION
#define SC "USER"  // SOLICITUD DE CONEXION USER usuario 
#define PW "PASS"  // Password del usuario  PASS password
#define RS "RSET"
#define SD  "QUIT"  // Finalizacion de la conexion de aplicacion
#define SD2 "EXIT"  // Finalizacion de la conexion de aplicacion 
#define ECHO "ECHO" // Definicion del comando "ECHO" para el servicio de eco
#define HELO "HELO"
#define MF "MAIL FROM:"
#define RT "RCPT TO:"
#define AS "SUBJECT:"
#define F_DATA ".\r\n"
#define VRFY "VRFY "
#define MAIL "DATA "
// RESPUESTAS A COMANDOS DE APLICACION

#define OK "2"
#define ER  "ER"
#define UNK_COMAND  "502"

//FIN DE RESPUESTA
#define CRLF "\r\n"
#define F_MENS "\r\n.\r\n"

//ESTADOS
#define S_WELC 0
#define S_HELO 1
#define S_MAILFROM 2
#define S_RCPT 3
#define S_DATA 4
#define S_MAIL 5
#define S_EXIT 6
#define S_RESET 8
#define S_QUIT 9

//PUERTO DEL SERVICIO
#define TCP_SERVICE_PORT	25 //Es el puerto fijo que utilizamos para esta practica

// NOMBRE Y PASSWORD AUTORIZADOS
#define USER		"alumno"
#define PASSWORD	"123456"