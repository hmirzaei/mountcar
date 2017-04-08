#!/bin/bash

k=88;t=0.3; for i in {1..10}; do ./pwm.sh 0 $k && sleep $t && ./pwm.sh $k 0 && sleep $t && ./pwm.sh 0 $k && sleep $t && ./pwm.sh $k 0 && sleep $t; done && ./pwm.sh 0 0
