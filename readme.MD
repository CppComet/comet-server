# comet-server
Комет сервер написанный на C++, для хранения данных использует MySQL. Полностью совместим с api SaaS комет сервиса https://comet-server.com и может быть использован как полноценная, бесплатная и независимая замена этого сервиса.

### Установка:
Рекомендуемые ОС ubuntu, debian, centos
```sh
apt-get update
apt-get install cmake make cpp gcc libssl-dev g++ nginx libmysqlclient-dev mysql-server mysql-client flex mailutils uuid-dev 
```

### Собираем из исходных кодов:
```sh
git clone https://github.com/Levhav/comet-server
cd comet-server
cmake .
make
```

### Настройки: 
  - Создайте базу в mysql на основе файла db.sql
  - В файле comet.conf укажите реквизиты для доступа к бд
```txt
db_host 127.0.0.1 # Адрес сервера с бд
db_pw pass        # Пароль
db_user comet     # Пользователь
db_port 3306      # Порт
db_name comet_db  # Имя базы данных
```
- Укажите пароль для доступа к api комет сервера
```txt
# пароль для доступа к api комет сервера
password 0000000000000000000000000000000000000000000000000000000000000000 
```
Остальные настройки из файла comet.conf можно не менять.

**Важно чтоб параметры из comet.conf учитывались они не должны иметь пробелов в начале строки.**
 
### Запуск: 
- Запуск в консольном режиме
```sh
./cpp_comet
```
- Запуск в режиме демона
```sh
sh ./comet-service.sh start
```
### Использование: 
Для подключения к api комет сервера из консоли используйте:
```sh
mysql -h127.0.0.1 -uroot -p0000000000000000000000000000000000000000000000000000000000000000 -DCometQL_v1 -P3307 --skip-ssl 
```
- Для подключения к api комет сервера из javascript используйте файл api/CometServerApi.js
- Для подключения к api комет сервера из любых других языков используйте реализацию клиета mysql для выбранного вами языка.
```php
// Например подключение из php
$link = mysqli_connect("127.0.0.1", "root", "0000000000000000000000000000000000000000000000000000000000000000", "CometQL_v1");
            mysqli_query (  $link, "INSERT INTO pipes_messages (name, event, message)VALUES('web_MainPageChat', '', '{ \"text\":\"My text\",\"name\":\"My name\"}' );" );
```

License
----

GPLv3

 
