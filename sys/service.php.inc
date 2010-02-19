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


include( 'sys/event.php.inc' );


/**
 * The base class for all SiteFusion services
 * 
 * The Service class is the parent class for all SiteFusion services. Extending
 * classes should always define an init($arguments) method. This method is called when
 * the service process has initialized, and thus is the place to setup your basic
 * service functionality.
*/

class Service
{
	public $globalEventHandlers = array();
	public $callHandlers = array();
	
	
	public function setDefaultEventGroup() {
		$parts = explode( '/', ServiceProcess::$ApplicationPath );
		$this->joinEventGroup( $parts[0] );
	}
	
	
	public function setCallHandler( $name, $obj, $method ) {
		$this->callHandlers[$name] = array($obj,$method);
	}
	
	public function removeCallHandler( $name ) {
		unset( $this->callHandlers[$name] );
	}
	
	public function handleCall( $name, $args ) {
		if( ! isset($this->callHandlers[$name]) )
			return;
		
		$handler = $this->callHandlers[$name];
		if( is_object($handler[0]) )
			return call_user_func_array( array(&$handler[0], $handler[1]), $args );
		else
			return call_user_func_array( array($handler[0], $handler[1]), $args );
	}
	
	
	/**
	 * Set a handler for a global event
	 * 
	 * Global events originate from other SiteFusion processes. A global event will only reach
	 * your service when it was fired from an application or service in the same application
	 * group. Global events are similar to local events, and can also carry additional data inside
	 * the Event object. The handler will receive the event object as the first argument, and
	 * event parameters as additional arguments.
	 * 
	 * @param string		$event	The event name
	 * @param object|string	$obj	Handler object or class
	 * @param string		$method	Name of the method to call on the handler object or class
	*/
	
	public function setGlobalEventHandler( $event, $obj, $method ) {
		if( ! isset($this->globalEventHandlers[$event]) )
			$this->globalEventHandlers[$event] = array();
		
		$this->globalEventHandlers[$event][] = array( $obj, $method );
	}
	
	
	/**
	 * Remove a handler for a global event
	 * 
	 * Removes a handler for the given global event with the same syntax as setGlobalEventHandler
	 * 
	 * @param string		$event	The event name
	 * @param object|string	$obj	Handler object or class
	 * @param string		$method	Name of the handler method on the handler object or class
	*/
	
	public function removeGlobalEventHandler( $event, $obj, $method ) {
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
	
	
	/**
	 * [INTERNAL FUNCTION] Handles incoming global events
	 * 
	 * This function is called by the ApplicationProcess when it receives a global event from the
	 * daemon.
	 * 
	 * @param Event	$event	Event object
	 * @param array $data	Event parameters
	 * @return Event	event object
	*/
	
	public function handleGlobalEvent( $event, $data ) {
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
	
	
	/**
	 * Fires a global event
	 * 
	 * This method fires a global event that will reach all SiteFusion processes within the
	 * same application group.
	 * 
	 * @param Event|string	$event	Event name or object
	 * @param array			$data	Event parameters
	*/
	
	public function fireGlobalEvent( $event, $data = NULL ) {
		if( !class_exists('ServiceProcess') )
			return;
		
		ServiceProcess::SendGlobalEvent( $event, $data );
	}
	
	
	/**
	 * Joins an Event Group
	 * 
	 * Applications and services that have joined the same event groups will receive each other's
	 * global events
	 * 
	 * @param string	$group		Name of the group
	*/
	
	public function joinEventGroup( $group ) {
		ServiceProcess::JoinGroup( $group );
	}
	
	
	/**
	 * Parts an Event Group
	 * 
	 * Applications and services that have joined the same event groups will receive each other's
	 * global events
	 * 
	 * @param string	$group		Name of the group
	*/
	
	public function partEventGroup( $group ) {
		ServiceProcess::PartGroup( $group );
	}
	
	
	/**
	 * Get a list of joined Event Groups
	 * 
	 * Applications and services that have joined the same event groups will receive each other's
	 * global events
	*/
	
	public function getEventGroups() {
		return ServiceProcess::GetGroups();
	}
	
	
	/**
	 * Sets a server-based timeout callback
	 * 
	 * This function is similar to the JavaScript window.setTimeout() function. It dispatches
	 * a timer for a certain amount of milliseconds, and fires the handler method when the
	 * timer expires. The timer can be cancelled by calling the cancel() method on the returned 
	 * timer object.
	 * 
	 * @param int			$timeout	Timeout in milliseconds
	 * @param object|string	$obj		Object or class name
	 * @param string		$func		Method to call on the object or class when the timer expires
	 * @return SiteFusionTimer	Timer object created
	*/
	
	public function setTimeout( $timeout, $obj, $func ) {
		if(! class_exists('ServiceProcess') )
			return;
		
		return $timer = new SiteFusionTimer( $timeout, $obj, $func );
	}
	
	
	/**
	 * Sets a server-based interval callback
	 * 
	 * This function is similar to the JavaScript window.setInterval() function. It dispatches
	 * a timer for a certain amount of milliseconds, and fires the handler method when the
	 * timer expires. The timer then resets and recycles, until it is cancelled by calling the
	 * cancel() method on the returned timer object.
	 * 
	 * @param int			$timeout	Timeout in milliseconds
	 * @param object|string	$obj		Object or class name
	 * @param string		$func		Method to call on the object or class when the timer expires
	 * @return SiteFusionTimer	Timer object created
	*/
	
	public function setInterval( $timeout, $obj, $func ) {
		if(! class_exists('ServiceProcess') )
			return;
		
		return $timer = new SiteFusionTimer( $timeout, $obj, $func, TRUE );
	}
}