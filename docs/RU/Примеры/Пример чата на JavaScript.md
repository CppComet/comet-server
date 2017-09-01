
# Готовый чат на JavaScript для сайта - chat плагин

Реализовать чат на html без серверной стороны конечно не возможно. Но не обязательно для каждого чата писать chat server самому, можно воспользоваться готовым решением. И не смотря на то что это казалось бы очевидно набирается почти 800 запросов "html chat" в месяц и это только к Яндексу.

В конце статьи прилагается файл на javascript реализующий простой чат, его достаточно вставить в html код страницы, и чат готов. Вся серверная сторона уже реализована.

# Готовый движок чата на JavaScript для любого сайта.

Как говорилось выше реализовать html чат без серверной части работать не будет. Для работы чата требуется комет сервер который будет отправлять push уведомления всем участникам чата. Но для этого чата комет сервер предоставляется бесплатно, вам даже регистрироваться там не обязательно. В таком случаи чат будет общий для всех сайтов которые его вставят. Это даже не плохо, особенно для сайтов с небольшой посещаемостью. А те кто хотят иметь чат только для своего сайта или своей группы сайтов должны [зарегистрироваться](https://comet-server.ru/%D1%80%D0%B0%D0%B7%D0%B4%D0%B5%D0%BB/10) и бесплатно получить [идентификатор разработчика](/docs/RU/API/%D0%9F%D0%B0%D1%80%D0%B0%D0%BC%D0%B5%D1%82%D1%80%D1%8B%20%D0%B4%D0%BB%D1%8F%20%D0%BF%D0%BE%D0%B4%D0%BA%D0%BB%D1%8E%D1%87%D0%B5%D0%BD%D0%B8%D1%8F.md) на comet-server.ru а затем его указать в коде инициализации чата ( вместо dev_id: 15 указать свой полученный id).

# Встраиваем чат в html вашего сайта.

В HTML код вставляем вот такой скрипт.

```

<script type="text/javascript" src="https://ajax.googleapis.com/ajax/libs/jquery/1/jquery.min.js" ></script>
<script type="text/javascript" src="https://comet-server.ru/CometServerApi.js" ></script>
<script type="text/javascript" src="https://comet-server.ru/doc/html_chat.js" ></script>
<link rel="stylesheet" type="text/css" href="https://comet-server.ru/doc/html_chat.css"></link>

<!-- Осталось настроить сам чат и запустить, для этого пишем небольшой скрипт. -->
<div id="html-chat"></div>
<style>
/* Здесь настроим css стили для чата*/
.holder-html-chat{ border: 1px solid #ccc;padding:10px;background-color: #fff;width: 600px;}
.html-chat-history{ max-width: 600px; overflow: auto;max-height: 900px; border: 1px solid #ccc;padding: 5px;}
.html-chat-js-name{ margin-top:10px; }
.html-chat-js-input{ max-width: 600px;max-height: 100px;width: 600px;margin-top:10px; }
.html-chat-js-button-holder{ margin-bottom: 0px;margin-top: 10px; }
.html-chat-js-button-holder input{ width: 220px; }
.html-chat-js-answer{ float:right; }
.html-chat-js-answer a{ color: #777;font-size: 12px; font-family: cursive;}
.html-chat-js-answer a:hover{ color: #338;font-size: 12px; font-family: cursive;}
.html-chat-msg{ margin: 0px; }
</style>

<script>

   /**
    * Чат работает на comet-server.ru
    * По любым вопросам обращайтесь support@comet-server.ru или на сайт comet-server.ru
    */
    $(document).ready(function()
    {
       // Запуск api комет сервера
       CometServer().start({dev_id: 15 }) // Идентификатор разработчика на comet-server.ru

       // Запуск чата. Передаём ему элемент в котором надо создать окно чата.
       htmljs_Chat_Init( $("#html-chat") )
    });
</script>

```


<html>

</html> 


___
Если вы хотите вставить этот чат себе на сайт но не "как есть", а с некоторыми изменениями то вероятно вам будет полезна [статья-пример чата похожего на этот чат](/docs/RU/%D0%9F%D1%80%D0%B8%D0%BC%D0%B5%D1%80%D1%8B/%D0%9F%D1%80%D0%B8%D0%BC%D0%B5%D1%80%20Realtime%20%D1%87%D0%B0%D1%82%D0%B0.md)
___
 

# История сообщений для чата

Можно до 99 последних сообщений чата загружать из истории комет сервера.
Для включения сохранения истории сообщений в канале чата надо выполнить CometQL запрос в online командной строке для CometQL на странице в разделе Real time monitor.

```

INSERT INTO pipes_settings ('name', 'length') VALUES ('web_chat_pipe', 10);

```


# Очищение истории сообщений чата

Для очищения истории сообщений в чате надо выполнить CometQL запрос в online командной строке для CometQL на странице в разделе Real time monitor.

```

DELETE FROM pipes_messages WHERE name = 'web_chat_pipe';

```



___
Обратите внимание на разницу в командной строке CometQL: на страницах с документацией демо доступ для dev_id = 15 а в разделе Real time monitor командная строка для управления вашим личным dev_id
___
 

# Другие реализации чатов

В данной статье рассмотрели публичный чат без регистрации.

  * Готовый [виджет чата для социальной сети](https://chatlab.pro/)
  * Если вам нужен чат для личного общения пользователей между собой (общение в диалогах один на один) то вам больше подойдёт [чат плагин личной переписки между пользователями](/docs/comet/star-comet-chat.md)
  * Есть так же учебный пример [скрипта чата с авторизацией](/docs/RU/%D0%9F%D1%80%D0%B8%D0%BC%D0%B5%D1%80%D1%8B/%D0%9A%D0%B0%D0%BA%20%D1%81%D0%B4%D0%B5%D0%BB%D0%B0%D1%82%D1%8C%20%D1%87%D0%B0%D1%82%20%D1%81%20%D0%B0%D0%B2%D1%82%D0%BE%D1%80%D0%B8%D0%B7%D0%B0%D1%86%D0%B8%D0%B5%D0%B9.md)
  * Ещё один простой пример [чата похожего на этот чат](/docs/RU/%D0%9F%D1%80%D0%B8%D0%BC%D0%B5%D1%80%D1%8B/%D0%9F%D1%80%D0%B8%D0%BC%D0%B5%D1%80%20Realtime%20%D1%87%D0%B0%D1%82%D0%B0.md)


___
Если вы не собираетесь сами разрабатывать уникальный чат с каким то необычным функционалом а хотите просто вставить на свой сайт готовый виджет чата то рекомендуем вам ознакомится с [сервисом чатов](https://chatlab.pro/) позволяющим всё сделать без программирования.
___
