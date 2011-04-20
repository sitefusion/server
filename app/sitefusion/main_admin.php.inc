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


include_once( 'debugger.php.inc' );

class SitefusionAdmin extends Application
{
	public $centerscreen = TRUE;
	public $width = 1024;
	public $height = 740;
	
	public function init( $event ) {
		$this->window->title( 'SiteFusion Admin' );
		
		$this->window->addChild(
			new XULToolBox(
				new XULMenuBar(
					$menuFile = new XULMenu( 'Admin',
						new XULMenuPopup(
							$mAdminRestart = new XULMenu( 'Restart...',
								new XULMenuPopup(
									$mAdminRestartNow = new XULMenuItem( 'Now' ),
									$mAdminRestartGrace = new XULMenuItem( 'When convenient' )
								)
							),
							$mAdminStop = new XULMenu( 'Stop...',
								new XULMenuPopup(
									$mAdminStopNow = new XULMenuItem( 'Now' ),
									$mAdminStopGrace = new XULMenuItem( 'When convenient' )
								)
							),
							new XULMenuSeparator,
							$mAdminEditJobs = new XULMenuItem( 'Edit Job Schedule...' ),
							$mAdminEditServices = new XULMenuItem( 'Edit Services...' ),
							new XULMenuSeparator,
							$mAdminExit = new XULMenuItem( 'Exit' )
						)
					),
					$menuTools = new XULMenu( 'Tools',
						new XULMenuPopup(
							$mToolsInfoSF = new XULMenuItem( "SiteFusion Info" ),
							$mToolsErrorConsole = new XULMenuItem( "Error Console" ),
							$mToolsCheckUpdates = new XULMenuItem( "Check for updates..." ),
							new XULMenuSeparator,
							$mToolsAddons = new XULMenuItem( "Manage Add-ons" ),
							$mToolsAddonsSub = new XULMenu( "Add-ons",
								$mPopupAddons = new XULMenuPopup()
							),
							new XULMenuSeparator,
							$mToolsInfoMemory = new XULMenuItem( "Memory Usage" )
						)
					)
				)
			),
			
			$gb1 = new XULGroupBox(
				$cp1 = new XULCaption( 'Master Process', 'bold' ),
				
				new XULHBox(
					new XULGrid(
						new XULColumns(
							new XULColumn,
							new XULColumn,
							new XULColumn
						),
						new XULRows(
							new XULRow( 'center',
								new XULLabel( 'Restart SiteFusion Daemon' ),
								$btRestartGrace = new XULButton( 'Restart When Convenient' ),
								$btRestartNow = new XULButton( 'Restart Now' )
							),
							new XULRow( 'center',
								new XULLabel( 'Stop SiteFusion Daemon' ),
								$btStopGrace = new XULButton( 'Stop When Convenient' ),
								$btStopNow = new XULButton( 'Stop Now' )
							)
						)
					),
					
					new XULSpacer( 1 ),
					
					new XULVBox( 'center',
						$this->lblShutdown = new XULLabel( '', 'bold' ),
						$this->btCancelShutdown = new XULButton( 'Cancel' )
					),
					
					new XULSpacer( 1 ),
					
					new XULGrid(
						new XULColumns(
							new XULColumn,
							$infocol = new XULColumn
						),
						new XULRows(
							new XULRow( 'center',
								new XULLabel( 'Uptime' ),
								$this->lblUptime = new XULLabel( '' )
							),
							new XULRow( 'center',
								new XULLabel( 'Total CPU Usage' ),
								$this->lblCPU = new XULLabel( '', 'bold' )
							),
							new XULRow( 'center',
								new XULLabel( 'Total Memory Usage' ),
								$this->lblMemory = new XULLabel( '', 'bold' )
							),
							new XULRow( 'center',
								new XULLabel( 'Total Memory Allocated' ),
								$this->lblMemoryPeak = new XULLabel( '', 'bold' )
							)
						)
					)
				)
			),
			
			$gb2 = new XULGroupBox( 1,
				$cp2 = new XULCaption( 'Child Processes', 'bold' ),
				
				$this->tree = new XULTree( 'single', TRUE, 1,
					new XULTreeCols(
						$col1 = new XULTreeCol( 'PID', 1 ),
						new XULSplitter( 'tree-splitter' ),
						new XULTreeCol( 'Type', 1 ),
						new XULSplitter( 'tree-splitter' ),
						new XULTreeCol( 'Session ID', 2 ),
						new XULSplitter( 'tree-splitter' ),
						new XULTreeCol( 'Port', 1 ),
						new XULSplitter( 'tree-splitter' ),
						new XULTreeCol( 'Memory Usage', 1 ),
						new XULSplitter( 'tree-splitter' ),
						new XULTreeCol( 'Memory Allocated', 1 ),
						new XULSplitter( 'tree-splitter' ),
						new XULTreeCol( 'CPU %', 1 ),
						new XULSplitter( 'tree-splitter' ),
						new XULTreeCol( 'Nodes', 1 ),
						new XULSplitter( 'tree-splitter' ),
						new XULTreeCol( 'CPU Time', 2 ),
						new XULSplitter( 'tree-splitter' ),
						new XULTreeCol( 'Client IP', 2 ),
						new XULSplitter( 'tree-splitter' ),
						new XULTreeCol( 'User', 2 ),
						new XULSplitter( 'tree-splitter' ),
						new XULTreeCol( 'Application', 3 ),
						new XULSplitter( 'tree-splitter' ),
						new XULTreeCol( 'Arguments', 2 )
					),
					$this->treeChildren = new XULTreeChildren
				)
			),
			
			new XULGroupBox(
				new XULCaption( 'Tasks', 'bold' ),
				
				new XULHBox(
					$btEditJobs = new XULButton( 'Edit Job Schedule...' ),
					$btEditServices = new XULButton( 'Edit Service List...' )
				)
			),
			
			new XULPopupSet(
				$treePopup = new XULPopup(
					$menuInfo = new XULMenuItem( 'Info' ),
					$menuDebug = new XULMenuItem( 'Debug' ),
					$menuTerm = new XULMenuItem( 'Shutdown Process' ),
					$menuKill = new XULMenuItem( 'Kill Process' ),
					$menuSendProcessMsg = new XULMenuItem( 'Send Message' ),
					new XULMenuSeparator,
					$menuSendBroadcastMsg = new XULMenuItem( 'Broadcast Message' )
				)
			),
			
			$this->ps = new PromptService
		);
		
		$infocol->width( 200 );
		
		$this->lblShutdown->setStyle( 'color', 'red' );
		$this->btCancelShutdown->hidden( TRUE )->setEvent( 'command', MSG_SEND, $this, 'cancelShutdown' );
		
		
		//check for the DOMInspector Component
		if (isset(ApplicationProcess::$ExtensionInfo['inspector@mozilla.org']) && ApplicationProcess::$ExtensionInfo['inspector@mozilla.org']['enabled'] == true)
		{
			$mPopupAddons->addChild(
				$mAddonsDI = new XULMenuItem( "DOM Inspector" )
			);
			$mAddonsDI->setEvent( 'command', MSG_SEND, 'DOMInspector', 'Open' );
		}
		
		//check for the Venkman Component
		if (isset(ApplicationProcess::$ExtensionInfo['{f13b157f-b174-47e7-a34d-4815ddfdfeb8}']) && ApplicationProcess::$ExtensionInfo['{f13b157f-b174-47e7-a34d-4815ddfdfeb8}']['enabled'] == true)
		{
			$mPopupAddons->addChild(
				$mAddonsVenkman = new XULMenuItem( "Venkman" )
			);
			$mAddonsVenkman->setEvent( 'command', MSG_SEND, 'Venkman', 'Open' );
		}
		
		if (!count($mPopupAddons->children))
		{
			$mToolsAddonsSub->disabled(TRUE);
		}
		
		$mAdminRestartNow->setEvent( 'command', MSG_SEND, $this, 'shutdownButton' );
		$mAdminRestartNow->restart = TRUE;
		$mAdminRestartNow->grace = FALSE;
		
		$mAdminRestartGrace->setEvent( 'command', MSG_SEND, $this, 'shutdownButton' );
		$mAdminRestartGrace->restart = TRUE;
		$mAdminRestartGrace->grace = TRUE;
		
		$mAdminStopNow->setEvent( 'command', MSG_SEND, $this, 'shutdownButton' );
		$mAdminStopNow->restart = FALSE;
		$mAdminStopNow->grace = FALSE;
		
		$mAdminStopGrace->setEvent( 'command', MSG_SEND, $this, 'shutdownButton' );
		$mAdminStopGrace->restart = FALSE;
		$mAdminStopGrace->grace = TRUE;
		
		$mAdminEditJobs->setEvent( 'command', MSG_SEND, $this, 'openJobScheduleDialog' );
		$mAdminEditServices->setEvent( 'command', MSG_SEND, $this, 'openServiceListDialog' );
		$mAdminExit->setEvent( 'command', MSG_SEND, $this->window, 'close' );
		
		$mToolsInfoSF->setEvent( 'command', MSG_SEND, $this, 'openAbout' );
		$mToolsInfoMemory->setEvent( 'command', MSG_SEND, $this, 'openAboutMemory' );
		$mToolsAddons->setEvent( 'command', MSG_SEND, $this, 'openExtensions' );
		$mToolsCheckUpdates->setEvent( 'command', MSG_SEND, $this, 'checkForUpdates' );
		$mToolsErrorConsole->setEvent( 'command', MSG_SEND, $this, 'openErrorConsole' );
		$this->tree->setEvent( 'dblclick', MSG_SEND, $this, 'openInfoWindow', $this->tree );
		
		$this->tree->context( $treePopup );
		$menuInfo->setEvent( 'command', MSG_SEND, $this, 'openInfoWindow', $this->tree );
		$menuDebug->setEvent( 'command', MSG_SEND, $this, 'startDebugger', $this->tree );
		$menuKill->setEvent( 'command', MSG_SEND, $this, 'killProcess', $this->tree );
		$menuTerm->setEvent( 'command', MSG_SEND, $this, 'termProcess', $this->tree );
		$menuSendProcessMsg->setEvent( 'command', MSG_SEND, $this, 'sendProcessMsg', $this->tree );
		$menuSendBroadcastMsg->setEvent( 'command', MSG_SEND, $this, 'sendBroadcastMsg' );
		
		$btEditJobs->setEvent( 'command', MSG_SEND, $this, 'openJobScheduleDialog' );
		$btEditServices->setEvent( 'command', MSG_SEND, $this, 'openServiceListDialog' );
		
		$btRestartNow->setEvent( 'command', MSG_SEND, $this, 'shutdownButton' );
		$btRestartNow->restart = TRUE;
		$btRestartNow->grace = FALSE;
		
		$btRestartGrace->setEvent( 'command', MSG_SEND, $this, 'shutdownButton' );
		$btRestartGrace->restart = TRUE;
		$btRestartGrace->grace = TRUE;
		
		$btStopNow->setEvent( 'command', MSG_SEND, $this, 'shutdownButton' );
		$btStopNow->restart = FALSE;
		$btStopNow->grace = FALSE;
		
		$btStopGrace->setEvent( 'command', MSG_SEND, $this, 'shutdownButton' );
		$btStopGrace->restart = FALSE;
		$btStopGrace->grace = TRUE;
		
		$this->window->focus();
		
		$this->setInterval( 1000, $this, 'updateProcessInterface' );
	}
	
	public function openInfoWindow ($e ) {
		if( count($this->tree->selectedRows) ) {
			$sessionId = $this->tree->selectedRows[0]->sid;
			if ($sessionId) {
				$appInfo = ApplicationProcess::GetApplicationInfo($sessionId);
				if ($appInfo == NULL)
				{
					$this->ps->alert("Method not supported", "Selected process does not contain any Application Info.");
					return;
				}
				$this->window->addChild(
					new SiteFusionAdminAppInfoWindow($appInfo)
				);
			}
		}
	}
	
	public function startDebugger( $e ) {
		if(! count($this->tree->selectedRows) )
			return;
		
		$this->window->addChild( new SitefusionDebugger( $this->tree->selectedRows[0]->pid ) );
	}
	
	public function killProcess( $e ) {
		if(! count($this->tree->selectedRows) )
			return;
		
	 	ApplicationProcess::SendSessionSignal( $this->tree->selectedRows[0]->pid, SIGKILL );
	}
	
	public function termProcess( $e ) {
		if(! count($this->tree->selectedRows) )
			return;
		
		ApplicationProcess::SendSessionSignal( $this->tree->selectedRows[0]->pid, SIGTERM );
	}
	
	public function sendProcessMsg() {
		if(! count($this->tree->selectedRows) )
			return;
		
		$sid = $this->tree->selectedRows[0]->sid;
		
		$this->window->addChild( new SiteFusionAdminMessageDialog( $sid ) );
	}
	
	public function sendBroadcastMsg() {
		$this->window->addChild( new SiteFusionAdminMessageDialog() );
	}
	
	public function openJobScheduleDialog() {
		$this->window->addChild( new SiteFusionAdminJobScheduleDialog() );
	}
	
	public function openServiceListDialog() {
		$this->window->addChild( new SiteFusionAdminServiceListDialog() );
	}

	public function shutdownButton( $e ) {
		$this->ps->confirm( ($e->sourceObject->restart ? 'Restart':'Shutdown'), "Are you sure you want to continue?", $this, 'doShutdown', $e );
	}
	
	public function doShutdown( $ps, $e ) {
		if( ! $ps->result )
			return;
		
		ApplicationProcess::SendDaemonShutdownFlags( TRUE, $e->sourceObject->grace, $e->sourceObject->restart );
		if( !$e->sourceObject->grace ) {
			sleep(1);
			$this->window->close();
		}
	}
	
	public function cancelShutdown() {
		ApplicationProcess::SendDaemonShutdownFlags( FALSE, FALSE, FALSE );
	}
	
	public function updateProcessInterface() {
		$plist = array();
		$daemonstatus = ApplicationProcess::GetDaemonStatus();
		$mem = $daemonstatus['memory'];
		$mempeak = $daemonstatus['memory_peak'];
		$totalcpu = 0;
		
		foreach ( ApplicationProcess::GetDaemonProcessList() as $process ) {
			$plist[$process['pid']] = $process;
			$mem += $process['memory'];
			$mempeak += $process['memory_peak'];
		}
		
		$pids = array_keys($plist);
		$pids[] = (int) DaemonProcess::$PID;
		$pids = join(' ',$pids);
		$ps = shell_exec( "/bin/ps -o pid,pcpu,time -p $pids" );
		
		$lines = explode( "\n", rtrim( $ps ) );
		array_shift( $lines );
		
		foreach ( $lines as $line ) {
			$psdata = preg_split( '/\s+/', trim($line) );
			if( count($psdata) < 3 )
				continue;
			
			list($pid,$cpu,$time) = $psdata;
			
			$totalcpu += floatval($cpu);
			
			if( $pid == DaemonProcess::$PID )
				continue;
			
			if( isset($plist[$pid]) ) {
				$plist[$pid]['cpu'] = $cpu;
				$plist[$pid]['time'] = $time;
			}
		}
		
		foreach( $this->treeChildren->children as $child ) {
			if( !isset($plist[$child->pid]) ) {
				$child->removeNode();
			}
			else {
				$child->updateRow( $plist[$child->pid] );
				unset( $plist[$child->pid] );
			}
		}
		
		foreach ( $plist as $process ) {
			$this->treeChildren->addChild( new SiteFusionProcessTreeItem($process) );
		}
		
		$this->lblMemory->value( number_format($mem,0,',','.'). ' Bytes' );
		$this->lblMemoryPeak->value( number_format($mempeak,0,',','.'). ' Bytes' );
		$d = new Duration;
		$this->lblUptime->value( $d->toString( time() - DaemonProcess::$Started ) );
		$this->lblCPU->value( $totalcpu.'%' );
		
		$this->lblShutdown->value( $daemonstatus['shutdown'] && $daemonstatus['graceful'] ? 'Daemon is set to '.($daemonstatus['restart'] ? 'Restart':'Shut Down') : '' );
		$this->btCancelShutdown->hidden( $daemonstatus['shutdown'] && $daemonstatus['graceful'] ? FALSE : TRUE );
		
	}
}


class SiteFusionProcessTreeItem extends XULTreeItem
{
	public function __construct( $process ) {
		$this->pid = $process['pid'];
		$this->sid = $process['id'];
		parent::__construct(
			new XULTreeRow(
				new XULTreeCell( (string) $process['pid'] ),
				new XULTreeCell( $process['type'] ),
				new XULTreeCell( $process['id'] ),
				new XULTreeCell( (string) $process['port'] ),
				new XULTreeCell( number_format($process['memory'],0,',','.'). ' Bytes' ),
				new XULTreeCell( number_format($process['memory_peak'],0,',','.'). ' Bytes' ),
				new XULTreeCell( $process['cpu'] ),
				new XULTreeCell( (string) $process['nodes'] ),
				new XULTreeCell( $process['time'] ),
				new XULTreeCell( $process['client_ip'] ),
				new XULTreeCell( $process['user'] ),
				new XULTreeCell( $process['app'] ),
				new XULTreeCell( $process['args'] )
			)
		);
	}
	
	public function updateRow( $process ) {
		$row = $this->children[0];
		$row->children[4]->label( number_format($process['memory'],0,',','.'). ' Bytes' );
		$row->children[5]->label( number_format($process['memory_peak'],0,',','.'). ' Bytes' );
		$row->children[6]->label( (isset($process['cpu']) ? $process['cpu'] : "") );
		$row->children[7]->label( (string) $process['nodes'] );
		$row->children[8]->label( isset($process['time']) ? $process['time'] : "" );
	}
}

class SiteFusionAdminAppInfoWindow extends XULChildWindow
{
	public $title = 'Application Info';
	public $resizable = TRUE;

	public function __construct( $appInfo = NULL ) {
		$this->appInfo = $appInfo;
	}
	
	public function init() {
		$this->size(800, 600);
		$this->addChild(
			new XULGrid(1,
				new XULColumns(
					new XULColumn(1),
					new XULColumn(1)
				),
				new XULRows(
					new XULRow(1,
						new XULVBox(1,
							new XULLabel("Application Info"),
							$this->listboxAppInfo = new XULListBox(1,
								new XULListHead(
									new XULListHeader("Key"),
									new XULListHeader("Value")
								),
								new XULListCols(
									new XULListCol(1),
									new XULListCol(1)
								)
							)
						),
						new XULVBox(1,
							new XULLabel("Extension Info"),
							$this->treeExtensionInfo = new XULTree(1,
								new XULTreeCols(
									new XULTreeCol("Key", 1, TRUE),
									new XULTreeCol("Value", 1)
								),
								new XULTreeChildren()
							)
						)
					),
					new XULRow(1,
						new XULVBox(1,
							new XULLabel("Platform Info"),
							$this->listboxPlatformInfo = new XULListBox(1,
								new XULListHead(
									new XULListHeader("Key"),
									new XULListHeader("Value")
								),
								new XULListCols(
									new XULListCol(1),
									new XULListCol(1)
								)
							)
						),
						new XULVBox(1,
							new XULLabel("Command Line Arguments"),
							$this->listboxCommandlineArgsInfo = new XULListBox(1,
								new XULListHead(
									new XULListHeader("Key"),
									new XULListHeader("Value")
								),
								new XULListCols(
									new XULListCol(1),
									new XULListCol(1)
								)
							)
						)
					)
				)
			)
		);
		
		if( is_array($this->appInfo['AppInfo']) ) {
			foreach( $this->appInfo['AppInfo'] as $key => $value) {
				
				$this->listboxAppInfo->addChild(
					new XULListItem(
						new XULListCell(strval($key)),
						new XULListCell(strval($value))
					)
				);
			}
		}
		
		if( is_array($this->appInfo['ExtensionInfo']) ) {
			foreach( $this->appInfo['ExtensionInfo'] as $key => $value) {
				
				$this->treeExtensionInfo->children[1]->addChild(
					new XULTreeItem(TRUE,
						new XULTreeRow(
							new XULTreeCell(strval($key))
						),
						$treeChildren = new XULTreeChildren()
					)
				);
						
				if (is_array($value)) {
					foreach ($value as $subvaluekey => $subvalue) {
						$treeChildren->addChild (
							new XULTreeItem(
								new XULTreeRow(
									new XULTreeCell(strval($subvaluekey)),
									new XULTreeCell(strval($subvalue))
								)
							)
						);
					}
				}
			}
		}
		
		if( is_array($this->appInfo['PlatformInfo']) ) {
			foreach( $this->appInfo['PlatformInfo'] as $key => $value) {
				
				$this->listboxPlatformInfo->addChild(
					new XULListItem(
						new XULListCell(strval($key)),
						new XULListCell(strval($value))
					)
				);
			}
		}
		
		if( is_array($this->appInfo['CommandLineArgs']) ) {
			foreach( $this->appInfo['CommandLineArgs'] as $key => $value) {
				
				$this->listboxCommandlineArgsInfo->addChild(
					new XULListItem(
						new XULListCell(strval($key)),
						new XULListCell(strval($value))
					)
				);
			}
		}
	}
	
}
class SiteFusionAdminMessageDialog extends XULDialog
{
	public $title = 'Send Message';
	public $modal = TRUE;
	
	public function __construct( $to = NULL ) {
		$this->to = $to;
	}
	
	public function init() {
		$this->addChild(
			$this->text = new XULTextBox( '', TRUE )
		);
		
		$this->text->flex( 1 );
		$this->size( 400, 400 );
		$this->setEvent( 'accept', MSG_SEND, $this, 'accept', $this->text );
	}
	
	public function accept() {
		ApplicationProcess::SendBroadcastMessage( $this->text->value(), $this->to );
	}
}

class SiteFusionAdminJobScheduleDialog extends XULDialog
{
	public $title = 'Edit Job Schedule';
	public $resizable = TRUE;
	public $modal = TRUE;
	
	public function init() {
		$this->addChild(
			new XULGroupBox( 1,
				new XULCaption( 'Current Job Schedule', 'bold' ),
				new XULScrollBox( 1, 
					new XULGrid(
						new XULColumns(
							new XULColumn,
							new XULColumn,
							new XULColumn,
							new XULColumn,
							new XULColumn,
							new XULColumn,
							new XULColumn,
							new XULColumn,
							new XULColumn,
							new XULColumn
						),
						$this->jobContainer = new XULRows(
							new XULRow(
								new XULLabel( 'Owner', 'bold' ),
								new XULLabel( 'Job', 'bold' ),
								new XULLabel( 'Arguments', 'bold' ),
								new XULLabel( 'Minute', 'bold' ),
								new XULLabel( 'Hour', 'bold' ),
								new XULLabel( 'Day of Month', 'bold' ),
								new XULLabel( 'Month', 'bold' ),
								new XULLabel( 'Day of Week', 'bold' ),
								new XULLabel( 'Mail to', 'bold' ),
								new XULSpacer
							)
						)
					)
				)
			),
			new XULHBox( $btAdd = new XULButton( 'Add Job' ) )
		);
		
		$this->jobContainer->setStyle( 'overflow', 'auto' );
		$this->size( 950, 500 );
		$btAdd->setEvent( 'command', MSG_SEND, $this, 'addJob' );
		
		$list = ApplicationProcess::GetJobList();
		
		$this->idList = array();
		$rows = array();
		
		foreach ( $list as $job ) {
			$this->idList[] = $job['id'];
			$this->jobContainer->addChild( $jobrow = new SiteFusionAdminJobScheduleItem($job) );
			$this->setEventYielder( 'accept', $jobrow );
			$rows[] = $jobrow;
		}
		
		$this->setEvent( 'accept', MSG_SEND, $this, 'accept', $rows );
	}
	
	public function addJob() {
		$this->jobContainer->addChild( $row = new SiteFusionAdminJobScheduleItem( array('owner'=>'','job'=>'','args'=>'','minute'=>'-1','hour'=>'-1','monthday'=>'-1','month'=>'-1','weekday'=>'-1','mail_output'=>'') ) );
		$this->setEventYielder( 'accept', $row );
	}
	
	public function accept() {
		foreach ( $this->jobContainer->children as $row ) {
			if(! isset($row->job) ) continue;
			if( isset($row->jobId) ) {
				ApplicationProcess::ChangeJob( $row->jobId, $row->job );
				$this->idList = array_diff( $this->idList, array($row->jobId) );
			}
			else
				ApplicationProcess::CreateJob( $row->job );
		}
		
		foreach ( $this->idList as $id ) {
			ApplicationProcess::DeleteJob( $id );
		}
	}
}

class SiteFusionAdminJobScheduleItem extends XULRow
{
	public function __construct( $job ) {
		$this->job = $job;
		if( isset($job['id']) )
			$this->jobId = $job['id'];
		$this->setEventHandler( 'onAfterAttach', $this, 'init' );
	}
	
	public function init() {
		$job = $this->job;
		
		$this->align( 'center' );
		$this->addChild(
			$this->inpOwner = new XULTextBox( $job['owner'] ),
			$this->inpJob = new XULTextBox( $job['job'] ),
			$this->inpArgs = new XULTextBox( $job['args'] ),
			$this->inpMinute = new XULMenuList( new XULMenuPopup( new XULMenuItem( 'Every', '-1' ) ) ),
			$this->inpHour = new XULMenuList( new XULMenuPopup( new XULMenuItem( 'Every', '-1' ) ) ),
			$this->inpMonthDay = new XULMenuList( new XULMenuPopup( new XULMenuItem( 'Every', '-1' ) ) ),
			$this->inpMonth = new XULMenuList( new XULMenuPopup( new XULMenuItem( 'Every', '-1' ) ) ),
			$this->inpWeekDay = new XULMenuList( new XULMenuPopup( new XULMenuItem( 'Any', '-1' ) ) ),
			$this->inpMailTo = new XULTextBox( $job['mail_output'] ),
			$removeButton = new XULButton( 'Remove' )
		);
		
		$this->inpOwner->width( 60 );
		$this->inpJob->width( 60 );
		$this->inpArgs->width( 70 );
		$this->inpMailTo->width( 80 );
		
		for( $n = 0; $n <= 59; $n++ )
			$this->inpMinute->children[0]->addChild( new XULMenuItem( "$n", "$n" ) );
		$this->inpMinute->selectItem( $job['minute'] );
		
		for( $n = 0; $n <= 23; $n++ )
			$this->inpHour->children[0]->addChild( new XULMenuItem( "$n", "$n" ) );
		$this->inpHour->selectItem( $job['hour'] );
		
		for( $n = 1; $n <= 31; $n++ )
			$this->inpMonthDay->children[0]->addChild( new XULMenuItem( "$n", "$n" ) );
		$this->inpMonthDay->selectItem( $job['monthday'] );
		
		$this->inpMonth->children[0]->addChild(
			new XULMenuItem( 'January', '1' ),
			new XULMenuItem( 'February', '2' ),
			new XULMenuItem( 'March', '3' ),
			new XULMenuItem( 'April', '4' ),
			new XULMenuItem( 'May', '5' ),
			new XULMenuItem( 'June', '6' ),
			new XULMenuItem( 'July', '7' ),
			new XULMenuItem( 'August', '8' ),
			new XULMenuItem( 'September', '9' ),
			new XULMenuItem( 'October', '10' ),
			new XULMenuItem( 'November', '11' ),
			new XULMenuItem( 'December', '12' )
		);
		$this->inpMonth->selectItem( $job['month'] );
		
		$this->inpWeekDay->children[0]->addChild(
			new XULMenuItem( 'Sunday', '0' ),
			new XULMenuItem( 'Monday', '1' ),
			new XULMenuItem( 'Tuesday', '2' ),
			new XULMenuItem( 'Wednesday', '3' ),
			new XULMenuItem( 'Thursday', '4' ),
			new XULMenuItem( 'Friday', '5' ),
			new XULMenuItem( 'Saturday', '6' )
		);
		$this->inpWeekDay->selectItem( $job['weekday'] );
		
		$removeButton->setEvent( 'command', MSG_SEND, $this, 'removeNode' );
		
		$this->setMethod( 'yield', array(), 'this.fireEvent("yield");' );
		
		$this->setEvent( 'yield', MSG_QUEUE, $this, 'collectYield', array($this->inpOwner,$this->inpJob,$this->inpArgs,$this->inpMinute,$this->inpHour,$this->inpMonthDay,$this->inpMonth,$this->inpWeekDay,$this->inpMailTo) );
	}
	
	public function collectYield() {
		$this->job['owner'] = $this->inpOwner->value();
		$this->job['job'] = $this->inpJob->value();
		$this->job['args'] = $this->inpArgs->value();
		$this->job['minute'] = $this->inpMinute->selectedValue;
		$this->job['hour'] = $this->inpHour->selectedValue;
		$this->job['monthday'] = $this->inpMonthDay->selectedValue;
		$this->job['month'] = $this->inpMonth->selectedValue;
		$this->job['weekday'] = $this->inpWeekDay->selectedValue;
		$this->job['mail_output'] = $this->inpMailTo->value();
	}
}



class SiteFusionAdminServiceListDialog extends XULDialog
{
	public $title = 'Edit Service List';
	public $resizable = TRUE;
	public $modal = TRUE;
	
	public function init() {
		$this->addChild(
			new XULGroupBox( 1,
				new XULCaption( 'Current Service List', 'bold' ),
				new XULScrollBox( 1, 
					new XULGrid(
						new XULColumns(
							new XULColumn,
							new XULColumn,
							new XULColumn,
							new XULColumn,
							new XULColumn
						),
						$this->serviceContainer = new XULRows(
							new XULRow(
								new XULLabel( 'Owner', 'bold' ),
								new XULLabel( 'Service', 'bold' ),
								new XULLabel( 'Arguments', 'bold' ),
								new XULLabel( 'Process Type', 'bold' ),
								new XULSpacer
							)
						)
					)
				)
			),
			new XULHBox( $btAdd = new XULButton( 'Add Service' ) )
		);
		
		$this->serviceContainer->setStyle( 'overflow', 'auto' );
		$this->size( 700, 450 );
		$btAdd->setEvent( 'command', MSG_SEND, $this, 'addService' );
		
		$list = ApplicationProcess::GetServiceList();
		
		$this->idList = array();
		$rows = array();
		
		foreach ( $list as $srv ) {
			$this->idList[] = $srv['id'];
			$this->serviceContainer->addChild( $srvrow = new SiteFusionAdminServiceListItem($srv) );
			$this->setEventYielder( 'accept', $srvrow );
			$rows[] = $srvrow;
		}
		
		$this->setEvent( 'accept', MSG_SEND, $this, 'accept', $rows );
	}
	
	public function addService() {
		$this->serviceContainer->addChild( $row = new SiteFusionAdminServiceListItem( array('owner'=>'','service'=>'','args'=>'','daemon'=>'0') ) );
		$this->setEventYielder( 'accept', $row );
	}
	
	public function accept() {
		foreach ( $this->serviceContainer->children as $row ) {
			if(! isset($row->service) ) continue;
			
			if( isset($row->serviceId) ) {
				if( $row->currentServiceData != $row->service )
					ApplicationProcess::ChangeService( $row->serviceId, $row->service );
				
				$this->idList = array_diff( $this->idList, array($row->serviceId) );
			}
			else {
				ApplicationProcess::CreateService( $row->service );
			}
		}
		
		foreach ( $this->idList as $id ) {
			ApplicationProcess::DeleteService( $id );
		}
	}
}

class SiteFusionAdminServiceListItem extends XULRow
{
	public function __construct( $srv ) {
		$this->service = $srv;
		if( isset($srv['id']) ) {
			$this->serviceId = $srv['id'];
			$this->currentServiceData = $srv;
		}
		$this->setEventHandler( 'onAfterAttach', $this, 'init' );
	}
	
	public function init() {
		$srv = $this->service;
		
		$this->align( 'center' );
		$this->addChild(
			$this->inpOwner = new XULTextBox( $srv['owner'] ),
			$this->inpService = new XULTextBox( $srv['service'] ),
			$this->inpArgs = new XULTextBox( $srv['args'] ),
			$this->inpDaemon = new XULMenuList(
				new XULMenuPopup(
					new XULMenuItem( 'One Continuous Process', '1' ),
					new XULMenuItem( 'Multiple On-demand Processes', '0' )
				)
			),
			$removeButton = new XULButton( 'Remove' )
		);
		
		$this->inpDaemon->selectItem( (string) $srv['daemon'] );
		
		$this->inpOwner->width( 100 );
		$this->inpService->width( 100 );
		$this->inpArgs->width( 100 );
		
		$removeButton->setEvent( 'command', MSG_SEND, $this, 'removeNode' );
		
		$this->setMethod( 'yield', array(), 'this.fireEvent("yield");' );
		
		$this->setEvent( 'yield', MSG_QUEUE, $this, 'collectYield', array($this->inpOwner,$this->inpService,$this->inpArgs,$this->inpDaemon) );
	}
	
	public function collectYield() {
		$this->service['owner'] = $this->inpOwner->value();
		$this->service['service'] = $this->inpService->value();
		$this->service['args'] = $this->inpArgs->value();
		$this->service['daemon'] = $this->inpDaemon->selectedValue;
	}
}


class Duration 
{ 
	static $plural = array(
		'year' => 'years',
		'month' => 'months',
		'week' => 'weeks',
		'day' => 'days',
		'hour' => 'hours',
		'minute' => 'mins',
		'sec' => 'secs'
	);
	
    function toString ($duration, $periods = null) 
    { 
        if (!is_array($duration)) { 
            $duration = Duration::int2array($duration, $periods); 
        } 
  
        return Duration::array2string($duration); 
    } 
  
  
    function int2array ($seconds, $periods = null) 
    {         
        // Define time periods 
        if (!is_array($periods)) { 
            $periods = array ( 
                    'year'     => 31556926, 
                    'month'    => 2629743, 
                    'week'     => 604800, 
                    'day'      => 86400, 
                    'hour'     => 3600, 
                    'minute'   => 60, 
                    'sec'   => 1 
                    ); 
        } 
  
        // Loop 
        $seconds = (float) $seconds; 
        foreach ($periods as $period => $value) { 
            $count = floor($seconds / $value); 
  
            if ($count == 0) { 
                continue; 
            } 
            
            $values[$period] = $count; 
            $seconds = $seconds % $value; 
        } 
  
        // Return 
        if (empty($values)) { 
            $values = null; 
        } 
  
        return $values; 
    } 
  
    function array2string ($duration) 
    { 
        if (!is_array($duration)) { 
            return false; 
        } 
        
        $max = 4;
        $count = 0;
  
        foreach ($duration as $key => $value) { 
            $segment_name = $key;
            
            if( $value != 1 )
            	$segment_name = self::$plural[$key];
            
            $segment = $value . ' ' . $segment_name;
  
            $array[] = $segment; 
            
            if( ++$count == $max )
            	break;
        }
  
        $str = implode(', ', $array); 
        return $str;
    } 
  
}
