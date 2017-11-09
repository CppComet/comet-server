<?php


class test_status extends testClass{
      
    function test()
    { 
        testQuery($this->link, "show status");   
        mysqli_query($this->link, "show global status");
        mysqli_query($this->link, "show ram status");
        mysqli_query($this->link, "show avg status");
        mysqli_query($this->link, "show filesystem status");
        mysqli_query($this->link, "show processlist"); 
        mysqli_query($this->link, "show uptime status");
        
        
        testQuery($this->link, "show tables"); 
        testQuery($this->link, "show databases"); 
        
        mysqli_query($this->link, "show no-flag status");
        mysqli_query($this->link, "SHOW SESSION VARIABLES"); 
        mysqli_query($this->link, "SHOW VARIABLES"); 
    } 
}
 

 