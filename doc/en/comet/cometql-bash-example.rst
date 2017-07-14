====== An example of sending a message from a bash script ======

An example of a console command that sends a message containing a load of data to the server.
<code bash>
echo "INSERT INTO pipes_messages (name, event, message)VALUES('web_MainPageChat', '', '{\"text\":\"`cat /proc/loadavg`\",\"name\":\"AVG `uname -n`\"}' );" | mysql -h app.comet-server.ru -u15 -plPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8 -DCometQL_v1
</code>
Run this command in linux console and a message containing information about the load on the server in the chat window.
<html>
<iframe src="//comet-server.com/doc/CometQL/MainPageChat/index.php" style="width: 100%;  display: block;  height: 250px;"></iframe>
</html>
