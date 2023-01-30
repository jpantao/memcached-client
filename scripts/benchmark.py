#!/bin/python3

import subprocess
import argparse
import shlex
import csv
import os
from time import sleep

PERF_EVENTS = [
    "cache-misses",
    "L1-dcache-loads",
    "L1-dcache-load-misses",
    "LLC-loads",
    "LLC-load-misses",
    "LLC-stores",
    "LLC-store-misses",
    "sw_prefetch_access.nta",
]

MEMCACHED_PORT = 11211


def debug():
    print(','.join(PERF_EVENTS))
    exit(0)


def launch_memcached(memcached_exec):
    command = f"perf stat -e {','.join(PERF_EVENTS)} {memcached_exec} -p {MEMCACHED_PORT}"
    print(command)
    p = subprocess.Popen(shlex.split(command), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    print('Memcached launched')
    return p


if __name__ == '__main__':
    # debug()
    parser = argparse.ArgumentParser(description='Launch memcached client')
    parser.add_argument('test_name', action='store', help='Name of the test')
    parser.add_argument('memcached_exec', action='store', help='Path to memcached executable')
    parser.add_argument('--n-runs', '-n', dest='n_runs', default=1, help='Number of runs (default=1)')
    parser.add_argument('--build-dir', '-b', dest='build_dir', default='build',
                        help='CMake build directory (default=build)')
    args = parser.parse_args()
    os.makedirs('logs', exist_ok=True)

    test_name = args.test_name
    build_dir = args.build_dir
    n_runs = args.n_runs

    # Prepare csv file
    file = open(f'logs/{test_name}.csv', 'w')
    fieldnames = ['run', 'n_ops', 'n_keys', 'key_size', 'value_size', 'duration', 'throughput', *PERF_EVENTS]
    writer = csv.DictWriter(file, fieldnames=fieldnames)
    writer.writeheader()

    # clean and build
    subprocess.run(shlex.split('make clean --directory build'), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    subprocess.run(shlex.split(f'cmake -B build'))
    subprocess.run(shlex.split('make --directory build'))
    print('Build done')

    n_ops = [1]
    n_keys = [1]
    key_size = [16]
    value_size = [64]

    for r in range(1, n_runs + 1):
        for n_op in n_ops:
            for n_key in n_keys:
                for k_size in key_size:
                    for v_size in value_size:
                        print(f'---- run={r} n_op={n_op}, n_key={n_key}, k_size={k_size}, v_size={v_size} ----')
                        memcached_process = launch_memcached(args.memcached_exec)
                        sleep(10)
                        cmd = f'./{build_dir}/memcached_client -no {n_op} -nk {n_key} -kl {k_size} -vl {v_size}'
                        subprocess.run(shlex.split(cmd))
                        sleep(10)
                        print('memcached perf stat:')
                        subprocess.run(shlex.split('killall memcached'))
                        print(memcached_process.communicate()[1].decode('utf-8'))
                        memcached_process.terminate()
                        print(f'-----------------------')
