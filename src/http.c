#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <simplehttp/simplehttp.h>
#include "third_party/zguide/zhelpers.h"

static void *socket = NULL;
static char *simplehttp_argv[5];

static struct config {
    char *push;
    uint64_t hwm;
    uint64_t swap;
} config;

static void usage(char *name) {
    printf("Usage: %s [OPTION...] PUSH\n\n", name);
    printf("      --hwm          set high-water mark (number of messages)\n");
    printf("      --interface    set http interface\n");
    printf("      --port         set http port\n");
    printf("      --swap         set swap size (bytes)\n");
    printf("  -h, --help         display this help list\n");
    printf("\n");
}

static void setup_config(int argc, char **argv) {
    int i, p = 0;

    config.push = NULL;
    config.hwm = 0;
    config.swap = 0;

    simplehttp_argv[0] = argv[0];

    simplehttp_argv[1] = calloc(3, sizeof(char));
    strcpy(simplehttp_argv[1], "-a");

    simplehttp_argv[3] = calloc(3, sizeof(char));
    strcpy(simplehttp_argv[3], "-p");

    simplehttp_argv[2] = NULL;
    simplehttp_argv[4] = NULL;

    for (i = 1; i < argc; i++) {
        int last = i==argc-1;

        if (!strcmp(argv[i], "--hwm") && !last) {
            config.hwm = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--swap") && !last) {
            config.swap = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--interface") && !last) {
            simplehttp_argv[2] = argv[++i];
        } else if (!strcmp(argv[i], "--port") && !last) {
            simplehttp_argv[4] = argv[++i];
        } else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            usage(argv[0]);
            exit(EXIT_SUCCESS);
        } else if (argv[i][0] != '-') {
            switch (p) {
                case 0:
                    config.push = argv[i];
                    break;
            }
            p++;
        } else {
            usage(argv[0]);
            errx(EXIT_FAILURE, "unknown argument");
        }
    }

    if (simplehttp_argv[2] == NULL) {
        simplehttp_argv[2] = calloc(8, sizeof(char));
        strcpy(simplehttp_argv[2], "0.0.0.0");
    }

    if (simplehttp_argv[4] == NULL) {
        simplehttp_argv[4] = calloc(5, sizeof(char));
        strcpy(simplehttp_argv[4], "8080");
    }
}

static void free_line(void *data, void *hint) {
    free(data);
}

void cb(struct evhttp_request *req, struct evbuffer *evb, void *ctx) {
    char *data;
    zmq_msg_t message;
    size_t data_length;

    data = evhttp_decode_uri(req->uri+sizeof(char));
    data_length = strlen(data);

    zmq_msg_init_data(&message, data, data_length, free_line, NULL);
    zmq_send(socket, &message, 0);
    zmq_msg_close(&message);

    evhttp_send_reply(req, HTTP_OK, "OK", evb);
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

    socket = zmq_socket(context, ZMQ_PUSH);
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

    simplehttp_init();
    simplehttp_set_cb("/*", cb, NULL);
    simplehttp_main(5, simplehttp_argv);

    zmq_close(socket);
    zmq_term(context);

    return 0;
}
