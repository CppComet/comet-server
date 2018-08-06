
IF(NOT /etc/comet-server)
    execute_process( COMMAND groupadd cppcomet
                     COMMAND useradd -c "cppcomet user" -s /bin/false -G cppcomet cppcomet ) 
endif()

    #execute_process(COMMAND chown -R cppcomet:cppcomet /etc/comet-server)
    execute_process(COMMAND systemctl daemon-reload)
    execute_process(COMMAND systemctl enable comet.service) 

MESSAGE("Installed")