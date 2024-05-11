#!/bin/bash

ARCH=`uname -m`

if [ "$ARCH" == "x86_64" ]; then
    echo "Installing Monero (x86_64)"
    wget https://downloads.getmonero.org/cli/monero-linux-x64-v0.18.3.3.tar.bz2 \
    && tar -xvf monero-linux-x64-v0.18.3.3.tar.bz2 \
    && mv monero-x86_64-linux-gnu-v0.18.3.3 /monero
else
    echo "Installing Monero (AARCH64)"
    wget https://downloads.getmonero.org/cli/monero-linux-armv8-v0.18.3.3.tar.bz2 \
    && tar -xvf monero-linux-armv8-v0.18.3.3.tar.bz2 \
    && mv monero-aarch64-linux-gnu-v0.18.3.3 /monero
fi

PWD=`pwd`
cd /monero
chmod +x monero-blockchain-export monero-blockchain-stats monero-wallet-rpc monero-blockchain-import monero-blockchain-usage monerod monero-blockchain-mark-spent-outputs monero-gen-ssl-cert monero-blockchain-ancestry monero-blockchain-prune monero-gen-trusted-multisig monero-blockchain-depth monero-blockchain-prune-known-spent-data monero-wallet-cli
cd $PWD
