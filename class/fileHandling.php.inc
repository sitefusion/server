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
 *
 * @package API
 * @subpackage FileHandling
*/
class XULFilePicker extends Node {

    public $remoteConstructor = 'FilePicker';
    public $file = NULL;
    public $files = NULL;
    public $returnCode = 'cancel';
    public $selectedFilter = FALSE;


    /**
     * Constructor
     *
     * @param string    $title          Title for the file picker dialog
     * @param string    $mode           Type of file picker ('open' = open single file, 'save' = save a file, 'getfolder' = select a folder, 'openmultiple' = open multiple files
     * @param string    $defaultString  Default value (file/folder path)
     * @param string    $defaultExtension   Default extension value. Needs to be set in order to enable reading the filter extensions.
    */
    public function __construct($title, $mode = 'open', $defaultString = '', $defaultExtension = '') {
        $this->title = $title;
        $this->defaultString = (string) $defaultString;
        $this->defaultExtension = $defaultExtension;

        if ($mode == 'open' || $mode == 'save' || $mode == 'getfolder' || $mode == 'openmultiple') {
            $this->mode = $mode;
        } else {
            throw new SFException( 'Invalid mode', ERR_REPORT_APP );
        }

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
     * @param string    $description    Description of the filter
     * @param string    $value          Value for the filter, semicolon separated
    */
    public function addFilter($description, $value) {
        $this->callMethod( 'addFilter', array($description, $value) );
    }

    /**
     * Adds a file type systemfilter
     *
     * Example: $filePicker->addSystemFilter("Components.interfaces.nsIFilePicker.filterAll");
     *
     * See: https://developer.mozilla.org/en-US/docs/XPCOM_Interface_Reference/nsIFilePicker#Filter_constants
     * @param string    $filter Identifier for the system Filter
    */
    public function addSystemFilter($filter) {
        $this->callMethod( 'addSystemFilter', array($filter) );
    }

    /**
     * Event handler for the yield event
    */
    public function yieldCollect($e, $returnCode, $path = NULL, $selectedFilter = FALSE) {
        $this->returnCode = $returnCode;
        $this->selectedFilter = array();
        if (is_array($selectedFilter) && count($selectedFilter) > 1) {
            $this->selectedFilter['filterName'] = $selectedFilter[0];
            $this->selectedFilter['filterContents'] = explode(';', $selectedFilter[1]);
        }

        if ($this->mode == 'openmultiple') {
            $this->files = $path;
            $this->file = NULL;
            $this->fireLocalEvent( 'closed', array( $this->returnCode, $this->files ) );
        } else {
            $this->file = $path;
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
 * @package API
 * @subpackage FileHandling
*/
class FileUploader extends Node {

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
     * @param string    $clientPath     Source file path on the client side
     * @param string    $destPath       Destination file path on the server side
    */
    public function __construct($clientPath = NULL, $destPath = NULL) {
        if ($clientPath) {
            $this->clientPath = $clientPath;
        }
        if ($destPath) {
            $this->destPath = $destPath;
        }

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
     * @param string    $clientPath     Source file path on the client side
     * @param string    $destPath       Destination file path on the server side
    */
    public function setPaths($clientPath, $destPath) {
        $this->clientPath = $clientPath;
        $this->destPath = $destPath;
    }


    /**
     * Start the upload
     *
     * Starts the uploader which fires the 'started' event beforehand
    */
    public function startUpload() {
        if (!isset($this->destPath) || !isset($this->clientPath)) {
            throw new SFException( 'No source or destination file path given. Use setPaths()', ERR_REPORT_APP );
        }

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
    public function handleError($e, $remotePath) {
        $path = $this->destPath;

        if (file_exists($path)) {
            @unlink( $path );
        }
    }


    /**
     * Event handler for the 'cycle' event
    */
    public function handleFileChunk($e, $filesize, $progress, $cycle, $data) {
        $path = $this->destPath;

        if (file_exists($path) && $cycle == 1) {
            @unlink( $path );
        }

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
 * @package API
 * @subpackage FileHandling
*/
class FileToStringUploader extends Node {

    public $data = NULL;
    public $bufferSize = NULL;
    public $fileSize = NULL;
    public $clientPath;
    public $remoteConstructor = 'FileUploader';


    /**
     * Constructor
     *
     * @param string    $clientPath     Source file path on the client side
    */
    public function __construct($clientPath = NULL) {
        if ($clientPath) {
            $this->clientPath = $clientPath;
        }

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
     * @param string    $clientPath     Source file path on the client side
    */
    public function setSourcePath($clientPath) {
        $this->clientPath = $clientPath;
    }


    /**
     * Start the upload
     *
     * Starts the uploader which fires the 'started' event beforehand
    */
    public function startUpload() {
        if (!isset($this->clientPath)) {
            throw new SFException( 'No source path given. Use setSourcePath()', ERR_REPORT_APP );
        }

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
    public function handleError($e, $remotePath) {
        $this->data = NULL;
        $this->bufferSize = NULL;
        $this->fileSize = NULL;
    }


    /**
     * Event handler for the 'cycle' event
    */
    public function handleFileChunk($e, $filesize, $progress, $cycle, $data) {
        if ($this->data === NULL) {
            $this->data = '';
        }

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
 * @package API
 * @subpackage FileHandling
*/
class FileToStreamUploader extends Node {

    public $stream = NULL;
    public $fileSize = NULL;
    public $clientPath;
    public $remoteConstructor = 'FileUploader';


    /**
     * Constructor
     *
     * @param string    $clientPath     Source file path on the client side
     * @param resource  $stream         Stream to write data to
    */
    public function __construct($clientPath = NULL, $stream = NULL) {
        if ($clientPath) {
            $this->clientPath = $clientPath;
        }
        if ($stream) {
            $this->setStream( $stream );
        }

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
     * @param string    $clientPath     Source file path on the client side
    */
    public function setSourcePath($clientPath) {
        $this->clientPath = $clientPath;
    }


    /**
     * Set the stream to write to
     *
     * @param resource  $stream     Stream to write data to
    */
    public function setStream($stream) {
        if (!is_resource($stream)) {
            throw new SFException( 'setStream(): not a stream resource', ERR_REPORT_APP );
        }

        $this->stream = $stream;
    }


    /**
     * Start the upload
     *
     * Starts the uploader which fires the 'started' event beforehand
    */
    public function startUpload() {
        if (!isset($this->clientPath) || !is_resource($this->stream)) {
            throw new SFException( 'No source path given or no stream set. Use setSourcePath() and setStream()', ERR_REPORT_APP );
        }

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
    public function handleError($e, $remotePath) {
        $this->fileSize = NULL;
    }


    /**
     * Event handler for the 'cycle' event
    */
    public function handleFileChunk($e, $filesize, $progress, $cycle, $data) {
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
 * @package API
 * @subpackage FileHandling
*/
class FileDownloader extends Node {

    public $filePath;
    public $destPath;
    public $remoteConstructor = 'FileDownloader';
    public $removeAfterTransfer = FALSE;
    public $destPathRelativeToTmp = FALSE;

    private $_transferStarted = FALSE;
    private $_transferSize = NULL;
    private $_transferFile = NULL;


    /**
     * Constructor
     *
     * @param string    $filePath       Source file path on the server side
     * @param string    $destPath       Destination file path on the client side
     * @param boolean   $destPathRelativeToTmp      Bool to indicate filePath is relative to temp
    */
    public function __construct($filePath = NULL, $destPath = NULL, $destPathRelativeToTmp = FALSE) {
        if ($filePath) {
            $this->filePath = $filePath;
        }
        if ($destPath) {
            $this->destPath = $destPath;
        }

        $this->destPathRelativeToTmp = $destPathRelativeToTmp;
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
     * @param string    $filePath       Source file path on the server side
     * @param string    $destPath       Destination file path on the client side
    */
    public function setPaths($filePath, $destPath) {
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
        if (!isset($this->destPath) || !isset($this->filePath)) {
            throw new SFException( 'No file path or name given. Use setPaths()', ERR_REPORT_APP );
        }

        $this->callMethod( 'startDownload', array( $this->destPath, $this->destPathRelativeToTmp ) );
    }


    /**
     * [INTERNAL FUNCTION]
     * This function is called by the daemon to start a transfer. It returns an array
     * of two elements, first is a string with the MIME content-type of the data, and
     * second is a integer number containing the size in bytes of the transfer.
     *
     * @return array    Transfer descriptives
    */
    public function transferStart() {
        $this->_transferStarted = TRUE;
        $this->_transferSize = filesize($this->filePath);
        $this->_transferFile = fopen( $this->filePath, 'r' );

        return array( 'application/octet-stream', $this->_transferSize );
    }


    /**
     * [INTERNAL FUNCTION]
     * This function is called by the daemon to get (a piece of) data of a transfer.
     * It should output this data to the output buffer, which will be intercepted
     * by the daemon.
    */
    public function transferGetData() {
        if (!$this->_transferStarted) {
            throw new SFException( 'File transfer not started!' );
        }

        echo fread( $this->_transferFile, 8192 );
    }


    /**
     * [INTERNAL FUNCTION]
     * This function is called by the daemon when all data has been transferred.
     * It is used to close open filehandles and clean up.
    */
    public function transferEnd() {
        fclose( $this->_transferFile );
        $this->_transferStarted = FALSE;
        $this->_transferSize = NULL;
        $this->_transferFile = NULL;

        if ($this->removeAfterTransfer) {
            unlink( $this->filePath );
        }
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
 * @package API
 * @subpackage FileHandling
*/
class FileFromStringDownloader extends Node {

    public $destPath;
    public $data = NULL;
    public $dataPos = NULL;
    public $remoteConstructor = 'FileDownloader';

    private $_transferStarted = FALSE;
    private $_transferSize = NULL;

    /**
     * Constructor
     *
     * @param string    $destPath       Destination file path on the client side
     * @param string    $data           Data to store in the file
    */
    public function __construct($destPath = NULL, $data = NULL) {
        if ($destPath !== NULL) {
            $this->destPath = $destPath;
        }
        if ($data !== NULL) {
            $this->data = (string) $data;
        }

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
     * @param string    $destPath       Destination file path on the client side
    */
    public function setDestinationPath($destPath) {
        $this->destPath = $destPath;
    }


    /**
     * Set the data to store in the file
     *
     * @param string    $data       Data to store in the file
    */
    public function setData($data) {
        $this->data = (string) $data;
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
        if (!isset($this->destPath)) {
            throw new SFException( 'No destination path. Use setDestinationPath()', ERR_REPORT_APP );
        }

        $this->callMethod( 'startDownload', array( $this->destPath, ($this->data == NULL)) );
    }


    /**
     * [INTERNAL FUNCTION]
     * This function is called by the daemon to start a transfer. It returns an array
     * of two elements, first is a string with the MIME content-type of the data, and
     * second is a integer number containing the size in bytes of the transfer.
     *
     * @return array    Transfer descriptives
    */
    public function transferStart() {
        $this->_transferStarted = TRUE;
        $this->_transferSize = strlen($this->data);
        $this->dataPos = 0;

        return array( 'application/octet-stream', $this->_transferSize );
    }


    /**
     * [INTERNAL FUNCTION]
     * This function is called by the daemon to get (a piece of) data of a transfer.
     * It should output this data to the output buffer, which will be intercepted
     * by the daemon.
    */
    public function transferGetData() {
        if (!$this->_transferStarted) {
            throw new SFException( 'File transfer not started!' );
        }

        echo substr( $this->data, $this->dataPos, ($len = min(8192,$this->_transferSize-$this->dataPos)) );
        $this->dataPos += $len;
    }


    /**
     * [INTERNAL FUNCTION]
     * This function is called by the daemon when all data has been transferred.
     * It is used to close open filehandles and clean up.
    */
    public function transferEnd() {
        $this->_transferStarted = FALSE;
        $this->_transferSize = NULL;
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
 * @package API
 * @subpackage FileHandling
*/
class FileFromStreamDownloader extends Node {

    public $destPath;
    public $stream = NULL;
    public $length = NULL;
    public $dataPos = NULL;
    public $remoteConstructor = 'FileDownloader';

    private $_transferStarted = FALSE;
    private $_transferSize = NULL;


    /**
     * Constructor
     *
     * @param string    $destPath       Destination file path on the client side
     * @param resource  $stream         Stream to read from
     * @param int       $length         Amount of bytes to read from the stream and write to the file
    */
    public function __construct($destPath = NULL, $stream = NULL, $length = NULL) {
        if ($destPath !== NULL) {
            $this->destPath = $destPath;
        }
        if ($stream !== NULL) {
            $this->setStream( $stream, (int) $length );
        }

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
     * @param string    $destPath       Destination file path on the client side
    */
    public function setDestinationPath($destPath) {
        $this->destPath = $destPath;
    }


    /**
     * Set the stream to read from
     *
     * @param resource  $stream     Stream to read from
     * @param int       $length     Amount of bytes to read from the stream and write to the file
    */
    public function setStream($stream, $length) {
        if (!is_resource($stream)) {
            throw new SFException( 'setStream(): not a stream resource', ERR_REPORT_APP );
        }

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
        if ((!isset($this->destPath)) || $this->data === NULL) {
            throw new SFException( 'No destination path or data. Use setDestinationPath() and setData()', ERR_REPORT_APP );
        }

        $this->callMethod( 'startDownload', array( $this->destPath ) );
    }


    /**
     * [INTERNAL FUNCTION]
     * This function is called by the daemon to start a transfer. It returns an array
     * of two elements, first is a string with the MIME content-type of the data, and
     * second is a integer number containing the size in bytes of the transfer.
     *
     * @return array    Transfer descriptives
    */
    public function transferStart() {
        $this->_transferStarted = TRUE;
        $this->_transferSize = $this->length;
        $this->dataPos = 0;

        return array( 'application/octet-stream', $this->_transferSize );
    }


    /**
     * [INTERNAL FUNCTION]
     * This function is called by the daemon to get (a piece of) data of a transfer.
     * It should output this data to the output buffer, which will be intercepted
     * by the daemon.
    */
    public function transferGetData() {
        if (!$this->_transferStarted) {
            throw new SFException( 'File transfer not started!' );
        }

        echo fread( $this->stream, ($len = min(8192,$this->_transferSize-$this->dataPos)) );
        $this->dataPos += $len;
    }


    /**
     * [INTERNAL FUNCTION]
     * This function is called by the daemon when all data has been transferred.
     * It is used to close open filehandles and clean up.
    */
    public function transferEnd() {
        $this->_transferStarted = FALSE;
        $this->_transferSize = NULL;
        $this->stream = NULL;
        $this->dataPos = NULL;
    }
}

class URLUploader extends Node {

    public $remoteConstructor = 'URLUploader';

    public $localPath = NULL;
    public $serverPath = NULL;
    public $cycleStart = 500;

    private $_transferStarted = FALSE;
    private $_transferFile = NULL;

    public function __construct($localPath = NULL, $serverPath = NULL, $cycleStart = NULL) {
        /* Set the local path */
        if (!empty($localPath)) {
            $this->localPath = $localPath;
        }

        /* Set the url */
        if (!empty($serverPath)) {
            $this->serverPath = $serverPath;
        }

        /* Set the url */
        if (!empty($cycleStart)) {
            $this->cycleStart = $cycleStart;
        }

        /* Load the parent constructor */
        parent::__construct();
    }

    public function attach() {
        $this->createRemoteObject(array($this->hostWindow));
        $this->setEventType('cycle', MSG_SEND);
        $this->setEventType('failed', MSG_SEND);
        $this->setEventType('finished', MSG_SEND);
        $this->setEventType('cancelled', MSG_SEND);
    }

    public function detach() {
        $this->unRegister();
    }

    public function setPaths($localPath, $serverPath) {
        $this->localPath = $localPath;
        $this->serverPath = $serverPath;
    }

    public function startUpload() {
        /* Check the local path and url */
        if (empty($this->localPath) || empty($this->serverPath)) {
            throw new SFException('No local path or server path given. Use setPaths()', ERR_REPORT_APP);
        }

        /* Start the upload */
        $this->callMethod('startUpload', array($this->localPath, $this->serverPath, intval($this->cycleStart)));
    }

    public function cancelUpload() {
         $this->callMethod('cancelUpload');
    }

    public function transferStart() {
        $this->_transferStarted = TRUE;
        $this->_transferFile = fopen($this->serverPath, 'w');
    }

    public function transferSetData($data) {
        /* Check the transfer state */
        if (!$this->_transferStarted) {
            throw new SFException('File transfer not started!');
        }

        /* Write the file */
        fwrite($this->_transferFile, $data);
    }

    public function transferEnd() {
        /* Close the file */
        fclose($this->_transferFile);

        /* Reset the values */
        $this->_transferStarted = FALSE;
        $this->_transferFile = NULL;
    }
}


/**
 * Background url downloader
 *
 * This class constructs a node that downloads a file from url to the client in
 * the background. Construct it with the url as the first argument and
 * the remote (client) path as the second argument, and call startDownload(). This control
 * fires a 'started' event when the download starts, a 'finished' event when the download
 * is finished and a 'cancelled' event when the download is cancelled.
 *
 * @see XULFilePicker
 * @package API
 * @subpackage FileHandling
*/
class URLDownloader extends Node {

    public $remoteConstructor = 'URLDownloader';

    public $url = NULL;
    public $destPath = NULL;

    /**
     * Constructor
     *
     * @param string    $url        The url of the file
     * @param string    $destPath   Destination file path on the client side
     */
    public function __construct($url = NULL, $destPath = NULL) {
        if ($url) {
            $this->url = $url;
        }

        if ($destPath) {
            $this->destPath = $destPath;
        }

        parent::__construct();
    }

    public function attach() {
        $this->createRemoteObject(array($this->hostWindow));
        $this->setEventType('failed', MSG_SEND);
        $this->setEventType('finished', MSG_SEND);
        $this->setEventType('cancelled', MSG_SEND);
    }

    public function detach() {
        $this->unRegister();
    }

    /**
     * Set the source and destination paths
     *
     * @param string    $url        The url of the file
     * @param string    $destPath   Destination file path on the client side
     */
    public function setPaths($url, $destPath) {
        $this->url = $url;
        $this->destPath = $destPath;
    }


    /**
     * Cancel a running download
     *
     * Cancels the downloader which fires the 'cancelled' event
    */
    public function cancelDownload() {
        $this->callMethod('cancelDownload');
    }

    /**
     * Start the download
     *
     * Starts the downloader which fires the 'started' event beforehand
    */
    public function startDownload() {
        if (empty($this->url) || empty($this->destPath)) {
            throw new SFException('No URL or path given. Use setPaths()', ERR_REPORT_APP);
        }

        $this->callMethod('startDownload', array($this->destPath, $this->url));
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
 *
 * @package API
 * @subpackage FileHandling
*/
class ClientFileService extends Node {

    const CRYPTO_MD2    = 1;
    const CRYPTO_MD5    = 2;
    const CRYPTO_SHA1   = 3;
    const CRYPTO_SHA256 = 4;
    const CRYPTO_SHA384 = 5;
    const CRYPTO_SHA512 = 6;

    public $remoteConstructor = 'FileService';
    private $_handlersList = array();
    private $_handlersCreateDirectory = array();
    private $_handlersRemoveDirectory = array();
    private $_handlersGetCryptoHash = array();
    private $_handlersExecuteFile = array();
    private $_handlersMonitorFile = array();
    private $_handlersGetFileInfo = array();
    private $_handlersRemoveFile = array();
    private $_handlersRenameFile = array();
    private $_handlersCopyFile = array();
    private $_handlersExtractZip = array();

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
     * - ClientFileService $fileService - this object
     * - bool $status: indicates whether the operation was successful
     * - ClientDirectory $dirFile: if successful, a file object referring to the remote directory with the directory contents in the ClientFile::$entries array
     *
     * @param string        $path               The path on the client side to read
     * @param object|string $handlerObj         The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod      The method to call on the handler object
    */
    public function getDirectory($path, $handlerObj, $handlerMethod) {
        $this->_handlersList[$path] = array( $handlerObj, $handlerMethod );
        $this->callMethod( 'getDirectory', array( $path, TRUE ) );
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
     * @param string        $id                 ID of the special folder, see link
     * @param object|string $handlerObj         The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod      The method to call on the handler object
     * @param bool          $retrieveContents   Retrieve the properties of the files in the specialDir
    */
    public function getSpecialDirectory($id, $handlerObj, $handlerMethod, $retrieveContents = TRUE) {
        $this->_handlersList[$id] = array( $handlerObj, $handlerMethod );
        $this->callMethod( 'getSpecialDirectory', array( $id, $retrieveContents ) );
    }


    /**
     * Creates the directory
     *
     * The handler receives these parameters:
     * - ClientFileService $fileService - this object
     * - bool $status: indicates whether the operation was successful
     * - ClientDirectory $dirFile: if successful, a file object referring to the new remote directory
     *
     * @param string        $path               The path on the client side to create
     * @param object|string $handlerObj         The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod      The method to call on the handler object
    */
    public function createDirectory($path, $handlerObj = NULL, $handlerMethod = NULL) {
        $this->_handlersCreateDirectory[$path] = ($handlerObj&&$handlerMethod ? array( $handlerObj, $handlerMethod ) : NULL);
        $this->callMethod( 'createDirectory', array( $path ) );
    }


    /**
     * Copies a file from server to client
     *
     * Returns the FileDownloader object to allow progress checking. The handler is attached to the 'finished', 'failed' and 'cancelled' events
     * of the FileDownloader.
     *
     * @param string        $localPath          The path on the server side
     * @param string        $clientPath         The path on the client side
     * @param object|string $handlerObj         The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod      The method to call on the handler object
     * @return FileDownloader $transfer         The transfer object keeping track of transfer progress
    */
    public function writeFile($localPath, $clientPath, $handlerObj = NULL, $handlerMethod = NULL) {

        $this->hostWindow->addChild( $fdn = new FileDownloader( $localPath, $clientPath ) );
        if ($handlerObj && $handlerMethod) {
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
     * @param string        $clientPath         The path on the client side
     * @param string        $data               Data to write to the file
     * @param object|string $handlerObj         The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod      The method to call on the handler object
     * @return FileFromStringDownloader $transfer           The transfer object keeping track of transfer progress
    */
    public function writeFileFromString($clientPath, $data, $handlerObj = NULL, $handlerMethod = NULL) {
        $this->hostWindow->addChild( $fdn = new FileFromStringDownloader( $clientPath, $data ) );
        if ($handlerObj && $handlerMethod) {
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
     * @param string        $clientPath         The path on the client side
     * @param resource      $stream             Stream to read from
     * @param int           $length             Amount of bytes to read from the stream and write to the file
     * @param object|string $handlerObj         The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod      The method to call on the handler object
     * @return FileFromStringDownloader $transfer           The transfer object keeping track of transfer progress
    */
    public function writeFileFromStream($clientPath, $stream, $length, $handlerObj = NULL, $handlerMethod = NULL) {
        $this->hostWindow->addChild( $fdn = new FileFromStreamDownloader( $clientPath, $stream, $length ) );
        if ($handlerObj && $handlerMethod) {
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
    public function onDownloaderFinished($event) {
        if ($event->sourceObject->isChild) {
            $event->sourceObject->removeNode();
        }
    }


    /**
     * Copies a file from client to server
     *
     * Returns the FileUploader object to allow progress checking. The handler is attached to the 'finished', 'failed' and 'cancelled' events
     * of the FileUploader.
     *
     * @param string        $clientPath         The path on the client side
     * @param string        $localPath          The path on the server side
     * @param object|string $handlerObj         The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod      The method to call on the handler object
     * @return FileUploader $transfer           The transfer object keeping track of transfer progress
    */
    public function readFile($clientPath, $localPath, $handlerObj = NULL, $handlerMethod = NULL) {
        $this->hostWindow->addChild( $fup = new FileUploader( $clientPath, $localPath ) );
        if ($handlerObj && $handlerMethod) {
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
     * @param string        $clientPath         The path on the client side
     * @param object|string $handlerObj         The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod      The method to call on the handler object
     * @return FileToStringUploader $transfer           The transfer object keeping track of transfer progress
    */
    public function readFileToString($clientPath, $handlerObj = NULL, $handlerMethod = NULL) {
        $this->hostWindow->addChild( $fup = new FileToStringUploader( $clientPath ) );
        if ($handlerObj && $handlerMethod) {
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
     * @param string        $clientPath         The path on the client side
     * @param resource      $stream             Stream to write to
     * @param object|string $handlerObj         The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod      The method to call on the handler object
     * @return FileToStreamUploader $transfer           The transfer object keeping track of transfer progress
    */
    public function readFileToStream($clientPath, $stream, $handlerObj = NULL, $handlerMethod = NULL) {
        $this->hostWindow->addChild( $fup = new FileToStreamUploader( $clientPath, $stream ) );
        if ($handlerObj && $handlerMethod) {
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
    public function onUploaderFinished($event) {
        if ($event->sourceObject->isChild) {
            $event->sourceObject->removeNode();
        }
    }


    /**
     * Removes the directory
     *
     * The handler receives these parameters:
     * - ClientFileService $fileService - this object
     * - bool $status: indicates whether the operation was successful
     * - string $path: the remote path
     *
     * @param string        $clientPath         The path on the client side to remove
     * @param bool          $recursive          If TRUE, all underlying contents are removed as well. If FALSE, the directory needs to be empty
     * @param object|string $handlerObj         The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod      The method to call on the handler object
    */
    public function removeDirectory($clientPath, $recursive, $handlerObj = NULL, $handlerMethod = NULL) {
        $this->_handlersRemoveDirectory[$clientPath] = ($handlerObj&&$handlerMethod ? array( $handlerObj, $handlerMethod ) : NULL);
        $this->callMethod( 'removeDirectory', array( $clientPath, (bool) $recursive ) );
    }


    /**
     * Removes the file
     *
     * The handler receives these parameters:
     * - ClientFileService $fileService - this object
     * - bool $status: indicates whether the operation was successful
     * - string $path: the remote path
     *
     * @param string        $clientPath         The path on the client side to remove
     * @param object|string $handlerObj         The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod      The method to call on the handler object
    */
    public function removeFile($clientPath, $handlerObj = NULL, $handlerMethod = NULL) {
        $this->_handlersRemoveFile[$clientPath] = ($handlerObj&&$handlerMethod ? array( $handlerObj, $handlerMethod ) : NULL);
        $this->callMethod( 'removeFile', array( $clientPath ) );
    }


    /**
     * Starts monitoring the file
     *
     * The handler receives these parameters when the file is created, removed or changed:
     * - ClientFileService $fileService - this object
     * - string $path: the remote path
     * - bool $exists: whether the file exists
     * - int $modificationTime: the modification time of the file
     * - int $size: file size
     *
     * @param string        $clientPath         The path on the client side to monitor
     * @param object|string $handlerObj         The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod      The method to call on the handler object
    */
    public function monitorFile($clientPath, $handlerObj, $handlerMethod) {
        if (isset($this->_handlersMonitorFile[$clientPath])) {
            throw new SFException( "Path $clientPath is already being monitored!", ERR_REPORT_APP );
        }

        $this->_handlersMonitorFile[$clientPath] = array( $handlerObj, $handlerMethod );
        $this->callMethod( 'monitorFile', array( $clientPath ) );
    }


    /**
     * Cancel monitoring the file
     *
     * @param string        $clientPath         The path on the client side to stop monitoring monitor
     */
    public function cancelMonitorFile ($clientPath) {
        $this->callMethod( 'cancelMonitorFile', array( $clientPath ));
        unset($this->_handlersMonitorFile[$clientPath]);
    }


    /**
     * Cancel all file monitors
     *
     */
    public function cancelAllMonitors () {
        $this->callMethod( 'cancelAllMonitors');
        $this->_handlersMonitorFile = array();
    }


    /**
     * Executes the file
     *
     * The handler receives these parameters:
     * - ClientFileService $fileService - this object
     * - bool $status: indicates whether the operation was successful
     * - string $path: the remote path
     *
     * @param string        $clientPath         The path on the client side to execute
     * @param array         $args               Arguments for the executable
     * @param bool          $async              If TRUE, the file is executed in the background. If FALSE, the application blocks until the executable ends
     * @param object|string $handlerObj         The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod      The method to call on the handler object
    */
    public function executeFile($clientPath, $args = array(), $async = TRUE, $handlerObj = NULL, $handlerMethod = NULL) {
        $this->_handlersExecuteFile[$clientPath] = ($handlerObj&&$handlerMethod ? array( $handlerObj, $handlerMethod ) : NULL);
        $this->callMethod( 'executeFile', array( $clientPath, count($args) ? $args : NULL, $async ) );
    }


    /**
     * Open the file with the native protocol handler for given file
     *
     * The handler receives these parameters:
     * - ClientFileService $fileService - this object
     * - string $path: the remote path
     *
     * @param string        $clientPath         The path on the client side to execute
     */
    public function openFileWithNativeProtocolHandler($clientPath) {
        $this->callMethod( 'openFileWithNativeProtocolHandler', array( $clientPath ));
    }


    /**
     * Open the file with the native protocol handler for given file
     *
     * The handler receives these parameters:
     * - ClientFileService $fileService - this object
     * - string $path: the remote path
     *
     * @param string        $uri            The URI to open using a native protocol handler
     */
    public function openURIWithNativeProtocolHandler($uri) {
        $this->callMethod( 'openURIWithNativeProtocolHandler', array( $uri ));
    }


    /**
     * Retreives the file info for a specific file
     *
     * The handler receives these parameters:
     * - ClientFileService $fileService  - this object
     * - bool $status: indicates whether the operation was successful
     * - ClientFile $file: the ClientFile object with file info
     *
     * @param string        $clientPath     The path on the client side
     * @param object|string $handlerObj     The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod  The method to call on the handler object
    */
    public function getFileInfo($clientPath, $handlerObj = NULL, $handlerMethod = NULL) {
        $this->_handlersGetFileInfo[$clientPath] = ($handlerObj&&$handlerMethod ? array( $handlerObj, $handlerMethod ) : NULL);
        $this->callMethod( 'getFileInfo', array($clientPath));
    }


    /**
     * Generates a hash based on the file located at the specified path, the following hash types are available :
     *      1 - MD2
     *      2 - MD5
     *      3 - SHA1
     *      4 - SHA256
     *      5 - SHA384
     *      6 - SHA512
     *
     * The handler receives these parameters:
     * - ClientFileService $fileService  - this object
     * - string $path: The path of the file where the hash was generated from
     * - bool $status: indicates whether the operation was successful
     * - string $hash: the string containing the returned hash (Empty if status is FALSE)
     *
     * @param string        $clientPath    The path leading to the file destination
     * @param int           $hashType      The type of hash to be generated
     * @param object|string $handlerObj    The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod The method to call on the handler object
    */
    public function getCryptoHash($clientPath, $hashType, $handlerObj = NULL, $handlerMethod = NULL) {
        $this->_handlersGetCryptoHash[$clientPath] = ($handlerObj&&$handlerMethod ? array( $handlerObj, $handlerMethod ) : NULL);
        $this->callMethod( 'getCryptoHash', array($clientPath, $hashType));
    }


    /**
     * Moves a file to the target directory
     *
     * The handler receives these parameters:
     * - ClientFileService $fileService  - this object
     * - bool $status: indicates whether the operation was successful
     * - string $newPath : The new path of the file
     *
     * @param string        $clientPath    The source path on the client side
     * @param string        $targetPath    The target path on the client side
     * @param object|string $handlerObj    The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod The method to call on the handler object
    */
    public function renameFile($clientPath, $targetPath, $handlerObj = NULL, $handlerMethod = NULL) {
        $this->_handlersRemoveFile[$clientPath] = ($handlerObj&&$handlerMethod ? array( $handlerObj, $handlerMethod ) : NULL);
        $this->callMethod( 'renameFile', array($clientPath, $targetPath));
    }

    /**
     * Copies a file to the target directory
     *
     * The handler receives these parameters:
     * - ClientFileService $fileService  - this object
     * - bool $status: indicates whether the operation was successful
     * - string $newPath : The path of the newly copied file
     *
     * @param string        $clientPath    The source path on the client side
     * @param string        $targetPath    The target path on the client side
     * @param object|string $handlerObj    The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod The method to call on the handler object
    */
    public function copyFile($clientPath, $targetPath, $handlerObj = NULL, $handlerMethod = NULL) {
        $this->_handlersCopyFile[$clientPath] = ($handlerObj&&$handlerMethod ? array( $handlerObj, $handlerMethod ) : NULL);
        $this->callMethod( 'copyFile', array($clientPath, $targetPath));
    }

    /**
     * Extract a zip file
     *
     * @param string        $clientPath    The source path on the client side
     * @param string        $targetPath    The target path on the client side
     * @param object|string $handlerObj    The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod The method to call on the handler object
    */
    public function extractZip($clientPath, $targetPath, $handlerObj = NULL, $handlerMethod = NULL) {
        $this->_handlersExtractZip[$clientPath] = ($handlerObj && $handlerMethod) ? array($handlerObj, $handlerMethod) : NULL;
        $this->callMethod('extractZip', array($clientPath, $targetPath));
    }

    /**
     * [INTERNAL FUNCTION] Event handler
    */
    public function onResult() {
        $args = func_get_args();
        $event = array_shift( $args );
        $op = array_shift( $args );
        $id = array_shift( $args );
        switch ($op) {
            case 'list':
                $status = array_shift( $args );
                $path = array_shift( $args );
                $files = array();
                if ($status) {
                    $dirFile = $this->fileFromResult( $path, $args[0] );
                    $dirFile->entries = array();
                    if (isset($args[1])) {
                        foreach ($args[1] as $file) {
                            $cf = $this->fileFromResult( $path.$this->getDirSeparator().$file[0], $file );
                            $dirFile->entries[$file[0]] = $cf;
                        }
                    }
                } else {
                    $dirFile = NULL;
                }

                if (isset($this->_handlersList[$id]) && is_array($this->_handlersList[$id])) {
                    call_user_func_array( $this->_handlersList[$id], array($this,$status,$dirFile) );
                }
            break;
            case 'createDirectory':
                $status = array_shift( $args );
                $path = array_shift( $args );
                if ($status) {
                    $file = array_shift( $args );
                    $dirFile = $this->fileFromResult( $path, $file );
                    $dirFile->entries = array();
                } else {
                    $dirFile = NULL;
                }
                if (isset($this->_handlersCreateDirectory[$id]) && is_array($this->_handlersCreateDirectory[$id])) {

                    call_user_func_array( $this->_handlersCreateDirectory[$id], array($this,$status,$dirFile) );
                }
            break;
            case 'removeDirectory':
                $status = array_shift( $args );
                $path = array_shift( $args );

                if (isset($this->_handlersRemoveDirectory[$id]) && is_array($this->_handlersRemoveDirectory[$id])) {

                    call_user_func_array( $this->_handlersRemoveDirectory[$id], array($this,$status,$path));
                }
            break;
            case 'removeFile':
                $status = array_shift( $args );
                $path = array_shift( $args );

                if (isset($this->_handlersRemoveFile[$id]) && is_array($this->_handlersRemoveFile[$id])) {

                    call_user_func_array( $this->_handlersRemoveFile[$id], array($this,$status,$path));
                }
            break;
            case 'fileChanged':
                $path = $id;
                list($exists,$modificationTime,$size) = $args;
                call_user_func_array( $this->_handlersMonitorFile[$path], array($this,$path,$exists,$modificationTime,$size) );
            break;
            case 'executeFile':
                list($status) = $args;

                if (isset($this->_handlersExecuteFile[$id]) && is_array($this->_handlersExecuteFile[$id])) {

                    call_user_func_array( $this->_handlersExecuteFile[$id], array($this,$status));
                }
            break;
            case 'getFileInfo':
                $status = array_shift( $args );
                $fileInfo = array_shift( $args );
                if (isset($this->_handlersGetFileInfo[$id]) && is_array($this->_handlersGetFileInfo[$id])) {
                    call_user_func_array( $this->_handlersGetFileInfo[$id], array($this, $status, ($status ? $this->fileFromResult($id, $fileInfo) : $fileInfo)));
                }
            break;
            case 'getCryptoHash':
                $status = array_shift( $args );
                $hash = array_shift( $args );

                if (isset($this->_handlersGetCryptoHash[$id]) && is_array($this->_handlersGetCryptoHash[$id])) {
                    call_user_func_array( $this->_handlersGetCryptoHash[$id], array($this, $id, $status, $hash));
                }
            break;
            case 'renameFile':
                $status = array_shift( $args );
                $newFile = array_shift( $args );

                if (isset($this->_handlersRenameFile[$id]) && is_array($this->_handlersRenameFile[$id])) {
                    call_user_func_array( $this->_handlersRenameFile[$id], array($this, $status, $newFile));
                }
            break;
            case 'copyFile':
                $status = array_shift( $args );
                $newFile = array_shift( $args );

                if (isset($this->_handlersCopyFile[$id]) && is_array($this->_handlersCopyFile[$id])) {
                    call_user_func_array( $this->_handlersCopyFile[$id], array($this, $status, $newFile));
                }
            break;
            case 'extractZip':
                if (!empty($this->_handlersExtractZip[$id])) {
                    list($status, $targetPath, $files) = $args;
                    call_user_func_array($this->_handlersExtractZip[$id], array($this, $status, $targetPath, $files));
                }
            break;
        }
    }

    public function fileFromResult($path, $file) {
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
        $cf->lastModifyTimestamp = $file[7];
        $cf->exists = TRUE;
        return $cf;
    }

    /**
     * Returns the directory separator for the current platform
     * @return string       The directory separator
    */
    static public function getDirSeparator() {
        if (stripos(ApplicationProcess::$PlatformInfo['platform'], 'win') !== FALSE) {
            return '\\';
        } else {
            return '/';
        }
    }

    /**
     * Returns the path supplied but converted to be compatible with the current platform
     * @param  string $path The path to be converted
     * @return string       The converted path
    */
    static public function ConvertToNativePath($path) {
        if (stripos(ApplicationProcess::$PlatformInfo['platform'], 'win') !== FALSE) {
            return str_replace('/', '\\', $path);
        } else {
            return str_replace('\\', '/', $path);
        }
    }
}


/**
 * Client filesystem class structure representing files (ClientFile) and directories (ClientDirectory) on the client side
 *
 * These classes form the object representation of the filesystem of the client and can be created from the
 * methods ClientFileService::getDirectory(), ClientFileService::getSpecialDirectory() and ClientFileService::createDirectory().
 *
 * @package API
 * @subpackage FileHandling
*/
class ClientBaseFile {

    public $fileService;
    public $path;
    public $name;
    public $isDirectory;
    public $isReadable;
    public $isWritable;
    public $isExecutable;
    public $isHidden;
    public $lastModifyTimestamp;
    public $exists = FALSE;

}


/**
 * @package API
 * @subpackage FileHandling
*/
class ClientFile extends ClientBaseFile {

    public $isDirectory = FALSE;
    public $size;


    /**
     * Copies a file from server to client
     *
     * Returns the FileDownloader object to allow progress checking. The handler is attached to the 'finished', 'failed' and 'cancelled' events
     * of the FileDownloader.
     *
     * @param string        $localPath          The path on the server side
     * @param object|string $handlerObj         The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod      The method to call on the handler object
     * @return FileDownloader $transfer         The transfer object keeping track of transfer progress
    */
    public function write($localPath, $handlerObj = NULL, $handlerMethod = NULL) {
        return $this->fileService->writeFile( $localPath, $this->path, $handlerObj, $handlerMethod );
    }


    /**
     * Copies a file from client to server
     *
     * Returns the FileUploader object to allow progress checking. The handler is attached to the 'finished', 'failed' and 'cancelled' events
     * of the FileUploader.
     *
     * @param string        $localPath          The path on the server side
     * @param object|string $handlerObj         The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod      The method to call on the handler object
     * @return FileUploader $transfer           The transfer object keeping track of transfer progress
    */
    public function read($localPath, $handlerObj = NULL, $handlerMethod = NULL) {
        return $this->fileService->readFile( $this->path, $localPath, $handlerObj, $handlerMethod );
    }


    /**
     * Stores a string in a file on the client side
     *
     * Returns the FileFromStringDownloader object to allow progress checking. The handler is attached to the 'finished', 'failed' and 'cancelled' events
     * of the FileFromStringDownloader.
     *
     * @param string        $data               Data to write to the file
     * @param object|string $handlerObj         The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod      The method to call on the handler object
     * @return FileFromStringDownloader $transfer           The transfer object keeping track of transfer progress
    */
    public function writeFromString($data, $handlerObj = NULL, $handlerMethod = NULL) {
        return $this->fileService->writeFileFromString( $this->path, $data, $handlerObj, $handlerMethod );
    }


    /**
     * Reads a file on the client side to a string
     *
     * Returns the FileToStringUploader object to allow progress checking. The handler is attached to the 'finished', 'failed' and 'cancelled' events
     * of the FileToStringUploader. The file contents are put in the FileToStringUploader::$data property
     *
     * @param object|string $handlerObj         The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod      The method to call on the handler object
     * @return FileToStringUploader $transfer           The transfer object keeping track of transfer progress
    */
    public function readToString($handlerObj = NULL, $handlerMethod = NULL) {
        return $this->fileService->readFileToString( $this->path, $handlerObj, $handlerMethod );
    }


    /**
     * Read from a stream and write to a file on the client side
     *
     * Returns the FileFromStreamDownloader object to allow progress checking. The handler is attached to the 'finished', 'failed' and 'cancelled' events
     * of the FileFromStreamDownloader.
     *
     * @param resource      $stream             Stream to read from
     * @param int           $length             Amount of bytes to read from the stream and write to the file
     * @param object|string $handlerObj         The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod      The method to call on the handler object
     * @return FileFromStreamDownloader $transfer           The transfer object keeping track of transfer progress
    */
    public function writeFromStream($stream, $length, $handlerObj = NULL, $handlerMethod = NULL) {
        return $this->fileService->writeFileFromStream( $this->path, $stream, $length, $handlerObj, $handlerMethod );
    }


    /**
     * Reads a file on the client side to a stream
     *
     * Returns the FileToStreamUploader object to allow progress checking. The handler is attached to the 'finished', 'failed' and 'cancelled' events
     * of the FileToStreamUploader.
     *
     * @param resource      $stream             Stream to write to
     * @param object|string $handlerObj         The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod      The method to call on the handler object
     * @return FileToStringUploader $transfer           The transfer object keeping track of transfer progress
    */
    public function readToStream($stream, $handlerObj = NULL, $handlerMethod = NULL) {
        return $this->fileService->readFileToStream( $this->path, $stream, $handlerObj, $handlerMethod );
    }

    /**
     * Removes the file
     *
     * The handler receives these parameters:
     * - ClientFileService $fileService - this object
     * - bool $status: indicates whether the operation was successful
     * - string $path: the remote path
     *
     * @param object|string $handlerObj         The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod      The method to call on the handler object
    */
    public function remove($handlerObj = NULL, $handlerMethod = NULL) {
        $this->fileService->removeFile( $this->path, $handlerObj, $handlerMethod );
    }


    /**
     * Starts monitoring the file
     *
     * The handler receives these parameters when the file is created, removed or changed:
     * - ClientFileService $fileService - this object
     * - string $path: the remote path
     * - bool $exists: whether the file exists
     * - int $modificationTime: the modification time of the file
     * - int $size: file size
     *
     * @param object|string $handlerObj         The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod      The method to call on the handler object
    */
    public function monitor($handlerObj, $handlerMethod) {
        $this->fileService->monitorFile( $this->path, $handlerObj, $handlerMethod );
    }


    /**
     * Executes the file
     *
     * The handler receives these parameters:
     * - ClientFileService $fileService - this object
     * - bool $status: indicates whether the operation was successful
     * - string $path: the remote path
     *
     * @param array         $args               Arguments for the executable
     * @param bool          $async              If TRUE, the file is executed in the background. If FALSE, the application blocks until the executable ends
     * @param object|string $handlerObj         The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod      The method to call on the handler object
    */
    public function execute($args = array(), $async = TRUE, $handlerObj = NULL, $handlerMethod = NULL) {
        $this->fileService->executeFile( $this->path, $args, $async, $handlerObj, $handlerMethod );
    }


    /**
    * Retreives the file info for a specific file
    *
    * The handler receives these parameters:
    * - ClientFileService $fileService  - this object
    * - bool $status: indicates whether the operation was successful
    * - ClientFile $file: the ClientFile object with file info
    *
    * @param object|string  $handlerObj         The object or classname to call the handler on when the operation finishes
    * @param string     $handlerMethod      The method to call on the handler object
    */
    public function getInfo($handlerObj = NULL, $handlerMethod = NULL) {
        $this->fileService->getFileInfo( $this->path, $handlerObj, $handlerMethod );
    }


    /**
     * Generates a hash based on the file, the following hash types are available :
     *      1 - MD2
     *      2 - MD5
     *      3 - SHA1
     *      4 - SHA256
     *      5 - SHA384
     *      6 - SHA512
     *
     * The handler receives these parameters:
     * - ClientFileService $fileService  - this object
     * - string $path: The path of the file where the hash was generated from
     * - bool $status: indicates whether the operation was successful
     * - string $hash: the string containing the returned hash (Empty if status is FALSE)
     *
     * @param  int           $hashType      The type of hash to be generated
     * @param  object|string $handlerObj    The object or classname to call the handler on when the operation finishes
     * @param  string        $handlerMethod The method to call on the handler object
     */
    public function getCryptoHash($hashType, $handlerObj = NULL, $handlerMethod = NULL) {
        $this->fileService->getCryptoHash( $this->path, $hashType, $handlerObj, $handlerMethod );
    }


    /**
     * Moves a file to the target directory
     *
     * The handler receives these parameters:
     * - ClientFileService $fileService  - this object
     * - bool $status: indicates whether the operation was successful
     * - string $newPath : The new path of the file
     *
     * @param string        $targetPath    The path on the client side
     * @param object|string $handlerObj    The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod The method to call on the handler object
    */
    public function rename($targetPath, $handlerObj = NULL, $handlerMethod = NULL) {
        $this->fileService->renameFile( $this->path, $targetPath, $handlerObj, $handlerMethod );
    }


    /**
     * Copies a file to the target directory
     *
     * The handler receives these parameters:
     * - ClientFileService $fileService  - this object
     * - bool $status: indicates whether the operation was successful
     * - string $newPath : The path of the newly copied file
     *
     * @param string        $targetPath    The path on the client side
     * @param object|string $handlerObj    The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod The method to call on the handler object
    */
    public function copy($targetPath, $handlerObj = NULL, $handlerMethod = NULL) {
        $this->fileService->copyFile( $this->path, $targetPath, $handlerObj, $handlerMethod );
    }
}


/**
 * @package API
 * @subpackage FileHandling
*/
class ClientDirectory extends ClientBaseFile {

    public $isDirectory = TRUE;
    public $entries = array();


    /**
     * Returns a new ClientDirectory representing a new directory in the current one
     *
     * @param string        $dirName        The directory name to append to the current path
     * @return ClientDirectory  The (new or existing) directory
    */
    public function appendDirectory($dirName) {
        if (isset($this->entries[$dirName])) {
            if ($this->entries[$dirName]->isDirectory) {
                return $this->entries[$dirName];
            } else {
                throw new SFException( "$dirName is a file", ERR_REPORT_APP );
            }
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
     * @param string        $fileName       The filename to append to the current path
     * @return ClientFile   The (new or existing) file
    */
    public function appendFile($fileName) {
        if (isset($this->entries[$fileName])) {
            if (!$this->entries[$fileName]->isDirectory) {
                return $this->entries[$fileName];
            } else {
                throw new SFException( "$fileName is a directory", ERR_REPORT_APP );
            }
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
     * - ClientFileService $fileService - this object
     * - bool $status: indicates whether the operation was successful
     * - ClientDirectory $dirFile: if successful, a file object referring to the new remote directory
     *
     * @param object|string $handlerObj         The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod      The method to call on the handler object
    */
    public function create($handlerObj = NULL, $handlerMethod = NULL) {
        $this->fileService->createDirectory( $this->path, $handlerObj, $handlerMethod );
    }


    /**
     * Gets the contents of a directory
     *
     * The handler receives these parameters:
     * - ClientFileService $fileService - this object
     * - bool $status: indicates whether the operation was successful
     * - ClientDirectory $dirFile: if successful, a file object referring to the remote directory with the directory contents in the ClientFile::$entries array
     *
     * @param object|string $handlerObj         The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod      The method to call on the handler object
    */
    public function get($handlerObj, $handlerMethod) {
        $this->fileService->getDirectory( $this->path, $handlerObj, $handlerMethod );
    }


    /**
     * Removes the directory
     *
     * The handler receives these parameters:
     * - ClientFileService $fileService - this object
     * - bool $status: indicates whether the operation was successful
     * - string $path: the remote path
     *
     * @param bool          $recursive          If TRUE, all underlying contents are removed as well. If FALSE, the directory needs to be empty
     * @param object|string $handlerObj         The object or classname to call the handler on when the operation finishes
     * @param string        $handlerMethod      The method to call on the handler object
    */
    public function remove($recursive = FALSE, $handlerObj = NULL, $handlerMethod = NULL) {
        $this->fileService->removeDirectory( $this->path, $recursive, $handlerObj, $handlerMethod );
    }
}
