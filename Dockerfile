FROM debian:trixie

EXPOSE 5550-5560

ENV PKRSRV_GDBFRONTEND_BIND_ADDRESS=0.0.0.0

COPY . /root/PokerServer

RUN set -eux; \
    echo 'Acquire::Retries "5";' > /etc/apt/apt.conf.d/80-retries; \
    apt-get update; \
    apt-get install -y --no-install-recommends \
        debian-archive-keyring \
        ca-certificates \
        git \
        gcc \
        build-essential \
        libpq-dev \
        libssl-dev \
        uuid-dev \
        zlib1g-dev \
        libreadline-dev \
        libgmp-dev \
        libcurl4 \
        libcurl4-openssl-dev \
        make \
        valgrind \
        gdb \
        tmux \
        procps \
        python3 \
        htop; \
    rm -rf /var/lib/apt/lists/*

WORKDIR /root
RUN set -eux; \
    git clone https://github.com/rohanrhu/gdb-frontend.git; \
    echo "set auto-load safe-path /" > /root/.gdbinit

WORKDIR /root/PokerServer

COPY docker-entrypoint.sh /usr/local/bin/
RUN chmod +x /usr/local/bin/docker-entrypoint.sh

ENTRYPOINT ["docker-entrypoint.sh"]
CMD []