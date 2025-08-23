#!/bin/sh
set -e

# Ensure default if not provided
: "${PKRSRV_GDBFRONTEND_BIND_ADDRESS:=0.0.0.0}"

# If no arguments passed, supply default command
if [ "$#" -eq 0 ]; then
  set -- /root/gdb-frontend/gdbfrontend -w /root/PokerServer -l "${PKRSRV_GDBFRONTEND_BIND_ADDRESS}"
fi

exec "$@"
