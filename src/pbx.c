/*
 * PBX: simulates a Private Branch Exchange.
 */
#include <stdlib.h>

#include "pbx.h"
#include "debug.h"
#include "csapp.h"

typedef struct pbx_node {
    TU *tu;
    int ext;
    struct pbx_node *next;
} PBX_NODE;

typedef struct pbx {
    PBX_NODE *head;
    sem_t mutex, w;
    int read_cnt;
} PBX;


static void add_reader() {
    P(&pbx->mutex);
    if (pbx->read_cnt == 0) {
        P(&pbx->w);
    }
    pbx->read_cnt++;
    V(&pbx->mutex);
}

static void remove_reader() {
    P(&pbx->mutex);
    pbx->read_cnt--;
    if (pbx->read_cnt == 0) {
        V(&pbx->w);
    }
    V(&pbx->mutex);
}

/*
 * Initialize a new PBX.
 *
 * @return the newly initialized PBX, or NULL if initialization fails.
 */
// #if 0
PBX *pbx_init() {
    pbx = calloc(1, sizeof(PBX));
    if (pbx == NULL)
        return NULL;
    Sem_init(&pbx->mutex, 0, 1);
    Sem_init(&pbx->w, 0, 1);
    return pbx;
}
// #endif

/*
 * Shut down a pbx, shutting down all network connections, waiting for all server
 * threads to terminate, and freeing all associated resources.
 * If there are any registered extensions, the associated network connections are
 * shut down, which will cause the server threads to terminate.
 * Once all the server threads have terminated, any remaining resources associated
 * with the PBX are freed.  The PBX object itself is freed, and should not be used again.
 *
 * @param pbx  The PBX to be shut down.
 */
// #if 0
void pbx_shutdown(PBX *pbx) {
    // TO BE IMPLEMENTED
    P(&pbx->w);
    PBX_NODE *node = pbx->head;
    while (node) {
        PBX_NODE *next = node->next;
        tu_unref(node->tu, "Shutting down PBX");
        free(node);
        node = next;
    }
    V(&pbx->w);
    // free the pbx, shut down file descriptors
}
// #endif

/*
 * Register a telephone unit with a PBX at a specified extension number.
 * This amounts to "plugging a telephone unit into the PBX".
 * The TU is initialized to the TU_ON_HOOK state.
 * The reference count of the TU is increased and the PBX retains this reference
 *for as long as the TU remains registered.
 * A notification of the assigned extension number is sent to the underlying network
 * client.
 *
 * @param pbx  The PBX registry.
 * @param tu  The TU to be registered.
 * @param ext  The extension number on which the TU is to be registered.
 * @return 0 if registration succeeds, otherwise -1.
 */
// #if 0
int pbx_register(PBX *pbx, TU *tu, int ext) {
    P(&pbx->w);
    PBX_NODE *node = malloc(sizeof(PBX_NODE));
    if (node == NULL) {
        V(&pbx->w);
        return -1;
    }
    node->tu = tu;
    node->ext = ext;
    node->next = pbx->head;
    pbx->head = node;
    tu_set_extension(tu, ext);
    tu_ref(tu, "Registering to PBX");
    V(&pbx->w);
    return 0;
}
// #endif

/*
 * Unregister a TU from a PBX.
 * This amounts to "unplugging a telephone unit from the PBX".
 * The TU is disassociated from its extension number.
 * Then a hangup operation is performed on the TU to cancel any
 * call that might be in progress.
 * Finally, the reference held by the PBX to the TU is released.
 *
 * @param pbx  The PBX.
 * @param tu  The TU to be unregistered.
 * @return 0 if unregistration succeeds, otherwise -1.
 */
// #if 0
int pbx_unregister(PBX *pbx, TU *tu) {
    P(&pbx->w);
    if (pbx->head == NULL) {
        V(&pbx->w);
        return -1;
    }
    
    PBX_NODE *removed;
    if (pbx->head->tu == tu) {
        removed = pbx->head;
        pbx->head = removed->next;
        tu_unref(removed->tu, "Unregistered tu");
        free(removed);
        V(&pbx->w);
        return 0;
    }

    PBX_NODE *curr = pbx->head;
    while (curr->next != NULL && curr->next->tu != tu) {
        curr = curr->next;
    }
    removed = curr->next;
    if (removed == NULL) {
        V(&pbx->w);
        return -1;
    }
    curr->next = removed->next;
    tu_unref(removed->tu, "Unregistered tu");
    free(removed);
    return 0;
}
// #endif

/*
 * Use the PBX to initiate a call from a specified TU to a specified extension.
 *
 * @param pbx  The PBX registry.
 * @param tu  The TU that is initiating the call.
 * @param ext  The extension number to be called.
 * @return 0 if dialing succeeds, otherwise -1.
 */
// #if 0
int pbx_dial(PBX *pbx, TU *tu, int ext) {
    add_reader();

    PBX_NODE *node = pbx->head;
    while (node != NULL && node->ext != ext) {
        node = node->next;
    }
    int res = tu_dial(tu, node == NULL ? NULL : node->tu);
    remove_reader();
    return res;
}
// #endif
