
# Установка

Рекомендуемые ОС ubuntu, debian, centos
<code bash>
apt-get update
apt-get install cmake make cpp gcc libssl-dev g++ nginx libmysqlclient-dev mysql-server mysql-client flex mailutils uuid-dev 

```


# Собираем из исходных кодов
<code bash>
git clone https://github.com/Levhav/comet-server
cd comet-server
cmake .
make

```


# Настройки

CppComet использует базу данных mysql для хранения данных пользователей для авторизации на сервере. И хранения времени, когда пользователь был в сети. И для хранения временных данных, таких как не доставленные сообщения и другие данные.

  * Создайте базу в mysql на основе файла [db.sql](https://github.com/Levhav/comet-server/blob/master/db.sql)
  * В файле [comet.ini](https://github.com/CppComet/comet-server/blob/master/comet.ini) укажите реквизиты для доступа к бд в секции [db]
<code ini>
[db]
host = localhost
user = root
password = root
name = comet_db
port = 3305

```

Укажите пароль для доступа к api комет сервера
<code ini>
[main] 
; пароль для доступа к api комет сервера
password = 0000000000000000000000000000000000000000000000000000000000000000

```

Остальные настройки из файла [comet.ini](https://github.com/CppComet/comet-server/blob/master/comet.ini) можно не менять.


___
Весь перечень настроек в статье [Настройка файла comet.ini](comet:ini-file) 
___

  
# Запуск
Запуск в консольном режиме
<code sh>
./cpp_comet

```

Запуск в режиме демона
<code sh>
systemctl start comet.service

```


# Добавление в автозагрузку


<code sh>cp ./comet.service /etc/systemd/system
systemctl daemon-reload
systemctl enable comet.service
```

# Настройка nginx в качестве реверс прокси

Для того чтоб настроить работу комет сервера на одной машине с другим сервером. Или просто иметь возможность работы не только по http но и по https надо настроить реверс прокси.

Ниже приведён пример конфигурации nginx для проксирования трафика до комет сервера с /comet-server на комет сервер запущенный на порту 82 и всего остального трафика на веб сервер запущенном на порту 8080
<file text default>
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
</file>

# Возможные проблемы после установки
 
Обратите внимание на то какие значения параметра port указаны в секциях [ws] и [cometql] именно на этих  портах комет сервер будет ожидать входящие подключения.

В шаблоне comet.ini поставляемом в репозитории параметр port  для подключений из JavaScrip api выставлен в 8087
Это значит что подключатся надо так:
<code JavaScript>cometApi.start({user_id:1, user_key:"userHash", node:"example.ru:8087"})
```


Параметр port  для подключений из CometQL выставлен в 3300
Это значит что подключатся надо так:
<code PHP>$link = mysqli_connect("example.ru", "root", "", "CometQL_v1", 3300);
```


Так же обратите внимание на то что надо указывать имя базы данных CometQL_v1, оно символизирует версию апи и вероятно потом выйдет версия CometQL_v2 и можно будет при подключении выбирать версию для работы.








