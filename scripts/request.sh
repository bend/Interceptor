#!/bin/bash - 
#===============================================================================
#
#          FILE: request.sh
# 
#         USAGE: ./request.sh 
# 
#   DESCRIPTION: 
# 
#       OPTIONS: ---
#  REQUIREMENTS: ---
#          BUGS: ---
#         NOTES: ---
#        AUTHOR: YOUR NAME (), 
#  ORGANIZATION: 
#       CREATED: 08/03/2017 15:32
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error
while true;do
  curl localhost:7003/index.html -O
  curl localhost:7000/index.html -O
  curl localhost:7004/index.html -O
done

