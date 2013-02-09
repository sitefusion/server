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
 * @subpackage Trees
*/


/**
 * Tree element
 * 
 * @link https://developer.mozilla.org/en/XUL/tree
 * @package API
 * @subpackage Trees
*/

class XULTree extends BranchNode
{
	public $remoteConstructor = 'Tree';
	public $initAttributes = array( 'seltype', 'enableColumnDrag', 'editable', 'selstyle', 'hidecolumnpicker', 'rows' );
	public $selectedRows = array();
	
	
	/**
	 * Dynamic Constructor
	 *
	 * @param string	$seltype
	 * @param bool		$enableColumnDrag
	 * @param string	$selstyle
	 * @param int		$flex
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_string($args[0]) )
				$this->seltype( array_shift($args) );
			if( count($args) && is_bool($args[0]) )
				$this->enableColumnDrag( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->selstyle( array_shift($args) );
			if( count($args) && is_int($args[0]) )
				$this->flex( array_shift($args) );
			
			parent::__construct( $args );
		}
		else parent::__construct();
		
		$this->setEventHandler( 'yield', $this, 'yieldCollect' );
	}
	
	public function yieldCollect() {
		$args = func_get_args();
		$e = array_shift( $args );
		
		$this->selectedRows = $args;
	}

	public function seltype( $type = NULL ) {
		return $this->attributeMethod( 'seltype', $type );
	}

	public function enableColumnDrag( $state = NULL ) {
		return $this->attributeMethod( 'enableColumnDrag', $state );
	}
	
	public function editable( $state = NULL ) {
		return $this->attributeMethod( 'editable', ($state == TRUE ? 'true':'false') );
	}

	public function selstyle( $style = NULL ) {
		return $this->attributeMethod( 'selstyle', ($style === NULL ? NULL : ($style == 'primary' ? 'primary':'')) );
	}
	
	public function hidecolumnpicker( $hide = NULL ) {
		return $this->attributeMethod( 'hidecolumnpicker', $hide );
	}
	
	public function rows( $rows = NULL ) {
		return $this->attributeMethod( 'rows', $rows );
	}
	
	public function select( $item ) {
		if( $this->isRegistered ) {
			if( $item instanceof XULTreeItem )
				$this->callMethod( 'select', $item );
			else
				$this->callMethod( 'element.view.selection.select', $item );
		}
	}
}


/**
 * Treecols element
 * 
 * @link https://developer.mozilla.org/en/XUL/treecols
 * @package API
 * @subpackage Trees
*/

class XULTreeCols extends BranchNode
{
	public $remoteConstructor = 'TreeCols';
}


/**
 * Treecol element
 * 
 * @link https://developer.mozilla.org/en/XUL/treecol
 * @package API
 * @subpackage Trees
*/

class XULTreeCol extends BranchNode
{
	public $remoteConstructor = 'TreeCol';
	public $initAttributes = array( 'primary', 'type', 'editable', 'src', 'ignoreincolumnpicker', 'hideheader', 'hidden', 'fixed' );
	public $type = 'text';

	
	/**
	 * Dynamic Constructor
	 *
	 * @param string	$label
	 * @param int		$flex
	 * @param bool		$primary
	 * @param string	$dataId
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_string($args[0]) )
				$this->label( array_shift($args) );
			if( count($args) && is_integer($args[0]) )
				$this->flex( array_shift($args) );
			if( count($args) && is_bool($args[0]) )
				$this->primary( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->dataId = array_shift($args);
			
			parent::__construct( $args );
		}
		else parent::__construct();
	}
	
	public function primary( $state = NULL ) {
		return $this->attributeMethod( 'primary', $state );
	}
	
	public function type( $state = NULL ) {
		return $this->attributeMethod( 'type', $state );
	}
	
	public function src( $src = NULL ) {
		if( $src === NULL )
			return (isset($this->src) ? $this->src : NULL);
		
		$this->src = $src;
		
		if( $this->isRegistered ) {
			$this->callMethod( 'src', $this->src );
			$this->setAttribute( 'class', 'treecol-image' );
		}
		
		return $this;
	}
	
	public function editable( $state = NULL ) {
		return $this->attributeMethod( 'editable', ($state == TRUE ? 'true':'false') );
	}
	
	public function ignoreincolumnpicker( $state = NULL ) {
		return $this->attributeMethod( 'ignoreincolumnpicker', $state );
	}

	public function hideheader( $state = NULL ) {
		return $this->attributeMethod( 'hideheader', $state );
	}
	
	public function hidden( $state = NULL ) {
		return $this->attributeMethod( 'hidden', $state );
	}
	
	public function fixed( $state = NULL ) {
		return $this->attributeMethod( 'fixed', $state );
	}
}


/**
 * Treechildren element
 * 
 * @link https://developer.mozilla.org/en/XUL/treechildren
 * @package API
 * @subpackage Trees
*/

class XULTreeChildren extends BranchNode
{
	public $remoteConstructor = 'TreeChildren';
}


/**
 * Treeitem element
 * 
 * @link https://developer.mozilla.org/en/XUL/treeitem
 * @package API
 * @subpackage Trees
*/

class XULTreeItem extends BranchNode
{
	public $remoteConstructor = 'TreeItem';
	public $initAttributes = array( 'container', 'open', 'src', 'mode' );
	
	
	/**
	 * Dynamic Constructor
	 *
	 * @param string	$label
	 * @param bool		$container
	 * @param bool		$open
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_string($args[0]) )
				$this->label( array_shift($args) );
			if( count($args) && is_bool($args[0]) )
				$this->container( array_shift($args) );
			if( count($args) && is_bool($args[0]) )
				$this->open( array_shift($args) );
			
			parent::__construct( $args );
		}
		else parent::__construct();
	}
	
	public function container( $state = NULL ) {
		return $this->attributeMethod( 'container', $state );
	}

	public function open( $state = NULL ) {
		return $this->attributeMethod( 'open', $state );
	}
	
	public function src( $src = NULL ) {
		return $this->attributeMethod( 'src', $src );
	}
	
	public function mode( $mode = NULL ) {
		return $this->attributeMethod( 'mode', $mode );
	}
}

/**
 * Treerow element
 * 
 * @link https://developer.mozilla.org/en/XUL/treerow
 * @package API
 * @subpackage Trees
*/

class XULTreeRow extends BranchNode
{
	public $remoteConstructor = 'TreeRow';
}


/**
 * Treecell element
 * 
 * @link https://developer.mozilla.org/en/XUL/treecell
 * @package API
 * @subpackage Trees
*/

class XULTreeCell extends BranchNode
{
	public $remoteConstructor = 'TreeCell';
	public $initAttributes = array( 'src', 'properties' );

	
	/**
	 * Dynamic Constructor
	 *
	 * @param string	$label
	 * @param string	$src
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_string($args[0]) )
				$this->label( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->src( array_shift($args) );
			
			parent::__construct( $args );
		}
		else parent::__construct();
	}
	
	public function src( $src = NULL ) {
		if( $src === NULL )
			return (isset($this->src) ? $this->src : NULL);
		
		$this->src = $src;
		
		if( $this->isRegistered )
			$this->callMethod( 'src', array($src) );
		
		return $this;
	}

	
	public function properties( $prop = NULL ) {
		return $this->attributeMethod( 'properties', $prop );
	}
}


/**
 * Treeseparator element
 * 
 * @link https://developer.mozilla.org/en/XUL/treeseparator
 * @package API
 * @subpackage Trees
*/

class XULTreeSeparator extends Node
{
	public $remoteConstructor = 'TreeSeparator';
}

