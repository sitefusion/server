<?php

/**
 * @package SiteFusion
 * @subpackage Editor
*/


class XULLayoutEditor extends BranchNode
{
	public $remoteConstructor = 'LayoutEditor';
	public $resultHTML = '';
	
	
	/**
	 * Dynamic Constructor
	 *
	 * @param string	$editortype
	 * @param int		$flex
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_string($args[0]) )
				$this->editortype( array_shift($args) );
			if( count($args) && is_int($args[0]) )
				$this->flex( array_shift($args) );
			
			parent::__construct( $args );
		}
		else
			parent::__construct();

		$this->setEventHandler( 'yield', $this, 'yieldCollect' );
		$this->setEventHandler( 'onAfterAttach', $this, 'init' );
	}
	
	public function init() {
		$this->callMethod( 'init' );
	}
	
	public function loadLayoutHTML( $html, $frameid ) {
		$this->callMethod( 'loadLayoutHTML', array($html,$frameid) );
	}

	public function loadBody( $html ) {
		$this->callMethod( 'loadBody', array( $html == '' ? '<br/>':$html ) );
	}
	
	public function createLink( $href, $target = '', $class = '' ) {
		$this->callMethod( 'createLink', array( (string) $href, $target, $class ) );
	}
	
	public function insertImage( $src = NULL, $width = NULL, $height = NULL, $align = NULL, $alt = NULL ) {
		$this->callMethod( 'insertImage', array( $src, $width, $height, $align, $alt ) );
	}
	
	public function removeTableRow() {
		$this->callMethod( 'removeTableRow' );
	}

	public function insertTableRow() {
		$this->callMethod( 'insertTableRow' );
	}
	
	public function removeTable() {
		$this->callMethod( 'removeTable' );
	}

	public function insertHTML( $html ) {
		$this->callMethod( 'insertHTML', array( $html ) );
	}
	
	public function elementSetAttribute( $el, $attr, $value ) {
		$this->callMethod( 'elementSetAttribute', array($el,$attr,$value) );
	}
	
	public function elementSetStyle( $el, $style, $value ) {
		$this->callMethod( 'elementSetStyle', array($el,$style,$value) );
	}

	public function elementSetClassname( $el, $class ) {
		$this->callMethod( 'elementSetClassname', array($el,$class) );
	}
	
	public function setTextClass( $class ) {
		$this->callMethod( 'setTextClass', array($class) );
	}
	
	public function deleteElement( $tagName ) {
		$this->callMethod( 'removeElement', array($tagName) );
	}
	
	public function yieldCollect( $e, $val ) {
		$source = $val;
		$decodedStr = "";
		$pos = 0;
		$len = strlen ($source);
		while ($pos < $len) {
			$charAt = substr ($source, $pos, 1);
			if ($charAt == '%' && substr($source,$pos+1,1) == 'u') {
				$pos += 2;
				$unicodeHexVal = substr ($source, $pos, 4);
				$unicode = hexdec ($unicodeHexVal);
				$entity = "&#". $unicode . ';';
				$decodedStr .= utf8_encode ($entity);
				$pos += 4;
			}
			else {
				$decodedStr .= $charAt;
				$pos++;
			}
		}
		
		$val = $decodedStr;

		foreach ( WPEditorSymbolTable::$Symbols as $code ) {
			$val = str_replace( html_entity_decode($code,ENT_QUOTES,'ISO-8859-1'), $code, $val );
		}
		
		$val = str_replace( '&nbsp;', ' ', $val );	// FIX ME
		
		$this->resultHTML = $val;
	}
}
