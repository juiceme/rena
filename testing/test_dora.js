var http = require("http");
var url = require("url");

var httpPort = 8765;
var startupcoordinates = {x:process.argv[2], y:process.argv[3], t:1};
var kulma = 0;

function toRad(Value) {
    /** Converts numeric degrees to radians */
    return Value * Math.PI / 180;
}

function servePage() {
    http.createServer(function(request, response){
	console.log(Date() + "     " + request.connection.remoteAddress);
	response.writeHeader(200, {"Content-Type": "text/html"});
	if(url.parse(request.url).query == "password=ToPsEcReT") {
            kulma = kulma + 10;
            if(kulma > 355) kulma = 0;
            var myx = parseFloat(startupcoordinates.x)+(0.001*Math.cos(toRad(kulma))),
                myy = parseFloat(startupcoordinates.y)+(0.001*Math.sin(toRad(kulma)));
            var currentcoordinates = {version:"0.1", lat:myx.toFixed(6), lon:myy.toFixed(6), t:1};
	    response.write(JSON.stringify(currentcoordinates));
	} else {
	    response.write("denied");
	}
	response.end();
    }).listen(httpPort);
}

servePage();

