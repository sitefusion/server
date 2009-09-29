<?php

function authorizeLogin( $args, $user, $pass ) {
   return ($user == 'user' && $pass == 'password');
}

function getApplication( $args ) {
   return 'SFGlobalEventExample';
}

class SFGlobalEventExample extends Application
{
	public function init( $event ) {
		$this->window->addChild(
			$this->textbox = new XULTextBox,
			$button = new XULButton( 'Send text' )
		);
		
		$button->setEvent( 'command', MSG_SEND, $this, 
					'sendText', $this->textbox );
		
		$this->setGlobalEventHandler( 'text', $this, 'receiveText' );
	}

	public function sendText( $event ) {
		$this->fireGlobalEvent( 'text', $this->textbox->value() );
	}

	public function receiveText( $event, $text ) {
		$this->window->alert( $text );
	}
}
