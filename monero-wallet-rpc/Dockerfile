FROM debian:bullseye

EXPOSE 18082

COPY get-monero.sh .

RUN apt-get update && apt-get install -y \
    wget bzip2 build-essential cmake git libboost-all-dev miniupnpc libunbound-dev graphviz doxygen libunwind8-dev pkg-config libssl-dev libzmq3-dev libpgm-dev libhidapi-dev libprotobuf-dev protobuf-compiler libqrencode-dev libsodium-dev \
    && rm -rf /var/lib/apt/lists/*

RUN chmod +x ./get-monero.sh
RUN ./get-monero.sh
    
RUN mkdir -p /root/host/wallets/main

CMD /monero/monero-wallet-rpc \
    "--$MONERO_NETWORK" \
    --daemon-host "$MONERO_NODE_ADDRESS" \
    --daemon-port "$MONERO_NODE_PORT" \
    --rpc-bind-ip 0.0.0.0 \
    --rpc-bind-port 18082 \
    --wallet-dir /root/host/wallets/main \
    --disable-rpc-login \
    --non-interactive \
    --confirm-external-bind \
    --log-level 4