var fs = require("fs");
var http = require("http");
var url = require("url");
var net = require('net');

var g_seqNo = 0x41;

try {
    var confData = fs.readFileSync("doraconfig.json", "utf8");
} catch (err) {
    console.log(err.message);
    process.exit(1);
}
g_config = JSON.parse(confData);
console.log(Date() + " <Starting server:> GPSport=" + g_config.gpsPort + ", HTTPport=" + g_config.httpPort);

function checksum(buf) {
    var cksum = 0;
    for (var i = 0; i < (buf.length); i++) { cksum = cksum + buf[i].charCodeAt(0); }
    cksum = cksum%256;
    return cksum.toString(16).toUpperCase();
}

function frameMessage(buf) {
    if(g_seqNo < 0x7A) { g_seqNo++; } else { g_seqNo = 0x41; }
    var buflen = buf.length+5;
    var msg = "@@" + String.fromCharCode(g_seqNo) + buflen + buf + "*";
    return msg + checksum(msg);
}

function checkValidClient(buf) {
    var found = null;
    g_config.devices.forEach(function(item, index) {
        if(item.password == buf) { found = index; }
    });
    return found;
}

function checkValidImei(buf) {
    var found = null;
    g_config.devices.forEach(function(item, index) {
        if(item.imei == buf) { found = index; }
    });
    return found;
}

function serveRenaClient() {
    http.createServer(function(request, response){
	response.writeHeader(200, {"Content-Type": "text/html"});
        if(url.parse(request.url).query != null) {
            var clientId = checkValidClient(url.parse(request.url).query.split('=')[1]);
            if(clientId != null) {
                var locationAge;
                if(g_config.devices[clientId].location == null) {
                    g_config.devices[clientId].lastAccess = 0;
                    g_config.devices[clientId].location = {version:"0.1", lat:"0.0", lon:"0.0", height:"0.0", age:"0.0"};
                    console.log(Date() + " <creating new location #" + clientId + ">");
                }
                if(g_config.devices[clientId].lastAccess == 0) { locationAge = 3600; }
		else { locationAge = (new Date().getTime()/1000 - g_config.devices[clientId].lastAccess); }
                if(locationAge > 3600) { locationAge = 3600; }
		g_config.devices[clientId].location.age = locationAge.toFixed(0);
                response.write(JSON.stringify(g_config.devices[clientId].location));
                console.log(Date() + " <serving location #" + clientId + ": " + JSON.stringify(g_config.devices[clientId].location) + ">");

                if(g_config.devices[clientId].connection != null) {
                    var request = frameMessage("," + g_config.devices[clientId].imei + ",A10");
                    console.log(Date() + " <requestig location #" + clientId + ": "+ request + ">");
                    g_config.devices[clientId].connection.write(request + '\r\n');
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
    }).listen(g_config.httpPort);
}

function serveMeitrackClient() {
    net.createServer(function (socket) {
        console.log(Date() + " <<" + socket.remoteAddress + ":" + socket.remotePort + ">>");
        socket.on('data', function (data) {
            var locationInput = data.toString().split(',');
            var clientId = checkValidImei(locationInput[1]);
            if(clientId != null) {
                if(g_config.devices[clientId].location == null) {
                    g_config.devices[clientId].lastAccess = 0;
                    g_config.devices[clientId].location = {version:"0.1", lat:"0.0", lon:"0.0", height:"0.0", age:"0.0"};
                    console.log(Date() + " <creating new location #" + clientId + ">");
                }
		g_config.devices[clientId].connection = socket;
                g_config.devices[clientId].lastAccess = new Date().getTime()/1000;
                g_config.devices[clientId].location.age = 0;
                g_config.devices[clientId].location.lat = locationInput[4];
                g_config.devices[clientId].location.lon = locationInput[5];
                g_config.devices[clientId].location.height = locationInput[13];
                console.log(Date() + " <input location from #" + clientId + "> " + JSON.stringify(g_config.devices[clientId].location));
                console.log(data.toString());
            } else {
                console.log(Date() + " <illegal IMEI>");
            }
        }); 

        socket.on('end', function () {
            console.log(Date() + " <<killed>>");
        });

        socket.on('error', function(err) {
            console.log(Date() + " <<Caught error:>>" + err.message);
        });
    }).listen(g_config.gpsPort);
}

serveRenaClient();
serveMeitrackClient();
