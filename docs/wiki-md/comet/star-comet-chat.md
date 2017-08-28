
# Интегрируемый чат плагин

Готовый к встраиванию в ваш сайт чат плагин личной переписки между пользователями.

Исходные коды размещены на [GitHub](https://github.com/Levhav/Star.Comet-Chat), [Демо версия чата](http://comet-server.ru/doc/CometQL/Star.Comet-Chat/backend-example/index.php)


___
Чат  опубликован под лицензией Apache License 2.0
По всем вопросам обращайтесь в скайп Levhav или на почту support@comet-server.ru
___

# Функционал чата

  * Возможность передавать текстовые сообщения с прикреплённым изображением
  * Содержит возможность поиска контактов в списке контактов
  * Отдельные вкладки для списков избранных и заблокированных пользователей
  * Звуковое уведомление при получении сообщения
  * Кнопка пожаловаться
  * Пригоден для работы в мобильных браузерах
  * Отслеживание прочитано сообщение или нет. И как только сообщение будет прочитано получателем у отправителя появится на против сообщения две галочки
  * Перевод сообщений с помощью api яндекс переводчика

# Функционал админки чата

  * Real-time мониторинг всех сообщений пользователей
  * Просмотр диалога любых двух пользователей
  * Возможность просматривать все сообщения отдельно выбранного пользователя
  * Список жалоб на пользователей от других пользователей
  * Возможность просматривать только сообщения с вложениями

{{ :star-comet-chat:screenshot_21_.png?600 |}}

# Скриншоты чата

{{ :star-comet-chat:screenshot_4_.png?600  |Внешний вид чата}}
# Как встроить чат на сайт


___
Если вы решили использовать этот чат или его модификации то пожалуйста сообщите об этом на адрес star-chat@comet-server.ru в данный момент в статье не хватает ссылок на успешные примеры использования чата и отзывов на этот плагин чата.

Если возникнут трудности в интеграции то обращайтесь в техническую поддержку.

___


Чат может быть размещён как на отдельном сервере так и на том же сервере где и основной сайт. Чат взаимодействует с вашим сайтом через простое api.
У чата все данные хранятся в собственной бд и информацию о пользователях и данные для авторизации он получает путём отправки запросов к вашему сайту.

Для установки чата на своём сервере вам понадобится скачать все исходники от [сюда](https://github.com/Levhav/Star.Comet-Chat)

Среди исходников будет файл database.sql в нём содержится структура таблиц необходимая для работы чата. На основании этого файла создаём базу данных.

Затем надо подправить конфигурацию в файлах.
  * ./config.php
  * ./backend-example/config.php

В файле ./config.php надо указать настройки для чата для подключения к базе данных. Для хранения переписки. И надо задать настройки доступа к комет серверу для рассылки уведомлений.
<code php> 
/**
 * Доступ к БД
 */
$conf['mysql_db'] = "StarCometChat";
$conf['mysql_user'] = "StarCometChat";
$conf['mysql_pw'] = "RLUJ4TXE22XL5JTh";
$conf['mysql_host'] = "localhost"; 

/**
 * Доступ к комет серверу
 * Получить ключи доступа можно здесь https://comet-server.ru/menu_id/10
 */
$conf['cometQL_dev_id'] = "lPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8";
$conf['cometQL_key'] = 15;

```


Так же в файле ./config.php есть опция admin_ids которая позволяет перечислить идентификаторы всех пользователей у которых есть права администраторов. Наделять такими правами очень много пользователей не рекомендуется, так как это может повлиять на скорость работы чата.
<code php>
$conf['admin_ids'] = array(1, 2);    // Список id пользователей с правами администратора

```


Параметр trusted_ip в файле ./config.php должен содержать ip сервера с которого разрешены вызовы api чата.
<code php>
/**
 * ip адрес с которого разрешено вызывать api методы управления чатом
 * Или false если ограничение отключено (не безопасно)
 */
$conf['trusted_ip'] = false;

```


___
По умолчанию параметр trusted_ip равен false так как это пример. В реальном приложении необходимо указать ip адрес с которого разрешены запросы к api чата.
___
 

Переменная $trusted_ip в файле ./backend-example/config.php должна содержать ip адрес сервера на котором расположен чат.
<code php>
/**
 * ip адрес с которого разрешено вызывать api методы управления чатом
 * Или false если ограничение отключено (не безопасно)
 */
$trusted_ip = false;

```


___
По умолчанию переменная $trusted_ip равна false так как это пример. В реальном приложении необходимо указать ip адрес с которого разрешены запросы к api бекенда.
___
 
# Обмен данными

В директории backend-example расположены файлы с примером интеграции чата в php проект. Для того чтобы интегрировать чат в ваш проект вам необходимо создать обработчики на те запросы которые будет отправлять модуль чата для получения данных о зарегистрированных у вас пользователях.

# URL_getUsersInfo

Чат будет отправлять вашему проекту post запросы для получения информации о пользователях. Адрес на который будут отправляться эти запросы задаётся в файле ./config.php, имя параметра "URL_getUsersInfo"
Вот фрагмент кода в котором определено это значение.
<code php>
/**
 * URL для запроса информации о пользователях в json
 */
$conf['URL_getUsersInfo'] = 'http://comet-server.ru/doc/CometQL/Star.Comet-Chat/backend-example/chat_get_users.php'; 

```


В запросе в параметре users будет передан список id пользователей через запятую для которых необходимо вернуть данные.

```

POST /doc/CometQL/Star.Comet-Chat/backend-example/chat_get_users.php HTTP/1.1
Host: comet-server.ru
Origin: https://comet-server.ru

users=1,2

```

В ответ он будет ожидать json строку со следующими объектами.
<code JavaScript>
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

Файл ./backend-example/chat_get_users.php содержит в себе код который правильно обрабатывает запрос и отдаёт правильные данные.

# URL_getUsersHash

Для получения хеша авторизации пользователя чат будет отправлять post запрос на адрес который содержится в параметре URL_getUsersHash в файле ./config.php,
Вот фрагмент кода в котором определено это значение.
<code php>
/**
 * URL для запроса хеша авторизации
 */
$conf['URL_getUsersHash'] = 'http://comet-server.ru/doc/CometQL/Star.Comet-Chat/backend-example/chat_get_user_hash.php';

```


В запросе в параметре id будет передан id пользователя для которого необходимо получить хеш авторизации.

```

POST /doc/CometQL/Star.Comet-Chat/backend-example/chat_get_user_hash.php HTTP/1.1
Host: comet-server.ru
Origin: https://comet-server.ru

id=1

```

В ответ он будет ожидать строку хеша авторизации
<code JavaScript>56ff3f23bfd1071e14749aad42e58d89
```

Файл ./backend-example/chat_get_user_hash.php содержит в себе код который правильно обрабатывает запрос и отдаёт правильные данные.


# JavaScript вызовы чата

Файл ./backend-example/userPage.php представляет из себя пример страницы пользователя. В нём есть пример интеграции JavaScript вызовов чата в страницы сайта.

Вот приведён код запуска чат плагина.
<code JavaScript>
var user_id = <?php echo $_SESSION['userId']; ?>;  
var user_key = "<?php echo getUserHash($_SESSION['userId']); ?>";  
        
$(document).ready(function()
{ 
    /**
     * Подключение к комет серверу. Для возможности принимать команды.
     * dev_id ваш публичный идентификатор разработчика
     * user_id идентификатор пользователя под которым вы вошли.
     * user_key ваш хеш авторизации.
     */
    CometServer().start({dev_id:15, user_id:user_id, user_key: user_key})
     
    /**
     * Инициализируем модуль чата, происходит инициализация и загрузка данных необходимых для работы.
     * Но окно чата этим вызовом не открывается.
     */
    StarCometChat.init({
        user_id: user_id,
        user_key: user_key, 
        open:false,
        
        // Параметр home_dir содержит адрес расположения php скриптов чата
        home_dir: "http://comet-server.ru/doc/CometQL/Star.Comet-Chat",
        
        // Функция назначенная в success вызывается после успешной инициализации чата.
        success:function()
        {
            // Вызов countNewMessagesSum возвращает количество новых сообщений. Работает корректно только после завершения инициализации чата
            var c = StarCometChat.countNewMessagesSum();
            if(c > 0)
            {
                $('#newMsgIndicator').html("У вас "+ c + " новых сообщений");
            }
        }
    });
});

```


Надеюсь комментарии в коде понятны. Следует отметить что для человек получает статус online только после того как выполняется код подключения к комет серверу.

# После завершения инициализации доступны следующие вызовы:
Возвращает кол-во непрочитанных сообщений
<code JavaScript>StarCometChat.countNewMessagesSum()
```

Возвращает кол-во непрочитанных сообщений от пользователя user_id
<code JavaScript>StarCometChat.countNewMessages(user_id)
```

Открывает список диалогов
<code JavaScript>StarCometChat.openDialog()
```

Открывает диалог с пользователем user_id
<code JavaScript>StarCometChat.openDialog(user_id)
```



# Решение проблем

Для включения вывода ошибок надо добавить в файл config.php код:
<code php>
ini_set('display_errors','on');
error_reporting (E_ALL & ~E_NOTICE);

```


Ошибка о том что указаны не верные пути к файлам чата выглядит как то так:
<code php>Error in line 14 include_once $_SERVER['DOCUMENT_ROOT'].'/config.php';
```



Ошибка при загрузке файла похожая на эту

```

Warning: move_uploaded_file(/home/www-data/web/comet-server.com/chatFiles/3_2_1488678883243.png): failed to open stream: No such file or directory in /home/www-data/web/administrator/sub_sites/comet.cms-machaon.ru/doc/CometQL/Star.Comet-Chat/sendMessage.php on line 62

Warning: move_uploaded_file(): Unable to move '/tmp/php27nQco' to '/home/www-data/web/comet-server.com/chatFiles/3_2_1488678883243.png' in /home/www-data/web/administrator/sub_sites/comet.cms-machaon.ru/doc/CometQL/Star.Comet-Chat/sendMessage.php on line 62

```

говорит о том что нет папки chatFiles или нет прав на запись в неё.

== Как попасть в админку? ==

Список пользователей с правами администратора задаётся в файле config.php в функции getAdminIds()
{{ :star-comet-chat:снимок_экрана_от_2017-03-06_14-39-30.png |}}