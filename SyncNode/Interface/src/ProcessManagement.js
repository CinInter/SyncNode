var util  = require('util');
const fs = require('fs');
var spawn = require('child_process').spawn;
const out = fs.openSync('./out.log', 'a');
var sockNum;
var callbackFunction;


function resetCProcess(socketNum,callback) {
	var ps    = spawn('ps');
	var	grep  = spawn('grep', ["./CProcess"]);
	sockNum   = socketNum;

	callbackFunction=function(){
		callback();
	}
	ps.stdout.on('data', function (data) {
		grep.stdin.write(data);
	});

	ps.on('exit', function (code) {
		if (code !== 0) {
			util.log('NODE PROCESS - LOG INFO - resetCProcess : ps process exited with code ' + code);
		}
		grep.stdin.end();
	});

	grep.stdout.on('data', function (data) {
		var dataLength 	= data.length;
		var dataChar	= [];
		var dataString;
		var responseArray;
		var isCProcessExists=0;
		var cProcessId;

		for(index=0;index<dataLength;++index)
			dataChar[index]=String.fromCharCode(data[index]);
		dataString=dataChar.join("");

		responseArrayLines = dataString.split("\n");
		for(var responseArrayLine = 0; responseArrayLine < responseArrayLines.length-1; responseArrayLine++) {
			responseArray = responseArrayLines[responseArrayLine].trimLeft().split(" ");
			util.log("NODE PROCESS - LOG INFO - resetCProcess : Process num "+responseArrayLine+" id : "+responseArray[0]+" type of executable : "+responseArray[6]);
			if(responseArray[6]=="grep"){
				util.log("NODE PROCESS - LOG INFO - resetCProcess : Killing grep process with id : "+responseArray[0]);
				var	kill  = spawn('kill', ['-9',responseArray[0]]);
			}else{
				isCProcessExists=1;
				cProcessId=responseArray[0];
			}
		}

		responseArray = dataString.split(" ");
		if(isCProcessExists==0)
			launchProcess(sockNum,callback);
		else{
			killProcess(cProcessId);
		}
	});

	grep.on('exit', function (code) {
		if (code !== 0) {
			launchProcess(sockNum,callback);
		}
	});
}

function killProcess(processId,callback) {
	util.log("NODE PROCESS - LOG INFO - killProcess   : Killing ./CProcess with id : "+processId);
	var	kill  = spawn('kill', ['-9',processId]);

	kill.on('exit', function (code) {
		if (code !== 0) {
			util.log('NODE PROCESS - LOG INFO - resetCProcess : Kill process exited with code ' + code);
			launchProcess(sockNum,callback);
		}
		else{
			util.log('NODE PROCESS - LOG INFO - resetCProcess : Killing of ./CProcess is finished');
			launchProcess(sockNum,callback);
		}
	});
}

function launchProcess(socketNum,callback){
	util.log("NODE PROCESS - LOG INFO - launchProcess : launch CProcess with socketNum = "+socketNum);
	var	CProcess  = spawn('./CProcess', [socketNum]);
	CProcess.stdout.pipe(process.stdout);
	showProcesses(callback);
}

function showProcesses(callback) {
	var ps    = spawn('ps');
	var	grep  = spawn('grep', ["./CProcess"]);
	
	ps.stdout.on('data', function (data) {
		callbackFunction();
	});
}
module.exports={
	resetCProcess
};