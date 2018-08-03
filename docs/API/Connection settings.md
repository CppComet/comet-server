 
# Public identifier Developer

In the examples are usually referred as dev_id. Used only when connecting to [SaaS version of CppComet](https://comet-server.com).
  * When you connect to the [open source version](https://github.com/CppComet/comet-server) from the JavaScript API may not be specified or be set to 0. [Read more here](https://comet-server.com/wiki/doku.php/en:comet:saas)
  * When connecting to the open source version from CometQL, the dev_id should be replaced with the string "root" [Read more here](https://comet-server.com/wiki/doku.php/en:comet:saas)

# Secret developer key

Secret Developer key, it is composed of 64 characters and is used for authentication to the comet server. No it does not tell anyone. In the examples, commonly referred to as dev_key. 
 
If you use [comet.ini](https://github.com/CppComet/comet-server]], it is specified in the settings in the [[/docs/Administration/file%20comet.ini) file, if you use [SaaS version of CppComet](https://comet-server.com) it is issued when you add a new service in your account.

# Possible problems after installation
 
Pay attention to what values of the port parameter are specified in the sections [ws] and [cometql] on these ports the comet server will wait for incoming connections.

In the comet.ini example in the repository, the port parameter for connections from JavaScrip api is set to 8087
This means that you need to connect like this:

```
cometApi.start({user_id:1, user_key:"userHash", node:"example.ru:8087"})
```


The port parameter for connections from CometQL is set to 3300
This means that you need to connect like this:

```
$link = mysqli_connect("example.ru", "root", "", "CometQL_v1", 3300);
```


 
 