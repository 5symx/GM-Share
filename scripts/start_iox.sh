#!/bin/bash

IOX_ROUDI_BIN=${HOME%%/}/gms/build/iox-roudi

if [[ ! -z "$GMS_HOME" ]]; then
    IOX_ROUDI_BIN=${GMS_HOME%%/}/build/iox-roudi
fi

$IOX_ROUDI_BIN -l debug