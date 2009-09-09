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
 * @package SiteFusion
 * @subpackage FileHandling
*/


class XULFilePicker extends Node
{
	public $remoteConstructor = 'FilePicker';
	public $file = NULL;
	public $files = NULL;
	public $returnCode = 'cancel';

	public function __construct( $title, $mode = 'open', $defaultString = '' ) {
		$this->title = $title;
		$this->defaultString = (string) $defaultString;
		
		if( $mode == 'open' || $mode == 'save' || $mode == 'getfolder' || $mode == 'openmultiple' )
			$this->mode = $mode;
		else
			Error::CallerFatal( 'Invalid mode' );
		
		$this->setEventHandler( 'yield', $this, 'yieldCollect' );
		
		parent::__construct();
	}
	
	public function attach() {
		$this->createRemoteObject( array( $this->hostWindow, $this->title, $this->mode, $this->defaultString ) );
	}
	
	public function detach() {
		$this->unRegister();
	}
	
	public function open() {
		$this->callMethod( 'open' );
	}

	public function addFilter($description, $value)
	{
		$this->callMethod( 'addFilter', array($description, $value) );
	}
	
	public function yieldCollect( $e, $returnCode, $path ) {
		$args = func_get_args();
		$e = array_shift($args);
		$this->returnCode = array_shift($args);
		
		if( $this->mode == 'openmultiple' ) {
			$this->files = $args;
			$this->file = NULL;
			
			$this->fireLocalEvent( 'closed', array( $this->returnCode, $this->files ) );
		}
		else {
			$this->file = (count($args) ? array_shift($args) : NULL);
			$this->files = NULL;

			$this->fireLocalEvent( 'closed', array( $this->returnCode, $this->file ) );
		}
		
		$this->removeNode();
	}
}


class FileUploader extends Node
{
	public $filePath;
	public $fileSize;
	public $fileName;
	public $destPath;
	public $destName;
	public $uploadedPath = NULL;
	public $remoteConstructor = 'FileUploader';
	
	public function __construct( $destPath = NULL, $destName = NULL ) {
		if( $destPath ) $this->destPath = $destPath;
		if( $destName ) $this->destName = $destName;
		
		parent::__construct();
	}
	
	public function attach() {
		$this->rootApplication->sendCommand(
			Comm::CreateRemoteObject( $this, array( $this->hostWindow ) )
		);
		
		$this->setEvent( 'cycle', MSG_SEND, $this, 'handleFileChunk' );
		$this->setEvent( 'cancelled', MSG_SEND, $this, 'handleError' );
		$this->setEvent( 'failed', MSG_SEND, $this, 'handleError' );
	}

	public function setDestination( $destPath, $destName ) {
		$this->destPath = $destPath;
		$this->destName = $destName;
	}
	
	public function startUpload( $remotePath ) {
		if( (!isset($this->destPath)) || (!isset($this->destName)) )
			Error::Fatal( 'No file path or name given. Use setDestination()' );
		
		$this->callMethod( 'startUpload', array( $remotePath ) );
	}
	
	public function cancelUpload() {
		$this->callMethod( 'cancelUpload' );
	}
	
	public function handleError( $e, $remotePath ) {
		$path = $this->destPath;
		$name = $this->destName;
		$fullpath = $path . '/' . $name;
		
		if( file_exists($fullpath) )
			@unlink( $fullpath );
	}
	
	public function handleFileChunk( $e, $filesize, $progress, $cycle, $data ) {
		$path = $this->destPath;
		$name = $this->destName;
		$fullpath = $path . '/' . $name;
		
		if( file_exists($fullpath) && $cycle == 1 )
			@unlink( $fullpath );
		
		$file = fopen( $fullpath, 'a' );
		fwrite( $file, $data );
		fclose( $file );
		
		$this->uploadedPath = $fullpath;
		
		$this->filePath = $path;
		$this->fileName = $name;
		clearstatcache();
		$this->fileSize = filesize( $this->uploadedPath );
	}
}


class FileDownloader extends Node
{
	public $filePath;
	public $destPath;
	public $remoteConstructor = 'FileDownloader';
	public $removeAfterTransfer = FALSE;
	
	public function __construct( $filePath = NULL, $destPath = NULL ) {
		if( $filePath ) $this->filePath = $filePath;
		if( $destPath ) $this->destPath = $destPath;
		
		parent::__construct();
	}
	
	public function attach() {
		$this->rootApplication->sendCommand(
			Comm::CreateRemoteObject( $this, array( $this->hostWindow ) )
		);
	}

	public function setPaths( $filePath, $destPath ) {
		$this->filePath = $filePath;
		$this->destPath = $destPath;
	}
	
	public function startDownload() {
		if( (!isset($this->destPath)) || (!isset($this->filePath)) )
			Error::Fatal( 'No file path or name given. Use setPaths()' );
		
		$this->callMethod( 'startDownload', array( $this->destPath ) );
	}
	
	public function outputStream() {
		if( class_exists('ApplicationProcess') )
			ApplicationProcess::Header( 'Content-Type: application/octet-stream' );
		else
			header( 'Content-Type: application/octet-stream' );
		readfile($this->filePath);
		
		if( $this->removeAfterTransfer )
			unlink( $this->filePath );
	}
}