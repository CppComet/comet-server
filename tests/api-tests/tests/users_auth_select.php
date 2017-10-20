<?php


class test_users_auth_select extends testClass{

    public $count = 0;

    function test()
    {
        $rand = rand(1, 9999);

        $query = "INSERT INTO users_auth (id, hash )VALUES (".$rand.", 'hash".$rand."');";
        testQuery($this->link, $query);
          
        $result = testQuery($this->link, "select * from users_auth where id = ".$rand);
         
        while($row = mysqli_fetch_assoc($result))
        {
            if($row['id'] != $rand)
            { 
                throw new Exception("Error id != ". $rand);
            }

            if($row['hash'] != 'hash'.$rand)
            {
                throw new Exception("Error hash != `hash". $rand+"` ");
            }
            
            echo $row['id']."=".$row['hash']."\n";
        } 
    }
}


