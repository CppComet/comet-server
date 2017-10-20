<?php


class test_pipes_settings extends testClass{
      
    function test()
    {
        $rand = rand(1, 9999);
        $randLen = rand(1, 90);
        testQuery($this->link, "INSERT INTO pipes_settings (name, length )VALUES ('name".$rand."', ".$randLen.");");
          
        $result = testQuery($this->link, "SELECT * FROM pipes_settings WHERE name = 'name".$rand."'");
         
        while($row = mysqli_fetch_assoc($result))
        {
            if($row['name'] != "name".$rand)
            {
                throw new Exception("Error name != name". $rand);
            } 
            
            if($row['length'] != $randLen)
            {
                throw new Exception("Error length != ". $randLen);
            } 
            
            echo json_encode($row)."\n";
        }
        
        testQuery($this->link, "DELETE FROM pipes_settings WHERE name = 'name".$rand."' "); 

        $result = testQuery($this->link, "SELECT * FROM pipes_settings WHERE name = 'name".$rand."'"); 
        if(mysqli_num_rows($result) != 0)
        {
            throw new Exception("Error mysqli_num_rows != 0");
        } 
        
        testQuery($this->link, "INSERT INTO pipes_settings (name, length )VALUES ('name".$rand."max', 9999999);");
        testQuery($this->link, "INSERT INTO pipes_settings (name, length )VALUES ('name".$rand."min', 0);");
    } 
}
 

 