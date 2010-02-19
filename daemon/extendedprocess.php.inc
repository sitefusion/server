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
 * @package Daemon
 * @subpackage Extension
*/



class ExtendedSiteFusionProcess extends SiteFusionProcess
{
	static protected $SessionID;
	static protected $ConnectedDebugger = NULL;
	static protected $Timers = array();
	static protected $ParentSocket;
	static protected $EventPipe;
	static protected $LastKeepalive;
	static protected $DebugSocket = NULL;
	static protected $Groups = array();
	static public $ApplicationPath = NULL;
	static public $DebugPaused = FALSE;
	static public $DebugTicks = FALSE;
	static public $DebugAppTicks = FALSE;
	static public $DebugLastTick = '';
	static public $PathCache = array();
	
	static protected function ListenToDebugger() {
		$read = array(self::$ConnectedDebugger);
		$write = NULL;
		$except = NULL;
		
		if( socket_select($read, $write, $except, 0) < 1 ) {
			return;
		}
		
		if( count($read) ) {
			$socket = $read[0];
			
			try {
				$cmd = self::ReadCommand( $socket );
			
				switch ( $cmd->command ) {
					case 'PAUSE': self::$DebugPaused = TRUE; break;
					case 'RESUME': self::$DebugPaused = FALSE; break;
					case 'TICKSON':
						try { register_tick_function( 'tickfunction' ); } catch ( SFException $ex ) {}
						self::$DebugTicks = TRUE;
						self::$DebugAppTicks = (bool) intval($cmd->appTicks);
						break;
					case 'TICKSOFF':
						try { unregister_tick_function( 'tickfunction' ); } catch ( SFException $ex ) {}
						self::$DebugTicks = FALSE;
						self::$DebugAppTicks = (bool) intval($cmd->appTicks);
						break;
				}
			}
			catch ( SFException $ex ) {
				try {
					socket_close( self::$ConnectedDebugger );
				}
				catch ( SFException $ex ) {}
				
				self::$ConnectedDebugger = NULL;
				return;
			}
		}
	}
	
	static protected function PollForDebugger() {
		try {
			$client = socket_accept( self::$DebugSocket );
		}
		catch( SFException $ex ) {
			return FALSE;
		}
		return $client;
	}
	
	static public function SendGlobalEvent( $event, $args = NULL ) {
		if( !$event instanceof Event )
			$event = new Event( (string) $event );
		else {
			$event = clone $event;
			unset($event->sourceObject);
		}
		
		$event->sourceSession = self::$SessionID;
		
		self::WriteCommand( self::$EventPipe, 'EVENT', NULL, serialize( array($event,$args) ) );
	}
	
	static public function SendDaemonShutdownFlags( $shutdown, $graceful, $restart ) {
		self::WriteCommand( self::$ParentSocket, 'ADMIN', array( 'action' => 'shutdown', 'shutdown' => ($shutdown ? '1':'0'), 'graceful' => ($graceful ? '1':'0'), 'restart' => ($restart ? '1':'0') ) );
	}
	
	static public function GetDaemonStatus() {
		self::WriteCommand( self::$ParentSocket, 'ADMIN', array( 'action' => 'getstatus' ) );
		$cmd = self::ReadCommand( self::$ParentSocket );
		
		return array(
			'shutdown' => (bool) intval($cmd->shutdown),
			'graceful' => (bool) intval($cmd->graceful),
			'restart' => (bool) intval($cmd->restart),
			'memory' => intval($cmd->memory),
			'memory_peak' => intval($cmd->memory_peak)
		);
	}
	
	static public function SendSessionSignal( $pid, $sig ) {
		self::WriteCommand( self::$ParentSocket, 'ADMIN', array( 'action' => ($sig == SIGKILL ? 'kill':'term'), 'pid' => $pid ) );
	}
	
	static public function SendBroadcastMessage( $message, $session = NULL ) {
		self::WriteCommand( self::$ParentSocket, 'ADMIN', array( 'action' => 'message', 'to' => ($session === NULL ? 'all':(string)$session) ), $message );
	}
	
	static public function StoreTimer( $timer ) {
		$time = microtime(TRUE) + ($timer->timeout()/1000);
		
		for( $n = 0; $n < count(self::$Timers); $n++ ) {
			if( self::$Timers[$n][0] > $time )
				break;
		}
		
		array_splice( self::$Timers, $n, 0, array(array($time,$timer)) );
	}
	
	static public function RemoveTimer( $timer ) {
		for( $n = 0; $n < count(self::$Timers); $n++ ) {
			if( self::$Timers[$n][1] === $timer ) {
				array_splice( self::$Timers, $n, 1 );
				break;
			}
		}
	}
	
	static public function CreateJob( $job ) {
		self::WriteCommand( self::$ParentSocket, 'JOB', array('action'=>'addjob'), serialize($job) );
		$cmd = self::ReadCommand( self::$ParentSocket );
		if( $cmd->command != 'OK' )
			throw new SFException( 'Error communicating with master process' );
		return TRUE;
	}
	
	static public function ChangeJob( $id, $job ) {
		self::WriteCommand( self::$ParentSocket, 'JOB', array('action' => 'changejob','id' => (int)$id), serialize($job) );
		$cmd = self::ReadCommand( self::$ParentSocket );
		if( $cmd->command != 'OK' )
			throw new SFException( 'Error communicating with master process' );
		return TRUE;
	}
	
	static public function DeleteJob( $id ) {
		self::WriteCommand( self::$ParentSocket, 'JOB', array('action' => 'deletejob','id' => (int)$id) );
		$cmd = self::ReadCommand( self::$ParentSocket );
		if( $cmd->command != 'OK' )
			throw new SFException( 'Error communicating with master process' );
		return TRUE;
	}
	
	static public function GetJobList() {
		self::WriteCommand( self::$ParentSocket, 'JOB', array('action' => 'getlist') );
		$cmd = self::ReadCommand( self::$ParentSocket );
		
		return unserialize( $cmd->data );
	}
	
	static public function GetDaemonProcessList() {
		self::VerifyDatabaseState();
		$res = self::DatabaseQuery( "SELECT * FROM `sitefusion`.`processes`" );
		$num = mysql_num_rows( $res );
		$ret = array();
		
		for( $n = 0; $n < $num; $n++ ) {
			$data = mysql_fetch_assoc( $res );
			unset( $data['ident'] );
			$ret[] = $data;
		}
		
		return $ret;
	}
	
	static public function CreateService( $srv ) {
		self::WriteCommand( self::$ParentSocket, 'SERVICE', array('action'=>'addservice'), serialize($srv) );
		$cmd = self::ReadCommand( self::$ParentSocket );
		if( $cmd->command != 'OK' )
			throw new SFException( 'Error communicating with master process' );
		return TRUE;
	}
	
	static public function ChangeService( $id, $srv ) {
		self::WriteCommand( self::$ParentSocket, 'SERVICE', array('action' => 'changeservice','id' => (int)$id), serialize($srv) );
		$cmd = self::ReadCommand( self::$ParentSocket );
		if( $cmd->command != 'OK' )
			throw new SFException( 'Error communicating with master process' );
		return TRUE;
	}
	
	static public function DeleteService( $id ) {
		self::WriteCommand( self::$ParentSocket, 'SERVICE', array('action' => 'deleteservice','id' => (int)$id) );
		$cmd = self::ReadCommand( self::$ParentSocket );
		if( $cmd->command != 'OK' )
			throw new SFException( 'Error communicating with master process' );
		return TRUE;
	}
	
	static public function GetServiceList() {
		self::WriteCommand( self::$ParentSocket, 'SERVICE', array('action' => 'getlist') );
		$cmd = self::ReadCommand( self::$ParentSocket );
		
		return unserialize( $cmd->data );
	}
	
	static public function JoinGroup( $group ) {
		if( in_array($group,self::$Groups) )
			return;
		
		self::WriteCommand( self::$ParentSocket, 'GROUP', array('action' => 'join', 'group' => (string)$group) );
		self::$Groups[] = $group;
	}

	static public function PartGroup( $group ) {
		if( !in_array($group,self::$Groups) )
			return;
		
		self::WriteCommand( self::$ParentSocket, 'GROUP', array('action' => 'part', 'group' => (string)$group) );
		array_splice( self::$Groups, array_search($group,self::$Groups), 1 );
	}
	
	static public function GetGroups() {
		return self::$Groups;
	}
	
	static public function ToDebuggerShowStack( $bt, $pause = FALSE, $crash = FALSE ) {
		if( ! self::$ConnectedDebugger )
			return;
		
		$view = array();
		
		for( $n = 0; $n < count($bt); $n++ ) {
			$file = (isset($bt[$n]['file']) ? $bt[$n]['file'] : '');
			$line = (string) (isset($bt[$n]['line']) ? $bt[$n]['line'] : '');
			$function = (isset($bt[$n]['function']) ? $bt[$n]['function'] : NULL);
			$class = (isset($bt[$n]['class']) ? $bt[$n]['class'] : NULL);
			$object = (isset($bt[$n]['object']) ? $bt[$n]['object'] : NULL);
			$type = (isset($bt[$n]['type']) ? $bt[$n]['type'] : NULL);
			$args = (isset($bt[$n]['args']) ? $bt[$n]['args'] : array());
			
			$call = array();
			
			if( $type == '::' )
				$call[] = $class . ' :: ' . $function;
			elseif( $type == '->' ) {
				$call[] = get_class($object) . ' -> ' . $function;
			}
			else
				$call[] = $function;
			
			$call[] = '(';

			for( $a = 0; $a < count($args); $a++ ) {
				if( $a != 0 )
					$call[] = ',';
				
				switch ( gettype($args[$a]) ) {
					case 'NULL':
						$call[] = 'NULL';
						break;
					case 'boolean':
						$call[] = ($args[$a] ? 'TRUE':'FALSE');
						break;
					case 'string':
						if( strlen($args[$a]) > 25 )
							$show = substr($args[$a],0,25).'...';
						else
							$show = $args[$a];
						$call[] = '"'.$show.'"';
						break;
					case 'object':
						$call[] = get_class($args[$a]);
						break;
					case 'array':
						$call[] = 'Array('.count($args[$a]).')';
						break;
					default:
						$call[] = (string) $args[$a];
				}
			}
			
			$call[] = ')';
			
			$row = array( $file, $line, implode( ' ', $call) );
			
			$view[] = $row;
		}
		
		$send = array(
			'stack' => $view,
			'crash' => $crash,
			'file' => $bt[0]['file'],
			'line' => $bt[0]['line'],
			'function' => $bt[0]['function']
		);
		
		try {
			self::WriteCommand( self::$ConnectedDebugger, 'STACK', array( 'pause' => ($pause ? '1':'0'), 'crash' => ($crash ? '1':'0') ), serialize($send) );
		
			if( $pause ) {
				self::$DebugPaused = TRUE;
				if( class_exists('ApplicationProcess') ) {
					ApplicationProcess::FlushRevComm();
					ApplicationProcess::ApplicationLoop( TRUE );
				}
				elseif( class_exists('ServiceProcess') ) {
					ServiceProcess::ServiceLoop( TRUE );
				}
			}
		}
		catch ( SFException $ex ) {}
	}
	
	static public function GetOption( $name ) {
		$protected = array(
			'databaseUsername',
			'databasePassword'
		);
		
		if( in_array( $name, $protected ) || !isset(self::$Options[$name]) )
			return NULL;
		
		return self::$Options[$name];
	}
	
	static public function Shutdown() {
		self::$Shutdown = TRUE;
	}
}



class SiteFusionTimer
{
	private $timeout;
	private $object;
	private $function;
	private $interval;
	private $cancelled = FALSE;
	
	public function __construct( $timeout, $obj, $func, $interval = FALSE ) {
		$this->timeout = $timeout;
		$this->object = $obj;
		$this->function = $func;
		$this->interval = $interval;
		
		ExtendedSiteFusionProcess::StoreTimer( $this );
	}
	
	public function timeout() {
		return $this->timeout;
	}
	
	public function trigger() {
		if( is_object($this->object) )
			call_user_func_array( array(&$this->object,$this->function), array($this) );
		else
			call_user_func_array( array($this->object,$this->function), array($this) );
		
		if( $this->interval && ! $this->cancelled )
			ExtendedSiteFusionProcess::StoreTimer( $this );
	}
	
	public function cancel() {
		ExtendedSiteFusionProcess::RemoveTimer( $this );
		$this->cancelled = TRUE;
	}
}


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


function sfdebug() {
	$bt = debug_backtrace();
	
	ExtendedSiteFusionProcess::ToDebuggerShowStack( $bt, FALSE );
}

function sfdebugpause() {
	$bt = debug_backtrace();
	
	ExtendedSiteFusionProcess::ToDebuggerShowStack( $bt, TRUE );
}

function tickfunction() {
	if( ! ExtendedSiteFusionProcess::$DebugTicks ) return;
	
	$bt = debug_backtrace();
	
	if( isset($bt[0]['file']) ) {
		if( dirname($bt[0]['file']) == dirname(__FILE__) )
			return;
		
		if( ExtendedSiteFusionProcess::$DebugLastTick == $bt[0]['file'].$bt[0]['line'] )
			return;
		
		$file = dirname($bt[0]['file']);
		$appPath = ExtendedSiteFusionProcess::GetOption('sitefusionPath') . '/app/' . dirname(ExtendedSiteFusionProcess::$ApplicationPath);
		if( ! isset(ExtendedSiteFusionProcess::$PathCache[$file]) )
			ExtendedSiteFusionProcess::$PathCache[$file] = realpath($file);
		if( ! isset(ExtendedSiteFusionProcess::$PathCache[$appPath]) )
			ExtendedSiteFusionProcess::$PathCache[$appPath] = realpath($appPath);
		
		if( ExtendedSiteFusionProcess::$DebugAppTicks && strpos( ExtendedSiteFusionProcess::$PathCache[$file], ExtendedSiteFusionProcess::$PathCache[$appPath] ) === FALSE )
			return;
		
		ExtendedSiteFusionProcess::$DebugLastTick = $bt[0]['file'].$bt[0]['line'];
		
		ExtendedSiteFusionProcess::ToDebuggerShowStack( $bt, TRUE );
	}
}
