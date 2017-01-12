# comet-server


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
 
License
----

GPLv3

 