var http = require("http");
var url = require("url");
var net = require('net');
var g_location = {version:"0.1", lat:"0.0", lon:"0.0", height:"0.0", age:"0.0"};
var g_locationAge = 0;
var g_connection = null;

var mt90Port = 8766;
var httpPort = 8765;
var validIMEI = "123456789012345";
var password = "ToPsEcReT";
var seqNo = 0x41;

function checksum(buf) {
    var cksum = 0;
    for (var i = 0; i < (buf.length); i++) { cksum = cksum + buf[i].charCodeAt(0); }
    cksum = cksum%256;
    return cksum.toString(16).toUpperCase();
}

function frameMessage(buf) {
    if(seqNo < 0x7A) { seqNo++; } else { seqNo = 0x41; }
    var buflen = buf.length+5;
    var msg = "@@" + String.fromCharCode(seqNo) + buflen + buf + "*";
    return msg + checksum(msg);
}

function serveRenaClient() {
    http.createServer(function(request, response){
	response.writeHeader(200, {"Content-Type": "text/html"});
        if(url.parse(request.url).query != null) {
	    if(url.parse(request.url).query.split('=')[1] == password) {
		var locationAge;
		if(g_locationAge == 0) { locationAge = 0; }
		else { locationAge = (new Date().getTime()/1000 - g_locationAge); }
		g_location.age = locationAge.toFixed(0);
                response.write(JSON.stringify(g_location));
                console.log(Date() + " <serving g_location: " + JSON.stringify(g_location) + ">");
                if(g_connection != null) {
                    var request = frameMessage("," + validIMEI + ",A10");
                    console.log(Date() + " <requestig location: "+ request + ">");
                    g_connection.write(request + '\r\n');
                }
	    } else {
	        response.write("denied");
                console.log(Date() + " <denied due wrong password>");
 	    }
        } else {
            response.write("password required");
            console.log(Date() + " <denied due no password>");
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
		g_connection = socket;
		g_locationAge = new Date().getTime()/1000;
                g_location.lat = myArr[4];
                g_location.lon = myArr[5];
		g_location.height = myArr[13];
                console.log(Date() + " <input> " + JSON.stringify(g_location));
                console.log(data.toString());
            } else {
                console.log(Date() + " <illegal IMEI>");
            }
        }); 

        socket.on('end', function () {
            console.log(Date() + " <<killed>>");
            g_connection = null;
        });

        socket.on('error', function(err) {
            console.log(Date() + " <<Caught error>>");
            console.log(err.stack);
	    g_connection = null;
        });
    }).listen(mt90Port);
}

serveRenaClient();
serveMeitrackClient();
