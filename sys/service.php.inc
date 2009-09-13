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
 * @package API
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