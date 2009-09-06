<?php

include( '../conf/webfrontend.conf' );

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


$name = $_GET['name'];

if( preg_match( '/^[a-zA-Z0-9\-_\/]+\.[a-zA-Z]{3}$/i', $name ) && is_file($WEBCONFIG['sitefusionPath'].'/'.$name) ) {
	switch ( strtolower(substr($name,-3)) ) {
		case 'jpg': header( 'Content-Type: image/jpeg' ); break;
		case 'gif': header( 'Content-Type: image/gif' ); break;
		case 'png': header( 'Content-Type: image/png' ); break;
		default: header( 'Content-Type: application/octet-stream' );
	}
	
	readfile( $WEBCONFIG['sitefusionPath'].'/'.$name );
}
else {
	die( 'No such file' );
}