<?php

include( '../conf/webfrontend.conf' );
include( 'functions.php' );

if( ! (isset($_GET['app']) && isset($_GET['args']) && isset($_POST['username']) && isset($_POST['password'])) )
	die();

try {
	ob_start();
	$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
	if ($socket === false)
	    throw new Exception( "socket_create() failed: reason: " . socket_strerror(socket_last_error()) );

	$result = socket_connect($socket, $WEBCONFIG['address'], $WEBCONFIG['port'] );
	if ($result === false)
	    throw new Exception( "socket_connect() failed.\nReason: ($result) " . socket_strerror(socket_last_error($socket)) );

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
	ob_end_clean();
}
catch ( Exception $ex ) {
	echo json_encode( array(
		"success" => FALSE,
		"error" => 'Could not connect to the SiteFusion server. Please try again later.'
	) );
	exit();
}

if( $cmd->success ) {
	echo json_encode( array(
		"success" => TRUE,
		"includeJs" => explode(',', $cmd->includeJs ),
		"application" => $cmd->app,
		"args" => $cmd->args,
		"user" => $cmd->user,
		"ident" => $cmd->ident,
		"sid" => $cmd->sid
	) );
}
else {
	echo json_encode( array(
		"success" => FALSE,
		"error" => $cmd->error
	) );
}

socket_close($socket);

?>