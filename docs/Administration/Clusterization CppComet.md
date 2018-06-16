

# Clusterization CppComet

In comet server there is possibility of clusterization in that every server of cluster can accept queries and send to their those servers of cluster that it is necessary to advise about an event. (It is possible to conduct an analogy with master-master replication at databases)

The operations of insertion of data (insert and set) are executed asynchronously, it means that you will not wait while a query will be sent on all servers of cluster.

The operations of retrieval of data (select and show) work synchronously. Because must get data and will return them responsible.

To activate of mechanism of clusterization it is necessary to add to the configuration file parameters for connecting to the servers of cluster in a section [ws] and in a section [cometqlproxy]


___
The mechanism of clusterization was worked out not a long ago. If problems will turn out in tuning or in-process or blanks in documentation don’t be shy [to ask the questions](https://gitter.im/CppComet/Lobby) and [ write bug reports.](https://github.com/CppComet/comet-server/issues)
___


# Tuning of section [ws]

In a section [ws] there is a parameter of cluster it presents from itself the list of parameters for connecting to other nodes of cluster. Every new connecting is set from a new line. Square brackets before the beginning of parameter specify on that this line does not redefine the previous value of parameter of cluster and complements it with another element of list of values.


```

; The parameters of the cluster (without spaces or anything else between the parameters, are case-sensitive)
cluster = []Server=127.0.0.1,Database=CometQL_v1,Uid=root,Pwd=0000000000000000000000000000000000000000000000000000000000000000,Port=3311 
cluster = []Server=127.0.0.1,Database=CometQL_v1,Uid=root,Pwd=0000000000000000000000000000000000000000000000000000000000000000,Port=3321

```


In an example two lines of connecting are higher appropriated the parameter of cluster to two other nodes of cluster. 
A line by itself presents a few parameters  
  * Server - name of host of comets of server
  * Database - version of api is always CometQL_v1 (until there is no a version 2 or whatever else)
  * Uid - user name is ordinary root
  * Pwd - password for connecting

If  us cluster consists from three nodes that means in configuration should be specified two nearby nodes. Thus, at the receipt of data from JavaScript API will be notified other the server of cluster.

# Tuning of section [cometqlproxy]

A section of cometqlproxy is an interface for appeals through CometQL API to the cluster. 
By the structure it looks like the section of cometql

If in the not cluster mode all queries from CometQL went away to the module of cometql on that port that was cauldron in a section [cometql] that in the mode of cluster you can activate on one node or on a few nodes cluster module of cometqlproxy and to send queries to it (it will not be to change queries) accepting the module of cometqlproxy CometQL query of parses it and depending on a query sends it to one or a few nodes of cluster. If a query was on the insertion of data (insert and set) that it will be sent to one of living node cluster. If request for the receipt of data that data will be extracted or with one node or in the worst cases will be polled all living nodes, whereupon a general answer will be collected from answers from all nodes.

In the section of cometqlproxy too there is a parameter of cluster looks like the analogue from the section of ws, except that it must list all the nodes of the cluster and not just the neighbors. So, if your cluster from three nodes that in the sections of ws each of nodes will be for two lines for connecting to other nodes and in the section of cometqlproxy will be indicated all three nodes cluster.

It is so done because the module of cometqlproxy can work on instance comets of server simply in the mode of proxying queries to other the comets to the servers without raising on the same node yet and modules of cometql and WS


```

[cometqlproxy]
; Clustering Functions
;
ip = 0.0.0.0
thread_num = 3 ; number of threads for receive message from cometql
statistics = 10
port = 3301
 
; The parameters of the cluster (without spaces or anything else between the parameters, are case sensitive)
cluster = []Server=127.0.0.1,Database=CometQL_v1,Uid=root,Pwd=0000000000000000000000000000000000000000000000000000000000000000,Port=3301 
cluster = []Server=127.0.0.1,Database=CometQL_v1,Uid=root,Pwd=0000000000000000000000000000000000000000000000000000000000000000,Port=3311
cluster = []Server=127.0.0.1,Database=CometQL_v1,Uid=root,Pwd=0000000000000000000000000000000000000000000000000000000000000000,Port=3321

```

