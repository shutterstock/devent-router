#include <err.h>
#include "third_party/zguide/zhelpers.h"

static struct config {
    char *push;
    char *topic;
    uint64_t hwm;
    uint64_t swap;
} config;

static void usage(char *name) {
    printf("Usage: %s [OPTION...] PUSH TOPIC\n\n", name);
    printf("      --hwm     set high-water mark (number of messages)\n");
    printf("      --swap    set swap size (bytes)\n");
    printf("  -h, --help    display this help list\n");
    printf("\n");
}

static void setup_config(int argc, char **argv) {
    int i, p = 0;

    config.push = NULL;
    config.topic = NULL;
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
                    config.push = argv[i];
                    break;
                case 1:
                    config.topic = argv[i];
                    break;
            }
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
        errx(EXIT_FAILURE, "topic argument is required");
    }

    if (config.topic == NULL) {
        usage(argv[0]);
        errx(EXIT_FAILURE, "topic argument is required");
    }

    void *context = zmq_init(1);
    assert(context);

    void *socket = zmq_socket(context, ZMQ_PUSH);
    assert(socket);

    if (config.hwm > 0) {
        rc = zmq_setsockopt(socket, ZMQ_HWM, &config.hwm, sizeof(config.hwm));
        if (rc != 0)
            errx(EXIT_FAILURE, "failed to set high-water mark \"%"PRIu64"\"", config.hwm);
    }

    if (config.swap > 0) {
        if (config.hwm < 1)
            errx(EXIT_FAILURE, "hwm must be set to use swap");

        rc = zmq_setsockopt(socket, ZMQ_SWAP, &config.swap, sizeof(config.swap));

        if (rc != 0)
            errx(EXIT_FAILURE, "failed to set swap size \"%"PRIu64"\"", config.swap);
    }

    rc = zmq_connect(socket, config.push);
    if (rc != 0)
        errx(EXIT_FAILURE, "failed to connect to push endpoint \"%s\"", config.push);

    s_catch_signals();

    char *line = NULL;
    size_t line_length = 0;
    size_t read_length = 0;
    size_t topic_length = strlen(config.topic);

    while ((read_length = getline(&line, &line_length, stdin)) != -1 && !s_interrupted) {
        zmq_msg_t topic;
        zmq_msg_t body;

        // Remove newline
        line[--read_length] = '\0';

        if (read_length > 0) {
            // TODO(ssewell): use zmq_msg_copy (couldn't get it to work)
            zmq_msg_init_size(&topic, topic_length);
            memcpy(zmq_msg_data(&topic), config.topic, topic_length);
            zmq_send(socket, &topic, ZMQ_SNDMORE);
            zmq_msg_close(&topic);

            zmq_msg_init_data(&body, line, read_length, free_line, NULL);
            zmq_send(socket, &body, 0);
            zmq_msg_close(&body);
        } else {
            free(line);
        }

        line = NULL;
    }

    zmq_close(socket);
    zmq_term(context);

    return EXIT_SUCCESS;
}
