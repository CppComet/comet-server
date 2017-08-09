<?php
  
class testClass{
    
    public $link;
    public $opt;
    
    function init($opt){
        $this->opt = $opt;
    }
    
    function start()
    {  
        $this->link = mysqli_connect(
                $this->opt['host'],
                $this->opt['user'],
                $this->opt['password'],
                $this->opt['api_version']);
        if(!$this->link || mysqli_errno($this->link))
        {
            echo "Error:".  mysqli_error($this->link);
            exit();
        }
    }
    
    function test(){
        
    }
    
    function stop()
    {
        mysqli_close($this->link);
    }
    
}