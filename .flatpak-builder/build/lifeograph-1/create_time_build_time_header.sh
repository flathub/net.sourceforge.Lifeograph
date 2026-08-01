#!/bin/sh

TIMESTAMP_REQUIRED=false
HEADER_FILE="$3/build_time.h"

if [ -f "$1" ] && [ -f "$HEADER_FILE" ]
then
    bin_mdf_date=$(stat -c %Y "$1")

    for filename in "$2"; do
        src_mdf_date=$(stat -c %Y "$filename")
        if [[ "$bin_mdf_date" < "$src_mdf_date" ]]; then
            TIMESTAMP_REQUIRED=true
            break
        fi
    done
else
    TIMESTAMP_REQUIRED=true
fi

if [ $TIMESTAMP_REQUIRED = true ]; then
    BUILD_TIMESTAMP=$(date "+%Y-%m-%d  %H:%M:%S")
    BUILD_TIMESTAMP="#define BUILD_TIMESTAMP \"${BUILD_TIMESTAMP}\""
    echo $BUILD_TIMESTAMP > "$HEADER_FILE"
fi
