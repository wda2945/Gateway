#!/bin/bash
# script to copy log files from BBB

today=`date '+%Y_%m_%d__%H_%M_%S'`;

mkdir /Users/martin/Dropbox/RoboticsCodebase/logfiles/gateway/$today

scp -r root@10.204.244.129:/root/logfiles /Users/martin/Dropbox/RoboticsCodebase/logfiles/gateway/$today

