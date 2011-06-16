<?php

class XULFlash extends XULEmbed
{
	private $flashvars = array();
	
	private function rebuildFlashVars() {
		
		$flasvars = array();
		foreach	($this->flashvars as $key => $value) {
				$flasvars[] = "$key=".$value."";
		}
		$this->setAttribute("flashvars", implode("&",$flasvars));
	}
	
	public function addVariable($name, $value) {
		
		if (preg_match('/[A-Za-z_][A-Za-z0-9_]*$/', $value)) {
			$this->flashvars[$name] = $value;
			$this->rebuildFlashVars();
			return TRUE;
		}
		else return FALSE;
	}
	
	public function addParam($param, $value) {
		$this->setAttribute($param, $value);
	}
}

class XULFlashProxy extends XULBrowser
{
	public $ready = FALSE;
	public $written = FALSE;
	public $version = 9;
	
	private $flashvars = array();
	private $params = array();
	
	public $remoteConstructor = 'FlashProxy';
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();

			if( count($args) && is_string($args[0]) )
				$this->swf( array_shift($args) );
			if( count($args))
				$this->width( array_shift($args) );
			if( count($args))
				$this->height( array_shift($args) );
				
			parent::__construct( $args );
		}
		else parent::__construct();
		
		$this->setEventHandler( 'ready', $this, 'ready' );
		$this->setEventHandler( 'flashReady', $this, 'flashReady' );
		$this->setEventHandler( 'onAfterAttach', $this, 'init' );
	}
	
	public function init() {
		$this->flex(1);

		$protocol = ((ApplicationProcess::$ServerSecure) ? "https://" : "http://");
		$serverHost = ApplicationProcess::$ServerHost;
		$serverPath = ApplicationProcess::$ServerPath;
		$this->setMethod("loadFlashContent", array("content", "width", "height", "version"), "
    	    ".$this->js.".so = new ".$this->js.".element.contentWindow.SWFObject(content,'flashContent',width,height,version, 'false',{},{},{}, function(e) {window.alert('test');});
    	");
    	$this->callMethod( 'setSrc', $protocol.$serverHost.$serverPath."/misc/flashproxy/flashproxy.html#".$this->id );
	}
	
	public function flashReady($e) {
		$this->rootWindow->alert('klaar!');
	}
	
	public function ready($e) {
		$this->ready = TRUE;
		$this->callMethod( 'loadFlashContent', array($this->swf, $this->width, $this->height, $this->version) );
		if ($this->written)
			$this->write();
	}
	
	public function addVariable($name, $value) {
		
		if (preg_match('/[A-Za-z_][A-Za-z0-9_]*$/', $name)) {
			$this->flashvars[$name] = $value;
			return TRUE;
		}
		else return FALSE;
	}
	
	public function addParam($param, $value) {

		$this->params[$param] = $value;
	}
	
	public function write() {
		$this->written = TRUE;
		if ($this->ready) {
			foreach ($this->params as $param => $value) {
				$this->callMethod( "so.addParam", array($param, $value));	
			}
			
			foreach ($this->flashvars as $name => $value) {
				$this->callMethod( "so.addVariable", array($name, $value));

			}
			$this->callMethod( "so.write", array('flash'));
			
		}
	}
	
	public function swf($swf = NULL) {
		if ($swf !== NULL) {
			$this->swf = $swf;
			if ($this->ready)
				$this->callMethod( 'loadFlashContent', array($this->swf, $this->width, $this->height, 9) );
		}
		return $this->swf;
	}
	
	public function version($version = NULL) {
		if ($swf !== NULL) {
			$this->version = $version;
		}
		return $version;
	}
	
}