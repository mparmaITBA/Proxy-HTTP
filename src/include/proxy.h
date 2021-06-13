#ifndef _PROXY_H_
#define _PROXY_H_

#include <connection.h>

typedef enum { CLIENT, SERVER } peer;

void write_proxy_statistics();

void send_message(char *message, int fd_client, connection_node *node);

typedef enum {
	// estos codigos usan valores negativos para distinguirlos de los que si devuelven las funciones involucradas
	BAD_REQUEST_ERROR = -20,
	RECV_ERROR_CODE, // fallo el receive por algo no relacionado al que socket sea no bloqueante
	SEND_ERROR_CODE, // fallo el send por algo no relacionado al que socket sea no bloqueante
	DOH_SEND_ERROR_CODE,
	DOH_TRY_ANOTHER_REQUEST,
	ACCEPT_CONNECTION_ERROR,
	SETUP_CONNECTION_ERROR_CODE,
	CLOSE_CONNECTION_ERROR_CODE,
	BROKEN_PIPE_ERROR_CODE,
	CLIENT_CLOSE_READ_ERROR_CODE,
	SERVER_CLOSE_READ_ERROR_CODE
} connection_error_code;

#endif
