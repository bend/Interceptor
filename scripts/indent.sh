#!/bin/bash
COMMAND="astyle --style=stroustrup --pad-oper --indent-col1-comments --indent-cases --indent-col1-comments --pad-header --align-pointer=type --indent=spaces=2 --indent-switches --indent-namespaces --break-blocks -j"
find $PWD/$1 -name "*.h" | xargs $COMMAND
find $PWD/$1 -name "*.cpp" | xargs $COMMAND
find $PWD/$1 -name "*.orig" | xargs rm -rf
