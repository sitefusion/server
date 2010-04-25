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
 * @subpackage CustomTree
*/


class XULCustomTree extends XULTree
{
	public $remoteConstructor = 'CustomTree';
	public $dataSet = NULL;
	
	
	/**
	 * Dynamic Constructor
	 *
	 * @param string	$seltype
	 * @param bool		$enableColumnDrag
	 * @param string	$selstyle
	 * @param int		$flex
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_string($args[0]) )
				$this->seltype( array_shift($args) );
			if( count($args) && is_bool($args[0]) )
				$this->enableColumnDrag( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->selstyle( array_shift($args) );
			if( count($args) && is_int($args[0]) )
				$this->flex( array_shift($args) );
			
			BranchNode::__construct( $args );
		}
		else BranchNode::__construct();
		
		$this->setEventHandler( 'onAfterAttach', $this, 'initView' );
		$this->setEventHandler( 'yield', $this, 'yieldCollect' );
	}
	
	public function initView() {
		$this->callMethod( 'setView' );
		
		$this->setEvent( 'cellValueChange', MSG_SEND, $this, 'cellValueYieldCollect' );
		$this->setEvent( 'treeDrop', MSG_SEND, $this, 'treeDropHandler' );
		$this->setEvent( 'nodeDrop', MSG_SEND, $this, 'nodeDropHandler' );
		$this->setEvent( 'openStateChange', MSG_SEND, $this, 'openStateCollector' );
		$this->setEvent( 'sortColumn', MSG_SEND, $this, 'sortColumnHandler' );
		
		$this->hostWindow->addStyleSheetRule(
			'treechildren::-moz-tree-checkbox { list-style-image: none; }',
			'treechildren::-moz-tree-checkbox(checked) { list-style-image: url("chrome://global/skin/checkbox/cbox-check.gif"); }'
		);
	}
	
	public function cellValueYieldCollect( $e, $rowId, $colId, $value ) {
		$value = $this->dataSet->idToRow[$rowId]->pushedUpdate( $colId, $value );
		$this->fireLocalEvent( 'cellUpdated', array( $this->dataSet->idToRow[$rowId], $this->dataSet->columns[$colId], $value ) );
	}
	
	public function treeDropHandler( $e, $sourceTree, $sourceSelection, $targetRow, $orientation ) {
		$selectedRows = array();
		foreach ( explode(',',$sourceSelection) as $rowId ) {
			$selectedRows[] = $sourceTree->dataSet->idToRow[$rowId];
		}
		
		$targetRow = $this->dataSet->idToRow[$targetRow];
		
		$this->fireLocalEvent( 'treeItemDropped', array( $sourceTree, $selectedRows, $targetRow, $orientation ) );
	}
	
	public function nodeDropHandler( $e, $sourceNode, $targetRow, $orientation ) {
		$this->fireLocalEvent( 'nodeDropped', array( $sourceNode, $this->dataSet->idToRow[$targetRow], $orientation ) );
	}
	
	public function sortColumnHandler( $e, $colIndex ) {
		$this->dataSet->sortColumn( (int) $colIndex );
	}
	
	public function yieldCollect() {
		$args = func_get_args();
		$e = array_shift( $args );
		$rows = array();
		
		foreach ( $args as $id ) {
			if( isset($this->dataSet->idToRow[$id]) )
				$rows[] = $this->dataSet->idToRow[$id];
		}
		
		$this->selectedRows = $rows;
	}
	
	public function select( $item ) {
		$ids = array();
		if(! is_array($item) )
			$item = array($item);
			
		foreach ( $item as $i ) {
			if( $i->id === NULL ) continue;
			$ids[] = $i->id;
		}
		
		$this->callMethod( 'select', array( json_encode($ids) ) );
	}
	
	public function openStateCollector( $e, $open, $rowId ) {
		$this->dataSet->idToRow[$rowId]->openStateChanged($open);
	}
}



class TreeDataSupports
{
	public $children = NULL;
	public $dataSet = NULL;
	
	public function insertRow( $row, $pos ) {
		if( ! $this->_isContainer ) throw new SFException( "Can't insert row in a non-container", ERR_REPORT_APP );
		if( $row->isChild )	throw new SFException( "Row is already a child", ERR_REPORT_APP );
		if( $this->dataSet !== $row->dataSet ) throw new SFException( "Row is from a different dataset", ERR_REPORT_APP );
		
		if( $pos === NULL )
			$this->children[] = $row;
		else
			array_splice( $this->children, $pos, 0, array($row) );

		$row->isChild = TRUE;
		$row->parentRow = $this;
		
		if( (!$this->dataSet->isSparse) || $this instanceof TreeDataSet || ($this->_isSync && $this->_isOpen) )
			$row->syncToDataSet( $pos );
	}
	
	public function addRow( $row ) {
		$pos = NULL;
		
		if( $this->dataSet->sortable && $this->dataSet->sortColumn !== NULL ) {
			switch( $this->dataSet->colSortTypes[$this->dataSet->sortColumn] ) {
				case 'numeric': $function = 'sortNumericCallback'; break;
				case 'alphanumeric': $function = 'sortAlphanumericCallback'; break;
				case 'date': $function = 'sortDateCallback'; break;
				default: $function = 'sortNaturalCallback'; break;
			}
			
			$val = array( $row->_cells[$this->dataSet->sortColumn] );
			
			for( $n = 0; $n < count($this->children); $n++ ) {
				$cmp = $this->{$function}( $val, array( $this->children[$n]->_cells[$this->dataSet->sortColumn] ) );
				if( ($this->dataSet->sortDirection == 'asc' && $cmp < 0) || ($this->dataSet->sortDirection == 'desc' && $cmp > 0) ) {
					$pos = $n;
					break;
				}
			}
		}
		
		$this->insertRow( $row, $pos );
	}
	
	public function insertRowBefore( $row, $before ) {
		if( ($index = array_search( $before, $this->children, TRUE )) === FALSE )
			throw new SFException( "Before-row is not a child of this row", ERR_REPORT_APP );
		
		$this->insertRow( $row, $index );
	}
	
	public function insertRowAfter( $row, $after ) {
		if( ($index = array_search( $after, $this->children, TRUE )) === FALSE )
			throw new SFException( "After-row is not a child of this row", ERR_REPORT_APP );
		
		$this->insertRow( $row, $index + 1 );
	}
	
	public function removeRow( $row ) {
		if( ! $this->_isContainer ) throw new SFException( "Can't remove row from a non-container" );
		if( ($index = array_search( $row, $this->children, TRUE )) === FALSE )
			throw new SFException( "Can't find row in childset", ERR_REPORT_APP );
		
		list($item) = array_splice( $this->children, $index, 1 );
		
		$item->isChild = FALSE;
		$item->parentRow = NULL;
		
		if( $item->_isSync )
			$item->removeFromDataSet();
		
		return $item;
	}
	
	public function sort( $recursive = FALSE ) {
		if( count($this->children) == 0 ) return;
		
		$col = $this->dataSet->sortColumn;
		$type = $this->dataSet->colSortTypes[$col];
		$dir = $this->dataSet->sortDirection;
		
		$values = array();
		foreach ( $this->children as $child ) {
			if( $recursive && $child->_isContainer )
				$child->sort( TRUE );
			
			$values[] = array( $child->_cells[$col], $child );
		}
		
		switch( $type ) {
			case 'numeric': $function = 'sortNumericCallback'; break;
			case 'alphanumeric': $function = 'sortAlphanumericCallback'; break;
			case 'date': $function = 'sortDateCallback'; break;
			default: $function = 'sortNaturalCallback'; break;
		}
		
		usort( $values, array(&$this,$function) );
		
		if( $dir == 'desc' ) $values = array_reverse( $values, TRUE );
		
		$isSync = $this->children[0]->_isSync;
		$newChildren = array();
		$ids = array();
		
		foreach ( $values as $item ) {
			$newChildren[] = $item[1];
			if( $isSync )
				$ids[] = $item[1]->id;
		}
		
		$this->children = $newChildren;
		
		if( $isSync )
			$this->dataSet->tree->callMethod( 'view.applyChildOrder', array( ($this instanceof TreeDataSet ? NULL:$this->id), json_encode($ids) ) );
	}
	
	private function sortNaturalCallBack( $a, $b ) {
		return strnatcasecmp( $a[0], $b[0] );
	}
	
	private function sortAlphanumericCallback( $a, $b ) {
		return strcasecmp( $a[0], $b[0] );
	}
	
	private function sortNumericCallback( $a, $b ) {
		$aa = (float) $a[0];
		$bb = (float) $b[0];
		return ($aa < $bb ? -1 : ($aa > $bb ? 1 : 0));
	}
	
	private function sortDateCallback( $a, $b ) {
		$aa = strtotime( $a[0] );
		$bb = strtotime( $b[0] );
		return ($aa < $bb ? -1 : ($aa > $bb ? 1 : 0));
	}
	
	public function getColumnIndex( $column ) {
		for( $colId = 0; $colId < count($this->dataSet->columns); $colId++ ) {
			if( $this->dataSet->columns[$colId] == $column )
				return $colId;
		}
		
		return NULL;
	}
}



class TreeDataItem extends TreeDataSupports
{
	public $isChild = FALSE;
	public $id = NULL;
	public $parentRow = NULL;
	public $isSeparator = FALSE;
	protected $_properties = NULL;
	protected $_cellProperties = NULL;
	protected $_isOpen = FALSE;
	protected $_isContainer = FALSE;
	protected $_isEmpty = TRUE;
	protected $_isSync = FALSE;

	public function remove() {
		if(! $this->isChild )
			throw new SFException( "Item is not a child", ERR_REPORT_APP );
		
		return $this->parentRow->removeRow( $this );
	}
	
	protected function updateProperty( $prop, $value ) {
		$rowObject = new stdClass;
		$rowObject->id = $this->id;
		$rowObject->{$prop} = $value;
		
		$this->dataSet->tree->callMethod( 'view.updateDataSetRow', array( json_encode($rowObject) ) );
	}

	protected function updateToDataSet() {
		$this->dataSet->tree->callMethod( 'view.updateDataSetRow', array( $this->getRowJSON() ) );
	}
	
	protected function syncToDataSet( $pos = NULL ) {
		if( $this->_isSync ) return;
		
		$this->id = $this->dataSet->nextIndex++;
		$this->_isSync = TRUE;
		$this->dataSet->idToRow[$this->id] = $this;
		
		$this->dataSet->tree->callMethod( 'view.addDataSetRow', array( $this->getRowJSON(), $pos ) );
		
		if( $this->_isContainer && ((!$this->dataSet->isSparse) || $this->isOpenRecursive()) ) {
			foreach ( $this->children as $row ) {
				$row->syncToDataSet();
			}
		}
	}
	
	protected function removeFromDataSet( $silent = FALSE ) {
		if( $this->_isContainer ) {
			foreach ( $this->children as $row ) {
				$row->removeFromDataSet(TRUE);
			}
		}
		
		if( (! $silent) && $this->_isSync )
			$this->dataSet->tree->callMethod( 'view.removeDataSetRow', array( $this->id ) );
		
		unset( $this->dataSet->idToRow[$this->id] );
		$this->id = NULL;
		$this->_isSync = FALSE;
	}
	
	public function isOpenRecursive() {
		$par = $this;
		
		while( $par instanceof TreeDataSet || $par->_isOpen ) {
			if( !isset($par->parentRow) )
				return TRUE;
				
			$par = $par->parentRow;
		}
		
		return FALSE;
	}
	
	protected function getRowJSON() {
		$rowObject = new stdClass;
		if( isset($this->_cells) )
			$rowObject->columns = $this->_cells;
		if( isset($this->_primaryImage) && $this->_primaryImage !== NULL )
			$rowObject->primaryImage = $this->_primaryImage;
		if( $this->isSeparator )
			$rowObject->isSeparator = TRUE;
		if( isset($this->_images) && $this->_images !== NULL )
			$rowObject->images = $this->_images;
		if( isset($this->_progressModes) && $this->_progressModes !== NULL )
			$rowObject->progressModes = $this->_progressModes;
		if( isset($this->_editableCells) && $this->_editableCells !== NULL )
			$rowObject->editableCells = $this->_editableCells;
		if( isset($this->_isDraggable) && $this->_isDraggable !== NULL )
			$rowObject->isDraggable = $this->_isDraggable;
		if( isset($this->_allowDrop) && $this->_allowDrop !== NULL )
			$rowObject->allowDrop = $this->_allowDrop;
		if( isset($this->_properties) && $this->_properties !== NULL )
			$rowObject->properties = $this->_properties;
		if( isset($this->_cellProperties) && $this->_cellProperties !== NULL )
			$rowObject->cellProperties = $this->_cellProperties;
		$rowObject->id = $this->id;
		$rowObject->isContainer = $this->_isContainer;
		$rowObject->isEmpty = $this->_isEmpty;
		$rowObject->isOpen = $this->_isOpen;
		$rowObject->parentId = ($this->parentRow instanceof TreeDataRow ? $this->parentRow->id : NULL);
		
		return json_encode($rowObject);
	}
	
	public function setRowProperties( $properties ) {
		if( ! is_array($properties) )
			throw new SFException( "Properties should be supplied as an array", ERR_REPORT_APP );
		
		$this->_properties = array_values($properties);
		
		if( $this->_isSync )
			$this->updateProperty( 'properties', $this->_properties );
		
		return $this;
	}
	
	public function setCellProperties( $column, $properties ) {
		if( ($index = $this->getColumnIndex($column)) === NULL )
			throw new SFException( "No such column: $column", ERR_REPORT_APP );
		if( ! is_array($properties) )
			throw new SFException( "Properties should be supplied as an array", ERR_REPORT_APP );
		
		if( ! is_array($this->_cellProperties) )
			$this->_cellProperties = array( $index => array_values( $properties ) );
		else
			$this->_cellProperties[$index] = array_values( $properties );
		
		if( $this->_isSync )
			$this->updateProperty( 'cellProperties', $this->_cellProperties );
		
		return $this;
	}
}


class TreeDataRow extends TreeDataItem
{
	public $data = NULL;
	protected $_cells = NULL;
	protected $_images = NULL;
	protected $_variables = array();
	protected $_primaryImage = NULL;
	protected $_progressModes = NULL;
	protected $_editableCells = NULL;
	protected $_isDraggable = NULL;
	protected $_allowDrop = NULL;
	
	public function __construct( $dataSet, $data = NULL, $container = FALSE ) {
		$this->dataSet = $dataSet;
		$this->_isContainer = (bool)$container;
		
		if( $this->_isContainer )
			$this->children = array();
		
		if( $data ) {
			$this->data = $data;
			$this->processData( $this->data );
		}
	}
	
	public function open( $state = NULL ) {
		if( $state === NULL )
			return $this->_isOpen;
		
		$this->openStateChanged( $state );
		
		if( $this->_isSync )
			$this->updateProperty( 'isOpen', (bool)$state );
		
		return $this;
	}
	
	public function primaryImage( $src = NULL ) {
		if( $src === NULL )
			return $this->_primaryImage;
		
		$this->_primaryImage = (string)$src;
		
		if( $this->_isSync )
			$this->updateProperty( 'primaryImage', (string)$src );
		
		return $this;
	}
	
	public function update( $data = NULL ) {
		if( $data ) {
			$this->data = $data;
			$this->processData( $this->data );
		}
		
		if( $this->_isSync )
			$this->updateToDataSet();
		
		return $this;
	}
	
	public function setImage( $column, $src ) {
		if( ($index = $this->getColumnIndex($column)) === NULL )
			throw new SFException( "No such column: $column", ERR_REPORT_APP );
		
		if(! is_array($this->_images) ) $this->_images = array( $index => $src );
		else $this->_images[$index] = $src;
		
		if( $this->_isSync )
			$this->updateProperty( 'images', $this->_images );
		
		return $this;
	}
	
	public function progressMode( $column, $mode ) {
		if( ($index = $this->getColumnIndex($column)) === NULL )
			throw new SFException( "No such column: $column", ERR_REPORT_APP );
		if( $this->dataSet->colTypes[$index] != 'progressmeter' )
			throw new SFException( "Column $column is not of type 'progressmeter'", ERR_REPORT_APP );
		
		if(! is_array($this->_progressModes) ) $this->_progressModes = array( $index => $mode );
		else $this->_progressModes[$index] = $mode;
		
		if( $this->_isSync )
			$this->updateProperty( 'progressModes', $this->_progressModes );
		
		return $this;
	}
	
	public function setEditable( $column, $state ) {
		if( ($index = $this->getColumnIndex($column)) === NULL )
			throw new SFException( "No such column: $column", ERR_REPORT_APP );
		
		if( (!is_array($this->_editableCells)) && $state ) $this->_editableCells = array( $index => $state );
		elseif( $state ) $this->_editableCells[$index] = $state;
		elseif( is_array($this->_editableCells) ) unset($this->_editableCells[$index]);
		
		if( $this->_isSync )
			$this->updateProperty( 'editableCells', $this->_editableCells );
		
		return $this;
	}
	
	public function setDraggable( $allow, $type = NULL ) {
		$types = array( 'move', 'copy', 'link', 'moveOrCopy' );
		
		if( $allow ) {
			if( ! in_array($type,$types) )
				throw new SFException( "Invalid type: $type", ERR_REPORT_APP );
			
			$this->_isDraggable = $type;
		}
		else
			$this->_isDraggable = FALSE;
		
		if( $this->_isSync )
			$this->updateProperty( 'isDraggable', $this->_isDraggable );
		
		return $this;
	}
	
	public function allowDrop( $allow, $type = 'any' ) {
		$types = array( 'any', 'on', 'between' );
		
		if( $allow ) {
			if( ! in_array($type,$types) )
				throw new SFException( "Invalid type: $type", ERR_REPORT_APP );
			
			$this->_allowDrop = $type;
		}
		else
			$this->_allowDrop = FALSE;
		
		if( $this->_isSync )
			$this->updateProperty( 'allowDrop', $this->_allowDrop );
		
		return $this;
	}
	
	public function insertRow( $row, $pos ) {
		parent::insertRow( $row, $pos );
		
		if( count($this->children) == 1 ) {
			if( $this->_isSync )
				$this->updateProperty( 'isEmpty', FALSE );
			$this->_isEmpty = FALSE;
		}
	}
	
	public function removeRow( $row ) {
		if( count($this->children) == 1 ) {
			if( $this->_isSync )
				$this->updateProperty( 'isEmpty', TRUE );
			$this->_isEmpty = TRUE;
		}
		
		return parent::removeRow( $row );
	}
	
	public function __set( $name, $value ) {
		for( $colId = 0; $colId < count($this->dataSet->columns); $colId++ ) {
			if( $this->dataSet->columns[$colId] == $name ) {
				switch ( $this->dataSet->colTypes[$colId] ) {
					case 'text':			$this->_cells[$colId] = ($value === NULL ? '':(string)$value); break;
					case 'checkbox':		$this->_cells[$colId] = ($value === NULL ? FALSE:(bool)$value); break;
					case 'progressmeter':	$this->_cells[$colId] = ($value === NULL ? 0:(int)$value); break;
				}
				
				if( $this->_isSync )
					$this->updateCell( $colId, $this->_cells[$colId] );
				
				return;
			}
		}
		
		$this->_variables[$name] = $value;
	}
	
	public function __get( $name ) {
		for( $colId = 0; $colId < count($this->dataSet->columns); $colId++ ) {
			if( $this->dataSet->columns[$colId] == $name )
				return $this->_cells[$colId];
		}
		
		if( isset($this->_variables[$name]) )
			return $this->_variables[$name];
		else
			throw new SFException( "No such column: $name", ERR_REPORT_APP );
	}
	
	public function __isset( $name ) {
		for( $colId = 0; $colId < count($this->dataSet->columns); $colId++ ) {
			if( $this->dataSet->columns[$colId] == $name )
				return TRUE;
		}
		return isset($this->_variables[$name]);
	}
	
	public function __unset( $name ) {
		for( $colId = 0; $colId < count($this->dataSet->columns); $colId++ ) {
			if( $this->dataSet->columns[$colId] == $name ) {
				switch ( $this->dataSet->colTypes[$colId] ) {
					case 'text':			$this->_cells[$colId] = ''; break;
					case 'checkbox':		$this->_cells[$colId] = FALSE; break;
					case 'progressmeter':	$this->_cells[$colId] = 0; break;
				}
				
				if( $this->_isSync )
					$this->updateCell( $colId, $this->_cells[$colId] );
				
				return;
			}
		}
		
		unset( $this->_variables[$name] );
	}
	
	public function openStateChanged( $state ) {
		$oldState = $this->_isOpen;
		$this->_isOpen = $state;
		
		if( $this->dataSet->isSparse && (!$oldState) && $state && $this->_isSync && isset($this->children[0]) && (!$this->children[0]->_isSync) )
			$this->addSparseBranch();
	}
	
	public function pushedUpdate( $colId, $value ) {
		switch ( $this->dataSet->colTypes[$colId] ) {
			case 'text':			return ($this->_cells[$colId] = (string)$value);
			case 'checkbox':		return ($this->_cells[$colId] = ($value == 'true' ? TRUE:FALSE));
			case 'progressmeter':	return ($this->_cells[$colId] = (int)$value);
		}
	}
	
	public function addSparseBranch() {
		for( $n = 0; $n < count($this->children); $n++ ) {
			$this->children[$n]->syncToDataSet();
		}
	}
	
	private function processData( $data ) {
		for( $colId = 0; $colId < count($this->dataSet->columns); $colId++ ) {
			$col = $this->dataSet->columns[$colId];
			
			if( is_object($data) ) {
				if( substr($col,-2) == '()' )
					$value = (method_exists($data,substr($col,0,strlen($col)-2)) ? $data->{substr($col,0,strlen($col)-2)}() : NULL);
				else
					$value = (isset($data->{$col}) ? $data->{$col} : NULL);
			}
			else
				$value = (isset($data[$col]) ? $data[$col] : NULL);
			
			switch ( $this->dataSet->colTypes[$colId] ) {
				case 'text':			$this->_cells[$colId] = ($value === NULL ? '':(string)$value); break;
				case 'checkbox':		$this->_cells[$colId] = ($value === NULL ? FALSE:(bool)$value); break;
				case 'progressmeter':	$this->_cells[$colId] = ($value === NULL ? 0:(int)$value); break;
			}
		}
	}
	
	protected function updateCell( $index, $label ) {
		$rowObject = new stdClass;
		$rowObject->id = $this->id;
		$rowObject->columns = array( $index => (string) $label );
		
		$this->dataSet->tree->callMethod( 'view.updateDataSetRow', array( json_encode($rowObject) ) );
	}
}


class TreeDataSeparator extends TreeDataItem
{
	public $isSeparator = TRUE;
	
	public function __construct( $dataSet ) {
		$this->dataSet = $dataSet;
	}
}


class TreeDataSet extends TreeDataSupports
{
	public $nextIndex = 0;
	public $idToRow = array();
	public $columns = array();
	public $colTypes = array();
	public $colSortTypes = array();
	public $sortable = FALSE;
	public $sortColumn = NULL;
	public $sortDirection = 'asc';
	public $columnProperties = array();
	public $isSparse = FALSE;
	public $preventCircularHeritage = TRUE;
	
	public function __construct( $tree, $sparse = FALSE ) {
		$this->tree = $tree;
		$this->isSparse = (bool)$sparse;
		$this->dataSet = $this;
		$tree->dataSet = $this;
		$this->_isContainer = TRUE;
		$this->children = array();
		
		foreach ( $tree->children[0]->children as $col ) {
			if( ! $col instanceof XULTreeCol )
				continue;
			
			if( ! isset($col->dataId) )
				throw new SFException( "XULTreeCol is missing dataId property", ERR_REPORT_APP );
			
			$this->columns[] = $col->dataId;
			$this->colTypes[] = $col->type();
			$this->colSortTypes[] = 'natural';
		}
		
		$tree->setProperty( 'isSparse', $this->isSparse );
		$tree->setProperty( 'preventCircularHeritage', $this->preventCircularHeritage );
		$tree->setProperty( 'sortable', $this->sortable );
	}
	
	public function preventCircularHeritage( $state ) {
		$this->tree->setProperty( 'preventCircularHeritage', ($this->preventCircularHeritage = (bool)$state) );
	}
	
	public function setSortable( $state ) {
		$this->tree->setProperty( 'sortable', ($this->sortable = (bool)$state) );
	}
	
	public function setColumnSortType( $col, $type ) {
		$types = array( 'natural', 'alphanumeric', 'numeric', 'date' );
		if(! in_array($type, $types) )
			throw new SFException( "Invalid sort type: $type", ERR_REPORT_APP );
		
		$this->colSortTypes[$this->getColumnIndex($col)] = $type;
	}
	
	public function setColumnProperties( $column, $properties ) {
		if( ($index = $this->getColumnIndex($column)) === NULL )
			throw new SFException( "No such column: $column", ERR_REPORT_APP );
		if( ! is_array($properties) )
			throw new SFException( "Properties should be supplied as an array", ERR_REPORT_APP );
		
		$this->columnProperties[$index] = array_values( $properties );
		$this->tree->callMethod( 'view.setColumnProperties', json_encode($this->columnProperties) );
	}
	
	public function allowForeignDrop( $allow ) {
		$this->tree->setProperty( 'allowForeignDrop', (bool)$allow );
	}
		
	public function sortColumn( $col, $dir = NULL ) {
		if( $this->sortColumn === $col && $dir === NULL ) {
			$this->sortDirection = ($this->sortDirection == 'asc' ? 'desc':'asc');
		}
		else {
			$this->sortColumn = $col;
			$this->sortDirection = ($dir == 'asc' || $dir === NULL ? 'asc':'desc');
		}
		
		if( $col !== NULL ) {
			$this->sort( TRUE );
		}
		
		$cols = $this->tree->children[0]->children;
		for( $n = 0; $n < count($cols); $n++ ) {
			if( $n === $col ) {
				$cols[$n]->setAttribute( 'sortActive', 'true' );
				$cols[$n]->setAttribute( 'sortDirection', ($this->sortDirection == 'asc' ? 'descending':'ascending') );
			}
			else {
				$cols[$n]->setAttribute( 'sortActive', 'false' );
				$cols[$n]->setAttribute( 'sortDirection', '' );
			}
		}
	}
	
	public function setDraggable( $allow, $type = 'move' ) {
		$types = array( 'move', 'copy', 'link', 'moveOrCopy' );
		
		if( $allow ) {
			if( ! in_array($type,$types) )
				throw new SFException( "Invalid type: $type", ERR_REPORT_APP );
			
			$this->tree->setProperty( 'isDraggable', $type );
		}
		else
			$this->tree->setProperty( 'isDraggable', FALSE );
	}
	
	public function allowDrop( $allow, $type = 'any' ) {
		$types = array( 'any', 'on', 'between' );
		
		if( $allow ) {
			if( ! in_array($type,$types) )
				throw new SFException( "Invalid type: $type", ERR_REPORT_APP );
			
			$this->tree->setProperty( 'allowDrop', $type );
		}
		else
			$this->tree->setProperty( 'allowDrop', FALSE );
	}
	
	public function createRow( $data = NULL, $container = FALSE ) {
		return new TreeDataRow( $this, $data, $container );
	}
	
	public function createSeparator() {
		return new TreeDataSeparator( $this );
	}
}
