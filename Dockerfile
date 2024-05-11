FROM debian:bullseye

EXPOSE 5550-5560

ARG GDBFRONTEND_BIND_ADDRESS=0.0.0.0

COPY . /root/PokerServer

RUN printf "deb http://httpredir.debian.org/debian bullseye-backports main non-free\ndeb-src http://httpredir.debian.org/debian bullseye-backports main non-free" > /etc/apt/sources.list.d/backports.list

RUN apt update -y && \
    apt upgrade -y && \
    apt install -y git \
                   gcc \
                   build-essential \
                   libpq-dev \
                   openssl \
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
                   htop \
    && rm -rf /var/lib/apt/lists/*


WORKDIR /root
RUN git clone https://github.com/rohanrhu/gdb-frontend.git
RUN echo "set auto-load safe-path /" > /root/.gdbinit

WORKDIR /root/PokerServer

CMD /root/gdb-frontend/gdbfrontend -w /root/PokerServer -l "$GDBFRONTEND_BIND_ADDRESS"