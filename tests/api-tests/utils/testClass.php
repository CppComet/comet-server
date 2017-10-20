<?php
  
class testClass{
    
    public $link;
    public $opt;
    
    function init($opt){
        $this->opt = $opt;
    }
    
    function start()
    {
        $this->link = @mysqli_connect(
                $this->opt['host'],
                $this->opt['user'],
                $this->opt['password'],
                $this->opt['api_version'],
                $this->opt['port']);
        if(!$this->link)
        {
            echo "Error[". mysqli_connect_errno()."]:". mysqli_connect_error();
            exit();
        }
        
        if(mysqli_errno($this->link))
        {
            echo "Error[".mysqli_errno($this->link)."]:".  mysqli_error($this->link);
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

function testQuery($link, $query)
{
    $res = mysqli_query($link, $query);
    if(mysqli_errno($link))
    { 
        throw new Exception("Error[".mysqli_errno($link)."]:".  mysqli_error($link)); 
    }
    
    return $res;
}