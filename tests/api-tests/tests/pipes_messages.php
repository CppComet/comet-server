<?php


class test_pipes_messages extends testClass{

    public $count = 0;

    function test()
    {
        $rand = rand(1, 9999);

        $query = "SELECT * FROM pipes_messages WHERE name = 'p".$rand."'"; 
        $result = testQuery($this->link, $query);
         
        while($row = mysqli_fetch_assoc($result))
        {
            if($row['name'] != 'p'.$rand)
            {
                throw new Exception("Error name != ". $rand);
            } 
            echo json_encode($row)."\n";
        }
        
        testQuery($this->link, "DELETE FROM pipes_messages WHERE name = p23");
        testQuery($this->link, "INSERT INTO pipes_messages (name, event, message)VALUES('pipe_name', 'event_in_pipe', 'text message');");
        testQuery($this->link, "INSERT INTO pipes_messages (name, event, message)VALUES('pi! pe_name', '#even t_in_pipe', 'text message');");
        
        testQuery($this->link, 'SELECT * FROM pipes_messages WHERE name IN( "web_admins", "web_php_chat");'); 
        
    }
}


