#!/bin/bash

SHARED_DIR="$(pwd)/shared"
LOCK_FILE="$SHARED_DIR/.lock"
CONTAINER_ID=$(hostname)

mkdir -p "$SHARED_DIR"

while true; do

    temp_filename=$(mktemp)
    echo "$temp_filename"
    (
    flock -x 200
        for i in $(seq -f "%03g" 1 100); do
            if [[ ! -f "$SHARED_DIR/$i" ]]; then
                echo "$i" > "$temp_filename"
                echo "$CONTAINER_ID $i" > "$SHARED_DIR/$i"
                cat "$SHARED_DIR/$i"
                break
            fi
        done
    ) 200>"$LOCK_FILE"

    filename=$(cat "$temp_filename")

    sleep 2

    if [[ -f "$SHARED_DIR/$filename" ]]; then
        rm -f "$SHARED_DIR/$filename"
        echo "Deleted: $filename"
    fi

    sleep 2
    
done