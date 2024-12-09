#include <stdlib.h>
#include <unistd.h>

#include "csapp.h"
#include "pbx.h"
#include "server.h"
#include "debug.h"
#include "main_helper.h"

static void terminate(int status);

/*
 * "PBX" telephone exchange simulation.
 *
 * Usage: pbx <port>
 */
int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.

    // Perform required initialization of the PBX module.
    debug("Initializing PBX...");
    pbx = pbx_init();

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function pbx_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.

    // if (argc < 3) {
    //     fprintf(stderr, )
    // }
    
    char *port = NULL;
    for (int i = 1; i < argc - 1; i++) {
        if (!strcmp(argv[i], "-p")) {
            i++;
            port = argv[i];
        }
    }

    if (port == NULL) {
        fprintf(stderr, "Usage: bin/pbx -p <port>\n");
        terminate(EXIT_FAILURE);
    }

    struct sigaction sa;
    sa.sa_handler = terminate;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGHUP, &sa, NULL) ==  -1) {
        fprintf(stderr, "Failed to add sigaction");
        exit(1);
    }

    // adapted from Lee-LEC21-Concurrency.pdf Slide 41
    int listenfd, *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    listenfd = Open_listenfd(port);

    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfdp = Malloc(sizeof(int));
        *connfdp = Accept(listenfd, (SA *) &clientaddr, &clientlen);
        debug("Main connfdp: %d", *connfdp);
        debug("Accepted connection");
        pthread_create(&tid, NULL, pbx_client_service, connfdp);
    }
    

    terminate(EXIT_FAILURE);
}

/*
 * Function called to cleanly shut down the server.
 */
static void terminate(int status) {
    debug("Shutting down PBX...");
    pbx_shutdown(pbx);
    debug("PBX server terminating");
    exit(status);
}
