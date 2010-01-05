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


include( '../conf/webfrontend.conf' );
include( 'functions.php' );

if( ! (isset($_GET['app']) && isset($_GET['args']) && isset($_GET['clientid']) && isset($_POST['username']) && isset($_POST['password'])) )
	ReturnError( 'input_error' );

try {
	$socket = @socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
	if ($socket === false)
	    throw new Exception( "socket_create() failed: reason: " . socket_strerror(socket_last_error()) );

	$result = @socket_connect($socket, $WEBCONFIG['address'], $WEBCONFIG['port'] );
	if ($result === false)
	    throw new Exception( "socket_connect() failed.\nReason: ($result) " . socket_strerror(socket_last_error($socket)) );
}
catch ( Exception $ex ) {
	ReturnError( 'server_offline' );
}

try {	
	WriteCommand( $socket, 'STARTAPP', array( 'clientid' => $_GET['clientid'] ) );
	WriteCommand( $socket, 'LOGIN',
		array(
			'app' => $_GET['app'],
			'args' => $_GET['args'],
			'username' => $_POST['username'],
			'password' => $_POST['password'],
			'ip' => $_SERVER['REMOTE_ADDR']
		)
	);

	$cmd = ReadCommand( $socket );
	$cmd->success = ($cmd->success ? TRUE:FALSE);
	
	socket_close($socket);
	
	if( $cmd->success )
		ReturnResult( json_encode($cmd) );
	else
		ReturnError( $cmd->error );
}
catch ( Exception $ex ) {
	// Give the daemon some time to collect error output
	usleep( 500000 );
	
	try {
		$socket = @socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
		if ($socket === false)
		    throw new Exception( "socket_create() failed: reason: " . socket_strerror(socket_last_error()) );

		$result = @socket_connect($socket, $WEBCONFIG['address'], $WEBCONFIG['port'] );
		if ($result === false)
		    throw new Exception( "socket_connect() failed.\nReason: ($result) " . socket_strerror(socket_last_error($socket)) );
		
		WriteCommand( $socket, 'GETERROR', array( 'clientid' => $_GET['clientid'] ) );
		$cmd = ReadCommand( $socket );
		
		if( $cmd->found )
			ReturnError( 'php_error', $cmd->data );
		else
			ReturnError( 'empty_error' );
	}
	catch ( Exception $ex ) {
		ReturnError( 'unspecified_error' );
	}
}

?>