<?php


class users_auth_insert extends testClass{
     
    public $count = 0;
    
    function test()
    {
        $this->count++;
        $query = "INSERT INTO users_auth (id, hash )VALUES (".$this->count.", 'hash".$this->count."');";
        mysqli_query($this->link, $query);
    } 
}
 
$cppTest = new users_auth_insert();
 