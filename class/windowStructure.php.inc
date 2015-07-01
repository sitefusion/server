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
 * @subpackage WindowStructure
*/


/**
 * Browser element
 *
 * @link https://developer.mozilla.org/en/XUL/browser
 * @package API
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


/**
 * Printbox element
 *
 * Call the content($html) method to fill the printbox with HTML content, and call
 * the printContent() method to open the printer dialog. Optionally set the $silent
 * parameter to TRUE to print directly without opening the print dialog first.
 * @package API
 * @subpackage WindowStructure
*/

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


/**
 * Statusbar element
 *
 * @link https://developer.mozilla.org/en/XUL/statusbar
 * @package API
 * @subpackage WindowStructure
*/

class XULStatusbar extends BranchNode
{
	public $remoteConstructor = 'Statusbar';
	public $orient = 'horizontal';
}


/**
 * StatusbarPanel element
 *
 * @link https://developer.mozilla.org/en/XUL/statusbarpanel
 * @package API
 * @subpackage WindowStructure
*/

class XULStatusbarPanel extends BranchNode
{
	public $remoteConstructor = 'StatusbarPanel';
	public $initAttributes = array( 'image' );


	/**
	 * Dynamic Constructor
	 *
	 * @param string	$label
	 * @param string	$image
	 * @param mixed		$childNodes
	*/

	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();

			if( count($args) && is_string($args[0]) )
				$this->label( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->image( array_shift($args) );

			parent::__construct( $args );
		}
		else parent::__construct();
	}

	public function label( $text = NULL ) {
		$ret = parent::label( $text );
		$this->setClass();
		return $ret;
	}

	private function setClass() {
		if( (isset($this->className) && $this->className == 'statusbarpanel-menu-iconic') || !isset($this->image) )
			return;

		if( isset($this->label) && $this->label != '' )
			$this->className( 'statusbarpanel-iconic-text' );
		else
			$this->className( 'statusbarpanel-iconic' );
	}

	public function image( $src = NULL ) {
		if( $src === NULL )
			return (isset($this->image) ? $this->image : NULL);

		$this->image = $src;

		if( $this->isRegistered ) {
			$this->callMethod( 'image', $this->image );
			$this->setClass();
		}

		return $this;
	}
}


/**
 * Dialog header element
 *
 * @link https://developer.mozilla.org/en/XUL/dialogheader
 * @package API
 * @subpackage WindowStructure
*/

class XULDialogHeader extends BranchNode
{
	public $remoteConstructor = 'DialogHeader';
	public $initAttributes = array( 'title', 'description' );


	/**
	 * Dynamic Constructor
	 *
	 * @param string	$title
	 * @param string	$description
	 * @param mixed		$childNodes
	*/

	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();

			if( count($args) && is_string($args[0]) )
				$this->title( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->description( array_shift($args) );

			parent::__construct( $args );
		}
		else parent::__construct();
	}

	public function title( $title = NULL ) {
		return $this->attributeMethod( 'title', $title );
	}

	public function description( $descr = NULL ) {
		return $this->attributeMethod( 'description', $descr );
	}
}
