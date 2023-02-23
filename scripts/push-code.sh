#!/bin/bash

# usage:
# ./scripts/push-code.sh  <remote-host> <memcached-location>

remote_host=$1
memcached_location=$2



rsync -aPv -e "ssh -p 22" "./scripts" "$remote_host":memcached-client
rsync -aPv -e "ssh -p 22" "./scripts" "$remote_host":memcached-client
rsync -aPv -e "ssh -p 22" "./client.c" "$remote_host":memcached-client
rsync -aPv -e "ssh -p 22" "./client_debug.c" "$remote_host":memcached-client
rsync -aPv -e "ssh -p 22" "./CMakeLists.txt" "$remote_host":memcached-client

rsync -aPv -e "ssh -p 22" "$memcached_location" "$remote_host":.
