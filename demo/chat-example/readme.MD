# PHP chat example

[Online demo](https://jsfiddle.net/o35kvmn2/5/)

![php chat demo](https://github.com/Levhav/comet-server/blob/master/demo/chat-example/chat.gif)

# Scheme of chat

Typical scheme of chat:

![scheme-of-chat](https://github.com/Levhav/comet-server/blob/master/demo/chat-example/scheme-of-chat.jpg)

* Connecting to the comet server by websockets
* Send ajax message for add new massage to chat
* Send message to CppComet
* CppComet send messages for all subscribers in pipe
* Add message to database (optional)

# Step 1. Connecting to the comet server

CppComet has cloud saas alternative that can be used for testing and demo access. In the following examples I will use demonstration access from comet-server.com for those who could not or were too lazy to deploy the server on their VPS

```Login: 15
Password:lPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8
Host: app.comet-server.ru
```

To connect to the comet server from the JavaScript API, use the following command:

```
cometApi.start({node:"app.comet-server.ru", dev_id:15})
```

* in parameter node - set hostname of your own server
* parameter dev_id - use only if you use saas service comet-server.com


# Step 2. send message to server

* Send ajax query to php back-end
* Send CometQL query for comet server

[code of php back-end](https://github.com/Levhav/comet-server/blob/master/demo/chat-example/chat.php)

Connection code to CppComet using MySQL protocol:
```
$host = "app.comet-server.ru";
$user = "15";
$password = "lPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8";
$comet = mysqli_connect($host, $user, $password, "CometQL_v1");
```


The code for sending a message to the pipe "simplechat" and event 'newMessage':
```
$query = "INSERT INTO pipes_messages (name, event, message)VALUES('simplechat', 'newMessage', '".$msg."')"; 
mysqli_query($comet, $query);
```


# Step 3. receive message from comet server

subscription Code to the pipe on comet server. This callback will be called when somebody send message into channel `simplechat`

```
    cometApi.subscription("simplechat.newMessage", function(event){
        $("#web_chat").append('<b>'+HtmlEncode(event.data.name)+'</b>')
        $("#web_chat").append('<pre>'+HtmlEncode(event.data.text)+'</pre>')
        $("#web_chat").append('<br>')
    })
```
