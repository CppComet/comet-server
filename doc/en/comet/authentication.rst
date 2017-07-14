====== Users’ authorization on the comet-server ======

Besides of channels where each user, who know channel’s name, can subscribe it, here is an opportunity to authorize users on the comet-server and send private messages to users by their IDs.  User authorization involved several steps. The first step – it is sending user’s ID and a random hash in your system to the comet-server.

<code sql>
mysql> INSERT INTO users_auth (id, hash )VALUES (1, 'auth_hash1');
</code>
  * Here the line with auth_hash1 – an authorization text key. You can generate it by yourself on your server and send it to comet-server via insert query in table users_auth, then you pass on JavaScript to authorization a certain user on the comet-server;
  * Numeric_user’s_id – it is user’s ID on your site, any positive number not longer than 9 digits;
  * dev_id – it’s a [[en:comet:dev_id|public developer’s identifier]].
<code JavaScript>
// At the second stage these data (user’s ID and hash) need to be passed to JavaScript Api.
$(document).ready(function()
{
    CometServer().start({dev_id:1, user_key:"auth_hash1", user_id:"Numeric_user’s_id" })
});
</code>
And now user can be successfully authorized on the comet-server.

====== Sending messages for authorized users ======

While messages from authorized users are sending by its ID ([[en:comet:cometql|insert query to table users_messages]]), messages is sending to all user’s devices (up to 16), where he is authorizing at this moment. It’s very convenient when some user has visited your site and has authorized here by more than one device.
  
If some user at that moment is offline, than sent message is placed into a queue of messages and would be delivered when it user will be online.

The main purpose of mentioned queue is delivering messages after a short-term user’s offline. For example, in those cases when user is updating some site’s page where was opened a connection, and this user goes offline approximately for one second.

====== The subscription to receiving private messages ======
In order to receiving private messages you should to subscribe to it. The subscription to messages with name of event “event1” from server is delivering in accordance to authorization data (by user’s ID) is carried out in the following way:
<code JavaScript>CometServer().subscription("msg.event1", function(e){ console.log(e)})</code>

The msg channel refers to [[en:comet:javascript_api:pipe-types|channels list with special properties]].
