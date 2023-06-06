#!/bin/bash
# This configuration allows to check (un)realizability only
# $1 contains the input filename (the name of the AIGER-file).

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
export PATH=$PATH:$SCRIPT_DIR
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SCRIPT_DIR/mona-install-prefix/lib

COMMAND="./lisa-syntcomp-opt $1 -i 800 -p 300000"
$COMMAND
res=$?
if [[ $res == 10 ]]; then
    echo "REALIZABLE"
elif [[ $res == 20 ]]; then
    echo "UNREALIZABLE"
else
    echo "UNKNOWN"
fi
exit $res
