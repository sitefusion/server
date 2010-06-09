<?php
// - - - - - - - - - - - - - BEGIN LICENSE BLOCK - - - - - - - - - - - - -
// Version: MPL 1.1/GPL 2.0/LGPL 2.1
//
// The contents of this file are subject to the Mozilla Public License Version
// 1.1 (the "License"); you may not use this file except in compliance with
// the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/
//
// Software distributed under the License is distributed on an "AS IS" basis,
// WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
// for the specific language governing rights and limitations under the
// License.
//
// The Original Code is sitefusion.sourceforge.net code.
//
// The Initial Developer of the Original Code is
// FrontDoor Media Group.
// Portions created by the Initial Developer are Copyright (C) 2009
// the Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Nikki Auburger <nikki@thefrontdoor.nl> (original author)
//   Tom Peeters <tom@thefrontdoor.nl>
//
// - - - - - - - - - - - - - - END LICENSE BLOCK - - - - - - - - - - - - -


/**
 * @package API
 * @subpackage TextAndImages
*/


/**
 * Label element
 * 
 * @link https://developer.mozilla.org/en/XUL/label
*/

class XULLabel extends Node
{
	public $remoteConstructor = 'Label';
	
	public function __construct( $text = '', $style = NULL ) {
		$this->value( $text );
		$this->textStyle( $style );
	}
}

/**
 * Description element
 * 
 * @link https://developer.mozilla.org/en/XUL/description
*/

class XULDescription extends BranchNode
{
	public $remoteConstructor = 'Description';
	
	
	/**
	 * Dynamic Constructor
	 *
	 * @param string	$value
	 * @param string	$textStyle
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_string($args[0]) )
				$this->value( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->textStyle( array_shift($args) );
			
			parent::__construct( $args );
		}
		else parent::__construct();
	}
}


/**
 * Image element
 * 
 * @link https://developer.mozilla.org/en/XUL/image
*/

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


/**
 * Direct GD image display element
 * 
 * Supply this element with an image type ('jpg', 'gif' or 'png') through the method setImageType($type)
 * and call cacheStream( $gdImage ) in which $gdImage is a valid GD image resource. Call recycle() to
 * show the cached image stream in the client.
 * 
 * @link https://developer.mozilla.org/en/XUL/image
*/

class XULDirectImage extends Node
{
	public $remoteConstructor = 'DirectImage';
	public $width = NULL;
	public $height = NULL;
	private $imageType = 'jpg';
	private $tempImagePath = NULL;
	public $jpgQuality = 75;
	
	private $transferStarted = FALSE;
	private $transferSize = NULL;
	
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
		
		$this->tempImagePath = $path;
	}
	
	public function recycle() {
		$this->callMethod( 'recycle' );
	}
	
	
	/**
	 * [INTERNAL FUNCTION]
	 * This function is called by the daemon to start a transfer. It returns an array
	 * of two elements, first is a string with the MIME content-type of the data, and
	 * second is a integer number containing the size in bytes of the transfer.
	 * 
	 * @return array	Transfer descriptives
	*/
	
	public function transferStart() {
		$this->transferStarted = TRUE;
		$this->transferSize = @filesize($this->tempImagePath);
		
		switch ( $this->imageType ) {
			case 'jpg': $contentType = 'image/jpeg'; break;
			case 'png': $contentType = 'image/png'; break;
			case 'gif': $contentType = 'image/gif'; break;
		}
		
		return array( 'application/octet-stream', $this->transferSize );
	}
	
	
	/**
	 * [INTERNAL FUNCTION]
	 * This function is called by the daemon to get (a piece of) the data of a transfer.
	 * It should output this data to the output buffer, which will be intercepted
	 * by the daemon.
	*/
	
	public function transferGetData() {
		if( !$this->transferStarted )
			throw new SFException( "File transfer not started!" );
		
		readfile( $this->tempImagePath );
	}
	
	
	/**
	 * [INTERNAL FUNCTION]
	 * This function is called by the daemon when all data has been transferred.
	 * It is used to close open filehandles and clean up.
	*/
	
	public function transferEnd() {
		$this->transferStarted = FALSE;
		$this->transferSize = NULL;
		
		if (file_exists($this->tempImagePath))
			unlink( $this->tempImagePath );
		$this->tempImagePath = NULL;
	}
}


/**
 * Direct filesystem image display element
 * 
 * This object behaves like a normal image, but accepts a filepath on the local (server) filesystem
 * 
 * @link https://developer.mozilla.org/en/XUL/image
*/

class XULFileSystemImage extends Node
{
	public $remoteConstructor = 'FileSystemImage';
	public $width = NULL;
	public $height = NULL;
	public $imagePath = NULL;
	
	private $transferStarted = FALSE;
	private $transferSize = NULL;

	public function __construct( $path = NULL, $width = NULL, $height = NULL ) {
		if( $path !== NULL )
			$this->src( $path );
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

	public function src($path) {
		$this->imagePath = $path;
		
		if( $this->isRegistered )
			$this->callMethod("load");
	}
	
	
	/**
	 * [INTERNAL FUNCTION]
	 * This function is called by the daemon to start a transfer. It returns an array
	 * of two elements, first is a string with the MIME content-type of the data, and
	 * second is a integer number containing the size in bytes of the transfer.
	 * 
	 * @return array	Transfer descriptives
	*/
	
	public function transferStart() {
		$this->transferStarted = TRUE;
		if (file_exists($this->imagePath)) {
			$this->transferSize = filesize($this->imagePath);
			
			switch ( strtolower(substr($this->imagePath,-3)) ) {
				case 'jpg': $contentType = 'image/jpeg'; break;
				case 'png': $contentType = 'image/png'; break;
				case 'gif': $contentType = 'image/gif'; break;
				default: $contentType = 'application/octet-stream';
			}
			
			return array( 'application/octet-stream', $this->transferSize );
		}
		else return FALSE;
	}
	
	
	/**
	 * [INTERNAL FUNCTION]
	 * This function is called by the daemon to get (a piece of) data of a transfer.
	 * It should output this data to the output buffer, which will be intercepted
	 * by the daemon.
	*/
	
	public function transferGetData() {
		if( !$this->transferStarted )
			throw new SFException( "File transfer not started!", ERR_REPORT_APP );
		
		readfile( $this->imagePath );
	}
	
	
	/**
	 * [INTERNAL FUNCTION]
	 * This function is called by the daemon when all data has been transferred.
	 * It is used to close open filehandles and clean up.
	*/
	
	public function transferEnd() {
		$this->transferStarted = FALSE;
		$this->transferSize = NULL;
	}
}


