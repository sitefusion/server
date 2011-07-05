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
 * @subpackage Scripting
*/



/**
 * Keyset element
 * 
 * @link https://developer.mozilla.org/en/XUL/keyset
*/

class XULKeySet extends BranchNode
{
	public $remoteConstructor = 'KeySet';
	
	public function attach() {
		parent::attach();
		
		$tb = $this->findAncestor("XULTabBox");
		if ($tb && $tb->children && count($tb->children))
		{
			foreach ($tb->children as $child)
			{
				if ($child instanceOf XULTabs)
					$child->onSwitchTab();
			}
		}
	}
	
}



/**
 * Key element
 * 
 * @link https://developer.mozilla.org/en/XUL/key
*/

class XULKey extends Node
{
	public $remoteConstructor = 'Key';
	public $initAttributes = array( 'modifiers', 'key', 'keycode' );
	
	public function __construct( $key = NULL, $modifiers = NULL, $evtObj = NULL, $evtMethod = NULL, $evtYielders = NULL ) {
		if( $key !== NULL ) {
			if( substr($key,0,3) == 'VK_' )
				$this->keycode( $key );
			else
				$this->key( $key );
		}
		if( $modifiers !== NULL )
			$this->modifiers( $modifiers );
		
		$this->evtObj = $evtObj;
		$this->evtMethod = $evtMethod;
		$this->evtYielders = $evtYielders;
		
		$this->setEventHandler( 'onAfterAttach', $this, 'init' );
		
		parent::__construct();
	}
	
	public function init() {
		if( $this->evtObj && $this->evtMethod )
			$this->setEvent( 'command', MSG_SEND, $this->evtObj, $this->evtMethod, $this->evtYielders );
	}
	
	public function key( $key = NULL ) {
		return $this->attributeMethod( 'key', $key );
	}

	public function keycode( $keycode = NULL ) {
		return $this->attributeMethod( 'keycode', $keycode );
	}
	
	public function modifiers( $modifiers = NULL ) {
		return $this->attributeMethod( 'modifiers', $modifiers );
	}
}


/**
 * Trigger, custom element
*/

class Trigger extends Node
{
	public $iterations;
	public $delay;
	public $remoteConstructor = 'Trigger';
	
	public function __construct( $iter, $delay ) {
		$this->iterations = (int) $iter;
		$this->delay = (int) $delay;
	}
	
	public function attach() {
		$this->rootApplication->sendCommand(
			Comm::CreateRemoteObject( $this, array($this->iterations,$this->delay) )
		);
		$this->setEventType( 'yield', MSG_SEND );
	}
	
	public function detach() {
		$this->rootApplication->sendCommand(
			Comm::CallMethod( $this, 'cancel' ),
			Comm::UnRegister( $this )
		);
	}
}

/**
 * PrefService, a SiteFusion wrapper for XPCOM client-side prefs
*/

class PrefService extends Node
{

	public $remoteConstructor = 'PrefService';
	private $prefTypes = array();
	private $prefValues = array();
	
	public function attach() {
		parent::attach();
		$this->setEvent( 'yield', MSG_SEND, $this, 'yieldCollect' );
	}
	
	private function arrayToObject($array = array()) {
	    if (!empty($array)) {
	        $data = new stdClass;
	
	        foreach ($array as $akey => $aval) {
	            $data -> {$akey} = $aval;
	        }
	
	        return $data;
	    }
	
	    return false;
	}
	
	public function setPrefRetriever($pref, $type) {
		$this->prefValues[strval($pref)] = FALSE;
		$this->prefTypes[strval($pref)] = $type;
		
		$this->setProperty("prefTypes", $this->arrayToObject($this->prefTypes));
	}
	
	/**
	 * [INTERNAL FUNCTION] Collects the results of a prompt
	*/
	
	public function yieldCollect( $e, $prefValues ) {
		foreach (get_object_vars($prefValues) as $key => $value) {
			$this->prefValues[$key] = $value;
		}
	}
	
	public function setIntPref( $pref, $value ) {
		$this->prefTypes[$pref] = "int";
		$this->prefValues[$pref] = $value;
		$this->callMethod( 'setIntPref', array($pref,$value) );
	}
	
	public function getIntPref( $pref) {
		if (isset($this->prefValues[$pref]) && $this->prefTypes[$pref] == "int") {
			return $this->prefValues[$pref];
		}
		else return FALSE;
	}
	
	public function setCharPref( $pref, $value ) {
		$this->prefTypes[$pref] = "string";
		$this->prefValues[$pref] = $value;
		$this->callMethod( 'setCharPref', array($pref,$value) );
	}
	
	public function getCharPref( $pref) {
		if (isset($this->prefValues[$pref]) && $this->prefTypes[$pref] == "string") {
			return $this->prefValues[$pref];
		}
		else return FALSE;
	}
}