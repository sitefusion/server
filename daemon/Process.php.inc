<?php

/**
 * @package Daemon
 * @subpackage Process
*/
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
			try {
				self::$Database = mysql_connect( self::$Options['databaseHost'], self::$Options['databaseUsername'], self::$Options['databasePassword'], TRUE );
			}
			catch ( Exception $ex ) {
				self::$Database = NULL;
			}
			
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
			$res = mysql_query( $sql, self::$Database );
		}
		catch ( Exception $ex ) {}
		
		if( ! $res ) {
			self::GetDatabaseConnection();
			$res = mysql_query( $sql, self::$Database );
			if( ! $res )
				throw new SFException( "Database Query Error: " . mysql_error() );
		}
		
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
	
	static public function ChangeIdentity( $user = NULL, $group = NULL ) {
		if( ! (($ginfo = posix_getgrnam( $group = ($group !== NULL ? $group : DaemonProcess::$Options['systemGroup']) )) && posix_setgid( $ginfo['gid'] )) ) {
			self::Log( 'Cannot change identity to group '.$group );
		}
		
		if( ! (($uinfo = posix_getpwnam( $user = ($user !== NULL ? $user : DaemonProcess::$Options['systemUser']) )) && posix_setuid( $uinfo['uid'] )) ) {
			self::Log( 'Cannot change identity to user '.$user );
		}
	}
	
	static protected function WriteCommand( $socket, $cmd, $param = NULL, $data = NULL ) {
		if(! is_array($param) )
			$param = array();
		
		if( $data !== NULL )
			$param['DATALENGTH'] = strlen($data);
		
		$msg = $cmd;
		
		foreach ( $param as $key => $val ) {
			if (is_array($val)) continue;
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