<?php

function WriteCommand( $socket, $cmd, $param = NULL, $data = NULL ) {
	if(! is_array($param) )
		$param = array();
	
	if( $data !== NULL )
		$param['DATALENGTH'] = strlen($data);
	
	$msg = $cmd;
	foreach ( $param as $key => $val ) {
		$msg .= ' '.$key.'='.$val;
	}
	
	$msg .= "\n";
	
	if( socket_write( $socket, $msg, strlen($msg) ) === FALSE )
		die( 'WriteCommand: socket_write() failed: '.socket_strerror(socket_last_error($socket)) );
	if( $data !== NULL ) {
		if( socket_write( $socket, $data, strlen($data) ) === FALSE )
			die( 'WriteCommand: socket_write() failed: '.socket_strerror(socket_last_error($socket)) );
	}
}

function ReadCommand( $socket ) {
	$cmddata = socket_read( $socket, 4096, PHP_NORMAL_READ );
	if( $cmddata === FALSE )
		die( 'ReadCommand: socket_read() failed: '.socket_strerror(socket_last_error($socket)) );

	$data = NULL;
	$cmd = explode( ' ', rtrim($cmddata) );

	$ret = new StdClass;
	$ret->command = array_shift( $cmd );

	foreach ( $cmd as $param ) {
		list($key,$val) = explode( '=', $param );
		$ret->{$key} = $val;
	}

	if( isset($ret->DATALENGTH) ) {
		$dl = (int) $ret->DATALENGTH;
	
		$data = '';
		while( strlen($data) < $dl ) {
			$moredata = socket_read( $socket, 2048, PHP_BINARY_READ );
			if( $moredata === FALSE )
				die( 'ReadCommand: socket_read() failed: '.socket_strerror(socket_last_error($socket)) );
			$data .= $moredata;
		}
	}

	$ret->data = $data;

	return $ret;
}

?>