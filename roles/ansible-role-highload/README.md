# Highload

Role for tuning Linux TCP/IP subsystem for highload.

Requirements
------------

Read kernel documentation  
[net.txt](https://www.kernel.org/doc/Documentation/sysctl/net.txt)  
[ip-sysctl.txt](https://www.kernel.org/doc/Documentation/networking/ip-sysctl.txt)  
and decide what values are appropriate for your environment

Role Variables
--------------

| Variable                             | Default               | Comments (type)                                                                                                              |
| :---                                 | :---                  | :---                                                                                                                         |
| `highload_rmem_max`                  | `16777216`            | The maximum receive socket buffer size in bytes                                                                              |
| `highload_wmem_max`                  | `16777216`            | The maximum send socket buffer size in bytes                                                                                 |
| `highload_rmem_default`              | `16777216`            | The default setting of the socket receive buffer in bytes                                                                    |
| `highload_wmem_default`              | `16777216`            | The default setting of the socket send buffer in bytes                                                                       |
| `highload_optmem_max`                | `40960`               | Maximum ancillary buffer size allowed per socket                                                                             |
| `highload_tcp_rmem`                  | `4096 87380 16777216` | This is a vector of 3 integers: [min, default, max].                                                                         |
|                                      |                       | These parameters are used by TCP to regulate receive buffer sizes                                                            |
|                                      |                       | `min`                                                                                                                        |
|                                      |                       | Minimum size of the receive buffer used by each TCP socket                                                                   |
|                                      |                       | `default`                                                                                                                    |
|                                      |                       | The default size of the receive buffer for a TCP socket                                                                      |
|                                      |                       | `max`                                                                                                                        |
|                                      |                       | The maximum size of the receive buffer used by each TCP socket                                                               |
| `highload_tcp_wmem`                  | `4096 65536 16777216` | This is a vector of 3 integers: [min, default, max]                                                                          |
|                                      |                       | These parameters are used by TCP to regulate send buffer sizes                                                               |
|                                      |                       | `min`                                                                                                                        |
|                                      |                       | Minimum size of the send buffer used by each TCP socket                                                                      |
|                                      |                       | `default`                                                                                                                    |
|                                      |                       | The default size of the send buffer for a TCP socket                                                                         |
|                                      |                       | `max`                                                                                                                        |
|                                      |                       | The maximum size of the send buffer used by each TCP socket                                                                  |
| `highload_ip_local_port_range`       | `1025 65000`          | Defines the local port range that is used by TCP and UDP to choose the local port                                            |
| `highload_tcp_tw_reuse`              | `1`                   | Allow to reuse TIME-WAIT sockets for new connections when it is safe from protocol viewpoint                                 |
| `highload_tcp_tw_recycle`            | `1`                   | Enable fast recycling TIME-WAIT sockets                                                                                      |
| `highload_netdev_max_backlog`        | `50000`               | Maximum number of packets, queued on the INPUT side, when the interface receives packets faster than kernel can process them |
| `highload_tcp_max_syn_backlog`       | `30000`               | Maximal number of remembered connection requests, which have not received an acknowledgment from connecting client           | 
| `highload_tcp_max_tw_buckets`        | `2000000`             | Maximal number of timewait sockets held by system simultaneously                                                             |
| `highload_tcp_fin_timeout`           | `30`                  | The length of time an orphaned (no longer referenced by any application) connection will remain in the FIN_WAIT_2            |
|                                      |                       | state before it is aborted at the local end                                                                                  |
| `highload_tcp_slow_start_after_idle` | `0`                   | If set, provide RFC2861 behavior and time out the congestion window after an idle period                                     |
| `highload_udp_rmem_min`              | `8192`                | Minimal size of receive buffer used by UDP sockets in moderation. The unit is byte                                           |
| `highload_udp_wmem_min`              | `8192`                | Minimal size of send buffer used by UDP sockets in moderation. The unit is byte                                              |
| `highload_tune_udp`                  | `false`               | Tune aforementioned udp parameters                                                                                           |


Dependencies
------------

No dependencies.


Example Playbook
----------------

See the [test playbook](tests/test.yml)


License
-------

BSD

## Contributing

 Send your suggestions and pull requests to https://github.com/kostyrevaa/ansible-role-highload/issues

 When send PR make sure your changes are backward-compatible.


Author Information
------------------

Aleksandr Kostyrev
