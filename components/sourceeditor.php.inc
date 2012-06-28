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
 * @subpackage SourceEditor
*/


/**
 * This class controls the client component SourceEditor
*/

class XULSourceEditor extends BranchNode
{
	public $language = 'html';
	public $selection = "";
	public $flex = 0;
	
	public $searchResults;
	
	public $selectionStart = 0;
	public $selectionEnd = 0;
	public $charCount = 0;
	public $lineCount = 0;
	public $mode = '';
		
	public $ready = FALSE;
	public $remoteConstructor = 'SourceEditor';
	
	public $value = "";
	
	private $setValueWhenReady = FALSE;
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_string($args[0]) )
				$this->value(array_shift($args));
			if( count($args) && is_string($args[0]) )
				$this->language = array_shift($args);
			if( count($args) && is_int($args[0]) )
				$this->flex( array_shift($args) );
				
		}
		else
			parent::__construct();
		
		
		$this->searchResults = new stdClass;
		
		$this->flex($this->flex);
		
		$this->setEventHandler( 'onAfterAttach', $this, 'init' );
		$this->setEventHandler( 'yield', $this, 'yieldCollect' );
		$this->setEventHandler( 'yieldSearch', $this, 'yieldSearchCollect' );
		$this->setEventHandler( 'ready', $this, 'ready' );
	}
	
	public function value($value = NULL) {
		if ($value) {
			$this->value = $value;
			
			if ($this->ready) {
				$this->callMethod("setText",$this->value);
			}
			else $this->setValueWhenReady = TRUE;
		}
		return $this->value;
	}
	
	public function init() {
		$this->callMethod("initSourceEditor",array($this->value, $this->language));
	}
	
	public function ready() {
		$this->ready = TRUE;
		if ($this->setValueWhenReady) {
			$this->callMethod("setText",$this->value);
			$this->setValueWhenReady = FALSE;
		}
	}
	public function find($val,$caseSensitive = FALSE, $reverseOrder = FALSE, $start = 0) {
		
		$this->callMethod("find",array($val,$caseSensitive,$reverseOrder,$start));
	}
	
	public function findNext($wrap=TRUE) {
		
		$this->callMethod("findNext",array($wrap));
	}
	
	public function findPrevious($wrap=TRUE) {
		
		$this->callMethod("findPrevious",array($wrap));
	}
	
	public function setText($text, $start = 0, $end = NULL) {
		
		if ($start && $end) {
			$this->callMethod("editor.setText",array($text, $start, $end));
		}
		else {
			$this->callMethod("editor.setText",array($text));
		}
	}
	
	public function dropSelection() {
		
		$this->callMethod("editor.dropSelection");
	}
	
	public function startCompoundChange() {
		
		$this->callMethod("editor.startCompoundChange");
	}
	
	public function endCompoundChange() {
		
		$this->callMethod("editor.endCompoundChange");
	}
	
	public function setSelection($start,$end) {
		if ($this->ready)
			$this->callMethod("editor.setSelection",array($start,$end));
	}
	
	public function setCaretOffset($offset) {
		if ($this->ready)
			$this->callMethod("editor.setCaretOffset",array($offset));
	}
	
	
	public function mode($mode = NULL) {
		if ($mode && $this->isRegistered) {
			$this->mode = $mode;
			$this->callMethod('setMode', array($mode));
		}
		
		return $mode;
	}
	
	public function yieldCollect( $e, $val = NULL, $selection = NULL, $selectionStart = NULL, $selectionEnd = NULL, $charCount = NULL, $lineCount = NULL, $mode = NULL ) {

		$this->value = $val;
		$this->selection = $selection;
		$this->selectionStart = $selectionStart;
		$this->selectionEnd = $selectionEnd;
		$this->charCount = $charCount;
		$this->lineCount = $lineCount;
		$this->mode = $mode;
	}
	
	public function yieldSearchCollect( $e, $searchOffset = NULL, $str = NULL, $index = NULL, $lastFound = NULL, $ignoreCase = NULL) {
		$this->searchResults->searchOffset = $searchOffset;
		$numargs = func_num_args();
		
		if ($numargs > 2) {
			$this->searchResults->str = $str;
			$this->searchResults->index = $index;
			$this->searchResults->lastFound = $lastFound;
			$this->searchResults->ignoreCase = $ignoreCase;
		}
	}
}