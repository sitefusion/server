<?php

/**
 * @package SiteFusion
 * @subpackage Lists
*/


class XULListBox extends BranchNode
{
	public $remoteConstructor = 'ListBox';
	public $initAttributes = array( 'seltype', 'rows' );
	public $selectedItems = array();
	public $selectedItem = NULL;
	public $selectedValue = NULL;
	
	
	/**
	 * Dynamic Constructor
	 *
	 * @param int		$flex
	 * @param string	$seltype
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_int($args[0]) )
				$this->flex( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->seltype( array_shift($args) );
			
			parent::__construct( $args );
		}
		else parent::__construct();
		
		$this->setEventHandler( 'yield', $this, 'yieldCollect' );
	}
	
	public function removeItems()
	{
		for ($i=0; $i < count($this->children); $i++)
		{
			if ($this->children[$i] instanceof XULListItem)
			{
				$this->removeChild($this->children[$i]);
				$i--;
			}

		}
	}
	
	public function yieldCollect() {
		$args = func_get_args();
		$e = array_shift( $args );
		
		$this->selectedItem = array_shift( $args );
		$this->selectedValue = (isset($this->selectedItem->value) ? $this->selectedItem->value : NULL);
		$this->selectedItems = $args;
	}

	public function seltype( $type = NULL ) {
		return $this->attributeMethod( 'seltype', ($type === NULL ? NULL : ($type == 'single' ? 'single':'multiple')) );
	}
	
	public function rows( $amount = NULL ) {
		return $this->attributeMethod( 'rows', $amount );
	}
}

class XULListItem extends BranchNode
{
	public $remoteConstructor = 'ListItem';
	public $initAttributes = array( 'image' );
	
	
	/**
	 * Dynamic Constructor
	 *
	 * @param string	$label
	 * @param string	$value
	 * @param string	$image
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_string($args[0]) )
				$this->label( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->value( array_shift($args) );
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
		
		if( $this->isRegistered ) {
			$this->callMethod( 'image', $this->image );
			$this->setAttribute( 'class', 'listitem-iconic' );
		}
		
		return $this;
	}
}

class XULListCols extends BranchNode
{
	public $remoteConstructor = 'ListCols';
}

class XULListCol extends BranchNode
{
	public $remoteConstructor = 'ListCol';

	
	/**
	 * Dynamic Constructor
	 *
	 * @param int		$flex
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_integer($args[0]) )
				$this->flex( array_shift($args) );
			
			parent::__construct( $args );
		}
		else parent::__construct();
	}
}

class XULListHead extends BranchNode
{
	public $remoteConstructor = 'ListHead';
}

class XULListHeader extends BranchNode
{
	public $remoteConstructor = 'ListHeader';

	
	/**
	 * Dynamic Constructor
	 *
	 * @param string	$label
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_string($args[0]) )
				$this->label( array_shift($args) );
			
			parent::__construct( $args );
		}
		else parent::__construct();
	}
}

class XULListCell extends BranchNode
{
	public $remoteConstructor = 'ListCell';

	
	/**
	 * Dynamic Constructor
	 *
	 * @param string	$label
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_string($args[0]) )
				$this->label( array_shift($args) );
			
			parent::__construct( $args );
		}
		else parent::__construct();
	}
}

