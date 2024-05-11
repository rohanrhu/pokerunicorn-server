#!/bin/bash

SCRIPT_FILE="$(realpath $0)"
SCRIPT_DIR="$(dirname "$SCRIPT_FILE")"
APP_PATH="$(realpath "$SCRIPT_DIR/..")"
DUMP_PATH="$APP_PATH/host/postgres/dummy_dump.sql"

PG_HOST="127.0.0.1"
PG_PORT="5432"
PG_USER="meowingcat"
PG_PASS="meow"
PG_DB="poker"

pg_dump --dbname="postgresql://$PG_USER:$PG_PASS@$PG_HOST:$PG_PORT/$PG_DB" -s -C > "$DUMP_PATH"
pg_dump --dbname="postgresql://$PG_USER:$PG_PASS@$PG_HOST:$PG_PORT/$PG_DB" -a -t tables >> "$DUMP_PATH"