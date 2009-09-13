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
 * @subpackage Layout
*/


class XULBox extends BranchNode
{
	public $remoteConstructor = 'Box';
	
	
	/**
	 * Dynamic Constructor
	 *
	 * @param int		$flex
	 * @param string	$align
	 * @param string	$pack
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_int($args[0]) )
				$this->flex( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->align( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->pack( array_shift($args) );
			
			parent::__construct( $args );
		}
		else parent::__construct();
	}
}

class XULHBox extends XULBox
{
	public $remoteConstructor = 'HBox';
}

class XULVBox extends XULBox
{
	public $remoteConstructor = 'VBox';
}

class XULBBox extends XULBox
{
	public $remoteConstructor = 'BBox';
}

class XULScrollBox extends XULBox
{
	public $remoteConstructor = 'ScrollBox';
	
	public function scrollTo( $x, $y ) {
		if( ! $this->isRegistered )
			Error::CallerFatal( 'Call to scrollTo() on an unregistered XULScrollBox' );
		$this->callMethod( 'scrollTo', array( $x, $y ) );
	}
	
	public function ensureElementIsVisible( $node ) {
		if( ! $this->isRegistered )
			Error::CallerFatal( 'Call to ensureElementIsVisible() on an unregistered XULScrollBox' );
		$this->callMethod( 'ensureElementIsVisible', array( $node ) );
	}
}

class XULHTMLBox extends Node
{
	public $remoteConstructor = 'HTMLBox';
	public $initAttributes = array( 'content' );
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_string($args[0]) )
				$this->content( array_shift($args) );
			if( count($args) && is_int($args[0]) )
				$this->flex( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->align( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->pack( array_shift($args) );
			
			parent::__construct( $args );
		}
		else parent::__construct();
	}
	
	public function content( $html = NULL ) {
		if( $html === NULL )
			return $this->content;
		
		$this->content = $html;
		
		if( $this->isRegistered )
			$this->callMethod( 'setContent', array($html) );
	}
}


class XULStack extends BranchNode
{
	public $remoteConstructor = 'Stack';
	
	
	/**
	 * Dynamic Constructor
	 *
	 * @param int		$flex
	 * @param string	$align
	 * @param string	$pack
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_int($args[0]) )
				$this->flex( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->align( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->pack( array_shift($args) );
			
			parent::__construct( $args );
		}
		else parent::__construct();
	}
}

class XULDeck extends BranchNode
{
	public $remoteConstructor = 'Deck';
	private $selectedPanel = NULL;
	
	
	/**
	 * Dynamic Constructor
	 *
	 * @param int		$flex
	 * @param string	$align
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_int($args[0]) )
				$this->flex( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->align( array_shift($args) );
			
			parent::__construct( $args );
		}
		else parent::__construct();
	}
	
	public function selectedPanel( $panel = NULL ) {
		if( $panel === NULL )
			return $this->selectedPanel;
		
		if( $this->isRegistered )
			$this->callMethod( 'selectedPanel', array($panel) );
		
		$this->selectedPanel = $panel;
	}
}


class XULGrid extends BranchNode
{
	public $remoteConstructor = 'Grid';


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

class XULColumns extends BranchNode
{
	public $remoteConstructor = 'Columns';
}

class XULRows extends BranchNode
{
	public $remoteConstructor = 'Rows';
}

class XULColumn extends BranchNode
{
	public $remoteConstructor = 'Column';

	
	/**
	 * Dynamic Constructor
	 *
	 * @param int		$flex
	 * @param string	$align
	 * @param string	$pack
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_int($args[0]) )
				$this->flex( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->align( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->pack( array_shift($args) );
			
			parent::__construct( $args );
		}
		else parent::__construct();
	}
}

class XULRow extends BranchNode
{
	public $remoteConstructor = 'Row';

	
	/**
	 * Dynamic Constructor
	 *
	 * @param int		$flex
	 * @param string	$align
	 * @param string	$pack
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_int($args[0]) )
				$this->flex( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->align( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->pack( array_shift($args) );
			
			parent::__construct( $args );
		}
		else parent::__construct();
	}
}
