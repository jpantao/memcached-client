#!/bin/bash

node=$(uniq "$OAR_NODE_FILE" | head -n 1)

kadeploy3 --no-kexec -a "$HOME"/public/kmem_dax_env.dsc -f "$OAR_NODE_FILE" -k
ssh root@"$node" 'apt update -y'
#ssh root@"$node" 'yes | apt dist-upgrade -y'
ssh root@"$node" 'apt install -y cmake linux-perf libc-bin libmemcached11'
ssh root@"$node" 'echo "jantao  ALL=(ALL:ALL) NOPASSWD: ALL" >> /etc/sudoers'
ssh root@"$node" 'ndctl create-namespace --mode=devdax --map=mem'
ssh root@"$node" 'daxctl reconfigure-device dax1.0 --mode=system-ram'
ssh root@"$node" 'sysctl -w kernel.perf_event_paranoid=1'

echo "Deployed on $node"