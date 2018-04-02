<?php
header('Access-Control-Allow-Origin: *');
$host    = "localhost";
$port    = 1501;
$message = $_SERVER['QUERY_STRING'];


$socket = socket_create(AF_INET, SOCK_STREAM, 0) or die("Could not create socket\n");
$result = socket_connect($socket, $host, $port) or die("Could not connect to server\n");  
socket_write($socket, $message, strlen($message)) or die("Could not send data to server\n");
$result = socket_read ($socket, 4096) or die("Could not read server response\n");
socket_close($socket);
echo $result
?>
