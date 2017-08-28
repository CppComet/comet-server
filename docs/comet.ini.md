# Configuring the comet.ini file

 * Parameters can be specified in any order.
 * Register is important
 * The comment begins with the `;`
 
# Section [main]
 
Password for access from cometQL

```
password = 0000000000000000000000000000000000000000000000000000000000000000
```

The maximum size of the buffer in bytes for working with messages

```
buf_size = 10384
```

Buffer size in bytes for storing fragmented messages

```
fragment_buf_size = 600
```

The maximum size of the fragmented messages storage buffer

```
fragment_buf_max_size = 10384
```

To output in a log queries coming through cometQL

```
useQueryLoger = false
```

The maximum size of the buffer to respond to a request

```
answer_buf_size = 1000000
```

The comet server can give out static files. By default, this feature is disabled. But if you set the parameter base_dir, then it starts to give files by name from this directory as a primitive http server. Supports only get requests. In the [content-type] section, you can set the extension to the content-type header

```
base_dir = /var/www/html
```

# Section [statistics]

In the comet server, there is a mechanism to collect usage statistics and send them to the developer server (For what and what data is sent, see http://statistics.comet-server.ru/api/statistics )
To disable sending statistics, set the value to false

```
allow = true
```

# Section [db]
Connecting to mysql

```
host = localhost
user = root
password = root
name = comet_db
port = 3305
```

# Section [ws]
ip address for receiving connections from websockets

```
ip = 0.0.0.0
```

The number of streams into which connections from Web sites are processed, each thread increases the total memory consumption by approximately 10 - 15 mb so that for a low-loaded configuration one can call 1 stream

```
thread_num = 4
```

The comet server takes into account the number of online connections and the number of network events, it is the interval in seconds between the measurements that the value 0 does not measure

```
statistics = 10
```

Port for listening to connections on websockets

```
port = 8087
```

The interval in seconds to check uptime for connections, 0 do not run the check

```
uptimeTestInterval = 600
```

The maximum uptime connection after which the connection is terminated forcibly

```
maxUptime = 0
```

A comma-separated list of hosts from which it is allowed to connect from javascript api (checked based on the http header), there is no limit by default

```
host = *
```

# Section [cometql]

```
ip = 0.0.0.0 
thread_num = 3 ; number of threads for receive message from cometql
statistics = 10
port = 3300
uptimeTestInterval = 600
maxUptime = 0
```
  
# Section [content-type]  

Allows you to set the content-type and file type.
For example:

```
js = text/javascript
css = text/css
```

# Section [log]
Setting the Logging Level The higher the number the more logs are added.
  * 0 - do not output
  * 100 - Critical Errors
  * 200 errors
  * 300 - Warnings
  * 400 - log
  * 500 - debugging mode

The parameter sets the maximum level of logging common for all subsystems

```
level = 200
```
 
Logging levels for individual subsystems, in order to be able to watch the log separately for different events, is used for debugging convenience

```
Any = 500
```

Events from class logger

```
TagLoger = 500
```

Class storage events

```
appConf  = 500
```

Events from the interaction class via pipe (in the operating system)

```
pipeCommands = 500
```

Statistics module

```
statistics = 500
```

Disabling overtime connections

```
removeOldConnections = 500
```

Working with mysql

```
MySqlServer = 500
```

Class of work with user data (authorization, the last time online)

```
UserIndex = 500
```

User class (authorization, last online time)

```
UserItem = 500
```

Working with websockets

```
ClientServer = 500
```

Outdated section with php api

```
ServerServer = 500
```

Working with radishes (obsolete section)

```
SimpleRedisClient = 500
```

Reception and distribution of network connections

```
tcpServer = 500
```

Blanks of clustering are not all ready

```
CometQLCluster = 500
```

Working with authorization from cometql connections

```
devManager = 500
```

Sending requests to the mysql database

```
dbLink = 500
```

