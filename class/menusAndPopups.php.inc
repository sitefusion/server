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
 * @package SiteFusion
 * @subpackage MenusAndPopups
*/


class XULMenuBar extends BranchNode
{
	public $remoteConstructor = 'MenuBar';
}

class XULMenuPopup extends BranchNode
{
	public $remoteConstructor = 'MenuPopup';
}


class XULMenuSeparator extends Node
{
	public $remoteConstructor = 'MenuSeparator';
}

class XULMenu extends BranchNode
{
	public $remoteConstructor = 'Menu';

	
	/**
	 * Dynamic Constructor
	 *
	 * @param string	$label
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_string($args[0]) )
				$this->label( array_shift($args) );
			
			parent::__construct( $args );
		}
		else parent::__construct();
	}
}


class XULMenuItem extends BranchNode
{
	public $remoteConstructor = 'MenuItem';
	public $initAttributes = array( 'type', 'name', 'image' );

	
	/**
	 * Dynamic Constructor
	 *
	 * @param string	$label
	 * @param string	$value
	 * @param string	$type
	 * @param string	$name
	 * @param string	$image
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_string($args[0]) )
				$this->label( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->value( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->type( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->name( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->image( array_shift($args) );
			
			parent::__construct( $args );
		}
		else parent::__construct();
	}
	
	public function type( $type ) {
		return $this->attributeMethod( 'type', (string) $type );
	}
	
	public function name( $name ) {
		return $this->attributeMethod( 'name', (string) $name );
	}

	public function image( $src = NULL ) {
		if( $src === NULL )
			return (isset($this->image) ? $this->image : NULL);
		
		$this->image = $src;
		
		if( $this->isRegistered ) {
			$this->callMethod( 'image', $this->image );
			$this->setAttribute( 'class', 'menuitem-iconic' );
		}
		
		return $this;
	}
}


class XULPopupSet extends BranchNode
{
	public $remoteConstructor = 'PopupSet';
}

class XULPopup extends BranchNode
{
	public $remoteConstructor = 'Popup';
	public $initAttributes = array( 'position', 'left', 'top' );

	
	/**
	 * Dynamic Constructor
	 *
	 * @param string	$position
	 * @param int		$left
	 * @param int		$top
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_string($args[0]) )
				$this->position( array_shift($args) );
			if( count($args) && is_int($args[0]) )
				$this->left( array_shift($args) );
			if( count($args) && is_int($args[0]) )
				$this->top( array_shift($args) );
			
			parent::__construct( $args );
		}
		else parent::__construct();
	}
	
	public function position( $pos = NULL ) {
		return $this->attributeMethod( 'position', (string) $pos );
	}

	public function left( $pix = NULL ) {
		return $this->attributeMethod( 'left', (int) $pix );
	}

	public function top( $pix = NULL ) {
		return $this->attributeMethod( 'top', (int) $pix );
	}
}


class XULToolTip extends BranchNode
{
	public $remoteConstructor = 'ToolTip';
	public $initAttributes = array( 'position', 'noautohide' );

	
	/**
	 * Dynamic Constructor
	 *
	 * @param bool		$noautohide
	 * @param string	$label
	 * @param string	$position
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_bool($args[0]) )
				$this->noautohide( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->label( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->position( array_shift($args) );
			
			parent::__construct( $args );
		}
		else parent::__construct();
	}
	
	public function position( $pos = NULL ) {
		return $this->attributeMethod( 'position', (string) $pos );
	}

	public function noautohide( $bool = NULL ) {
		return $this->attributeMethod( 'noautohide', $bool );
	}
}


class XULPanel extends BranchNode
{
	public $remoteConstructor = 'Panel';
	public $initAttributes = array( 'position', 'left', 'top' );

	
	/**
	 * Dynamic Constructor
	 *
	 * @param string	$position
	 * @param int		$left
	 * @param int		$top
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_string($args[0]) )
				$this->position( array_shift($args) );
			if( count($args) && is_int($args[0]) )
				$this->left( array_shift($args) );
			if( count($args) && is_int($args[0]) )
				$this->top( array_shift($args) );
			
			parent::__construct( $args );
		}
		else parent::__construct();
	}
	
	public function position( $pos = NULL ) {
		return $this->attributeMethod( 'position', (string) $pos );
	}

	public function left( $pix = NULL ) {
		return $this->attributeMethod( 'left', (int) $pix );
	}

	public function top( $pix = NULL ) {
		return $this->attributeMethod( 'top', (int) $pix );
	}
}