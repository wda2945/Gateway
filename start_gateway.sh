#!/bin/bash
# Gateway Service

cd /root

while true
do
  /root/stable/gateway
  EXITCODE=$?

  if [ $EXITCODE -eq 143 ]
  then
# SIGTERM
    exit 0
  fi

  if [ $EXITCODE -eq 137 ]
  then
# SIGKILL
    exit 0
  fi

done
