<?php

/**
 * @package SiteFusion
 * @subpackage Toolbars
*/


class XULToolBarSeparator extends Node
{
	public $remoteConstructor = 'ToolBarSeparator';
}

class XULToolBox extends BranchNode
{
	public $remoteConstructor = 'ToolBox';
	
	
	/**
	 * Dynamic Constructor
	 *
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		$this->addChild( $dummy = new XULButton( 'dummy' ) );
		$dummy->hidden( TRUE );
		
		$args = func_get_args();
		parent::__construct( $args );
	}
}

class XULToolBar extends BranchNode
{
	public $remoteConstructor = 'ToolBar';
}

class XULToolBarButton extends BranchNode
{
	public $remoteConstructor = 'ToolBarButton';
	public $initAttributes = array( 'image', 'type' );
	
	
	/**
	 * Dynamic Constructor
	 *
	 * @param string	$label
	 * @param string	$image
	 * @param string	$textStyle
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_string($args[0]) )
				$this->label( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->image( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->textStyle( array_shift($args) );
			
			parent::__construct( $args );
		}
		else parent::__construct();
	}
	
	public function image( $src = NULL ) {
		if( $src === NULL )
			return (isset($this->image) ? $this->image : NULL);
		
		$this->image = $src;
		
		if( $this->isRegistered )
			$this->callMethod( 'image', $this->image );
		
		return $this;
	}
	
	public function type( $type = NULL ) {
		return $this->attributeMethod( 'type', $type );
	}
}
