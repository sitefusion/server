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
 * @subpackage FileHandling
*/


/**
 * OS-native filepicker dialog
 * 
 * Opening modes can be one of 'open', 'save', 'getfolder' or 'openmultiple'. This node needs to be
 * added to a window as a child first. Then the open() method can be called, after which the dialog
 * opens. You can set an event handler for the local event 'closed', at which time the property
 * XULFilePicker->returnCode will contain one of the strings 'ok', 'cancel' or 'replace', indicating
 * the action desired by the user. The property XULFilePicker->file will contain the selected file or
 * folder if the dialog was of mode 'save', 'open' or 'getfolder'. In the case of an 'openmultiple'
 * picker the property XULFilePicker->files will contain an array of the selected paths.
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
			throw new SFException( 'Invalid mode', ERR_REPORT_APP );
		
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


/**
 * Background file uploader
 * 
 * This class creates an object that uploads a file from the client to the server.
 * Construct it with a destination path and name (on the server). Then call the 
 * method startUpload( $path ) where $path is the location of the file on the client
 * side. This path can be retrieved with f.e. XULFilePicker. This control fires the
 * event 'finished' when the upload was successful, 'failed' when it went wrong or 
 * 'cancelled' when the upload was cancelled through the cancelUpload() method. It
 * also fires the event 'cycle' for every chunk received, allowing you to keep track
 * of progress.
 * 
 * @see XULFilePicker
*/

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
			throw new SFException( 'No file path or name given. Use setDestination()', ERR_REPORT_APP );
		
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
		fwrite( $file, base64_decode($data) );
		fclose( $file );
		
		$this->uploadedPath = $fullpath;
		
		$this->filePath = $path;
		$this->fileName = $name;
		clearstatcache();
		$this->fileSize = filesize( $this->uploadedPath );
	}
}


/**
 * Background file downloader
 * 
 * This class constructs a node that downloads a file from the server to the client in 
 * the background. Construct it with the local (server) path as the first argument and
 * the remote (client) path as the second argument, and call startDownload(). This control
 * fires a 'started' event when the download starts, a 'cycle' event periodically with progress
 * information, a 'finished' event when the download is finished and a 'cancelled' event when
 * the download is cancelled.
 * 
 * @see XULFilePicker
*/

class FileDownloader extends Node
{
	public $filePath;
	public $destPath;
	public $remoteConstructor = 'FileDownloader';
	public $removeAfterTransfer = FALSE;
	
	private $transferStarted = FALSE;
	private $transferSize = NULL;
	private $transferFile = NULL;
	
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
	
	public function cancelDownload() {
		$this->callMethod( 'cancelDownload' );
	}
	
	public function startDownload() {
		if( (!isset($this->destPath)) || (!isset($this->filePath)) )
			throw new SFException( 'No file path or name given. Use setPaths()', ERR_REPORT_APP );
		
		$this->callMethod( 'startDownload', array( $this->destPath ) );
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
		$this->transferSize = filesize($this->filePath);
		$this->transferFile = fopen( $this->filePath, 'r' );
		
		return array( 'application/octet-stream', $this->transferSize );
	}
	
	
	/**
	 * [INTERNAL FUNCTION]
	 * This function is called by the daemon to get (a piece of) data of a transfer.
	 * It should output this data to the output buffer, which will be intercepted
	 * by the daemon.
	*/
	
	public function transferGetData() {
		if( !$this->transferStarted )
			throw new SFException( "File transfer not started!" );
		
		echo fread( $this->transferFile, 8192 );
	}
	
	
	/**
	 * [INTERNAL FUNCTION]
	 * This function is called by the daemon when all data has been transferred.
	 * It is used to close open filehandles and clean up.
	*/
	
	public function transferEnd() {
		fclose( $this->transferFile );
		$this->transferStarted = FALSE;
		$this->transferSize = NULL;
		$this->transferFile = NULL;
		
		if( $this->removeAfterTransfer )
			unlink( $this->filePath );
	}
}



/**
 * 
*/

class ClientFileService extends Node
{
	public $remoteConstructor = 'FileService';
	private $handlers = array();
	private $fileMonitors = array();
	
	public function __construct() {
		parent::__construct();
	}
	
	public function attach() {
		$this->createRemoteObject( array( $this->hostWindow ) );
		$this->setEvent( 'result', MSG_SEND, $this, 'onResult' );
	}
	
	public function detach() {
		$this->callMethod( 'cancelAllMonitors' );
		$this->unRegister();
	}
	
	public function getDirectory( $path, $handlerObj, $handlerMethod ) {
		$this->handlers[] = array( $handlerObj, $handlerMethod );
		$this->callMethod( 'getDirectory', array( $path ) );
	}
	
	public function getSpecialDirectory( $id, $handlerObj, $handlerMethod ) {
		$this->handlers[] = array( $handlerObj, $handlerMethod );
		$this->callMethod( 'getSpecialDirectory', array( $id ) );
	}
	
	public function createDirectory( $path, $handlerObj = NULL, $handlerMethod = NULL ) {
		$this->handlers[] = ($handlerObj&&$handlerMethod ? array( $handlerObj, $handlerMethod ) : NULL);
		$this->callMethod( 'createDirectory', array( $path ) );
	}
	
	public function writeFile( $localPath, $clientPath, $handlerObj = NULL, $handlerMethod = NULL ) {
		$this->hostWindow->addChild( $fdn = new FileDownloader( $localPath, $clientPath ) );
		if( $handlerObj && $handlerMethod ) {
			$fdn->setEventHandler( 'finished', $handlerObj, $handlerMethod );
			$fdn->setEventHandler( 'failed', $handlerObj, $handlerMethod );
			$fdn->setEventHandler( 'cancelled', $handlerObj, $handlerMethod );
		}
		$fdn->setEventHandler( 'finished', $this, 'onDownloaderFinished' );
		$fdn->setEventHandler( 'failed', $this, 'onDownloaderFinished' );
		$fdn->setEventHandler( 'cancelled', $this, 'onDownloaderFinished' );
		$fdn->startDownload();
		
		return $fdn;
	}
	
	public function onDownloaderFinished( $event ) {
		$event->sourceObject->removeNode();
	}
	
	public function readFile( $clientPath, $localPath, $handlerObj = NULL, $handlerMethod = NULL ) {
		$this->hostWindow->addChild( $fup = new FileUploader( dirname($localPath), basename($localPath) ) );
		if( $handlerObj && $handlerMethod ) {
			$fup->setEventHandler( 'finished', $handlerObj, $handlerMethod );
			$fup->setEventHandler( 'failed', $handlerObj, $handlerMethod );
			$fup->setEventHandler( 'cancelled', $handlerObj, $handlerMethod );
		}
		$fup->setEventHandler( 'finished', $this, 'onUploaderFinished' );
		$fup->setEventHandler( 'failed', $this, 'onUploaderFinished' );
		$fup->setEventHandler( 'cancelled', $this, 'onUploaderFinished' );
		$fup->startUpload( $clientPath );
		
		return $fup;
	}
	
	public function onUploaderFinished( $event ) {
		$event->sourceObject->removeNode();
	}
	
	public function removeDirectory( $clientPath, $recursive, $handlerObj = NULL, $handlerMethod = NULL ) {
		$this->handlers[] = ($handlerObj&&$handlerMethod ? array( $handlerObj, $handlerMethod ) : NULL);
		$this->callMethod( 'removeDirectory', array( $clientPath, (bool)$recursive ) );
	}
	
	public function removeFile( $clientPath, $handlerObj = NULL, $handlerMethod = NULL ) {
		$this->handlers[] = ($handlerObj&&$handlerMethod ? array( $handlerObj, $handlerMethod ) : NULL);
		$this->callMethod( 'removeFile', array( $clientPath ) );
	}
	
	public function monitorFile( $clientPath, $handlerObj, $handlerMethod ) {
		if( isset($this->fileMonitors[$clientPath]) )
			throw new SFException( "Path $clientPath is already being monitored!", ERR_REPORT_APP );
		
		$this->fileMonitors[$clientPath] = array( $handlerObj, $handlerMethod );
		$this->callMethod( 'monitorFile', array( $clientPath ) );
	}
	
	public function onResult() {
		$args = func_get_args();
		$event = array_shift( $args );
		$op = array_shift( $args );
		
		switch ( $op ) {
			case 'list':
				$status = array_shift( $args );
				$path = array_shift( $args );
				$files = array();
				if( $status ) {
					$dirFile = $this->fileFromResult( $path, $args[0] );
					$dirFile->entries = array();
					foreach ( $args[1] as $file ) {
						$cf = $this->fileFromResult( $path.$this->getDirSeparator().$file[0], $file );
						$dirFile->entries[$file[0]] = $cf;
					}
				}
				else $dirFile = NULL;
				
				call_user_func_array( array_shift($this->handlers), array($this,$status,$dirFile) );
			break;
			
			case 'createDirectory':
				$status = array_shift( $args );
				$path = array_shift( $args );
				if( $status ) {
					$file = array_shift( $args );
					$dirFile = $this->fileFromResult( $path, $file );
					$dirFile->entries = array();
				}
				else $dirFile = NULL;
				
				if( ($handler = array_shift($this->handlers)) !== NULL )
					call_user_func_array( $handler, array($this,$status,$dirFile) );
			break;
			
			case 'removeDirectory':
			case 'removeFile':
				$status = array_shift( $args );
				$path = array_shift( $args );
				
				if( ($handler = array_shift($this->handlers)) !== NULL )
					call_user_func_array( $handler, array($this,$status,$path) );
			break;
			
			case 'fileChanged':
				list($path,$exists,$modificationTime,$size) = $args;
				call_user_func_array( $this->fileMonitors[$path], array($this,$path,$exists,$modificationTime,$size) );
			break;
		}
	}
	
	private function fileFromResult( $path, $file ) {
		$cf = new ClientFile();
		$cf->fileService = $this;
		$cf->path = $path;
		$cf->name = $file[0];
		$cf->isDirectory = $file[1];
		$cf->isReadable = $file[2];
		$cf->isWritable = $file[3];
		$cf->isExecutable = $file[4];
		$cf->isHidden = $file[5];
		$cf->size = $file[6];
		return $cf;
	}
	
	public function getDirSeparator() {
		if( stripos(ApplicationProcess::$PlatformInfo['platform'],'win') != -1 )
			return "\\";
		else
			return "/";
	}
}


class ClientFile
{
	public $fileService;
	public $path;
	public $name;
	public $isDirectory;
	public $isReadable;
	public $isWritable;
	public $isExecutable;
	public $isHidden;
	public $size;
	public $entries = NULL;
	public $exists = TRUE;
	
	public function append( $fileName ) {
		$file = new self;
		$file->fileService = $this->fileService;
		$file->path = $this->path.$this->fileService->getDirSeparator().$fileName;
		$file->name = $fileName;
		$file->exists = FALSE;
		return $file;
	}
	
	public function createDirectory( $handlerObj = NULL, $handlerMethod = NULL ) {
		$this->fileService->createDirectory( $this->path, $handlerObj, $handlerMethod );
	}
	
	public function getDirectory( $handlerObj, $handlerMethod ) {
		$this->fileService->getDirectory( $this->path, $handlerObj, $handlerMethod );
	}
	
	public function writeFile( $localPath, $handlerObj = NULL, $handlerMethod = NULL ) {
		return $this->fileService->writeFile( $localPath, $this->path, $handlerObj, $handlerMethod );
	}
	
	public function readFile( $localPath, $handlerObj = NULL, $handlerMethod = NULL ) {
		return $this->fileService->readFile( $this->path, $localPath, $handlerObj, $handlerMethod );
	}
	
	public function removeDirectory( $recursive = FALSE, $handlerObj = NULL, $handlerMethod = NULL ) {
		$this->fileService->removeDirectory( $this->path, $recursive, $handlerObj, $handlerMethod );
	}
	
	public function removeFile( $handlerObj = NULL, $handlerMethod = NULL ) {
		$this->fileService->removeFile( $this->path, $handlerObj, $handlerMethod );
	}
	
	public function monitorFile( $handlerObj, $handlerMethod ) {
		$this->fileService->monitorFile( $this->path, $handlerObj, $handlerMethod );
	}
}

