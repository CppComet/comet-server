<?php


class test_users_time extends testClass{

    public $count = 0;

    function test()
    {
        $rand = rand(1, 9999);

        $query = "INSERT INTO users_time (id, hash )VALUES (".$rand.", 'hash".$rand."');";
        mysqli_query($this->link, $query);
           
        mysqli_query($this->link, "delete from users_time where id = 3;");
        
        $result = testQuery($this->link, "select * from users_time where id in(".$rand.")");
         
        while($row = mysqli_fetch_assoc($result))
        {
            if($row['id'] != $rand)
            { 
                throw new Exception("Error id != ". $rand);
            } 
        }
    }
}


