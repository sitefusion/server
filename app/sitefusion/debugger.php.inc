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
 * @package SiteFusionAdmin
*/


class SitefusionDebugger extends XULChildWindow
{
	public $title = 'Debugger';
	private $lastFileLine = '';
	
	public function __construct( $pid ) {
		$this->debugPid = $pid;
		$this->showLinesValue = 7;
	}
	
	public function init() {
		$this->socket = ApplicationProcess::GetDebugSocket( $this->debugPid );
		$this->appStatus = 'running';
		$this->setEvent( 'close', MSG_SEND, $this, 'closeSocket' );
		$this->timer = $this->rootApplication->setInterval( 50, $this, 'listenToApplication' );
		
		$this->addChild(
			new XULHBox( 1,
				$this->stackTree = new XULTree( 'single',
					new XULTreeCols(
						new XULTreeCol( 'Stack', 1 )
					),
					$this->stackTreeItems = new XULTreeChildren
				),
				new XULVBox( 1,
					new XULHBox( 'center',
						new XULVBox(
							$this->ticksCheck = new XULCheckBox( 'Pause at every line' ),
							$this->appTicksCheck = new XULCheckBox( 'Application lines only' )
						),
						$this->pauseButton = new XULButton(),
						$this->resumeButton = new XULButton(),
						new XULLabel( 'Status:' ),
						$this->appStatusLabel = new XULLabel( 'Running', 'bold' )
					),
					$this->stackDeck = new XULDeck( 1 )
				)
			)
		);
		
		$this->stackTree->width(200)->hidecolumnpicker(TRUE)->setEvent( 'select', MSG_SEND, $this, 'treeSelect', $this->stackTree );
		$this->pauseButton->image( '/app/sitefusion/images/player_pause.png' )->setEvent( 'command', MSG_SEND, $this, 'pauseProcess' );
		$this->resumeButton->image( '/app/sitefusion/images/player_play.png' )->setEvent( 'command', MSG_SEND, $this, 'resumeProcess' );
		$this->resumeButton->disabled( TRUE );
		$this->ticksCheck->setEvent( 'command', MSG_SEND, $this, 'setTicks', $this->ticksCheck );
		$this->appTicksCheck->disabled( TRUE )->setEvent( 'command', MSG_SEND, $this, 'setTicks', $this->appTicksCheck );
		$this->size( 750, 550 );
	}
	
	public function closeSocket() {
		$this->removeEventHandler( 'close', $this, 'closeSocket' );
		$this->timer->cancel();
		
		try {
			socket_close( $this->socket );
		}
		catch ( SFException $ex ) {}
	}
	
	public function treeSelect() {
		if( ! count($this->stackTree->selectedRows) )
			return;
		
		$this->stackDeck->selectedPanel( $this->stackTree->selectedRows[0]->panel );
	}
	
	public function setTicks( $e ) {
		ApplicationProcess::DebuggerWriteCommand( $this->socket, ($this->ticksCheck->checked() ? 'TICKSON':'TICKSOFF'), array( 'appTicks' => $this->appTicksCheck->checked() ? '1':'0') );
		$this->appTicksCheck->disabled( !$this->ticksCheck->checked() );
	}

	public function listenToApplication() {
		while( 1 ) {
			$read = array($this->socket);
			$write = NULL;
			$except = NULL;
			
			try {
				if( socket_select($read, $write, $except, 0) < 1 )
					return;
			
				if( count($read) ) {
					$socket = $read[0];
				
					$cmd = ApplicationProcess::DebuggerReadCommand( $socket );
				
					switch ( $cmd->command ) {
						case 'STACK': $this->handleStackCommand( $cmd ); break;
					}
				}
				else return;
			}
			catch ( SFException $ex ) {
				$this->applicationClosed();
				return;
			}
		}
	}
	
	public function handleStackCommand( $cmd ) {
		$pause = intval($cmd->pause);
		$crash = intval($cmd->crash);
		$data = unserialize( $cmd->data );

/*		if( $this->lastFileLine == $data['file'].$data['line'] && $data['function'] == 'tickfunction' && $this->ticksCheck->checked() ) {
			$this->resumeProcess();
			return;
		}*/
				
		$this->lastFileLine = $data['file'].$data['line'];
		
		if( ! $crash ) {
			$this->appStatus = ($pause ? 'paused':'running');
			$this->appStatusLabel->value( $pause ? 'Paused':'Running' );
			$this->pauseButton->disabled( $pause );
			$this->resumeButton->disabled( !$pause );
		}
		else $this->applicationCrashed();
		
		$this->stackTreeItems->addChild(
			$treeItem = new XULTreeItem(
				new XULTreeRow(
					new XULTreeCell( ($crash ? 'CRASH: ':'') . basename($data['file']) . '(' . $data['line'] . ')' )
				)
			)
		);
		
		$this->stackDeck->addChild(
			$panel = new XULScrollBox(
				new XULGroupBox(
					new XULCaption( 'Call Stack' ),
					$grid = new XULGrid(
						new XULColumns(
							new XULColumn,
							new XULColumn,
							new XULColumn
						),
						$stackItems = new XULRows()
					)
				),
				new XULGroupBox(
					new XULCaption( 'Code Tracker' ),
					new XULStack(
						$this->codeBox = new XULVBox,
						new XULVBox( 'end', $this->showLines = new XULMenuList( new XULMenuPopup ) )
					)
				)
			)
		);
		
		$this->showLines->children[0]->addChild(
			new XULMenuItem( '15 lines', '15' ),
			new XULMenuItem( '13 lines', '13' ),
			new XULMenuItem( '11 lines', '11' ),
			new XULMenuItem( '9 lines', '9' ),
			new XULMenuItem( '7 lines', '7' ),
			new XULMenuItem( '5 lines', '5' ),
			new XULMenuItem( '3 lines', '3' )
		)->setEvent( 'command', MSG_SEND, $this, 'showLinesChanged', $this->showLines );
		
		$this->showLines->selectItem((string)$this->showLinesValue);
		
		if( $crash )
			$stackItems->addChild( new XULLabel( $data['crash'] ) );
		
		$stackItems->addChild(
			new XULRow(
				new XULLabel( 'File', 'bold' ),
				new XULLabel( 'Line', 'bold' ),
				new XULLabel( 'Call', 'bold' )
			)
		);
		
		foreach ( $data['stack'] as $row ) {
			$stackItems->addChild( new SitefusionDebuggerStackRow( $row ) );
		}
		
		$panel->orient( 'vertical' )->setStyle( 'overflow', 'auto' );
		$treeItem->panel = $panel;
		$this->stackDeck->selectedPanel( $panel );
		$this->stackTree->select( $treeItem );
		$stackItems->setEventHandler( 'lineSelected', $this, 'onLineSelect' );

		$stackItems->children[1]->select();
	}
	
	public function showLinesChanged( $e ) {
		$this->showLinesValue = (int) $this->showLines->selectedValue;
		$this->showCode( $this->lastFile, $this->lastLine );
	}
	
	public function showCode( $file, $line ) {
		$this->codeBox->removeChildren();
		
		$this->lastFile = $file;
		$this->lastLine = $line;
		
		if( $file == '' || $line == '' ) {
			$this->codeBox->addChild( $descr = new XULDescription( '[Internal Function]' ) );
			return;
		}
		
		$startline = max($line-floor($this->showLinesValue/2),0);
		$endline = $line + floor($this->showLinesValue/2);
		$code = explode( "\n", shell_exec( "sed '$startline,$endline!d' ".$file ) );
		for( $n = $startline; $n <= $endline; $n++ ) {
			$this->codeBox->addChild( $descr = new XULDescription( $n . ': ' . array_shift($code) ) );
			$descr->setStyle( 'whiteSpace', 'pre' );
			if( $n == $line )
				$descr->setStyle( 'color', 'blue' );
		}
	}
	
	public function onLineSelect( $e, $stackLine ) {
		$this->showCode( $stackLine[0], $stackLine[1] );
	}
	
	public function pauseProcess() {
		$this->appStatus = 'paused';
		$this->appStatusLabel->value( 'Paused' );
		$this->pauseButton->disabled( TRUE );
		$this->resumeButton->disabled( FALSE );
		ApplicationProcess::DebuggerWriteCommand( $this->socket, 'PAUSE' );
	}
	
	public function resumeProcess() {
		$this->appStatus = 'running';
		$this->appStatusLabel->value( 'Running' );
		$this->pauseButton->disabled( FALSE );
		$this->resumeButton->disabled( TRUE );
		ApplicationProcess::DebuggerWriteCommand( $this->socket, 'RESUME' );
	}
	
	public function applicationClosed() {
		$this->closeSocket();
		$this->pauseButton->disabled( TRUE );
		$this->resumeButton->disabled( TRUE );
		$this->appStatus = 'closed';
		$this->appStatusLabel->value( 'Closed' );
	}
	
	public function applicationCrashed() {
		$this->closeSocket();
		$this->pauseButton->disabled( TRUE );
		$this->resumeButton->disabled( TRUE );
		$this->appStatus = 'crashed';
		$this->appStatusLabel->value( 'Crashed' )->setStyle( 'color', 'red' );
	}
}

class SitefusionDebuggerStackRow extends XULRow
{
	public function __construct( $row ) {
		$this->stackLine = $row;
		$this->selected = FALSE;
		
		$this->addChild(
			new XULLabel( basename($row[0]) ),
			new XULLabel( (string) $row[1] ),
			new XULLabel( $row[2] )
		);
		
		$this->setEventHandler( 'onAfterAttach', $this, 'init' );
	}
	
	public function init() {
		$this->setEventReflex( 'mouseover', "$this->js.element.style.textDecoration = 'underline';" );
		$this->setEventReflex( 'mouseout', "$this->js.element.style.textDecoration = '';" );
		
		$this->setEvent( 'click', MSG_SEND, $this, 'select' );
		$this->parent->setEventHandler( 'lineSelected', $this, 'checkSelection' );
	}
	
	public function checkSelection( $e, $stackLine ) {
		if( $stackLine == $this->stackLine && ! $this->selected )
			$this->select();
		else if( $stackLine != $this->stackLine && $this->selected )
			$this->deselect();
	}
	
	public function select() {
		$this->selected = TRUE;
		$this->setStyle( 'color', 'blue' );
		$this->parent->fireLocalEvent( 'lineSelected', array($this->stackLine) );
	}
	
	public function deselect() {
		$this->selected = FALSE;
		$this->setStyle( 'color', '' );
	}
}

