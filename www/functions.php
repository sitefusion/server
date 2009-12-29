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
	$cmddata = @socket_read( $socket, 4096, PHP_NORMAL_READ );
	if( $cmddata === FALSE )
		throw new Exception( 'ReadCommand: socket_read() failed: '.socket_strerror(socket_last_error($socket)) );

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
			$sbuf = @socket_read( $socket, 2048, PHP_BINARY_READ );
			if( $sbuf === FALSE ) {
				if( socket_last_error($socket) != 35 )
					throw new SFException( 'ReadCommand: socket_read() failed: '.socket_strerror(socket_last_error($socket)) );
			}
			$data .= $sbuf;
		}
	}

	$ret->data = $data;

	return $ret;
}

?>