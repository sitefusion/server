<?php

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