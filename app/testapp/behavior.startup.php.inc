<?php

function authorizeLogin( $args, $user, $pass ) {
	return TRUE;
}

function getApplication( $args ) {
	return 'SFBehaviorExample';
}

class SFBehaviorExample extends Application
{
   public function init( $event ) {
		$this->window->title( 'Behavior example' );
		$this->window->size( 400, 300 );
		$this->window->addChild(
			$label = new XULLabel("Hello World!"),
			$button = new XULButton("Change text"),
			$label2 = new XULLabel(),
			$button2 = new XULButton("Change number")
		);
		
		$label->addBehavior('CapsText', new MyBehavior);
		$button->setEventHandler('command', $this, 'setText');
		
		$label2->addBehavior('MultiplyingCounter', 'MyComplexBehavior', array('multiplier' => 3));
		$button2->setEventHandler('command', $this, 'addNumber');
		
		$this->label = $label;
		$this->label2 = $label2;
	}
	
	public function setText($event) {
		$this->label->setAllCapsText("New text");
	}
	
	public function addNumber($event) {
		$this->label2->addMultipliedNumber(2);
	}
}

class MyBehavior extends Behavior
{
	public function setAllCapsText($text) {
		$this->getOwner()->value(strtoupper($text));
	}
}

class MyComplexBehavior extends Behavior
{
	public $multiplier = 1;
	public $value = 0;
	
	public function init() {
		$this->getOwner()->value((string)$this->value);
	}
	
	public function addMultipliedNumber($number) {
		$this->value += $number * $this->multiplier;
		$this->getOwner()->value((string)$this->value);
	}
}