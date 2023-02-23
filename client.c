#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <libmemcached/memcached.h>

#define DEFAULT_CONFIG "--SERVER=127.0.0.1:11211"

#define DEFAULT_N_OPS 10    // default number of operations to perform
#define DEFAULT_N_KEYS 10   // default number of items to insert
#define DEFAULT_KEY_LEN 16  // default key size = 16 bytes
#define DEFAULT_VAL_LEN 64  // default value size = 64 bytes

char *config_string = DEFAULT_CONFIG;
int n_keys = DEFAULT_N_KEYS;
int key_len = DEFAULT_KEY_LEN;
int val_len = DEFAULT_VAL_LEN;
int n_ops = DEFAULT_N_OPS;
bool verbose = false;

static __inline__ unsigned long time_diff(struct timeval *start, struct timeval *stop) {
    register unsigned long sec_res = stop->tv_sec - start->tv_sec;
    register unsigned long usec_res = stop->tv_usec - start->tv_usec;
    return 1000000 * sec_res + usec_res;
}

void argparse(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--config") == 0 || strcmp(argv[i], "-c") == 0) {
            config_string = argv[++i];
        } else if (strcmp(argv[i], "--n-keys") == 0 || strcmp(argv[i], "-nk") == 0) {
            n_keys = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--key-len") == 0 || strcmp(argv[i], "-kl") == 0) {
            key_len = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--val-len") == 0 || strcmp(argv[i], "-vl") == 0) {
            val_len = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--n-operations") == 0 || strcmp(argv[i], "-no") == 0) {
            n_ops = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
            verbose = true;
        } else {
            printf("Usage: %s "
                   "[--config|-c <config_string>] "
                   "[--n-keys|-nk <n_keys>] "
                   "[--key-len|-kl <key_len>] "
                   "[--val-len|-vl <val_len>] "
                   "[--n-operations|-no <n_ops>]"
                   "\n", argv[0]);
            exit(1);
        }
    }
}

int generate_string(char *str, int len) {
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
    argparse(argc, argv);

    memcached_st *memc = memcached(config_string, strlen(config_string));
    struct timeval tstart, tend;
    char **keys = malloc(n_keys * sizeof(char *));
    init_keys(keys, n_keys, key_len);

    // insert keys
    char value[val_len];
    generate_string(value, val_len);
    for (int i = 0; i < n_keys; i++) {
        fprintf(stderr,"Inserting key %s with value %s\n", keys[i], value);
        memcached_return rc = memcached_add(memc, keys[i], key_len, value, val_len, 0, 0);
        if (rc != MEMCACHED_SUCCESS)
            fprintf(stderr,"Error inserting key %s: %s\n", keys[i], memcached_strerror(memc, rc));
    }

    // main loop
    gettimeofday(&tstart, NULL);
    for (int i = 0; i < n_ops; i++) {
        char *key = keys[i % n_keys];

        // get key
        fprintf(stderr,"Getting key %s\n", key);
        size_t value_length;
        uint32_t flags;
        memcached_return rc;
        char *v = memcached_get(memc, key, key_len, &value_length, &flags, &rc);
        if (v == NULL) {
            fprintf(stderr,"memcached_get() failed: %s, key: %s\n", memcached_strerror(memc, rc), key);
            continue;
        }
        fprintf(stderr,"Got %s -> %s\n", key, v);

    }
    gettimeofday(&tend, NULL);


    free_keys(keys, n_keys);
    free(keys);
    memcached_free(memc);

    // print results

    double duration = time_diff(&tstart, &tend) / 1000; // in ms
    double throughput = 0;
    if (duration > 0) throughput = n_ops / duration;
    if (verbose) {
        printf("Config: %s\n", config_string);
        printf("Number of keys: %d\n", n_keys);
        printf("Key length: %d\n", key_len);
        printf("Value length: %d\n", val_len);
        printf("Number of operations: %d\n\n", n_ops);
        printf("Duration: %f ms\n", duration / n_ops);
        printf("Throughput: %f ops/ms\n", throughput);
    } else {
        printf("%f,%f\n", duration, throughput);
    }

    return 0;
}
