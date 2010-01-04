<?php

function authorizeLogin( $args, $user, $pass ) {
	return TRUE;
}

function getApplication( $args ) {
	return 'SFPrefWindowExample';
}


class SFPrefWindowExample extends Application
{
	public function init() {
		$this->window->addChild( $button = new XULButton( 'Open Preferences' ) );
		
		$button->setEvent( 'command', MSG_SEND, $this, 'openPrefs' );
	}
	
	public function openPrefs( $event ) {
		$this->window->addChild(
			new XULPrefWindow( 'Preferences',
				new XULLabel( 'bla' ),
				new XULPrefPane( 'Panel 1', '/app/sitefusion/images/player_fwd.png',
					new XULLabel( 'Pref1!' ),
					new XULButton( 'Do something' )
				),
				new XULPrefPane( 'Panel 2', '/app/sitefusion/images/player_play.png',
					new XULLabel( 'Pref2!' ),
					new XULButton( 'Do something' ),
					new XULLabel( 'Pref3!' ),
					new XULButton( 'Do more!' )
				)
			)
		);
	}
}