#include <limits.h>
#include "gtest/gtest.h"
#include "Tools.hpp"

int main(int argc, char **argv) {
	/*FILELog::ReportingLevel() = FILELog::FromString("DEBUG4");
  	FILE* log_fd = fopen( "NodeJSInterface_test_logfile.txt", "w" );
  	Output2FILE::Stream() = log_fd;*/
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
