<?php


class test_status extends testClass{
      
    function test()
    { 
        testQuery($this->link, "show status");   
        testQuery($this->link, "show global status");
        testQuery($this->link, "show ram status");
        testQuery($this->link, "show avg status");
        testQuery($this->link, "show filesystem status");
        testQuery($this->link, "show processlist"); 
        testQuery($this->link, "show uptime status");
        
        
        testQuery($this->link, "show tables"); 
        testQuery($this->link, "show databases"); 
        
        testQuery($this->link, "show no-flag status");
        testQuery($this->link, "SHOW SESSION VARIABLES"); 
        testQuery($this->link, "SHOW VARIABLES"); 
    } 
}
 

 