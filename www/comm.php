<?php

ignore_user_abort( TRUE );

include( '../conf/webfrontend.conf' );
include( 'functions.php' );

header( 'Content-Type: application/x-javascript; charset=utf-8' );

$db = mysql_connect( $WEBCONFIG['databaseHost'], $WEBCONFIG['databaseUsername'], $WEBCONFIG['databasePassword'] );
mysql_select_db( 'sitefusion' );
$res = mysql_query( "SELECT * FROM `processes` WHERE `id` = '".mysql_escape_string($_GET['sid'])."'" );
if(! $res )
	die( mysql_error() );

if( ! mysql_num_rows($res) )
	die( 'No session' );

$dbSession = mysql_fetch_assoc( $res );

if( $dbSession['ident'] != $_GET['ident'] )
	die( 'Not authorized' );

$port = (int) $dbSession['port'];

$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
if ($socket === false)
    die( "socket_create() failed: reason: " . socket_strerror(socket_last_error()) );

$result = socket_connect($socket, $WEBCONFIG['address'], $port );
if ($result === false)
    die( "socket_connect() failed.\nReason: ($result) " . socket_strerror(socket_last_error($socket)) );

for( $n = 1; isset($_POST[(string)$n]); $n++ ) {
	$_POST[(string)$n] = stripslashes($_POST[(string)$n]);
}

WriteCommand( $socket, 'COMM', NULL, http_build_query($_POST) );

$cmd = ReadCommand( $socket );
socket_close($socket);

echo $cmd->data;
