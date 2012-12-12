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
	public $theme = 'default';
	public $flex = 1;
	public $ready = FALSE;
	public $remoteConstructor = 'CodeMirror';
	
	public $value = "";
	public $selection = "";
	public $themes = FALSE;
	
	private $setValueWhenReady = FALSE;
	private $constructOptions = array();
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_string($args[0]) )
				$this->value(array_shift($args));
			if( count($args) && is_string($args[0]) )
				$this->language = array_shift($args);
			if( count($args) && is_int($args[0]) )
				$this->flex( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->theme = array_shift($args);
				
			parent::__construct( $args );
		}
		else
			parent::__construct();
			
		if (isset(ApplicationProcess::$ExtensionInfo['codemirror@sitefusion.org']) && ApplicationProcess::$ExtensionInfo['codemirror@sitefusion.org']['enabled']) {
			switch (substr(ApplicationProcess::$ExtensionInfo['codemirror@sitefusion.org']['version'],0,1)) {
				case 1:	
				$this->setEventHandler( 'yield', $this, 'yieldCollect' );
				break;
				case 2:
				case 3:
				if ($this->language == "html") {
					//fix for html mode
					$this->language = "htmlmixed";
				}
				if ($this->language == "sql") {
					//fix for sql mode
					$this->language = "psql";
				}	
				$this->setEventHandler( 'yield', $this, 'yieldCollectv2' );
				break;
			}
		}
		$this->flex($this->flex);
		
		$this->setEventHandler( 'yieldThemes', $this, 'yieldCollectThemes' );
		$this->setEventHandler( 'ready', $this, 'ready' );
		$this->setEventHandler( 'onAfterAttach', $this, 'init' );
	}
	
	public function value($value = NULL) {
		if ($value !== NULL) {
			$this->value = $value;
			if ($this->ready) {
				if (isset(ApplicationProcess::$ExtensionInfo['codemirror@sitefusion.org']) && ApplicationProcess::$ExtensionInfo['codemirror@sitefusion.org']['enabled']) {
					switch (substr(ApplicationProcess::$ExtensionInfo['codemirror@sitefusion.org']['version'],0,1)) {
					case 1:
					$this->callMethod( 'element.contentWindow.editor.setCode', array($this->value) );
					break;
					case 2:
					case 3:
					$this->callMethod( 'element.contentWindow.editor.setValue', array($this->value) );
					break;
					}
				}
				
			}
			else {
				//codemirror not yet ready when value function is called (or value set through constructor)
				$this->setValueWhenReady = TRUE;	
			}
		}
		return $this->value;
	}
	
	public function setOption($option, $value) {
		if ($this->isRegistered && $this->ready) {
			$this->callMethod('element.contentWindow.editor.setOption', array($option, $value));
		}
		else {
			$this->constructOptions[$option] = $value;	
		}
	}
	
	public function init() {
		if (isset(ApplicationProcess::$ExtensionInfo['codemirror@sitefusion.org']) && ApplicationProcess::$ExtensionInfo['codemirror@sitefusion.org']['enabled']) {
			switch (substr(ApplicationProcess::$ExtensionInfo['codemirror@sitefusion.org']['version'],0,1)) {
				case 1:
				$this->setMethod("yield",array(), $this->js.". v1.yield();");
				$this->src("chrome://codemirror/content/".$this->language.".html#".$this->id);
				break;
				case 2:
				case 3:
				$this->setMethod("yield",array(), $this->js.".v2.yield();");
				$this->src("chrome://codemirror/content/editor.html#".$this->id);
				$this->callMethod( 'element.setAttribute', array("theme", $this->theme) );
				$this->callMethod( 'element.setAttribute', array("mode", $this->language) );
				break;
			}
		}
	}
	
	public function yieldCollect( $e, $val = NULL ) {
		$this->value = $val;
	}
	
	public function yieldCollectv2( $e, $val = NULL, $selection = NULL ) {
		$this->value = $val;
		$this->selection = $selection;
	}
	
	public function yieldCollectThemes ($e) {
		$numargs = func_num_args();
		$this->themes = array();
		if ($numargs > 1) { 
			$arg_list = func_get_args();
	    	for ($i = 1; $i < $numargs; $i++) {
				$this->themes[] = $arg_list[$i];
    		}
			$this->fireClientEvent("themesLoaded");
		}
	}
	
	public function search($text = NULL, $caseSensitive = FALSE, $reverseOrder = FALSE) {
		if ($this->isRegistered && isset(ApplicationProcess::$ExtensionInfo['codemirror@sitefusion.org']) && ApplicationProcess::$ExtensionInfo['codemirror@sitefusion.org']['enabled']) {
			switch (substr(ApplicationProcess::$ExtensionInfo['codemirror@sitefusion.org']['version'],0,1)) {
				case 1:
				$this->callMethod('v1.search');
				break;	
				case 2:
				case 3:
				$this->callMethod('v2.search', array($text, $caseSensitive, $reverseOrder));
				break;
			}
			
		}	
	}
	
	public function replace($text, $replace, $caseSensitive = FALSE, $reverseOrder = FALSE) {
		$this->callMethod('v2.replace', array($text, $replace, $caseSensitive, $reverseOrder));
	}
	
	public function replaceAll($text, $replace, $caseSensitive = FALSE) {
		$this->callMethod('v2.replaceAll', array($text, $replace, $caseSensitive));
	}
	
	public function ready($e) {
		$this->ready = TRUE;
		
		if ($this->setValueWhenReady && $this->value) {
			$this->callMethod( 'element.contentWindow.editor.setValue', array($this->value) );
		}
		
		if (count($this->constructOptions)) {
			foreach($this->constructOptions as $option => $value) {
				$this->setOption($option, $value);
			}
		}
		$this->callMethod( "v2.getAvailableThemes");
	}
}