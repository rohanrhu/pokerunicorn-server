#!/bin/bash

exec /monero/monero-wallet-rpc \
    "--${MONERO_NETWORK}" \
    --daemon-host "${MONERO_NODE_ADDRESS}" \
    --daemon-port "${MONERO_NODE_PORT}" \
    --rpc-bind-ip 0.0.0.0 \
    --rpc-bind-port 18082 \
    --wallet-dir /root/host/wallets/main \
    --disable-rpc-login \
    --non-interactive \
    --confirm-external-bind \
    --log-level 4