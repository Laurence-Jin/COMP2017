#!/bin/bash

./procchat & 
SERVER_PID=$!
./test_client.out

sleep 10
kill -SIGKILL $SERVER_PID