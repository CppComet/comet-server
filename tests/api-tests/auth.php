<?php
// More info about CppComet http://comet-server.org/doku.php/en
// More info about CometQL http://comet-server.org/doku.php/en:comet:cometql
// More info about auth in CppComet http://comet-server.org/doku.php/en:comet:authentication

header('Content-Type: text/html; charset=utf-8');
header("Access-Control-Allow-Origin: *");


if(file_exists("./conf.php"))
{
    include "./conf.php";
}

?>
mysql -h<?php echo $serverConf['host']; ?> -u<?php echo $serverConf['user']; ?> -p<?php echo $serverConf['password']; ?> -D<?php echo $serverConf['api_version']; ?> -P<?php echo $serverConf['port']; ?> --skip-ssl
<?php

// We connect to the comet server with login and password for the access demo (you can get your data for connection after registration at comet-server.com)
// Login 15 
// Password lPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8
// CometQL_v1 database
$link = mysqli_connect($serverConf['host'], $serverConf['user'], $serverConf['password'], $serverConf['api_version'], $serverConf['port']);
 if(!$link)
{
    echo "Error[". mysqli_connect_errno()."]:". mysqli_connect_error();
    exit();
}

if(mysqli_errno($link))
{
    echo "Error[".mysqli_errno($link)."]:".  mysqli_error($link);
    exit();
}

$user_id = $_GET['user_id'];
$user_key = $_GET['user_key'];

$q = "INSERT INTO users_auth (id, hash)VALUES('".mysqli_real_escape_string($link, $user_id)."', '".mysqli_real_escape_string($link, $user_key)."' );";

// The table users_auth contains data of user’s authorizing on comet server.
// More info about table “users_auth” https://comet-server.com/wiki/doku.php/en:comet:cometql#table_users_auth
mysqli_query (  $link, $q);

if(mysqli_errno($link))
{
    echo mysqli_errno($link).": ".mysqli_error($link);
}
else
{
    echo "id:".$user_id."\n"; 
    echo "key:".$user_key."\n"; 
    echo "query:".$q; 
}
