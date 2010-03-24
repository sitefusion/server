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
 * @subpackage Application
*/


include_once( 'extendedprocess.php.inc' );


class ApplicationProcess extends ExtendedSiteFusionProcess
{
	static public $Started;
	static private $ApplicationStarted = FALSE;
	static public $Application;
	static public $JSLibraries = array();
	static public $IncludePath = NULL;
	static public $ClientID = NULL;
	static private $LastContact;
	static private $Headers;
	static private $RevCommSocket = NULL;
	static private $EventQueue = array();
	static private $NextCommOutput = NULL;
	static private $ConnectedServices = array();
	static private $DebugWindows = array();
	
	static public $ServerHost;
	static public $ServerSecure;
	
	static public $AppInfo;
	static public $PlatformInfo;
	static public $ExtensionInfo;
	static public $CommandLineArgs;
	static public $ClientVersion;
	
	static public $ExtensionPolicy = array(
		'require' => array(),
		'disable' => array()
	);
	
	static public function StartServer( $client, $parentSocket, $eventPipe, $loginCmd, $port, $clientid ) {
		if( self::$ApplicationStarted || self::$ProcessType )
			return;
		
		self::$ParentSocket = $parentSocket;
		self::$EventPipe = $eventPipe;
		self::$ProcessType = 'app';
		self::$ApplicationStarted = TRUE;
		self::$ClientID = $clientid;

		if( self::$Options['debug'] )
			self::$DebugSocket = self::OpenUnixSocket( self::$Options['runtimeDirectory'] . '/' . 'debug'.posix_getpid().'.sock' );
		
		self::$Socket = self::OpenSocket( self::$Options['address'], $port );
		
		try {
			$param = self::AuthorizeLogin( $loginCmd );
		}
		catch ( SFException $ex ) {
			$err = fopen( 'php://stderr', 'w' );
			fwrite( $err, $ex->getFormattedReport() );
			fflush( $err );
			fclose( $err );
			socket_close( $client );
			self::StopApplicationServer();
			exit(1);
		}
		
		if( (! $param['success']) || count($param['extensionPolicy']) ) {
			self::WriteCommand( $client, 'LOGIN', NULL, json_encode($param) );
			socket_close( $client );
			self::StopApplicationServer();
			exit();
		}
		
		self::$ApplicationPath = $param['app'];
		self::$ServerHost = $param['server'];
		self::$ServerSecure = $param['secure'];
		
		self::SetSignalHandlers();
		
		$param['sid'] = self::$SessionID = md5( time() . $port . mt_rand() );
		self::WriteCommand( $client, 'LOGIN', NULL, json_encode($param) );
		socket_close( $client );
		
		$extendedInfo = array();
		$extendedInfo['AppInfo'] = (isset(self::$AppInfo) ? self::$AppInfo : NULL);
		$extendedInfo['PlatformInfo'] = (isset(self::$PlatformInfo) ? self::$PlatformInfo : NULL);
		$extendedInfo['ExtensionInfo'] = (isset(self::$ExtensionInfo) ? self::$ExtensionInfo : NULL);
		$extendedInfo['CommandLineArgs'] = (isset(self::$CommandLineArgs) ? self::$CommandLineArgs : NULL);
		$extendedInfo['ClientVersion'] = self::$ClientVersion;
		
		self::WriteCommand( self::$ParentSocket, 'LOGIN', $param, serialize($extendedInfo) );
		
		self::$Port = $port;
		self::$Started = time();
		self::$LastKeepalive = time();
		self::$LastContact = time();
		
		self::InitializeApplication();
		
		self::UpdateDatabase();
		self::ApplicationLoop();
		exit();
	}
	
	static private function StopApplicationServer() {
		try { socket_close( self::$Socket ); } catch ( SFException $ex ) {}
		try { socket_close( self::$ParentSocket ); } catch ( SFException $ex ) {}
		try { socket_close( self::$EventPipe ); } catch ( SFException $ex ) {}
		foreach ( self::$ConnectedServices as $service ) {
			try {
				self::WriteCommand( $service->socket, 'CLOSE' );
				socket_close( $service->socket );
			} catch ( SFException $ex ) {}
		}
		if( self::$Options['debug'] ) {
			try {
				socket_close( self::$DebugSocket );
			} catch ( SFException $ex ) {}
		}
	}
	
	static protected function ApplicationLoop( $subloop = FALSE ) {
		try {
			while(1) {
				if( self::$Shutdown || time() - self::$LastContact > self::$Options['sessionTimeout'] ) {
					self::StopApplicationServer();
					return;
				}
			
				if( time() - self::$LastKeepalive > self::$Options['keepaliveInterval'] ) {
					self::WriteCommand( self::$ParentSocket, 'LIVE' );
					self::$LastKeepalive = time();
				}
			
				foreach ( self::$ConnectedServices as $service ) {
					if( time() - $service->lastKeepalive > self::$Options['keepaliveInterval'] ) {
						self::WriteCommand( $service->socket, 'LIVE' );
						$service->lastKeepalive = time();
					}
				}
				
				if( self::$Options['debug'] ) {
					if( self::$ConnectedDebugger )
						self::ListenToDebugger();
					else
						self::$ConnectedDebugger = self::PollForDebugger();
				}
				
				// Update database every two seconds
				if( (!isset($lastTime)) || time() - $lastTime > 1 ) {
					self::UpdateDatabase();
					$lastTime = time();
				}
				
				if(! self::$DebugPaused ) {
					self::ListenToParent();
					self::ListenToServices();
					self::ProcessEventQueue();
					self::CheckTimers();
					self::FlushRevComm();
			
					if( $client = self::PollForClient() ) {
						try {
							$cmd = self::ReadCommand( $client );

							switch ( $cmd->command ) {
								case 'COMM': self::HandleComm( $client, $cmd ); break;
								case 'REVCOMM': self::HandleRevComm( $client ); break;
								case 'FILE': self::HandleFile( $client, $cmd ); break;
							}
						}
						catch ( SFException $ex ) {
							throw $ex;
						}
					}
					//else usleep( self::$Options['daemonCycleInterval'] );
				}
				else usleep( self::$Options['daemonCycleInterval'] );
				
				if( $subloop && !self::$DebugPaused )
					return;
			}
		}
		catch ( SFException $ex ) {
			if( self::$ConnectedDebugger ) {
				self::ToDebuggerShowStack( $ex->getTrace(), FALSE, $ex->getMessage() . ' in ' . $ex->getFile() . ' at line ' . $ex->getLine() );
			}
			self::Log( 'Uncaught Exception in application ('.self::$SessionID.'): ' . $ex->getFormattedReport() );
			self::StopApplicationServer();
			return;
		}
	}
	
	static private function ListenToParent() {
		while( 1 ) {
			$read = array(self::$ParentSocket,self::$EventPipe);
			$write = NULL;
			$except = NULL;
			
			try {
				if( socket_select($read, $write, $except, 0) < 1 )
					return;
			}
			catch ( SFException $ex ) {
				return;
			}
			
			if( count($read) ) {
				foreach ( $read as $socket ) {
					try {
						$cmd = self::ReadCommand( $socket );
				
						switch ( $cmd->command ) {
							case 'EVENT': self::HandleEvent( $cmd ); break;
							case 'MESSAGE': self::HandleMessage( $cmd ); break;
						}
					}
					catch ( SFException $ex ) {
						if( self::$ConnectedDebugger )
							throw $ex;
					
						self::Log('Uncaught Exception in application ('.self::$SessionID.'): ' . $ex->getFormattedReport());
					}
				}
			}
			else return;
		}
	}
	
	static private function ListenToServices() {
		$read = array();
		$write = NULL;
		$except = NULL;
		
		foreach ( self::$ConnectedServices as $s ) {
			$read[] = $s->socket;
		}
		
		if( ! count($read) )
			return;
		
		try {
			if( socket_select($read, $write, $except, 0) < 1 )
				return;
		}
		catch ( SFException $ex ) { return; }
		
		foreach ( $read as $socket ) {
			$service = self::GetServiceBySocket( $socket );
			
			try {
				$cmd = self::ReadCommand( $socket );
				
				switch ( $cmd->command ) {
					case 'CALLBACK':	self::HandleServiceCmdCallback( $service, $cmd ); break;
				}
			}
			catch ( SFException $ex ) {}
		}
	}
	
	static private function PollForClient() {
		try {
			$read = array( self::$Socket );
			$write = NULL;
			$except = NULL;
			if( socket_select( $read, $write, $except, 0, self::$Options['daemonCycleInterval'] ) > 0 ) {
				$client = socket_accept( self::$Socket );
			}
			else {
				return NULL;
			}
		}
		catch( SFException $ex ) {
			//self::Log( 'PollForClient: ' . $ex->getMessage() );
			return FALSE;
		}
		return $client;
	}
	
	static private function ReconstructNodeReferences( &$data ) {
		for( $n = 0; $n < count($data); $n++ ) {
			if( is_array($data[$n]) )
				self::ReconstructNodeReferences( $data[$n] );
			else if( is_object($data[$n]) && isset($data[$n]->__sfNode) )
				$data[$n] = (isset(self::$Application->registry->all[$data[$n]->__sfNode]) ? self::$Application->registry->all[$data[$n]->__sfNode] : NULL);
		}
	}
	
	static private function HandleComm( $client, $cmd ) {
		if( ! isset(self::$Application) )
			throw new SFException( 'Invalid COMM request, not logged in yet' );
		
		try {
			ob_start();
			
			$post = json_decode( $cmd->data );
			$app = self::$Application;
			if(!$post)
				throw new SFException("Invalid JS data: ".$cmd->data);
			
			foreach ( $post as $event ) {
				self::ReconstructNodeReferences( $event );
				
				$id = array_shift( $event );
				$func = array_shift( $event );

				if( isset($app->registry->all[$id]) )
					$app->registry->all[$id]->commandHandler( $func, $event );
			}
			
			ob_end_clean();
		}
		catch ( SFException $ex ) {
			if( self::$ConnectedDebugger )
				throw $ex;
			
			self::$NextCommOutput = $ex->getFormattedReport();
			
			if( self::$Options['debug'] )
				self::Log( 'Uncaught Exception in application ('.self::$SessionID.'): ' . self::$NextCommOutput );
		}
		
		$output = (self::$NextCommOutput ? self::$NextCommOutput : implode("\n",self::$Application->commandBuffer)."\"EXEC_COMPLETE\";");
		self::$NextCommOutput = NULL;
		
		self::WriteCommand( $client, 'COMM', NULL, $output );
		$app->commandBuffer = array();
		self::$EventQueue = array();
		
		socket_close( $client );
		
		self::$LastContact = time();
	}
	
	static private function HandleRevComm( $client ) {
		if( self::$RevCommSocket ) {
			try { socket_close( self::$RevCommSocket ); } catch ( SFException $ex ) {}
		}
		
		self::$RevCommSocket = $client;
		
		self::$LastContact = time();
	}
	
	static private function HandleFile( $client, $cmd ) {
		if( ! isset(self::$Application) )
			throw new SFException( 'Invalid FILE request, not logged in yet' );
		$app = self::$Application;
		
		//if index is not set, object is already destructed
		if (isset($app->registry->all[$cmd->cid]))
		{
			try {
				$transferable = $app->registry->all[$cmd->cid];
				
				switch( $cmd->action ) {
					case 'start':
						$descr = $transferable->transferStart();
						self::WriteCommand( $client, 'FILE', array( 'contentType' => $descr[0], 'size' => $descr[1] ) );
						break;
					
					case 'data':
						ob_start();
						$transferable->transferGetData();
						$content = ob_get_clean();
						self::WriteCommand( $client, 'FILE', NULL, $content );
						break;
					
					case 'end':
						$transferable->transferEnd();
						break;
				}
			}
			catch ( SFException $ex ) {
				if( self::$ConnectedDebugger )
					throw $ex;
				
				self::$NextCommOutput = $ex->getFormattedReport();
				
				if( self::$Options['debug'] )
					self::Log( 'Uncaught Exception in application ('.self::$SessionID.'): ' . self::$NextCommOutput );
			}
		}
		
		socket_close( $client );
		
		self::$LastContact = time();
	}
	
	static private function HandleEvent( $cmd ) {
		if( ! isset(self::$Application) )
			throw new SFException( 'Invalid EVENT request, not logged in yet' );
		
		try {
			list($event,$args) = unserialize( $cmd->data );
		}
		catch ( SFException $ex ) {
			if( self::$Options['debug'] )
				self::Log( "Unable to process EVENT: " . $ex->getMessage() );
			return;
		}
		
		self::$EventQueue[] = array( $event, $args );
	}
	
	static private function HandleMessage( $cmd ) {
		if( ! isset(self::$Application) )
			throw new SFException( 'Invalid MESSAGE request, not logged in yet' );
		
		self::$Application->window->alert( $cmd->data );
	}
	
	static private function HandleServiceCmdCallback( $service, $cmd ) {
		try {
			list($name,$args) = unserialize( $cmd->data );
		}
		catch ( SFException $ex ) {
			self::WriteCommand( $service->socket, 'ERROR' );
			return;
		}
		
		self::WriteCommand( $service->socket, 'OK' );
		
		$service->handleCallback( $name, $args );
	}
	
	static private function GetServiceBySocket( $socket ) {
		foreach ( self::$ConnectedServices as $service ) {
			if( isset($service->socket) && $socket == $service->socket )
				return $service;
		}
	}
	
	static private function AuthorizeLogin( $cmd ) {
		ob_start();
		
		if( isset($cmd->loginObject) ) {
			$loginObject = json_decode($cmd->data);
			$app = $loginObject->app;
			$args = (isset($loginObject->args) && $loginObject->args != '' ? explode(',',$loginObject->args) : array());
			$error = NULL;
			$user = $loginObject->username;
			$pass = $loginObject->password;
			$server = $loginObject->server;
			$secure = $loginObject->secure;
			$ip = $loginObject->ip;
			self::$AppInfo = (array) $loginObject->appInfo;
			self::$PlatformInfo = (array) $loginObject->platformInfo;
			self::$ExtensionInfo = (array) $loginObject->extensionInfo;
			foreach ( array_keys(self::$ExtensionInfo) as $id ) { $arr = (array) self::$ExtensionInfo[$id]; self::$ExtensionInfo[$id] = $arr; }
			self::$CommandLineArgs = (array) $loginObject->cmdlineArgs;
			self::$ClientVersion = self::$AppInfo['version'];
		}
		else {
			$app = $cmd->app;
			$args = (isset($cmd->args) && $cmd->args != '' ? explode(',',$cmd->args) : array());
			$error = NULL;
			$user = $cmd->username;
			$pass = $cmd->password;
			$server = NULL;
			$secure = NULL;
			$ip = $cmd->ip;
			self::$AppInfo = NULL;
			self::$PlatformInfo = NULL;
			self::$ExtensionInfo = NULL;
			self::$CommandLineArgs = NULL;
			self::$ClientVersion = '1.2.0';
		}
		
		$granted = FALSE;
		
		self::$IncludePath = self::$Options['sitefusionPath'] . '/';
		$_SERVER['HTTP_HOST'] = 'localhost';
		
		if( (!preg_match( '/^[a-zA-Z0-9_\-\/]+$/', $app )) || (! file_exists(self::$Options['sitefusionPath'].'/app/' . $app . '.startup.php.inc')) )
			return array( 'success' => 0, 'error' => 'invalid_app' );

		include_once( 'sys/application.php.inc' );
		include_once( 'app/' . $app . '.startup.php.inc' );

		if( function_exists( 'authorizeLogin' ) )
			$granted = authorizeLogin( $args, $user, $pass );
		else
			return array( 'success' => 0, 'error' => 'no_auth_func' );

		if( ! $granted )
			return array( 'success' => 0, 'error' => 'invalid_login' );

		$ident = md5( $ip . $user . md5($pass) );

		$class = getApplication( $args );
		
		self::$Application = new $class ( $user, $ip, $ident, $app, $args );
		
		ob_end_clean();
		
		
		$extensionPolicy = array();
		
		if( version_compare( self::$ClientVersion, '1.2.1' ) >= 0 ) {
			foreach ( self::$ExtensionPolicy['require'] as $requireExtension ) {
				if( (!isset(self::$ExtensionInfo[$requireExtension['id']]))
				 	or ($requireExtension['minVersion'] !== NULL && version_compare(self::$ExtensionInfo[$requireExtension['id']]['version'],$requireExtension['minVersion']) == -1)
					or ($requireExtension['maxVersion'] !== NULL && version_compare(self::$ExtensionInfo[$requireExtension['id']]['version'],$requireExtension['maxVersion']) == 1) )
				{
					$extensionPolicy[] = array( $requireExtension['id'], 'get', $requireExtension['fileName'] );
				}
				elseif( !self::$ExtensionInfo[$requireExtension['id']]['enabled'] ) {
					$extensionPolicy[] = array( $requireExtension['id'], 'enable' );
				}
			}

			foreach ( self::$ExtensionPolicy['disable'] as $disableExtension ) {
				if( isset(self::$ExtensionInfo[$disableExtension['id']]) && self::$ExtensionInfo[$disableExtension['id']]['enabled'] )
					$extensionPolicy[] = array( $disableExtension['id'], 'disable' );
			}
		}
		
		$loginParam = array(
			'success' => 1,
			'ip' => $ip,
			'type' => 'app',
			'ident' => $ident,
			'app' => $app,
			'args' => join(',',$args),
			'user' => $user,
			'server' => $server,
			'secure' => ($secure == 1 ? TRUE : FALSE),
			'includeJs' => implode( ",", self::$JSLibraries ),
			'alwaysLowered' => self::$Application->alwaysLowered,
			'alwaysRaised' => self::$Application->alwaysRaised,
			'centerscreen' => self::$Application->centerscreen,
			'resizable' => self::$Application->resizable,
			'extensionPolicy' => $extensionPolicy
		);
		
		if( is_int(self::$Application->width) && is_int(self::$Application->height) ) {
			$loginParam['width'] = self::$Application->width;
			$loginParam['height'] = self::$Application->height;
		}
		
		return $loginParam;
	}
	
	static private function InitializeApplication() {
		self::$Application->preinit( self::$Application->applicationArguments );
	}
	
	static private function ProcessEventQueue() {
		if( ! count(self::$EventQueue) )
			return;
		
		ob_start();
		
		foreach ( self::$EventQueue as $ge ) {
			self::$Application->handleGlobalEvent( $ge[0], $ge[1] );
		}
		
		ob_end_clean();
		
		self::$EventQueue = array();
	}
	
	static private function CheckTimers() {
		if( isset(self::$Timers[0]) && self::$Timers[0][0] <= microtime(TRUE) ) {
			try {
				list($time,$timer) = array_shift( self::$Timers );
				
				ob_start();
				$timer->trigger();
				ob_end_clean();
			}
			catch ( SFException $ex ) {
				if( self::$ConnectedDebugger )
					throw $ex;
				
				self::$NextCommOutput = $ex->getFormattedReport();
			}
		}
	}
	
	static protected function FlushRevComm() {
		if( (! self::$RevCommSocket) || ( (!count(self::$Application->commandBuffer)) && (!self::$NextCommOutput) ) )
			return;
		
		try {
			$read = NULL;
			$write = array( self::$RevCommSocket );
			$except = NULL;
			
			if( socket_select( $read, $write, $except, 0 ) > 0 ) {
				$output = (self::$NextCommOutput ? self::$NextCommOutput : implode("\n",self::$Application->commandBuffer)."\"EXEC_COMPLETE\";");
				self::$NextCommOutput = NULL;
				
				self::WriteCommand( self::$RevCommSocket, 'REVCOMM', NULL, $output );
				self::$Application->commandBuffer = array();
				
				socket_close( self::$RevCommSocket );
				self::$RevCommSocket = NULL;
				
				self::$LastContact = time();
			}
		}
		catch ( SFException $ex ) {}
	}
	
	static public function GetService( $id ) {
		self::WriteCommand( self::$ParentSocket, 'GETSERVICE', array( 'id' => (int)$id ) );
		$cmd = self::ReadCommand( self::$ParentSocket );
		
		if( $cmd->command != 'OK' )
			throw new SFException( 'Service ID invalid' );
		
		$socket = socket_create( AF_UNIX, SOCK_STREAM, 0 );
		if( $socket === FALSE )
			throw new SFException( "Socket error: " . socket_strerror(socket_last_error()) );
		
		$result = FALSE;
		
		while( $result === FALSE ) {
			try {
				$result = socket_connect( $socket, self::$Options['runtimeDirectory'] . '/service' . $cmd->pid . '.sock' );
			}
			catch( SFException $ex ) {}
		}
		
		self::WriteCommand( $socket, 'GETSERVICE' );
		
		$service = new SiteFusionService( $id, $socket );
		
		self::$ConnectedServices[] = $service;
		
		return $service;
	}
	
	static public function GetServiceByName( $owner, $name ) {
		$list = array();
		foreach ( self::GetServiceList() as $service ) {
			if( $service['owner'] == $owner && $service['service'] == $name )
				$list[] = $service;
		}
		
		if(! count($list) )
			throw new SFException( 'Invalid service identifier' );
		
		if( count($list) == 1 )
			return self::GetService( $list[0]['id'] );
		else
			return self::GetService( $list[mt_rand(0,count($list)-1)]['id'] );
	}
	
	static public function CallServiceFunction( $service, $name, $args ) {
		self::WriteCommand( $service->socket, 'CALL', array( 'name' => $name ), serialize($args) );
		
		return self::ReadCommand( $service->socket );
	}
	
	static private function UpdateDatabase() {
		if( !self::$Application )
			return;
		
		self::VerifyDatabaseState();
		self::DatabaseQuery( "UPDATE `sitefusion`.`processes` SET `memory` = ".memory_get_usage().", `memory_peak` = ".memory_get_peak_usage().", `nodes` = ".count(self::$Application->registry->all)." WHERE `id` = '".self::$SessionID."' LIMIT 1" );
	}
	
	static public function Header( $text ) {
		self::$Headers[] = $text;
	}
	
	static public function DebugShowStack( $bt ) {
		if( !self::$Application )
			return;
		
		$file = $bt[0]['file'];
		$line = $bt[0]['line'];
		
		if( isset(self::$DebugWindows[$file.$line]) ) {
			if( ! self::$DebugWindows[$file.$line]->isRegistered )
				self::$Application->window->addChild( self::$DebugWindows[$file.$line] = new SiteFusionDebugWindow( $bt ) );
			else
				self::$DebugWindows[$file.$line]->addStackView( $bt );
		}
		else
			self::$Application->window->addChild( self::$DebugWindows[$file.$line] = new SiteFusionDebugWindow( $bt ) );
	}
	
	static public function DebuggerWriteCommand( $socket, $cmd, $args = NULL, $data = NULL ) {
		if( self::$ApplicationPath != 'sitefusion/admin' )
			return;
		
		return self::WriteCommand( $socket, $cmd, $args, $data );
	}

	static public function DebuggerReadCommand( $socket ) {
		if( self::$ApplicationPath != 'sitefusion/admin' )
			return;
		
		return self::ReadCommand( $socket );
	}
	
	static public function GetDebugSocket( $pid ) {
		if( self::$ApplicationPath != 'sitefusion/admin' )
			return;

		$socket = socket_create( AF_UNIX, SOCK_STREAM, 0 );
		if( $socket === FALSE )
			throw new SFException( "Service unavailable: " . socket_strerror(socket_last_error()) );
		
		$result = socket_connect( $socket, self::$Options['runtimeDirectory'] . '/debug' . $pid . '.sock' );
		if( $result === FALSE )
			throw new SFException( "Service unavailable: " . socket_strerror(socket_last_error($socket)) );
		
		return $socket;
	}
}


function sfstack() {
	$bt = debug_backtrace();
	
	ApplicationProcess::DebugShowStack( $bt );
}



