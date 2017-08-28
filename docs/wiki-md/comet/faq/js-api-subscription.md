
# Как принять сообщение из канала в JavaScript?



```

<!DOCTYPE HTML>
<html>
<head>
    <!-- Подключаем библиотеки -->
    <script src="//comet-server.ru/CometServerApi.js" type="text/javascript"></script>
    <script src="jquery.min.js" type="text/javascript"></script>
</head>
<body>

<!-- Блок в который мы добавим полученное сообщение -->
<div id="textHolder" style="margin-top:10px; border:1px solid #000;padding:10px;">
    Текст
</div>
    
<script type="text/javascript">

$(document).ready(function(){

    /** 
     * Подписываемся на получение сообщения из канала Pipe_name
     */
    CometServer().subscription("Pipe_name", function(event){
        console.log("Мы получили сообщение из канала Pipe_name",  event.data.text, event);
        $("#textHolder").html( $("#textHolder").html() +"<hr>"+event.data.text);
    })

    /** 
     * Подключение к комет серверу. Для возможности принимать команды.
     * dev_id ваш публичный идентифиукатор разработчика
     */
    CometServer().start({dev_id:15 })
})

</script>
</body>
</html>

```


# Online demo получение сообщения из канала на JavaScript
Для того что бы отправить сообщение в канал выполните следующий запрос

```

INSERT INTO pipes_messages (name, event, message)VALUES("Pipe_name", "event_in_pipe", '{"text":"message"}');

```

<html>
<iframe src="http://comet-server.com/doc/example/3" width="910px"></iframe>
</html>