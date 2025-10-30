#!/bin/bash

shopt -s expand_aliases
alias python3="python3.9"

GMS_CLIENT_LIB=libgms_client.so

GMS_CLIENT_LIB_PATH=${HOME%%/}/gms/build/$GMS_CLIENT_LIB

if [[ ! -z "$GMS_HOME" ]]; then
    GMS_CLIENT_LIB_PATH=${GMS_HOME%%/}/build/$GMS_CLIENT_LIB
fi

LD_PRELOAD=$GMS_CLIENT_LIB_PATH $@