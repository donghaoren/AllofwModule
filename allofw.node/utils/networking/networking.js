var Networking = function(config, role) {
    var self = this;
    this.broadcast = function() { };

    allofwutils.utils.makeEventSource(this);

    if(role == "renderer") {
        var sub = require("zmq").socket("sub");
        sub.connect(config.broadcasting.renderer.sub);
        sub.subscribe("");
        sub.on("message", function(msg) {
            var obj = JSON.parse(msg);
            self.raise.apply(this, [ obj[0] ].concat(obj[1]));
        });
        console.log("Renderer: Listening on " + config.broadcasting.renderer.sub);
    }
    if(role == "controller") {
        var pub = require("zmq").socket("pub");
        pub.bind(config.broadcasting.controller.pub);
        console.log("Controller: Braodcasting on " + config.broadcasting.controller.pub);
        this.broadcast = function(path) {
            var obj = [ path, Array.prototype.slice.call(arguments, 1) ];
            pub.send(JSON.stringify(obj));
        };
    }
};

allofwutils.Networking = Networking;

function HTTPServer(config) {
    var self = this;
    this.handlers = { };

    var express = require("express");
    var app = express();
    var http = require('http').Server(app);
    var io = require('socket.io')(http);

    app.use(express.static(config.http.static));

    http.listen(config.http.port, function() {
        console.log("HTTPServer: Listening on port " + config.http.port);
    });

    io.on('connection', function(socket) {
        // socket.on('disconnect', function() { });

        socket.on('m', function(msg) {
            try {
                if(self.handlers[msg[0]]) {
                    self.handlers[msg[0]].apply(null, msg[1]);
                }
            } catch(e) {
                console.log(e.stack);
            }
        });
    });
}

HTTPServer.prototype.on = function(event, handler) {
    this.handlers[event] = handler;
};

allofwutils.HTTPServer = HTTPServer;
