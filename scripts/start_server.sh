#!/bin/bash

GMS_SERVER_BIN=${HOME%%/}/gms/build/gms_server

if [[ ! -z "$GMS_HOME" ]]; then
    GMS_SERVER_BIN=${GMS_HOME%%/}/build/gms_server
fi
echo $GMS_SERVER_BIN

if [[ "$1" == "profile" ]]; then
    echo "Launching Gms Server with nsys profile..."
    # nsys profile -o my_profile_report --delay 5 --duration 10 $GMS_SERVER_BIN
    nsys profile --capture-range cudaProfilerApi --trace cuda,osrt,nvtx --stats=true --force-overwrite true $GMS_SERVER_BIN
else
    echo "Launching Gms Server directly (without nsys profile)..."
    $GMS_SERVER_BIN
fi