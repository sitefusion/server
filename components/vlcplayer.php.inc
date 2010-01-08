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
 * @subpackage VLCPlayer
*/


/**
 * This class controls the client component VLCPlayer
*/


class XULVlcPlayer extends Node
{
	public $remoteConstructor = 'VlcPlayer';
	private $state = 0;
	
	
		public function __construct( $url = NULL ) {
		$this->url = $url;
		$this->setEventHandler("stateChange", $this, "onStateChange");
		parent::__construct();
	}
	
	public function attach() {
		$this->createRemoteObject( array( $this->hostWindow, $this->url ) );
		$this->insertElement();
		$this->callMethodDelayed("afterinit");
	}
	
	public function width( $width = NULL ) {
		return $this->attributeMethod( 'width', $width );
	}
	
	public function height( $height = NULL ) {
		return $this->attributeMethod( 'height', $height );
	}
	
	public function add($src)
	{
		$this->items[] = $src;
		$this->callMethodDelayed('add', array($src));
	}
	
	public function sizeToContent()
	{
		//if either playing or paused
		if ($this->state == 3 || $this->state == 4)
			$this->callMethodDelayed('sizeToContent', NULL, 100);
	}
	
	public function play()
	{
		$this->callMethod('play');
	}
	
	public function togglePause()
	{
		$this->callMethod('togglePause');
	}
	
	public function stop()
	{
		$this->callMethod('stop');
	}
	
	public function onStateChange($e, $state)
	{
		/*
		IDLE=0, OPENING=1, BUFFERING=2, PLAYING=3, PAUSED=4, STOPPING=5, ENDED=6, ERROR=7
		*/
		
		if ($state == 6) {
			$this->stop();
			return;
		}
		
		$this->state = $state;
		
		if ($this->state == 3) {
			$this->sizeToContent();
		}	
		
	}
	
}