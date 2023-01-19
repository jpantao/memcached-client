#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <libmemcached/memcached.h>


#define DEFAULT_CONFIG "--SERVER=127.0.0.1:11211"
#define DEFAULT_N_KEYS 10       // default number of items to insert
#define DEFAULT_KEY_LEN 16      // default key size = 16 bytes
#define DEFAULT_VAL_LEN 1024    // default value size = 1 K
#define DEFAULT_N_OPS 10        // default number of operations to perform

char *config_string = DEFAULT_CONFIG;
int n_keys = DEFAULT_N_KEYS;
int key_len = DEFAULT_KEY_LEN;
int val_len = DEFAULT_VAL_LEN;
int n_ops = DEFAULT_N_OPS;


static __inline__ unsigned long time_diff(struct timeval *start, struct timeval *stop) {
    register unsigned long sec_res = stop->tv_sec - start->tv_sec;
    register unsigned long usec_res = stop->tv_usec - start->tv_usec;
    return 1000000 * sec_res + usec_res;
}


int generate_string(char* str, int len) {
    int i;
    for (i = 0; i < len; i++) {
        str[i] = 'a' + (rand() % 26);
    }
    str[len] = '\0';
    return 0;
}

int init_keys(char **keys, int n, int len) {
    for (int i = 0; i < n; i++) {
        keys[i] = (char *) malloc(len + 1);
        generate_string(keys[i], len);
    }
    return 0;
}

int free_keys(char **keys, int n) {
    int i;
    for (i = 0; i < n; i++) {
        free(keys[i]);
    }
    return 0;
}


int main(int argc, char *argv[]) {
    memcached_st *memc = memcached(config_string, strlen(config_string));

    struct timeval tstart, tend;


    char **keys = malloc(n_keys * sizeof(char *));
    init_keys(keys, n_keys, key_len);

    // insert keys
    for (int i = 0; i < n_keys; i++) {
        char value[val_len];
        generate_string(value, val_len);
        printf("Inserting key %s with value %s\n", keys[i], value);
        memcached_add(memc, keys[i], key_len, value , val_len,0, 0);
    }

    // main loop
    gettimeofday(&tstart, NULL);
    for (int i = 0; i < n_ops; i++) {
        char* key = keys[i % n_keys];

        // get key
        printf("Getting key %s\n", key);
        size_t value_length;
        uint32_t flags;
        memcached_return rc;
        char *value = memcached_get(memc, key, key_len, &value_length, &flags, &rc);
        if (value == NULL) {
            printf("memcached_get() failed: %s, key: %s\n", memcached_strerror(memc, rc), key);
            exit(1);
        }

        // update key
        // generate new value (required? or can we use a predefined one?)
        generate_string(value, val_len);
        printf("Updating key %s with value %s\n", key, value);
        memcached_replace(memc, key, key_len, value, val_len, 0, 0);
        free(value);
    }
    gettimeofday(&tend, NULL);


    free_keys(keys, n_keys);
    free(keys);
    memcached_free(memc);


    return 0;
}
