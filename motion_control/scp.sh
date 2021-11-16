#!/usr/bin/expect -f
set timeout 30
spawn scp /home/liu/motion_control/build/motion_control   nvidia@192.168.1.110:/home/nvidia/Templates/ly
expect "*password:"
send "Robot2020\r"
expect eof
#Robot@2020
