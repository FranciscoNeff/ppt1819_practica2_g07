#ifndef protocolostpte_practicas_headerfile
#define protocolostpte_practicas_headerfile
#endif

// COMANDOS DE APLICACION
#define SC "USER"  // SOLICITUD DE CONEXION USER usuario 
#define PW "PASS"  // Password del usuario  PASS password
#define RS "RESET"
#define SD  "QUIT"  // Finalizacion de la conexion de aplicacion
#define SD2 "EXIT"  // Finalizacion de la conexion de aplicacion 
#define ECHO "ECHO" // Definicion del comando "ECHO" para el servicio de eco
//#define SUM "SUM"   // Definicion del comando "SUM" para sumar
#define HELO "helo"
#define MF "MAIL FROM:"
#define RT "RCPT TO:"
#define F_DATA ".\r\n"
// RESPUESTAS A COMANDOS DE APLICACION
#define OK  "OK"
#define ER  "ER"

//FIN DE RESPUESTA
#define CRLF "\r\n"

//ESTADOS
#define S_WELC 0
#define S_HELO 1
#define S_MAILFROM 2
#define S_RCPT 3
#define S_DATA 4
#define S_QUIT 5
#define S_EXIT 6

//PUERTO DEL SERVICIO
#define TCP_SERVICE_PORT	25 //Es el puerto fijo que utilizamos para esta practica

// NOMBRE Y PASSWORD AUTORIZADOS
#define USER		"alumno"
#define PASSWORD	"123456"