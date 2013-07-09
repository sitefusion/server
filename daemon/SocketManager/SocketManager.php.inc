<?php

include_once('Connection.php.inc');
include_once('AUpdate.php.inc');

use \SocketManager;

class SocketManager {

    static public $LimitFirstMessageSize = TRUE;
    static public $LimitFirstMessageSizeBytes = 10000;
    static private $instance;

    /**
     *
     * @return \SocketManager
     */
    static public function GetInstance($domain = AF_INET, $commType = SOCK_STREAM, $protocol = SOL_TCP, $keepAlive = 3, $timeout = 60) {
        if (!self::$instance) {
            self::$instance = new self();
            self::$instance->init($domain, $commType, $protocol, $keepAlive, $timeout);
        }

        return self::$instance;
    }

    static public function RemoveInstance() {
        if (self::$instance)
            self::$instance->dispose();
        self::$instance = NULL;
    }

	static public function SetNewInstance($domain = AF_INET, $commType = SOCK_STREAM, $protocol = SOL_TCP, $keepAlive = 3, $timeout = 60) {
		self::$instance = NULL;
		return self::$instance = self::GetInstance($domain, $commType, $protocol, $keepAlive, $timeout);
	}

    /**
     *
     * @param int $domain
     * @param int $commType
     * @param int $protocol
     * @param int $keepAlive
     * @param int $timeout
     * @return SocketManager
     */
    static public function NewInstance($domain = AF_INET, $commType = SOCK_STREAM, $protocol = SOL_TCP, $keepAlive = 3, $timeout = 60) {
        $newInstance = new self();
        $newInstance->init($domain, $commType, $protocol, $keepAlive, $timeout);
        return $newInstance;
    }

    public function init($domain, $type, $protocol, $keepAliveTime, $maxTimeout) {
        if (!$this->initialized) {
            $this->initialized = TRUE;
            $this->domain = $domain;
            $this->type = $type;
            $this->protocol = $protocol;
            $this->keepAliveTime = $keepAliveTime;
            $this->maxTimeout = $maxTimeout;

            $this->socketResources = array();
            $this->socketObjects = array();
            $this->temporarySocketObjects = array();
            $this->options = array();
        }
    }

    private $initialized = FALSE;
    private $keepAliveTime;
    private $maxTimeout;

    /**
     * Array containing all the resources of the SocketConnections managed by this, indexed on name
     * @var resource[]
     */
    private $socketResources;

    /**
     * Array containing all the SocketConnection instances managed by this, indexed on name
     * @var \SocketManager\Connection[]
     */
    public $socketObjects;

    /**
     * Temporary accepted SocketConnection objects will be stored here until the implementor decides whether to accept or deny them.
     * @var \SocketManager\Connection[]
     */
    public $temporarySocketObjects;

    /**
     *
     * @var int AF_INET|AF_INET6|AF_UNIX
     */
    private $domain;

    /**
     *
     * @var int SOCK_STREAM|SOCK_DRAM|SOCK_RAW|SOCK_SEQPACKET
     */
    private $type;

    /**
     *
     * @var int SOL_SOCKET|SOL_TCP|SOL_UDP
     */
    private $protocol;

    /**
     *
     * @var array
     */
    private $options;

    /**
     *
     * @var Closure function to call when a connection is set up successfully (connecting side)
     */
    private $onSuccessfulConnect;

    /**
     *
     * @var Closure function to call when a new client connected and is accepted
     */
    private $onNewConnection;

    /**
     *
     * @var Closure function to call when a connection is lost (either side)
     */
    private $onConnectionDropped;

    /**
     *
     * @var Closure function to call which returns a name based on IP address
     */
    private $onNameResolve;

    /**
     *
     * @var boolean
     */
    public $triggerEvents = TRUE;
    private $removeAfterWrite = array();

    public function __destruct() {
        $this->dispose();
    }

    public function dispose() {
        foreach ($this->socketObjects as $sock) {
            $sock->destroy();
        }
        $this->socketObjects = array();
        $this->socketResources = array();

        foreach ($this->temporarySocketObjects as $temp) {
            $temp->destroy();
        }

        $this->temporarySocketObjects = array();
    }

    /**
     *
     * @param Closure $function
     * @throws Exception
     */
    public function setEventHandler_OnSuccessfulConnect($function) {
        $funcCheck = new \ReflectionFunction($function);
        if ($funcCheck->getNumberOfParameters() != 1) {
            throw new Exception("Eventhandlers must have exactly 1 parameter: f(array \$options);");
        }
        $pars = $funcCheck->getParameters();
        if (!$pars[0]->isArray()) {
            throw new Exception("Parameter needs to be an array. Array=\$a[\$sock,\$eventDetails]");
        }

        $this->onSuccessfulConnect = $function;
    }

    /**
     *
     * @param \Closure $function
     * @throws Exception
     */
    public function setEventHandler_OnNewConnection(Closure $function) {
        $funcCheck = new \ReflectionFunction($function);
        if ($funcCheck->getNumberOfParameters() != 1) {
            throw new Exception("Eventhandlers must have exactly 1 parameter: f(array \$options);");
        }
        $pars = $funcCheck->getParameters();
        if (!$pars[0]->isArray()) {
            throw new Exception("Parameter needs to be an array. Array=\$a[\$sock,\$eventDetails]");
        }

        $this->onNewConnection = $function;
    }

    /**
     *
     * @param \Closure $function
     * @throws Exception
     */
    public function setEventHandler_OnConnectionDropped($function) {
        $funcCheck = new \ReflectionFunction($function);
        if ($funcCheck->getNumberOfParameters() != 1) {
            throw new \Exception("Eventhandlers must have exactly 1 parameter: f(array \$options);");
        }
        $pars = $funcCheck->getParameters();
        if (!$pars[0]->isArray()) {
            throw new \Exception("Parameter needs to be an array. Array=\$a[\$sock,\$eventDetails]");
        }

        $this->onConnectionDropped = $function;
    }

    private $nameResArgs = 1;
    public function setNameResolver($function) {
        $funcCheck = new \ReflectionFunction($function);
        if ($funcCheck->getNumberOfParameters() < 1 || $funcCheck->getNumberOfParameters() > 2) {
            throw new \Exception("Eventhandlers must have exactly 1 or 2 parameters: f(string \$IP, [SocketConnection $hostSock]);");
        }

        $this->nameResArgs = $funcCheck->getNumberOfParameters();

        $this->onNameResolve = $function;
    }

    /**
     *
     * @param array $par
     */
    private function trigger_OnSuccessfulConnect($par) {
        if ($this->onSuccessfulConnect !== NULL && $this->triggerEvents) {
            $f = $this->onSuccessfulConnect;
            $f($par);
        }
    }

    /**
     *
     * @param array $par
     */
    private function trigger_OnNewConnection($par) {
        if ($this->onNewConnection !== NULL && $this->triggerEvents) {
            $f = $this->onNewConnection;
            $f(array($par, "newConn"));
        }
    }

    /**
     *
     * @param array $par
     */
    private function trigger_onConnectionDropped($par) {
        if ($this->onConnectionDropped !== NULL && $this->triggerEvents) {
            $f = $this->onConnectionDropped;
            $f($par);
        }
    }

    /**
     *
     * @param string $ip
     * @return string $name
     */
    private function nameResolver($ip, $sock = NULL) {
        if ($this->onNameResolve !== NULL) {
            $f = $this->onNameResolve;
            if($this->nameResArgs == 1) {
                return $f($ip);
            } else if ($this->nameResArgs == 2) {
                return $f($ip, $sock);
            }
        }
        \Utility::Log("\n\n\nnameResolver did not return a valid name or was not set\n\n\n",0, 35);
        return '_TMP_ACCEPT';
    }

    public function renameSocket($socket, $newName) {
        if (is_string($socket)) {
            $socket = @$this->socketObjects[$socket];
        } else if (is_object($socket)) {
            $socket = @$this->socketObjects[$socket->name];
        } else {
            throw new \Exception("Unsupported search type");
        }

        if (!$socket) {
            throw new \Exception("Unknown socket");
        }

        unset($this->socketObjects[$socket->name]);
        unset($this->socketResources[$socket->name]);

        $socket->name = $newName;

        $this->addSocket($socket);
    }

    /**
     * Sets the options (overrides any previous settings)
     *
     * <b>Example</b>:
     * array( <br/>
     *  SOL_SOCKET => array(<br/>
     *      SO_DEBUG => 1,<br/>
     *      SO_RCVTIMEO => array("sec"=>1, "usec"=>500000)<br/>
     *  ),<br/>
     *  SOL_TCP => array(<br/>
     *      TCP_NODELAY => 1<br/>
     *  )
     * );
     * @param array $options
     */
    public function setOptions(array $options) {

        foreach ($this->socketResources as $socketRes) {
            foreach ($options as $level => $opts) {
                foreach ($opts as $optName => $optValue) {
                    socket_set_option($socketRes, $level, $optName, $optValue);
                }
            }
        }

        $this->options = $options;
    }

    /**
     *
     * @param \SocketManager\Connection|resource $socket
     * @param array $options
     */
    public function setOptions_Socket($socket, $options = NULL) {
        if (!is_resource($socket) && is_object($socket) && $socket instanceof SocketManager\Connection) {
            $socket = $socket->getSocketResource();
        }

        if ($options === NULL)
            $options = $this->options;

        foreach ($options as $level => $opts) {

            foreach ($opts as $optName => $optValue) {
                socket_set_option($socket, $level, $optName, $optValue);
            }
        }
    }

    /**
     *
     * @param string $sockName
     * @param string $ipAddress
     * @param int $port
     * @return boolean $success
     */
    public function newConnection($sockName, $ipAddress, $port, $encrypted = FALSE) {

        if (array_key_exists($sockName, $this->socketObjects))
            return TRUE;

        $sockObj = new \SocketManager\Connection();
        $success = $sockObj->connect($ipAddress, $port, $encrypted);

        $sockObj->name = $sockName;

        if ($success) {
            $sockObj->sendKeepAlives = TRUE;
            //\Utility::Log("Connected to $ipAddress:$port, identifying connection as '$sockName'.");
            $this->addSocket($sockObj);
            $this->trigger_OnSuccessfulConnect(array($sockObj, "success"));
            return TRUE;
        } else {
            //\Utility::Log("Failed to connect to $ipAddress:$port.");
            $sockObj->destroy();
            return FALSE;
        }
    }

    /**
     *
     * @param type $find
     * @return \SocketManager\Connection|null
     */
    public function findSocket($find) {
        if (is_resource($find)) {

            foreach ($this->socketObjects as $socketOb) {
                if ($socketOb->getSocketResource() == $find) {
                    return $socketOb;
                }
            }

        } else if (is_string($find)) {

            foreach ($this->socketObjects as $socketOb) {
                if ($socketOb->name == $find) {
                    return $socketOb;
                }
            }

        } else {
            \Utility::Log("SocketManager::findSocket() - Unsupported type: " . gettype($find) . "." . @get_class($find));
            return NULL;
        }
    }

    /**
     * Finds out whether the given socket, searched by name, is alive
     * @param string $query a name, or an IP
     * @param int $mode 1 for name-search, 2 = ipmode
     * @return bool
     */
    public function haveSocket($query, $mode = 1) {
        if ($mode == 1) {
            return isset($this->socketObjects[$query]);
        } else {
            foreach ($this->socketObjects as $sock) {
                if ($sock->getConnectedIp() == $query)
                    return TRUE;
            }
        }
        return FALSE;
    }

    /**
     *
     * @param SocketManager\Connection $socketOb
     */
    public function addSocket(SocketManager\Connection $socketOb) {
        $this->socketObjects[$socketOb->name] = $socketOb;
        $resource = $socketOb->getSocketResource();
		if(!is_resource($resource))
			throw new \Exception("Tried to add non-resource to SocketManager!");
        $this->socketResources[$socketOb->name] = $resource;
    }

    /**
     *
     * @param resource|string|\SocketManager\Connection $par
     */
    public function removeSocket($par) {

        $name = FALSE;
        if (is_resource($par)) {
            foreach ($this->socketObjects as $socketOb) {
                if ($socketOb->getSocketResource() == $par) {
                    $name = $socketOb->name;
                }
            }
        } else if (is_string($par)) {

            if (isset($this->socketObjects[$par])) {
                $name = $par;
            }
        } else if (is_object($par) && $par instanceof SocketManager\Connection) {

            $name = $par->name;
        }

        if ($name) {

            if (isset($this->socketObjects[$name])) {
                unset($this->socketResources[$name]);
                $this->socketObjects[$name]->destroy();
                $b = $this->socketObjects[$name];
                unset($this->socketObjects[$name]);

                if ($name !== "_TMP_ACCEPT")
                    $this->trigger_onConnectionDropped(array($b, "removed"));
            } else {
                \Utility::Log("Tried removing unknown socket ($name)");
            }

            return TRUE;
        }
        return FALSE;
    }

    public function unassociateSocket($socket) {
        $name = NULL;
        if ($socket instanceof \SocketManager\Connection) {
            $name = $socket->name;
        } else if (gettype($socket) == 'string') {
            $name = $socket;
        } else {
            throw new Exception("Invalid type given for unassociateSocket() - Valid types: String / \\SocketManager\\Connection");
        }

        if (array_key_exists($name, $this->socketObjects)) {
            $socketOb = $this->socketObjects[$name];

            unset($this->socketObjects[$name]);
            unset($this->socketResources[$name]);

            return $socketOb;
        } else {
            throw new Exception("Unknown socket");
        }
    }

    /**
     *
     * @param string $sockName
     * @param string $ipAddress
     * @param int $port
     */
    public function openHost($sockName, $ipAddress, $port, $sslCert = NULL) {

        $ssl = ($sslCert != NULL && file_exists($sslCert));

        $proto = ($ssl ? 'tls' : 'tcp');
        $context = stream_context_create();

        if ($ssl) {
            stream_context_set_option($context, 'ssl', 'allow_self_signed', TRUE);
            stream_context_set_option($context, 'ssl', 'local_cert', $sslCert);
        }

        $eCode = 0;
        $eString = '';

        $res = stream_socket_server("$proto://$ipAddress:$port", $eCode, $eString, STREAM_SERVER_BIND | STREAM_SERVER_LISTEN, $context);
        if ($res) {
            \Utility::Log("Opened host '$sockName' on $ipAddress:$port");
            stream_set_blocking($res, FALSE);
            $sockObj = new \SocketManager\Connection($res);
            $sockObj->lastError = &$eCode;
            $sockObj->lastErrorS = &$eString;
            $sockObj->mode = \SocketManager\Connection::MODE_SERVERMAINSOCK;
            $sockObj->name = $sockName;
            if ($ssl)
                $sockObj->encrypted = TRUE;

            $sockObj->state = TRUE;
            $this->socketResources[$sockName] = $res;
            $this->socketObjects[$sockName] = $sockObj;

            return $sockObj;
        }

        throw new \Exception("Failed to open host successfully.");
    }

    /**
     *
     * @param \SocketManager\Connection|string $socket
     * @param array|string|Message $message
     * @return boolean
     */
    public function writeTo($socket, $message, $removeAfterWrite = FALSE) {
        $sockOb = NULL;
        if (is_string($socket)) {
            $sockOb = @$this->socketObjects[$socket];
        } else if (is_object($socket)) {
            if ($socket instanceof SocketManager\Connection) {
                $name = $socket->name;
                if (isset($this->socketObjects[$name])) {
                    if ($this->socketObjects[$name] === $socket) {
                        $sockOb = $socket;
                    } else {
                        \Utility::Log("Unknown socket ({$socket}).");
                        return FALSE;
                    }
                } else {
                    \Utility::Log("Tried to write to a nonexisting socket ($name).");
                    return FALSE;
                }
            }
        }

        if (!$sockOb) {
            \Utility::Log("No Socket Object with given name ({$socket})");
            return FALSE;
        }

        if(!$this->removeAfterWrite) {
            unset($this->removeAfterWrite[$sockOb->name]);
        }

        if (array_key_exists($sockOb->name, $this->removeAfterWrite)) {
            \Utility::Log("Socket exists in removeAfterWrite");
            return FALSE;
        }

        $sockOb->write($message);
        if ($removeAfterWrite) {
            $this->removeAfterWrite[$sockOb->name] = 1;
        }

        return TRUE;
    }

    /**
     * Writes the given message to the socket, not returning until all data has been written
     * @param \SocketManager\Connection $socket The connection to write the message on
     * @param SocketManager\Message|array $message The message to write (either Message or array format)
     * @param boolean $removeAfterWrite Whether the socket needs to be removed after writing
     * @return boolean
     */
    public function writeTo_Wait($socket, $message, $removeAfterWrite = FALSE) {


        try {

            \Utility::LogToFile('FrontEnd', "In writeTo_Wait()");

            if (is_string($socket)) {
                $sockOb = @$this->socketObjects[$socket];
            } else if (is_object($socket)) {
                if ($socket instanceof SocketManager\Connection) {
                    if (@$this->socketObjects[$socket->name] == $socket) {
                        $sockOb = $socket;
                    }
                }
            }

            if (@!$sockOb) {
                \Utility::Log("No socket found for writeTo_Wait");
                \Utility::SmartDump($socket, 0, "socket for writeTo_Wait");
                \Utility::SmartDump(array_keys($this->socketObjects), 0, "all sockets in arr");
                return FALSE;
            }

            if(!$removeAfterWrite) {
                unset($this->removeAfterWrite[$sockOb->name]);
            }

            if (array_key_exists($sockOb->name, $this->removeAfterWrite))
                return FALSE;

            $sockOb->write($message);

            $totWrite = 0;
            $eCount = 0;

            $writeA = array($sockOb->getSocketResource());
            \Utility::LogToFile('FrontEnd', \Utility::SmartDump($writeA, 0, "sockets in select", NULL, TRUE));
            $read = $except = NULL;

            \Utility::LogToFile('FrontEnd', "About to enter writeloop");

            do {
                \Utility::LogToFile('FrontEnd', "Have content in buffer");

                $write = @stream_select($read, $writeA, $except, 0, 50000);

                \Utility::LogToFile('FrontEnd', "after select");

                if ($write) {
                    \Utility::LogToFile('FrontEnd', "Can write according to stream_select ($write)");
                    $part = $sockOb->write_getCurrentMessage();
                    $w = $sockOb->write_doWrite($part);
                    \Utility::LogToFile('FrontEnd', "Write $w bytes");
                    if (!$w) {
                        $eCount++;
                    } else {
                        $totWrite += $w;
                    }
                } else {
                    \Utility::LogToFile('FrontEnd', "Could not write according to write (error: " . error_get_last() . ")");
                    \Utility::LogToFile('FrontEnd', \Utility::SmartDump($write, 0, "", NULL, TRUE));

                    $eCount++;
                }

                if ($eCount == 4) {
                    \Utility::LogToFile('FrontEnd', "Returning - eCount = 4");
                    return FALSE;
                }

                if (!$sockOb->write_checkDone()) {
                    $writeA = array($sockOb->getSocketResource());
                    \Utility::LogToFile('FrontEnd', "Still have content after trying write - redoing loop");
                }
            } while ($sockOb->write_hasBuffer() && !$sockOb->write_checkDone());

            \Utility::LogToFile('FrontEnd', "... Done?");

            if ($removeAfterWrite)
                $this->removeSocket($sockOb);
        } catch (\Exception $e) {
            \Utility::SmartDump($e);
        }


        \Utility::LogToFile('FrontEnd', "TotWrite = $totWrite");
        return $totWrite;
    }

    /**
     *
     * @param \SocketManager\Connecting $sock
     * @param string $name
     */
    public function acceptClient($sock, $name) {
        $k = array_keys($this->temporarySocketObjects);
        $found = FALSE;
        foreach ($k as $socketKey) {
            if ($this->temporarySocketObjects[$socketKey] == $sock) {
                unset($this->temporarySocketObjects[$socketKey]);
                $found = TRUE;
            }
        }

        if (!$found)
            throw new Exception("Couldn't find socket to accept");

        $sock->name = $name;
        $this->socketObjects[$name] = $sock;
        $this->trigger_OnNewConnection($sock);
    }

    public function denyClient($sock) {
        $k = array_keys($this->temporarySocketObjects);
        $found = FALSE;
        foreach ($k as $socketKey) {
            if ($this->temporarySocketObjects[$socketKey] == $sock) {
                unset($this->temporarySocketObjects[$socketKey]);
                $found = TRUE;
            }
        }

        if (!$found)
            throw new Exception("Couldn't find socket to deny.");

        $sock->destroy();
    }

    /**
     *
     * @param float $timeOut
     * @return SocketManager\AUpdate[]
     */
    public function update($timeOut) {
        // Split the float-timeout into a seconds part and a µ-seconds part
        $timeInt = (int) floor($timeOut);
        $timePart = fmod($timeOut, 1) * 1000000;

        $now = microtime(TRUE);

        $read = array();
        $write = array();

        $nonAllowedWrites = array();

        foreach ($this->socketObjects as $socket) {
            if ($socket->name == '_TMP_ACCEPT')
                throw new \Exception("Always handle all new connections immediately.");

            // If there's a read block on the socket, don't grab it for selecting
            if ($socket->read_allowed($now) && !array_key_exists($socket->name, $this->removeAfterWrite)) {
                $read[] = $socket->getSocketResource();
            }

            // Check whether the socket needs to send a KeepAlive packet
            $socket->checkKeepAlive($this->keepAliveTime);

            // Check whether the socket has something to write and there is no write block active
            if ($socket->write_hasBuffer()) {
                if ($socket->write_allowed($now)) {
                    $write[] = $socket->getSocketResource();
                } else {
                    $nonAllowedWrites[] = $socket;
                }
            }
        }

        // All sockets are checked for errors
        $except = array_values($this->socketResources);

        // Prepare the returnvalue array
        $result = array();

        // If there's actually anything to select
        if ((count($read) + count($write) + count($except)) > 0) {

            $countSockets = @stream_select($read, $write, $except, $timeInt, $timePart);

            // If there's 1 or more socket to be handled,
            if ($countSockets) {

                // Loop through all sockets who are still in the resource arrays after selecting.
                // Attempt to read on all sockets where there's data waiting
                foreach ($read as $resource) {

                    $ob = $this->findSocket($resource);

                    // The 'accepting' socket has a different handling
                    if ($ob->mode == \SocketManager\Connection::MODE_SERVERMAINSOCK) {

                        $newClient = $ob->accept();

                        if ($ob->resolveNames) {
                            $name = $this->nameResolver($newClient->getConnectedIP(), $ob);
                        }
                        if (@!$name)
                            $name = "_TMP_ACCEPT";

                        $newClient->name = $name;

                        $this->temporarySocketObjects[] = $newClient;

                        $result[] = new SocketManager\UpdateNewConnection($ob, $newClient);
                    } else { // All other sockets are handled 'normally'
                        $in = $ob->read();

                        if ($in && !self::IsKeepAlive($in)) {
                            $result[] = new SocketManager\UpdateRead($ob, $in);
                        } else if ($in === FALSE) {
                            \Utility::LogToFile("FrontendReader", "Removing socket {$ob->name}, read FALSE.");
                            $this->removeSocket($ob);
                            $result[] = new SocketManager\UpdateError($ob, FALSE, self::E_READ_ERROR);
                        }
                    }
                }

                // Attempt to write on the sockets where it's allowed - and we actually have data to write.
                foreach ($write as $resource) {

                    $ob = $this->findSocket($resource);
                    if (!$ob)
                    // This could happen if something went wrong with reading.
                        continue;

                    $cMsg = $ob->write_getCurrentMessage();
                    $written = $ob->write_doWrite($cMsg);

                    if ($written === FALSE) {
                        $this->removeSocket($ob);
                        $this->trigger_onConnectionDropped(array($ob, "write error"));
                        $result[] = new SocketManager\UpdateError($ob, FALSE, self::E_WRITE_ERROR);
                    } else {
                        if ($ob->write_checkDone()) {
                            if (array_key_exists($ob->name, $this->removeAfterWrite)) {
                                unset($this->removeAfterWrite[$ob->name]);
                                $this->removeSocket($ob);
                            }
                        }
                    }
                }

                // Never actually occurred, but fallback in case it does.
                // No idea why/where this would ever happen
                foreach ($except as $resource) {

                    $ob = $this->findSocket($resource);
                    if (!$ob)
                        continue;

                    \Utility::Log("Exception on socket {$ob->tag}.{$ob->name}");

                    $eCode = $ob->lastError;
                    $this->removeSocket($ob);
                    $this->trigger_onConnectionDropped(array($ob, "exception"));
                    $result[] = new SocketManager\UpdateError($ob, $eCode, self::E_UNKNOWN_ERROR);
                }
            } else if ($countSockets === FALSE) {
                // These are almost impossible to debug - you can't select a specific socket to get the error of
                // and thus you have the chance that you might get the last edited errorstate of a different socket.
                $result[] = new SocketManager\UpdateError(NULL, 0, self::E_SELECT_ERROR);
            }
        } else {
            // What's the use of a socketmanager if you're not going to handle any sockets?
            $result[] = new SocketManager\UpdateError(NULL, NULL, self::E_NO_SOCKETS);
        }

        // Timeouts
        foreach ($this->socketObjects as $socket) {
			if($socket->timeoutOverride)
				continue;

            //Utility::Log("{$socket->name} timeout check: inactivity=" . ($now - $socket->lastAlive) . ", timeout=" . $this->maxTimeout);

            $compare = ($socket->timeoutOverride ? $socket->timeoutOverride : $this->maxTimeout);

            if ($socket->mode != \SocketManager\Connection::MODE_SERVERMAINSOCK && $socket->expectKeepAlives && ($now - $socket->lastAlive) >= $compare) {
                $this->removeSocket($socket);
                $this->trigger_onConnectionDropped(array($socket, "timeout"));
                $result[] = new SocketManager\UpdateError($socket, NULL, self::E_SOCKET_TIMEOUT);
            }
        }

        return $result;
    }

    static public function IsKeepAlive(array $message) {
        if (count($message) == 1) {

            if (array_key_exists(1, $message) && is_bool($message[1])) {
                if ($message[1] === FALSE) {
                    return TRUE;
                }
            }
        }
        return FALSE;
    }

    const E_NO_SOCKETS = -100,
        E_UNKNOWN_ERROR = -90,
        E_NO_ERROR = 0,
        E_SELECT_ERROR = 1,
        E_READ_ERROR = 2,
        E_WRITE_ERROR = 3,
        E_SOCKET_TIMEOUT = 4;

}
