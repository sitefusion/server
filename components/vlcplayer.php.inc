<?php
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
		$this->callMethodDelayed('addSrc', array($src));
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