
# Report about testing of 64,000 connections online


At previous article we examined the [ how to conduct the stress testing.](/docs/wiki-md/comet/load-testing.md) And here I want to show you how I conducted it by myself, and give its results.


___
The load of 64,000 is the maximum that will allow the operational system to create. If you want more then you need to test one server from several machines with tsung at the same time. The TCP connection is unique and defined by the four [source ip, source port, dest ip, dest port], so you can create no more than 64,000 simultaneous connections between one machine and one server port. 
___


# Results # 

On the screenshot you can see 3 consoles.  
{{ :comet:снимок_экрана_от_2017-06-08_15-35-11.png|Пик тестирования}}

  - At the top was running the tsung
  - At the left is the comet server
  - At the right - htop

# The output of the comet server # 

The numbers are on the screenshot:

Number of connections online;
Server’s operating time in seconds;
A column with a list with amount of network events was processed for the entire time (such as connecting, receiving messages, closing the connection);
The first process in the list was processed most of all as it was engaged in receiving incoming connections (under a normal load, there is not such a large separation because the incoming connection is set for a long time);
Other processes for already received connections processed incoming messages;
The PcS column indicates how many network events were processed in the last second (there are no zeros on the gif animation) Zero since the connections are accepted and hang in anticipation of incoming messages, but with this test scenario we do not send any additional events.
  
{{:comet:out.png?direct |CppComet}}

# The output of htop # 

The numbers are on the screenshot:

Comet server processes (It can be seen that he spent 4891 MB of RAM);
tsung processes (It can be seen that he spent 2262 MB of RAM);
Total memory consumption (The rest was consumed by the operating system and other programs started at the time of testing). 

{{:comet:htop.png?direct |htop}}

# The testing process # 

Gif-animation of the testing process. It is seen that there is an increase of about 2,500 connections per second and that all the kernels are loaded almost evenly. You can also see an increase in memory consumption. 

{{https://comet-server.com/doc/CppComet/HL-test2-64000.gif}}

# The tsung report # 


___
Below are only 2 graphs, [the full version of the report](https://cppcomet.github.io/comet-server/HL-tests/HL-test1/20170608-1534/report.html) posted on github.
___


Queries per second

{{https://cppcomet.github.io/comet-server/HL-tests/HL-test1/20170608-1534/images/graphes-Perfs-rate.png|queries per second}}

The number of simultaneous connections

{{https://cppcomet.github.io/comet-server/HL-tests/HL-test1/20170608-1534/images/graphes-Users-simultaneous.png|the number of simultaneous connections}}

