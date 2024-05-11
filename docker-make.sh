#!/bin/bash

command="docker exec -it "
# command+="-e PKRSRV_BUILD_VERBOSE=1 "
# command+="-e PKRSRV_BUILD_REDIS_HOST=redis "
# command+="-e PKRSRV_BUILD_REDIS_PORT=6379 "
# command+="-e PKRSRV_BUILD_POSTGRES_HOST=postgres "
# command+="-e PKRSRV_BUILD_POSTGRES_PORT=5432 "
# command+="-e PKRSRV_BUILD_POSTGRES_USERNAME=meowingcat "
# command+="-e PKRSRV_BUILD_POSTGRES_PASSWORD=meow "
# command+="-e PKRSRV_BUILD_POSTGRES_DB=poker "
command+="pkrsrv-gdbfrontend "
command+="make -C /root/PokerServer && sleep 1 && "
command+="curl http://127.0.0.1:5550/api/runtime/run"

echo $command

eval $command