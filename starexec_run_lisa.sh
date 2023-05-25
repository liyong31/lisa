#!/bin/bash

# run two processes of tlsf-sdf in parallel:
# - one for the original spec
# - one for the dualized spec


# all agruments are passed through to tlsf-sdf-opt

# needed because we rely on syfco which is located ./)
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
export PATH=$PATH:$SCRIPT_DIR

fileR=$(mktemp)
fileU=$(mktemp)

function check_and_exit() {
    pR_dead=0
    pU_dead=0

    if ! kill -0 $pR > /dev/null 2>&1; then
        pR_dead=1
    fi

    if ! kill -0 $pU > /dev/null 2>&1; then
        pU_dead=1
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
    fi
    
    if (($pU_dead == 1)); then
        #echo "pR dead"
        status=$(head -n1 $fileU)
        if [ "$status" = "REALIZABLE" ]; then
            cat $fileU
            kill -9 $pR > /dev/null 2>&1
            exit 10
        fi
        if [ "$status" = "UNREALIZABLE" ]; then
            cat $fileU
            kill -9 $pR > /dev/null 2>&1
            exit 20
        fi
    fi

#    if (($pU_dead == 1)); then
        #echo "pU dead"
#        status=$(head -n1 $fileU)
#        if [ "$status" = "UNREALIZABLE" ]; then
#            cat $fileU
#            kill -9 $pR > /dev/null 2>&1
#            exit 20
#        fi
#    fi

    if (($pR_dead == 1)) && (($pU_dead == 1)); then
        # reaching this point means both statuses are UNKNOWN
        echo UNKNOWN
        exit 30
    fi
}

#cmdR="sleep 1; echo real-process; echo UNKNOWN"
#cmdU="sleep 0.1; echo unreal-process; echo UNKNOWN"

cmdR="./lisa-syntcomp-opt $@ "
cmdU="./lisa-syntcomp-opt $@ -i 800 -p 300000"

(eval "$cmdR") > $fileR &  # start synthesizer for the original spec
pR=$!

(eval "$cmdU") > $fileU &  # start synthesizer for the dualized spec
pU=$!

# busy wait (bcz StarExec's bash is outdated and does not have `wait -n`)
while :
do
  check_and_exit
  sleep 1
done

