<?php


class test_common extends testClass{

    public $count = 0;

    function test()
    {
        mysqli_query($this->link, "select database CometQL_v1");
        mysqli_query($this->link, "select systemvarible CometQL_v1");
        
        $rand = rand(1, 9999); 
        mysqli_query($this->link, "INSERT INTeO users_time (id, hash )VALUES (".$rand.", 'hash".$rand."');");
          
            mysqli_query($this->link, "select * from users_time where hash = '123' ");
            
        $bigArr = [];
        for($j = 0; $j < 10; $j++)
        {
            for($i = 0; $i < 1000; $i++)
            {
                $bigArr[] = $j*$i;
            }
            mysqli_query($this->link, "select * from users_time where id in (". join(",", $bigArr).")");
        }
        
        
        
        // @todo select users_in_pipes
        // @todo insert users_in_pipes
        // @todo show columns
        // @todo delete_from_users_time
        // @todo delete_from_users_in_pipes
    }
}


