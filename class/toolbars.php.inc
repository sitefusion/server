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
 * @subpackage Toolbars
*/


/**
 * ToolbarSeparator element
 * 
 * @link https://developer.mozilla.org/en/XUL/toolbarseparator
*/

class XULToolBarSeparator extends Node
{
	public $remoteConstructor = 'ToolBarSeparator';
}


/**
 * Toolbox element
 * 
 * @link https://developer.mozilla.org/en/XUL/toolbox
*/

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


/**
 * Toolbar element
 * 
 * @link https://developer.mozilla.org/en/XUL/toolbar
*/

class XULToolBar extends BranchNode
{
	public $remoteConstructor = 'ToolBar';
}


/**
 * Toolbarbutton element
 * 
 * @link https://developer.mozilla.org/en/XUL/toolbarbutton
*/

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
