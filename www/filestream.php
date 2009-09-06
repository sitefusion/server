<?php

ignore_user_abort( TRUE );

include( '../conf/webfrontend.conf' );
include( 'functions.php' );


if( ! (isset($_GET['cid']) && isset($_GET['sid']) && isset($_GET['ident']) ) )
	die( 'No parameters' );

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

WriteCommand( $socket, 'FILE', array('cid' => $_GET['cid']) );

$cmd = ReadCommand( $socket );
socket_close($socket);

ob_end_clean();

//var_dump( $cmd );
header( 'Content-type: ' . $cmd->contentType );
echo $cmd->data;
