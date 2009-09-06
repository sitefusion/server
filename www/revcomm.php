<?php

ignore_user_abort( TRUE );

include( '../conf/webfrontend.conf' );
include( 'functions.php' );

header( 'Content-Type: text/html; charset=utf-8' );

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

WriteCommand( $socket, 'REVCOMM' );

$timeout = $WEBCONFIG['revCommTimeout'];
$startTime = time();

while( (time() - $startTime) < $timeout ) {
	
	$read = array( $socket );
	$write = NULL;
	$except = NULL;
	
	if( socket_select( $read, $write, $except, 0 ) > 0 ) {
		$cmd = ReadCommand( $socket );
		echo $cmd->data;
		break;
	}
	
	if( connection_status() != 0 )
		break;
	
	usleep( 100000 );
}

socket_close($socket);
