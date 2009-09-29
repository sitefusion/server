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
			)
		);

		// Tell the textboxes to call the yield
		// function on themselves when they lose
		// focus, let this event flush the queue
		// by setting it to MSG_SEND and have
		// $this->doSomething() handle it:
		foreach ( $this->textboxes as $textbox ) {
			$textbox->setEventType( 'blur', MSG_SEND )
				->setEventYielder( 'blur', $textbox )
				->setEventHandler( 'blur', $this, 'doSomething' );
		}

		// The shorthand method setEvent() would be
		// in this case:
		// $textbox->setEvent( 'blur', MSG_SEND, $this,
		//			'doSomething', $textbox );
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