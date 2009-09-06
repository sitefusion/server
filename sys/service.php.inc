<?php

/**
 * @package SiteFusion
 * @subpackage Service
*/


class Service
{
	public $globalEventHandlers = array();
	public $callHandlers = array();
	
	final public function setCallHandler( $name, $obj, $method ) {
		$this->callHandlers[$name] = array($obj,$method);
	}
	
	final public function removeCallHandler( $name ) {
		unset( $this->callHandlers[$name] );
	}
	
	final public function handleCall( $name, $args ) {
		if( ! isset($this->callHandlers[$name]) )
			return;
		
		$handler = $this->callHandlers[$name];
		if( is_object($handler[0]) )
			return call_user_func_array( array(&$handler[0], $handler[1]), $args );
		else
			return call_user_func_array( array($handler[0], $handler[1]), $args );
	}
	
	final public function setGlobalEventHandler( $event, $obj, $method ) {
		if( ! isset($this->globalEventHandlers[$event]) )
			$this->globalEventHandlers[$event] = array();
		
		$this->globalEventHandlers[$event][] = array( $obj, $method );
	}
	
	final public function removeGlobalEventHandler( $event, $obj, $method ) {
		if( ! isset($this->globalEventHandlers[$event]) )
			return;
		
		$list = $this->globalEventHandlers[$event];
		for( $n = 0; $n < count($this->globalEventHandlers[$event]); $n++ ) {
			if( $this->globalEventHandlers[$event][$n][0] === $obj && $this->globalEventHandlers[$event][$n][0] === $method ) {
				array_splice( $this->globalEventHandlers[$event], $n, 1 );
				return;
			}
		}
	}
	
	final public function handleGlobalEvent( $event, $data ) {
		if(! is_array($data) )
			$data = array($data);
		
		if( $event instanceof Event ) {
			$eventobj = $event;
			$event = $eventobj->name;
		}
		else $eventobj = new Event( $event, $this );

		array_unshift( $data, $eventobj );
		
		if( isset($this->globalEventHandlers[$event]) ) {
			foreach ( $this->globalEventHandlers[$event] as $handler ) {
				if( is_object($handler[0]) )
					call_user_func_array( array(&$handler[0], $handler[1]), $data );
				else
					call_user_func_array( array($handler[0], $handler[1]), $data );
				
				if( $eventobj->cancel == TRUE )
					break;
			}
		}
		
		return $eventobj;
	}
	
	final public function fireGlobalEvent( $event, $data = NULL ) {
		if( !class_exists('ServiceProcess') )
			return;
		
		ServiceProcess::SendGlobalEvent( $event, $data );
	}
	
	final public function setTimeout( $timeout, $obj, $func ) {
		if(! class_exists('ServiceProcess') )
			return;
		
		return $timer = new SiteFusionTimer( $timeout, $obj, $func );
	}
	
	final public function setInterval( $timeout, $obj, $func ) {
		if(! class_exists('ServiceProcess') )
			return;
		
		return $timer = new SiteFusionTimer( $timeout, $obj, $func, TRUE );
	}
}