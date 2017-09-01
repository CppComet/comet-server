
# JavaScript API

  * [Download CometServerApi.js](http://comet-server.com/CometServerApi.js)
  * [GitHab repository with CometServerApi.js](https://github.com/Levhav/CometServerApi.js)

# Connection with server


The function start accepts connection settings and opens new connection. Here opt – it’s an object with connecting options.

```
cometApi.start({dev_id:15, user_id:1, user_key:"userHash", node:"app.comet-server.ru"})
```

  * dev_id it’s a binding parameter – [public developers’ key](/docs/EN/Connection%20settings.md). 
  * user_id it’s a non-binding parameter – user’s identifier is necessary to  [authorization on the comet server](/docs/EN/API/Authorization%20on%20the%20CppComet.md); 
  * user_key it’s non-binding parameter – it is an authorization hash which necessary to [authorize user on the comet server](/docs/EN/API/Authorization%20on%20the%20CppComet.md).  
  * node - If you deploy [CppComet](https://github.com/CppComet/comet-server) on your server, you must specify the address of your server

To reconnecting to server use the restart function.

```
cometApi.restart({dev_id:15, user_id:1, user_key:"userHash"})
```

To reconnecting you can override connection parameters or left ‘em previous. 
# Subscribe to receive messages from channel

The function subscription adds subscribes on channel, events and delivery reports.  

```
cometApi.subscription("channel_name", function(data){ console.log(data) } )
```

  * First argument – the channel name must be shorter than 32 symbols and may consist of the next symbols: A-Za-z0-9 and “-“ or “_”.
  * Second argument – the callback function which will be call while message is delivering from this channel.

Pay attention that as the body of message may be json line. If it’s true then is can automatically converted to object.

```
cometApi.subscription( "pipe_name.event_name", function(e){ console.log(["event", e])})
```

Subscription on channel “channel_name” 

```
cometApi.subscription("channel_name", function(e){ console.log(e)})
```

Subscription on channel’s events “event_name”on channel “channel_name”

```
cometApi.subscription("channel_name.event_name", function(e){ console.log(e)})
```

Subscription on delivery report on channel “channel_name” 

```
cometApi.subscription("#channel_name", function(e){ console.log(e)})
```

Subscription on all incoming messages from all of the channels which subscribed current client 

```
cometApi.subscription(function(e){ console.log(e)})
```


An example with online demo [How to accept message from channel in JavaScript](/docs/EN/How%20to%20take%20a%20message%20out%20of%20the%20channel%20in%20JavaScript.md)

# Unsubscription of receiving messages from channel

The subscription function returns the subscription_id line which can be necessary if we want to unsubscribe of receiving messages.

```
var subscriptionId = cometApi.subscription("channel_name.event_name", function(e){ console.log(e)})
```


For unsubscription of receiving messages, call: 

```
cometApi.unsubscription(subscriptionId)
```


# Reserved channel names

The main article [reserved channel names](/docs/EN/API/Reserved%20channel%20names.md).

___
We don’t recommend to use in our project such channels names as «bin_*», «big_*», «push_*», «comet_*» и «sys_*». These names could be used for new functions. And they can have some special properties besides of current channels names.
___


Also we already have some channels with extra properties:
  * msg – used for sending messages in order to authorizing data;
  * user_status_* - used for automatic notification JS API about users’ status;
  * web_* - channels which can receive messages from CometQL and JS API.
  * track_* - automatic notification JS api about what someone has subscribed or unsubscribed from this channel
# The subscription on receiving private messages ( pipe msg )======
The subscription on messages from server, delivered in order to authorization data (by user’s id):

```
cometApi.subscription("msg", function(e){ console.log(e)})
```

The subscription on messages from server with event name “event_name”, delivered in order to authorization data (by user’s id):

```
cometApi.subscription("msg.event_name", function(e){ console.log(e)})
```



___
More details about authorization mechanism on comet server you and about private messages look in article “[User’s authorization on comet server](/docs/EN/API/Authorization%20on%20the%20CppComet.md)”
___


# Subscription on changing user’s status ( pipes user_status_* )======

It is possible to subscribe from JS on notifications about some user’s activity on comet server. 

When user has authorized on comet server then server automatically sends a signal to channel user_status_{user_identifier} with event name online. When user is went offline, server generates some event too. 



```

    // Subscribe on notification that user with id=12 is online
    cometApi.subscription("user_status_12.online", function(event)
    {
         console.log("User with id=12 online")
    })

    // Subscribe on notification that user with id=12 is offline
    cometApi.subscription("user_status_12.offline", function(event)
    {
         console.log("User with id=12 offline")
    })

```



# List of online users (channels track_*)

Channels with a name like track_* automatically generate events subscription and unsubscription inside every time someone subscribes or unsubscribes from this channel


```

cometApi.Subscription("track_online.subscription", function(msg)
{
    // Event Processing that someone went to the website and subscribe to the channel track_online
});
cometApi.Subscription("track_online.unsubscription", function(msg)
{
    // Event Processing that someone left the site and / or unsubscribe from the channel track_online
});

```


This type of channel is designed specifically to facilitate the creation of dynamically updated lists of online users.
Main article [list of online users](/docs/en/comet/faq/realtime-users-list.md)
# Sending messages to channel from JS (pipes web_*)

The function web_pipe_send lets from JavaScript send messages to channel bypassing your server (directly calls comet server). It allows resending messages between clients without loading your server. Also owing to direct call to comet server, delivery time of message from client to client is minimal.


___
Send messages to channel from JavaScript is able when name of channel begins with “web_”. CometQL does not have any limitations in this case.
___



```
cometApi.web_pipe_send("web_pipe_name", "event_name", "message")
```


To getting a report about message delivery to channel “channel_name” use “subscription”.

```
cometApi.subscription("#channel_name", function(e){ console.log(e)})
```


Since comet server supports users’ authorization, it automatically adds user id to message such a way that a sender can’t send foreign id. To deactivate this option, is needed to add symbol “@” before channel name which would accept message. In this case, delivered message would be look alike that it was send by non-authorized user.

```
cometApi.web_pipe_send("@web_pipe_name", "event_name", "message")
```

 
<html>
<script async src="//jsfiddle.net/07noamea/3/embed/"></script>
</html>
# The format of incoming messages

```

 data: {}                   // User’s message
 server_info:
    event: "undefined"      // Event name
    history: false          // If true then it’s the data downloaded from channel’s history, not incoming at that moment
    marker: undefined       // Special identifier, implemented just only when history equals true 
    pipe: "web_chat_pipe"   // Channel’s name which directed message
    user_id: 0              // Sender id, if=0 then is omitted. It will be add automatically if sender was authorized on comet server.

```


Field server_info.user_id in incoming message fills out by some information besides zero just if message was sent to channel from JavaScript API and sender was authorized on comet server. Also it contains sender’s user_id.
# Getting the last messages from channel
The comet server includes possibility to activate storing the last N messages mechanism for some channels.

___
Logging of transferred messages may be activate by CometQL query to table “[pipes_settings](/docs/EN/API/CometQL/CometQL%20API.md)”.
___
 

If logging function is active, then method call “get_pipe_log” initiate sending all messages from history to client.

```
cometApi.get_pipe_log("web_pipe_name")
```

Delivered from history messages will have a property “history=true”.

# Getting a number of subscribers

The function “count_user_in_pipe” can helps to determine a number of subscribes for current channel.

Also this function has the first argument – name of channel and the second is callback function which holds an answer.

```

cometApi.count_users_in_pipe("web_chat_pipe", function(res)
{
    console.log("count_users_in_pipe", res, res.data.user_in_pipe)
})

```


Besides of [CometQL query](/docs/EN/API/CometQL/CometQL%20API.md), this function can show a number of subscribers only for those channels which name begins with “web_” (for example, for “web_chat_pipe” it will work but for “chat_pipe” – will not work). This limitation entered in order to be able to create such a channel which a number of subscribers can’t be accessible for anyone via JS API.
 
# Determination of authorization status on comet server

The main article [An authorization on comet server](/docs/EN/API/Authorization%20on%20the%20CppComet.md)

The comet server has an opportunity to authorize users. An authorizing can be useful for determination of sender, who create message.

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

# Determination of master tab

JavaScript API has one function which chooses from several tabs and granted one of these tabs as a “chief” tab – this tab called like master tab and others automatically determines like salve tabs. The function isMaster may return true if it performs like “chief” tab and return false if it performs like salve tab.


```
cometApi.isMaster()
```


Determination of which tab can be the master tab may be useful if you want to perform some action just on one tab. For example, you have opened a chat by 3 pages and at every income message plays audio notification. Here might be a good thing if just only one of these tabs will be make a sound, rather than all.

For more details about cooperation between these tabs, search in article [Message exchange between browser tabs](https://habrahabr.ru/company/comet-server/blog/250719/).

# References

In some cases file CometServer.js is inserted in the following way:

```

<script src="//comet-server.ru/CometServerApi.js" type="text/javascript"></script>

```

This approach can be right just only for tests and during developing period but not for permanent using. You can find the last version of JavaScript API [here](http://comet-server.ru/CometServerApi.js). Check it please to avoid some situation of incompatibility of your app with this soft. New version of JavaScript API might appear as soon as possible.  
To prevent this situation you can just only once download the file with CometServerApi.js on your server and you can use it until you won’t need updating.
  
# Additional Information
 
  * [Reserved channel names](/docs/EN/API/Reserved%20channel%20names.md)
  * [CometQL API](/docs/EN/API/CometQL/CometQL%20API.md)
  * [What is and why do we need "A public developer token" and "Secret developer key"](/docs/EN/Connection%20settings.md)
