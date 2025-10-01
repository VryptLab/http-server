#include <unistd.h>
#include <stdatomic.h>
#include "stats.h"

atomic_long total_connections = 0;
atomic_int current_requests = 0;
atomic_int last_rps = 0;

void *rps_counter(void *arg) {
    (void)arg;
    while (1) {
        sleep(1);
        atomic_store(&last_rps, atomic_load(&current_requests));
        atomic_store(&current_requests, 0);
    }
    return NULL;
}
