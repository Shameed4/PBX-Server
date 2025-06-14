/*
 * TU: simulates a "telephone unit", which interfaces a client with the PBX.
 */
#include <stdlib.h>
#include <pthread.h>
#include <csapp.h>
#include <stdio.h>

#include "pbx.h"
#include "debug.h"

typedef struct tu {
    int fd;
    int ext;
    TU_STATE state;
    struct tu *peer;
    sem_t mutex;
    int ref_count;
} TU;

// assumes that there is a lock 
void print_state(TU *tu) {
    switch (tu->state) {
        case TU_ON_HOOK:
        dprintf(tu->fd, "ON HOOK %d\r\n", tu->ext);
        break;

        case TU_CONNECTED:
        dprintf(tu->fd, "CONNECTED %d\r\n", tu->peer->ext);
        break;

        default:
        dprintf(tu->fd, "%s\r\n", tu_state_names[tu->state]);
    }
}

/*
 * Initialize a TU
 *
 * @param fd  The file descriptor of the underlying network connection.
 * @return  The TU, newly initialized and in the TU_ON_HOOK state, if initialization
 * was successful, otherwise NULL.
 */
// #if 0
TU *tu_init(int fd) {
    // TO BE IMPLEMENTED
    TU *tu = calloc(1, sizeof(TU));
    if (tu == NULL) {
        return NULL;
    }
    Sem_init(&tu->mutex, 0, 1);
    tu->fd = fd;
    return tu;
}
// #endif

// tu_ref, but it assumes there is already a lock on tu
void tu_ref_nl(TU *tu, char *reason) {
    tu->ref_count += 1;
    debug("Refing because: %s. Ref count: %d", reason, tu->ref_count);
}

/*
 * Increment the reference count on a TU.
 *
 * @param tu  The TU whose reference count is to be incremented
 * @param reason  A string describing the reason why the count is being incremented
 * (for debugging purposes).
 */
// #if 0
void tu_ref(TU *tu, char *reason) {
    // TO BE IMPLEMENTED
    P(&tu->mutex);
    tu_ref_nl(tu, reason);
    V(&tu->mutex);
}
// #endif

/*
 * Decrement the reference count on a TU, freeing it if the count becomes 0.
 *
 * @param tu  The TU whose reference count is to be decremented
 * @param reason  A string describing the reason why the count is being decremented
 * (for debugging purposes).
 */
// #if 0
void tu_unref(TU *tu, char *reason) {
    // TO BE IMPLEMENTED
    P(&tu->mutex);
    tu->ref_count -= 1;
    debug("Unrefing because: %s. Ref count: %d", reason, tu->ref_count);
    if (tu->ref_count == 0) {
        debug("Deleting tu");
        V(&tu->mutex);
        close(tu->fd);
        sem_destroy(&tu->mutex);
        free(tu);
    }
    else {
        V(&tu->mutex);
    }
}
// #endif

/*
 * Get the file descriptor for the network connection underlying a TU.
 * This file descriptor should only be used by a server to read input from
 * the connection.  Output to the connection must only be performed within
 * the PBX functions.
 *
 * @param tu
 * @return the underlying file descriptor, if any, otherwise -1.
 */
// #if 0
int tu_fileno(TU *tu) {
    // TO BE IMPLEMENTED
    P(&tu->mutex);
    int fd = tu->fd;
    V(&tu->mutex);
    return fd;
}
// #endif

/*
 * Get the extension number for a TU.
 * This extension number is assigned by the PBX when a TU is registered
 * and it is used to identify a particular TU in calls to tu_dial().
 * The value returned might be the same as the value returned by tu_fileno(),
 * but is not necessarily so.
 *
 * @param tu
 * @return the extension number, if any, otherwise -1.
 */
// #if 0
int tu_extension(TU *tu) {
    // TO BE IMPLEMENTED
    P(&tu->mutex);
    int ext = tu->ext;
    V(&tu->mutex);
    return ext;
}
// #endif

/*
 * Set the extension number for a TU.
 * A notification is set to the client of the TU.
 * This function should be called at most once one any particular TU.
 *
 * @param tu  The TU whose extension is being set.
 */
// #if 0
int tu_set_extension(TU *tu, int ext) {
    // TO BE IMPLEMENTED
    if (tu == NULL)
        return -1;
    P(&tu->mutex);
    tu->ext = ext;
    print_state(tu);
    V(&tu->mutex);
    return 0;
}
// #endif

void lock(TU *tu, TU *target) {
    P(&tu->mutex);
    int tu_fd = tu->fd;
    V(&tu->mutex);

    P(&target->mutex);
    int target_fd = target->fd;
    V(&target->mutex);
    
    if (tu_fd == target_fd) {
        debug("WHAT - tu == target");
        exit(EXIT_FAILURE);
    }
    else if (tu_fd < target_fd) {
        P(&tu->mutex);
        P(&target->mutex);
    }
    else if (target_fd < tu_fd) {
        P(&target->mutex);
        P(&tu->mutex);
    }
}

void unlock(TU *tu, TU *target) {
    debug("Before locking 2");
    int tu_fd = tu->fd;
    int target_fd = target->fd;
    
    if (tu_fd == target_fd) {
        debug("WHAT - tu == target");
        exit(EXIT_FAILURE);
    }
    else if (target_fd < tu_fd) {
        V(&tu->mutex);
        V(&target->mutex);
    }
    else if (tu_fd < target_fd) {
        V(&target->mutex);
        V(&tu->mutex);
    }
}

/*
 * Initiate a call from a specified originating TU to a specified target TU.
 *   If the originating TU is not in the TU_DIAL_TONE state, then there is no effect.
 *   If the target TU is the same as the originating TU, then the TU transitions
 *     to the TU_BUSY_SIGNAL state.
 *   If the target TU already has a peer, or the target TU is not in the TU_ON_HOOK
 *     state, then the originating TU transitions to the TU_BUSY_SIGNAL state.
 *   Otherwise, the originating TU and the target TU are recorded as peers of each other
 *     (this causes the reference count of each of them to be incremented),
 *     the target TU transitions to the TU_RINGING state, and the originating TU
 *     transitions to the TU_RING_BACK state.
 *
 * In all cases, a notification of the resulting state of the originating TU is sent to
 * to the associated network client.  If the target TU has changed state, then its client
 * is also notified of its new state.
 *
 * If the caller of this function was unable to determine a target TU to be called,
 * it will pass NULL as the target TU.  In this case, the originating TU will transition
 * to the TU_ERROR state if it was in the TU_DIAL_TONE state, and there will be no
 * effect otherwise.  This situation is handled here, rather than in the caller,
 * because here we have knowledge of the current TU state and we do not want to introduce
 * the possibility of transitions to a TU_ERROR state from arbitrary other states,
 * especially in states where there could be a peer TU that would have to be dealt with.
 *
 * @param tu  The originating TU.
 * @param target  The target TU, or NULL if the caller of this function was unable to
 * identify a TU to be dialed.
 * @return 0 if successful, -1 if any error occurs that results in the originating
 * TU transitioning to the TU_ERROR state. 
 */
// #if 0
int tu_dial(TU *tu, TU *target) {
    P(&tu->mutex);
    if (tu->state != TU_DIAL_TONE) {
        debug("Cannot dial - not in DIAL TONE state");
        print_state(tu);
        V(&tu->mutex);
        return 0;
    }
    if (target == NULL) {
        debug("Updating to error state");
        tu->state = TU_ERROR;
        V(&tu->mutex);
        return -1;
    }
    V(&tu->mutex);
    
    P(&target->mutex);
    if (target->state != TU_ON_HOOK) {
        V(&target->mutex);
        
        P(&tu->mutex);
        tu->state = TU_BUSY_SIGNAL;
        print_state(tu);
        V(&tu->mutex);
        return 0;
    }
    else
        V(&target->mutex);
    
    debug("About to lock tu and target");
    lock(tu, target);
    debug("Successfully locked");
 
    tu->state = TU_RING_BACK;
    tu->peer = target;
    tu_ref_nl(tu, "Is the caller");
    target->state = TU_RINGING;
    target->peer = tu;
    tu_ref_nl(target, "Is being called");
    print_state(tu);
    print_state(target);

    debug("About to unlock tu and target");
    unlock(tu, target);
    debug("Successfully unlocked");
    return 0;
}
// #endif

/*
 * Take a TU receiver off-hook (i.e. pick up the handset).
 *   If the TU is in neither the TU_ON_HOOK state nor the TU_RINGING state,
 *     then there is no effect.
 *   If the TU is in the TU_ON_HOOK state, it goes to the TU_DIAL_TONE state.
 *   If the TU was in the TU_RINGING state, it goes to the TU_CONNECTED state,
 *     reflecting an answered call.  In this case, the calling TU simultaneously
 *     also transitions to the TU_CONNECTED state.
 *
 * In all cases, a notification of the resulting state of the specified TU is sent to
 * to the associated network client.  If a peer TU has changed state, then its client
 * is also notified of its new state.
 *
 * @param tu  The TU that is to be picked up.
 * @return 0 if successful, -1 if any error occurs that results in the originating
 * TU transitioning to the TU_ERROR state. 
 */
// #if 0
int tu_pickup(TU *tu) {
    // TO BE IMPLEMENTED
    P(&tu->mutex);
    debug("State before pickup: %s", tu_state_names[tu->state]);
    switch (tu->state) {
        case TU_ON_HOOK:
        tu->state = TU_DIAL_TONE;
        print_state(tu);
        debug("New state: %s", tu_state_names[tu->state]);
        V(&tu->mutex);
        break;

        case TU_RINGING:
        V(&tu->mutex);
        TU *target = tu->peer;
        lock(tu, target);

        tu->state = TU_CONNECTED;
        target->state = TU_CONNECTED;

        print_state(tu);
        print_state(target);

        unlock(tu, target);

        break;

        default:
        print_state(tu);
        V(&tu->mutex);
    }
    return 0;
}
// #endif

/*
 * Hang up a TU (i.e. replace the handset on the switchhook).
 *
 *   If the TU is in the TU_CONNECTED or TU_RINGING state, then it goes to the
 *     TU_ON_HOOK state.  In addition, in this case the peer TU (the one to which
 *     the call is currently connected) simultaneously transitions to the TU_DIAL_TONE
 *     state.
 *   If the TU was in the TU_RING_BACK state, then it goes to the TU_ON_HOOK state.
 *     In addition, in this case the calling TU (which is in the TU_RINGING state)
 *     simultaneously transitions to the TU_ON_HOOK state.
 *   If the TU was in the TU_DIAL_TONE, TU_BUSY_SIGNAL, or TU_ERROR state,
 *     then it goes to the TU_ON_HOOK state.
 *
 * In all cases, a notification of the resulting state of the specified TU is sent to
 * to the associated network client.  If a peer TU has changed state, then its client
 * is also notified of its new state.
 *
 * @param tu  The tu that is to be hung up.
 * @return 0 if successful, -1 if any error occurs that results in the originating
 * TU transitioning to the TU_ERROR state. 
 */
// #if 0
int tu_hangup(TU *tu) {
    // TO BE IMPLEMENTED
    P(&tu->mutex);
    switch (tu->state) {
        case TU_CONNECTED:
        case TU_RINGING:
        case TU_RING_BACK:
        TU *target = tu->peer;
        V(&tu->mutex);
        lock(tu, target);
        
        tu->state = TU_ON_HOOK;
        tu->peer = NULL;
        if (tu->state == TU_RING_BACK)
            target->state = TU_ON_HOOK;
        else
            target->state = TU_DIAL_TONE;
        target->peer = NULL;
        print_state(tu);
        print_state(target);

        unlock(tu, target);

        tu_unref(tu, "Hung up");
        tu_unref(target, "Got hung up on");
        break;

        default:
        tu->state = TU_ON_HOOK;
        print_state(tu);
        V(&tu->mutex);
        break;
    }
    return 0;
}
// #endif

/*
 * "Chat" over a connection.
 *
 * If the state of the TU is not TU_CONNECTED, then nothing is sent and -1 is returned.
 * Otherwise, the specified message is sent via the network connection to the peer TU.
 * In all cases, the states of the TUs are left unchanged and a notification containing
 * the current state is sent to the TU sending the chat.
 *
 * @param tu  The tu sending the chat.
 * @param msg  The message to be sent.
 * @return 0  If the chat was successfully sent, -1 if there is no call in progress
 * or some other error occurs.
 */
// #if 0
int tu_chat(TU *tu, char *msg) {
    // TO BE IMPLEMENTED
    P(&tu->mutex);
    if (tu->state != TU_CONNECTED) {
        print_state(tu);
        V(&tu->mutex);
        return -1;
    }
    dprintf(tu->peer->fd, "CHAT %s\r\n", msg);
    print_state(tu);
    V(&tu->mutex);
    return 0;
}
// #endif
