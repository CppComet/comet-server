
# Пример отправки сообщения из bash скрипта

Пример консольной команды которая отправляет сообщение содержащие данные о нагрузке на сервер.

```

echo "INSERT INTO pipes_messages (name, event, message)VALUES('web_MainPageChat', '', '{\"text\":\"`cat /proc/loadavg`\",\"name\":\"AVG `uname -n`\"}' );" | mysql -h app.comet-server.ru -u15 -plPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8 -DCometQL_v1

```

Выполните эту команду в linux консоли и в окне чата появится сообщение содержащие данные о нагрузке на сервер.
<html>
<iframe src="//comet-server.ru/doc/CometQL/MainPageChat/index.php" style="width: 100%;  display: block;  height: 250px;"></iframe>
</html>
