<?php

include( '../conf/webfrontend.conf' );

header( 'Content-Type: application/x-javascript; charset=utf-8' );
/*
$db = mysql_connect( $WEBCONFIG['databaseHost'], $WEBCONFIG['databaseUsername'], $WEBCONFIG['databasePassword'] );
mysql_select_db( 'sitefusion' );
$res = mysql_query( "SELECT `ident` FROM `processes` WHERE `id` = '".mysql_escape_string($_GET['sid'])."'" );
if(! $res )
	die( mysql_error() );

if( ! mysql_num_rows($res) )
	die( 'No session' );

$dbSession = mysql_fetch_assoc( $res );

if( $dbSession['ident'] != $_GET['ident'] )
	die( 'Not authorized' );
*/

$name = $_GET['name'];

if( preg_match( '/^[a-zA-Z0-9\-_\/]+$/', $name ) && is_file($WEBCONFIG['sitefusionPath'].'/'.$name.'.js') ) {
	include( $WEBCONFIG['sitefusionPath'].'/'.$name.'.js' );
}
else {
	die( 'No such library' );
}