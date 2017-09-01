
# Пример Realtime чата

В этой статье будет рассмотрен пример простого чата между пользователями. Для реализации чата не потребуется использование серверной стороны, в этой статье всё только на JavaScript. Живой пример работы можно наблюдать в конце статьи.


___
В данной статье рассматривается вопрос создания чата где все пользователи общаются в одной общей комнате. Если вам нужен чат для личного общения пользователей между собой (общение в диалогах один на один) то вам больше подойдёт [чат плагин личной переписки между пользователями](/docs/comet/star-comet-chat.md)
___

# Принцип организации чата

  * Все сообщения от всех пользователей отправляются в общий канал web_chat_pipe.
  * Структура сообщения содержит в себе текст сообщения и имя отправившего это сообщение.
  * Все пользователи участвующие в чате подписаны на канал чата. И полученные из него сообщения отображаются в ленте общения.
  * В момент отправки мы сохраняем текущее время в переменную timer, а в момент получения отчёта о доставке сообщения определяем сколько было затрачено времени.

# Весь код примера:
Полностью исходный код чата web_chat.js

```

var timer = new Date();
function web_send_msg()
{
    // Получение значений из элементов ввода.
    var text = $("#WebChatTextID").val();
    var name = $("#WebChatNameID").val();
    
    // Очистка формы
    $("#WebChatTextID").val("");  
    
    // Зпишем время в момент отправки сообщения
    timer = new Date();
    
    // Добавление отправленного сообщения к списку сообщений.
    //$("#WebChatFormForm").append("<p><b>"+HtmlEncode(name)+": </b>"+HtmlEncode(text)+"</p>");
    
    // Отправка сообщения в канал чата
    CometServer().web_pipe_send("web_chat_pipe", {"text":text, "name":name});
    
    // Уведомим остальные вкладки о том что мы добавили сообщение в чат
    comet_server_signal().send_emit("AddToChat", {"text":text, "name":name})
}

   
// Функция выполнится в после загрузки страницы
$(document).ready(function()
{
    // Создание формы для чата. Вёрстка.
    var html =  "<div style=\"border: 1px solid #ccc;padding:10px;\" >"
	          + "<div id=\"WebChatFormForm\" style=\"max-width: 600px;overflow: auto;max-height: 900px;\"></div>"
		  + "<input type=\"text\" id=\"WebChatNameID\" style=\"margin-top:10px;\" placeholder=\"Укажите ваше имя...\" > <div id=\"answer_div\" style=\"float:right;\" ></div>"
	          + "<textarea id = \"WebChatTextID\" placeholder = \"Отправьте сообщение в online чат...\" style=\"max-width: 600px;max-height: 100px;width: 600px;margin-top:10px;\" ></textarea>"

                  + "<div style=\"margin-bottom: 0px;margin-top: 10px;\">"
                  +    "<input type=\"button\" style=\"width: 220px;\" onclick=\"web_send_msg();\" value=\"Отправить\" >"
                  +    " <div id=\"answer_error\"  style=\"float:right;\" ></div>"
                  + "</div>"
             +  "</div>";
    $("#web_chat_holder").html(html);

    // Подписываемся на канал в который и будут отпавлятся сообщения чата. 
    CometServer().subscription("web_chat_pipe", function(msg){
        console.log(["msg", msg]);
        // Добавление полученого сообщения к списку сообщений.
        $("#WebChatFormForm").append("<p><b>"+HtmlEncode(msg.data.name)+": </b>"+HtmlEncode(msg.data.text)+"</p>");
    });

    // Подписываемся на событие добавления сообщения в чат нами, для того чтобы если чат открыт в нескольких вкладках
    // наше сообщение добавленое на одной вкладке отобразилось на всех остальных без перезагрузки страницы
    comet_server_signal().connect("AddToChat", function(msg){
        console.log(["msg", msg]);
        // Добавление полученого сообщения к списку сообщений.
        $("#WebChatFormForm").append("<p><b>"+HtmlEncode(msg.name)+": </b>"+HtmlEncode(msg.text)+"</p>");
    });
    
    // Подписываемся на канал в который и будут отпавлятся уведомления о доставке отправленых сообщений.
    CometServer().subscription("#web_chat_pipe", function(p)
    {
        // Зпишем время в момент получения отчёта о доставке сообщения
        var etime = new Date();
        
        console.log(["answer_to_web_chat_pipe", p]);
        $("#answer_div").html("Сообщение доставлено "+p.data.number_messages+" получателям за "+ (etime.getTime() - timer.getTime() )+"ms");
        $("#answer_error").html(" "+p.data.error);
    });

   // Загружаем историю сообщений
   CometServer().get_pipe_log("web_chat_pipe");
});


function HtmlEncode(s)
{
  var el = document.createElement("div");
  el.innerText = el.textContent = s;
  s = el.innerHTML;
  return s;
}

```

Следует отметить что сообщения от правленые в канал приходят всем кроме того к отправил это сообщение, ему приходит отчёт о доставке либо уведомление об ошибке.

Отчёт о доставке содержит количество человек получивших сообщение. То есть если на канал было подписано два человека и один из них отправил сообщение в этот канал то в отчёте количество получателей будет 1. А если на канал был подписан только один человек и он же отправил сообщение в этот канал в отчёте будет указан 0.

<html>

</html>

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
 

# Примечания


___
Если вы захотите указать другое имя канала для работы чата то учтите что из Javascript api можно отправлять сообщения только в каналы начинающиеся с web_ Более подробно об этом смотрите в  [JavaScript API - зарезервированные_имена_каналов](/docs/RU/API/JavaScript%20API.md)
___
