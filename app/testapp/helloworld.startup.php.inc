<?php

function authorizeLogin( $args, $user, $pass ) {
   return ($user == 'user' && $pass == 'password');
}

function getApplication( $args ) {
   return 'SFHelloWorld';
}

class SFHelloWorld extends Application
{
   public function init( $event ) {
      $this->window->title( 'Hello World!' );
      $this->window->size( 400, 300 );
      $this->window->addChild(
         new XULLabel("Hello World!")
      );
   }
}