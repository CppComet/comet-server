<?php


class test_users_messages extends testClass{

    public $count = 0;

    function test()
    {
        $rand = rand(1, 9999);

        $query = "SELECT * FROM users_messages WHERE id = '".$rand."'"; 
        $result = testQuery($this->link, $query);
         
        while($row = mysqli_fetch_assoc($result))
        {
            if($row['name'] != 'p'.$rand)
            {
                throw new Exception("Error name != ". $rand);
            } 
            echo json_encode($row)."\n";
        }
        
        testQuery($this->link, "DELETE FROM users_messages WHERE id = 23"); 
        testQuery($this->link, " INSERT INTO users_messages (id, event, message)VALUES (2, 'event', 'message');");
        
    }
}


