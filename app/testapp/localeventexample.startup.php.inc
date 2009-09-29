<?php

function authorizeLogin( $args, $user, $pass ) {
   return ($user == 'user' && $pass == 'password');
}

function getApplication( $args ) {
   return 'SFLocalEventExample';
}

class SFLocalEventExample extends Application
{
	public function init( $event ) {
		$this->window->addChild(
			$button = new XULButton( 'Get me a textbox' )
		);
		
		$button->setEventHandler( 'command', $this, 'newTextbox' );
	}
	
	public function newTextbox( $event ) {
		$talkingTextbox = new TalkingTextbox( "I'm a textbox" );
		
		$this->window->alert( 'This is before attaching' );

		$this->window->addChild( $talkingTextbox );
		$this->window->sizeToContent();
	}
}

class TalkingTextbox extends XULTextBox
{
	public function __construct( $text ) {
		$this->sayWhat = $text;
		$this->setEventHandler( 'onAfterAttach', $this, 'talk' );
	}

	public function talk( $event ) {
		$this->hostWindow->alert( $this->sayWhat );
	}
}
