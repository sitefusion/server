<?php
class ServiceClient
{
	public $socket;
	public $started;
	public $lastKeepalive;
	public $connected = TRUE;
	
	public function callBack( $name, $args = NULL ) {
		if( $args === NULL ) $args = array();
		
		return ServiceProcess::SendClientCallback( $this, $name, $args );
	}
}