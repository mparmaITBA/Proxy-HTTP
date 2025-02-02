// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include <arpa/inet.h>
#include <getopt.h>
#include <logger.h>
#include <netutils.h>
#include <proxyargs.h>

#define DEFAULT_PROXY_IPV4 "0.0.0.0"
#define DEFAULT_PROXY_IPV6 "::"
#define DEFAULT_MGMT_IPV4 "127.0.0.1"
#define DEFAULT_MGMT_IPV6 "::1"

static void version();
static void usage(const char *program_name);
static uint16_t check_port(const char *service);
static void setup_proxy_addr();
static void setup_management_addr();

proxy_arguments args = {
	// seteo defaults
	.doh_ip = "127.0.0.1", .doh_host = "localhost",	  .doh_port = "8053",	 .doh_path = "/getnsrecord",
	// IP default es NULL para diferenciar de la ingresada por parametro
	.proxy_port = "8080",  .management_port = "9090", .proxy_ip = NULL,	.management_ip = NULL,
	.proxy_addr4 = {0}, .proxy_addr6 = {0}, .management_addr4 = {0}, .management_addr6 = {0},
	.doh_query = "?dns=", .password_dissector = 1
};

void parse_proxy_args(const int argc, char **argv) {

	// variables para getopt_long()
	int c, long_opts_idx;
	char *flags = "hvNp:o:l:L:", has_invalid_arguments = 0;

	// flags con formato long (precedidos por --)
	static const struct option long_opts[] = {{.name = "doh-ip", .has_arg = required_argument, .flag = NULL, .val = DOH_IP},
											  {.name = "doh-host", .has_arg = required_argument, .flag = NULL, .val = DOH_HOST},
											  {.name = "doh-port", .has_arg = required_argument, .flag = NULL, .val = DOH_PORT},
											  {.name = "doh-path", .has_arg = required_argument, .flag = NULL, .val = DOH_PATH},
											  {.name = "doh-query", .has_arg = required_argument, .flag = NULL, .val = DOH_QUERY},
											  {.name = 0, .has_arg = 0, .flag = 0, .val = 0}};

	while (1) {
		c = getopt_long(argc, argv, flags, long_opts, &long_opts_idx);
		if (c == -1) { break; }

		switch (c) {
			case 'h':
				usage(argv[0]);
				break;
			case 'v':
				version();
				break;
			case 'N':
				args.password_dissector = 0;
				break;
			case 'p':
				// check_port(optarg);
				args.proxy_port = optarg;
				break;
			case 'o':
				// check_port(optarg);
				args.management_port = optarg;
				break;
			case 'l':
                // check_addr(optarg);
				args.proxy_ip = optarg;
				break;
			case 'L':
                // check_addr(optarg);
				args.management_ip = optarg;
				break;
			case DOH_IP:
				// check_addr(optarg);
				args.doh_ip = optarg;
				break;
			case DOH_HOST:
				args.doh_host = optarg;
				break;
			case DOH_PORT:
				// check_port(optarg);
				args.doh_port = optarg;
				break;
			case DOH_PATH:
				args.doh_path = optarg;
				break;
			case DOH_QUERY:
				args.doh_query = optarg;
				break;
			default:
				fprintf(stderr, "Invalid argument %s\n", argv[optind - 1]);
				has_invalid_arguments = 1;
				break;
		}
	}

	if (optind < argc) {
		fprintf(stderr, "Argument%snot accepted\n\t\t", optind == (argc - 1) ? " " : "s ");
		while (optind < argc) {
			fprintf(stderr, " %s,", argv[optind++]);
			if (optind == argc) fprintf(stderr, "%c", '\b');
		}
		exit(EXIT_FAILURE);
	}

	if(has_invalid_arguments) exit(EXIT_FAILURE);

	setup_proxy_addr();
	setup_management_addr();

	// Puerto y direccion donde se encuentra el servidor DoH
	uint16_t doh_port = check_port(args.doh_port);
	addr_info doh_addr;
	if(!(parse_ip_address(args.doh_ip, doh_port, &doh_addr)))
		logger(FATAL, "Bad IP address for DoH server");

	args.doh_addr_info = doh_addr;
}

static void setup_proxy_addr() {
	// Validaciones + asisgnar a la estructura addr correspondiente
	// Puerto y direccion donde escucha el proxy
	uint16_t proxy_port = check_port(args.proxy_port);
	addr_info proxy_addr_aux;
	if(args.proxy_ip != NULL) {
		//	Caso IP para el proxy pasada por argumento
		if(!(parse_ip_address(args.proxy_ip, proxy_port, &proxy_addr_aux)))
		logger(FATAL, "Bad argument for IP address for proxy");

		switch (proxy_addr_aux.addr.sa_family) {
			case AF_INET:
				args.proxy_addr4 = proxy_addr_aux.in4;
				break;
			case AF_INET6:
				args.proxy_addr6 = proxy_addr_aux.in6;
				break;
		}
	} else {
		//	Seteo los addr default para escuchar en todas las interfaces
		if(!(parse_ip_address(DEFAULT_PROXY_IPV4, proxy_port, &proxy_addr_aux)))
		logger(FATAL, "Error when setting up proxy default IPv4 address");

		args.proxy_addr4 = proxy_addr_aux.in4;

		if(!(parse_ip_address(DEFAULT_PROXY_IPV6, proxy_port, &proxy_addr_aux)))
		logger(FATAL, "Error when setting up proxy default IPv6 address");

		args.proxy_addr6 = proxy_addr_aux.in6;
	}
}

static void setup_management_addr() {
	// Puerto y direccion donde escucha el servidor para monitoreo
	uint16_t management_port = check_port(args.management_port);
	addr_info management_addr_aux;

	if(args.management_ip != NULL) {
		//	Caso IP para el proxy pasada por argumento
		if(!(parse_ip_address(args.management_ip, management_port, &management_addr_aux)))
			logger(FATAL, "Bad argument IP address for management");

		switch (management_addr_aux.addr.sa_family) {
			case AF_INET:
				args.management_addr4 = management_addr_aux.in4;
				break;
			case AF_INET6:
				args.management_addr6 = management_addr_aux.in6;
				break;
		}
	} else {
		//	Seteo los addr default para escuchar en todas las interfaces
		if(!(parse_ip_address(DEFAULT_MGMT_IPV4, management_port, &management_addr_aux)))
			logger(FATAL, "Error when setting up management default IPv4 address");

		args.management_addr4 = management_addr_aux.in4;

		if(!(parse_ip_address(DEFAULT_MGMT_IPV6, management_port, &management_addr_aux)))
			logger(FATAL, "Error when setting up management default IPv6 address");

		args.management_addr6 = management_addr_aux.in6;
	}
}

static void version() {
	fprintf(stderr, "Proxy HTTP version 1.0\n"
					"ITBA\n"
					"Protocolos de Comunicacion 2021/1 - Grupo 7\n"
					"Licencia: ...\n");
	exit(EXIT_SUCCESS);
}

static void usage(const char *program_name) {
	fprintf(stderr,
			"OPTIONS\n"
			"\t-h\n\t\tImprime la ayuda y finaliza.\n\n"
			"\t-v\n\t\tImprime la versión del programa %s y finaliza.\n\n"
			"\t-N\n\t\tDeshabilita los passwords disectors.\n\n"
			"\t-p puerto-local\n\t\tPuerto TCP donde el proxy HTTP escucha conexiones. Por defecto toma el valor 8080.\n\n"
			"\t-o puerto-de-management\n\t\tPuerto donde el servicio de management escucha conexiones. Por defecto toma el valor "
			"9090.\n\n"
			"\t-l dirección-http\n\t\tEstablece la dirección donde el proxy HTTP brinda servicio. Por defecto escucha en todas "
			"las interfaces.\n\n"
			"\t-L dirección-de-management\n\t\tEstablece la dirección donde se brinda el servicio de management. Por defecto "
			"escucha en loopback.\n\n"
			"\t--doh-ip dirección\n\t\tEstablece la direccion del servidor DoH. Por defecto toma el valor 127.0.0.1 .\n\n"
			"\t--doh-port puerto\n\t\tPuerto donde se encuentra el servidor DoH. Por defecto toma el valor 8053.\n\n"
			"\t--doh-host hostname\n\t\tEstablece el valor del header Host de la petición DoH. Por defecto localhost.\n\n"
			"\t--doh-path path\n\t\tEstablece el path de la petición DoH. Por defecto toma el valor /getnsrecord.\n\n",
			program_name);
	exit(EXIT_SUCCESS);
}

static uint16_t check_port(const char *service) {
	uint16_t port;

	if(!parse_port(service, &port))
		logger(FATAL, "Port should be between 0 - 65535: %s", service);

	return port;
}
