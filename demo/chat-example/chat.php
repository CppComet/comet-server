<?php

$host = "app.comet-server.ru";
$user = "15";
$password = "lPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8";

$comet = mysqli_connect($host, $user, $password, "CometQL_v1");

if(mysqli_errno($comet))
{
    echo "Error:".mysqli_error($link);
}

$msg = Array( "name" => $_POST["name"], "text"  => $_POST["text"] );
$msg = json_encode($msg);
$msg = mysqli_real_escape_string($comet, $msg);

$query = "INSERT INTO pipes_messages (name, event, message)" .
  "VALUES('simplechat', 'newMessage', '".$msg."')";
 
mysqli_query($comet, $query); 

if(mysqli_errno($comet))
{
    echo "Error:".mysqli_error($comet);
} 
else
{
    echo "ok";
}
