<?php

function authorizeLogin( $args, $user, $pass ) {
   return ($user == 'user' && $pass == 'password');
}

function getApplication( $args ) {
   return 'SFEventExample';
}

class SFEventExample extends Application
{
	public function init( $event ) {
		
		$this->window->addChild(
			$this->textboxes = array(
				new XULTextBox(),
				new XULTextBox()
			),
			$button = new XULButton( 'Do something' )
		);

		// Attach the handler to the event, in
		// this case $this->doSomething()
		$button->setEventHandler( 'command', $this, 'doSomething' );

		// Tell the button to call the yield 
		// function on the textboxes when the
		// command event fires:
		$button->setEventYielder( 'command', $this->textboxes );

		// Alternatively, you could use the
		// shorthand method setEvent() that also
		// includes the event message type:
		// $button->setEvent( 'command', MSG_SEND, $this,
		//			'doSomething', $this->textboxes );
	}

	public function doSomething( $event ) {
		$this->window->alert(
			"Event fired: " . $event->name .
			"\nTextbox 1 contains: " . 
			$this->textboxes[0]->value() . 
			"\nTextbox 2 contains: " . 
			$this->textboxes[1]->value()
		);
	}
}