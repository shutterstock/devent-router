#include <err.h>
#include "third_party/zguide/zhelpers.h"

static struct config {
	char *push;
	uint64_t hwm;
	uint64_t swap;
} config;

static void usage(char *name) {
	printf("Usage: %s [OPTION...] PUSH\n\n", name);
	printf("      --hwm     set high-water mark (number of messages)\n");
	printf("      --swap    set swap size (bytes)\n");
	printf("  -h, --help    display this help list\n");
	printf("\n");
}

static void setup_config(int argc, char **argv) {
	int i, p = 0;

	config.push = NULL;
	config.hwm = 0;
	config.swap = 0;

	for (i = 1; i < argc; i++) {
		int last = i==argc-1;

		if (!strcmp(argv[i], "--hwm") && !last) {
			config.hwm = atoi(argv[++i]);
		} else if (!strcmp(argv[i], "--swap") && !last) {
			config.swap = atoi(argv[++i]);
		} else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			usage(argv[0]);
			exit(EXIT_SUCCESS);
		} else if (p == 0) {
			config.push = argv[i];
			p++;
		} else {
			usage(argv[0]);
			errx(EXIT_FAILURE, "unknown argument");
		}
	}
}

static void free_line(void *data, void *hint) {
	free(data);
}

int main(int argc, char **argv) {
	int rc;

	setup_config(argc, argv);

	if (config.push == NULL) {
		usage(argv[0]);
		errx(EXIT_FAILURE, "no push option set");
	}

	void *context = zmq_init(1);
	assert(context);

	void *socket = zmq_socket(context, ZMQ_PUSH);
	assert(socket);

	if (config.hwm > 0) {
		rc = zmq_setsockopt(socket, ZMQ_HWM, &config.hwm, sizeof(config.hwm));
		if (rc != 0)
			errx(EXIT_FAILURE, "failed to set high-water mark \"%i\"", config.hwm);
	}

	if (config.swap > 0) {
		if (config.hwm < 1)
			errx(EXIT_FAILURE, "hwm must be set to use swap");

		rc = zmq_setsockopt(socket, ZMQ_SWAP, &config.swap, sizeof(config.swap));

		if (rc != 0)
			errx(EXIT_FAILURE, "failed to set swap size \"%i\"", config.swap);
	}

	rc = zmq_connect(socket, config.push);
	if (rc != 0)
		errx(EXIT_FAILURE, "failed to connect to push endpoint \"%s\"", config.push);

	s_catch_signals();

    char *line = NULL;
    size_t line_length, read_length = 0;

	while ((read_length = getline(&line, &line_length, stdin)) != -1 && !s_interrupted) {
		zmq_msg_t message;

		// Remove newline
		line[--read_length] = '\0';

		if (read_length > 0) {
			zmq_msg_init_data(&message, line, read_length, free_line, NULL);
			zmq_send(socket, &message, 0);
			zmq_msg_close(&message);
		} else {
			free(line);
		}

		line = NULL;
	}

	zmq_close(socket);
	zmq_term(context);

	return EXIT_SUCCESS;
}
