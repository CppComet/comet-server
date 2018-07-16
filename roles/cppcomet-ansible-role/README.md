
[![Gitter](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/CppComet/Lobby?utm_source=share-link&utm_medium=link&utm_campaign=share-link)

# Ansible role for CppComet 
 
Comet server is written in C++, for storing data using MySQL.   
[CppComet repository](https://github.com/CppComet/comet-server) and [documentation](http://comet-server.com/)

# Variables and their default values:

[vars documentation in comet.ini file](https://comet-server.com/wiki/doku.php/en:comet:ini-file)
```
cppcomet_node_name: n1
cppcomet_useQueryLoger: false
cppcomet_base_dir: static
cppcomet_buf_size: 10384
cppcomet_fragment_buf_max_size: 10384
cppcomet_answer_buf_size: 1000000
cppcomet_db_host: localhost
cppcomet_db_user: root
cppcomet_db_password: root
cppcomet_db_name: comet_db
cppcomet_db_port: 3306
cppcomet_ws_ip: 0.0.0.0
cppcomet_ws_thread_num: 4
cppcomet_statistics: 10
cppcomet_ws_port: 8087
cppcomet_cometql_ip: 0.0.0.0
cppcomet_cometql_thread_num: 4 
cppcomet_cometql_port: 3307
cppcomet_mysqlproxy_ip: 0.0.0.0
cppcomet_mysqlproxy_thread_num: 4
cppcomet_mysqlproxy_port: 3308
cppcomet_log_level: 200
cppcomet_password: root
cppcomet_fragment_buf_size: 600
cppcomet_git_branch: master 
```

# Example Playbook


Install role `ansible-galaxy install CppComet.CppComet-ansible-role`

Playbook example:

```
- hosts: all 
  roles:  
    - { role: CppComet }  
```
    
# Support 

[![Gitter](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/CppComet/Lobby?utm_source=share-link&utm_medium=link&utm_campaign=share-link)
Free support in gitter chat and [RU forum](http://community.comet-server.com/t/ru-forum) and [EN forum](http://community.comet-server.com/t/ru-forum)

# License

MIT / BSD