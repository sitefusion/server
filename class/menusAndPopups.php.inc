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
 * @package API
 * @subpackage MenusAndPopups
*/



/**
 * Menubar element
 * 
 * @link https://developer.mozilla.org/en/XUL/menubar
*/

class XULMenuBar extends BranchNode
{
	public $remoteConstructor = 'MenuBar';
}


/**
 * Menupopup element
 * 
 * @link https://developer.mozilla.org/en/XUL/menupopup
*/

class XULMenuPopup extends BranchNode
{
	public $remoteConstructor = 'MenuPopup';
	
	public function attach() {
		$this->createRemoteObject( $this->hostWindow, ($this->parent instanceof XULMenu), ($this->parent instanceof XULMenu ? $this->parent:NULL) );
		$this->insertElement();
	}
}



/**
 * Menuseparator element
 * 
 * @link https://developer.mozilla.org/en/XUL/menuseparator
*/

class XULMenuSeparator extends Node
{
	public $remoteConstructor = 'MenuSeparator';
}


/**
 * Menu element
 * 
 * @link https://developer.mozilla.org/en/XUL/menu
*/

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
	
	public function attach() {
		$this->createRemoteObject( $this->hostWindow, ($this->parent instanceof XULMenuBar) );
		$this->insertElement();
	}
}



/**
 * Menuitem element
 * 
 * @link https://developer.mozilla.org/en/XUL/menuitem
*/

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
	
	public function attach() {
		parent::attach();
		$this->setEvent( 'yield', MSG_QUEUE, $this, 'yieldCollect' );
		
		// FIXME:
		// This is nescessary to fix situations where menulists that have no
		// explicit selection right after construction are drawed empty instead
		// of showing the first menuitem as the selected item.
		// Remove when Mozilla fixes implicit initial selection on menulists
		if( $this->parent->parent instanceof XULMenuList && count($this->parent->children) == 1 ) {
			$this->parent->parent->selectItem(0);
		}
		// /FIXME: end of workaround code
	}
	
	public function yieldCollect( $e, $val ) {
		$this->checked = $val;
	}
	
	public function type( $type = NULL ) {
		if( $type == 'checkbox' )
		{
			$this->setEventYielder( 'command', $this );
		}
		
		return $this->attributeMethod( 'type', (string) $type );
	}
	
	public function checked( $state = NULL) {
		if ($state === NULL)
			return (isset($this->checked) ? $this->checked : NULL);
		
		$this->checked = (bool)$state;
		if ($this->isRegistered) {
			if ($this->checked)
				$this->setAttribute("checked", "true");
			else
				$this->removeAttribute("checked");
		}
	}
	
	
	public function name( $name = NULL ) {
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


/**
 * Popupset element
 * 
 * @link https://developer.mozilla.org/en/XUL/popupset
*/

class XULPopupSet extends BranchNode
{
	public $remoteConstructor = 'PopupSet';
}


/**
 * Popup element
 * 
 * @link https://developer.mozilla.org/en/XUL/popup
*/

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



/**
 * Tooltip element
 * 
 * @link https://developer.mozilla.org/en/XUL/tooltip
*/

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



/**
 * Panel element
 * 
 * @link https://developer.mozilla.org/en/XUL/panel
*/

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
	
	public function type( $type = NULL ) {
		return $this->attributeMethod( 'type', $type );
	}
	
	public function noautofocus( $var = FALSE ) {
		return $this->attributeMethod( 'noautofocus', ($var ? "true" : "false") );
	}
	
}