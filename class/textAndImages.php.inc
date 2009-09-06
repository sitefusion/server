<?php

/**
 * @package SiteFusion
 * @subpackage TextAndImages
*/


class XULLabel extends Node
{
	protected $value = '';
	protected $style = '';
	public $remoteConstructor = 'Label';
	
	
	public function __construct( $text = '', $style = NULL ) {
		$this->value( $text );
		$this->textStyle( $style );
	}
}

class XULDescription extends BranchNode
{
	protected $value = '';
	protected $style = '';
	public $remoteConstructor = 'Description';
	
	
	public function __construct( $text = '', $style = NULL ) {
		$this->value( $text );
		$this->textStyle( $style );
	}
}


class XULImage extends Node
{
	public $remoteConstructor = 'Image';
	public $initAttributes = array( 'src' );
	public $src = NULL;
	public $width = NULL;
	public $height = NULL;
	
	public function __construct( $src = NULL, $width = NULL, $height = NULL ) {
		if( $src !== NULL )
			$this->src( $src );
		if( $width !== NULL )
			$this->width( $width );
		if( $height !== NULL )
			$this->height( $height );
		
		parent::__construct();
	}
	
	public function attach() {
		$this->createRemoteObject( array( $this->hostWindow, $this->src, $this->width, $this->height ) );
		$this->insertElement();
	}
	
	public function src( $src = NULL ) {
		if( $src === NULL )
			return (isset($this->src) ? $this->src : NULL);
		
		$this->src = $src;
		
		if( $this->isRegistered )
			$this->callMethod( 'src', array($src) );
		
		return $this;
	}
}


class XULDirectImage extends Node
{
	public $remoteConstructor = 'DirectImage';
	public $width = NULL;
	public $height = NULL;
	private $imageType = 'jpg';
	public $imageStreamData = NULL;
	public $jpgQuality = 75;
	
	public function __construct( $width = NULL, $height = NULL ) {
		if( $width !== NULL )
			$this->width( $width );
		if( $height !== NULL )
			$this->height( $height );
		
		parent::__construct();
	}
	
	public function attach() {
		$this->createRemoteObject( array( $this->hostWindow, $this->width, $this->height ) );
		$this->insertElement();
	}
	
	public function setImageType( $type ) {
		switch ( $type ) {
			case 'jpg': $this->imageType = 'jpg'; break;
			case 'png': $this->imageType = 'png'; break;
			case 'gif': $this->imageType = 'gif'; break;
			default: $this->imageType = 'jpg';
		}
	}
	
	public function cacheStream( $image ) {
		$path = tempnam( '/tmp', 'WPIMG' );
		switch ( $this->imageType ) {
			case 'jpg': imagejpeg($image,$path,$this->jpgQuality); break;
			case 'png': imagepng($image,$path); break;
			case 'gif': imagegif($image,$path); break;
		}
		
		$this->imageStreamData = $path;
	}
	
	public function recycle() {
		$this->callMethod( 'recycle' );
	}
	
	public function getSrc() {
		return $this->callMethod( 'getSrc' );
	}
	
	public function outputStream() {
		switch ( $this->imageType ) {
			case 'jpg': $contentType = 'image/jpeg'; break;
			case 'png': $contentType = 'image/png'; break;
			case 'gif': $contentType = 'image/gif'; break;
		}
		
		if( class_exists('ApplicationProcess') )
			ApplicationProcess::Header( 'Content-Type: ' . $contentType );
		else
			header( 'Content-Type: ' . $contentType );
		
		readfile($this->imageStreamData);
		unlink($this->imageStreamData);
	}
}


class XULFileSystemImage extends Node
{
	public $remoteConstructor = 'FileSystemImage';
	public $width = NULL;
	public $height = NULL;
	public $imagePath = NULL;
	
	public function __construct( $path=NULL, $width = NULL, $height = NULL ) {
		$this->imagePath = $path;
		if( $width !== NULL )
			$this->width( $width );
		if( $height !== NULL )
			$this->height( $height );
		
		parent::__construct();
	}
	
	public function attach() {
		$this->createRemoteObject( array( $this->hostWindow, $this->width, $this->height ) );
		$this->insertElement();

		if ($this->imagePath !== NULL)
			$this->callMethod( 'load' );
	}

	public function src($path)
	{
		$this->imagePath = $path;
		$this->callMethod("load");
	}
	public function outputStream() {
		switch ( strtolower(substr($this->imagePath,-3)) ) {
			case 'jpg': $contentType = 'image/jpeg'; break;
			case 'png': $contentType = 'image/png'; break;
			case 'gif': $contentType = 'image/gif'; break;
			default: $contentType = 'application/octet-stream';
		}
		
		if( class_exists('ApplicationProcess') )
			ApplicationProcess::Header( 'Content-Type: ' . $contentType );
		else
			header( 'Content-Type: ' . $contentType );
		readfile($this->imagePath);
	}
}


