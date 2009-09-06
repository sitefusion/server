<?php

/**
 * @package SiteFusion
 * @subpackage HelperElements
*/


class XULGrippy extends Node
{
	public $remoteConstructor = 'Grippy';
}


class XULArrowScrollBox extends BranchNode
{
	public $remoteConstructor = 'ArrowScrollBox';
	public $orient = 'vertical';
	
	
	/**
	 * Dynamic Constructor
	 *
	 * @param int		$flex
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_int($args[0]) )
				$this->flex( array_shift($args) );
						
			parent::__construct( $args );
		}
		else parent::__construct();
	}
}

