#!/bin/bash

set -e  # stop on error

# some colors for printing
GREEN='\033[1;32m'
NC='\033[0m' # No Color

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_DIR=$SCRIPT_DIR/syntcomp-build/
TP_DIR=$SCRIPT_DIR

export PATH=$PATH:$SCRIPT_DIR/bin

export LISA_TP=$SCRIPT_DIR/third_parties/

echo -e "${GREEN}building spot..${NC}"
cd $LISA_TP/spot-2.10.5/
./configure --prefix=$LISA_TP/spot-install-prefix/
make -j8
make install

echo -e "${GREEN}building MONA..${NC}"
cd $LISA_TP/MONA/
./configure --prefix=$LISA_TP/mona-install-prefix/
make
make install-strip

echo -e "${GREEN}building lisa-syntcomp..${NC}"
rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR

cmake $SCRIPT_DIR
make -j8 lisa-syntcomp-opt

cd $SCRIPT_DIR

rm -rf bin
mkdir bin
ln -s $BUILD_DIR/src/lisa-syntcomp-opt $SCRIPT_DIR/bin/lisa-syntcomp-opt
ln -s $LISA_TP/mona-install-prefix/bin/mona $SCRIPT_DIR/bin/mona
cp $SCRIPT_DIR/starexec_run_lisa.sh $SCRIPT_DIR/bin/starexec_run_lisa.sh

echo
echo -e "${GREEN}executable was created in bin/${NC}"

# remove intermediate compilation files of spot to save space
# (this _keeps_ the installed binaries and libs)
cd $LISA_TP/spot-2.10.5/
make clean

