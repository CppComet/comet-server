 
[![Gitter](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/CppComet/Lobby?utm_source=share-link&utm_medium=link&utm_campaign=share-link)
 
![CppComet](https://comet-server.com/img/CppComet.png)

Comet server is written in C++, for storing data using MySQL.   

# Features

Comet technology – allows sending arbitrary messages to client through server initiative

* Api for more then 9 languages. [CometQL API](http://comet-server.org/doku.php/en:comet:cometql) allows you connect to server by mysql protocol.
* Good documentation in [Russian](http://comet-server.org/doku.php/ru) and [English](http://comet-server.org/doku.php/en) languages
* [Supports users authenticating on the server](https://comet-server.com/wiki/doku.php/en:comet:authentication#users_authorization_on_the_cometserver)
* [Allow sending a private message to users](https://comet-server.com/wiki/doku.php/en:comet:cometql#table_users_messages)
* [Allows you to send public messages to users via channels](https://comet-server.com/wiki/doku.php/en:comet:cometql#table_pipes_messages)
* [Allows subscription from JavaScript to changing other user’s status](https://comet-server.com/wiki/doku.php/en:comet:javascript_api#subscribe_to_receive_messages_from_channel)
* [Allows subscription to updates of online users list in real-time](https://comet-server.com/wiki/doku.php/en:comet:javascript_api#list_of_online_users_channels_track)
* [Sending messages to channel from JS](https://comet-server.com/wiki/doku.php/en:comet:javascript_api#sending_messages_to_channel_from_js_pipes_web)
* [Allows get information when users were online](https://comet-server.com/wiki/doku.php/en:comet:cometql#table_users_time)
 
# How it fast

[Report about load testing](https://comet-server.com/wiki/doku.php/en:comet:load-testing-result#report_about_testing_of_64000_connections_online) of 64,000 connections online

# How it works

* Introduction to comet technology [EN](https://comet-server.com/wiki/doku.php/en:comet:introduction-to-comet#introduction) and [RU](https://comet-server.com/wiki/doku.php/comet:introduction-to-comet#introduction) 

# Documentation and examples

Documentation in [Russian](http://comet-server.org/doku.php/ru) and [English](http://comet-server.org/doku.php/en) languages
 
 * How to build and run [EN](http://comet-server.org/doku.php/en:comet:building-from-source) or [RU](http://comet-server.org/doku.php/comet:building-from-source)
 * CometQL API documentation [EN](http://comet-server.org/doku.php/en:comet:cometql) or [RU](http://comet-server.org/doku.php/comet:cometql) (To connect to the comet server api from any server-side language is used CometQL)
 * JavaScript API documentation [EN](http://comet-server.org/doku.php/en:comet:javascript_api) or [RU](http://comet-server.org/doku.php/comet:javascript_api) (To connect to the server from JavaScript API use file from [api/CometServerApi.js](https://github.com/CppComet/comet-server/tree/master/api) )

# Demo
 
* https://github.com/CppComet/php-chat-example  ( [One of the simplest way to create php websockets chat with CppComet](https://medium.com/one-of-the-simplest-way-to-create-php-websockets/one-of-the-simplest-way-to-create-websockets-chat-with-php-and-cppcomet-bad0a7e47df0) )
* https://codepen.io/Levhav/pen/vJWWqW ( [Creating a simple chat using CppComet](https://medium.com/one-of-the-simplest-way-to-create-php-websockets/creating-a-simple-chat-using-cppcomet-e0e68664afe1) )
* https://cppcomet.github.io/comet-server/demo/simple-chat/simple-chat.html
* https://github.com/CppComet/auth-example

## Demo access to server API

For testing CppComet without install on vps  you can use [free cloud service with same api](https://comet-server.com/). 
In the all examples I will use demonstration access from [comet-server.com](http://comet-server.com) for those who could not or were too lazy to deploy the server on their vps.

For demo access use credentials:
```
Login: 15
Password:lPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8
Host: app.comet-server.ru
```

Example of connecting to [cometQL api](http://comet-server.org/doku.php/en:comet:cometql) from console using mysql-client:
```
mysql -h app.comet-server.ru -u15 -plPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8 -DCometQL_v1 --skip-ssl
```
## Simple chat demo

[Simple chat demo page](http://comet-server.com/doc/CometQL/MainPageChat/index.php)
![Simple chat demo gif](https://comet-server.com/doc/CometQL/CometQL-demo.gif)

On this gif animation you see how send message to [web chat](http://comet-server.com/doc/CometQL/MainPageChat/index.php) from CppComet api.

We connect to CppComet by mysql protocol and send [CometQL query](http://comet-server.org/doku.php/en:comet:cometql).

For connect to demo server use this command:
```
mysql -h app.comet-server.ru -u15 -plPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8 -DCometQL_v1 --skip-ssl
```
For send message to demo chat use this query:
```
INSERT INTO pipes_messages (name, event, message)VALUES("web_MainPageChat", "", '{ "text":"My text","name":"My name"}' );
```

License
----

GNU AGPLv3

 
