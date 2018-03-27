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
           
        
        mysqli_query($this->link, "SELECT * FROM no_pipes_messages WHERE name IN( 'name".$rand."');"); 
        
        mysqli_query($this->link, "INSERT INTO no_pipes_messages (id, hash )VALUES (".$rand.", 'hash".$rand."');");
        mysqli_query($this->link, "DELETE FROM no_pipes_messages where id = 12");
        
        mysqli_query($this->link, "INSERT INTO no_pipes_messages (id, hash, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12 )VALUES "
                . "(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);");
        
        mysqli_query($this->link, "ab");
        
        
        
        
        mysqli_query($this->link, "select database CometQL_v00A");
        
        mysqli_query($this->link, "use CometQL_v1");
        mysqli_query($this->link, "use CometQL_v00A");
        mysqli_query($this->link, "SHOW DATABASES");
        // @todo select users_in_pipes
        // @todo insert users_in_pipes
        // @todo show columns
        // @todo delete_from_users_time
        // @todo delete_from_users_in_pipes
        // 
        // 
        // @todo delete_from_users_auth так чтоб отключало подключённого клиента
        // @todo select_from_users_auth так чтоб запросило подключённого клиента
        
         
        $bigArr = [];
        for($j = 0; $j < 2; $j++)
        {
            for($i = 0; $i < 100; $i++)
            {
                $bigArr[] = $j*$i;
            }
            mysqli_query($this->link, "select * from users_time where id in (". join(",", $bigArr).")");
        }
        
        
    }
}


