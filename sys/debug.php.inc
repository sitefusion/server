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
 * @subpackage Debugger
*/


class SiteFusionDebugWindow extends XULChildWindow
{
	private $stackCount = 1;
	private $bufferStackAdds = TRUE;
	private $stackAdds = array();
	
	public function __construct( $bt ) {
		$this->backtrace = $bt;
		$this->title = 'Stack @ ' . basename($bt[0]['file']) . '(' . $bt[0]['line'] . ')';
	}
	
	public function init() {
		$this->addChild(
			new XULHBox( 1,
				$this->stackTree = new XULTree( 'single',
					new XULTreeCols(
						new XULTreeCol( 'Stack', 1 )
					),
					$this->stackTreeItems = new XULTreeChildren
				),
				$this->scrollBox = new XULScrollBox( 1,
					$this->stackViewBox = new XULDeck( 1 )
				)
			)
		);
		
		$this->scrollBox->setStyle( 'overflow', 'auto' );
		$this->stackTree->flex(1)->width(70)->hidecolumnpicker(TRUE)->setEvent( 'select', MSG_SEND, $this, 'treeSelect', $this->stackTree );
		$this->bufferStackAdds = FALSE;
		
		array_unshift( $this->stackAdds, $this->backtrace );
		foreach ( $this->stackAdds as $bt ) {
			$this->addStackView( $bt );
		}
		
		$this->sizeToContent();
	}
	
	public function addStackView( $bt ) {
		if( $this->bufferStackAdds ) {
			$this->stackAdds[] = $bt;
			return;
		}
		
		$this->stackTreeItems->addChild(
			$treeItem = new XULTreeItem(
				new XULTreeRow(
					new XULTreeCell( '# '.$this->stackCount++ )
				)
			)
		);
		
		$this->stackViewBox->addChild( $panel = new SiteFusionDebugStackView( $bt ) );
		$treeItem->setEvent( 'select', MSG_SEND, $this, 'treeSelect' );
		$treeItem->panel = $panel;
		$this->stackViewBox->selectedPanel( $panel );
		$this->stackTree->select( $treeItem );
		
		$this->sizeToContent();
	}
	
	public function treeSelect() {
		if( ! count($this->stackTree->selectedRows) )
			return;
		
		$this->stackViewBox->selectedPanel( $this->stackTree->selectedRows[0]->panel );
	}
}


class SiteFusionDebugStackView extends XULGrid
{
	public function __construct( $bt ) {
		$this->backtrace = $bt;
		$this->setEventHandler( 'onAfterAttach', $this, 'init' );
	}
	
	public function init() {
		$bt = $this->backtrace;
		
		$this->addChild(
			new XULColumns(
				new XULColumn,
				new XULColumn,
				new XULColumn
			),
			$this->stackItems = new XULRows(
				new XULRow(
					new XULLabel( 'File', 'bold' ),
					new XULLabel( 'Line', 'bold' ),
					new XULLabel( 'Call', 'bold' )
				)
			)
		);
		
		for( $n = 0; $n < count($bt); $n++ ) {
			$file = (isset($bt[$n]['file']) ? $bt[$n]['file'] : '');
			$line = (string) (isset($bt[$n]['line']) ? $bt[$n]['line'] : '');
			$function = (isset($bt[$n]['function']) ? $bt[$n]['function'] : NULL);
			$class = (isset($bt[$n]['class']) ? $bt[$n]['class'] : NULL);
			$object = (isset($bt[$n]['object']) ? $bt[$n]['object'] : NULL);
			$type = (isset($bt[$n]['type']) ? $bt[$n]['type'] : NULL);
			$args = (isset($bt[$n]['args']) ? $bt[$n]['args'] : array());

			$this->stackItems->addChild(
				new XULRow( new XULLabel( basename($file) ), new XULLabel( $line ), $callBox = new XULHBox() )
			);

			if( $type == '::' )
				$callBox->addChild( new XULLabel( $class . ' ' . $type . ' ' . $function ) );
			elseif( $type == '->' ) {
				$callBox->addChild( $label = new XULLabel( get_class($object), 'underline' ), new XULLabel( '-> '.$function ) );
				$label->setStyle( 'cursor', 'pointer' )->setStyle( 'color', 'blue' );
				$label->setEvent( 'click', MSG_SEND, $this, 'showObject' );
				$label->object = $object;
			}
			else
				$callBox->addChild( new XULLabel( $function ) );
			
			$callBox->addChild( new XULLabel( '(', 'bold' ) );

			for( $a = 0; $a < count($args); $a++ ) {
				if( $a != 0 )
					$callBox->addChild( new XULLabel( ',', 'bold' ) );
				
				switch ( gettype($args[$a]) ) {
					case 'NULL':
						$callBox->addChild( new XULLabel( 'NULL' ) );
						break;
					case 'boolean':
						$callBox->addChild( new XULLabel( $args[$a] ? 'TRUE':'FALSE' ) );
						break;
					case 'string':
						if( strlen($args[$a]) > 25 )
							$show = substr($args[$a],0,25).'...';
						else
							$show = $args[$a];
						$callBox->addChild( $label = new XULLabel( '"'.$show.'"' ) );
						$label->setStyle( 'cursor', 'pointer' )->setStyle( 'color', 'maroon' );
						$label->setEvent( 'click', MSG_SEND, $this, 'showObject' );
						$label->object = $args[$a];
						break;
					case 'object':
						$callBox->addChild( $label = new XULLabel( get_class($args[$a]), 'underline' ) );
						$label->setStyle( 'cursor', 'pointer' )->setStyle( 'color', 'blue' );
						$label->setEvent( 'click', MSG_SEND, $this, 'showObject' );
						$label->object = $args[$a];
						break;
					case 'array':
						$callBox->addChild( $label = new XULLabel( 'Array('.count($args[$a]).')', 'underline' ) );
						$label->setStyle( 'cursor', 'pointer' )->setStyle( 'color', 'blue' );
						$label->setEvent( 'click', MSG_SEND, $this, 'showObject' );
						$label->object = $args[$a];
						break;
					default:
						$callBox->addChild( $label = new XULLabel( (string) $args[$a] ) );
				}
			}
			
			$callBox->addChild( new XULLabel( ')', 'bold' ) );
		}
	}
	
	public function showObject( $e ) {
		$this->addChild( new SiteFusionDebugObjectInspector($e->sourceObject) );
	}
}


class SiteFusionDebugObjectInspector extends XULChildWindow
{
	public function __construct( $label ) {
		$this->title = $label->value();
		$this->object = $label->object;
	}
	
	public function init() {
		if( gettype($this->object) == 'string' ) {
			$this->addChild(
				$textbox = new XULTextBox( $this->object, TRUE )
			);
			$textbox->flex(1);
			$this->size( 500, 300 );
		}
		else {
			$this->addChild(
				$this->scrollBox = new XULScrollBox( 1,
					new XULGrid(
						new XULColumns(
							new XULColumn,
							new XULColumn
						),
						$rows = new XULRows(
							new XULRow(
								new XULLabel( gettype($this->object) == 'object' ? 'Property':'Key', 'bold' ),
								new XULLabel( 'Value', 'bold' )
							)
						)
					)
				)
			);
			
			$this->scrollBox->setStyle( 'overflow', 'auto' );
			
			foreach ( (array) $this->object as $key => $val ) {
				$rows->addChild( $row = new XULRow );
			
				$row->addChild( new XULLabel( (string) $key ) );
			
				switch ( gettype($val) ) {
					case 'boolean':
						$row->addChild( new XULLabel( $val ? 'TRUE':'FALSE' ) );
						break;
					case 'string':
						if( strlen($val) > 150 )
							$show = substr($val,0,150).'...';
						else
							$show = $val;
						$row->addChild( $label = new XULLabel( '"'.$show.'"' ) );
						$label->setStyle( 'cursor', 'pointer' )->setStyle( 'color', 'maroon' );
						$label->setEvent( 'click', MSG_SEND, $this, 'showObject' );
						$label->object = $val;
						break;
					case 'object':
						$row->addChild( $label = new XULLabel( get_class($val), 'underline' ) );
						$label->setStyle( 'cursor', 'pointer' )->setStyle( 'color', 'blue' );
						$label->setEvent( 'click', MSG_SEND, $this, 'showObject' );
						$label->object = $val;
						break;
					case 'array':
						$row->addChild( $label = new XULLabel( 'Array('.count($val).')', 'underline' ) );
						$label->setStyle( 'cursor', 'pointer' )->setStyle( 'color', 'blue' );
						$label->setEvent( 'click', MSG_SEND, $this, 'showObject' );
						$label->object = $val;
						break;
					default:
						$row->addChild( $label = new XULLabel( (string) $val ) );
				}
			}
		
			$this->sizeToContent();
		}
	}
	
	public function showObject( $e ) {
		$this->addChild( new self($e->sourceObject) );
	}
}



