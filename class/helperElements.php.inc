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
 * @subpackage HelperElements
*/


/**
 * Grippy element
 * 
 * @link https://developer.mozilla.org/en/XUL/grippy
*/

class XULGrippy extends Node
{
	public $remoteConstructor = 'Grippy';
}


/**
 * Arrowscrollbox element
 * 
 * @link https://developer.mozilla.org/en/XUL/arrowscrollbox
*/

class XULArrowScrollBox extends BranchNode
{
	public $remoteConstructor = 'ArrowScrollBox';
	public $orient = 'vertical';
	
	
	/**
	 * Dynamic Constructor
	 *
	 * @param int		$flex
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_int($args[0]) )
				$this->flex( array_shift($args) );
						
			parent::__construct( $args );
		}
		else parent::__construct();
	}
}


/**
 * Dropmarker element
 * 
 * @link https://developer.mozilla.org/en/XUL/dropmarker
*/

class XULDropmarker extends Node
{
	public $remoteConstructor = 'Dropmarker';
}


/**
 * Sound element
 * 
*/
class XULSound extends Node
{
	public $remoteConstructor = 'Sound';
	public $path;
	
	private $transferStarted = FALSE;
	private $transferSize = NULL;
	
	/**
	 * Dynamic Constructor
	 *
	 * @param string	$url
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_string($args[0]) )
				$this->load( array_shift($args) );
						
			parent::__construct( $args );
		}
		else parent::__construct();
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
		if (file_exists($this->path)) {
			$this->transferSize = filesize($this->path);
			
			switch ( strtolower(substr($this->path,-3)) ) {
				case 'wav': $contentType = 'audio/x-wav'; break;
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
		
		readfile( $this->path );
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
	
	public function attach() {
		parent::attach();
		if ($this->path)
			$this->load($this->path);
	}
	
	public function load($path) {
		if (file_exists($path))
			$this->path = $path;
		elseif (file_exists(ApplicationProcess::GetOption("sitefusionPath") . $path))
			$this->path = ApplicationProcess::GetOption("sitefusionPath") . $path;
		else throw new SFException( "Audio file $path not found!", ERR_REPORT_APP );
		
		if ($this->isRegistered)
			$this->callMethod( 'load' );
	}
	
	public function play() {
		$this->callMethod( 'play');
	}
	
	public function playSystemSound($soundAlias) {
		$this->callMethod( 'playSystemSound', array( $soundAlias  ) );
	}
	
	public function beep() {
		$this->callMethod( 'beep' );
	}
}