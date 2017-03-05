#!/bin/bash
cd ./../../Core/src/
./prepareExecutable.sh
cd -
cp ../../Core/src/CProcess ../src/
node ../index.js