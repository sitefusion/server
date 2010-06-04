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
//   Tom Peeters <tom@thefrontdoor.nl> (original author)
//
// - - - - - - - - - - - - - - END LICENSE BLOCK - - - - - - - - - - - - -


/**
 * @package ClientComponents
 * @subpackage CodeMirror
*/


/**
 * This class controls the client component CodeMirror
*/

class XULCodeMirror extends XULBrowser
{
	public $language = '';
	public $ready = FALSE;
	public $remoteConstructor = 'CodeMirror';
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_string($args[0]) )
				$this->value = array_shift($args);
			if( count($args) && is_string($args[0]) )
				$this->language = array_shift($args);
			if( count($args) && is_int($args[0]) )
				$this->flex( array_shift($args) );
			
			parent::__construct( $args );
		}
		else
			parent::__construct();

		$this->setEventHandler( 'yield', $this, 'yieldCollect' );
		$this->setEventHandler( 'ready', $this, 'ready' );
		$this->setEventHandler( 'onAfterAttach', $this, 'init' );
	}
	
	public function value($value = NULL) {
		if ($value !== NULL) {
			$this->value = $value;
			if ($this->ready)
				$this->callMethod( 'element.contentWindow.editor.setCode', array($this->value) );
		}
		return $this->value;
	}
	
	public function init() {
		$this->callMethod( 'element.setAttribute', array("source", $this->value) );
		$this->src("chrome://codemirror/content/".$this->language.".html#".$this->id);
	}
	
	public function yieldCollect( $e, $val = NULL ) {
		$this->value = $val;
	}
	
	public function search() {
		if ($this->isRegistered)
			$this->callMethod('search');	
	}
	
	public function ready($e) {
		$this->ready = TRUE;
	}
}