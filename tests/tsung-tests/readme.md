
# Load testing

Starting the server in console mode

```
ulimit -m 64000
./cpp_comet```

Run the load test
```
ulimit -m 64000
tsung -f ./test/tsung.xml```

## Check how many connections are currently open
 
Let's check how many connections are currently open by an independent tool
```
ss -p | grep "cpp_comet" | wc -l
```

## Building a test report
```
cd ~/.tsung/log/20170524-1159
/usr/lib/tsung/bin/tsung_stats.pl```
 
