
# Проверка хостинга на предмет возможности подключится к комет серверу 

У некоторых хостинг провайдеров установлен запрет на внешние соединения это достаточно часто встречается на бесплатных или очень очень дешёвых хостингах. На многих хостингах сразу предупреждают об ограничениях но не везде.

Для работы с сервисом Star.Comet требуется поддержка вашим хостером функции работы с сокетами и отсутствие запрета на внешние соединения. Эти возможности встречаются у большинства платных хостинг провайдеров.

# Скрипт проверки хостинга
Для проверки хостинга на предмет запрета внешних соединений скачайте этот скрипт и запустите его на своём хостинге. 
<file php testHosting.php>
<?php
    header('Content-Type: text/html; charset=utf-8');
        
    ini_set('display_errors','on');
    error_reporting(E_ALL);  
       
    $resTest = Array();
    $f = fsockopen("app.comet-server.ru", 80,$e1,$e2);
    if(!$f)
    {
        $resTest[] = Array("info" => "Не удалось создать соединение к comet-server.ru: error code:".$e1."(".$e2.")", "error" => true);
    }
    else
    {
        $str1 = "GET / HTTP/1.1\r\nHost: app.comet-server.ru\r\n\r\n";  
        if( fputs($f, $str1, strlen($str1) ) # false)
        { 
            $resTest[] = Array("info" => "fputs error on app.comet-server.ru", "error" => true);
        }
        else
        {
            $tmp = fgets($f); 
            $resTest[] = Array("info" => "OK", "error" => false);
        }
    }
    
    $f = fsockopen("googl.com", 80,$e1,$e2);
    if(!$f)
    {
        $resTest[] = Array("info" => "Не удалось создать соединение к googl.com: error code:".$e1."(".$e2.")", "error" => true);
    }
    else
    {
        $str1 = "GET / HTTP/1.1\r\nHost: googl.com\r\n\r\n";  
        if( fputs($f, $str1, strlen($str1) ) # false)
        { 
            $resTest[] = Array("info" => "fputs error on googl.com", "error" => true);
        }
        else
        {
            $tmp = fgets($f); 
            $resTest[] = Array("info" => "OK", "error" => false);
        }
    }
     
    $f = fsockopen("yandex.ru", 80,$e1,$e2);
    if(!$f)
    {
        $resTest[] = Array("info" => "Не удалось создать соединение к yandex.ru: error code:".$e1."(".$e2.")", "error" => true);
    }
    else
    {
        $str1 = "GET / HTTP/1.1\r\nHost: yandex.ru\r\n\r\n";  
        if( fputs($f, $str1, strlen($str1) ) # false)
        { 
            $resTest[] = Array("info" => "fputs error on yandex.ru", "error" => true);
        }
        else
        {
            $tmp = fgets($f); 
            $resTest[] = Array("info" => "OK", "error" => false);
        }
    }
         
    $link = mysqli_connect("app.comet-server.ru", "15", "lPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8", "CometQL_v1");
    if(!$link)
    {
        $resTest[] = Array("info" => "Не удалось создать соединение c CometQL (Использование CometQL не возможно)", "error" => true);
    }
    else
    {
        
        $result = mysqli_query (  $link, "show status" ); 
        if(mysqli_errno($link) != 0 && @mysqli_num_rows($result))
        {
            $resTest[] = Array("info" => "<b>Error code:<a href='https://comet-server.ru/wiki/doku.php/comet:cometql:error'  target='_blank' >".mysqli_errno($link)."</a>&nbsp;&nbsp;Error text:<a href='https://comet-server.ru/wiki/doku.php/comet:cometql:error' target='_blank' >".mysqli_error($link)."</a></b>", "error" => true);
        }
        else
        { 
            $resTest[] = Array("info" => "OK", "error" => false);
        }
    }
    
?>

<!DOCTYPE HTML>
<html>
<head> 
</head>
<body>
    
<h1>Проверка хостинга</h1>

<?php 
    if($resTest[0]["error"]){ echo "<b style='color:#F77;' >";}else{echo "<b style='color:#494;' >";}
        echo $resTest[0]["info"]."</b><br>\n";
         
    if($resTest[1]["error"]){ echo "<b style='color:#F77;' >";}else{echo "<b style='color:#494;' >";}
        echo $resTest[1]["info"]."</b><br>\n";
        
    if($resTest[2]["error"]){ echo "<b style='color:#F77;' >";}else{echo "<b style='color:#494;' >";}
        echo $resTest[2]["info"]."</b><br>\n";
    
    if($resTest[4]["error"]){ echo "<b style='color:#F77;' >";}else{echo "<b style='color:#494;' >";}
        echo $resTest[3]["info"]."</b><br>\n";
 ?>
    
</body>
</html> 
</file>