<?php


class test_users_auth extends testClass{
      
    function test()
    {
        $rand = rand(1, 9999);
        testQuery($this->link, "INSERT INTO users_auth (id, hash )VALUES (".$rand.", 'hashTE".$rand."');");
          
        $result = testQuery($this->link, "SELECT * FROM users_auth WHERE id = '".$rand."'");
         
        while($row = mysqli_fetch_assoc($result))
        {
            if($row['id'] != $rand)
            {
                throw new Exception("Error id != ". $rand);
            } 
            
            if($row['hash'] != 'hashTE'.$rand)
            {
                throw new Exception("Error hash != hashTE". $rand);
            } 
            
            echo json_encode($row)."\n";
        }
        
        testQuery($this->link, "DELETE FROM users_auth WHERE id = ".$rand.""); 

        $result = testQuery($this->link, "SELECT * FROM users_auth WHERE id = '".$rand."'"); 
        if(mysqli_num_rows($result) != 0)
        {
            throw new Exception("Error mysqli_num_rows != 0");
        } 
    } 
}
 

 