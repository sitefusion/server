<?php

/**
 * @package SiteFusion
 * @subpackage Application
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



class Event
{
	public $name;
	public $sourceObject;
	public $cancel = FALSE;
	public $returnValue = NULL;
	
	public function __construct( $name = NULL, $sourceObject = NULL ) {
		$this->name = $name;
		$this->sourceObject = $sourceObject;
	}
}


class Application
{
	public $pid;
	public $window;
	public $registry;
	public $applicationName;
	public $commLastAction;
	public $commTimeout = 300;
	public $shutdownError = NULL;
	public $name;
	public $globalEventHandlers = array();
	public $commandBuffer = array();
	public $handlerFor = array();
	public $yielderFor = array();
	protected $forwardCommandTo = NULL;
	
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
	
	public function flushCommandBuffer() {
		ob_clean();
		
		$n = -1;
		while( ++$n < sizeof($this->commandBuffer) ) {
			echo "\n", $this->commandBuffer[$n], "\n";
		}
		
		$this->commandBuffer = array();
	}
	
	public function forwardCommands( $dest = NULL ) {
		$this->forwardCommandTo = $dest;
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
		if( !class_exists('ApplicationProcess') )
			return;
		
		ApplicationProcess::SendGlobalEvent( $event, $data );
	}
	
	public function setTimeout( $timeout, $obj, $func ) {
		if(! class_exists('ApplicationProcess') )
			return;
		
		return $timer = new SiteFusionTimer( $timeout, $obj, $func );
	}
	
	public function setInterval( $timeout, $obj, $func ) {
		if(! class_exists('ApplicationProcess') )
			return;
		
		return $timer = new SiteFusionTimer( $timeout, $obj, $func, TRUE );
	}
	
	public function getService( $owner, $name ) {
		if( class_exists('ApplicationProcess') )
			return ApplicationProcess::GetServiceByName( $owner, $name );
	}
	
	public function closeApplication() {
		$this->window->close();
	}
	
	public function destroyProcess() {
		$pid = $this->pid;
		unset( $_SESSION['proc'][$pid] );
		unset( $_SESSION['lastop'][$pid] );
		unset( $_SESSION['timeout'][$pid] );

		return TRUE;
	}
	
	public function onWindowClose() {
		if( class_exists('ApplicationProcess') )
			ApplicationProcess::Shutdown();
	}
}