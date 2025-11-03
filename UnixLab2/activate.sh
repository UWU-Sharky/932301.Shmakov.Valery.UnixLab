#!/bin/bash

for i in $(seq 1 $2); do
    docker run -d --rm --name "dockercontainer$i" -v $(pwd)/shared:/shared $1
done

