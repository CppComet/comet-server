<?php

// More info about CppComet http://comet-server.org/doku.php/en
// More info about CometQL http://comet-server.org/doku.php/en:comet:cometql
// More info about auth in CppComet http://comet-server.org/doku.php/en:comet:authentication

header('Content-Type: text/html; charset=utf-8');
header("Access-Control-Allow-Origin: *");
if($_SERVER['REQUEST_METHOD'] == 'OPTIONS')
{
    exit();
}

ini_set('display_errors','on');
error_reporting (E_ALL);
session_start();
 
include './utils/timetest.php';
include './utils/testClass.php'; 

$count = 100;
if(isset($_GET['count']))
{
    $count = $_GET['count'];
    if($count > 100)
    {
        return "-1";
    }
}

$test = preg_replace("#[^A-z0-9\_\-]#usi", "", $_GET['test']);
if(!file_exists("./tests/".$test.".php"))
{ 
    die("Error 404");
}

include "./tests/".$test.".php";

if(file_exists("./conf.php"))
{
    include "./conf.php";
}

$className = "test_".$test;
$cppTest = new $className();
   
echo "RUN:".$test."\n";
$cppTest->init($serverConf);

$cppTest->start();
timeTest::test('cpp', 'test'); 
for($i =0; $i<$count; $i++)
{
    $cppTest->test();
}
$t = timeTest::test('cpp','test'); 
$t = $t[count($t) - 1];

$cppTest->stop();

echo json_encode([
    'count' => $count, 
    'time' => $t['time']
]);
