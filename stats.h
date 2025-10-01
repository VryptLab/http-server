#ifndef STATS_H
#define STATS_H

#include <stdatomic.h>

extern atomic_long total_connections;
extern atomic_int current_requests;
extern atomic_int last_rps;

void *rps_counter(void *arg);

#endif
