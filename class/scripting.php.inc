<?php

/**
 * @package SiteFusion
 * @subpackage Scripting
*/


class XULKeySet extends BranchNode
{
	public $remoteConstructor = 'KeySet';
}


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
			$this->setEvent( 'command', sendMessage, $this->evtObj, $this->evtMethod, $this->evtYielders );
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
