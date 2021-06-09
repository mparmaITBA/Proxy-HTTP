#ifndef DOHDATA_H
#define DOHDATA_H

#include <buffer.h>
#include <netinet/in.h>
#include <stdint.h>

// Manejo de codigos de return para saber el estado despues de correr
// una funcion de parseo de DoH. Son valores negativos ya que algunas de las funciones
// que lo utilizan retornan valores no negativos para expresar cantidades
typedef enum { DOH_PARSE_INCOMPLETE = -3, DOH_PARSE_COMPLETE, DOH_PARSE_ERROR } doh_parser_status_code;

// Similar a doh_parser_status_code pero para funciones que envian al servidor DoH
typedef enum { DOH_WRITE_NOT_SET = -4, DOH_SEND_INCOMPLETE, DOH_SEND_COMPLETE, DOH_SEND_ERROR } doh_send_status_code;

// Manejo de estados de parseo de response de DoH para saber desde donde
// retomar el parseo en caso de haber respuestas parciales
typedef enum {
	DOH_INIT,
	PREPARING_DOH_PACKET,
	SENDING_DOH_PACKET,
	FINDING_HTTP_STATUS_CODE,
	FINDING_CONTENT_LENGTH,
	PARSING_CONTENT_LENGTH,
	FINDING_HTTP_BODY,
	PARSING_DNS_MESSAGE,
	DNS_READY
} doh_state;

typedef struct {
	int sock;						   // socket activo con servidor DoH
	doh_state state;				   // estado del parseo del response DoH
	buffer *doh_buffer;
	long response_content_length;
} doh_data;

#endif
