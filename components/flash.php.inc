<?php

/**
 * XUL Flash element
 * 
 * Use this element to directly include a flash movie in your SiteFusion application.
*/

class XULFlash extends XULHTMLEmbed
{
	private $flashvars = array();
	
	private function rebuildFlashVars() {
		
		$flasvars = array();
		foreach	($this->flashvars as $key => $value) {
				$flasvars[] = "$key=".$value."";
		}
		$this->setAttribute("flashvars", implode("&",$flasvars));
	}
	
		
	/**
	 * Add a Flash variable
	 *
	 * Please note that flashvars can only be set before the initialisation of an SWF 
	 *
	 * @param string	$name	Name of the variable
	 * @param string	$value	Contents of the variable
	*/
	
	public function addVariable($name, $value) {
		
		if (preg_match('/[A-Za-z_][A-Za-z0-9_]*$/', $value)) {
			$this->flashvars[$name] = $value;
			$this->rebuildFlashVars();
			return TRUE;
		}
		else return FALSE;
	}
	
	/**
	 * Add a paramater
	 *
	 * @param string	$param	Name of the parameter
	 * @param string	$value	Contents of the variable
	*/
	
	public function addParam($param, $value) {
		$this->setAttribute($param, $value);
	}
}

/**
 * XUL Flash element
 * 
 * Uses a proxy in your webfrontend in order to display flash content.
 * This proxy bypasses any security messages from flash, as long as the actual SWF is located on the same domain as your SF webfrontend.
*/


class XULFlashProxy extends XULBrowser
{
	public $ready = FALSE;
	public $written = FALSE;
	public $version = 9;
	
	private $flashvars = array();
	private $params = array();
	
	public $remoteConstructor = 'FlashProxy';
	
	/**
	 * Dynamic Constructor
	 *
	 * @param string	$swf
	 * @param int	$width
	 * @param int	$height
	*/
	
	
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
		$this->setEventHandler( 'onAfterAttach', $this, 'init' );
	}
	
	public function init() {
		$this->flex(1);

		$protocol = ((ApplicationProcess::$ServerSecure) ? "https://" : "http://");
		$serverHost = ApplicationProcess::$ServerHost;
		$serverPath = ApplicationProcess::$ServerPath;
		$this->setMethod("loadFlashContent", array("content", "width", "height", "version"), "
    	    ".$this->js.".so = new ".$this->js.".element.contentWindow.SWFObject(content,'flashContent',width,height,version, 'false',{});
    	");
    	$this->callMethod( 'setSrc', $protocol.$serverHost.$serverPath."/misc/flashproxy/flashproxy.html#".$this->id );
	}
	
	/**
	 * [INTERNAL FUNCTION]
	 * This function is called by the content in the browser element when it's ready to accept flash content.
	 * It fires a JS dynamicly generated JS function in order to do so.
	*/
	
	public function ready($e) {
		$this->ready = TRUE;
		$this->callMethod( 'loadFlashContent', array($this->swf, $this->width, $this->height, $this->version) );
		if ($this->written)
			$this->write();
	}
	
	/**
	 * Add a Flash variable
	 *
	 * @param string	$name	Name of the variable
	 * @param string	$value	Contents of the variable
	*/
	
	public function addVariable($name, $value) {
		
		if (preg_match('/[A-Za-z_][A-Za-z0-9_]*$/', $name)) {
			$this->flashvars[$name] = $value;
			return TRUE;
		}
		else return FALSE;
	}
	
	/**
	 * Add a paramater
	 *
	 * @param string	$param	Name of the parameter
	 * @param string	$value	Contents of the variable
	*/
	
	public function addParam($param, $value) {

		$this->params[$param] = $value;
	}
	
	/**
	 * Write the actual flash content
	 *
	*/
	
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
	
	/**
	 * Set swf that should be loaded
	 *
	 * @param string	$swf
	*/
	
	public function swf($swf = NULL) {
		if ($swf !== NULL) {
			$this->swf = $swf;
			if ($this->ready)
				$this->callMethod( 'loadFlashContent', array($this->swf, $this->width, $this->height, 9) );
		}
		return $this->swf;
	}
	
	/**
	 * Set the required flash version
	 *
	 * @param int	$version
	*/
	
	public function version($version = NULL) {
		if ($swf !== NULL) {
			$this->version = $version;
		}
		return $version;
	}
	
}