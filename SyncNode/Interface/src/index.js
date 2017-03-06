var net 				= require('net');
var bodyParser  		= require('../node_modules/body-parser/index.js');
var express     		= require('../node_modules/express/index.js');
var formidable 			= require('../node_modules/formidable/index.js');
var path 				= require('path');
var fs 					= require('fs');
var util  				= require('util');
var ProcessManagement 	= require('./ProcessManagement.js');
var cProcessSocket		= 51712;

var app = express();
app.set('port', (process.env.PORT || 5000));
app.use(bodyParser.json());
app.disable('etag');
app.listen(app.get('port'), function() {
	console.log('Node app is running on port', app.get('port'));
});

/*
	CProcess management	
*/

app.post('/loadFile', function (req, res) {
	var form 		= new formidable.IncomingForm();
	var client 		= new net.Socket();

	form.multiples 	= true;
	form.uploadDir = path.join(__dirname, '/audioFile');
	form.parse(req);

	util.log("NODE PROCESS - LOG INFO - loadFile	  : -------------------------");

	form.on('field', function(name, field) {
		field_in_json_format 	= JSON.parse(field);
		var command				= field_in_json_format.command;

		form.on('file', function(field, file) {
			fs.rename(file.path, path.join(form.uploadDir, file.name.split("/")[1]));

			ProcessManagement.resetCProcess(cProcessSocket,function(){

				client.connect(cProcessSocket, '127.0.0.1', function() {
					client.write(command.split(" ")[0]+" ./audioFile/"+command.split(" ")[1]);
				});

				client.on('data', function(data) {
					var dataLength 	= data.length;
					var dataChar	= [];
					var dataString;
					var responseArray;
					for(index=0;index<dataLength;++index)
						dataChar[index]=String.fromCharCode(data[index]);
					dataString=dataChar.join("");
					util.log("NODE PROCESS - LOG INFO - loadFile	  : Response from CProcess : "+ dataString);
					responseArray = dataString.split(" ");

					if(responseArray[0]=="OK")
						res.status(200).json({"success": true, "data": responseArray[1]});
					else
						res.status(200).json({"success": false, "data": responseArray[1]});

					client.destroy();
				});
			});
		});
	});
});

app.get('/launchSynchronization',function(req,res){
	var client 		= new net.Socket();

	client.connect(cProcessSocket, '127.0.0.1', function() {
		client.write("PLAY_FILE");	
	});

	client.on('data', function(data) {
		var dataLength 	= data.length;
		var dataChar	= [];
		var dataString;
		var responseArray;
		for(index=0;index<dataLength;++index)
			dataChar[index]=String.fromCharCode(data[index]);
		dataString=dataChar.join("");
		util.log("NODE PROCESS - LOG INFO - launchSynchronization: Response from CProcess: "+dataString);
		responseArray = dataString.split(" ");

		if(responseArray[0]=="OK")
			res.status(200).json({"success": true, "data": responseArray[1]});
		else
			res.status(200).json({"success": false, "data": responseArray[1]});
		
		client.destroy();
	});
	
	client.on('close',function(){
	});
});

app.get('/getTimeStamp',function(req,res){
	var client 		= new net.Socket();

	client.connect(cProcessSocket, '127.0.0.1', function() {
		client.write("GET_TIMST");	
	});

	client.on('data', function(data) {
		var dataLength 	= data.length;
		var dataChar	= [];
		var dataString;
		var responseArray;
		for(index=0;index<dataLength;++index)
			dataChar[index]=String.fromCharCode(data[index]);
		dataString=dataChar.join("");
		util.log("NODE PROCESS - LOG INFO - getTimeStamp  : Response from CProcess : "+dataString);
		responseArray = dataString.split(" ");
		if(responseArray[0]=="OK")
			res.status(200).json({"success": true, "data": responseArray[1]});
		else
			res.status(200).json({"success": false, "data": responseArray[1]});
		
		client.destroy();
	});
	
	client.on('close',function(){
	});
});

var isLaunched = false;
var launchTimeStamp = 0;
app.get('/connect',function(req,res){
	var filmName = "Spectre007";
	var filmDescription = "This is a thriller film, with lot of suspense";
	var file = __dirname + '/audioFile/right.wav';
	var readStream =fs.createReadStream(file);
	readStream.pipe(res);
  	//res.download(file); // Set disposition and send it.
	console.log("faissal");
	res.status(200).json({"success": true, "filmName": filmName , "filmDescription" : filmDescription});
});

app.post('/synchronize',function(req,res){
	var device_TS		= req.body.deviceTS;
	if(!isLaunched)
		res.status(200).json({"success": false, "data": "Film isn't yet launched"});
	else{
		var delta = Date.now() - launchTimeStamp;
		res.status(200).json({"success": true, "playerDelay": delta, "deviceTS": device_TS});
	}
});
app.get('/launch',function(req,res){
	if(isLaunched)
		res.status(200).json({"success": true, "data": "Film is already launched"});
	else{
		isLaunched = true;
		launchTimeStamp = Date.now();
		res.status(200).json({"success": true, "data": "Launching timestamp is "+ launchTimeStamp});
	}
});
/*
	End user communication
*/


/*
	Main server communication
*/

app.post('/upload', function(req, res){

  // create an incoming form object
  var form = new formidable.IncomingForm();

  // specify that we want to allow the user to upload multiple files in a single request
  form.multiples = true;

  // store all uploads in the /uploads directory
  form.uploadDir = path.join(__dirname, '/uploads');

  // every time a file has been uploaded successfully,
  // rename it to it's orignal name
  form.on('file', function(field, file) {
    fs.rename(file.path, path.join(form.uploadDir, file.name));
  });

  // log any errors that occur
  form.on('error', function(err) {
    console.log('An error has occured: \n' + err);
  });

  // once all the files have been uploaded, send a response to the client
  form.on('end', function() {
    res.end('success');
  });

  // parse the incoming request containing the form data
  form.parse(req);

});