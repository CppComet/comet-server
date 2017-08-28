
# Как сделать чат
В данной статье рассмотрим как сделать чат с авторизацией на php с использованием комет сервера. Наш php chat будет работать по технологии websocket. Для связи php с websocket сервером надо использовать php comet api.

Первый пример это html скрипт чата для сайта, он будет работать без регистрации пользователей и легко вставится в любую html страницу. Мы его рассмотрим первым по тому что это наиболее простой пример чата использующего комет сервер, а затем на его основе напишем чат для сайта на php с авторизацией пользователей в чате и уведомлениями о входе и выходе пользователей.


___
В данной статье рассматривается вопрос создания чата где все пользователи общаются в одной общей комнате. Если вам нужен чат для личного общения пользователей между собой (общение в диалогах один на один) то вам больше подойдёт [чат плагин личной переписки между пользователями](comet:star-comet-chat)
___

# Пример создания простейшего чата на php

Первое в чём следует разобраться что такое каналы (pipe) в комет сервере. Канал это именованный поток сообщений, в который можно самому отправлять сообщения и можно подписаться на те сообщения которые туда отправляются кем то другим.

Если рассматривать наиболее простой скрипт чата для сайта то нам понадобится всего один канал. Все участники подписываются на сообщения из этого канала, а для добавления сообщения в чат просто отправляем сообщение в канал.

Вот принцип работы простого чата для сайта
  - Все сообщения от всех пользователей отправляются в общий канал web_chat_pipe.
  - Структура сообщения содержит в себе текст сообщения и имя отправившего это сообщение.
  - Все пользователи участвующие в чате подписаны на канал чата. И полученные из него сообщения отображаются в ленте общения.
**Примечание:** В примерах будет использована библиотека JQuery.

Сначала разберём как работает простой скрипт онлайн чата. Приведу сразу весь код а потом разберём его подробно.
Первое что надо отметить это то что в данном чате люди общаются используя только лишь комет сервер. Это имеет ряд недостатков делающий из чата скорее прощёную учебную модель.

Недостатки:
  - Нет возможности забанить человека
  - Нет авторизации, это позволяет любому представится кем угодно.
Достоинства:
  - Просто в реализации, фактически это javascript чат (php не используется)
  - Прост поцес установки чата на сайт
<code JavaScript chat.html>
<!DOCTYPE HTML>
<head>
<title>Comet Server - Пример php чата</title>
<script language="JavaScript" type="text/javascript" src="https://comet-server.ru/template/Profitable_site/js/jquery-2.0.3.min.js" ></script>
<script language="JavaScript" type="text/javascript" src="https://comet-server.ru/CometServerApi.js" ></script>
</head>
<body>
    <div id="WebChatFormForm" ></div> 
    <input type="text" id="WebChatNameID" placeholder="Укажите ваше имя..." > <br>
    <textarea id = "WebChatTextID" placeholder = "Отправьте сообщение в online чат..." ></textarea><br>
    
    <input type="button" onclick="web_send_msg();" value="Отправить" >
    <div id="answer_div" ></div>
    
    <script>
         // Отправляет сообщение в чат
         function web_send_msg()
         {
             // Получение значений из элементов ввода.
             var text = $("#WebChatTextID").val(); // Получаем текст сообщения
             var name = $("#WebChatNameID").val(); // Получаем имя пользователя
              
             // Очистка поля с текстом сообщения
             $("#WebChatTextID").val("");  
             
             // Добавление отправленного сообщения к списку сообщений.
             $("#WebChatFormForm").append("<p><b>"+name+": </b>"+text+"</p>");
             
             // Отправка сообщения в канал чата
             CometServer().web_pipe_send("web_chat_pipe", {"text":text, "name":name});
         }
         
            
         // Функция выполнится после загрузки страницы
         $(document).ready(function()
         {
             CometServer().start({dev_id:1}) // Подключаемся к комет серверу
             
             // Подписываемся на канал в который и будут отправляться сообщения чата.
             CometServer().subscription("web_chat_pipe", function(msg)
             {
                console.log(msg)
                 // Добавление полученного сообщения к списку сообщений.
                 $("#WebChatFormForm").append("<p><b>"+msg.data.name+": </b>"+msg.data.text+"</p>");
             });
         
             // Подписываемся на канал в который и будут отправляться уведомления о доставке отправленных сообщений.
             CometServer().subscription("#web_chat_pipe", function(p)
             {
                console.log(p)
                 $("#answer_div").html("Сообщение доставлено "+p.data.number_messages+" получателям. "+p.data.error);
             });
         });
    </script>
</body>
</html>

```


# Online demo

[Online demo](https://comet-server.com/doc/example/4)

Ну вот теперь по кусочкам рассмотрим работу скрипта онлайн чата:

Функция web_send_msg отправляет сообщение в чат.
<code JavaScript>
     function web_send_msg()
     {
         // Получение значений из html элементов ввода.
         var text = $("#WebChatTextID").val(); // Получаем текст сообщения
         var name = $("#WebChatNameID").val(); // Получаем имя пользователя
          
         // Очистка поля с текстом сообщения
         $("#WebChatTextID").val("");  
         
         // Добавление отправленного сообщения в свой список сообщений.
         $("#WebChatFormForm").append(">p<>b<"+name+": >/b<"+text+">/p<");
         
         // Отправка сообщения в канал чата.
         CometServer().web_pipe_send("web_chat_pipe", {"text":text, "name":name});
     }

```

Здесь можно уточнить что строка: $("#WebChatFormForm").append("<p><b>"+name+": </b>"+text+"</p>"); добавляет напечатанное сообщение в нашу чат ленту, так чтоб потом было удобно перечитывать сообщения.

А вот вызов CometServer().web_pipe_send("web_chat_pipe", {"text":text, "name":name}); производит непосредственно отправку нашего сообщения всем остальным участникам онлайн чата. Первый аргумент это имя канала в который мы отправим сообщение. Второй аргумент это само сообщение, содержащие имя отправителя и текст сообщения.


___
Обратите внимание что имя канала (web_chat_pipe) начинается с префикса "web_". Во все каналы имя которых начинается с префикса "web_" сообщение может отправить любой кто знает имя канала прямо из JavaScript. В ряде случаев это может позволить значительно разгрузить ваш собственный сервер.
___


# Разберём вторую половину скрипта чата:

Следующий код выполняется один раз при загрузке страницы. В нём мы подписываемся на канал для получения сообщений из чата и назначаем функцию обработчик для получения отчёта о доставке сообщений.

<code JavaScript>
$(document).ready(function()
 {
     CometServer().start({dev_id:1}) // Подключаемся к комет серверу
     
     // Подписываемся на канал в который и будут отправляться сообщения чата.
     CometServer().subscription("web_chat_pipe", function(msg)
     {
         // Добавление полученного сообщения к списку сообщений.
         $("#WebChatFormForm").append("<p><b>"+msg.data.name+": </b>"+msg.data.text+"</p>");
     });
 
     // Подписываемся на канал в который и будут отправляться уведомления о доставке отправленных сообщений.
     CometServer().subscription("#web_chat_pipe", function(p)
     {
         $("#answer_div").html("Сообщение доставлено "+p.number_messages+" получателям. "+p.error);
     });
 });

```


Здесь вызов <code JavaScript>CometServer().start({dev_id:1})
```
 выполняет подключение к комет серверу. В качестве dev_id надо передавать свой публичный ключ разработчика.

Следующий код назначает функцию обработчик которая будет вызываться каждый раз когда в канал web_chat_pipe будет приходить сообщение. Первый аргумент это имя канала, а второй это функция обработчик.

<code JavaScript>
 CometServer().subscription("web_chat_pipe", function(data)
 {
     // Добавление полученного сообщения к списку сообщений.
     $("#WebChatFormForm").append("<p><b>"+msg.data.name+": </b>"+msg.data.text+"</p>");
 });

```

В функцию обработчик передаётся первым параметром объект содержащий информацию о сообщении и само сообщение.

Вот формат входящих сообщений. Поле data содержит те данные которые мы отправили в качестве сообщения. А поле server_info содержит информацию которую добавляет комет сервер к нашему сообщению.

<code JavaScript>
 data: 
    name: ""
    text: ""
 server_info:
    event: "undefined"      // Имя события
    history: false          // Если true то это данные загружаемые из истории канала а не пришедшие сейчас
    marker: undefined       // Специальный идентификатор, определён только когда history равно true 
    pipe: "web_chat_pipe"   // Имя канала которому адресовано сообщение
    user_id: 0              // Id отправителя, если 0 то не задан. Будет добавляется автоматически если человек авторизован на комет сервере. 

```

В данном случаи что бы к нам не пришло, мы рассчитываем что это сообщение для чата которое мы уже научились отправлять вызовом <code JavaScript>CometServer().web_pipe_send("web_chat_pipe", {"text":text, "name":name}); 
```
Обработку ошибок оставим на потом чтоб не раздувать код примеров.

Следующий фрагмент кода назначает функцию обработки на получение отчёта о доставке сообщения.

<code JavaScript>
 CometServer().subscription("_answer_to_web_chat_pipe", function(p)
 {
     $("#answer_div").html("Сообщение доставлено "+p.data.number_messages+" получателям. "+p.data.error);
 });

```

Здесь первый аргумент это строка в которой первый символ # и за ним имя канала отчёт о доставки сообщения в который мы ждём. В прицепе нет необходимости в обработке отчёта о доставке. Из него мы можем узнать сколько людей получили наше сообщение и не возникало ли каких либо ошибок.



# Как сделать чат с авторизацией
В предыдущем примере скрипта мини чата каждый может представится любым именем, в большинстве случаев на практике это не так. По этому рассмотрим как сделать чат с авторизацией. Для этого придётся часть кода чата написать на php.

Здесь рабочее [online демо чата с авторизацией](https://comet-server.ru/doc/example/5/). 
Теперь рассмотрим общий алгоритм работы чата с авторизацией:

  * Сначала появляется окно авторизации в чате.
  * В момент авторизации отправляется ключ авторизации в комет сервер и пользователю на страницу.
  * Пользователь соединяется с комет сервером по webSocets и передаёт ему свой ключ и id.
  * Комет сервер сверяет полученные от пользователя данные с тем что прислал php и если всё верно то авторизация считается пройденной.
  * После авторизации обмен ведётся как и в первом примере чата, но теперь к сообщениям от пользователя комет сервер добавляет id пользователя.
Всё. Таким образом пользователь не может подделать данные и выбрать чужое имя в чате так как для этого надо знать чужой пароль.

# Код примера чата на php
Теперь весь код примера чата на php, а затем мы его рассмотрим подробно:

<code php authChat.php>
<?php

 // Список пользователей, вместо базы используется массив чтоб не усложнять пример
$users = Array(
    "victor"=> Array("pass" => "qwerty", "id" => 1, "name" => "Виктор"),
    "dimon"=> Array("pass" => "qwerty", "id" => 2, "name" => "Димон"),
    "olga"=> Array("pass" => "qwerty", "id" => 3, "name" => "Оля"),
    "kate"=> Array("pass" => "qwerty", "id" => 4, "name" => "Катя"),
    "masha"=> Array("pass" => "qwerty", "id" => 5, "name" => "Маша"),
    );

 // Список соответсвия имени к id пользователя
$usersPublicData = Array( 1 => "Виктор", 2 => "Димон", 3 => "Оля", 4 => "Катя", 5 => "Маша",   );
  
// Включаем php ссесию
session_start();
     
/**
 * Выполняем авторизацию на комет сервере 
 * Второй параметр это ваш публичный идентификатор разработчика
 * Третий параметр это ваш секретный ключ разработчика
 */
$comet = mysqli_connect("app.comet-server.ru",
                        "15", "lPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8", "CometQL_v1");

// Если получаем команду уничтожить ссесию
if( isset($_GET["exit"]))
{   
    echo "Вы покинули php чат. <a href="/" >Перейти к форме авторизации в чате</a>";
    
    if(isset($_SESSION["userName"]))
    {
        // Оповещаем всех что человек покинул чат
        mysqli_query($comet, "INSERT INTO pipes_messages (name, event, message)VALUES('loginPipe', 'userExit', '".mysqli_real_escape_string($comet,json_encode(Array( "name" => $_SESSION["userName"] )))."')");  
    }
    
    session_destroy();
    exit;
}

// Если получили переменные login и pass то выполним авторизацию
if( isset($_GET["login"]) &&  isset($_GET["pass"]))
{ 
    if( !isset($users[$_GET["login"]]))
    {
        echo "В базе нет такого пользователя";
        header("Location: /");
        exit;
    }
    
    if(  $users[$_GET["login"]]["pass"] !== $_GET["pass"] )
    {
        echo "Пароль не верный";
        header("Location: /");
        exit;
    }
    
    // Оповещаем всех что залогинился человек и теперь онлайн в чате
    mysqli_query($comet, "INSERT INTO pipes_messages (name, event, message)VALUES('loginPipe', 'userLogin', '".mysqli_real_escape_string($comet,json_encode(Array( "name" => $users[$_GET["login"]]["name"])))."')");    
    
    // Генерируем ключ авторизации для пользователя на комет сервере. Длиной не более 32 символов.
    $userCometHash =  md5("Соль для хеша ".date("U"));
    
    // Сообщаем ключ авторизации комет серверу.
    mysqli_query($comet, "INSERT INTO users_auth (id, hash)VALUES (".$users[(int)$_GET["login"]]["id"].", '".mysqli_real_escape_string($comet, $userCometHash)."')"); 
    
    echo "<pre>";
    echo $userCometHash."\n";
    echo "</pre>";
    
    // Добавляем в сессию данные о пользователе
    $_SESSION["userHash"] = $userCometHash;
    $_SESSION["userId"] = $users[$_GET["login"]]["id"];
    $_SESSION["userLogin"] = $_GET["login"];
    $_SESSION["userName"] = $users[$_GET["login"]]["name"];
    
    echo "Авторизация прошла успешно. <a href="/" >Перейти к чату</a>";
    exit;
}

?>
<!DOCTYPE HTML>
<head>
    <title>Простой чат на php</title>
    <script language="JavaScript" type="text/javascript" src="https://comet-server.ru/template/Profitable_site/js/jquery-2.0.3.min.js" ></script>
    <script language="JavaScript" type="text/javascript" src="https://comet-server.ru/CometServerApi.js" ></script>
</head>
<body>
    
    <?php 
    if( !isset($_SESSION["userLogin"] ))
    { ?>
    <h1>Форма авторизации</h1>
    <form action="" method="GET">
        <input type="text" placeholder="Укажите ваш логин" name="login" value="victor"> <br>
        <input type="text" placeholder="Укажите ваш пароль" name="pass" value="qwerty" ><br>
        
        <input type="submit" value="Войти" >
        <pre>
            Возможные имена: victor, dimon, olga, kate, masha
        </pre>
    </form>
    <?php
    }
    else
    {
    ?>
        <div id="WebChatFormForm" ></div> 
        Ваше имя в чате <?php echo $_SESSION["userName"]; ?><br>
        <textarea id= "WebChatTextID" placeholder= "Сообщение в online чат..." ></textarea><br>

        <input type="button" onclick="web_send_msg();" value="Отправить" >
        <div id="answer_div" ></div>
        
        <a href="?exit">Выйти</a>
    <?php
    }
    ?>
    <script>
    // Общедоступная информация о пользователях (содержит связку id с паролем)
    var usersPublicData = <?php echo json_encode($usersPublicData); ?>;
    var myName = "<?php echo $_SESSION["userName"]; ?>";
    
    // Отправляет сообщение в чат
    function web_send_msg()
    {
        // Получение значений из элементов ввода.
        var text = $("#WebChatTextID").val(); // Получаем текст сообщения 

        // Очистка поля с текстом сообщения
        $("#WebChatTextID").val("");  

        // Добавление отправленного сообщения к списку сообщений.
        $("#WebChatFormForm").append("<p><b>"+myName+": </b>"+text+"</p>");

        // Отправка сообщения в канал чата
        CometServer().web_pipe_send("web_php_chat", {"text":text});
    }

    // Функция выполнится после загрузки страницы
    $(document).ready(function()
    {
        // Подключаемся к комет серверу
        CometServer().start({dev_id:15, // Ваш публичный id
            user_id:"<?php echo $_SESSION["userId"] ?>", // id пользователя
            user_key:"<?php echo $_SESSION["userHash"] ?>"})  // ключ пользователя

        // Подписываемся на канал в который и будут отправляться сообщения чата.
        CometServer().subscription("web_php_chat", function(msg)
        {
           console.log(msg)
           
           var name = "Аноним";
           if(msg.server_info.user_id > 0)
           {
               name = usersPublicData[msg.server_info.user_id];
           }
            // Добавление полученного сообщения к списку сообщений.
            $("#WebChatFormForm").append("<p><b>"+name+": </b>"+msg.data.text+"</p>");
        });

        // Подписываемся на сообщения о входе людей в чат (отпраляются из php)
        CometServer().subscription("loginPipe.userLogin", function(msg)
        {
            // Добавление уведомления в ленту сообщений
            $("#WebChatFormForm").append("<p>Пользователь <b>"+msg.data.name+"</b> вошол в чат.</p>");
        });

        // Подписываемся на сообщения о выходе людей из чата (отпраляются из php)
        CometServer().subscription("loginPipe.userExit", function(msg)
        {
            // Добавление уведомления в ленту сообщений
            $("#WebChatFormForm").append("<p>Пользователь <i>"+msg.data.name+"</i> покинул в чат.</p>");
        });            
        
        // Подписываемся отчёт о доставке сообщения в чат.
        CometServer().subscription("#web_php_chat", function(p)
        {
           console.log(p)
            $("#answer_div").html("Сообщение получили "+p.data.number_messages+" человек. "+p.data.error);
        });
    });
</script>
</body>
</html>

```

Теперь рассмотрим код чата на php подробно:

Для реализации авторизации в чате на php нам надо иметь базу пользователей, и какой то backend для работы с этой базой. Так как это учебный пример, и цель примера научить работе с комет сервером, то мы вместо хранения логинов и паролей пользователей чата в базе данных mysql зададим массив в котором жёстко пропишем имена и пароли для 5 тестовых пользователей php чата.

<code php>
// Список пользователей, вместо базы используется массив чтоб не усложнять исходники чата на php.
$users = Array(
    "victor"=> Array("pass" => "qwerty", "id" => 1, "name" => "Виктор"),
    "dimon"=> Array("pass" => "qwerty", "id" => 2, "name" => "Димон"),
    "olga"=> Array("pass" => "qwerty", "id" => 3, "name" => "Оля"),
    "kate"=> Array("pass" => "qwerty", "id" => 4, "name" => "Катя"),
    "masha"=> Array("pass" => "qwerty", "id" => 5, "name" => "Маша"),
    );

 // Список соответсвия имени к id пользователя.
$usersPublicData = Array( 1 => "Виктор", 2 => "Димон", 3 => "Оля", 4 => "Катя", 5 => "Маша",   );

```


Код подключения из php к комет серверу. Указываем ваш ключ и пароль для комет сервера.
<code php>
$comet = mysqli_connect("app.comet-server.ru", "15", "lPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8", "CometQL_v1"); 

```


Второй параметр это ваш [публичный идентификатор разработчика](comet:dev_id).
Третий параметр это ваш [секретный ключ разработчика](comet:dev_id).

Код обработки выхода пользователя из php чата:
<code php>
// Если получаем команду уничтожить сессию
if( isset($_GET["exit"]))
{   
    session_destroy();
    echo "Вы покинули чат. <a href="/" >Перейти к форме авторизации</a>";
    
    // Оповещаем всех что человек покинул чат
    mysqli_query($comet, "INSERT INTO pipes_messages (name, event, message)VALUES('loginPipe', 'userExit', '".mysqli_real_escape_string($comet,json_encode(Array( "name" => $_SESSION["userName"] )))."')"); 
    exit;
}

```

Когда человек нажимает на ссылку выхода из чата мы уничтожаем сессию и отправляем всем остальным участникам сообщение об этом. Здесь loginPipe это имя канала, а userExit это имя события в канале. Для удобства можно через один канал отправлять события с разными именами.


___
Следует обратить внимание что канал для уведомлений о попадании в чат называется loginPipe, то есть не содержит префикса "web_" а это означает что в него можно отправить сообщение только через CometQL при наличии секретного ключа разработчика.
___


# Авторизация пользователей в чате
Теперь авторизация пользователей в чате. В начале идёт код проверки логина и пароля, затем уведомление всех о входе пользователя в чат. А потом начинается интересное.

<code php>
// Если получили переменные login и pass то выполним авторизацию
if( isset($_GET["login"]) &&  isset($_GET["pass"]))
{
    if( !isset($users[$_GET["login"]]))
    {
        echo "В базе нет такого пользователя";
        exit;
    }
    
    if(  $users[$_GET["login"]]["pass"] !== $_GET["pass"] )
    {
        echo "Пароль не верный";
        exit;
    }
    
    // Оповещаем всех что залогинился человек и теперь онлайн в чате
    mysqli_query($comet, "INSERT INTO pipes_messages (name, event, message)VALUES('loginPipe', 'userLogin', '".mysqli_real_escape_string($comet,json_encode(Array( "name" => $users[$_GET["login"]]["name"])))."')");    
    
    // Генерируем ключ авторизации для пользователя на комет сервере. Длиной не более 32 символов.
    $userCometHash =  md5("Соль для хеша ".date("U"));
    
    // Сообщаем ключ авторизации комет серверу.
    mysqli_query($comet, "INSERT INTO users_auth (id, hash)VALUES (".$users[(int)$_GET["login"]]["id"].", '".mysqli_real_escape_string($comet, $userCometHash)."')"); 
      
    echo "<pre>";
    echo $userCometHash."\n"; 
    echo "</pre>";
    
    // Добавляем в сессию данные о пользователе
    $_SESSION["userHash"] = $userCometHash;
    $_SESSION["userId"] = $users[$_GET["login"]]["id"];
    $_SESSION["userLogin"] = $_GET["login"];
    $_SESSION["userName"] = $users[$_GET["login"]]["name"];
    
    echo "Авторизация прошла успешно. <a href="/" >Перейти к чату</a>";
    exit;
}

```

В переменную $userCometHash генерируется ключ из 32 символов. Вот этот ключ мы в следующей строке кода отправляем комет серверу вместе с идентификатором пользователя. 
<code php>
// Генерируем ключ авторизации для пользователя на комет сервере. Длиной не более 32 символов.
$userCometHash =  md5("Соль для хеша ".date("U"));
  
// Сообщаем ключ авторизации комет серверу.
mysqli_query($comet, "INSERT INTO users_auth (id, hash)VALUES (".$users[(int)$_GET["login"]]["id"].", '".mysqli_real_escape_string($comet, $userCometHash)."')"); 

```

Всё теперь комет сервер имеет связку id+хеш по которой он может идентифицировать нашего пользователя. Теперь осталось этот же хеш авторизации и id передать пользователю. Это мы сделаем в JavaScript при подключении к комет серверу.

Вот в этом фрагменте кода мы подключаемся к комет серверу и передаём туда публичный идентификатор разработчика, идентификатор пользователя и его ключ авторизации. Стоит посмотреть где этот вызов находится в общем коде.

<code php>
// Подключаемся к комет серверу
CometServer().start({dev_id:15, // Ваш публичный id
                user_id:"", // id пользователя
                user_key:""})  // ключ пользователя

```

С авторизацией думаю ясно. Теперь посмотрим на получение уведомлений о входе и выходе собеседников. Здесь довольно просто. Сообщения как о входе так и о выходе людей отправляются в один и тот же канал, но имеют разное имя события. Что бы назначить обработчик на определённое событие а не на все сообщения в канале мы указываем сначала имя канала потом ставим точку и пишем имя события. Таким образом следующий код ставит две функции каждая из которых обрабатывает свои тип событий из одного и того же канала.

<code php>
// Подписываемся на сообщения о входе людей в чат
CometServer().subscription("loginPipe.userLogin", function(msg)
{
    // Добавление уведомления в ленту сообщений
    $("#WebChatFormForm").append("<p>Пользователь <b>"+msg.data.name+"</b> вошёл в чат.</p>");
});

// Подписываемся на сообщения о выходе людей из чата
CometServer().subscription("loginPipe.userExit", function(msg)
{
    // Добавление уведомления в ленту сообщений
    $("#WebChatFormForm").append("<p>Пользователь <i>"+msg.data.name+"</i> покинул в чат.</p>");
});

```

Вот вообщем всё, мы рассмотрели пример создания простого чата на php. Comet сервер позволил в значительной мере упростить создание чата на php.

# Сохранение сообщений в бд

Если вам надо сохранять сообщения в бд или дополнительно их фильтровать или как то дополнять данными. То вот есть ещё пример чата https://github.com/CppComet/php-chat-example похожий на примеры этой статьи но в нём отправка сообщений идёт не через вызов функции web_pipe_send, а через ajax запрос к php скрипту который уже направляет сообщение в комет сервер. Если использовать его то можно вставить свою произвольную обработку сообщений, в том числе и их сохранение в файле https://github.com/CppComet/php-chat-example/blob/master/chat.php