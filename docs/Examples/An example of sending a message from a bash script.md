
# An example of sending a message from a bash script

An example of a console command that sends a message containing a load of data to the server.

```

echo "INSERT INTO pipes_messages (name, event, message)VALUES('web_MainPageChat', '', '{\"text\":\"`cat /proc/loadavg`\",\"name\":\"AVG `uname -n`\"}' );" | mysql -h app.comet-server.ru -u15 -plPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8 -DCometQL_v1

```

Run this command in linux console and a message containing information about the load on the server in the chat window.
<html>

</html>
