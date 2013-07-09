<?php
class SiteFusionService
{
	public $serviceId;
	public $socket;
	public $lastKeepalive;
	private $callbackHandlers = array();
	
	public function __construct( $id, $socket ) {
		$this->serviceId = $id;
		$this->socket = $socket;
		$this->lastKeepalive = time();
	}
	
	public function __call( $name, $args ) {
		$cmd = ApplicationProcess::CallServiceFunction( $this, $name, $args );
		
		$ret = unserialize( $cmd->data );
		
		if( is_object($ret) && $ret instanceof SFException )
			throw $ret;
		
		return $ret;
	}
	
	public function setCallback( $name, $obj, $method ) {
		$this->callbackHandlers[$name] = array($obj,$method);
	}
	
	public function removeCallback( $name ) {
		unset( $this->callbackHandlers[$name] );
	}
	
	public function handleCallback( $name, $args ) {
		if( ! isset($this->callbackHandlers[$name]) )
			return;
		
		$handler = $this->callbackHandlers[$name];
		if( is_object($handler[0]) )
			return call_user_func_array( array(&$handler[0], $handler[1]), $args );
		else
			return call_user_func_array( array($handler[0], $handler[1]), $args );
	}
}