<?php
 
class test_mysqli_connect extends testClass{
    
    function test()
    {     
        $l = mysqli_connect($this->opt['host'], $this->opt['user'], $this->opt['password'], $this->opt['api_version']);
        echo mysqli_error($l)."\n";
        mysqli_close($l); 
    }
}
 
$cppTest = new test_mysqli_connect();
