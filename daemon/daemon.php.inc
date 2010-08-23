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
 * @subpackage Master
*/


class DaemonProcess extends SiteFusionProcess
{
	static public $Started;
	static public $PID;
	static public $MemoryUsed;
	static public $MemoryPeak;
	static private $DaemonStarted = FALSE;
	static private $Restart = FALSE;
	static private $Graceful = FALSE;
	static private $Children = array();
	static private $DeadChildren = array();
	static private $UsedPorts = array();
	static private $SleepingPorts = array();
	
	static private $DefaultOptions = array(
		'address' => '127.0.0.1',
		'port' => '5173',
		'logfile' => 'main.log',
		'timezone' => 'Europe/Amsterdam',
		'keepaliveInterval' => 60,
		'childTimeout' => 300,
		'sessionTimeout' => 1800,
		'serviceClientTimeout' => 300,
		'portSleepTime' => 240,
		'daemonCycleInterval' => 10000,
		'databaseHost' => 'localhost',
		'databaseUsername' => '',
		'databasePassword' => '',
		'databaseName' => 'sitefusion',
		'databaseFailureRetries' => 5,
		'databaseFailureRetryInterval' => 1000,
		'sitefusionPath' => '',
		'debug' => FALSE,
		'daemonize' => TRUE,
		'runtimeDirectory' => '/var/run/sitefusion',
		'changeIdentity' => TRUE,
		'systemUser' => 'nobody',
		'systemGroup' => 'nobody'
	);
	
	static public function StartServer( $options ) {
		if( self::$DaemonStarted || self::$ProcessType )
			return;
			
		self::$ProcessType = 'daemon';
		
		self::$Options = array_merge( self::$DefaultOptions, $options, self::GetCommandLineOptions() );
		
		date_default_timezone_set( self::$Options['timezone'] );
		
		if( ! self::OpenLog() ) exit(1);
		
		try {
			self::Log( 'Starting server...' );
			
			self::Log( 'Setting up runtime directory...' );
			$currentUser = posix_getpwuid(posix_getuid());
			if( self::$Options['changeIdentity'] && $currentUser['name'] == self::$Options['systemUser'] )
				self::$Options['changeIdentity'] = FALSE;
			
			if( ! self::InitRuntimeDir() ) exit(1);
			
			if( self::$Options['changeIdentity'] )
				self::ChangeIdentity();
			
			self::WritePIDFile();
			
			self::Log( 'Opening main server socket on '.self::$Options['address'].':'.self::$Options['port'] );
			self::$Socket = self::OpenSocket( self::$Options['address'], self::$Options['port'] );
			
			self::Log( 'Setting up signal handlers...' );
			self::SetSignalHandlers();
			
			self::Log( 'Initializing database...' );
			self::InitDatabase();
			
			self::$Started = time();
			self::$DaemonStarted = TRUE;
			self::$MemoryUsed = memory_get_usage();
			self::$MemoryPeak = memory_get_peak_usage();
			self::$PID = posix_getpid();
			
			self::Log( 'Starting services...' );
			self::StartDaemonServices();
			
			self::Log( 'Entering main event loop' );
			self::MainLoop();
		}
		catch( SFException $ex ) {
			self::Log( $ex->getMessage() . (self::$Options['debug'] ? ' at ' . $ex->getFile() . ' on line ' . $ex->getLine() : '') );
			if( self::$Options['debug'] )
				self::Log( "Debug backtrace:\n" . $ex->getTraceAsString() );
		}
	}
	
	static private function GetCommandLineOptions() {
		$ret = array();
		if( in_array('-f',$_SERVER['argv']) ) $ret['daemonize'] = FALSE;
		if( in_array('-d',$_SERVER['argv']) ) $ret['debug'] = TRUE;
		return $ret;
	}
	
	static private function StopServer() {
		self::Log( 'Closing parent socket...' );
		socket_close( self::$Socket );
		
		self::Log( 'Closing session database table...' );
		try { self::CloseDatabase(); } catch( SFException $ex ) { self::Log( "Database error: " . $ex->getMessage() ); }
		
		self::Log( 'Sending SIGTERM to children...' );
		foreach ( array_keys(self::$Children) as $pid ) {
			if( ! posix_kill( $pid, SIGTERM ) )
				self::RemoveChild( $pid );
		}
		
		self::Log( 'Waiting for children to exit, 2 minutes grace time...' );
		$start = time();
		while( count(self::$Children) && (time() - $start) < 120 ) {
			if( self::$ShutdownKill )
				break;
			
			foreach ( array_keys(self::$Children) as $pid ) {
				if( ! posix_kill( $pid, 0 ) )
					self::RemoveChild( $pid );
			}
			
			pcntl_wait( $status, WNOHANG OR WUNTRACED );
			usleep( self::$Options['daemonCycleInterval'] );
		}
		
		if( $count = count(self::$Children) ) {
			self::Log( "Grace time expired, killing $count remaining children" );
			foreach ( array_keys(self::$Children) as $pid ) {
				self::KillChild( $pid );
			}
		}
		
		self::RemovePIDFile();
		
		if( self::$Restart ) {
			self::Log( 'Restarting...' );
			exit(2);
		}
		else {
			self::Log( 'Shutting down...' );
			exit(0);
		}
	}
	
	static private function InitRuntimeDir() {
		try {
			$dir = self::$Options['runtimeDirectory'];
		
			if( file_exists( $dir ) ) {
				foreach ( scandir( $dir ) as $file ) {
					if( $file != '.' && $file != '..' )
						unlink( $dir . '/' . $file );
				}
			
				if( self::$Options['changeIdentity'] ) {
					rmdir( $dir );
					mkdir( $dir, 0755 );
				}
			}
			else
				mkdir( $dir, 0755 );
		
			if( self::$Options['changeIdentity'] )
				chown( $dir, self::$Options['systemUser'] );
			
			return TRUE;
		}
		catch ( SFException $ex ) {
			self::Log( 'Failed to initialize runtime directory: ' . $ex->getMessage() );
			return FALSE;
		}
	}
	
	static private function WritePIDFile() {
		try {
			return (bool) file_put_contents( self::$Options['runtimeDirectory'].'/sitefusion.pid', posix_getpid()."\n" );
		}
		catch ( SFException $ex ) {
			return FALSE;
		}
	}
	
	static private function RemovePIDFile() {
		try {
			unlink( self::$Options['runtimeDirectory'].'/sitefusion.pid' );
			return TRUE;
		}
		catch ( SFException $ex ) {}
		
		return !file_exists( self::$Options['runtimeDirectory'].'/sitefusion.pid' );
	}
	
	static private function RemoveChild( $pid ) {
		switch ( self::$Children[$pid]->type ) {
			case 'app':
				if( self::$Children[$pid]->loggedIn )
					self::ChildEnded( self::$Children[$pid] );
				if( isset(self::$Children[$pid]->error) )
					self::$DeadChildren[$pid] = self::$Children[$pid];
				
				self::$SleepingPorts[ self::$Children[$pid]->port ] = time();
				try { socket_close( self::$Children[$pid]->socket ); } catch ( SFException $ex ) {}
				try { socket_close( self::$Children[$pid]->eventPipe ); } catch ( SFException $ex ) {}
				break;
			
			case 'job':
				self::ChildEnded( self::$Children[$pid] );
				break;
			
			case 'service':
				self::ChildEnded( self::$Children[$pid] );
				
				try { socket_close( self::$Children[$pid]->socket ); } catch ( SFException $ex ) {}
				try { socket_close( self::$Children[$pid]->eventPipe ); } catch ( SFException $ex ) {}
				break;
		}
		
		unset( self::$Children[$pid] );
	}
	
	static private function CleanSleepingPorts() {
		foreach ( array_keys(self::$SleepingPorts) as $port ) {
			if( time() - self::$SleepingPorts[$port] >= self::$Options['portSleepTime'] ) {
				if( ($index = array_search($port,self::$UsedPorts)) !== FALSE ) {
					array_splice( self::$UsedPorts, $index, 1 );
				}
				unset( self::$SleepingPorts[$port] );
			}
		}
	}
	
	static private function Daemonize() {
		$pid = pcntl_fork();
		
		if( $pid == -1 )
			throw new SFException( "Could not fork" );
		else if( $pid )
			exit();
		
		posix_setsid();
		
		self::$Daemonized = TRUE;
	}
	
	static private function MainLoop() {
		while(1) {
			while( pcntl_wait($status, WNOHANG OR WUNTRACED) > 0 ) {
				usleep( self::$Options['daemonCycleInterval'] );
			}
			
			$appCount = 0;
			
			foreach ( array_keys(self::$Children) as $pid ) {
				if(! posix_kill($pid,0) ) {
					self::RemoveChild( $pid );
					continue;
				}
				if( self::$Children[$pid]->type == 'app' )
					$appCount++;
			}
			
			if( self::$Shutdown ) {
				if( (self::$Graceful && $appCount == 0) || !self::$Graceful ) {
					self::StopServer();
					return;
				}
			}
			
			self::CleanSleepingPorts();
			
			$jobCheckDate = time();
			if(! isset($lastJobMinute) ) $lastJobMinute = idate('i',$jobCheckDate-60);
			if( $lastJobMinute != idate('i',$jobCheckDate) ) {
				$minute = idate('i',$jobCheckDate);
				$hour = idate('H',$jobCheckDate);
				$monthday = idate('d',$jobCheckDate);
				$month = idate('m',$jobCheckDate);
				$weekday = idate('w',$jobCheckDate);
				$ljm = $lastJobMinute;
				while( ++$ljm <= $minute ) {
					if( $ljm == 60 ) $ljm = 0;
					self::CheckJobSchedule( $ljm, $hour, $monthday, $month, $weekday );
				}
				$lastJobMinute = $minute;
			}
			
			if( count(self::$Children) )
				self::ListenToChildren();
			
			foreach( array_keys(self::$DeadChildren) as $pid ) {
				if( time() - self::$DeadChildren[$pid]->errorTimestamp > 60 )
					unset(self::$DeadChildren[$pid]);
			}
			
			foreach ( self::$Children as $pid => $child ) {
				if( $child->type == 'app' && time() - $child->lastKeepalive > self::$Options['childTimeout'] )
					self::KillChild( $pid );
			}
			
			if( $client = self::PollForClient() ) {
				self::HandleClient( $client );
			}
			//else usleep( self::$Options['daemonCycleInterval'] );
		}
	}
	
	static private function KillChild( $pid ) {
		self::Log( 'Killing child with pid '.$pid );
		posix_kill( $pid, SIGKILL );
		self::RemoveChild( $pid );
	}
	
	static private function ListenToChildren() {
		$read = array();
		$write = NULL;
		$except = NULL;
		
		foreach ( self::$Children as $child ) {
			if( $child->type == 'app' || $child->type == 'service' ) {
				$read[] = $child->socket;
				$read[] = $child->eventPipe;
			}
		}
		
		if( ! count($read) )
			return;
		
		try {
			if( socket_select($read, $write, $except, 0) < 1 )
				return;
		}
		catch ( SFException $ex ) { return; }
		
		foreach ( $read as $socket ) {
			$child = self::GetChildBySocket( $socket );
			
			try {
				$cmd = self::ReadCommand( $socket );
				
				switch ( $cmd->command ) {
					case 'LIVE':		self::HandleChildCmdKeepalive( $child ); break;
					case 'LOGIN':		self::ChildStarted( $child, $cmd ); break;
					case 'EVENT':		self::HandleEvent( $child, $cmd ); break;
					case 'ADMIN':		self::HandleAdminCmd( $child, $cmd ); break;
					case 'JOB':			self::HandleJobCmd( $child, $cmd ); break;
					case 'SERVICE':		self::HandleServiceCmd( $child, $cmd ); break;
					case 'GETSERVICE':	self::HandleGetServiceCmd( $child, $cmd ); break;
					case 'GROUP':		self::HandleGroupCmd( $child, $cmd ); break;
				}
			}
			catch ( SFException $ex ) {}
		}
	}
	
	static private function CheckJobSchedule( $minute, $hour, $monthday, $month, $weekday ) {
		$res = self::DatabaseQuery(
			"SELECT `owner`,`job`,`args`,`mail_output`
			FROM `".self::$Options['databaseName']."`.`jobs`
			WHERE
			(`minute` = -1 || `minute` = $minute) &&
			(`hour` = -1 || `hour` = $hour) &&
			(`monthday` = -1 || `monthday` = $monthday) &&
			(`month` = -1 || `month` = $month) &&
			(`weekday` = -1 || `weekday` = $weekday)"
		);
		
		$num = mysql_num_rows( $res );
		
		for( $n = 0; $n < $num; $n++ ) {
			$jobdata = mysql_fetch_assoc( $res );
			
			self::StartJob( $jobdata['owner'].($jobdata['owner'] == '' ? '':'/').$jobdata['job'], $jobdata['args'], ($jobdata['mail_output'] ? $jobdata['mail_output'] : NULL) );
		}
	}
	
	static private function ChildStarted( $child, $param ) {
		$child->loggedIn = TRUE;
		$child->sessionId = $param->sid;
		$child->ident = $param->ident;
		$child->user = $param->user;
		$child->app = $param->app;
		$child->args = $param->args;
		$child->ip = $param->ip;
		if( $child->type == 'app' )
			$child->extendedInfo = unserialize($param->data);
		
		$res = self::DatabaseQuery( "INSERT INTO `".self::$Options['databaseName']."`.`processes` (
		`id` ,
		`type` ,
		`ident` ,
		`user` ,
		`app` ,
		`args` ,
		`client_ip` ,
		`port`,
		`pid`
		)
		VALUES (
		'".$param->sid."', '".$param->type."', '".$param->ident."', '".mysql_escape_string($param->user)."', '".mysql_escape_string($param->app)."', '".mysql_escape_string($param->args)."', '".mysql_escape_string($param->ip)."', '".$child->port."', '".$child->pid."'
		)
		" );
		
		if( ! $res )
			throw new SFException( 'ChildStarted: MySQL Error: '.mysql_error() );
		
		switch ( $child->type ) {
			case 'app':
				self::Log( "Application started ($child->sessionId) at port $child->port with [pid: $child->pid], [application: $child->app], [args: $child->args], [user: $child->user], [ip: $child->ip]" );
				break;
			
			case 'job':
				self::Log( "Job started ($child->sessionId) with [pid: $child->pid], [job: $child->app], [args: $child->args]" );
				break;
			
			case 'service':
				self::Log( "Service started ($child->sessionId) with [pid: $child->pid], [service: $child->app], [args: $child->args]" );
				break;
		}
		
		return TRUE;
	}
	
	static private function ChildEnded( $child ) {
		$res = self::DatabaseQuery( "DELETE FROM `".self::$Options['databaseName']."`.`processes` WHERE `id` = '".$child->sessionId."' LIMIT 1" );
		
		if( ! $res )
			throw new SFException( 'ChildEnded: MySQL Error: '.mysql_error() );
		
		switch ( $child->type ) {
			case 'app':
				foreach ( self::$Children as $c ) {
					if( $c->type == 'service' && $c->master == $child->pid )
						posix_kill( $c->pid, SIGTERM );
				}
				if( self::$Options['debug'] ) {
					try { unlink( self::$Options['runtimeDirectory'] . '/debug' . $child->pid . '.sock' ); } catch( SFException $ex ) {}
				}
				self::Log( "Application ended ($child->sessionId) at port $child->port with [pid: $child->pid], [application: $child->app], [args: $child->args], [user: $child->user], [ip: $child->ip]" );
				break;
			
			case 'job':
				self::Log( "Job ended ($child->sessionId) with [pid: $child->pid], [job: $child->app], [args: $child->args]" );
				break;
			
			case 'service':
				try { unlink( self::$Options['runtimeDirectory'] . '/service' . $child->pid . '.sock' ); } catch( SFException $ex ) {}
				if( self::$Options['debug'] ) {
					try { unlink( self::$Options['runtimeDirectory'] . '/debug' . $child->pid . '.sock' ); } catch( SFException $ex ) {}
				}
				self::Log( "Service ended ($child->sessionId) with [pid: $child->pid], [service: $child->app], [args: $child->args]" );
				if( $child->daemon && !self::$Shutdown )
					self::StartService( $child->serviceId );
				break;
		}
		
		return TRUE;
	}
	
	static private function HandleClient( $client ) {
		$cmd = self::ReadCommand( $client );
		
		switch( $cmd->command ) {
			case 'STARTAPP':
				self::StartApplication( $client, $cmd->clientid );
				break;
			
			case 'GETERROR':
				self::HandleGetError( $client, $cmd->clientid );
				break;
		}
	}
	
	static private function HandleChildCmdKeepalive( $child ) {
		$child->lastKeepalive = time();
	}
	
	static private function HandleGetError( $client, $clientid ) {
		foreach ( array_keys(self::$DeadChildren) as $pid ) {
			if( self::$DeadChildren[$pid]->clientId == $clientid && isset(self::$DeadChildren[$pid]->error) ) {
				$error = self::$DeadChildren[$pid]->error;
				unset( self::$DeadChildren[$pid] );
				break;
			}
		}
		if( ! isset($error) ) {
			foreach ( array_keys(self::$Children) as $pid ) {
				if( self::$Children[$pid]->clientId == $clientid && isset(self::$Children[$pid]->error) ) {
					$error = self::$Children[$pid]->error;
					break;
				}
			}
		}
		
		self::WriteCommand( $client, 'ERROR', array( 'found' => isset($error) ), (isset($error) ? $error:NULL) );
	}
	
	static private function HandleEvent( $sourceChild, $cmd ) {
		foreach ( self::$Children as $child ) {
			if( $child === $sourceChild )
				continue;
			
			if( (($child->type == 'app' && $child->loggedIn) || $child->type == 'service') && count(array_intersect($sourceChild->groups,$child->groups)) )
				self::WriteCommand( $child->eventPipe, 'EVENT', NULL, $cmd->data );
		}
	}
	
	static private function HandleAdminCmd( $child, $cmd ) {
		if( $child->app != 'sitefusion/admin' )
			return;
		
		switch ( $cmd->action ) {
			case 'kill':
				posix_kill( (int)$cmd->pid, SIGKILL );
				self::Log( "Process $cmd->pid killed by user $child->user" );
				break;
			
			case 'term':
				posix_kill( (int)$cmd->pid, SIGTERM );
				self::Log( "Process $cmd->pid terminated by user $child->user" );
				break;
			
			case 'shutdown':
				self::$Shutdown = (int)$cmd->shutdown;
				self::$Graceful = (int)$cmd->graceful;
				self::$Restart = (int)$cmd->restart;
				self::Log( "Shutdown flags set to [shutdown: $cmd->shutdown], [restart: $cmd->restart], [graceful: $cmd->graceful] by user $child->user" );
				break;
			
			case 'getstatus':
				self::WriteCommand( $child->socket, 'STATUS', array( 'shutdown' => (int)self::$Shutdown, 'graceful' => (int)self::$Graceful, 'restart' => (int)self::$Restart, 'memory' => memory_get_usage(), 'memory_peak' => memory_get_peak_usage() ) );
				break;
			
			case 'getappinfo':
				$extendedInfo = NULL;
				foreach ( self::$Children as $rChild ) {
					if( $rChild->type == 'app' && $rChild->loggedIn && $cmd->sid == $rChild->sessionId )
						$extendedInfo = $rChild->extendedInfo;
				}
				self::WriteCommand( $child->socket, 'APPINFO', NULL, serialize($extendedInfo) );
				break;
			
			case 'message':
				foreach ( self::$Children as $rChild ) {
					if( $rChild->type == 'app' && $rChild->loggedIn && ($cmd->to == 'all' || $cmd->to == $rChild->sessionId) )
						self::WriteCommand( $rChild->socket, 'MESSAGE', NULL, $cmd->data );
				}
				break;
		}
	}
	
	static private function HandleJobCmd( $child, $cmd ) {
		switch ( $cmd->action ) {
			case 'getlist':
				$res = self::DatabaseQuery(
					"SELECT *
					FROM `".self::$Options['databaseName']."`.`jobs`
					" . ($child->app == 'sitefusion/admin' ? '':"WHERE `owner` = '".mysql_escape_string(self::GetApplicationOwnerName($child->app))."'")
				);
				$num = mysql_num_rows($res);
				$ret = array();
				for( $n = 0; $n < $num; $n++ ) {
					$ret[] = mysql_fetch_assoc( $res );
				}
				self::WriteCommand( $child->socket, 'JOBS', NULL, serialize($ret) );
				break;
			
			case 'addjob':
				$param = unserialize( $cmd->data );
				self::DatabaseQuery(
					"INSERT INTO `".self::$Options['databaseName']."`.`jobs`
					(`owner`,`job`,`args`,`minute`,`hour`,`monthday`,`month`,`weekday`,`mail_output`)
					VALUES (
					'".mysql_escape_string($child->app == 'sitefusion/admin' ? $param['owner'] : self::GetApplicationOwnerName($child->app))."',
					'".mysql_escape_string($param['job'])."',
					'".mysql_escape_string($param['args'])."',
					'".((int)$param['minute'])."',
					'".((int)$param['hour'])."',
					'".((int)$param['monthday'])."',
					'".((int)$param['month'])."',
					'".((int)$param['weekday'])."',
					'".mysql_escape_string($param['mail_output'])."')"
				);
				
				self::WriteCommand( $child->socket, 'OK' );
				break;
			
			case 'changejob':
				$param = unserialize( $cmd->data );
				if( $child->app != 'sitefusion/admin' ) {
					$res = self::DatabaseQuery( "SELECT count(*) FROM `".self::$Options['databaseName']."`.`jobs` WHERE `owner` = '".mysql_escape_string(self::GetApplicationOwnerName($child->app))."' && `id` = ".(int)$cmd->id );
					list($num) = mysql_fetch_row($res);
					if( ! $num ) return;
				}
				self::DatabaseQuery(
					"UPDATE `".self::$Options['databaseName']."`.`jobs`
					SET
					`owner` = '".mysql_escape_string($child->app == 'sitefusion/admin' ? $param['owner'] : self::GetApplicationOwnerName($child->app))."',
					`job` = '".mysql_escape_string($param['job'])."',
					`args` = '".mysql_escape_string($param['args'])."',
					`minute` = '".((int)$param['minute'])."',
					`hour` = '".((int)$param['hour'])."',
					`monthday` = '".((int)$param['monthday'])."',
					`month` = '".((int)$param['month'])."',
					`weekday` = '".((int)$param['weekday'])."',
					`mail_output` = '".mysql_escape_string($param['mail_output'])."'
					WHERE `id` = ".(int)$cmd->id
				);
				
				self::WriteCommand( $child->socket, 'OK' );
				break;
			
			case 'deletejob':
				$id = (int) $cmd->id;
				if( $child->app != 'sitefusion/admin' ) {
					$res = self::DatabaseQuery( "SELECT count(*) FROM `".self::$Options['databaseName']."`.`jobs` WHERE `owner` = '".mysql_escape_string(self::GetApplicationOwnerName($child->app))."' && `id` = ".(int)$cmd->id );
					list($num) = mysql_fetch_row($res);
					if( ! $num ) return;
				}
				self::DatabaseQuery(
					"DELETE FROM `".self::$Options['databaseName']."`.`jobs` WHERE `id` = ".(int)$cmd->id
				);
				
				self::WriteCommand( $child->socket, 'OK' );
				break;
		}
	}
	
	static private function HandleServiceCmd( $child, $cmd ) {
		switch ( $cmd->action ) {
			case 'getlist':
				$res = self::DatabaseQuery(
					"SELECT *
					FROM `".self::$Options['databaseName']."`.`services`
					" . ($child->app == 'sitefusion/admin' ? '':"WHERE `owner` = '".mysql_escape_string(self::GetApplicationOwnerName($child->app))."'")
				);
				$num = mysql_num_rows($res);
				$ret = array();
				for( $n = 0; $n < $num; $n++ ) {
					$ret[] = mysql_fetch_assoc( $res );
				}
				self::WriteCommand( $child->socket, 'SERVICES', NULL, serialize($ret) );
				break;
			
			case 'addservice':
				$param = unserialize( $cmd->data );
				self::DatabaseQuery(
					"INSERT INTO `".self::$Options['databaseName']."`.`services`
					(`owner`,`service`,`args`,`daemon`)
					VALUES (
					'".mysql_escape_string($child->app == 'sitefusion/admin' ? $param['owner'] : self::GetApplicationOwnerName($child->app))."',
					'".mysql_escape_string($param['service'])."',
					'".mysql_escape_string($param['args'])."',
					'".((int)$param['daemon'])."')"
				);
				if( intval($param['daemon']) )
					self::StartService( mysql_insert_id(self::$Database) );
				
				self::WriteCommand( $child->socket, 'OK' );
				break;
			
			case 'changeservice':
				$param = unserialize( $cmd->data );
				if( $child->app != 'sitefusion/admin' ) {
					$res = self::DatabaseQuery( "SELECT count(*) FROM `".self::$Options['databaseName']."`.`services` WHERE `owner` = '".mysql_escape_string(self::GetApplicationOwnerName($child->app))."' && `id` = ".(int)$cmd->id );
					list($num) = mysql_fetch_row($res);
					if( ! $num ) return;
				}
				self::DatabaseQuery(
					"UPDATE `".self::$Options['databaseName']."`.`services`
					SET
					`owner` = '".mysql_escape_string($child->app == 'sitefusion/admin' ? $param['owner'] : self::GetApplicationOwnerName($child->app))."',
					`service` = '".mysql_escape_string($param['service'])."',
					`args` = '".mysql_escape_string($param['args'])."',
					`daemon` = '".((int)$param['daemon'])."'
					WHERE `id` = ".(int)$cmd->id
				);
				if( intval($param['daemon']) )
					self::RestartService( (int)$cmd->id );
				
				self::WriteCommand( $child->socket, 'OK' );
				break;
			
			case 'deleteservice':
				$id = (int) $cmd->id;
				if( $child->app != 'sitefusion/admin' ) {
					$res = self::DatabaseQuery( "SELECT count(*) FROM `".self::$Options['databaseName']."`.`services` WHERE `owner` = '".mysql_escape_string(self::GetApplicationOwnerName($child->app))."' && `id` = ".(int)$cmd->id );
					list($num) = mysql_fetch_row($res);
					if( ! $num ) return;
				}

				$res = self::DatabaseQuery( "SELECT * FROM `".self::$Options['databaseName']."`.`services` WHERE `id` = ".(int)$cmd->id );
				$num = mysql_num_rows($res);
				if( ! $num ) return;
				$param = mysql_fetch_assoc( $res );
				
				self::DatabaseQuery(
					"DELETE FROM `".self::$Options['databaseName']."`.`services` WHERE `id` = ".(int)$cmd->id
				);
				if( intval($param['daemon']) )
					self::StopService( (int)$cmd->id );
				
				self::WriteCommand( $child->socket, 'OK' );
				break;
		}
	}
	
	static private function HandleGetServiceCmd( $child, $cmd ) {
		$id = (int) $cmd->id;
		
		$res = self::DatabaseQuery( "SELECT * FROM `".self::$Options['databaseName']."`.`services` WHERE `id` = ".$id );
		$num = mysql_num_rows($res);
		if( ! $num ) {
			self::WriteCommand( $child->socket, 'ERROR' );
			return;
		}
		
		$srv = mysql_fetch_assoc( $res );
		
		if( $srv['daemon'] ) {
			foreach ( self::$Children as $c ) {
				if( $c->type == 'service' && $c->serviceId == $id ) {
					self::WriteCommand( $child->socket, 'OK', array( 'pid' => $c->pid ) );
					return;
				}
			}
			self::WriteCommand( $child->socket, 'ERROR' );
			return;
		}
		else {
			$pid = self::StartService( $id, $child->pid );
			self::WriteCommand( $child->socket, 'OK', array( 'pid' => $pid ) );
			return;
		}
	}
	
	static private function HandleGroupCmd( $child, $cmd ) {
		switch ( $cmd->action ) {
			case 'join':
				$child->groups[] = $cmd->group;
			break;
			case 'part':
				for( $n = 0; $n < count($child->groups); $n++ ) {
					if( $child->groups[$n] == $cmd->group ) {
						array_splice( $child->groups, $n, 1 );
						break;
					}
				}
			break;
		}
	}
	
	static private function GetApplicationOwnerName( $app ) {
		$dat = explode( '/', $app );
		if( count($dat) == 1 ) return '';
		return $dat[0];
	}
	
	static private function StartApplication( $client, $clientid ) {
		$port = self::GetFreePort();
		list($parentCommandPipe,$childCommandPipe) = self::GetSocketPair();
		list($parentEventPipe,$childEventPipe) = self::GetSocketPair();
		$pid = self::ForkChild();
		
		if( $pid ) {	// Parent
			socket_close( $childCommandPipe );
			socket_close( $childEventPipe );
			$childObj = new DaemonChild;
			$childObj->type = 'app';
			$childObj->pid = $pid;
			$childObj->port = $port;
			$childObj->socket = $parentCommandPipe;
			$childObj->eventPipe = $parentEventPipe;
			$childObj->started = time();
			$childObj->lastKeepalive = time();
			$childObj->clientId = $clientid;
			self::$Children[$pid] = $childObj;
			self::$UsedPorts[] = $port;
			self::ResetDatabase();
			socket_close( $client );
			return;
		}
		else {		// Child
			self::$ProcessType = NULL;
			self::$Shutdown = FALSE;
			socket_close( $parentCommandPipe );
			socket_close( $parentEventPipe );
			socket_close( self::$Socket );
			self::ResetDatabase();
			
			ini_set( 'include_path', ini_get('include_path').':'.self::$Options['sitefusionPath'].':' );
			
			// Login checking
			$loginCmd = self::ReadCommand( $client );
			if( $loginCmd->command != 'LOGIN' ) {
				socket_close( $client );
				exit();
			}
			
			include_once( 'app.php.inc' );
			ApplicationProcess::StartServer( $client, $childCommandPipe, $childEventPipe, $loginCmd, $port, $clientid );
		}
	}
	
	static private function StartJob( $job, $args, $mailTo = NULL ) {
		$pid = self::ForkChild();
		
		if( $pid ) {	// Parent
			$childObj = new DaemonChild;
			$childObj->type = 'job';
			$childObj->pid = $pid;
			$childObj->port = 0;
			$childObj->started = time();
			self::$Children[$pid] = $childObj;
			self::ResetDatabase();
			
			$param = new StdClass;
			$param->app = $job;
			$param->args = $args;
			$param->sid = md5( time() . $job . $args . mt_rand() );
			$param->ident = '';
			$param->type = 'job';
			$param->user = 'sitefusion';
			$param->ip = self::$Address;
			
			self::ChildStarted( $childObj, $param );
			return;
		}
		else {		// Child
			self::$ProcessType = NULL;
			socket_close( self::$Socket );
			self::ResetDatabase();
			
			ini_set( 'include_path', ini_get('include_path').':'.self::$Options['sitefusionPath'].':' );
			
			include_once( 'job.php.inc' );
			JobProcess::StartServer( $job, $args, $mailTo );
		}
	}
	
	static private function StartService( $id, $master = NULL ) {
		$res = self::DatabaseQuery( "SELECT * FROM `".self::$Options['databaseName']."`.`services` WHERE `id` = ".(int)$id );
		$num = mysql_num_rows($res);
		if( !$num ) return;
		$srv = mysql_fetch_assoc( $res );
		if( ($master && $srv['daemon']) || ((!$master) && (!$srv['daemon'])) )
			return;
		
		list($parentCommandPipe,$childCommandPipe) = self::GetSocketPair();
		list($parentEventPipe,$childEventPipe) = self::GetSocketPair();
		$sessionId = md5( time() . $srv['service'] . $srv['owner'] . mt_rand() );
		
		$pid = self::ForkChild();
		
		if( $pid ) {	// Parent
			socket_close( $childCommandPipe );
			socket_close( $childEventPipe );
			$childObj = new DaemonChild;
			$childObj->type = 'service';
			$childObj->pid = $pid;
			$childObj->port = 0;
			$childObj->socket = $parentCommandPipe;
			$childObj->eventPipe = $parentEventPipe;
			$childObj->started = time();
			$childObj->lastKeepalive = time();
			$childObj->serviceId = $srv['id'];
			$childObj->master = $master;
			$childObj->daemon = $srv['daemon'];
			self::$Children[$pid] = $childObj;
			self::ResetDatabase();
			
			$param = new StdClass;
			$param->app = $srv['owner'] . ($srv['owner'] == '' ? '':'/') . $srv['service'];
			$param->args = $srv['args'];
			$param->sid = $sessionId;
			$param->ident = '';
			$param->type = 'service';
			$param->user = 'sitefusion';
			$param->ip = self::$Address;
			
			self::ChildStarted( $childObj, $param );
			return $pid;
		}
		else {		// Child
			self::$ProcessType = NULL;
			self::$Shutdown = FALSE;
			socket_close( $parentCommandPipe );
			socket_close( $parentEventPipe );
			socket_close( self::$Socket );
			self::ResetDatabase();
			
			ini_set( 'include_path', ini_get('include_path').':'.self::$Options['sitefusionPath'].':' );
			
			include_once( 'service.php.inc' );
			ServiceProcess::StartServer( $childCommandPipe, $childEventPipe, $srv['owner'], $srv['service'], $srv['args'], $sessionId );
		}
	}
	
	static private function StopService( $id ) {
		foreach( self::$Children as $pid => $child ) {
			if( $child->type == 'service' && $child->serviceId == $id ) {
				if( ! posix_kill( $pid, SIGTERM ) )
					self::RemoveChild( $pid );
				break;
			}
		}
	}
	
	static private function RestartService( $id ) {
		self::StopService( $id );
		self::StartService( $id );
	}
	
	static private function StartDaemonServices() {
		$res = self::DatabaseQuery( "SELECT `id` FROM `".self::$Options['databaseName']."`.`services` WHERE `daemon` = 1" );
		
		$num = mysql_num_rows( $res );
		
		for( $n = 0; $n < $num; $n++ ) {
			list($id) = mysql_fetch_row( $res );
			
			self::StartService( $id );
		}
	}
	
	static private function ChangeIdentity() {
		if( ! (($uinfo = posix_getpwnam( DaemonProcess::$Options['systemUser'] )) && posix_setuid( $uinfo['uid'] )) ) {
			self::Log( 'Cannot change identity to user '.DaemonProcess::$Options['systemUser'] );
		}
		
		if( ! (($ginfo = posix_getgrnam( DaemonProcess::$Options['systemGroup'] )) && posix_setuid( $ginfo['gid'] )) ) {
			self::Log( 'Cannot change identity to group '.DaemonProcess::$Options['systemGroup'] );
		}
	}
	
	static private function GetChildBySocket( $socket ) {
		foreach ( self::$Children as $child ) {
			if( isset($child->socket) && ($socket == $child->socket || $socket == $child->eventPipe) )
				return $child;
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
	
	static private function ForkChild() {
		$pid = pcntl_fork();
		
		if( $pid == -1 )
			throw new SFException( "Could not fork" );
		
		return $pid;
	}
	
	static private function GetFreePort() {
		$port = self::$Options['port'];
		
		while( in_array(++$port,self::$UsedPorts) ) {}
		
		return $port;
	}
	
	static private function GetSocketPair() {
		$sockets = array();
		if( socket_create_pair(AF_UNIX, SOCK_STREAM, 0, $sockets) === false ) {
			throw new SFException( "socket_create_pair() failed: reason: ".socket_strerror(socket_last_error()) );
		}
		return $sockets;
	}
	
	static protected function SetSignalHandlers() {
		pcntl_signal( SIGHUP, array( 'DaemonProcess', 'SignalHandler' ) );
		pcntl_signal( SIGTERM, array( 'DaemonProcess', 'SignalHandler' ) );
		pcntl_signal( SIGINT, array( 'DaemonProcess', 'SignalHandler' ) );
		pcntl_signal( SIGCHLD, array( 'DaemonProcess', 'SignalHandler' ) );
	}
	
	static public function SignalHandler( $signo ) {
		switch ( $signo ) {
			case SIGHUP:
				break;
			
			case SIGINT:
			case SIGTERM:
				if( self::$Shutdown )
					self::$ShutdownKill = TRUE;
				else
					self::$Shutdown = TRUE;
				break;
			
			case SIGCHLD:
				$pid = pcntl_waitpid(-1, $status);
				
				if( $pid > 0 && pcntl_wifexited($status) && pcntl_wexitstatus($status) ) {
					$read = array(STDIN);
					$write = NULL;
					$except = NULL;
					
					if( stream_select( $read, $write, $except, 0, 10000 ) > 0 ) {
						self::$Children[$pid]->error = fread( $read[0], 8192 );
						self::$Children[$pid]->errorTimestamp = time();
						if( self::$Options['debug'] )
							self::Log( 'Fatal error in session ('.self::$Children[$pid]->sessionId.'): '.trim(self::$Children[$pid]->error) );
					}
				}
				break;
		}
	}
}


class SiteFusionProcess
{
	static protected $Options;
	static protected $Socket;
	static protected $Database;
	static protected $Log = FALSE;
	static protected $Address;
	static protected $Port;
	static protected $Daemonized = FALSE;
	static protected $Shutdown = FALSE;
	static protected $ShutdownKill = FALSE;
	static protected $ProcessType = NULL;
	
	
	protected function __construct() {}
	
	static protected function OpenLog() {
		try {
			if(! file_exists( self::$Options['logfile'] ) ) {
				touch( self::$Options['logfile'] );
				if( self::$Options['changeIdentity'] ) {
					chown( self::$Options['logfile'], self::$Options['systemUser'] );
					chmod( self::$Options['logfile'], 0664 );
				}
			}
		
			if(! self::$Log = fopen( self::$Options['logfile'], 'a' ) ) {
				echo "Unable to open logfile ".self::$Options['logfile']." for writing, exiting...\n";
				return FALSE;
			}
		}
		catch( SFException $ex ) {
			echo $ex->getMessage() . "\n";
			return FALSE;
		}
		
		return TRUE;
	}
	
	static protected function Log( $text ) {
		if(! self::$Daemonized )
			echo $text."\n";
		if( self::$Log )
			fwrite( self::$Log, date('d-m-Y H:i:s: ').$text."\n" );
	}
	
	static protected function GetDatabaseConnection() {
		$tries = 0;
		
		while( $tries++ <= self::$Options['databaseFailureRetries'] ) {
			self::$Database = mysql_connect( self::$Options['databaseHost'], self::$Options['databaseUsername'], self::$Options['databasePassword'], TRUE );
			if( ! self::$Database ) {
				self::Log( 'Database connection failed, retry '.$tries.' in '.self::$Options['databaseFailureRetryInterval'].'ms' );
				usleep( self::$Options['databaseFailureRetryInterval'] * 1000 );
			}
			else break;
		}
		
		if( ! self::$Database )
			throw new SFException( 'Database connection failed: ' . mysql_error() );
		
		if( ! mysql_select_db( self::$Options['databaseName'], self::$Database ) )
			throw new SFException( "Failed to select database: " . mysql_error() );
	}
	
	static protected function InitDatabase() {
		self::GetDatabaseConnection();
		
		if( ! mysql_query( "TRUNCATE TABLE `".self::$Options['databaseName']."`.`processes`", self::$Database ) )
			throw new SFException( "Failed to truncate table 'processes': " . mysql_error() );
	}
	
	static protected function CloseDatabase() {
		if( ! self::$Database )
			self::GetDatabaseConnection();
		
		if( ! mysql_query( "TRUNCATE TABLE `".self::$Options['databaseName']."`.`processes`", self::$Database ) )
			throw new SFException( "Failed to truncate table 'processes': " . mysql_error() );
	}
	
	static protected function ResetDatabase() {
		try {
			mysql_close( self::$Database );
		}
		catch ( SFException $ex ) {}
		
		self::GetDatabaseConnection();
	}
	
	static protected function DatabaseQuery( $sql ) {
		$res = FALSE;
		try {
			$res = @mysql_query( $sql, self::$Database );
		}
		catch ( Exception $ex ) {}
		
		if( ! $res )
			self::GetDatabaseConnection();
	
		$res = mysql_query( $sql, self::$Database );
		if( ! $res )
			throw new SFException( "Database Query Error: " . mysql_error() );
	
		return $res;
	}
	
	static protected function OpenSocket( $address, $port ) {
		if (($sock = socket_create(AF_INET, SOCK_STREAM, SOL_TCP)) === false) {
			throw new SFException( "socket_create() failed: reason: " . socket_strerror(socket_last_error()) );
		}
		
		$bound = FALSE;
		
		while( !$bound ) {
			try {
				if (socket_bind($sock, $address, $port) === false) {
					throw new SFException( "socket_bind() failed: reason: " . socket_strerror(socket_last_error($sock)) );
				}
				$bound = TRUE;
			}
			catch ( SFException $ex ) { sleep(1); }
		}

		if (socket_listen($sock, 5) === false) {
			throw new SFException( "socket_listen() failed: reason: " . socket_strerror(socket_last_error($sock)) );
		}
		
		socket_set_nonblock($sock); 
		
		return $sock;
	}
	
	static protected function OpenUnixSocket( $address ) {
		if (($sock = socket_create(AF_UNIX, SOCK_STREAM, 0)) === false) {
			throw new SFException( "socket_create() failed: reason: " . socket_strerror(socket_last_error()) );
		}
		
		$bound = FALSE;
		
		while( !$bound ) {
			try {
				if (socket_bind($sock, $address) === false) {
					throw new SFException( "socket_bind() failed: reason: " . socket_strerror(socket_last_error($sock)) );
				}
				$bound = TRUE;
			}
			catch ( SFException $ex ) { sleep(1); }
		}

		if (socket_listen($sock, 5) === false) {
			throw new SFException( "socket_listen() failed: reason: " . socket_strerror(socket_last_error($sock)) );
		}
		
		socket_set_nonblock($sock); 
		
		return $sock;
	}
	
	static protected function SetSignalHandlers() {
		pcntl_signal( SIGHUP, array( 'SiteFusionProcess', 'SignalHandler' ) );
		pcntl_signal( SIGTERM, array( 'SiteFusionProcess', 'SignalHandler' ) );
		pcntl_signal( SIGINT, array( 'SiteFusionProcess', 'SignalHandler' ) );
		pcntl_signal( SIGCHLD, array( 'SiteFusionProcess', 'SignalHandler' ) );
	}
	
	static public function SignalHandler( $signo ) {
		switch ( $signo ) {
			case SIGHUP:
				break;
			
			case SIGINT:
			case SIGTERM:
				self::$Shutdown = TRUE;
				break;
			
			case SIGCHLD:
				pcntl_waitpid(-1, $status);
				break;
		}
	}
	
	static protected function WriteCommand( $socket, $cmd, $param = NULL, $data = NULL ) {
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
			throw new SFException( 'WriteCommand: socket_write() failed: '.socket_strerror(socket_last_error($socket)) );
		if( $data !== NULL ) {
			if( socket_write( $socket, $data, strlen($data) ) === FALSE )
				throw new SFException( 'WriteCommand: socket_write() failed: '.socket_strerror(socket_last_error($socket)) );
		}
	}
	
	static protected function ReadCommand( $socket ) {
		$cmddata = socket_read( $socket, 4096, PHP_NORMAL_READ );
		if( $cmddata === FALSE )
			throw new SFException( 'ReadCommand: socket_read() failed: '.socket_strerror(socket_last_error($socket)) );
	
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
				$sbuf = socket_read( $socket, min($dl-strlen($data),2048), PHP_BINARY_READ );
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
}



class DaemonChild
{
	public $type;
	
	public $pid;
	public $port;
	public $socket;
	public $started;
	public $lastKeepalive;
	
	public $loggedIn = FALSE;
	public $sessionId;
	public $user;
	public $ident;
	public $ip;
	public $app;
	public $args;
	
	public $groups = array();
	
	public $clientId;
	public $error;
	public $errorTimestamp;
}
