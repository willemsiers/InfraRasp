#!/bin/bash
## WPI 1 = IR led driver
## WPI 0 = IR led receiver

gpio mode 1 OUT
gpio mode 2 IN

while true; do
echo setting high, reading:
gpio write 1 1;
gpio read 2;

sleep 0.5

echo seting low, receiving:
gpio write 1 0;
gpio read 2;

sleep 0.5

done;
