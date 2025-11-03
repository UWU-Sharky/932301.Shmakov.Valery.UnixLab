#!/bin/bash

SHARED_DIR="$(pwd)/shared"
LOCK_FILE="$SHARED_DIR/.lock"
CONTAINER_ID=$(hostname)

mkdir -p "$SHARED_DIR"

while true; do

    filename=""

    (
    flock -x 200
        for i in $(seq -f "%03g" 1 100); do
            if [[ ! -f "$SHARED_DIR/$i" ]]; then
                filename="$i"
                echo "$CONTAINER_ID $filename" > "$SHARED_DIR/$filename"
                cat "$SHARED_DIR/$filename"
                break
            fi
        done
    ) 200>"$LOCK_FILE"


    sleep 2

    if [[ -f "$SHARED_DIR/$filename" ]]; then
       (
        flock -x 200
        rm -f "$SHARED_DIR/$filename"
        echo "Deleted: $filename"
        ) 200>"$LOCK_FILE"
    fi

    sleep 2
    
done