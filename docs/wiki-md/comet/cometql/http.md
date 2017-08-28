
# Отправка CometQL запросов по http/https

Есть случаи когда нет возможности использовать протокол MySQL для соединения с комет сервером. Для таких случаев можно отправлять команды комет серверу по HTTP/HTTPS. 


___
Это не рекомендованный способ взаимодействия с сервером. Он работает медленнее чем отправка CometQL запросов на прямую.
___


___
Метод работает в тестовом режиме. О неполадках сообщайте в техническую поддержку.
___



___
Метод работает в тестовом режиме. И поэтому интерфейс или адрес для приёма запросов может быть изменён.

В низу статьи приведён исходный код php файла. Вы можете его скачать и поставить на свой хостинг чтобы обезопасить себя от изменений в интерфейсе работы api по HTTP. На время пока работа через HTTP запросы не выйдет из статуса бета тестирования, если оно вообще выйдет из этого статуса.

___


Для отправки запроса


```

http://comet-server.ru/cometapi.php?query=show+tables&id=15&key=lPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8

```


  * query это CometQL запрос
  * id это [идентификатор разработчика](comet/dev_id.md)
  * key это [секретный ключ разработчика](comet/dev_id.md)

Ответ придёт в json формате. В случаи успеха ответ будет таким

```
{"data":[{"Tables":"users_auth"},{"Tables":"users_time"},{"Tables":"users_messages"},{"Tables":"pipes_messages"},{"Tables":"users_in_pipes"},{"Tables":"pipes"},{"Tables":"pipes_settings"}]}
```


В случаи ошибки ответ будет таким

```
{"error":2,"text":"Syntax error in query","see":"https:\/\/comet-server.ru\/wiki\/doku.php\/comet:cometql:error"}
```

# Примечание

Ниже приведён исходный код файла выполняющего роль промежуточного звена которое принимает запросы по HTTP и передаёт их дальше по протоколу mysql. Вы можете его скачать и поставить на свой хостинг чтобы обезопасить себя от изменений в интерфейсе работы HTTP api. 


```

<?php

header('Content-Type: application/json; charset=utf-8');
header("Access-Control-Allow-Origin: *");

ini_set('display_errors','on');
error_reporting (E_ALL);


function getParam($name)
{
    if(isset($_GET[$name]))
    {
        return trim(urldecode($_GET[$name]));
    }
    else if(isset($_POST[$name]))
    {
        return trim(urldecode($_POST[$name]));
    }

    return NULL;
}

$q       = getParam("query");
$dev_id  = getParam("id");
$dev_key = getParam("key");
if($q == NULL || $dev_id == NULL || $dev_key == NULL)
{
    header('HTTP/1.1 400 Bad Request');
    die(json_encode(array("error" => "Bad Request, query, id or key is empty")));
}

$q = trim(urldecode($q));
$link = mysqli_connect("app.comet-server.ru", $dev_id, $dev_key, "CometQL_v1");


if ( !$link )
{
    header('HTTP/1.1 403 Bad Request');
    die(json_encode(array("error" => "Невозможно подключение к CometQL")));
}

$result = mysqli_query (  $link, $q );
if(mysqli_errno($link) != 0)
{
    header('HTTP/1.1 400 Bad Request');
    die(json_encode(array("error" => mysqli_errno($link), "text" => mysqli_error($link), "see" => "https://comet-server.ru/wiki/doku.php/comet:cometql:error")));
}

$data = array();
while ($row = mysqli_fetch_assoc($result))
{
    $data[] = $row;
}

header('HTTP/1.1 200 OK');
die(json_encode(array("data" => $data)));

```
