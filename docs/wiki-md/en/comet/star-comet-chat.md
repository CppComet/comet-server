
# Integrated chat plugin

It’s ready-made plugin chat which could embed on your site for private correspondence between users.

Source codes you can find on [GitHub](https///github.com/Levhav/Star.Comet-Chat).md, [Demo version](http///comet-server.ru/doc/CometQL/Star.Comet-Chat/backend-example/index.php).md
# Chat functionality

  * Possibility to transmit text messages with any attachments;
  * Searching contacts in contacts list;
  * Separate tabs for blocked and favorite users lists;
  * Audio notification when receiving a message;
  * Button “complain”;
  * Can be used by mobile browsers;
  * Can determine message status (read or unread). When user read the message you can see in front of message two check marks;
  * Opportunity to translate messages by Yandex.Translator API.

# Admin panel functionality

  * Real-time monitoring user’s messages;
  * Possibility to view any dialog;
  * Possibility to find all messages of selected user;
  * Appeal list of users;
  * Possibility to read messages only with attachments.

{{ :star-comet-chat:screenshot_21_.png?600 |}}

# Screenshots

{{ :star-comet-chat:screenshot_4_.png?600  |Внешний вид чата}}
# How to embed this chat


___
If you want to use this chat or some of its versions, please, contact us by email – star-chat@comet-server.ru. Today exists a shortage of useful links and examples of using this chat as well as comments from users about correct or incorrect work.  If you have some troubles with embedding this chat, please, contact us.

___


This chat can be located on separate server and on the same server, where you site has been existed. This chat can cooperate with your site via API. A lot of chat’s data can be held by its database. New information such as about users or some other data for authorization it receives from special queries.

For installing this chat on your server you need to download all of the examples from [here](https///github.com/Levhav/Star.Comet-Chat).md

One of the source files “Database.sql” includes tables’ structure, which this chat uses for correct work.  This file will be based for our database.

After this you must change file configuration:
  * ./config.php
  * ./backend-example/config.php

In file ./config.php you must specify chat settings for correct connection to database and storing correspondence. Also it’s necessary to specify setting to access comet server for sending out notifications.

```
 
/**
 * Database access
 */
$conf['mysql_db'] = "StarCometChat";
$conf['mysql_user'] = "StarCometChat";
$conf['mysql_pw'] = "RLUJ4TXE22XL5JTh";
$conf['mysql_host'] = "localhost"; 

/**
 * Comet server access
 * Get keys for access you can here https://comet-server.com
 */
$conf['cometQL_dev_id'] = "lPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8";
$conf['cometQL_key'] = 15;

```


Also in file ./config.php exists one option “admin_ids”, which lets itemize all users’ identifiers with administration roots. Grant a lot of users by these roots is not recommended, because it can seriously affects on chat’s working speed.

```

$conf['admin_ids'] = array(1, 2);    // List of users’ id with administration roots

```


Parameter trusted_ip in file ./config.php must include server IP which could call chat’s API.

```

/**
 * IP address which can call API methods
 * Or false if limit disables (non-safety)
 */
$conf['trusted_ip'] = false;

```


___
By default parameter trusted_ip equals false (because this is just example of code). In real life it’s necessary to determine IP address, which can makes queries.
___
 

Variable $trusted_ip in file ./backend-example/config.php may include server’s IP address which holds chat. 

```

/**
 * IP address which can call API methods
 * Or false if limit disables (non-safety)
 */
$trusted_ip = false;

```


___
By default parameter trusted_ip equals false (because this is just example of code). In real life it’s necessary to determine IP address, which can make query to API backend. 
___
 
# Data exchange

The directory backend-example includes files with example of integration this chat to PHP project. If you want to repeat this action, it’s necessary to create handler for those queries which a chat module will be send for getting data about registered users.

# URL_getUsersInfo

The chat will be send to your project post-queries for getting data about users. Final address with these queries contains in file ./fonfig.php, name of parameter “URL_getUserInfo”. There is an example code, determines this value.

```

/**
 * URL getting data about users json
 */
$conf['URL_getUsersInfo'] = 'http://comet-server.ru/doc/CometQL/Star.Comet-Chat/backend-example/chat_get_users.php'; 

```


The parameter users in this query may pass id list and for which, separated by commas, it’s necessary to return data.

```

POST /doc/CometQL/Star.Comet-Chat/backend-example/chat_get_users.php HTTP/1.1
Host: comet-server.ru
Origin: https://comet-server.ru

users=1,2

```

As answer it would be waiting for json line with next objects:

```

[{
    "user_id":1,
    "avatar_url":"http:\/\/comet-server.ru\/doc\/CometQL\/Star.Comet-Chat\/img\/avatar0.png",
    "name":"Виктор",
    "city":"Владивосток",
    "age":24,
    "status":"active",
    "login":"victor"
},
{
    "user_id":2,
    "avatar_url":"http:\/\/comet-server.ru\/doc\/CometQL\/Star.Comet-Chat\/img\/avatar0.png",
    "name":"Лена",
    "city":"Владивосток",
    "age":25,
    "status":"active",
    "login":"lena"
},]

```

The file ./backend-example/chat_get_users.php consists of code which correctly performs query and back correct data.

# URL_getUsersHash

For getting user’s authorization hash the chat will be sending post-query for address, which stores in parameter URL_getUsersHash in file ./config.php. There is a fragment of code, which determines this value.


```

/**
 * URL for an authorization hash query
 */
$conf['URL_getUsersHash'] = 'http://comet-server.ru/doc/CometQL/Star.Comet-Chat/backend-example/chat_get_user_hash.php';

```


The parameter id in this query may pass user’s id for which it’s necessary to get authorization hash.

```

POST /doc/CometQL/Star.Comet-Chat/backend-example/chat_get_user_hash.php HTTP/1.1
Host: comet-server.ru
Origin: https://comet-server.ru

id=1

```

As the answer it will be waiting for authorization hash line.

```
56ff3f23bfd1071e14749aad42e58d89
```

The file ./backend-example/chat_get_users.php consists of code which correctly performs query and back correct data.


# JavaScript chat calls

The file ./backend-example/chat_get_users.php – it’s an example of user’s page. This file has an example of integration JavaScript chat calls.

There is code of launching chat plugin:

```

var user_id = <?php echo $_SESSION['userId']; ?>;  
var user_key = "<?php echo getUserHash($_SESSION['userId']); ?>";  
        
$(document).ready(function()
{ 
    /**
     * Connecting to the comet server for opportunity to accept commands
     * dev_id – your public developer’s identifier
     * user_id – user’s identifier which helps you to log in
     * user_key – you authorization hash
     */
    CometServer().start({dev_id:15, user_id:user_id, user_key: user_key})
     
    /**
     * We initialize chat module, next performs initialization and loading of required data for the correct work
     * Chat window in such case is not opened
     */
    StarCometChat.init({
        user_id: user_id,
        user_key: user_key, 
        open:false,
        
        // The parameter home_dir includes address with PHP scripts 
        home_dir: "http://comet-server.ru/doc/CometQL/Star.Comet-Chat",
        
        // The function, implemented in success, calls after successful chat initialization  
        success:function()
        {
            // The call of countNewMessagesSum return an amount of new messages. It works correctly just after ending of chat initialization
            var c = StarCometChat.countNewMessagesSum();
            if(c > 0)
            {
                $('#newMsgIndicator').html("You have "+ c + " new messages");
            }
        }
    });
});

```


We hope that comments in the code are clear to you. Each user gets online status just after a code of connecting to the comet server performs.

# After ending of initialization you can perform the next calls:
This method returns the value of unread messages.

```
StarCometChat.countNewMessagesSum()
```

This method returns the value of unread messages from user with user_id.

```
StarCometChat.countNewMessages(user_id)
```

This method opens the list of dialogs.

```
StarCometChat.openDialog()
```

This method opens the dialog with user with user_id. 

```
StarCometChat.openDialog(user_id)
```





