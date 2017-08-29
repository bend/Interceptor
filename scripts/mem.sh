#!/bin/bash - 
#===============================================================================
#
#          FILE: mem.sh
# 
#         USAGE: ./mem.sh 
# 
#   DESCRIPTION: 
# 
#       OPTIONS: ---
#  REQUIREMENTS: ---
#          BUGS: ---
#         NOTES: ---
#        AUTHOR: YOUR NAME (), 
#  ORGANIZATION: 
#       CREATED: 08/03/2017 12:36
#      REVISION:  ---
#===============================================================================

rm /tmp/mem.log

while true;do
  ps -C $1 -o pid=,%mem=,vsz= >> /tmp/mem.log
  sleep 1
done

