#include <climits>
#include <cstdlib>
#include "gtest/gtest.h"
#include "Configuration.hpp"
#include "Tools.hpp"

#include <cstdlib>
#include <signal.h>
#include <vector>

#include "NodeJSInterface.hpp"

class NodeJSInterfaceTest : public ::testing::Test {
 protected:
  virtual void SetUp() { 
  	
  }
  virtual void TearDown() { }
};

TEST_F(NodeJSInterfaceTest,NodeJSInterfaceTest_socket){
	std::string lv_read;
	int lv_childPid;
	NodeJSInterface nodeJSInterface;
	lv_childPid = fork();

	if(lv_childPid){
		nodeJSInterface.listen();
		nodeJSInterface.read(lv_read);
		EXPECT_EQ(lv_read,"lol");
		kill(lv_childPid,SIGKILL);
		nodeJSInterface.closeClient();
		nodeJSInterface.closeServer();
	}
	else{
		system("node NodeJSInterface_test.js 1");
	}
}
TEST_F(NodeJSInterfaceTest,NodeJSInterfaceTest_echo){
	std::string lv_write;
	int lv_childPid;
	usleep(1000000);
	NodeJSInterface nodeJSInterface;
	lv_childPid = fork();

	if(lv_childPid){
		nodeJSInterface.listen();
		nodeJSInterface.read(lv_write);
		nodeJSInterface.write(lv_write);
		nodeJSInterface.read(lv_write);
		EXPECT_EQ(lv_write,"hello");
		kill(lv_childPid,SIGKILL);
		nodeJSInterface.closeClient();
		nodeJSInterface.closeServer();
	}
	else{
		system("node NodeJSInterface_test.js 2");
	}
}