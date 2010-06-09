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
 * @subpackage Editor
*/

class XULEditor extends BranchNode
{
	public $remoteConstructor = 'Editor';
	public $resultHTML = '';
	protected $editable = FALSE;
	protected $value = "";
	public $disabled = FALSE;
	
	/**
	 * Dynamic Constructor
	 *
	 * @param string	$html
	 * @param int		$flex
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_string($args[0]) )
				$this->value = array_shift($args);
			if( count($args) && is_int($args[0]) )
				$this->flex( array_shift($args) );
			
			parent::__construct( $args );
		}
		else
			parent::__construct();

		$this->setEventHandler( 'yield', $this, 'yieldCollect' );
		$this->setEventHandler( 'onAfterAttach', $this, 'init' );
		$this->setEventHandler( 'initialized', $this, 'afterInit' );
		$this->setEventHandler( 'madeEditable', $this, 'afterMadeEditable' );
	}
	
	public function init() {
		$this->callMethod( 'init' );
	}
	
	public function afterMadeEditable()
	{
		$this->callMethod( 'editorLoaded');
		$this->editable = TRUE;
		
		$this->callMethod( 'setValue', $this->value);
		if($this->disabled)
			$this->callMethod( 'disableInput', $this->disabled );

	}
	
	public function afterInit() 
	{
		$this->callMethod( 'makeEditable');
	}
	
	public function value($value = NULL)
	{
		if ($value !== NULL)
		{
			$this->value = $value;
		}

		if( $this->editable )
		 $this->callMethod( 'setValue', $this->value );
	
		return $this->value;
	}
	
	public function disabled($disabled = NULL)
	{
		if ($disabled !== NULL)
		{
			$this->disabled = (bool)$disabled;
			
			$this->setProperty( 'disabled', ($this->disabled ? true : false));
			if( $this->editable )
				 $this->callMethod( 'disableInput', $this->disabled );
		}
		
		return $this->disabled;
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

	public function yieldCollect( $e, $val = NULL ) {
		$this->value = $val;
	}
}

class XULLayoutEditor extends XULEditor
{
	public $remoteConstructor = 'LayoutEditor';
	public $resultHTML = '';
	
	public function afterInit() 
	{

	}
	
	public function afterMadeEditable() {
		$this->callMethod( 'editorLoaded');
		$this->editable = TRUE;
	}
	
	public function loadLayoutHTML( $html, $frameid ) {
		$this->callMethod( 'loadLayoutHTML', array($html,$frameid) );
	}

	public function loadBody( $html ) {
		$this->callMethod( 'loadBody', array( $html == '' ? '<br/>':$html ) );
	}
	
	public function yieldCollect( $e, $val = NULL ) {
		$this->resultHTML = $val;
	}
}
