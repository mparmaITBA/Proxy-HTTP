#ifndef DOH_SENDER_H

#define DOH_SENDER_H

#include <unistd.h>

ssize_t write_http_request(int fd, char *name);

#endif
