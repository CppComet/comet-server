
#  Редакции  CppComet # 

Есть две редакции CppComet  [опенсорс версия](https://github.com/CppComet/comet-server) и  [SaaS платформа](https://comet-server.com).
Апи у них полностью совместимо, и единственная разница только при указании параметров для подключения.

#  Подключение из JavaScript API  # 

При подключении из JavaScript API  к SaaS версии 

```
CometServer().start({dev_id:15, user_id:1, user_key:"userHash"})
```


При подключении из JavaScript API  к  опенсорс версии 

```
CometServer().start({dev_id:0, user_id:1, user_key:"userHash", node:"example.com"})
```


#  Подключение из CometQL # 

При подключении из CometQL к SaaS версии 

```

$dev_id = "15";

// Выдаётся в личном кабинете на сайте comet-server.com
$dev_key = "lPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8";
$link = mysqli_connect("app.comet-server.ru", $dev_id, $dev_key, "CometQL_v1");

```


При подключении из CometQL к опенсорс версии 

```

$dev_id = "root";

// Задаётся в файле comet.ini
$dev_key = "lPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8"; 
$link = mysqli_connect("app.comet-server.ru", $dev_id, $dev_key, "CometQL_v1");

```


# Безопасность совместного использования # 

В SaaS версии все данные пользователей изолированы друг от друга на основании параметра dev_id который указывается при подключении к серверу. Вы используя свой dev_id ни как не будете влиять на клиентов указавших при подключении другой dev_id.
Если говорить об [опенсорс версии](https://github.com/CppComet/comet-server) то там нет разделения на dev_id то есть она рассчитана что на один экземпляр сервера приходится только один пользователь. Поэтому параметр dev_id там не требуется указывать при подключении.


