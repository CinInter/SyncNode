#!/bin/bash

if [ "$OSTYPE" == "linux-gnueabihf" ] ; then
	PKG_OK=$(dpkg-query -W --showformat='${Status}\n' libasound2-dev|grep "install ok installed")
	echo Checking for libasound2-dev: $PKG_OK
	if [ "" == "$PKG_OK" ]; then
		echo "No somelib. Setting up somelib."
		sudo apt-get --force-yes --yes install libasound2-dev
	fi
	g++ -c 	CrossCorrelation.cpp 	\
			MicrophoneInterface.cpp	\
			NodeJSInterface.cpp		\
			Thread.cpp				\
			Core.cpp				\
			Configuration.hpp 		\
			CrossCorrelation.hpp 	\
			Log.hpp 				\
			MicrophoneInterface.hpp	\
			NodeJSInterface.hpp		\
			Tools.hpp 				\
			Thread.hpp				\
			-I. -std=c++14 -O3 -mfpu=neon
	g++ -o CProcess *.o -lpthread -lasound
	rm *.o *.gch
	echo "Compilation is done successfully"

elif [ "$OSTYPE" == "darwin16" ];then
	g++ -c 	CrossCorrelation_test.cpp		\
			Main_TestAll.cpp 				\
			NodeJSInterface_test.cpp 		\
			../src/CrossCorrelation.cpp 	\
			../src/NodeJSInterface.cpp		\
			../src/Thread.cpp				\
			../src/Configuration.hpp 		\
			../src/CrossCorrelation.hpp 	\
			../src/Log.hpp 					\
			../src/NodeJSInterface.hpp		\
			../src/Tools.hpp 				\
			../src/Thread.hpp				\
	-I../src -DCORETEST -I../include/gtestMac -std=c++14 -O3
	g++ -o test *.o -L../lib/gtestMac -lgtest -lgtest_main -lpthread 
	rm *.o ../src/*.gch
	echo "Compilation is done successfully"
else
	echo "Platform not supported"
fi