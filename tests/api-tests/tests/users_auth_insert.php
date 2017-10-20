<?php


class test_users_auth_insert extends testClass{
     
    public $count = 0;
    
    function test()
    {
        $this->count++;
        $query = "INSERT INTO users_auth (id, hash )VALUES (".$this->count.", 'hash".$this->count."');";
        mysqli_query($this->link, $query);
        if(mysqli_errno($this->link))
        { 
            throw new Exception("Error[".mysqli_errno($this->link)."]:".  mysqli_error($this->link)." in ".$query); 
        }

    } 
}
 

 