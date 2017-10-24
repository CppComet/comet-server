
# CometQL

**CometQL** - it’s an API for work with comet server through MySQL protocol.

Advantages of CometQL:
 
  -  Unified API for more than 12 programming languages.
  -  Simple and intelligible query view.
  -  PHP includes resources for maintaining persistent connections with MySQL and now you can use it for co-working with comet server.

So, data is stored in tables and commands for select or insert perform some actions. For example, for receiving information when user was online, you can perform next query:

```
select * from users_time where id = 2;
```

And what we can see:

```

mysql> select * from users_time where id in( 2, 3, 145);
+-----+------------+
| id  | time       |
+-----+------------+
| 2   | 0          |
| 3   | 1438245468 |
| 145 | -1         |
+-----+------------+
3 rows in set (0.31 sec)

```

Here user with id=2 at the moment online, user with id=3 was online at July 30 and for user with id=145 there is no available data.
# How to connect and try by yourself
You can by yourself connect to demo data and try it now. We recommend you to connect via console, because there are some graphics MySQL clients, which work inappropriate with comet server. 

```

# Server app.comet-server.ru
# Login 15
# Password lPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8
# Database CometQL_v1

# Line to connect from command prompt
mysql -h app.comet-server.ru -u15 -plPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8 -DCometQL_v1  --skip-ssl

```


Else you can use [online command prompt](/docs/EN/Online%20CometQL%20CLI.md). You can find it on the bottom of the right corner on all of the pages.  


___
[Here you can test source code based on PHP with CometQL](/docs/EN/Online%20CometQL%20CLI.md) using for online command prompt implementation.
___

 
An example of connecting to a comet server from php.

```

$dev_id = "15"; // Used as login
$dev_key = "lPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8"; // Used as a password

// The connection looks like we connected to the database. You can use the functions to work with mysql
// But in fact you are connecting to the comet server.
$link = mysqli_connect("app.comet-server.ru", $dev_id, $dev_key, "CometQL_v1");
if(!$link)
{  
    die("Could not create connection with CometQL");
}

$result = mysqli_query (  $link, "show status" ); 
if(mysqli_errno($link) != 0)
{
    echo "Error code:<a href='https://comet-server.com/wiki/doku.php/en:comet:cometql:error'  target='_blank' >".mysqli_errno($link)."</a>";
    echo "Error text:<a href='https://comet-server.com/wiki/doku.php/en:comet:cometql:error' target='_blank' >".mysqli_error($link)."</a>";
    exit;
}

while($row = mysqli_fetch_assoc($result))
{
    echo "<pre>";
    var_dump($row);
    echo "</pre><br>";
}  


```

# Table description # 


___
Names of tables and columns always use in lower case.
___


Notes of current implementation of CometSQL:
  * CometSQL doesn’t implement all methods of SQL. A part of functions planned to realize in future. But some notes were applied in order to optimize inner structure of Comet server.
  * To [optimize internal logics](/docs/EN/API/Optimization%20delete%2C%20and%20insert%20in%20CometQL.md), such operations like delete and insert doesn’t return amount of used lines (it belongs to CometQL v.1.0).
  * Supporting such operators like AND, OR and ORDER BY in queries will be realize soon.


___
Some of hosting providers determine forbiddance (или ban) on external connections – you can face with this on some free or half-free hosting services. In order to check ability to use CometQL, you can take advantage of [hosting verification script](/docs/EN/Checking%20the%20hosting%20for%20possible%20connect%20to%20the%20comet%20server.md). 
___

# Table pipes_messages

The table pipes_messages contains messages which transported via channels. For sending messages via channel you need to perform query of insert (INSERT) in this table

```

mysql> insert into pipes_messages (name, event, message)values("pipe_name", "event_in_pipe", "text message");
Query OK, 0 rows affected (0.13 sec)

```
 

Fields “name” and “event” must match to next regular term [0-9A-z=+/_].


___
Find out quantity of attendance on channel is possible by sending query to pipes table, because this message sends to all channel’s subscribers. In this way you can enquire amount of attendance, which get this message.
___

 
Selection query from pipes_messages returns history of messages of current channel if the save-function was activate for this channel.

```

mysql> select * from pipes_messages where name = "p10";
+------+-------+-------+--------------+
| name | index | event | message      |
+------+-------+-------+--------------+
| p10  | 0     | event | msgData      |
| p10  | 1     | event | msgqqrrata   |
| p10  | 2     | evt3  | msgqqrrata   |
+------+-------+-------+--------------+
3 rows in set (0.00 sec)

```


Clears messages history of this channel.

```

mysql> delete from pipes_messages where name = 'p10';
Query OK, 0 rows affected (0.13 sec)

```


# Online example
Input channel name “pipe _name” and click “subscribe”.
<html>

</html>
Now perform with online command prompt insert-query with pipes_messages and check the result.

```

mysql> insert into pipes_messages (name, event, message)values("pipe_name", "event_in_pipe", "text message");

```
 
# Table pipes 
The table “pipes” contains information about quantity of subscribers on messages from channels. This table is available only for reading. 

```

mysql> select * from pipes where name in( "web_admins", "web_php_chat");
+--------------+-------+
| name         | users |
+--------------+-------+
| web_admins   | 0     |
| web_php_chat | 0     |
+--------------+-------+
2 rows in set (0.30 sec)

```

 
# Online example

Perform this query:

```

mysql> select * from pipes where name in( "web_admins", "web_php_chat");

```

Input channel name “web_admins” and click “subscribe”. 
<html>

</html>
Now perform query again and you will see that amount of subscribers are increase.
# Table users_in_pipes
The table “users_in_pipes” contains data about [authorized users](/docs/EN/API/Authorization%20on%20the%20CppComet.md) which subscribe on channel. This table is available only for reading.  

```

mysql> select * from users_in_pipes where name = "web_admins";
+------------+---------+
| name       | user_id |
+------------+---------+
| web_admins | 2       |
| web_admins | 4       |
| web_admins | 14      |
| web_admins | 9       |
+------------+---------+
4 row in set (0.32 sec)

```



___
Field user in table pipes contain aggregate amount of subscribers ([authorized and non-authorized](/docs/EN/API/Authorization%20on%20the%20CppComet.md)), besides of table users_in_pipes includes only authorized subscribers list.
___


# Table pipes_settings 
The table “pipes_settings” contains settings of logging channels. By default, messages which passes through the channel, doesn’t store. If you activate logging mechanism for channel, comet server will be store last “n” messages. 
To activate logging mechanism you must perform next query:

```

mysql> insert into pipes_settings ("name", "length") values ('p10', 10);
Query OK, 1 row affected (0.00 sec)

```

Here parameter length – it’s a number of last stored messages. It takes values from 0 to 99.

To get setting values, need to perform selection query from pipes_settings.

```

mysql> select * from pipes_settings where name = 'p10';
+------+--------+
| name | length |
+------+--------+
| p10  | 10     |
+------+--------+
1 row in set (0.00 sec)

```


To deactivate logging mechanism, need to delete from pipes_settings settings record.

```

mysql> delete from  pipes_settings where name = 'p10';
Query OK, 0 rows affected (0.00 sec)

```


# Table users_messages
The table “users_messages” intended sending messages to [authorized users](/docs/EN/API/Authorization%20on%20the%20CppComet.md) by their identifiers.

___
Sending messages by identifiers gives more powerful protection tools while data is transferring. Also it increases probability to deliver message to right user.
___

For example, for sending message to user with id=2 and message body “message” you must perform next query:

```

mysql> insert into users_messages (id, event, message)values (2, 'event', 'message');
Query OK, 0 row affected (0.00 sec)

```

Message put into queue to be send to user soon or send immediately. 

If you want to get all of the undelivered messages and which stuck in queue, perform the query “select”

```

mysql> select * from users_messages where id = 2;
+----+-------+-------+---------+
| id | index | event | message |
+----+-------+-------+---------+
| 2  | 0     | evnt1 | message |
| 2  | 1     | evnt2 | messag2 |
+----+-------+-------+---------+
2 rows in set (0.00 sec)

```

Now 2 messages wait for sending. They will be sending at once when user becomes online. This table contains 4 columns.
  - id - user’s identifier.
  - index - message’s number in queue.
  - event - name of event.
  - message - message’s body.

To clearing queue use “delete” query.

```

mysql> delete from users_messages where id = 2;
Query OK, 0 rows affected (0.08 sec)

```


___
After delivering message to user, it will be automatically deleted from queue.
___


# Table users_time
The table users_time contains data about when users were online.  This table is available only for reading. Data timing stores in UNIX-time.

```

mysql> select * from users_time where id in( 2, 3, 145);
+-----+------------+
| id  | time       |
+-----+------------+
| 2   | 0          |
| 3   | 1438245468 |
| 145 | -1         |
+-----+------------+
3 rows in set (0.31 sec)

```

Here user with id=2 at that moment online, user with id=3 – was online at July 30, and for user with id=145 data is unavailable.
 
# Table users_auth

The table users_auth contains data of user’s [authorizing](/docs/EN/API/Authorization%20on%20the%20CppComet.md) on comet server. 

```

mysql> insert into users_auth (id, hash )values (12, 'hash1');
Query OK, 1 row affected (0.13 sec)

mysql> select * from users_auth where id in(2, 3, 12);
+----+----------------------------+
| id | hash                       |
+----+----------------------------+
| 2  | bjl6knotdb2t1oov958mhuian7 |
| 12 | hash1                      |
+----+----------------------------+
2 rows in set (0.32 sec)

```

Here for user with id=3 no data available, for users with id=2 and id=12 data is exist.

___
In field hash you can transmit only lines (strings) less than 32 symbols and it must match with regular term [0-9A-z=+/_].
___

To deleting data of [user’s authorizing](/docs/RU/API/%D0%90%D0%B2%D1%82%D0%BE%D1%80%D0%B8%D0%B7%D0%B0%D1%86%D0%B8%D1%8F%20%D0%BF%D0%BE%D0%BB%D1%8C%D0%B7%D0%BE%D0%B2%D0%B0%D1%82%D0%B5%D0%BB%D0%B5%D0%B9%20%D0%BD%D0%B0%20%D0%BA%D0%BE%D0%BC%D0%B5%D1%82%20%D1%81%D0%B5%D1%80%D0%B2%D0%B5%D1%80%D0%B5.md), you can use “delete” query.

```

delete from users_auth where id = 12;
Query OK, 0 rows affected (0.00 sec)

```


___
To optimize internal logics, such operations like delete and insert doesn’t return amount of used lines (it belongs to CometQL v.1.0).
___


# Other information

  * [Example of sending messages from bash.](/docs/EN/An%20example%20of%20sending%20a%20message%20from%20a%20bash%20script.md)
  * [Error code in CometQL](/docs/EN/API/Error%20codes%20in%20CometQL.md)
  * [Reserved channel names](/docs/EN/API/Reserved%20channel%20names.md)
  * [JavaScript API](/docs/EN/API/JavaScript%20API/JavaScript%20API.md)
  * [What it is and why do we need "A public developer token" and "Secret developer key?"](/docs/EN/Connection%20settings.md)

# Reserved channels names

Main article [Reserved channels names and channels with extra properties](/docs/EN/API/Reserved%20channel%20names.md).

___
We don’t recommend to use in our project such channels names as «bin_*», «big_*», «push_*», «comet_*» и «sys_*». These names could be used for new functions. And they can have some special properties besides of current channels names.
___



Also we already have some channels with extra properties:
  * msg – used for sending messages in order to authorizing data;
  * user_status_* - used for automatic notification JS API about users’ status;
  * web_* - channels which can receive messages from CometQL and JS API. 

# Wrapping over CometQL api

If you have written a wrapper for working with CometQL api for which a framework that would be great if you share its running time with other users. Send us links to your repository on the wrapper support@comet-server.com