#!/bin/python3

import subprocess
import argparse
import signal
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
DRAM_NUMA = 0
PMEM_NUMA = 3


def is_event(string):
    for event in PERF_EVENTS:
        if event in string:
            return True
    return False


def is_counted(line):
    return '<not counted>' not in line


def extract_perf_results(perf_out):
    lines = perf_out.split('\n')[2:]
    lines = filter(is_event, lines)
    lines = filter(is_counted, lines)
    table = [cols.strip().split() for cols in lines]
    return [val[0].strip().replace(',', '') for val in table]


def extract_sec_time_elapsed(perf_out):
    for line in perf_out.split('\n'):
        if 'seconds time elapsed' in line:
            return line.split()[0]
    return


def launch_memcached(memcached_exec, numa_node):
    command = f"numactl --membind={numa_node} perf stat -e {','.join(PERF_EVENTS)} {memcached_exec} -p {MEMCACHED_PORT}"
    # print(command)
    p = subprocess.Popen(shlex.split(command), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return p


def kill_memcached():
    subprocess.run(shlex.split('killall memcached'), stdout=subprocess.PIPE, stderr=subprocess.PIPE)


def run_memcached_client():
    command = f"./{build_dir}/memcached_client -no {n_op} -nk {n_key} -kl {k_size} -vl {v_size}"
    print(command)
    p = subprocess.run(shlex.split(command), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return p


def print_dict(d):
    for k, v in d.items():
        print(f'{k}: {v}')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Launch memcached client')
    parser.add_argument('test_name', action='store', help='Name of the test')
    parser.add_argument('memcached_exec', action='store', help='Path to memcached executable')
    parser.add_argument('--n-runs', '-n', dest='n_runs', default=1, help='Number of runs (default=1)')
    parser.add_argument('--dram-only', '-d', dest='dram_only', action='store_true', default=False,
                        help='Only benchmark DRAM')
    parser.add_argument('--prefetch', '-p', dest='prefetch', action='store_true', default=False,
                        help='Compile memcached with prefetch activated')
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
    # writer.writeheader()

    # clean and build
    flags = 'CFLAGS="-DPREFETCH"' if args.prefetch else ""
    subprocess.run(shlex.split('make clean --directory build'), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    subprocess.run(shlex.split('cmake -B build'), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    subprocess.run(shlex.split(f'make --directory build {flags}'), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    # print('Build done')

    n_ops = [100_000]
    n_keys = [100_000]
    key_size = [16]
    value_size = [100]
    nodes = [DRAM_NUMA]
    if not args.dram_only:
        nodes.append(PMEM_NUMA)

    for r in range(1, n_runs + 1):
        for n_op in n_ops:
            for n_key in n_keys:
                for k_size in key_size:
                    for v_size in value_size:
                        for node in nodes:
                            print(f'- run={r} n_op={n_op}, n_key={n_key}, k_size={k_size}, v_size={v_size}, '
                                  f'node={node} -')

                            server_process = launch_memcached(args.memcached_exec, node)
                            client_process = run_memcached_client()
                            # print('Killing memcached')
                            sleep(10)
                            kill_memcached()

                            pref_out = server_process.communicate()[1].decode('utf-8').strip()
                            # print(pref_out)
                            client_out = client_process.stdout.decode('utf-8').strip().split(',')
                            # print(client_out)

                            row = {
                                'run': r,
                                'node': node,
                                'throughput': client_out[1],
                                'exec_time_ms': client_out[0],
                                'n_ops': n_op,
                                'n_keys': n_key,
                                'key_size': k_size,
                                'value_size': v_size,
                                **dict(zip(PERF_EVENTS, extract_perf_results(pref_out)))
                            }
                            # print_dict(row)
                            print(f'throughput: {row["throughput"]}')
                            # writer.writerow(row)
                            server_process.terminate()
                            print(f'-----------------------')
