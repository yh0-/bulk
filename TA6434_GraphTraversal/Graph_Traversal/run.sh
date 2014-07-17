#!/bin/sh
if [ -e main.jar ]
then
    exec java -jar main.jar 2> /dev/null > /dev/null &
else
    echo "Run make"
fi

