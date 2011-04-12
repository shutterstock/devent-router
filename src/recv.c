#include <err.h>
#include "third_party/zguide/zhelpers.h"

static struct config {
	char *filter;
	char *sub;
} config;

static void usage(char *name) {
	printf("Usage: %s [OPTION...] SUB [FILTER]\n\n", name);
	printf("  -h, --help    display this help list\n");
	printf("\n");
}

static void setup_config(int argc, char **argv) {
	int i, p = 0;

	config.filter = NULL;
	config.sub = NULL;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			usage(argv[0]);
			exit(EXIT_SUCCESS);
		} else if (p == 0) {
			config.sub = argv[i];
			p++;
		} else if (p == 1) {
			config.filter = argv[i];
			p++;
		} else {
			usage(argv[0]);
			errx(EXIT_FAILURE, "unknown argument");
		}
	}
}

int main(int argc, char **argv) {
	int rc;

	setup_config(argc, argv);

	if (config.sub == NULL) {
		usage(argv[0]);
		errx(EXIT_FAILURE, "no sub option set");
	}

	void *context = zmq_init(1);
	assert(context);

	void *socket = zmq_socket(context, ZMQ_SUB);
	assert(socket);

	if (config.filter == NULL) {
		zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0);
	} else {
		zmq_setsockopt(socket, ZMQ_SUBSCRIBE, config.filter, strlen(config.filter));
	}

	rc = zmq_connect(socket, config.sub);
	if (rc != 0)
		errx(EXIT_FAILURE, "failed to connect to sub endpoint \"%s\"", config.sub);

	s_catch_signals();

	char *line = NULL;

	while (!s_interrupted) {
		line = s_recv(socket);
		if (line) {
			printf("%s\n", line);
			free(line);
		}
		line = NULL;
	}

	zmq_close(socket);
	zmq_term(context);

	printf("\n");

	return EXIT_SUCCESS;
}
