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
 * @subpackage TabsAndGrouping
*/



/**
 * Tabbox element
 * 
 * @link https://developer.mozilla.org/en/XUL/tabbox
*/

class XULTabBox extends BranchNode
{
	public $remoteConstructor = 'TabBox';
	public $align = 'stretch';
	public $flex = 1;
	public $selectedIndex = NULL;
	protected $selectedTab = NULL;
	protected $selectedPanel = NULL;
	
	
	/**
	 * Dynamic Constructor
	 *
	 * @param int		$flex
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_int($args[0]) )
				$this->flex( array_shift($args) );
			
			parent::__construct( $args );
		}
		else parent::__construct();
	}
	
	public function attach() {
		parent::attach();
		$this->setEventHandler( 'yield', $this, 'yieldCollect' );
	}
	
	public function selectedTab( $tab = NULL ) {
		if( $tab === NULL )
			return $this->selectedTab;
		if( ! $tab instanceof XULTab and ! $tab instanceof XULTabPanel )
			throw new SFException( 'Object is not a XULTab or XULTabPanel', ERR_REPORT_APP );
		
		$this->selectedTab = $tab;
		
		if( $this->isRegistered )
			$this->callMethod( 'selectedTab', array($tab) );
	}

	public function selectedPanel( $tab = NULL ) {
		if( $tab === NULL )
			return $this->selectedPanel;
		if( ! $tab instanceof XULTabPanel )
			throw new SFException( 'Object is not a XULTabPanel', ERR_REPORT_APP );
		
		$this->selectedPanel = $tab;
		
		if( $this->isRegistered )
			$this->callMethod( 'selectedPanel', array($tab) );
	}
	
	public function yieldCollect( $e, $selectedIndex = NULL, $selectedTab = NULL, $selectedPanel = NULL) {
		$this->selectedIndex = $selectedIndex;
		$this->selectedTab = $selectedTab;
		$this->selectedPanel = $selectedPanel;
	}
}


/**
 * Tabs element
 * 
 * @link https://developer.mozilla.org/en/XUL/tabs
*/

class XULTabs extends BranchNode
{
	public $remoteConstructor = 'Tabs';
	protected $selectedItem = NULL;
	public $orient = 'horizontal';
	public $selectedIndex = 0;
		
	
	public function selectedItem( $tab = NULL ) {
		if( $tab === NULL )
			return $this->selectedItem;
		if( ! $tab instanceof XULTab )
			throw new SFException( 'Object is not a XULTab', ERR_REPORT_APP );
		
		$this->selectedItem = $tab;
		
		if( $this->isRegistered )
			$this->callMethod( 'selectedItem', array($tab) );
	}	
	
	public function advanceSelectedTab($dir = 1, $wrap = FALSE)
	{
		if( $this->isRegistered )
			$this->callMethod( 'advanceSelectedTab', array($dir, $wrap) );
	}
	
	public function attach() {
		parent::attach();
		$this->setEventHandler( 'yield', $this, 'yieldCollect' );
		$this->setEvent("select", MSG_SEND, $this, "onSwitchTab", array($this));
	}
	
	public function onSwitchTab()	{
		if(! ($tb = $this->findAncestor('XULTabBox')) )
			return;
		
		if(! count($tps = $tb->getDescendants('XULTabPanels')) )
			return;
		
		$tps = $tps[0];
		$panels = $tps->getDescendants( 'XULTabPanel' );
		
		for( $n = 0; $n < count($panels); $n++ ) {
			foreach ( $panels[$n]->getDescendants('XULKeySet') as $keyset ) {
				$keyset->disabled( $n != $this->selectedIndex );
			}
		}
	}
	
	public function yieldCollect( $e, $selectedItem = NULL, $selectedIndex = NULL) {
		$this->selectedItem = $selectedItem;
		$this->selectedIndex = $selectedIndex;
	}
}


/**
 * Tab element
 * 
 * @link https://developer.mozilla.org/en/XUL/tab
*/

class XULTab extends BranchNode
{
	public $remoteConstructor = 'Tab';
	public $initAttributes = array( 'image' );
	
	
	/**
	 * Dynamic Constructor
	 *
	 * @param string	$label
	 * @param string	$image
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_string($args[0]) )
				$this->label( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->image( array_shift($args) );
						
			parent::__construct( $args );
		}
		else parent::__construct();
	}
	
	public function attach() {
		parent::attach();

		if( $this->parent->parent->selectedTab() === NULL )
			$this->parent->parent->selectedTab( $this );
	}
	
	public function image( $src = NULL ) {
		if( $src === NULL )
			return (isset($this->image) ? $this->image : NULL);
		
		$this->image = $src;
		
		if( $this->isRegistered )
			$this->callMethod( 'image', $this->image );
		
		return $this;
	}
}


/**
 * Tabpanels element
 * 
 * @link https://developer.mozilla.org/en/XUL/tabpanels
*/

class XULTabPanels extends BranchNode
{
	public $remoteConstructor = 'TabPanels';
	public $flex = 1;
}


/**
 * Tabpanel element
 * 
 * @link https://developer.mozilla.org/en/XUL/tabpanel
*/

class XULTabPanel extends BranchNode
{
	public $remoteConstructor = 'TabPanel';
	public $flex = 1;
}


/**
 * Groupbox element
 * 
 * @link https://developer.mozilla.org/en/XUL/groupbox
*/

class XULGroupBox extends BranchNode
{
	public $remoteConstructor = 'GroupBox';

	
	/**
	 * Dynamic Constructor
	 *
	 * @param int		$flex
	 * @param string	$align
	 * @param string	$pack
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_int($args[0]) )
				$this->flex( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->align( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->pack( array_shift($args) );
			
			parent::__construct( $args );
		}
		else parent::__construct();
	}
}


/**
 * Caption element
 * 
 * @link https://developer.mozilla.org/en/XUL/caption
*/

class XULCaption extends BranchNode
{
	public $remoteConstructor = 'Caption';
	public $initAttributes = array( 'image' );

	
	/**
	 * Dynamic Constructor
	 *
	 * @param string	$label
	 * @param string	$textStyle
	 * @param string	$image
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_string($args[0]) )
				$this->label( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->textStyle( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->image( array_shift($args) );
			
			parent::__construct( $args );
			
		}
		else parent::__construct();
	}
	
	public function image( $src = NULL ) {
		if( $src === NULL )
			return (isset($this->image) ? $this->image : NULL);
		
		$this->image = $src;
		
		if( $this->isRegistered )
			$this->callMethod( 'image', $this->image );
		
		return $this;
	}
}


/**
 * Separator element
 * 
 * @link https://developer.mozilla.org/en/XUL/separator
*/

class XULSeparator extends Node
{
	public $remoteConstructor = 'Separator';
	
	
	/**
	 * Constructor
	 *
	 * @param string	$orient
	 * @param string	$className
	*/
	
	public function __construct( $orient = NULL, $className = NULL ) {
		if( $orient !== NULL )
			$this->orient( $orient );
		if( $className != NULL )
			$this->className( $className );
		
		parent::__construct();
	}
}


/**
 * Spacer element
 * 
 * @link https://developer.mozilla.org/en/XUL/spacer
*/

class XULSpacer extends Node
{
	public $remoteConstructor = 'Spacer';
	
	
	/**
	 * Dynamic Constructor
	 *
	 * @param int		$flex
	 * @param int		$width
	 * @param int		$height
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_int($args[0]) )
				$this->flex( array_shift($args) );
			if( count($args) && is_int($args[0]) )
				$this->width( array_shift($args) );
			if( count($args) && is_int($args[0]) )
				$this->height( array_shift($args) );
		}
		
		parent::__construct();
	}
}
