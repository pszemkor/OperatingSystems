#!/bin/bash
rm fifo

./master fifo &

sleep 1

./slave fifo 2 &
./slave fifo 4 &
./slave fifo 3 &


wait