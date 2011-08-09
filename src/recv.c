#include <err.h>
#include "third_party/zguide/zhelpers.h"

static struct config {
    char *topic;
    char *sub;
    int  show_topic;
} config;

static void usage(char *name) {
    printf("Usage: %s [OPTION...] SUB [TOPIC]\n\n", name);
    printf("  -h, --help    display this help list\n");
    printf("  --show-topic  prefix lines with topic\n");
    printf("\n");
}

static void setup_config(int argc, char **argv) {
    int i, p = 0;

    config.topic = NULL;
    config.sub = NULL;
    config.show_topic = 0;

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            usage(argv[0]);
            exit(EXIT_SUCCESS);
        } else if (!strcmp(argv[i], "--show-topic")) {
            config.show_topic = 1;
        } else if (argv[i][0] != '-') {
            switch (p) {
                case 0:
                    config.sub = argv[i];
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

    if (config.topic == NULL) {
        zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0);
    } else {
        zmq_setsockopt(socket, ZMQ_SUBSCRIBE, config.topic, strlen(config.topic));
    }

    rc = zmq_connect(socket, config.sub);
    if (rc != 0)
        errx(EXIT_FAILURE, "failed to connect to sub endpoint \"%s\"", config.sub);

    s_catch_signals();

    size_t size = 0;
    char *line = NULL;
    char *prefix = NULL;

    while (!s_interrupted) {
        zmq_msg_t topic;
        zmq_msg_t body;

        zmq_msg_init(&topic);
        zmq_msg_init(&body);

        if (!zmq_recv(socket, &topic, 0)) {
            int64_t more;
            size_t more_size = sizeof(more);

            // Get topic name
            size = zmq_msg_size(&topic);
            prefix = malloc(size+1);
            memcpy(prefix, zmq_msg_data(&topic), size);
            zmq_msg_close(&topic);
            prefix[size] = '\0';

            zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &more_size);

            if (more) {
                zmq_recv(socket, &body, 0);
                size = zmq_msg_size(&body);
                line = malloc(size+1);
                memcpy(line, zmq_msg_data(&body), size);
                zmq_msg_close(&body);
                line[size] = '\0';
            }
        }

        if (config.show_topic && prefix && line) {
            printf("%s\t%s\n", prefix, line);
        } else if (line) {
            printf("%s\n", line);
        }
        if (prefix) free(prefix);
        if (line) free(line);
        prefix = NULL;
        line = NULL;
    }

    zmq_close(socket);
    zmq_term(context);

    printf("\n");

    return EXIT_SUCCESS;
}
