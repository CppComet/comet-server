<?php


class test_pipes extends testClass{

    public $count = 0;

    function test()
    {
        $rand = rand(1, 9999);

        $query = "SELECT * FROM pipes WHERE name IN( 'web_admins', 'web_php_chat".$rand."', 'php_chat".$rand."');"; 
        $result = testQuery($this->link, $query);
         
        while($row = mysqli_fetch_assoc($result))
        {
            if( !in_array($row['name'], ['web_admins', "web_php_chat".$rand, "php_chat".$rand]))
            {
                throw new Exception("Error name['".$row['name']."'] != ". $rand);
            } 
            echo $row['name']."=".$row['users']."\n";
            // @todo добавить проверку количества онлайн
        }
        
        mysqli_query($this->link, "DELETE FROM pipes WHERE name = p23"); 
        mysqli_query($this->link, "INSERT INTO pipes (name, event, message)VALUES('pi! pe_name', '#even t_in_pipe', 'text message');"); 
    }
}


