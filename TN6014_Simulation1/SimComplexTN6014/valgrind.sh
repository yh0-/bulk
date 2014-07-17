#!/bin/sh
# valgrind --leak-check=full --show-reachable=yes -v 2> valgrind2.log > valgrind1.log ./SimComplex

if command -v valgrind 2> /dev/null > /dev/null && [ -x ./SimComplex ]
then
    valgrind --leak-check=full --show-reachable=yes -v ./SimComplex -n
    valgrind --leak-check=full --show-reachable=yes -v ./SimComplex
fi
