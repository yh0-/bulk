#!/bin/sh
if [ -f "$1" ]
then
    if command -v unix2dos 2> /dev/null > /dev/null
    then
        unix2dos "$1"
    else
        sed 's/$'"/`echo \\\r`/" -i "$1"
    fi
fi

