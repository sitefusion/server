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
//   Tom Peeters <tom@thefrontdoor.nl> (original author)
//
// - - - - - - - - - - - - - - END LICENSE BLOCK - - - - - - - - - - - - -


/**
 * @package ClientComponents
 * @subpackage SFImageLib
*/


define("CXIMAGE_FORMAT_BMP", 1);
define("CXIMAGE_FORMAT_GIF", 2);
define("CXIMAGE_FORMAT_JPG", 3);
define("CXIMAGE_FORMAT_PNG", 4);
define("CXIMAGE_FORMAT_ICO", 5);
define("CXIMAGE_FORMAT_TIF", 6);
define("CXIMAGE_FORMAT_TGA", 7);
define("CXIMAGE_FORMAT_PCX", 8);
define("CXIMAGE_FORMAT_WBMP", 9);
define("CXIMAGE_FORMAT_WMF", 10);
define("CXIMAGE_FORMAT_JP2", 11);
define("CXIMAGE_FORMAT_JPC", 12);
define("CXIMAGE_FORMAT_PGX", 13);
define("CXIMAGE_FORMAT_PNM", 14);
define("CXIMAGE_FORMAT_RAS", 15);

define("IM_NEAREST_NEIGHBOUR", 1);
define("IM_BILINEAR", 2);
define("IM_BSPLINE", 3);
define("IM_BICUBIC", 4);
define("IM_BICUBIC2", 5);
define("IM_LANCZOS", 6);
define("IM_BOX", 7);
define("IM_HERMITE", 8);
define("IM_HAMMING", 9);
define("IM_SINC", 10);
define("IM_BLACKMAN", 11);
define("IM_BESSEL", 12);
define("IM_GAUSSIAN", 13);
define("IM_QUADRATIC", 14);
define("IM_MITCHELL", 15);
define("IM_CATROM", 16);


/**
 * This class controls the client component SFImageLib
*/

class SFImageLib extends Node
{
	public $remoteConstructor = 'SFImageLib';
	
	
	public function __construct( $url = NULL ) {
		$this->url = $url;
		$this->setEventHandler("stateChange", $this, "onStateChange");
		parent::__construct();
	}
	
	public function attach() {
		if (version_compare(ApplicationProcess::$ClientVersion, "2.0.0") < 0) {
			$this->createRemoteObject( array( $this->hostWindow ) );
			
	 		$this->setEvent( 'createTempPath', MSG_SEND, $this, 'createTempPath' );
	 		$this->setEvent( 'setImageDetails', MSG_SEND, $this, 'setImageDetails' );
	 		
			$this->setEventReflex( 'createTempPath', 'eventArguments.push( '.$this->js.'.getTempFilePath());' );
		}
		else throw(new SFException("SFImageLib is not compatible with SiteFusion 2.0.", ERR_REPORT_APP));
	}
	
	public function getTempFilePath($format)
	{
		switch ($format)
		{
			case CXIMAGE_FORMAT_JPG:
			$extension = ".jpg";
			break;
			case CXIMAGE_FORMAT_PNG:
			$extension = ".png";
			break;
			case CXIMAGE_FORMAT_GIF:
			$extension = ".gif";
			break;
			default:
			$extension = ".jpg";
			break;
		}
		$this->tempFilePath = $this->tempFilePath.$extension;
		$this->setProperty( 'temppath', $this->tempFilePath );
		return $this->tempFilePath;
	}
	
	public function cleanTemp()
	{
		if ($this->tempFilePath)
		{
			$this->callMethod( 'cleanTemp' );
		}
	}
	
	public function createTempPath ($e, $temppath)
	{
		$this->tempFilePath = $temppath;
	}
	
	public function loadImage($path)
	{
		$this->path = $path;
		$this->height = NULL;
		$this->width = NULL;
		$this->format = NULL;
		$this->tempFilePath = NULL;
		$this->setProperty( 'path', $this->path );
		$this->callMethod( 'loadImage', array( $path  ) );
	}
	
	public function saveImage($path, $type)
	{
		$this->callMethod( 'saveImage', array( $path, $type  ) );
	}
	
	public function resample($width, $height, $mode)
	{
		$this->callMethod( 'resample', array( $width, $height, $mode  ) );
	}
	
	public function setImageDetails($e, $localevent, $width, $height, $format)
	{
		$this->width = $width;
		$this->height = $height;
		$this->format = $format;
		
		$this->fireLocalEvent($localevent);
	}
	
	public function getWidth()
	{
		return $this->width;
	}
	
	public function getHeight()
	{
		return $this->height;
	}
	
	public function getFormat()
	{
		return $this->format;
	}
	
	public function getPath()
	{
		return $this->path;
	}
	
}