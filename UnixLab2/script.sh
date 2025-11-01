#!/bin/bash

SHARED_DIR="$(pwd)/shared"
LOCK_FILE="$SHARED_DIR/.lock"
CONTAINER_ID=$(hostname)
COUNTER=1

mkdir -p "$SHARED_DIR"

find_next_available() {
    local i=1
    while [[ $i -le 99 ]]; do
        echo "$i"
        num=$(printf "%03d" "$i")
        if [ ! -e "$SHARED_DIR/$num" ]; then
            echo $num
            return 0
        else ((i++))
        fi
        
    done
}


until [[ $COUNTER  -eq 99 ]]; do
    (
    flock -x 200
        filename=$(find_next_available)
        echo "$CONTAINER_ID $filename" > "$SHARED_DIR/$filename"
        echo "Created: $filename"
    ) 200>"$LOCK_FILE"


    sleep 1

    if [[ -f "$SHARED_DIR/$filename" ]]; then
        rm -f "$SHARED_DIR/$filename"
        echo "Deleted: $filename"
    fi
    
    ((COUNTER++))

done



files=("$SHARED_DIR"/*)
for i in "${files[@]}"; do
    echo $i
    rm -rf "$i"
done