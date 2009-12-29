<?php
LoadClass( 'vlcplayer', 'components' );

function authorizeLogin( $args, $user, $pass ) {
   return ($user == 'user' && $pass == 'password');
}

function getApplication( $args ) {
   return 'SFMediaPlayer';
}

class SFMediaPlayer extends Application
{
	private $playing = FALSE;
	
   public function init( $event ) {
      $this->window->title( 'Mediaplayer' );
      $this->window->size( 1024, 740 );
      if ($this->clientComponents['VlcPlayer'])
      {
	      $this->window->addChild(
	     	 new XULHBox(
	     			$this->player = new XULVlcPlayer("http://pebble.nl/wp/video/trailers/Trailer_tiptap_FLASH.flv"),
	     			new XULVBox(
	     				$this->button = new XULButton("Play"),
	     				$this->pausebutton = new XULButton("Pause")
	     				)
	     		)
	      );
	      //$player->width(640);
	      //$player->height(480);
				
				$this->button->setEventHandler("command", $this, "onPressButton");
				$this->pausebutton->setEventHandler("command", $this, "onPressPause");
				$this->player->setEventHandler("stateChange", $this, "onStateChange");
		}
		else	{
				$this->window->addChild(
					new XULLabel("VLC Component not found in client!")
				);
		}
   }
   
   public function onStateChange($e, $state)
   {
   		switch($state)
   		{
   			case 1:
   			$this->button->label("Opening...");
   			$this->button->disabled(FALSE);
   			break;
   			case 2:
   			$this->button->label("Buffering...");
   			$this->button->disabled(FALSE);
   			break;
   			case 3:
   			//display stop
   			$this->playing = TRUE;
   			$this->button->label("Stop");
   			$this->button->disabled(FALSE);
   			break;
   			case 5:
   			$this->button->label("Stopping...");
   			$this->button->disabled(FALSE);
   			break;
   			case 0:
   			case 6:
   			//display play
   			$this->playing = FALSE;
   			$this->button->label("Play");
   			$this->button->disabled(FALSE);
   			break;
   			case 7:
   			$this->button->label("Error!");
   			$this->button->disabled(TRUE);
   			break;
   		}
   		
   }
   
   public function onPressPause()
   {
   	$this->player->togglePause();
   }
   
   public function onPressButton()
   {
   	
   	if ($this->playing)
   	{
   		//stop
   		$this->player->stop();
   	}
   	else
   	{
   		//play
   		$this->player->play();
   	}

   }
   
}