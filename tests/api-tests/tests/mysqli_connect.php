<?php
 
class test_mysqli_connect extends testClass{
    
    function test()
    {     
        $link = mysqli_connect($this->opt['host'], $this->opt['user'], $this->opt['password'], $this->opt['api_version'], $this->opt['port']);
        if(!$link)
        {
            throw new Exception("Error[". mysqli_connect_errno()."]:". mysqli_connect_error());
        }
        
        if(mysqli_errno($link))
        {
            throw new Exception("Error[".mysqli_errno($link)."]:".  mysqli_error($link));
        }
        
        @mysqli_connect($this->opt['host'], $this->opt['user'], "noPass", $this->opt['api_version'], $this->opt['port']);
        @mysqli_connect($this->opt['host'], $this->opt['user'], null, null, $this->opt['port']);
        @mysqli_connect($this->opt['host'], "haproxy_check", null, null, $this->opt['port']);
    }
}
  
