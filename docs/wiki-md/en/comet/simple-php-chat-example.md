
# PHP chat example

An example of using the [CppComet](https://github.com/CppComet/comet-server) server to create a chat.   

  * [jsfiddle.net online demo](https://jsfiddle.net/Levhav/o35kvmn2/17/)
  * [Github repo](https://github.com/CppComet/php-chat-example)

![chat demo](https://comet-server.com/wiki/lib/exe/fetch.php/en:comet:chat.gif )

# Scheme of chat

Typical scheme of chat:

![Typical scheme of chat](https://comet-server.com/wiki/lib/exe/fetch.php/en:comet:scheme-of-chat.jpg )

  * Connecting to the comet server by websockets
  * Send ajax message for add new massage to chat
  * Send message to CppComet
  * CppComet send messages for all subscribers in pipe
  * Add message to database (optional)
 
# Step 1. Connecting to the comet server

[CppComet](https://github.com/CppComet/comet-server) has cloud saas alternative that can be used for testing and demo access. In the following examples I will use demonstration access from https://comet-server.com for those who could not or were too lazy to [deploy the server on their VPS](https://github.com/CppComet/comet-server#building-from-source)


```
Login: 15
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

[code of php back-end](https://github.com/CppComet/php-chat-example/blob/master/chat.php)

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


# Full chat code

<html>
<script async src="//jsfiddle.net/Levhav/o35kvmn2/17/embed/"></script>
</html>

# Links

  * [JavaScript API](/docs/wiki-md/en/comet/javascript_api.md)
  * [CometQL API](/docs/wiki-md/en/comet/cometql.md)
  * [Online demo](https://jsfiddle.net/o35kvmn2/5/)
  * [CppComet](https://github.com/CppComet/comet-server)
  * [Creating a simple chat using CppComet](https://www.codeproject.com/script/Articles/ArticleVersion.aspx?waid=235463&aid=1181698)
