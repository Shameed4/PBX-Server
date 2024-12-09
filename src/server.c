/*
 * "PBX" server module.
 * Manages interaction with a client telephone unit (TU).
 */
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "debug.h"
#include "pbx.h"
#include "server.h"

#define BUFFER_BLOCK_LEN 100

/*
 * Thread function for the thread that handles interaction with a client TU.
 * This is called after a network connection has been made via the main server
 * thread and a new thread has been created to handle the connection.
 */
// #if 0
void *pbx_client_service(void *arg) {
    if (pthread_detach(pthread_self())) {
        fprintf(stderr, "Failed to detach thread\n");
        exit(1);
    }
    int connfdp = *(int*)(arg);
    free(arg);
    debug("Thread connfdp: %d", connfdp);
    while (1) {
        char *buffer = malloc(BUFFER_BLOCK_LEN + 1);
        if (buffer == NULL) {
            fprintf(stderr, "Failed to allocate memory\n");
            exit(EXIT_FAILURE);
        }
        buffer[0] = '\0';

        int len = 0;
        int curr_read_len = 0;
        char prev = 0;
        int break_index = -1;
        while ((curr_read_len = read(connfdp, (buffer + len), BUFFER_BLOCK_LEN)) > 0) {
            len += curr_read_len;
            buffer[len] = '\0';
            for (int i = len - curr_read_len; i < len; i++) {
                char curr = buffer[i];
                if (prev == '\r' && curr == '\n') {
                    break_index = i - 1;
                    break;
                }
                prev = curr;
            }

            if (break_index != -1) {
                break;
            }

            char *re_buffer = realloc(buffer, len + BUFFER_BLOCK_LEN + 1);
            if (!re_buffer) {
                fprintf(stderr, "Failed to reallocate memory\n");
                free(buffer);
                exit(EXIT_FAILURE);
            }
            buffer = re_buffer;
        }
        buffer[break_index] = '\0';

        if (!strcmp(buffer, "pickup")) {
            debug("Picking up");
        }
        else if (!strcmp(buffer, "hangup")) {
            debug("Hanging up");
        }
        else if (!strncmp(buffer, "dial ", 5)) {
            debug("Dialing");
        }
        else if (!strncmp(buffer, "chat ", 5)) {
            debug("Chatting");
        }
        else {
            debug("Invalid command");
        }
        free(buffer);
    }
    return NULL;
}
// #endif
