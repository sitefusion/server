<?php

/**
 * @package SiteFusion
 * @subpackage WindowStructure
*/


class XULBrowser extends Node
{
	public $remoteConstructor = 'Browser';
	public $initAttributes = array( 'src' );

	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();

			if( count($args) && is_string($args[0]) )
				$this->src( array_shift($args) );
			if( count($args) && is_int($args[0]) )
				$this->flex( array_shift($args) );

			parent::__construct( $args );
		}
		else parent::__construct();
	}

	public function src( $src = NULL ) {
		if( $src === NULL )
			return $this->src;

		$this->src = $src;

		if( $this->isRegistered )
			$this->callMethod( 'setSrc', array($src) );
	}
	
	public function reload() {
		if( $this->isRegistered )
			$this->callMethod( 'reload' );
	}
}

class XULPrintBox extends XULHTMLBox
{
	public $remoteConstructor = 'PrintBox';
	public $busy = FALSE;
	
	public function attach() {
		parent::attach();
		$this->setEvent( 'ready', MSG_SEND, $this, 'setReady' );
	}
	
	public function printContent( $silent = FALSE ) {
		$this->busy = TRUE;
		$this->callMethod( 'print', array( (bool) $silent ) );
	}
	
	public function setReady( $e ) {
		$this->busy = FALSE;
	}
}

