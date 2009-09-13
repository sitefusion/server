<?php
// - - - - - - - - - - - - - BEGIN LICENSE BLOCK - - - - - - - - - - - - -
// Version: MPL 1.1/GPL 2.0/LGPL 2.1
//
// The contents of this file are subject to the Mozilla Public License Version
// 1.1 (the "License"); you may not use this file except in compliance with
// the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/
//
// Software distributed under the License is distributed on an "AS IS" basis,
// WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
// for the specific language governing rights and limitations under the
// License.
//
// The Original Code is sitefusion.sourceforge.net code.
//
// The Initial Developer of the Original Code is
// FrontDoor Media Group.
// Portions created by the Initial Developer are Copyright (C) 2009
// the Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Nikki Auburger <nikki@thefrontdoor.nl> (original author)
//   Tom Peeters <tom@thefrontdoor.nl>
//
// - - - - - - - - - - - - - - END LICENSE BLOCK - - - - - - - - - - - - -


/**
 * @package Webfrontend
*/


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
