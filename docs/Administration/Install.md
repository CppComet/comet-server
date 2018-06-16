
# Install

Recommended OS ubuntu, debian, centos

```

apt-get update
apt-get install cmake make cpp gcc libssl-dev g++ nginx libmysqlclient-dev mysql-server mysql-client flex mailutils uuid-dev 

```


# Building it from source code

```

git clone https://github.com/Levhav/comet-server
cd comet-server
cmake .
make

```


# Settings
CppComet use mysql database for storage users credentials for authorization on server. And to store the time when the user was on the online. And for storing temporary data, such as undelivered messages and other data.
 
  * Create a database in mysql based on [db.sql](https://github.com/Levhav/comet-server/blob/master/db.sql) file
  * In [comet.ini](https://github.com/CppComet/comet-server/blob/master/comet.ini) file, set the details to access the database

```

[db]
host = localhost   # The server address database
user = root        # User
password = root    # Password
name = comet_db    # database name
port = 3305        # Port

```

Enter the password to access the comet-server api

```

[main]  
password = 0000000000000000000000000000000000000000000000000000000000000000

```

 
# Add to Startup
 

```
cp ./comet.service /etc/systemd/system
systemctl daemon-reload
systemctl enable comet.service
```


After successes run server we can begin create chat. If you get error, you should create issue in [github repository](https://github.com/Levhav/comet-server/issues).

# Launch
Run in console mode

```

./cpp_comet

```

Running in daemon mode

```

systemctl start comet.service

```


# Configuring nginx as a reverse proxy

In order to configure the operation of comets on one machine with a web server, or just have the ability to work not only on http but also on https, you need to configure the reverse proxy.

The following is an example of the nginx configuration for proxy traffic to comet servers with /comet-server to the comet server running on port 82 and all other traffic to the web server running on port 8080


```

server {
	listen 0.0.0.0:80;   
	server_name comet-server.com;
 
	location / {
		proxy_pass http://127.0.0.1:8080;
		proxy_set_header Host $host;
		proxy_set_header X-Real-IP $remote_addr;
		proxy_set_header X-Forwarded-For $remote_addr;
		proxy_connect_timeout 120;
		proxy_send_timeout 120;
		proxy_read_timeout 180;
	}

	keepalive_disable none;
	lingering_close always;
	send_timeout 3600s;

	location /comet-server {
        proxy_pass http://127.0.0.1:82;
        
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header Host $http_host;
        proxy_set_header X-NginX-Proxy true;
        
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
        
        proxy_redirect off;
        keepalive_timeout 900;
        proxy_read_timeout 900;
	} 
}

# HTTPS server


server {
	listen 0.0.0.0:443;  
	server_name comet-server.com;

	ssl on;
	ssl_certificate /etc/letsencrypt/live/comet-server.com/fullchain.pem;
	ssl_certificate_key /etc/letsencrypt/live/comet-server.com/privkey.pem;
	 
	ssl_session_timeout 70m;

	ssl_protocols SSLv3 TLSv1;
	ssl_ciphers ALL:!ADH:!EXPORT56:RC4+RSA:+HIGH:+MEDIUM:+LOW:+SSLv3:+EXP;
	ssl_prefer_server_ciphers on;

	keepalive_disable none;
	lingering_close always;
	send_timeout 3600s;
 
	location / {
		proxy_pass http://127.0.0.1:8080;
		proxy_set_header Host $host;
		proxy_set_header X-Real-IP $remote_addr;
		proxy_set_header X-Forwarded-For $remote_addr;
		proxy_connect_timeout 120;
		proxy_send_timeout 120;
		proxy_read_timeout 180;
		 
	}
	 
	location /comet-server {
        proxy_pass http://127.0.0.1:82;
        
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header Host $http_host;
        proxy_set_header X-NginX-Proxy true;
        
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
        
        proxy_redirect off; 
        keepalive_timeout 900;
        proxy_read_timeout 900;
	}
}

```


# Possible problems after installation
 
Pay attention to what values of the port parameter are specified in the sections [ws] and [cometql] on these ports the comet server will wait for incoming connections.

In the comet.ini example in the repository, the port parameter for connections from JavaScrip api is set to 8087.
This means that you need to connect like this:

```
cometApi.start({user_id:1, user_key:"userHash", node:"example.ru:8087"})
```


The port parameter for connections from CometQL is set to 3300
This means that you need to connect like this:

```
$link = mysqli_connect("example.ru", "root", "", "CometQL_v1", 3300);
```


# Unreadable file comet.ini

Some text editors add unprintable simbols to the utf8 file. After that, the file may look the same but the comet server will not parse it correctly. You can try using another editor or delete the old file and create a new one with the same text.

# Error while assembling in CentOS


```

CMake Error: The following variables are used in this project, but they are set to NOTFOUND.
Please set them or make sure they are set and tested correctly in the CMake files:
MYSQL_INCLUDE_DIR (ADVANCED)

```


Can not find the client's mysql header files. You can try installing the mysql-devel package


```

yum install mysql-devel

```


# Error while assembling in CentOS


```

/etc/comet-server/comet-server/src/mystring.cpp:15:51: fatal error: uuid / uuid.h: No such file or directory exists
 #include <uuid/uuid.h>

```


Can not find the file uuid.h. You can try installing the libuuid libuuid-devel package


```

yum install libuuid libuuid-devel

```
