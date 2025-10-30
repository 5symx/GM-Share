#!/bin/bash

GMS_CLIENT_LIB=libgms_client_local.so

GMS_CLIENT_LIB_PATH=${HOME%%/}/gms/build/$GMS_CLIENT_LIB

if [[ ! -z "$GMS_HOME" ]]; then
    GMS_CLIENT_LIB_PATH=${GMS_HOME%%/}/build/$GMS_CLIENT_LIB
fi

#GMS_CLIENT_LIB_PATH=/home/gms/shim.so
LD_PRELOAD=$GMS_CLIENT_LIB_PATH $@
