/**
 * Эмуляция объекта window для запуска тестов js api в nodejs
 * https://www.npmjs.com/package/websocket
 */ 
var WebSocket = require('websocket').client;

 
var window = {
    WebSocket: {},
    inNodeJs:true
}
var document = {}



window.location = {
    href: "nodejs-tests"
}

window.WebSocket = function(url)
{
    var socket = new WebSocket();
     
    socket.on('connectFailed', function(error)
    {
        if(socket.onerror)
        {
            socket.onerror(error)
        }
        if(socket.onclose)
        {
            socket.onclose(error)
        }
    });

    socket.on('connect', function(connection) 
    {
        console.log("[js-test] WebSocket Client Connected");
        
        socket.send = function(data){ 
            var res = connection.sendUTF(data);
            console.log("[js-api] Send:`"+ data+"`" );
            
        }
        
        socket.close = function(){  
            console.log("[js-api] socket.close" );
            connection.close()
        }
        
        connection.on('error', function(error) {
            console.log("[js-test] Connection Error: " + error.toString());
            
            if(socket.onerror)
            {
                socket.onerror(error)
            }
        });
        connection.on('close', function() {
            console.log('echo-protocol Connection Closed');
            
            if(socket.onclose)
            {
                socket.onclose(error)
            }
        });
        connection.on('message', function(message) {
            if (message.type === 'utf8') {
                //console.log("[js-test] Received: '" + message.utf8Data + "'"); 
            }

            if(socket.onmessage)
            { 
                socket.onmessage({data:message.utf8Data})
            }
        });
 
        if(socket.onopen)
        {
            socket.readyState = 1
            socket.onopen()
        }
         
    });
    
    setTimeout(function(){
        socket.connect(url);
    }, 100)
     
    return socket;
}
      
