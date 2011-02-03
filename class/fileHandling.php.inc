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
	public $selectedFilter = FALSE;
	
	/**
	 * Constructor
	 * 
	 * @param string	$title			Title for the file picker dialog
	 * @param string	$mode			Type of file picker ('open' = open single file, 'save' = save a file, 'getfolder' = select a folder, 'openmultiple' = open multiple files
	 * @param string	$defaultString	Default value (file/folder path)
	 * @param string	$defaultExtension	Default extension value. Needs to be set in order to enable reading the filter extensions.
	*/
	
	public function __construct( $title, $mode = 'open', $defaultString = '', $defaultExtension = '' ) {
		$this->title = $title;
		$this->defaultString = (string) $defaultString;
		$this->defaultExtension = $defaultExtension;
		
		if( $mode == 'open' || $mode == 'save' || $mode == 'getfolder' || $mode == 'openmultiple' )
			$this->mode = $mode;
		else
			throw new SFException( 'Invalid mode', ERR_REPORT_APP );
		
		$this->setEventHandler( 'yield', $this, 'yieldCollect' );
		
		parent::__construct();
	}
	
	public function attach() {
		$this->createRemoteObject( array( $this->hostWindow, $this->title, $this->mode, $this->defaultString, $this->defaultExtension ) );
	}
	
	public function detach() {
		$this->unRegister();
	}
	
	
	/**
	 * Opens the file picker
	*/
	
	public function open() {
		$this->callMethod( 'open' );
	}

	
	/**
	 * Adds a file type filter
	 * 
	 * Example: $filePicker->addFilter("Images", "*.jpg; *.gif; *.png; *.");
	 *
	 * @param string	$description	Description of the filter
	 * @param string	$value			Value for the filter, semicolon separated
	*/
	
	public function addFilter($description, $value) {
		$this->callMethod( 'addFilter', array($description, $value) );
	}
	
	
	/**
	 * Event handler for the yield event
	*/
	
	public function yieldCollect( $e, $returnCode, $path, $selectedFilter ) {
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
			$this->selectedFilter = array();
			list($this->selectedFilter['filterName'],$this->selectedFilter['filterContents']) = (count($args) ? array_shift($args) : NULL);
			$this->selectedFilter['filterContents'] = explode(";",$this->selectedFilter['filterContents']);
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
 * Construct it with a source (client) path and destination path. Then call the 
 * method startUpload(). This control fires the
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
	public $clientPath;
	public $uploadedPath = NULL;
	public $remoteConstructor = 'FileUploader';
	
	
	/**
	 * Constructor
	 *
	 * @param string	$clientPath		Source file path on the client side
	 * @param string	$destPath		Destination file path on the server side
	*/
	
	public function __construct( $clientPath = NULL, $destPath = NULL ) {
		if( $clientPath ) $this->clientPath = $clientPath;
		if( $destPath ) $this->destPath = $destPath;
		
		parent::__construct();
	}
	
	public function attach() {
		$this->createRemoteObject( array( $this->hostWindow ) );
		
		$this->setEvent( 'cycle', MSG_SEND, $this, 'handleFileChunk' );
		$this->setEvent( 'cancelled', MSG_SEND, $this, 'handleError' );
		$this->setEvent( 'failed', MSG_SEND, $this, 'handleError' );
		$this->setEventType( 'finished', MSG_SEND );
	}
	
	public function detach() {
		$this->unRegister();
	}
	
	
	/**
	 * Set the source and destination paths
	 *
	 * @param string	$clientPath		Source file path on the client side
	 * @param string	$destPath		Destination file path on the server side
	*/
	
	public function setPaths( $clientPath, $destPath ) {
		$this->clientPath = $clientPath;
		$this->destPath = $destPath;
	}
	
	
	/**
	 * Start the upload
	 * 
	 * Starts the uploader which fires the 'started' event beforehand
	*/
	
	public function startUpload() {
		if( (!isset($this->destPath)) || (!isset($this->clientPath)) )
			throw new SFException( 'No source or destination file path given. Use setPaths()', ERR_REPORT_APP );
		
		$this->callMethod( 'startUpload', array( $this->clientPath ) );
	}
	
	
	/**
	 * Cancel a running upload
	 * 
	 * Cancels the uploader which fires the 'cancelled' event
	*/
	
	public function cancelUpload() {
		$this->callMethod( 'cancelUpload' );
	}
	
	
	/**
	 * Event handler for the 'failed' and 'cancelled' events
	*/
	
	public function handleError( $e, $remotePath ) {
		$path = $this->destPath;
		
		if( file_exists($path) )
			@unlink( $path );
	}
	
	
	/**
	 * Event handler for the 'cycle' event
	*/
	
	public function handleFileChunk( $e, $filesize, $progress, $cycle, $data ) {
		$path = $this->destPath;
		
		if( file_exists($path) && $cycle == 1 )
			@unlink( $path );
		
		$file = fopen( $path, 'a' );
		fwrite( $file, base64_decode($data) );
		fclose( $file );
		
		$this->uploadedPath = $path;
		
		$this->filePath = dirname($path);
		$this->fileName = basename($path);
		clearstatcache();
		$this->fileSize = filesize( $path );
	}
}



/**
 * Background file-to-memory uploader
 * 
 * This class creates an object that uploads a file from the client to the server memory.
 * Construct it with a source (client) path. Then call the method startUpload(). This control fires the
 * event 'finished' when the upload was successful, 'failed' when it went wrong or 
 * 'cancelled' when the upload was cancelled through the cancelUpload() method. It
 * also fires the event 'cycle' for every chunk received, allowing you to keep track
 * of progress. The resulting file contents are placed in the FileToStringUploader::$data property.
 * 
 * @see XULFilePicker
*/

class FileToStringUploader extends Node
{
	public $data = NULL;
	public $bufferSize = NULL;
	public $fileSize = NULL;
	public $clientPath;
	public $remoteConstructor = 'FileUploader';
	
	
	/**
	 * Constructor
	 *
	 * @param string	$clientPath		Source file path on the client side
	*/
	
	public function __construct( $clientPath = NULL ) {
		if( $clientPath ) $this->clientPath = $clientPath;
		
		parent::__construct();
	}
	
	public function attach() {
		$this->createRemoteObject( array( $this->hostWindow ) );
		
		$this->setEvent( 'cycle', MSG_SEND, $this, 'handleFileChunk' );
		$this->setEvent( 'cancelled', MSG_SEND, $this, 'handleError' );
		$this->setEvent( 'failed', MSG_SEND, $this, 'handleError' );
		$this->setEventType( 'finished', MSG_SEND );
	}

	public function detach() {
		$this->unRegister();
	}
	
	
	/**
	 * Set the source path
	 *
	 * @param string	$clientPath		Source file path on the client side
	*/
	
	public function setSourcePath( $clientPath ) {
		$this->clientPath = $clientPath;
	}
	
	
	/**
	 * Start the upload
	 * 
	 * Starts the uploader which fires the 'started' event beforehand
	*/
	
	public function startUpload() {
		if( (!isset($this->clientPath)) )
			throw new SFException( 'No source path given. Use setSourcePath()', ERR_REPORT_APP );
		
		$this->callMethod( 'startUpload', array( $this->clientPath ) );
	}
	
	
	/**
	 * Cancel a running upload
	 * 
	 * Cancels the uploader which fires the 'cancelled' event
	*/
	
	public function cancelUpload() {
		$this->callMethod( 'cancelUpload' );
	}
	
	
	/**
	 * Event handler for the 'failed' and 'cancelled' events
	*/
	
	public function handleError( $e, $remotePath ) {
		$this->data = NULL;
		$this->bufferSize = NULL;
		$this->fileSize = NULL;
	}
	
	
	/**
	 * Event handler for the 'cycle' event
	*/
	
	public function handleFileChunk( $e, $filesize, $progress, $cycle, $data ) {
		if( $this->data === NULL )
			$this->data = '';
		
		$this->data .= base64_decode($data);
		$this->bufferSize = strlen( $this->data );
		$this->fileSize = $filesize;
	}
}



/**
 * Background file-to-stream uploader
 * 
 * This class creates an object that uploads a file from the client to a stream on the server.
 * Construct it with a source (client) path and a stream resource. Then call the method startUpload(). This control fires the
 * event 'finished' when the upload was successful, 'failed' when it went wrong or 
 * 'cancelled' when the upload was cancelled through the cancelUpload() method. It
 * also fires the event 'cycle' for every chunk received, allowing you to keep track
 * of progress.
 *
 * @see XULFilePicker
*/

class FileToStreamUploader extends Node
{
	public $stream = NULL;
	public $fileSize = NULL;
	public $clientPath;
	public $remoteConstructor = 'FileUploader';
	
	
	/**
	 * Constructor
	 *
	 * @param string	$clientPath		Source file path on the client side
	 * @param resource	$stream			Stream to write data to
	*/
	
	public function __construct( $clientPath = NULL, $stream = NULL ) {
		if( $clientPath ) $this->clientPath = $clientPath;
		if( $stream ) $this->setStream( $stream );
		
		parent::__construct();
	}
	
	public function attach() {
		$this->createRemoteObject( array( $this->hostWindow ) );
		
		$this->setEvent( 'cycle', MSG_SEND, $this, 'handleFileChunk' );
		$this->setEvent( 'cancelled', MSG_SEND, $this, 'handleError' );
		$this->setEvent( 'failed', MSG_SEND, $this, 'handleError' );
		$this->setEventType( 'finished', MSG_SEND );
	}

	public function detach() {
		$this->unRegister();
	}
	
	
	/**
	 * Set the source path
	 *
	 * @param string	$clientPath		Source file path on the client side
	*/
	
	public function setSourcePath( $clientPath ) {
		$this->clientPath = $clientPath;
	}
	
	
	/**
	 * Set the stream to write to
	 *
	 * @param resource	$stream		Stream to write data to
	*/
	
	public function setStream( $stream ) {
		if( ! is_resource($stream) )
			throw new SFException( "setStream(): not a stream resource", ERR_REPORT_APP );
		
		$this->stream = $stream;
	}
	
	
	/**
	 * Start the upload
	 * 
	 * Starts the uploader which fires the 'started' event beforehand
	*/
	
	public function startUpload() {
		if( (!isset($this->clientPath)) || (!is_resource($this->stream)) )
			throw new SFException( 'No source path given or no stream set. Use setSourcePath() and setStream()', ERR_REPORT_APP );
		
		$this->callMethod( 'startUpload', array( $this->clientPath ) );
	}
	
	
	/**
	 * Cancel a running upload
	 * 
	 * Cancels the uploader which fires the 'cancelled' event
	*/
	
	public function cancelUpload() {
		$this->callMethod( 'cancelUpload' );
	}
	
	
	/**
	 * Event handler for the 'failed' and 'cancelled' events
	*/
	
	public function handleError( $e, $remotePath ) {
		$this->fileSize = NULL;
	}
	
	
	/**
	 * Event handler for the 'cycle' event
	*/
	
	public function handleFileChunk( $e, $filesize, $progress, $cycle, $data ) {
		fwrite( $this->stream, base64_decode($data) );
		$this->fileSize = $filesize;
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
	
	
	/**
	 * Constructor
	 *
	 * @param string	$filePath		Source file path on the server side
	 * @param string	$destPath		Destination file path on the client side
	*/
	
	public function __construct( $filePath = NULL, $destPath = NULL ) {
		if( $filePath ) $this->filePath = $filePath;
		if( $destPath ) $this->destPath = $destPath;
		
		parent::__construct();
	}
	
	public function attach() {
		$this->createRemoteObject( array( $this->hostWindow ) );
		$this->setEventType( 'finished', MSG_SEND );
		$this->setEventType( 'cancelled', MSG_SEND );
		$this->setEventType( 'failed', MSG_SEND );
	}
	
	public function detach() {
		$this->unRegister();
	}
	
	
	/**
	 * Set the source and destination paths
	 *
	 * @param string	$filePath		Source file path on the server side
	 * @param string	$destPath		Destination file path on the client side
	*/
	
	public function setPaths( $filePath, $destPath ) {
		$this->filePath = $filePath;
		$this->destPath = $destPath;
	}
	
	
	/**
	 * Cancel a running download
	 * 
	 * Cancels the downloader which fires the 'cancelled' event
	*/
	
	public function cancelDownload() {
		$this->callMethod( 'cancelDownload' );
	}
	
	
	/**
	 * Start the download
	 * 
	 * Starts the downloader which fires the 'started' event beforehand
	*/
	
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
 * Background file-from-memory downloader
 * 
 * This class constructs a node that downloads from server memory to a file on the client in 
 * the background. Construct it with the remote (client) destination path as the first argument and
 * the data string as the second, then call startDownload(). This control
 * fires a 'started' event when the download starts, a 'cycle' event periodically with progress
 * information, a 'finished' event when the download is finished and a 'cancelled' event when
 * the download is cancelled.
 * 
 * @see XULFilePicker
*/

class FileFromStringDownloader extends Node
{
	public $destPath;
	public $data = NULL;
	public $dataPos = NULL;
	public $remoteConstructor = 'FileDownloader';
	
	private $transferStarted = FALSE;
	private $transferSize = NULL;
	
	
	/**
	 * Constructor
	 *
	 * @param string	$destPath		Destination file path on the client side
	 * @param string	$data			Data to store in the file
	*/
	
	public function __construct( $destPath = NULL, $data = NULL ) {
		if( $destPath !== NULL ) $this->destPath = $destPath;
		if( $data !== NULL ) $this->data = (string) $data;
		
		parent::__construct();
	}
	
	public function attach() {
		$this->createRemoteObject( array( $this->hostWindow ) );
		$this->setEventType( 'finished', MSG_SEND );
		$this->setEventType( 'cancelled', MSG_SEND );
		$this->setEventType( 'failed', MSG_SEND );
	}
	
	public function detach() {
		$this->unRegister();
	}
	
	
	/**
	 * Set the destination path
	 *
	 * @param string	$destPath		Destination file path on the client side
	*/
	
	public function setDestinationPath( $destPath ) {
		$this->destPath = $destPath;
	}
	
	
	/**
	 * Set the data to store in the file
	 *
	 * @param string	$data		Data to store in the file
	*/
	
	public function setData( $data ) {
		$this->data = (string)$data;
	}
	
	
	/**
	 * Cancel a running download
	 * 
	 * Cancels the downloader which fires the 'cancelled' event
	*/
	
	public function cancelDownload() {
		$this->callMethod( 'cancelDownload' );
	}
	
	
	/**
	 * Start the download
	 * 
	 * Starts the downloader which fires the 'started' event beforehand
	*/
	
	public function startDownload() {
		if( (!isset($this->destPath)) || $this->data === NULL )
			throw new SFException( 'No destination path or data. Use setDestinationPath() and setData()', ERR_REPORT_APP );
		
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
		$this->transferSize = strlen($this->data);
		$this->dataPos = 0;
		
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
		
		echo substr( $this->data, $this->dataPos, ($len = min(8192,$this->transferSize-$this->dataPos)) );
		$this->dataPos += $len;
	}
	
	
	/**
	 * [INTERNAL FUNCTION]
	 * This function is called by the daemon when all data has been transferred.
	 * It is used to close open filehandles and clean up.
	*/
	
	public function transferEnd() {
		$this->transferStarted = FALSE;
		$this->transferSize = NULL;
		$this->data = NULL;
		$this->dataPos = NULL;
	}
}



/**
 * Background file-from-stream downloader
 * 
 * This class constructs a node that downloads from a stream resource on the server to a file on the client in 
 * the background. Construct it with the remote (client) destination path as the first argument,
 * the data stream as the second and the amount of bytes to read as the third, then call startDownload(). This control
 * fires a 'started' event when the download starts, a 'cycle' event periodically with progress
 * information, a 'finished' event when the download is finished and a 'cancelled' event when
 * the download is cancelled.
 * 
 * @see XULFilePicker
*/

class FileFromStreamDownloader extends Node
{
	public $destPath;
	public $stream = NULL;
	public $length = NULL;
	public $dataPos = NULL;
	public $remoteConstructor = 'FileDownloader';
	
	private $transferStarted = FALSE;
	private $transferSize = NULL;
	
	
	/**
	 * Constructor
	 *
	 * @param string	$destPath		Destination file path on the client side
	 * @param resource	$stream			Stream to read from
	 * @param int		$length			Amount of bytes to read from the stream and write to the file
	*/
	
	public function __construct( $destPath = NULL, $stream = NULL, $length = NULL ) {
		if( $destPath !== NULL ) $this->destPath = $destPath;
		if( $stream !== NULL ) $this->setStream( $stream, (int) $length );
		
		parent::__construct();
	}
	
	public function attach() {
		$this->createRemoteObject( array( $this->hostWindow ) );
		$this->setEventType( 'finished', MSG_SEND );
		$this->setEventType( 'cancelled', MSG_SEND );
		$this->setEventType( 'failed', MSG_SEND );
	}
	
	public function detach() {
		$this->unRegister();
	}
	
	
	/**
	 * Set the destination path
	 *
	 * @param string	$destPath		Destination file path on the client side
	*/
	
	public function setDestinationPath( $destPath ) {
		$this->destPath = $destPath;
	}
	
	
	/**
	 * Set the stream to read from
	 *
	 * @param resource	$stream		Stream to read from
 	 * @param int		$length		Amount of bytes to read from the stream and write to the file
	*/
	
	public function setStream( $stream, $length ) {
		if( ! is_resource($stream) )
			throw new SFException( "setStream(): not a stream resource", ERR_REPORT_APP );
		
		$this->stream = $stream;
		$this->length = (int) $length;
	}
	
	
	/**
	 * Cancel a running download
	 * 
	 * Cancels the downloader which fires the 'cancelled' event
	*/
	
	public function cancelDownload() {
		$this->callMethod( 'cancelDownload' );
	}
	
	
	/**
	 * Start the download
	 * 
	 * Starts the downloader which fires the 'started' event beforehand
	*/
	
	public function startDownload() {
		if( (!isset($this->destPath)) || $this->data === NULL )
			throw new SFException( 'No destination path or data. Use setDestinationPath() and setData()', ERR_REPORT_APP );
		
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
		$this->transferSize = $this->length;
		$this->dataPos = 0;
		
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
		
		echo fread( $this->stream, ($len = min(8192,$this->transferSize-$this->dataPos)) );
		$this->dataPos += $len;
	}
	
	
	/**
	 * [INTERNAL FUNCTION]
	 * This function is called by the daemon when all data has been transferred.
	 * It is used to close open filehandles and clean up.
	*/
	
	public function transferEnd() {
		$this->transferStarted = FALSE;
		$this->transferSize = NULL;
		$this->stream = NULL;
		$this->dataPos = NULL;
	}
}



/**
 * Client filesystem service
 * 
 * This class is used to access the client filesystem, to read and write
 * to directories and files, to run executables and to monitor files.
 * All operations are asynchronous, meaning that the call will not block
 * the script. A handler object and method can be given to handle the
 * result of the operation.
*/

class ClientFileService extends Node
{
	public $remoteConstructor = 'FileService';
	private $handlers = array();
	private $fileMonitors = array();
	
	public function attach() {
		$this->createRemoteObject( array( $this->hostWindow ) );
		$this->setEvent( 'result', MSG_SEND, $this, 'onResult' );
	}
	
	public function detach() {
		$this->callMethod( 'cancelAllMonitors' );
		$this->unRegister();
	}
	
	
	/**
	 * Gets the contents of a directory
	 * 
	 * The handler receives these parameters:
	 * - ClientFileService $fileService	- this object
	 * - bool $status: indicates whether the operation was successful
	 * - ClientDirectory $dirFile: if successful, a file object referring to the remote directory with the directory contents in the ClientFile::$entries array
	 *
	 * @param string		$path				The path on the client side to read
	 * @param object|string	$handlerObj			The object or classname to call the handler on when the operation finishes
	 * @param string		$handlerMethod		The method to call on the handler object
	*/
	
	public function getDirectory( $path, $handlerObj, $handlerMethod ) {
		$this->handlers[] = array( $handlerObj, $handlerMethod );
		$this->callMethod( 'getDirectory', array( $path ) );
	}
	
	
	/**
	 * Gets the contents of a special directory
	 * 
	 * The handler receives these parameters:
	 * - ClientFileService $fileService: this object
	 * - bool $status: indicates whether the operation was successful
	 * - ClientDirectory $dirFile: if successful, a file object referring to the remote directory with the directory contents in the ClientFile::$entries array
	 *
	 * @link https://developer.mozilla.org/en/Code_snippets/File_I%2F%2FO#Getting_special_files
	 * @param string		$id					ID of the special folder, see link
	 * @param object|string	$handlerObj			The object or classname to call the handler on when the operation finishes
	 * @param string		$handlerMethod		The method to call on the handler object
	*/
	
	public function getSpecialDirectory( $id, $handlerObj, $handlerMethod ) {
		$this->handlers[] = array( $handlerObj, $handlerMethod );
		$this->callMethod( 'getSpecialDirectory', array( $id ) );
	}
	
	
	/**
	 * Creates the directory
	 * 
	 * The handler receives these parameters:
	 * - ClientFileService $fileService	- this object
	 * - bool $status: indicates whether the operation was successful
	 * - ClientDirectory $dirFile: if successful, a file object referring to the new remote directory
	 *
	 * @param string		$path				The path on the client side to create
	 * @param object|string	$handlerObj			The object or classname to call the handler on when the operation finishes
	 * @param string		$handlerMethod		The method to call on the handler object
	*/
	
	public function createDirectory( $path, $handlerObj = NULL, $handlerMethod = NULL ) {
		$this->handlers[] = ($handlerObj&&$handlerMethod ? array( $handlerObj, $handlerMethod ) : NULL);
		$this->callMethod( 'createDirectory', array( $path ) );
	}
	
	
	/**
	 * Copies a file from server to client
	 * 
	 * Returns the FileDownloader object to allow progress checking. The handler is attached to the 'finished', 'failed' and 'cancelled' events
	 * of the FileDownloader.
	 *
	 * @param string		$localPath			The path on the server side
	 * @param string		$clientPath			The path on the client side
	 * @param object|string	$handlerObj			The object or classname to call the handler on when the operation finishes
	 * @param string		$handlerMethod		The method to call on the handler object
	 * @return FileDownloader $transfer			The transfer object keeping track of transfer progress
	*/
	
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
	
	
	/**
	 * Stores a string in a file on the client side
	 * 
	 * Returns the FileFromStringDownloader object to allow progress checking. The handler is attached to the 'finished', 'failed' and 'cancelled' events
	 * of the FileFromStringDownloader.
	 *
	 * @param string		$clientPath			The path on the client side
	 * @param string		$data				Data to write to the file
	 * @param object|string	$handlerObj			The object or classname to call the handler on when the operation finishes
	 * @param string		$handlerMethod		The method to call on the handler object
	 * @return FileFromStringDownloader $transfer			The transfer object keeping track of transfer progress
	*/
	
	public function writeFileFromString( $clientPath, $data, $handlerObj = NULL, $handlerMethod = NULL ) {
		$this->hostWindow->addChild( $fdn = new FileFromStringDownloader( $clientPath, $data ) );
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
	
	
	/**
	 * Stores data read from a stream in a file on the client side
	 * 
	 * Returns the FileFromStreamDownloader object to allow progress checking. The handler is attached to the 'finished', 'failed' and 'cancelled' events
	 * of the FileFromStringDownloader.
	 *
	 * @param string		$clientPath			The path on the client side
	 * @param resource		$stream				Stream to read from
	 * @param int			$length				Amount of bytes to read from the stream and write to the file
	 * @param object|string	$handlerObj			The object or classname to call the handler on when the operation finishes
	 * @param string		$handlerMethod		The method to call on the handler object
	 * @return FileFromStringDownloader $transfer			The transfer object keeping track of transfer progress
	*/
	
	public function writeFileFromStream( $clientPath, $stream, $length, $handlerObj = NULL, $handlerMethod = NULL ) {
		$this->hostWindow->addChild( $fdn = new FileFromStreamDownloader( $clientPath, $stream, $length ) );
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
	
	
	/**
	 * [INTERNAL FUNCTION] Event handler
	*/
	
	public function onDownloaderFinished( $event ) {
		if($event->sourceObject->isChild)
			$event->sourceObject->removeNode();
	}
	
	
	/**
	 * Copies a file from client to server
	 * 
	 * Returns the FileUploader object to allow progress checking. The handler is attached to the 'finished', 'failed' and 'cancelled' events
	 * of the FileUploader.
	 *
	 * @param string		$clientPath			The path on the client side
	 * @param string		$localPath			The path on the server side
	 * @param object|string	$handlerObj			The object or classname to call the handler on when the operation finishes
	 * @param string		$handlerMethod		The method to call on the handler object
	 * @return FileUploader $transfer			The transfer object keeping track of transfer progress
	*/
	
	public function readFile( $clientPath, $localPath, $handlerObj = NULL, $handlerMethod = NULL ) {
		$this->hostWindow->addChild( $fup = new FileUploader( $clientPath, $localPath ) );
		if( $handlerObj && $handlerMethod ) {
			$fup->setEventHandler( 'finished', $handlerObj, $handlerMethod );
			$fup->setEventHandler( 'failed', $handlerObj, $handlerMethod );
			$fup->setEventHandler( 'cancelled', $handlerObj, $handlerMethod );
		}
		$fup->setEventHandler( 'finished', $this, 'onUploaderFinished' );
		$fup->setEventHandler( 'failed', $this, 'onUploaderFinished' );
		$fup->setEventHandler( 'cancelled', $this, 'onUploaderFinished' );
		$fup->startUpload();
		
		return $fup;
	}
	
	
	/**
	 * Reads a file on the client side to a string
	 * 
	 * Returns the FileToStringUploader object to allow progress checking. The handler is attached to the 'finished', 'failed' and 'cancelled' events
	 * of the FileToStringUploader. The file contents are put in the FileToStringUploader::$data property
	 *
	 * @param string		$clientPath			The path on the client side
	 * @param object|string	$handlerObj			The object or classname to call the handler on when the operation finishes
	 * @param string		$handlerMethod		The method to call on the handler object
	 * @return FileToStringUploader $transfer			The transfer object keeping track of transfer progress
	*/
	
	public function readFileToString( $clientPath, $handlerObj = NULL, $handlerMethod = NULL ) {
		$this->hostWindow->addChild( $fup = new FileToStringUploader( $clientPath ) );
		if( $handlerObj && $handlerMethod ) {
			$fup->setEventHandler( 'finished', $handlerObj, $handlerMethod );
			$fup->setEventHandler( 'failed', $handlerObj, $handlerMethod );
			$fup->setEventHandler( 'cancelled', $handlerObj, $handlerMethod );
		}
		$fup->setEventHandler( 'finished', $this, 'onUploaderFinished' );
		$fup->setEventHandler( 'failed', $this, 'onUploaderFinished' );
		$fup->setEventHandler( 'cancelled', $this, 'onUploaderFinished' );
		$fup->startUpload();
		
		return $fup;
	}
	
	
	/**
	 * Reads a file on the client side and writes it to a stream resource on the server
	 * 
	 * Returns the FileToStreamUploader object to allow progress checking. The handler is attached to the 'finished', 'failed' and 'cancelled' events
	 * of the FileToStreamUploader.
	 *
	 * @param string		$clientPath			The path on the client side
	 * @param resource		$stream				Stream to write to
	 * @param object|string	$handlerObj			The object or classname to call the handler on when the operation finishes
	 * @param string		$handlerMethod		The method to call on the handler object
	 * @return FileToStreamUploader $transfer			The transfer object keeping track of transfer progress
	*/
	
	public function readFileToStream( $clientPath, $stream, $handlerObj = NULL, $handlerMethod = NULL ) {
		$this->hostWindow->addChild( $fup = new FileToStreamUploader( $clientPath, $stream ) );
		if( $handlerObj && $handlerMethod ) {
			$fup->setEventHandler( 'finished', $handlerObj, $handlerMethod );
			$fup->setEventHandler( 'failed', $handlerObj, $handlerMethod );
			$fup->setEventHandler( 'cancelled', $handlerObj, $handlerMethod );
		}
		$fup->setEventHandler( 'finished', $this, 'onUploaderFinished' );
		$fup->setEventHandler( 'failed', $this, 'onUploaderFinished' );
		$fup->setEventHandler( 'cancelled', $this, 'onUploaderFinished' );
		$fup->startUpload();
		
		return $fup;
	}
	
	
	/**
	 * [INTERNAL FUNCTION] Event handler
	*/
	
	public function onUploaderFinished( $event ) {
		if( $event->sourceObject->isChild )
			$event->sourceObject->removeNode();
	}
	
	
	/**
	 * Removes the directory
	 * 
	 * The handler receives these parameters:
	 * - ClientFileService $fileService	- this object
	 * - bool $status: indicates whether the operation was successful
	 * - string $path: the remote path
	 *
	 * @param string		$clientPath			The path on the client side to remove
	 * @param bool			$recursive			If TRUE, all underlying contents are removed as well. If FALSE, the directory needs to be empty
	 * @param object|string	$handlerObj			The object or classname to call the handler on when the operation finishes
	 * @param string		$handlerMethod		The method to call on the handler object
	*/
	
	public function removeDirectory( $clientPath, $recursive, $handlerObj = NULL, $handlerMethod = NULL ) {
		$this->handlers[] = ($handlerObj&&$handlerMethod ? array( $handlerObj, $handlerMethod ) : NULL);
		$this->callMethod( 'removeDirectory', array( $clientPath, (bool)$recursive ) );
	}
	
	
	/**
	 * Removes the file
	 * 
	 * The handler receives these parameters:
	 * - ClientFileService $fileService	- this object
	 * - bool $status: indicates whether the operation was successful
	 * - string $path: the remote path
	 *
	 * @param string		$clientPath			The path on the client side to remove
	 * @param object|string	$handlerObj			The object or classname to call the handler on when the operation finishes
	 * @param string		$handlerMethod		The method to call on the handler object
	*/
	
	public function removeFile( $clientPath, $handlerObj = NULL, $handlerMethod = NULL ) {
		$this->handlers[] = ($handlerObj&&$handlerMethod ? array( $handlerObj, $handlerMethod ) : NULL);
		$this->callMethod( 'removeFile', array( $clientPath ) );
	}
	
	
	/**
	 * Starts monitoring the file
	 * 
	 * The handler receives these parameters when the file is created, removed or changed:
	 * - ClientFileService $fileService	- this object
	 * - string $path: the remote path
	 * - bool $exists: whether the file exists
	 * - int $modificationTime: the modification time of the file
	 * - int $size: file size
	 *
	 * @param string		$clientPath			The path on the client side to monitor
	 * @param object|string	$handlerObj			The object or classname to call the handler on when the operation finishes
	 * @param string		$handlerMethod		The method to call on the handler object
	*/
	
	public function monitorFile( $clientPath, $handlerObj, $handlerMethod ) {
		if( isset($this->fileMonitors[$clientPath]) )
			throw new SFException( "Path $clientPath is already being monitored!", ERR_REPORT_APP );
		
		$this->fileMonitors[$clientPath] = array( $handlerObj, $handlerMethod );
		$this->callMethod( 'monitorFile', array( $clientPath ) );
	}
	
	
	/**
	 * Executes the file
	 * 
	 * The handler receives these parameters:
	 * - ClientFileService $fileService	- this object
	 * - bool $status: indicates whether the operation was successful
	 * - string $path: the remote path
	 *
	 * @param string		$clientPath			The path on the client side to execute
	 * @param array 		$args				Arguments for the executable
	 * @param bool			$async				If TRUE, the file is executed in the background. If FALSE, the application blocks until the executable ends
	 * @param object|string	$handlerObj			The object or classname to call the handler on when the operation finishes
	 * @param string		$handlerMethod		The method to call on the handler object
	*/
	
	public function executeFile( $clientPath, $args = array(), $async = TRUE, $handlerObj = NULL, $handlerMethod = NULL ) {
		$this->handlers[] = ($handlerObj&&$handlerMethod ? array( $handlerObj, $handlerMethod ) : NULL);
		$this->callMethod( 'executeFile', array( $clientPath, (array) $args, $async ) );
	}
	
	
	/**
	 * [INTERNAL FUNCTION] Event handler
	*/
	
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
			
			case 'executeFile':
				list($status) = $args;
				if( ($handler = array_shift($this->handlers)) !== NULL )
					call_user_func_array( $handler, array($this,$status) );
			break;
		}
	}
	
	public function fileFromResult( $path, $file ) {
		$cf = ($file[1] ? new ClientDirectory() : new ClientFile());
		$cf->fileService = $this;
		$cf->path = $path;
		$cf->name = $file[0];
		$cf->isDirectory = $file[1];
		$cf->isReadable = $file[2];
		$cf->isWritable = $file[3];
		$cf->isExecutable = $file[4];
		$cf->isHidden = $file[5];
		$cf->size = $file[6];
		$cf->exists = TRUE;
		return $cf;
	}
	
	public function getDirSeparator() {
		if( stripos(ApplicationProcess::$PlatformInfo['platform'],'win') !== FALSE )
			return "\\";
		else
			return "/";
	}
}


/**
 * Client filesystem class structure representing files (ClientFile) and directories (ClientDirectory) on the client side
 * 
 * These classes form the object representation of the filesystem of the client and can be created from the
 * methods ClientFileService::getDirectory(), ClientFileService::getSpecialDirectory() and ClientFileService::createDirectory().
*/

class ClientBaseFile
{
	public $fileService;
	public $path;
	public $name;
	public $isDirectory;
	public $isReadable;
	public $isWritable;
	public $isExecutable;
	public $isHidden;
	public $exists = FALSE;
}

class ClientFile extends ClientBaseFile
{	
	public $isDirectory = FALSE;
	public $size;
	
	
	/**
	 * Copies a file from server to client
	 * 
	 * Returns the FileDownloader object to allow progress checking. The handler is attached to the 'finished', 'failed' and 'cancelled' events
	 * of the FileDownloader.
	 *
	 * @param string		$localPath			The path on the server side
	 * @param object|string	$handlerObj			The object or classname to call the handler on when the operation finishes
	 * @param string		$handlerMethod		The method to call on the handler object
	 * @return FileDownloader $transfer			The transfer object keeping track of transfer progress
	*/
	
	public function write( $localPath, $handlerObj = NULL, $handlerMethod = NULL ) {
		return $this->fileService->writeFile( $localPath, $this->path, $handlerObj, $handlerMethod );
	}
	
	
	/**
	 * Copies a file from client to server
	 * 
	 * Returns the FileUploader object to allow progress checking. The handler is attached to the 'finished', 'failed' and 'cancelled' events
	 * of the FileUploader.
	 *
	 * @param string		$localPath			The path on the server side
	 * @param object|string	$handlerObj			The object or classname to call the handler on when the operation finishes
	 * @param string		$handlerMethod		The method to call on the handler object
	 * @return FileUploader $transfer			The transfer object keeping track of transfer progress
	*/
	
	public function read( $localPath, $handlerObj = NULL, $handlerMethod = NULL ) {
		return $this->fileService->readFile( $this->path, $localPath, $handlerObj, $handlerMethod );
	}
	
	
	/**
	 * Stores a string in a file on the client side
	 * 
	 * Returns the FileFromStringDownloader object to allow progress checking. The handler is attached to the 'finished', 'failed' and 'cancelled' events
	 * of the FileFromStringDownloader.
	 *
	 * @param string		$data				Data to write to the file
	 * @param object|string	$handlerObj			The object or classname to call the handler on when the operation finishes
	 * @param string		$handlerMethod		The method to call on the handler object
	 * @return FileFromStringDownloader $transfer			The transfer object keeping track of transfer progress
	*/
	
	public function writeFromString( $data, $handlerObj = NULL, $handlerMethod = NULL ) {
		return $this->fileService->writeFileFromString( $this->path, $data, $handlerObj, $handlerMethod );
	}
	
	
	/**
	 * Reads a file on the client side to a string
	 * 
	 * Returns the FileToStringUploader object to allow progress checking. The handler is attached to the 'finished', 'failed' and 'cancelled' events
	 * of the FileToStringUploader. The file contents are put in the FileToStringUploader::$data property
	 *
	 * @param object|string	$handlerObj			The object or classname to call the handler on when the operation finishes
	 * @param string		$handlerMethod		The method to call on the handler object
	 * @return FileToStringUploader $transfer			The transfer object keeping track of transfer progress
	*/
	
	public function readToString( $handlerObj = NULL, $handlerMethod = NULL ) {
		return $this->fileService->readFileToString( $this->path, $handlerObj, $handlerMethod );
	}
	
	
	/**
	 * Read from a stream and write to a file on the client side
	 * 
	 * Returns the FileFromStreamDownloader object to allow progress checking. The handler is attached to the 'finished', 'failed' and 'cancelled' events
	 * of the FileFromStreamDownloader.
	 *
	 * @param resource		$stream				Stream to read from
	 * @param int			$length				Amount of bytes to read from the stream and write to the file
	 * @param object|string	$handlerObj			The object or classname to call the handler on when the operation finishes
	 * @param string		$handlerMethod		The method to call on the handler object
	 * @return FileFromStreamDownloader $transfer			The transfer object keeping track of transfer progress
	*/
	
	public function writeFromStream( $stream, $length, $handlerObj = NULL, $handlerMethod = NULL ) {
		return $this->fileService->writeFileFromStream( $this->path, $stream, $length, $handlerObj, $handlerMethod );
	}
	
	
	/**
	 * Reads a file on the client side to a stream
	 * 
	 * Returns the FileToStreamUploader object to allow progress checking. The handler is attached to the 'finished', 'failed' and 'cancelled' events
	 * of the FileToStreamUploader.
	 *
	 * @param resource		$stream				Stream to write to
	 * @param object|string	$handlerObj			The object or classname to call the handler on when the operation finishes
	 * @param string		$handlerMethod		The method to call on the handler object
	 * @return FileToStringUploader $transfer			The transfer object keeping track of transfer progress
	*/
	
	public function readToStream( $stream, $handlerObj = NULL, $handlerMethod = NULL ) {
		return $this->fileService->readFileToStream( $this->path, $stream, $handlerObj, $handlerMethod );
	}
	
	
	/**
	 * Removes the file
	 * 
	 * The handler receives these parameters:
	 * - ClientFileService $fileService	- this object
	 * - bool $status: indicates whether the operation was successful
	 * - string $path: the remote path
	 *
	 * @param object|string	$handlerObj			The object or classname to call the handler on when the operation finishes
	 * @param string		$handlerMethod		The method to call on the handler object
	*/
	
	public function remove( $handlerObj = NULL, $handlerMethod = NULL ) {
		$this->fileService->removeFile( $this->path, $handlerObj, $handlerMethod );
	}
	
	
	/**
	 * Starts monitoring the file
	 * 
	 * The handler receives these parameters when the file is created, removed or changed:
	 * - ClientFileService $fileService	- this object
	 * - string $path: the remote path
	 * - bool $exists: whether the file exists
	 * - int $modificationTime: the modification time of the file
	 * - int $size: file size
	 *
	 * @param object|string	$handlerObj			The object or classname to call the handler on when the operation finishes
	 * @param string		$handlerMethod		The method to call on the handler object
	*/
	
	public function monitor( $handlerObj, $handlerMethod ) {
		$this->fileService->monitorFile( $this->path, $handlerObj, $handlerMethod );
	}

	
	/**
	 * Executes the file
	 * 
	 * The handler receives these parameters:
	 * - ClientFileService $fileService	- this object
	 * - bool $status: indicates whether the operation was successful
	 * - string $path: the remote path
	 *
	 * @param array 		$args				Arguments for the executable
	 * @param bool			$async				If TRUE, the file is executed in the background. If FALSE, the application blocks until the executable ends
	 * @param object|string	$handlerObj			The object or classname to call the handler on when the operation finishes
	 * @param string		$handlerMethod		The method to call on the handler object
	*/
	
	public function execute( $args = array(), $async = TRUE, $handlerObj = NULL, $handlerMethod = NULL ) {
		$this->fileService->executeFile( $this->path, $args, $async, $handlerObj, $handlerMethod );
	}
}

class ClientDirectory extends ClientBaseFile
{
	public $isDirectory = TRUE;
	public $entries = array();
	
	
	/**
	 * Returns a new ClientDirectory representing a new directory in the current one
	 * 
	 * @param string		$dirName		The directory name to append to the current path
	 * @return ClientDirectory	The (new or existing) directory
	*/
	
	public function appendDirectory( $dirName ) {
		if( isset($this->entries[$dirName]) ) {
			if( $this->entries[$dirName]->isDirectory )
				return $this->entries[$dirName];
			else
				throw new SFException( "$dirName is a file", ERR_REPORT_APP );
		}
		
		$file = new ClientDirectory;
		$file->fileService = $this->fileService;
		$file->path = $this->path.$this->fileService->getDirSeparator().$dirName;
		$file->name = $dirName;
		$file->exists = FALSE;
		return $file;
	}
	
	
	/**
	 * Returns a new ClientFile representing a new file in the current directory
	 * 
	 * @param string		$fileName		The filename to append to the current path
	 * @return ClientFile	The (new or existing) file
	*/
	
	public function appendFile( $fileName ) {
		if( isset($this->entries[$fileName]) ) {
			if( !$this->entries[$fileName]->isDirectory )
				return $this->entries[$fileName];
			else
				throw new SFException( "$fileName is a directory", ERR_REPORT_APP );
		}
		
		$file = new ClientFile;
		$file->fileService = $this->fileService;
		$file->path = $this->path.$this->fileService->getDirSeparator().$fileName;
		$file->name = $fileName;
		$file->exists = FALSE;
		return $file;
	}
	
	
	/**
	 * Creates the directory
	 * 
	 * The handler receives these parameters:
	 * - ClientFileService $fileService	- this object
	 * - bool $status: indicates whether the operation was successful
	 * - ClientDirectory $dirFile: if successful, a file object referring to the new remote directory
	 *
	 * @param object|string	$handlerObj			The object or classname to call the handler on when the operation finishes
	 * @param string		$handlerMethod		The method to call on the handler object
	*/
	
	public function create( $handlerObj = NULL, $handlerMethod = NULL ) {
		$this->fileService->createDirectory( $this->path, $handlerObj, $handlerMethod );
	}
	
	
	/**
	 * Gets the contents of a directory
	 * 
	 * The handler receives these parameters:
	 * - ClientFileService $fileService	- this object
	 * - bool $status: indicates whether the operation was successful
	 * - ClientDirectory $dirFile: if successful, a file object referring to the remote directory with the directory contents in the ClientFile::$entries array
	 *
	 * @param object|string	$handlerObj			The object or classname to call the handler on when the operation finishes
	 * @param string		$handlerMethod		The method to call on the handler object
	*/
	
	public function get( $handlerObj, $handlerMethod ) {
		$this->fileService->getDirectory( $this->path, $handlerObj, $handlerMethod );
	}
	
	
	/**
	 * Removes the directory
	 * 
	 * The handler receives these parameters:
	 * - ClientFileService $fileService	- this object
	 * - bool $status: indicates whether the operation was successful
	 * - string $path: the remote path
	 *
	 * @param bool			$recursive			If TRUE, all underlying contents are removed as well. If FALSE, the directory needs to be empty
	 * @param object|string	$handlerObj			The object or classname to call the handler on when the operation finishes
	 * @param string		$handlerMethod		The method to call on the handler object
	*/
	
	public function remove( $recursive = FALSE, $handlerObj = NULL, $handlerMethod = NULL ) {
		$this->fileService->removeDirectory( $this->path, $recursive, $handlerObj, $handlerMethod );
	}
}


/**
 * Executes AppleScript code on MacOSX clients
 */

class AppleScriptService extends Node
{
	public $remoteConstructor = 'AppleScriptService';
	
	public function attach() {
		$this->createRemoteObject();
	}
	
	public function detach() {
		$this->unRegister();
	}
	
	
	/**
	 * Executes AppleScript code on the client
	 * 
	 * @param string	$code		Code to execute
	 */
	
	public function execute( $code ) {
		$lines = explode( "\n", $code );
		for( $n = 0; $n < count($lines); $n++ ) {
			$lines[$n] = trim($lines[$n]);
		}
		$this->callMethod( 'execute', array($lines) );
	}
}



