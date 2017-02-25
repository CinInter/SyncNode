#include "NodeJSInterface.hpp"

NodeJSInterface::NodeJSInterface(){
	int lv_option = 1;

	ov_serverSocketFileDesc = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(ov_serverSocketFileDesc, SOL_SOCKET, SO_REUSEADDR, &lv_option, sizeof(lv_option));

	if (ov_serverSocketFileDesc < 0){
		LOG_ERROR << "Error on opening socket";
	}

	ov_portNum 	= SYNCNODE_CPP_PROCESS_PORT_NUM;

	memset((char *) &ov_serverSocketAddr,0,sizeof(ov_serverSocketAddr));
	ov_serverSocketAddr.sin_family = AF_INET;
	ov_serverSocketAddr.sin_addr.s_addr = INADDR_ANY;
	ov_serverSocketAddr.sin_port = htons(ov_portNum);

	if (bind(ov_serverSocketFileDesc, (struct sockaddr *) &ov_serverSocketAddr,sizeof(ov_serverSocketAddr)) < 0){
		LOG_ERROR << "Error on binding";
	}
}

int NodeJSInterface::listen(){
	socklen_t ov_cliLen;

	::listen(ov_serverSocketFileDesc,1);
	ov_cliLen = sizeof(ov_clientSocketAddr);
	ov_clientSocketFileDesc = accept(ov_serverSocketFileDesc, (struct sockaddr *) &ov_clientSocketAddr, &ov_cliLen);

	if (ov_clientSocketFileDesc < 0){
		LOG_ERROR << "Error on accept";
		return NODEJSINT_ERROR;
	}
	return NODEJSINT_OK;	
}

int NodeJSInterface::read(std::string &av_readString){
	int lv_readMsgLen;
	memset(op_buffer,0,MEMSIZE);
	lv_readMsgLen = ::read(ov_clientSocketFileDesc,op_buffer,255);
	if (lv_readMsgLen < 0){
		LOG_ERROR << "Error in reading from socket";
		return NODEJSINT_ERROR;
	}
	av_readString.assign(op_buffer, lv_readMsgLen);
	return NODEJSINT_OK;	
}

int NodeJSInterface::write(const std::string &av_stringToWrite){
	int lv_writeMsgLen;
	lv_writeMsgLen = ::write(ov_clientSocketFileDesc, av_stringToWrite.c_str(), av_stringToWrite.length());
	if (lv_writeMsgLen < 0){
		LOG_ERROR << "Error in writing message on socket";
		return NODEJSINT_ERROR;
	}
	return NODEJSINT_OK;
}

int NodeJSInterface::closeServer(){
	::close(ov_serverSocketFileDesc);
	return NODEJSINT_OK;
}

int NodeJSInterface::closeClient(){
	::close(ov_clientSocketFileDesc);
	return NODEJSINT_OK;
}