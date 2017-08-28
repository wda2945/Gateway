#!/bin/bash

cp -Rv /Users/martin/Dropbox/RoboticsCodebase/Fido/BBB/modules /Users/martin/Downloads/Users/martin/Dropbox/RoboticsCodebase/Fido/BBB/modules
cp -Rv /Users/martin/Dropbox/RoboticsCodebase/Fido/BBB/platforms /Users/martin/Downloads/Users/martin/Dropbox/RoboticsCodebase/Fido/BBB/platforms
cp -Rv /Users/martin/Dropbox/RoboticsCodebase/Fido/BBB/robots /Users/martin/Downloads/Users/martin/Dropbox/RoboticsCodebase/Fido/BBB/robots
cp -Rv /Users/martin/Dropbox/RoboticsCodebase/Fido/shared_code /Users/martin/Downloads/Users/martin/Dropbox/RoboticsCodebase/Fido/shared_code
cp -Rv /Users/martin/Dropbox/EclipseBBB/fido/Debug /Users/martin/Downloads/Users/martin/Dropbox/EclipseBBB/fido/Debug

scp -r /Users/martin/Downloads/Users root@192.168.2.5:/Users
