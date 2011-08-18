#include <err.h>
#include "third_party/zguide/zhelpers.h"

static struct config {
	char *pull;
	char *pub;
	uint64_t hwm;
	uint64_t swap;
} config;

static void usage(char *name) {
	printf("Usage: %s [OPTION...] PULL PUB\n\n", name);
	printf("  --hwm      set high-water mark (number of messages)\n");
	printf("  --swap     set swap size (bytes)\n");
	printf("  -h, --help display this help list\n");
	printf("\n");
}

static void setup_config(int argc, char **argv) {
	int i, p = 0;

	config.pull = NULL;
	config.pub = NULL;
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
		} else if (argv[i][0] != '-') {
			switch (p) {
				case 0:
					config.pull = argv[i];
					break;
				case 1:
					config.pub = argv[i];
					break;
			}
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

	if (config.pull == NULL) {
		usage(argv[0]);
		errx(EXIT_FAILURE, "no pull option set");
	}

	if (config.pub == NULL) {
		usage(argv[0]);
		errx(EXIT_FAILURE, "no pub option set");
	}

	void *context = zmq_init(1);
	assert(context);

	void *pull_socket = zmq_socket(context, ZMQ_PULL);
	assert(pull_socket);

	void *pub_socket = zmq_socket(context, ZMQ_PUB);
	assert(pub_socket);

	if (config.hwm > 0) {
		rc = zmq_setsockopt(pub_socket, ZMQ_HWM, &config.hwm, sizeof(config.hwm));
		if (rc != 0)
			errx(EXIT_FAILURE, "failed to set high-water mark \"%"PRIu64"\"", config.hwm);
	}

	if (config.swap > 0) {
		if (config.hwm < 1)
			errx(EXIT_FAILURE, "hwm must be set to use swap");

		rc = zmq_setsockopt(pub_socket, ZMQ_SWAP, &config.swap, sizeof(config.swap));

		if (rc != 0)
			errx(EXIT_FAILURE, "failed to set swap size \"%"PRIu64"\"", config.swap);
	}

	rc = zmq_bind(pull_socket, config.pull);
	if (rc != 0)
		errx(EXIT_FAILURE, "failed to bind pull endpoint \"%s\"", config.pull);

	rc = zmq_bind(pub_socket, config.pub);
	if (rc != 0)
		errx(EXIT_FAILURE, "failed to bind pub endpoint \"%s\"", config.pub);

	s_catch_signals();

	int64_t more;
	size_t more_size;

	zmq_msg_t msg;
	zmq_msg_init(&msg);

	while (!s_interrupted) {
		if (zmq_recv(pull_socket, &msg, 0)) continue;

		more_size = sizeof(more);
		zmq_getsockopt(pull_socket, ZMQ_RCVMORE, &more, &more_size);
		zmq_send(pub_socket, &msg, more ? ZMQ_SNDMORE : 0);
	}

	zmq_close(pull_socket);
	zmq_close(pub_socket);
	zmq_term(context);

	printf("\n");

	return EXIT_SUCCESS;
}
