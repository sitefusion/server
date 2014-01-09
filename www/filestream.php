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
ob_implicit_flush();

include( '../conf/webfrontend.conf' );
include( 'functions.php' );

try {
	if( ! (isset($_GET['cid']) && isset($_GET['sid']) && isset($_GET['ident']) ) )
		throw new Exception( 'No parameters' );
	$sid = $_GET['sid'];
	$dbSession = GetSessionFromSID($sid, $WEBCONFIG['databaseUsername'], $WEBCONFIG['databasePassword'], (isset($WEBCONFIG['databaseDSN']) ? $WEBCONFIG['databaseDSN'] : ""), $WEBCONFIG['databaseHost'], $WEBCONFIG['databaseName']);
	
	if( $dbSession['ident'] != $_GET['ident'] )
		throw new Exception( 'Not authorized' );
	
	$port = (int) $dbSession['port'];
}
catch ( Exception $ex ) {
	echo $ex->getMessage();
	exit(1);
}

function getConnection() {
	global $WEBCONFIG, $port;
	
	try {
		$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
		if ($socket === false)
		    throw new Exception( "socket_create() failed: reason: " . socket_strerror(socket_last_error()) );
		
		$result = socket_connect($socket, $WEBCONFIG['address'], $port );
		if ($result === false)
			throw new Exception( "socket_connect() failed.\nReason: ($result) " . socket_strerror(socket_last_error($socket)) );
		
		return $socket;
	}
	catch ( Exception $ex ) {
		echo $ex->getMessage();
		exit(1);
	}
}

try {
	$socket = getConnection();
	WriteCommand( $socket, 'FILE', array( 'cid' => $_GET['cid'], 'action' => 'start' ) );
	$cmd = ReadCommand( $socket );
	socket_close( $socket );
	
	$size = $cmd->size;
	header( 'Content-type: ' . $cmd->contentType );
	header( 'Content-length: ' . $size );
	
	$sentBytes = 0;
	
	while( $sentBytes < $size ) {
		$socket = getConnection();
		
		WriteCommand( $socket, 'FILE', array( 'cid' => $_GET['cid'], 'action' => 'data' ) );
		$cmd = ReadCommand( $socket );
		
		socket_close( $socket );
		
		if( strlen($cmd->data) == 0 )
			throw new Exception( 'File transfer error' );
		
		echo $cmd->data;
		
		if( connection_aborted() )
			break;
		
		$sentBytes += strlen($cmd->data);
	}
	
	$socket = getConnection();
	
	WriteCommand( $socket, 'FILE', array( 'cid' => $_GET['cid'], 'action' => 'end' ) );
	
	socket_close($socket);
}
catch ( Exception $ex ) {
	$socket = getConnection();
	
	WriteCommand( $socket, 'FILE', array( 'cid' => $_GET['cid'], 'action' => 'end' ) );
	
	socket_close($socket);
	
	echo $ex->getMessage();
	exit(1);
}
