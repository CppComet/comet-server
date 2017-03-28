![CppComet](https://comet-server.com/img/CppComet.png)

Comet technology – allows sending arbitrary messages to client through server initiative

## How push notification works

* Browser opens page of your site.
* After loading this page, JavaScript establishes permanent connection with Comet server.
* While page is open, your server can send random message to client. It appeals via [CometQL API](http://comet-server.org/doku.php/en:comet:cometql) to server and transfer message for browser.
* Comet server uses open connection with browser. Than server delivers message to browser.
* [JavaScript API](http://comet-server.org/doku.php/en:comet:javascript_api) delivers this message to your callback.

# Features

* Api for more then 9 languages. [CometQL API](http://comet-server.org/doku.php/en:comet:cometql) allows you connect to server by mysql protocol.
* Good documentation in [Russian](http://comet-server.org/doku.php/ru) and [English](http://comet-server.org/doku.php/en) languages

# Demo
 
For demo access use credentials:
```
Login: 15
Password:lPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8
Host: app.comet-server.ru
```

Example of connecting to [cometQL api](http://comet-server.org/doku.php/en:comet:cometql) from console using mysql-client:
```
mysql -h app.comet-server.ru -u15 -plPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8 -DCometQL_v1 --skip-ssl
```
## Simple chat demo

[Simple chat demo page](http://comet-server.com/doc/CometQL/MainPageChat/index.php)
![Simple chat demo gif](https://comet-server.com/doc/CometQL/CometQL-demo.gif)
On this gif animation you see how send message to [web chat](http://comet-server.com/doc/CometQL/MainPageChat/index.php) from CppComet api.
We connect to CppComet by mysql protocol and send [CometQL query](http://comet-server.org/doku.php/en:comet:cometql).
For connect to demo server use this command:
```
mysql -h app.comet-server.ru -u15 -plPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8 -DCometQL_v1 --skip-ssl
```
For send message to demo chat use this query:
```
INSERT INTO pipes_messages (name, event, message)VALUES("web_MainPageChat", "", '{ "text":"My text","name":"My name"}' );
```

# Documentation and examples

Comet server is written in C++, for storing data using MySQL.   

[Documentation and examples](http://comet-server.org/)
  
### Building from source

How to build and run [EN](http://comet-server.org/doku.php/en:comet:building-from-source) or [RU](http://comet-server.org/doku.php/comet:building-from-source)

```
apt-get update
apt-get install cmake make cpp gcc libssl-dev g++ nginx libmysqlclient-dev mysql-server mysql-client flex mailutils uuid-dev 
git clone https://github.com/Levhav/comet-server
cd comet-server
cmake .
make
```

Create a database in mysql based on [db.sql](https://github.com/Levhav/comet-server/blob/master/db.sql) file
In [comet.conf](https://github.com/Levhav/comet-server/blob/master/comet.conf) file, set the details to access the database

```
db_host 127.0.0.1 # The server address database
db_pw pass        # Password
db_user comet     # User
db_port 3306      # Port
db_name comet_db  # database name
```

In [comet.conf](https://github.com/Levhav/comet-server/blob/master/comet.conf) file, set the password to access the cometql api
```
password 0000000000000000000000000000000000000000000000000000000000000000 
```

## Launch

Running in daemon mode

```
sh ./comet-service.sh start
```

Run in console mode
```
./cpp_comet
```

### CometQL API: 

CometQL API documentation [EN](http://comet-server.org/doku.php/en:comet:cometql) or [RU](http://comet-server.org/doku.php/comet:cometql)

 - To connect to the comet server api from any server-side language is used CometQL [EN](http://comet-server.org/doku.php/en:comet:cometql) or [RU](http://comet-server.org/doku.php/comet:cometql)
 
### JavaScript API: 

JavaScript API documentation [EN](http://comet-server.org/doku.php/en:comet:javascript_api) or [RU](http://comet-server.org/doku.php/comet:javascript_api)

- To connect to the server from JavaScript API use file from api/CometServerApi.js

License
----

GNU AGPLv3

 
