#ifndef __NODEJSINTERFACE_HPP__
#define __NODEJSINTERFACE_HPP__
#include "Configuration.hpp"
#include "Tools.hpp"

#include <unistd.h>
#include <cstring>
#include <netinet/in.h>

#define MEMSIZE 256

enum NodeJSIntEnum 		{ NODEJSINT_OK, NODEJSINT_ERROR};

class NodeJSInterface{
private:
	int 				ov_portNum;
	struct sockaddr_in 	ov_serverSocketAddr;
	struct sockaddr_in  ov_clientSocketAddr;
	int 				ov_serverSocketFileDesc;
	int					ov_clientSocketFileDesc;
	char 				op_buffer[MEMSIZE];
public:
	NodeJSInterface();
	~NodeJSInterface(){};
	int listen();
	int read(std::string &av_readString);
	int write(const std::string &av_stringToWrite);
	int closeServer();
	int closeClient();
};
#endif