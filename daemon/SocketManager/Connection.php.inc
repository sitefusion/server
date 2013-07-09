<?php

namespace SocketManager;

include_once('Message/Message.php.inc');
include_once('Message/ReadBuffer.php.inc');
include_once('Message/WriteBuffer.php.inc');

class Connection {

    private $isFirstRead = TRUE;

    /**
     *
     * @var resource
     */
    private $socket;

    /**
     *
     * @var int
     */
    private $port;

    /**
     *
     * @var string
     */
    private $remoteAddress = NULL;

    /**
     *
     * @var \SocketManager\Message\ReadBuffer
     */
    private $readBuffer = NULL;

    /**
     *
     * @var boolean
     */
    public $state;

    /**
     *
     * @var string
     */
    public $tag;

    /**
     *
     * @var string
     */
    public $name;

    /**
     *
     * @var int Timestamp
     */
    public $lastAlive = 0;

    /**
     *
     * @var float Timestamp when the socket was created
     */
    public $createdAt = 0;

    /**
     *
     * @var boolean
     */
    public $isDestroyed = FALSE;

    /**
     *
     * @var int Socket error code
     */
    public $lastError = 0;

    /**
     *
     * @var string Last error code in string format
     */
    public $lastErrorS = '';

    /**
     * Whether the socket should send keepalive messages
     * (Connecting side/Client side of $expectKeepAlives)
     * @var boolean
     */
    public $sendKeepAlives = FALSE;

    /**
     * Whether the client should send keepalive messages
     * (Accepted side/Host side of $sendKeepAlives)
     * @var boolean
     */
    public $expectKeepAlives = TRUE;

    /**
     * Resolve DNS names
     * @var boolean
     */
    public $resolveNames = TRUE;

    /**
     *
     * @var boolean
     */
    public $timeoutOverride = FALSE;

    /**
     * Free data to identify sockets further than name and tag
     * @var object
     */
    public $userData = NULL;

    /**
     *
     * @var string A packed { @link \SocketManager\Message } containing the keepalive message.
     */
    private static $KeepAliveMessage;

    /**
     *
     * @var array
     */
    private $writeQueue;

    const MODE_SERVERMAINSOCK = -1;
    const MODE_HOST = 0;
    const MODE_CLIENT = 1;

    /**
     *
     * @var type
     */
    public $mode = 0;
    private $readErrorCount = 0;
    private $readErrorTime = 0;
    private $writeErrorCount = 0;
    private $writeErrorTime = 0;

    const READ_ERROR_TIMEOUT = 0.2;
    const WRITE_ERROR_TIMEOUT = 0.2;

    /**
     * Constructs or sets a socket
     * @param resource|int $sockOrAfType Either a AF_* protocol family type, or a socket resource
     * @param int $sockType A SOCK_* socket type (Only if $sockOrAfType is AF_*)
     * @param int $solType a SOL_* protocol type (Only if $sockOrAfType is AF_*)
     */
    public function __construct($sockOrAfType = NULL) {

        if (self::$KeepAliveMessage === NULL) {
            $message = new \SocketManager\Message(array(1 => FALSE));
            self::$KeepAliveMessage = $message . ''; // force __toString
        }

        $this->writeQueue = array();

        if (is_resource($sockOrAfType)) {
            if (@stream_get_meta_data($sockOrAfType) !== FALSE) {
                stream_set_blocking($sockOrAfType, FALSE);

                $this->socket = $sockOrAfType;
                $this->state = TRUE;
                $this->lastAlive = microtime(TRUE);
            }
        }

        $this->createdAt = microtime(TRUE);
    }

    /**
     *
     * @param string $name
     * @return mixed
     */
//    public function __get($name) {
//        switch ($name) {
//            case "modeS":
//                if ($this->mode === -1)
//                    return "mainSock";
//                else if ($this->mode === 0)
//                    return "hostSock";
//                else
//                    return "clientSock";
//        }
//    }

    public function __toString() {
        return ($this->name ? $this->name : ($this->modeS . ' ' . $this->tag));
    }

    /**
     *
     * @return resource
     */
    public function getSocketResource() {
        return $this->socket;
    }

//    /**
//     *
//     * @param int $level
//     * @param int $optName
//     * @param int $optVal
//     * @return boolean
//     */
//    public function setOption($level, $optName, $optVal) {
//        return socket_set_option($this->socket, $level, $optName, $optVal);
//    }

    /**
     *
     * @return string
     */
    public function getConnectedIP() {
        return $this->remoteAddress;
    }

    /**
     *
     * @return string
     */
    public function getLocalIP() {

        $meta = stream_get_meta_data($this->socket);
        $address = $meta['uri'];

        return (is_array($meta) === TRUE ? $address : NULL);
    }

    /**
     *
     * @param string $address
     * @param string $port
     * @return boolean
     */
    public function connect($address, $port = 0, $encrypted = FALSE) {
        if (!$this->state) {

            $this->remoteAddress = $address;
            $this->port = $port;

            $this->client = TRUE;

            $context = stream_context_create();
            if ($encrypted) {
                stream_context_set_option($context, 'ssl', 'allow_self_signed', TRUE);
                \Utility::Log("@Socket->Connect - Connection is SSL (TLS)");
            }

            $proto = ($encrypted ? "tls" : "tcp");
//            // flags = STREAM_CLIENT_ASYNC_CONNECT

            $this->socket = @stream_socket_client("$proto://$address:$port", $this->lastError, $this->lastErrorS, ini_get("default_socket_timeout"), STREAM_CLIENT_CONNECT, $context);

            if ($this->socket === FALSE) {
                return FALSE;
            }

            stream_set_blocking($this->socket, FALSE);
            $this->sendKeepAlives = TRUE;

            $this->state = TRUE;
            $this->lastAlive = microtime(TRUE);
            return TRUE;
        }

        return TRUE;
    }

    /**
     * @return \SocketManager\Connection
     */
    public function accept() {
        $name = '';
        $cli = stream_socket_accept($this->socket, 1, $name);

        if ($cli !== FALSE) {
            if (isset($this->encrypted))
                stream_socket_enable_crypto($cli, TRUE, STREAM_CRYPTO_METHOD_TLS_SERVER);


            $newCli = new \SocketManager\Connection($cli);

            $parts = explode(':', $name);
            $newCli->remoteAddress = $parts[0];

            $newCli->mode = 1;
            $newCli->lastAlive = microtime(TRUE);
            $newCli->state = TRUE;
            return $newCli;
        }
        return FALSE;
    }

    /**
     * Reads the socket
     * @param bool $mode TRUE if you expect Message-type input, else FALSE. Default TRUE.
     * @param int $length Bytecount of socket read (if $mode = FALSE)
     * @return array|string|bool
     */
    public function read($mode = TRUE, $length = 4096) {
        if ($this->state) {
            if (!$mode) {
                return $this->readString($length);
            } else {
                return $this->readMessage();
            }
        }
        \Utility::LogToFile("FrontendReader", "Returning false on socket->read() as ->state = FALSE");
        return FALSE;
    }

    /**
     *
     * @return array|boolean|null
     */
    private function readMessage() {

        if ($this->readBuffer === NULL) {
            $this->readBuffer = new \SocketManager\Message\ReadBuffer();
        }

        if (\SocketManager::$LimitFirstMessageSize && $this->isFirstRead) {
            if ($this->readBuffer->currentMessageSize >= \SocketManager::$LimitFirstMessageSizeBytes) {
                return FALSE;
            }
        }

        $rsize = min($this->readBuffer->readSize, 8192);
        $in = fread($this->socket, $rsize);

        $eCode = $this->lastError;

        if ($in === FALSE) {
            if (feof($this->socket)) {
                \Utility::Log("FEOF on socket {$this->name}.{$this->tag}");
                $retVal = FALSE;
            }
            $retVal = FALSE;
        } else if ($in === '') {
            if ($this->mode == self::MODE_CLIENT && $eCode == 115) {
                $retVal = FALSE;
            } else if ($eCode == 0) {
                $retVal = FALSE;
            } else {
                if ($eCode == 32) {
                    $retVal = FALSE;
                } else {
                    $retVal = NULL;
                }
            }
        } else {
            $this->readBuffer->add($in);
            if ($this->readBuffer->isComplete()) {
                $retVal = $this->readBuffer->getMessage();
                $this->readBuffer = NULL;
            } else {
                \Utility::LogToFile("FrontendReader", "ClientSocket got content, message not complete yet");
                $retVal = NULL;
            }
        }

        if ($retVal === FALSE || $retVal === NULL) {
            if ($this->readError()) {
                \Utility::LogToFile("FrontendReader", "Negative readvalue for too long => returning FALSE");
                $retVal = FALSE;
            } else {
                \Utility::LogToFile("FrontendReader", "Negative readvalues still within range => returning NULL");
                $retVal = NULL;
            }
        } else {
            \Utility::LogToFile("FrontendReader", "Positive readvalue retrieved => returning read value");
            $this->unsetReadError();
            $this->lastAlive = microtime(TRUE);
        }

        $this->isFirstRead = FALSE;

        return $retVal;
    }

    public function read_now() {
        $readArray = array($this->socket);
        $writeArray = array();
        $exceptArray = array();

        $ret = FALSE;
        do {
            $read = $this->readMessage();

            if ($read === FALSE) {
                $ret = FALSE;
                break;
            } else if ($ret !== NULL) {
                $ret = $read;
                break;
            }


            $readArray = array($this->socket);
        } while (@stream_select($readArray, $writeArray, $exceptArray, 0) > 0);

        if (is_string($ret) && $ret == '') {
            if (feof($this->socket)) {
                return FALSE;
            }
        }

        return $ret;
    }

    public function read_allowed($now = NULL) {
//        if ($this->readErrorCount && $this->readErrorTime) {
//
//            if (!$now)
//                $now = microtime(TRUE);
//
//            if ($now - $this->readErrorTime > self::READ_ERROR_TIMEOUT) {
//                return TRUE;
//            }
//            return FALSE;
//        }
        return TRUE;
    }

    private function readError() {
        if (++$this->readErrorCount == 33) {
            return TRUE;
        }

        $this->readErrorTime = microtime(TRUE);
        return FALSE;
    }

    private function unsetReadError() {
        $this->readErrorCount = 0;
        $this->readErrorTime = 0;
    }

    public function write_allowed($now) {
        if ($this->writeErrorCount && $this->writeErrorTime) {
            if (!$now)
                $now = microtime(TRUE);

            if ($now - $this->writeErrorTime > self::WRITE_ERROR_TIMEOUT) {
                return TRUE;
            }
            return FALSE;
        }
        return TRUE;
    }

    private function writeError() {
        if (++$this->writeErrorCount == 33) {
            return TRUE;
        }

        $this->writeErrorTime = microtime(TRUE);
        return FALSE;
    }

    private function unsetWriteError() {
        $this->writeErrorCount = 0;
        $this->writeErrorTime = 0;
    }

    public function write($message) {
        if (is_array($message))
            $message = new \SocketManager\Message($message);

        if (!is_string($message))
            $message = $message . '';

        array_push($this->writeQueue, new \SocketManager\Message\WriteBuffer($message));
    }

    public function write_hasBuffer() {
        return (count($this->writeQueue) > 0);
    }

    /**
     *
     * @return \SocketManager\MessageBufferWrite
     */
    public function write_getCurrentMessage() {
        $arrP = array_slice($this->writeQueue, 0, 1, false);
        return $arrP[0];
    }

    public function write_checkDone() {
        if ($this->write_getCurrentMessage()->isDone()) {
            array_shift($this->writeQueue);
            return TRUE;
        }
        return FALSE;
    }

    /**
     *
     * @param MessageBufferWrite $message
     * @return int|bool
     */
    public function write_doWrite($message) {
        $mPart = $message->getMessagePart();

        $wsize = min(strlen($mPart), 8192);
        $written = @fwrite($this->socket, $mPart, $wsize);

        if ($written === FALSE) {
            if ($this->writeError())
                return FALSE;
            else
                return NULL;
        } else if ($written === 0) {
            if ($this->writeError())
                return FALSE;
            else
                return NULL;
        } else {
            $this->unsetWriteError();
            $message->movePointer($written);

            return $written;
        }
    }

    /**
     * @param int $length
     * @return string|null|boolean
     */
    private function readString($length) {
        if ($this->state) {
            $buffer = NULL;

            $buffer = fread($this->socket, $length);

            if ($buffer === FALSE) {
                if ($this->readError())
                    return FALSE;
            } else if ($buffer === '') {
                if ($this->readError())
                    return FALSE;
            } else {
                $this->unsetReadError();
                $this->lastAlive = microtime(TRUE);

                return $buffer;
            }
        }
        return FALSE;
    }

    /**
     *
     * @param int $keepAlivePeriod
     * @return boolean
     */
    public function checkKeepAlive($keepAlivePeriod = 1) {

        if ($this->state) {
            if ($this->sendKeepAlives) {

                $diff = microtime(TRUE) - $this->lastAlive;

                if ($diff >= $keepAlivePeriod) {
                    $this->write(self::$KeepAliveMessage);
                    $this->lastAlive = microtime(TRUE);
                }
            }

            return TRUE;
        }

        return FALSE;
    }

    /**
     *
     */
    public function destroy() {
        if (!$this->isDestroyed) {
            $this->state = FALSE;

            $this->isDestroyed = TRUE;
            if (is_resource($this->socket)) {
                fclose($this->socket);
                $this->socket = NULL;
            }
        }
    }

}

