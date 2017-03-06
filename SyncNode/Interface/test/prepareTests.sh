#!/bin/bash
rm ../src/CProcess
rm -rf ../src/audioFile/
cd ./../../Core/src/
./prepareExecutable.sh
cd -
cp ../../Core/src/CProcess ../src/
mkdir ../src/audioFile/
cd ../src/
node index.js