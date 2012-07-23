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


class XULVlcPlayer extends XULHTMLEmbed
{
	public $remoteConstructor = 'VlcPlayer';
	private $state = 0;
	public $width = 0;
	public $height = 0;
	
	public function __construct($width = NULL, $height = NULL, $src = NULL, $target = NULL, $mrl = NULL, $filename = NULL, $autoplay = NULL, $allowfullscreen = NULL, $mute = NULL, $loop = NULL, $toolbar = NULL, $bgcolor = NULL ) {
		$this->setEventHandler("stateChange", $this, "onStateChange");
		
		//for a file (default)
		$this->src = $src;
			
		//this is for streams
		$this->target = $target;
		$this->mrl = $mrl;
		$this->filename = $filename;
		
		$this->autoplay = $autoplay;
		$this->allowfullscreen = $allowfullscreen;
		
		$this->mute = $mute;
		$this->loop = $loop;
		$this->toolbar = $toolbar;
		$this->bgcolor = $bgcolor;
		
		$this->bgcolor = $bgcolor;
		
		//we can use these default Node methods
		if( $width !== NULL )
			$this->width( $width );
		if( $height !== NULL )
			$this->height( $height );
	}

	public function attach() {
		//order of client side arguments: win, width, height, src, target, mrl, filename, autoplay, allowfullscreen, mute, loop, toolbar, bgcolor
		$this->autoplay = TRUE;
		
		$this->createRemoteObject( array( $this->hostWindow, $this->width, $this->height, $this->src, $this->target, $this->mrl, $this->filename, ($this->autoplay === NULL ? TRUE : $this->autoplay), ($this->allowfullscreen === NULL ? TRUE : $this->allowfullscreen), ($this->mute === NULL ? FALSE : $this->mute), ($this->loop === NULL ? FALSE : $this->loop), ($this->toolbar === NULL ? TRUE : $this->toolbar), $this->bgcolor ) );
		$this->insertElement();
		$this->callMethodDelayed("afterinit", array($this->width, $this->height ));
	}
	
	public function width( $width = NULL ) {
		$this->width = $width;
		if ($this->isRegistered)
			$this->setAttribute( 'width', $width );
	}
	
	public function height( $height = NULL ) {
		$this->height = $height;
		if ($this->isRegistered)
			$this->setAttribute( 'height', $height );
	}
	
	public function add($src, $fancyname = NULL, $options = NULL)
	{
		$this->items[] = $src;
		if ($this->isRegistered)
			$this->callMethod('add', array($src,$fancyname,$options));
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
		
		
	}
	
}