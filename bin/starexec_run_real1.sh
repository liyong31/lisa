#!/bin/bash

# run two processes of tlsf-sdf in parallel:
# - one for the original spec
# - one for the dualized spec


# all agruments are passed through to tlsf-sdf-opt

# needed because we rely on syfco which is located ./)
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
export PATH=$PATH:$SCRIPT_DIR
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SCRIPT_DIR/mona-install-prefix/lib

fileR=$(mktemp)

function check_and_exit() {
    pR_dead=0

    if ! kill -0 $pR > /dev/null 2>&1; then
        pR_dead=1
    fi

    if (($pR_dead == 1)); then
        #echo "pR dead"
        status=$(head -n1 $fileR)
        if [ "$status" = "REALIZABLE" ]; then
            cat $fileR
            kill -9 $pU > /dev/null 2>&1
            exit 10
        fi
        if [ "$status" = "UNREALIZABLE" ]; then
            cat $fileR
            kill -9 $pU > /dev/null 2>&1
            exit 20
        fi
        echo UNKNOWN
        exit 30
    fi
}

#cmdR="sleep 1; echo real-process; echo UNKNOWN"
#cmdU="sleep 0.1; echo unreal-process; echo UNKNOWN"

cmdR="./lisa-syntcomp-opt $@ "

(eval "$cmdR") > $fileR &  # start synthesizer for the original spec
pR=$!

# busy wait (bcz StarExec's bash is outdated and does not have `wait -n`)
while :
do
  check_and_exit
done

