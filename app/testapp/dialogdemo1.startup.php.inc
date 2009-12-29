<?php

function authorizeLogin( $args, $user, $pass ) {
	return TRUE;
}

function getApplication( $args ) {
	return 'SFDialogDemo';
}

class SFDialogDemo extends Application
{
	public function init( $event ) {
		$this->window->addChild(
			$btDialog = new XULButton( 'Open Dialog' ),
			$btChildWindow = new XULButton( 'Open Window' )
		);

		$btDialog->setEventHandler( 'command', $this, 'openDialog' );
		$btChildWindow->setEventHandler( 'command', $this, 
							'openChildWindow' );
	}
	
	public function openDialog( $event ) {
		$this->window->addChild(
			$dialog = new XULDialog( 'Nice Dialog',
				new XULLabel( 'This is a nice dialog indeed' )
			)
		);
	}

	public function openChildWindow( $event ) {
		$this->window->addChild(
			$cwin = new XULChildWindow( 'Some Window',
				new XULLabel( 'Hi there' )
			)
		);
	}
}
