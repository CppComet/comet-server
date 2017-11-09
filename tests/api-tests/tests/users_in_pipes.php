<?php


class test_users_in_pipes extends testClass{

    public $count = 0;

    function test()
    {
        $rand = rand(1, 9999);

        $query = "SELECT * FROM users_in_pipes WHERE name = 'p".$rand."'"; 
        $result = testQuery($this->link, $query);
         
        while($row = mysqli_fetch_assoc($result))
        {
            if($row['name'] != 'p'.$rand)
            {
                throw new Exception("Error name != ". $rand);
            } 
            echo json_encode($row)."\n";
        }
        
        mysqli_query($this->link, "DELETE FROM users_in_pipes WHERE name = p23");
        mysqli_query($this->link, "INSERT INTO users_in_pipes (name, event, message)VALUES('pipe_name', 'event_in_pipe', 'text message');");
        mysqli_query($this->link, "INSERT INTO users_in_pipes (name, event, message)VALUES('pi! pe_name', '#even t_in_pipe', 'text message');");
        
        testQuery($this->link, 'SELECT * FROM users_in_pipes WHERE name IN( "web_admins", "web_php_chat");');  
    }
}


