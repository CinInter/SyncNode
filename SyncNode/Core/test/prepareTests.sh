#!/bin/bash

PKG_OK=$(dpkg-query -W --showformat='${Status}\n' libasound2-dev|grep "install ok installed")
echo Checking for libasound2-dev: $PKG_OK
if [ "" == "$PKG_OK" ]; then
  echo "No somelib. Setting up somelib."
  sudo apt-get --force-yes --yes install libasound2-dev
fi

if [ "$OSTYPE" == "linux-gnueabihf" ] ; then
	g++ -c *.cpp ../src/*.cpp ../src/*.hpp -I../src -DCORETEST -I../include/gtestRpi -std=c++14 -O3 -mfpu=neon
	g++ -o test *.o -L../lib/gtestRpi -lgtest -lgtest_main -lpthread -lasound
	rm *.o ../src/*.gch
	echo "Compilation is done successfully"

elif [ "$OSTYPE" == "darwin16" ];then
	g++ -c *.cpp ../src/*.cpp ../src/*.hpp -I../src -DCORETEST -I../include/gtestMac -std=c++14 -O3
	g++ -o test *.o -L../lib/gtestMac -lgtest -lgtest_main -lpthread 
	rm *.o ../src/*.gch
	echo "Compilation is done successfully"
else
	echo "Platform not supported"
fi