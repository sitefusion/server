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
 * @package Libraries
 * @subpackage Database
*/


class Database
{
	public static $hostname = 'localhost';
	public static $username = '';
	public static $password = '';
	public static $failureRetries = 5;
	public static $failureRetryInterval = 1000;
	public static $connection = NULL;
	private static $activeDb = NULL;
	
	public function __construct( $db = NULL ) {
		if( $db === NULL )
			throw new SFException( "Database: No database name given", ERR_REPORT_APP );
		
		$this->db = $db;
		
		if( ! self::$connection )
			$this->initialize( $db );
		else
			$this->selectDb( $db );
	}
	
	public static function Disconnect() {
		if( self::$connection )
			mysql_close( self::$connection );
	}
	
	public static function DatabaseExists ($db)
	{
		if(! mysql_select_db( $db) )
			return FALSE;
		else return TRUE;
	}
	
	public function selectDb( $db ) {
		if( self::$activeDb == $db )
			return;
		
		try {
			if(! mysql_select_db( $db, self::$connection ) )
				throw new SFException( 'Select DB failed' );
		}
		catch ( SFException $ex ) {
			$this->initialize( $db );
		}
	
		$this->db = $db;
		self::$activeDb = $db;
	}
	
	public function initialize( $db ) {
		$tries = 0;
		
		while( $tries++ <= self::$failureRetries ) {
			self::$connection = mysql_connect( self::$hostname, self::$username, self::$password, TRUE );
			if( ! self::$connection ) {
				usleep( self::$failureRetryInterval * 1000 );
			}
			else break;
		}
		
		if( ! self::$connection )
			throw new SFException( 'Database connection failed: ' . mysql_error(), ERR_REPORT_APP );
		
		$this->db = $db;
		
		if( $db && (! mysql_select_db( $db, self::$connection )) )
			throw new SFException( "Failed to select database: " . mysql_error(), ERR_REPORT_APP );
		
		mysql_set_charset( 'utf8', self::$connection );
		self::$activeDb = $db;
	}
	
	public function getCol( $table, $col, $where ) {
		$where = (array) $where;
		$str = array();
		foreach ( $where as $key => $val ) {
			$str[] = "`".mysql_real_escape_string($key)."` = '".mysql_real_escape_string($val)."'";
		}
		$res = $this->query( "SELECT `".mysql_real_escape_string($col)."` FROM `".mysql_real_escape_string($table)."` WHERE " . implode( ' && ', $str ) );
		$num = mysql_num_rows( $res );
		
		if( $num == 0 ) return NULL;
		if( $num == 1 ) {
			list($val) = mysql_fetch_row( $res );
			return $val;
		}
		else {
			$ret = array();
			for( $n = 0; $n < $num; $n++ ) {
				list($val) = mysql_fetch_row( $res );
				$ret[] = $val;
			}
			return $ret;
		}
	}
	
	public function query( $sql ) {
		$this->selectDb( $this->db );
		
		$res = FALSE;
		try {
			$res = mysql_query( $sql, self::$connection );
		}
		catch ( Exception $ex ) {}
		
		if( ! $res ) {
			$this->initialize( $this->db );
			$res = mysql_query( $sql, self::$connection );
			if( ! $res )
				self::error();
		}
		
		return $res;
	}
	
	public function insert( $table, $array1, $array2 = NULL ) {
		$this->selectDb( $this->db );
		
		$sql = 'REPLACE INTO `'.$table.'` ';
		
		if( $array2 !== NULL )
			$sql .= Database::quoteCols($array1);
		
		$sql .= Database::quoteValues( ($array2 === NULL) ? $array1:$array2 );
		
		if(! $this->query( $sql ) )
			self::error();
	}

	public function update( $table, $idfield, $id, $array1, $array2 ) {
		$this->selectDb( $this->db );
		
		$sql = 'UPDATE `'.$table.'` SET ';
		
		for( $n = 0; $n < count($array1); $n++ ) {
			$sql .= ($n > 0 ? ',':'') . "`" . $array1[$n] . "`='" . mysql_real_escape_string($array2[$n]) . "'";
		}
		
		$sql .= " WHERE `" . $idfield . "` = '" . mysql_real_escape_string($id) . "' LIMIT 1";
		
		if(! $this->query( $sql ) )
			self::error();
	}
	
	public function delete( $table, $field, $value ) {
		$this->selectDb( $this->db );
		
		if(! $this->query( "DELETE FROM `".$table."` WHERE `".$field."` = '".mysql_real_escape_string($value)."' LIMIT 1" ) )
			self::error();
	}
	
	public static function ExecuteQuery($table, $query) {
		$db = new self($table);
		
		if(is_array($query)) {
		
			$ret = array();
			foreach($query as $id => $q) {
				$ret[$id] = $db->query($q);
			}
			
			return $ret;
		} else {
			return $db->query($query);
		}
	}
	
	static function error() {
		throw new SFException( 'MySQL error: ' . mysql_error(), ERR_REPORT_APP );
	}
	
	static function quoteCols() {
		$args = func_get_args();
		return Database::quoteList( '`', $args );
	}

	static function quoteValues() {
		$args = func_get_args();
		return 'VALUES ' . Database::quoteList( "'", $args );
	}
	
	static function quoteList() {
		$list = func_get_args();
		$quote = array_shift( $list );
		$res = array();
		
		for( $n = 0; $n < count($list); $n++ ) {
			if( is_array($list[$n]) ) {
				array_splice( $list, $n, 1, $list[$n] );
				$n--;
			}
			else {
				if( $list[$n] instanceof DBUnquoted )
					array_push( $res, $list[$n]->string );
				else
					array_push( $res, $quote . mysql_real_escape_string($list[$n]) . $quote );
			}
		}
		
		return '(' . join( ', ', $res ) . ') ';
	}
}



class DBUnquoted
{
	public $string;
	
	public function __construct( $str ) {
		$this->string = $str;
	}
}



class DatabaseObject
{
	public $exists = FALSE;
	
	protected $_database = '';
	protected $_table = '';
	protected $_id_field = 'id';
	protected $_id_auto_inc = TRUE;
	protected $_default_value = '';
	protected $_indices = array();
	protected $_fields = array(	//   $variable => 'database_field'
		'id' => 'id'
	);
	protected $_data = array(
		'id' => ''
	);
	protected $_variables = array();
	protected $_process = array();	//   $variable => 'process_method'
	protected $_before_retrieve;
	protected $_after_retrieve;
	protected $_before_store;
	protected $_after_store;
	protected $_before_delete;
	protected $_after_delete;
	
	private $_update = array();
	
	public function __construct( $id = NULL ) {
		if( $id !== NULL )
			$this->retrieve( $id );
	}
	
	public function __set( $name, $value ) {
		if( array_key_exists( $name, $this->_fields ) ) {
			$this->_data[$name] = $value;
			$this->_update[$name] = TRUE;
		}
		else {
			$this->_variables[$name] = $value;
		}
	}
	
	public function __get( $name ) {
		if( array_key_exists( $name, $this->_fields ) ) {
			if( array_key_exists( $name, $this->_process ) )
				$this->_update[$name] = TRUE;
			
			if(! isset( $this->_data[$name] ) )
				return $this->_default_value;
			else
				return $this->_data[$name];
		}
		else {
			if(! isset( $this->_variables[$name] ) )
				throw new SFException( "Undefined property '$name'", ERR_REPORT_APP );
			
			return $this->_variables[$name];
		}
	}
	
	public function __isset( $name ) {
		if( array_key_exists( $name, $this->_fields ) )
			return isset( $this->_data[$name] );
		else
			return isset( $this->_variables[$name] );
	}
	
	public function __unset( $name ) {
		if( array_key_exists( $name, $this->_fields ) ) {
			$this->_data[$name] = NULL;
			$this->_update[$name] = TRUE;
		}
		else
			unset( $this->_variables[$name] );
	}
	
	public function store() {
		if( isset($this->_before_store) )
			$this->{ $this->_before_store }();
		
		$db = new Database( $this->_database );
		
		$fields = array();
		$values = array();
		
		foreach ( $this->_update as $name => $t ) {
			$fields[] = $this->_fields[$name];
			
			if( array_key_exists( $name, $this->_process ) )
				$value = $this->{ $this->_process[$name] }( $this->_data[$name], TRUE, $name );
			else
				$value = $this->_data[$name];
			
			$values[] = $value;
		}
		
		if( count($fields) ) {
			if( $this->exists )
				$db->update( $this->_table, $this->_id_field, $this->_data[$this->_id_field], $fields, $values );
			else
				$db->insert( $this->_table, $fields, $values );
			
			if( (!$this->exists) && $this->_id_auto_inc )
				$this->_data[$this->_id_field] = mysql_insert_id();
			
			$this->exists = TRUE;
		}
		
		$this->_update = array();

		if( isset($this->_after_store) )
			$this->{ $this->_after_store }();
	}
	
	public function retrieve( $res = NULL, $index = NULL ) {
		if( isset($this->_before_retrieve) )
			$this->{ $this->_before_retrieve }();
		
		$this->_update = array();

		if( ! is_resource($res) ) {
			if( $res === NULL ) {
				if(! isset($this->_data[$this->_id_field]) )
					throw new SFException( "DatabaseObject: Can't retrieve(), no id or resource given", ERR_REPORT_APP );
				$res = $this->_data[$this->_id_field];
			}
			
			$id = $res;
			
			if( $index === NULL )
				$index = $this->_id_field;
			elseif( $index != $this->_id_field && ! in_array($index,$this->_indices) )
				throw new SFException( "DatabaseObject: Unrecognized index field '$index'", ERR_REPORT_APP );
			
			$db = new Database( $this->_database );
			$res = $db->query( "SELECT `" . implode( '`,`', array_values($this->_fields) ) . "` FROM `" . $this->_table . "` WHERE `" . $index . "` = '" . mysql_real_escape_string($id) . "' LIMIT 1" );
			
			if(! is_resource($res) )
				throw new SFException( 'DatabaseObject MySQL Error: ' . mysql_error(), ERR_REPORT_APP );
			if( mysql_num_rows($res) != 1 ) {
				$this->exists = FALSE;
				return $this;
			}
		}
		elseif( mysql_num_rows($res) == 0 ) {
			return FALSE;
		}
		
		$this->exists = TRUE;
		
		$data = mysql_fetch_assoc( $res );
		

		foreach ( $this->_fields as $name => $field ) {
			if( ! isset( $data[$field] ) )
				continue;
			
			if( array_key_exists( $name, $this->_process ) )
				$this->_data[$name] = $this->{ $this->_process[$name] }( $data[$field], FALSE, $name );
			else
				$this->_data[$name] = $data[$field];
		}
		
		if( isset($this->_after_retrieve) )
			$this->{ $this->_after_retrieve }();

		return $this;
	}
	
	public function delete() {
		if( ! $this->exists )
			return FALSE;
		
		if( isset($this->_before_delete) )
			$this->{ $this->_before_delete }();

		$db = new Database( $this->_database );
		$db->delete( $this->_table, $this->_id_field, $this->_data[$this->_id_field] );
			
		if( isset($this->_after_delete) )
				$this->{ $this->_after_delete }();
		
		$this->exists = FALSE;

		return TRUE;
	}
	
	public function getIndex( $index = NULL ) {
		if( $index === NULL )
			$index = $this->_id_field;
		elseif( $index != $this->_id_field && ! in_array($index,$this->_indices) )
			throw new SFException( "DatabaseObject: Unrecognized index field '$index'", ERR_REPORT_APP );
		
		$db = new Database( $this->_database );
		$res = $db->query( "SELECT `" . $index . "` FROM `" . $this->_table . "`" );
		
		if(! is_resource($res) )
			throw new SFException( "DatabaseObject MySQL Error: " . mysql_error(), ERR_REPORT_APP );
		
		$list = array();
		
		while( list($id) = mysql_fetch_row($res) ) {
			$list[] = $id;
		}
		
		return $list;
	}
	
	public function getFields( $sql = FALSE ) {
		return ($sql ? array_values($this->_fields) : array_keys($this->_fields));
	}

	public function addField($php, $sql = NULL){
		$this->_fields[$php] = ($sql ? $sql : $php);
	}
	
	public function phpFieldToSQL( $field ) {
		return ( isset($this->_fields[$field]) ? $this->_fields[$field] : NULL );
	}
	
	public function sqlFieldToPHP( $field ) {
		$fields = array_flip( $this->_fields );
		return ( isset($fields[$field]) ? $fields[$field] : NULL );
	}
	
	public function getTable() {
		return $this->_table;
	}

    public function getDatabase()	{
		return $this->_database;
	}

	public function getId() {
		return $this->{$this->_id_field};
	}
}
