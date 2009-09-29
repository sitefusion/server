<?php

function authorizeLogin( $args, $user, $pass ) {
   return ($user == 'user' && $pass == 'password');
}

function getApplication( $args ) {
   return 'SFMiniChat';
}

class SFMiniChat extends Application
{
	public function init( $event ) {
		$this->window->size( 400, 400 );
		$this->window->title( 'MiniChat' );
		
		// Basic interface construction
		$this->window->addChild(
			$this->messagesBox = new XULScrollBox,
			
			new XULHBox( 'center',
				$this->textBox = new XULTextBox,
				$button = new XULButton( 'Send message' )
			)
		);
		
		// Make the scrollbox stretch, show a scrollbar and
		// distribute its children vertically
		$this->messagesBox
			->flex(1)
			->setStyle( 'overflow', 'auto' )
			->orient( 'vertical' );
		
		// Textbox stretches in the width
		$this->textBox->flex(1);
		
		// Connect the button to the sendText() handler and
		// yield the textbox
		$button->setEvent( 'command', MSG_SEND, $this, 
					'sendText', $this->textBox );
		
		// Add the promptservice and the return key handler
		$this->window->addChild(
			$this->ps = new PromptService,
			new XULKeySet(
				new XULKey( 'VK_RETURN', '', $this, 'sendText', $this->textBox )
			)
		);
		
		// Attach the global event 'message' to the handler
		// $this->receiveText()
		$this->setGlobalEventHandler( 'message', $this, 'receiveText' );
		
		// Let the promptservice open a prompt for the user's
		// name and have it's result handled by the 
		// $this->login() method
		$this->ps->prompt( 'Your name', "What's your name?", '', $this, 'login' );
	}
	
	// Promptservice handlers always receive the promptservice object
	public function login( $ps ) {
		if( ! $ps->result ) { // User cancelled
			$this->window->close();
			return;
		}
		
		// Save the name
		$this->myName = $ps->value;
		
		// Fire a 'message' global event with the text as first argument
		$this->fireGlobalEvent( 'message', $this->myName . ' joined the chat' );
	}

	public function sendText( $event ) {
		$text = $this->myName . " says: " . $this->textBox->value();
		// Clear the textbox
		$this->textBox->value('');
		
		// Fire the message global event
		$this->fireGlobalEvent( 'message', $text );
		
		// We don't receive our own messages back so add
		// them separately:
		$this->messagesBox->addChild( $msg = new XULDescription($text) );
		$this->messagesBox->ensureElementIsVisible( $msg );
	}

	public function receiveText( $event, $text ) {
		$this->messagesBox->addChild( $msg = new XULDescription($text) );
		$this->messagesBox->ensureElementIsVisible( $msg );
	}
}
