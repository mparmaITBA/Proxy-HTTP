#ifndef __PARSER_H__
#define __PARSER_H__

#include <buffer.h>
#include <netinet/in.h>
#include <stdint.h>
typedef enum {
	MAX_HOST_NAME_LENGTH = 0xFF, // 255
	MAX_HEADER_NAME = 255,
	MAX_HEADER_VALUE = 255,
	MAX_HEADERS = 5,
	MAX_BODY_LENGTH = 1023,
	MAX_METHOD_LENGTH = 24,
	MAX_PROTOCOL_LENGTH = 24,
	MAX_IP_LENGTH = 24,
	MAX_RELATIVE_PATH = 64,
} http_request_constraints;

typedef enum {
	PS_METHOD,
	PS_PATH,
	PS_PATH_RELATIVE,
	PS_PATH_PROTOCOL,
	PS_PATH_DOMAIN,
	PS_IP,
	PS_IPv4,
	PS_IPv6,
	PS_URI,
	PS_HTTP_VERSION,
	PS_CR,
	PS_LF,
	PS_CR_END,
	PS_LF_END,
	PS_FIN // cuando se recibe \cr\lf\cr\lf
} http_parser_state;

typedef struct {
	char when;
	char upper_bound; // con limite incluido
	char lower_bound; // con limite incluido
	http_parser_state destination;
	void (*transition)(char);
} http_parser_state_transition;

typedef struct {
	char major; // parte izquierda de la version http1.0 -> 1
	char minor; // parte derecha de la version http1.0 -> 0
} http_version;

typedef enum { ABSOLUTE, RELATIVE, NO_RESOURCE } http_path_type;

typedef enum { IPV4, IPV6, DOMAIN } http_host_type;

typedef union {
	struct sockaddr_in ipv4;
	struct sockaddr_in6 ipv6;
	// uint8_t domain[MAX_FQDN_LENGTH + 1];   // null terminated
} http_host;

typedef union {
	char host_name[MAX_HOST_NAME_LENGTH + 1];
	char ip_addr[MAX_IP_LENGTH + 1];
	char relative_path[MAX_RELATIVE_PATH + 1];
} http_request_target;

typedef struct {
	http_path_type path_type;
	http_request_target request_target;
	http_host_type host_type;
	http_host host;
	in_port_t port;
} http_target;

typedef struct {
	char method[MAX_METHOD_LENGTH + 1];
	http_target target;
	http_version version;
} http_start_line;

typedef struct {
	char header_name[MAX_HEADER_NAME + 1];
	char header_value[MAX_HEADER_VALUE + 1];
} http_header;

typedef struct {
	http_start_line start_line; // start_line(por si no se completo)
	http_header header;			// header actual(por si no se completo)
	buffer *parsed_request;		// listo para enviar
	http_parser_state parser_state;
	size_t copy_index;
	char protocol[MAX_PROTOCOL_LENGTH + 1];
} http_request;

int parse_request(http_request *request, buffer *read_buffer);

#endif
