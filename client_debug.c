#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <libmemcached/memcached.h>

#define DEFAULT_CONFIG "--SERVER=127.0.0.1:11211"

static __inline__ unsigned long time_diff(struct timeval *start, struct timeval *stop) {
    register unsigned long sec_res = stop->tv_sec - start->tv_sec;
    register unsigned long usec_res = stop->tv_usec - start->tv_usec;
    return 1000000 * sec_res + usec_res;
}

int main(int argc, char *argv[]) {
    memcached_st *memc = memcached(DEFAULT_CONFIG, strlen(DEFAULT_CONFIG));
//    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_CONNECT_TIMEOUT, INT64_MAX);
//    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_SND_TIMEOUT, INT64_MAX);
//    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_RCV_TIMEOUT, INT64_MAX);


    struct timeval tstart, tend;

    char *key = "debugKey";
    char *val = "debugvalue";
    memcached_return_t rc;

    //insert key
    if (strcmp(argv[1], "-i") == 0) {
        gettimeofday(&tstart, NULL);
        rc = memcached_add(memc, key, strlen(key), val, strlen(val), 0, 0);
        if (rc != MEMCACHED_SUCCESS)
            printf("Error inserting key %s: %s\n", key, memcached_strerror(memc, rc));
        gettimeofday(&tend, NULL);
        printf("key: %s (len=%lu), val: %s(len=%lu)\n", key, strlen(key), val, strlen(val));
        printf("Insert time: %lu us\n", time_diff(&tstart, &tend));
    }

    //get key
    if (strcmp(argv[1], "-g") == 0) {
        gettimeofday(&tstart, NULL);
        char *v = memcached_get(memc, key, strlen(key), NULL, NULL, &rc);
        if (v == NULL)
            printf("memcached_get() failed: %s, key: %s\n", memcached_strerror(memc, rc), key);
        else
            printf("key: %s (len=%lu), val: %s(len=%lu)\n", key, strlen(key), v, strlen(v));
        gettimeofday(&tend, NULL);
        printf("Get time: %lu us\n", time_diff(&tstart, &tend));
    }


    return 0;
}
