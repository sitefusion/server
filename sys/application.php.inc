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
 * @package SiteFusion
 * @subpackage Application
*/


/**
 * This global function loads class libraries, PHP as well as JavaScript.
 * Calls to this function need to be in the global scope as well, allowing all classes
 * to be loaded before the application gets initialized. Class names need to be entered
 * without extensions, so when loading class 'controls' consisting of 'controls.php.inc'
 * and 'controls.js', just enter 'controls'. The type parameter defines the directory
 * to look in relative to the SiteFusion installation directory. This defaults to 'class'.
 * 
 * All built-in classes are loaded by default. To load a custom class from an application
 * directory, for example 'mycontrols' in application 'myapp', use 
 * LoadClass('mycontrols','app/myapp')
 *
 * @param string	$class	The name of the class without extension
 * @param string	$type	Directory name to load from
*/

function LoadClass( $class, $type = 'class' ) {
	$basename = $type . '/' . $class;
	$searchname = $type . '/' . $class;
	
	if( isset( $GLOBALS['include_path'] ) ) {
		$searchname = $GLOBALS['include_path'] . $searchname;
	}
	
	if( is_file( $searchname . '.php.inc' ) )
		include_once( $searchname . '.php.inc' );

	if( is_file( $searchname . '.js' ) && ! in_array($basename,$GLOBALS['LoadJS']) )
		array_push( $GLOBALS['LoadJS'], $basename );
	
	if( is_file( $searchname . '.css' ) && ! in_array($basename,$GLOBALS['LoadCSS']) )
		array_push( $GLOBALS['LoadCSS'], $basename );
}



// System classes
LoadClass( 'interface', 'sys' );
LoadClass( 'xulNode', 'sys' );
LoadClass( 'branchNode', 'sys' );
LoadClass( 'registry', 'sys' );
LoadClass( 'comm', 'sys' );

// User classes
LoadClass( 'windows' );
LoadClass( 'controls' );
LoadClass( 'layout' );
LoadClass( 'lists' );
LoadClass( 'menusAndPopups' );
LoadClass( 'tabsAndGrouping' );
LoadClass( 'textAndImages' );
LoadClass( 'helperElements' );
LoadClass( 'toolbars' );
LoadClass( 'windowStructure' );
LoadClass( 'trees' );
LoadClass( 'customTree' );
LoadClass( 'fileHandling' );
LoadClass( 'scripting' );
LoadClass( 'layoutEditor' );



/**
 * Represents events on the client and server side
 * 
 * Objects of the Event class represent events at both the client and the server side.
 * They can be constructed manually, for example to add additional properties for the 
 * handler(s), with a name and sourceObject parameter. They can then be fired on a node
 * through the fireLocalEvent method. The event handler(s) set on that node will then
 * be called, and the Event object is passed as the first argument.  An event handler 
 * can cancel the propagation of the event by setting the 'cancel' property to TRUE.
 * 
 * For events originating from the client, the sourceObject property will always be set
 * to the original source node of the event.
 * 
 * @property string	$name			Name of the event
 * @property Node	$sourceObject	Source node
 * @property bool	$cancel			Set to TRUE to cancel the event handling
*/

class Event
{
	public $name;
	public $sourceObject;
	public $cancel = FALSE;
	
	public function __construct( $name = NULL, $sourceObject = NULL ) {
		$this->name = $name;
		$this->sourceObject = $sourceObject;
	}
}


/**
 * The base class for all SiteFusion applications
 * 
 * The Application class is the parent class for all SiteFusion applications. Extending
 * classes should always define an init($arguments) method. This method is called when
 * the application process and root window have initialized, and thus is the place to
 * construct your basic user interface.
 * 
 * The root element of all applications is the window object. This is an object of the 
 * class XULWindow and is preconstructed and initialized before the init() function is 
 * called. Application->window contains a reference to this object, and all nodes added 
 * to the window and its child nodes will contain a reference to this window in the 
 * property Node->rootWindow, and to the application in the property Node->rootApplication.
 * 
 * @property int		$pid					POSIX id for the application process
 * @property XULWindow	$window					Root window and node of the application
 * @property string		$applicationName		Name of the application
 * @property Registry	$registry				[INTERNAL] Node registry for the Comm layer
 * @property array		$globalEventHandlers	[INTERNAL] Contains references to handlers of global events
 * @property array		$handlerFor				[INTERNAL] Contains references to objects that this object is a handler for
 * @property array		$yielderFor				[INTERNAL] Contains references to objects that this object is a yielder for
*/

class Application
{
	public $pid;
	public $window;
	public $registry;
	public $applicationName;
	public $globalEventHandlers = array();
	public $commandBuffer = array();
	public $handlerFor = array();
	public $yielderFor = array();
	
	public function initBaseApplication( $pid, $window ) {
		$this->pid = $pid;
		$this->window = $window;
		$window->application = $this;
		
		$this->registry = new Registry();
		$this->registry->add( NULL, $window );
		
		$window->setEvent( 'close', MSG_SEND, $this, 'onWindowClose' );
	}
	
	public function postInit() {
		$this->window->callMethod( 'center' );
	}
	
	public function sendCommand() {
		$args = func_get_args();
		
		for( $n = 0; $n < count($args); $n++ ) {
			if( is_array($args[$n]) )
				array_splice( $args, $n, 1, $args[$n--] );
			else if( $args[$n] instanceof CommObject )
				array_splice( $args, $n, 1, $args[$n--]->buffer );
		}
		
		if( $this->forwardCommandTo != NULL )
			$this->forwardCommandTo->sendCommand( join("\n",$args) );
		else
			array_push( $this->commandBuffer, join("\n",$args) );
		
		return TRUE;
	}
	
	public function setGlobalEventHandler( $event, $obj, $method ) {
		if( ! isset($this->globalEventHandlers[$event]) )
			$this->globalEventHandlers[$event] = array();
		
		$this->globalEventHandlers[$event][] = array( $obj, $method );
	}
	
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
	
	public function fireGlobalEvent( $event, $data = NULL ) {
		ApplicationProcess::SendGlobalEvent( $event, $data );
	}
	
	public function setTimeout( $timeout, $obj, $func ) {
		return $timer = new SiteFusionTimer( $timeout, $obj, $func );
	}
	
	public function setInterval( $timeout, $obj, $func ) {
		return $timer = new SiteFusionTimer( $timeout, $obj, $func, TRUE );
	}
	
	public function getService( $owner, $name ) {
		return ApplicationProcess::GetServiceByName( $owner, $name );
	}
	
	public function closeApplication() {
		$this->window->close();
	}
	
	public function onWindowClose() {
		ApplicationProcess::Shutdown();
	}
}