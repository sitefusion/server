<?php

class SFDBUser extends DatabaseObject
{
	public $_id_field = 'serial';
	protected $_id_auto_inc = FALSE;
	protected $_table = 'users';
	protected $_database = 'sitefusion';

	protected $_fields = array(
		'serial' => 'serial',
		'name' => 'name',
		'company' => 'company'
	);

	static public function GetAll() {
		$db = new Database( 'sitefusion' );
		$res = $db->query( "SELECT * FROM `users`" );
		$num = mysql_num_rows( $res );

		$ret = array();

		for( $n = 0; $n < $num; $n++ ) {
			$ret[] = new self( $res );
		}

		return $ret;
	}
}

class SFDBSubscription extends DatabaseObject
{
	public $_id_field = 'id';
	protected $_id_auto_inc = TRUE;
	protected $_table = 'subscriptions';
	protected $_database = 'sitefusion';

	protected $_fields = array(
		'id' => 'id',
		'serial' => 'serial',
		'displayname' => 'displayname',
		'description' => 'description',
		'uri' => 'uri',
		'application' => 'application',
		'args' => 'args'
	);

	static public function GetAll( $serial = NULL ) {
		$db = new Database( 'sitefusion' );
		$res = $db->query( "SELECT * FROM `subscriptions`" . ($serial === NULL ? '':" WHERE `serial` = '".mysql_escape_string($serial)."'") );
		$num = mysql_num_rows( $res );

		$ret = array();

		for( $n = 0; $n < $num; $n++ ) {
			$ret[] = new self( $res );
		}

		return $ret;
	}
}
