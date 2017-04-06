# PHP chat example

![php chat demo](https://github.com/Levhav/comet-server/blob/master/demo/chat-example/chat.gif)

[Online demo](https://jsfiddle.net/o35kvmn2/5/)

# Scheme of chat

Tipycal scheme of chat:

![scheme-of-chat](https://github.com/Levhav/comet-server/blob/master/demo/chat-example/scheme-of-chat.jpg)

* Connecting to the comet server by websockets
* Send ajax message for add new massage to chat
* Send message to CppComet
* CppComet send messages for all subscribers in pipe
* Add message to database (optionals)

# Step 1. Connecting to the comet server from JavaScript api

CppComet has cloud saas alternative that can be used for testing and demo access. In the following examples I will use demonstration access from comet-server.com for those who could not or were too lazy to deploy the server on their vps

```Login: 15
Password:lPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8
Host: app.comet-server.ru
```

For connecting to the comet server from JavaScript api use this command:

```
cometApi.start({node:"app.comet-server.ru", dev_id:15})
```

* in parametr node - set hostname of yure ouwn server
* parametr dev_id - use only if yure use saas servece comet-server.com


# Step 2. send message to server

* Send ajax query to php backend
* Send CometQL query for comet server

[code of php beackend](https://github.com/Levhav/comet-server/blob/master/demo/chat-example/chat.php)

Code of connecting to CppComet by mysql protocol:
```
$host = "app.comet-server.ru";
$user = "15";
$password = "lPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8";
$comet = mysqli_connect($host, $user, $password, "CometQL_v1");
```


Code of send message to pipe "simplechat" and event 'newMessage':
```
$query = "INSERT INTO pipes_messages (name, event, message)VALUES('simplechat', 'newMessage', '".$msg."')"; 
mysqli_query($comet, $query);
```


# Step 3. recive message from comet server

Code of subscription to pipe on comet server. This callback will be called when sombody send message to pipe `simplechat`

```
    cometApi.subscription("simplechat.newMessage", function(event){
        $("#web_chat").append('<b>'+HtmlEncode(event.data.name)+'</b>')
        $("#web_chat").append('<pre>'+HtmlEncode(event.data.text)+'</pre>')
        $("#web_chat").append('<br>')
    })
```
