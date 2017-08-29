
# Stress testing

The program called “tsung” can be used for stress testing.

# Tsung installation


```

apt install tsung

```


# Test scenario # 

The tsung program needs to transfer a file with description of the test script. Here is an example of a simple test scenario:


```

<?xml version="1.0"?>
<!DOCTYPE tsung SYSTEM "/usr/share/tsung/tsung-1.0.dtd">
<tsung loglevel="debug" version="1.0">
  <clients>
    <client host="localhost" use_controller_vm="true" maxusers="64000" />
  </clients>

  <servers>
    <server host="localhost" port="8087" type="tcp" /> 
  </servers>

  <load>
    <arrivalphase phase="1" duration="60" unit="second">
        <users maxnumber="64000" arrivalrate="2500" unit="second" />
    </arrivalphase>
  </load>

  <options>
    <option name="ports_range" min="1025" max="65535"/>
  </options>
  
  <sessions>
    <session name="websocket" probability="100" type="ts_websocket">
        <request>
             <websocket type="connect" path="/comet-server/ws/sesion=&amp;myid=&amp;devid=0&amp;v=3.24&amp;uuid=48wTOvoa-uEtC0thHzBkIKir14sXgkOy&amp;api=js"></websocket>
        </request>

       
        <for var="i" from="1" to="20000" incr="1">
          <thinktime value="150"/>
        </for>
        
        <request>
            <websocket type="close"></websocket>
        </request>
    </session>
  </sessions>
</tsung>

```


There is specified that to localhost port 8087 it is necessary to be connected via web sockets and establish 2,500 connections every second until they total 64,000.

That is, this script simply creates a test load of 64,000 users online. The synthetic test and real 64000 will load the server somewhat differently and not just hang online, but it's already from the application and the prospective scenario of using the comet server that you can create your own test script that would reflect your type of load on the comet server.

# Overload more than 64,000 online # 



The load of 64,000 is the maximum that will allow the operational system to create. If you want more then you need to test one server from several machines with tsung at the same time. The TCP connection is unique and defined by the four [source ip, source port, dest ip, dest port], so you can create no more than 64,000 simultaneous connections between one machine and one server port.

# Server launching # 

In order for the OS to remove as many connections as possible, it is necessary to increase the limit on the number of file descriptors with the next command


```

ulimit -m 64000 

```


In the section benchmark of file comet.ini it is necessary to establish the options of benchmark and ws sections:


```

[benchmark]
to_log = true   ; Output of measurements of load in the log  


[ws]
ip = 0.0.0.0
backlog = 10000
epoll_size = 100000
thread_num = 12    ; threads number, do more than the cores on the server so that the server is not lock-free
benchmark = 1      ; Interval between load measurements (0 = do not measure)
port = 8087
uptimeTestInterval = 600 ; The interval for the uptime checks for connections (also for the one connection fixes the value of the online user count)
maxUptime = 0 ; The maximum value of uptime after which the connection is disabled.

```


And then start the server in console mode to see the statistics output

```

./cpp_comet

```


# Test launching # 

Stress testing will be run thus:

```

ulimit -m 64000
tsung -f ~/tsung.xml start

```

In “tsung” we transfer with “if” value the file with test scenario

# The analysis of the results # 

In the testing process, you can watch the OS load for example via the htop or iotop programs.

Check that the built-in comet server counter of the total number of connections online shows numbers close to the truth it is possible with the next command:


```

ss -p | grep "cpp_comet" | wc -l

```


It calculates the number of incoming connections using the operating system. After testing tsung, gives a test report to the folder. But that it was possible to look it is necessary to process it by a script which goes together with tsung. You can find that script in the next folder /usr/lib/tsung/bin/tsung_stats.pl


```

cd /home/victor/.tsung/log/20170524-1159
/usr/lib/tsung/bin/tsung_stats.pl

```


After that, the script will generate an already human-readable test report.


___
 It should be taken into account that in this scenario both the test and the server on one machine so that they load it together. For the purity of the experiment, it is recommended to run the server on one machine, and tsung on another machine. 
___
