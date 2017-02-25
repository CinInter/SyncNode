var net 			= require('net');
var client 			= new net.Socket();
var cProcessSocket	= 51712;

function functionLaunchedInOneSecond1(){
	client.connect(cProcessSocket, '127.0.0.1', function() {
		client.write("lol");
	});
};
function functionLaunchedInOneSecond2(){
	client.connect(cProcessSocket, '127.0.0.1', function() {
		client.write("hello");
		client.on('data', function(data) {
			client.write(data);
		});
	});
};
if(process.argv[2] == 1)
	setTimeout(functionLaunchedInOneSecond1,1000);
else
	setTimeout(functionLaunchedInOneSecond2,1000);

