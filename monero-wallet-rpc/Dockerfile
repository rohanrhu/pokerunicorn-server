FROM debian:trixie

EXPOSE 18082

COPY monero-wallet-rpc/get-monero.sh .

# Add retry + minimize recommended packages for faster/more reliable builds
RUN set -eux; \
    echo 'Acquire::Retries "5";' > /etc/apt/apt.conf.d/80-retries; \
    apt-get update; \
    apt-get install -y --no-install-recommends \
        ca-certificates \
        wget \
        bzip2 \
        build-essential \
        cmake \
        git \
        libboost-all-dev \
        miniupnpc \
        libunbound-dev \
        graphviz \
        doxygen \
        libunwind8-dev \
        pkg-config \
        libssl-dev \
        libzmq3-dev \
        libpgm-dev \
        libhidapi-dev \
        libprotobuf-dev \
        protobuf-compiler \
        libqrencode-dev \
        libsodium-dev; \
    rm -rf /var/lib/apt/lists/*

RUN chmod +x ./get-monero.sh && ./get-monero.sh

RUN mkdir -p /root/host/wallets/main

COPY monero-wallet-rpc/start-wallet-rpc.sh /start-wallet-rpc.sh
RUN chmod +x /start-wallet-rpc.sh

CMD ["bash", "/start-wallet-rpc.sh"]