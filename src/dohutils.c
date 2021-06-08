#include <dohdata.h>
#include <dohutils.h>
#include <errno.h>
#include <logger.h>
#include <stdlib.h>
#include <string.h>

int setup_doh_resources(connection_node *node, int doh_fd) {
	node->data.doh = malloc(sizeof(doh_data));
	if (node->data.doh == NULL) goto EXIT;

	node->data.doh->doh_buffer = malloc(sizeof(buffer));
	if (node->data.doh->doh_buffer == NULL) goto FREE_DOH_DATA;

	node->data.doh->doh_buffer->data = malloc(MAX_DOH_PACKET_SIZE * SIZE_8);
	if (node->data.doh->doh_buffer->data == NULL) goto FREE_BUFFER;

	buffer_init(node->data.doh->doh_buffer, MAX_DOH_PACKET_SIZE, node->data.doh->doh_buffer->data);

	node->data.doh->sock = doh_fd;
	node->data.doh->state = DOH_INIT;
	node->data.doh->addr_info_first = node->data.doh->addr_info_current = NULL;

	return 0;

FREE_BUFFER:
	free(node->data.doh->doh_buffer);
FREE_DOH_DATA:
	free(node->data.doh);
EXIT:
	logger(ERROR, "malloc(): %s", strerror(errno));
	return -1;
}

int add_ip_address(connection_node *node, int addr_family, void *addr) {

	addr_info_node *new = malloc(sizeof(addr_info_node));
	if (new == NULL) goto EXIT;

	long parsed_port = strtol(node->data.parser->request.target.port, NULL, 10);
	if ((parsed_port == 0 && errno == EINVAL) || parsed_port < 0 || parsed_port > 65535) {
		logger(ERROR, "connect_to_doh_server(): invalid port. Use a number between 0 and 65535");
		goto FREE_NODE;
	}

	switch (addr_family) {
		case AF_INET:
			new->in4.sin_family = AF_INET;
			new->in4.sin_addr = *((struct in_addr *)addr);
			new->in4.sin_port = htons(parsed_port);
			break;
		case AF_INET6:
			new->in6.sin6_family = AF_INET6;
			new->in6.sin6_addr = *((struct in6_addr *)addr);
			new->in6.sin6_port = htons(parsed_port);
			break;
		default:
			return -1;
	}

	new->next = NULL;

	if (node->data.doh->addr_info_first == NULL) {
		node->data.doh->addr_info_first = new;
		node->data.doh->addr_info_current = node->data.doh->addr_info_first;
	} else {
		addr_info_node *search = node->data.doh->addr_info_first;
		while (search->next != NULL)
			search = search->next;
		search->next = new;
	}

	return 0;

FREE_NODE:
	free(new);
EXIT:
	return -1;
}

void free_doh_resources(connection_node *node) {
	doh_data *data = node->data.doh;

	addr_info_node *addr_node = data->addr_info_first;
	addr_info_node *prev = addr_node;

	while (addr_node != NULL) {
		prev = addr_node;
		addr_node = prev->next;
		free(prev);
	}

	free(data->doh_buffer->data);
	free(data->doh_buffer);
	free(data);

	node->data.doh = NULL;
}

void read_big_endian_16(uint16_t *dest, uint8_t *src, size_t n) {
	for (size_t j = 0; j < n; j++) {
		*dest = 0;
		for (size_t i = 0; i < SIZE_16; i++) {
			*dest = *dest << 8;
			*dest += (uint16_t)*src;
			src += 1;
		}
		dest += SIZE_16;
	}
}

void read_big_endian_32(uint32_t *dest, uint8_t *src, size_t n) {
	for (size_t j = 0; j < n; j++) {
		*dest = 0;
		for (size_t i = 0; i < SIZE_32; i++) {
			*dest = *dest << 8;
			*dest += (uint32_t)*src;
			src += 1;
		}
		dest += SIZE_32;
	}
}

void write_big_endian_16(uint8_t *dest, uint16_t *src, size_t n) {
	for (size_t j = 0; j < n; j++) {
		*dest = 0;
		for (int i = SIZE_16 - 1; i >= 0; i--) {
			dest[i] = (uint8_t)*src;
			*src = *src >> 8;
		}
		dest += SIZE_16;
		src += SIZE_16;
	}
}

void write_big_endian_32(uint8_t *dest, uint32_t *src, size_t n) {
	for (size_t j = 0; j < n; j++) {
		*dest = 0;
		for (int i = SIZE_32 - 1; i >= 0; i--) {
			dest[i] = (uint8_t)*src;
			*src = *src >> 8;
		}
		dest += SIZE_32;
		src += SIZE_32;
	}
}