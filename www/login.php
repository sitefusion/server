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

if( ! (isset($_GET['app']) && isset($_GET['args']) && isset($_GET['clientid'])) )
	ReturnError( 'input_error' );

try {
	$input = fopen( 'php://input', 'r' );
	$content = '';
	while( strlen($content) < $_SERVER['CONTENT_LENGTH'] ) {
		$content .= fread( $input, 8192 );
	}
	fclose( $input );

	$loginObj = (array) json_decode( $content );

	if( ! (isset($loginObj['username']) && isset($loginObj['password'])) )
		throw new Exception( 'No username or password given' );
	
	$username = $loginObj['username'];
	$password = $loginObj['password'];
	$clientVersion = $loginObj['appInfo']->version;
}
catch ( Exception $ex ) {
	ReturnError( 'input_error', $ex->getMessage() );
}


try {
	$socket = @socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
	if ($socket === false)
	    throw new Exception( "socket_create() failed: reason: " . socket_strerror(socket_last_error()) );

	$result = @socket_connect($socket, $WEBCONFIG['address'], $WEBCONFIG['port'] );
	if ($result === false)
	    throw new Exception( "socket_connect() failed.\nReason: ($result) " . socket_strerror(socket_last_error($socket)) );
}
catch ( Exception $ex ) {
	ReturnError( 'server_offline', $ex->getMessage() );
}

try {
	$loginParam = array(
		'app' => $_GET['app'],
		'args' => $_GET['args'],
		'username' => $username,
		'password' => $password,
		'ip' => $_SERVER['REMOTE_ADDR'],
		'serverHost' => ((isset($_SERVER['HTTP_HOST']) && $_SERVER['HTTP_HOST'] != '') ? $_SERVER['HTTP_HOST'] : $_SERVER['SERVER_NAME']),
		'serverPath' => dirname($_SERVER['PHP_SELF']),
		'secure' => ((isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] != '') ? 1 : 0),
		'clientVersion' => $clientVersion
	);

	WriteCommand( $socket, 'STARTAPP', array( 'clientid' => $_GET['clientid'] ) );
	
	$loginParam['extensionInfo'] = (array) $loginObj['extensionInfo'];
	$loginParam['appInfo'] = (array) $loginObj['appInfo'];
	$loginParam['platformInfo'] = (array) $loginObj['platformInfo'];
	$loginParam['cmdlineArgs'] = (array) $loginObj['cmdlineArgs'];
	WriteCommand( $socket, 'LOGIN', array('loginObject' => '1'), json_encode($loginParam) );
	
	$cmd = ReadCommand( $socket );
	$loginResponse = (array) json_decode($cmd->data);
	
	socket_close($socket);
	
	if( $loginResponse['success'] )
		ReturnResult( $cmd->data );
	else
		ReturnError( $loginResponse['error'] );
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