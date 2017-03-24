![CppComet](https://comet-server.com/img/CppComet.png)

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

CometQL API documentation [EN](http://comet-server.org/doku.php/comet:cometql) or [RU](http://comet-server.org/doku.php/en:comet:cometql)

 - To connect to the comet server api from any server-side language is used CometQL [EN](http://comet-server.org/doku.php/comet:cometql) or [RU](http://comet-server.org/doku.php/en:comet:cometql)
 
### JavaScript API: 

JavaScript API documentation [EN](http://comet-server.org/doku.php/en:comet:javascript_api) or [RU](http://comet-server.org/doku.php/comet:javascript_api)

- To connect to the server from JavaScript API use file from api/CometServerApi.js

License
----

GNU AGPLv3

 
