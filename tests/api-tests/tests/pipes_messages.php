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
        mysqli_query($this->link, "INSERT INTO pipes_messages (name, event, message)VALUES('pi! pe_name', '#even t_in_pipe', 'text message');");
        
        testQuery($this->link, 'SELECT * FROM pipes_messages WHERE name IN( "web_admins", "web_php_chat");'); 
        
        
        testQuery($this->link, "INSERT INTO pipes_messages (name, event, message)VALUES('name".$rand."', 'event_in_pipe', 'text message".$rand."');");
        testQuery($this->link, "INSERT INTO pipes_settings (name, length )VALUES ('name".$rand."', 0);");
        testQuery($this->link, "INSERT INTO pipes_settings (name, length )VALUES ('name".$rand."', 10);");
        testQuery($this->link, "INSERT INTO pipes_messages (name, event, message)VALUES('name".$rand."', 'event_in_pipe', 'text message".$rand."');");
        testQuery($this->link, "INSERT INTO pipes_settings (name, length )VALUES ('name".$rand."', 0);");
        
        mysqli_query($this->link, "INSERT INTO pipes_messages (event, message)VALUES('event_in_pipe', 'text message".$rand."');");
        mysqli_query($this->link, "INSERT INTO pipes_messages (name, event, message)VALUES('e', 'event_in_pipe', 'text message".$rand."');");
        mysqli_query($this->link, "INSERT INTO pipes_messages (name, event, message)VALUES('event_in_pipe', '0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789', 'text message".$rand."');");
        mysqli_query($this->link, "INSERT INTO pipes_messages (name, event, message)VALUES('0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123456789', '', 'text message".$rand."');");
        
        testQuery($this->link, "INSERT INTO pipes_messages (name, event, message)VALUES('name".$rand."', 'event_in_pipe', 'text message".$rand."');");
        
        $result = testQuery($this->link, "SELECT * FROM pipes_messages WHERE name IN( 'name".$rand."');"); 
        while($row = mysqli_fetch_assoc($result))
        {
            if($row['message'] != 'text message'.$rand)
            {
                throw new Exception("Error message != 'text message". $rand."'");
            }
            echo json_encode($row)."\n";
        }
        
        
        testQuery($this->link, "INSERT INTO pipes_messages (name, event, message)VALUES('trust_name".$rand."', 'event_in_pipe', 'text message".$rand."');");
        
        
        // @todo sql_delete_from_pipes_messages
    }
}


