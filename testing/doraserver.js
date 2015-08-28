var http = require("http");
var url = require("url");
var net = require('net');
var currentcoordinates = {version:"0.1", lat:"0.0", lon:"0.0", height:"0.0"};

var mt90Port = 8766;
var httpPort = 8765;
var validIMEI = "123456789012345";
var password = "ToPsEcReT";

function serveRenaClient() {
    http.createServer(function(request, response){
	response.writeHeader(200, {"Content-Type": "text/html"});
	if(url.parse(request.url).query.split('=')[1] == password) {
            response.write(JSON.stringify(currentcoordinates));
            console.log(Date() + " <serving currentcoordinates: " +
                        JSON.stringify(currentcoordinates) + ">");
	} else {
	    response.write("denied");
            console.log(Date() + " <denied currentcoordinates>");
	}
	response.end();
    }).listen(httpPort);
}

function serveMeitrackClient() {
    net.createServer(function (socket) {
        console.log(Date() + " <<" + socket.remoteAddress + ":" + socket.remotePort + ">>");
        socket.on('data', function (data) {
            var myArr = data.toString().split(',');
            if(myArr[1] == validIMEI) {
                currentcoordinates = {version:"0.1", lat:myArr[4], lon:myArr[5], height:myArr[13]};
                console.log(Date() + " <input> " + JSON.stringify(currentcoordinates));
            } else {
                console.log(Date() + " <illegal IMEI>");
            }
        }); 

        socket.on('end', function () {
            console.log(Date() + " <<killed>>");
        });
    }).listen(mt90Port);
}

serveRenaClient();
serveMeitrackClient();
