
# Users’ authorization on the comet-server

Besides of channels where each user, who know channel’s name, can subscribe it, here is an opportunity to authorize users on the comet-server and send private messages to users by their IDs.  User authorization involved several steps. The first step – it is sending user’s ID and a random hash in your system to the comet-server.


```

mysql> INSERT INTO users_auth (id, hash )VALUES (1, 'auth_hash1');

```

  * Here the line with auth_hash1 – an authorization text key. You can generate it by yourself on your server and send it to comet-server via insert query in table users_auth, then you pass on JavaScript to authorization a certain user on the comet-server;
  * Numeric_user’s_id – it is user’s ID on your site, any positive number not longer than 9 digits;
  * dev_id – it’s a [public developer’s identifier](/docs/EN/API/Connection%20settings.md).

```

// At the second stage these data (user’s ID and hash) need to be passed to JavaScript Api.
$(document).ready(function()
{
    CometServer().start({dev_id:1, user_key:"auth_hash1", user_id:"Numeric_user’s_id" })
});

```

And now user can be successfully authorized on the comet-server.

# Sending messages for authorized users

While messages from authorized users are sending by its ID ([insert query to table users_messages](/docs/EN/API/CometQL/CometQL%20API.md)), messages is sending to all user’s devices (up to 16), where he is authorizing at this moment. It’s very convenient when some user has visited your site and has authorized here by more than one device.
  
If some user at that moment is offline, than sent message is placed into a queue of messages and would be delivered when it user will be online.

The main purpose of mentioned queue is delivering messages after a short-term user’s offline. For example, in those cases when user is updating some site’s page where was opened a connection, and this user goes offline approximately for one second.

# The subscription to receiving private messages
In order to receiving private messages you should to subscribe to it. The subscription to messages with name of event “event1” from server is delivering in accordance to authorization data (by user’s ID) is carried out in the following way:

```
CometServer().subscription("msg.event1", function(e){ console.log(e)})
```


The msg channel refers to [channels list with special properties](/docs/EN/API/Reserved%20channel%20names.md).

# Determining the authorization status

You can track down changing authorization status on comet server from JavaScript API.

```

// Adding the callback function for notification about successful authorization
cometApi.onAuthSuccess(function(){
    console.log("Connection and authorization were successful")
})

// Adding the callback function for notification about unsuccessful authorization
cometApi.onAuthFalill(function(){
    console.log("Connection was successful, but no authorization found")
})

```

These functions will be called when authorization status will be changing. I.e. at least once an authorization will be successful or unsuccessful when connecting to comet server or while status will be changed. 

Also you can call the function “isAuthorized” to determine authorization status.

```
cometApi.isAuthorized()
```

The function “isAuthorized” may return 3 different values: 
  * false – authorization is not complete;
  * true - authorization is complete;
  * undefined – the status in undefined – this answer returns even before an attempt to connect to comet server.

# Online demo

[code of online demo](https://github.com/CppComet/auth-example) [online demo on github](https://cppcomet.github.io/auth-example/index.html) [Look in codepen.io](https://codepen.io/Levhav/pen/XaWLra)

<html>
<iframe height='800' scrolling='no' title='CppComet auth chat example' src='//codepen.io/Levhav/embed/XaWLra/?height=265&theme-id=dark&default-tab=js,result&embed-version=2' frameborder='no' allowtransparency='true' allowfullscreen='true' style='width: 100%;'>See the Pen <a href='https://codepen.io/Levhav/pen/XaWLra/'>CppComet auth chat example</a> by Trapenok Victor (<a href='https://codepen.io/Levhav'>@Levhav</a>) on <a href='https://codepen.io'>CodePen</a>.
</iframe>
</html>