#!/bin/bash - 
#===============================================================================
#
#          FILE: plot.sh
# 
#         USAGE: ./plot.sh 
# 
#   DESCRIPTION: 
# 
#       OPTIONS: ---
#  REQUIREMENTS: ---
#          BUGS: ---
#         NOTES: ---
#        AUTHOR: YOUR NAME (), 
#  ORGANIZATION: 
#       CREATED: 08/03/2017 12:50
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error
gnuplot plot.script

