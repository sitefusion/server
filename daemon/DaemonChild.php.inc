<?php

/**
 * @package Daemon
 * @subpackage Master
*/

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
