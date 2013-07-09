<?php

namespace ChildManager;

class Process {

    public $cPid;
    public $signalHappened = FALSE;
    public $returnValue;
    public $returnValueRead = FALSE;
    public $done = FALSE;
    public $startedAt;
    public $doneAt;
    private $isChild;
    public $function;
    public $isDisposing = FALSE;
    public $ignoreTimeLimit = FALSE;
    public $name;
    public $started = FALSE;

    /**
     *
     * @var \SocketManager\Connection
     */
    public $controlSocket;

    /**
     *
     * @var \SocketManager\Connection
     */
    public $dataSocket;

    /**
     * @var \ChildManager
     */
    public $manager;
    public $writeReturnValue = TRUE;
    public $processTimeout = 60;
    public $killed = FALSE;
    public $error = FALSE;

    /*
     * Diagnostics
     */
    public static $Benchmarking = array();
    private static $BenchmarkingIndex = 0;
    public $benchmarking = array();
    public $benchmarksRetrieved = FALSE;
    public static $Errors = array();
    public $customErrors = NULL;

	/**
	 *
	 * @var Process
	 */
	public static $CurrentProcess = NULL;

    static public function OverrideBaseLabel($label) {
        self::$Benchmarking['label'] = $label;
    }

    static public function AddCustomBenchmark($name = NULL, $label = NULL, $start = NULL, $end = NULL, $color = NULL) {
        if (!isset(self::$Benchmarking[self::$BenchmarkingIndex]))
            self::$Benchmarking[self::$BenchmarkingIndex] = array();

        if ($name !== NULL)
            self::$Benchmarking[self::$BenchmarkingIndex]['name'] = $name;

        if ($label !== NULL)
            self::$Benchmarking[self::$BenchmarkingIndex]['label'] = $label;

        if ($start !== NULL)
            self::$Benchmarking[self::$BenchmarkingIndex]['start'] = $start;

        if ($end !== NULL)
            self::$Benchmarking[self::$BenchmarkingIndex]['end'] = $end;

        if ($color !== NULL)
            self::$Benchmarking[self::$BenchmarkingIndex]['color'] = $color;

        if (isset(self::$Benchmarking[self::$BenchmarkingIndex]['name']) && isset(self::$Benchmarking[self::$BenchmarkingIndex]['label']) && isset(self::$Benchmarking[self::$BenchmarkingIndex]['start']) && isset(self::$Benchmarking[self::$BenchmarkingIndex]['end'])) {
            self::$BenchmarkingIndex++;
        }
    }

    static public function AddError($errorDescription, $isExternal = TRUE) {
        self::$Errors[] = array($errorDescription, $isExternal);
    }

    public function __construct($function) {
        $this->function = $function;
    }

    public function __toString() {
        return sprintf("(%s.%s)", $this->name, $this->cPid);
    }

    /**
     *
     * @return int
     */
    public function getWorkingTime() {
        $until = ($this->doneAt !== NULL ? $this->doneAt : microtime(true));
        return ($until - $this->startedAt);
    }

    /**
     *
     * @return \SocketManager\Connection;
     */
    public function process($arguments = array()) {

        // Create sockets for the IPC
        // Control sockets are the sockets through which ONLY the returnvalue is written
        $controlSockets = stream_socket_pair(AF_UNIX, SOCK_STREAM, 0);
        // Data sockets are sockets through which free communication with the 'spawner' or 'creator' is possible (for example Daemon)
        $dataSockets = stream_socket_pair(AF_UNIX, SOCK_STREAM, 0);

        if ($controlSockets === FALSE || $dataSockets === FALSE) {
            return FALSE;
        }

        $pid = pcntl_fork();
        if ($pid == -1) {
            // Error during forking (Shouldn't be happening unless I screwed up by overloading the server again)
            return FALSE;
        } else if ($pid > 0) {

            // Parent process
            // Close the sockets for the child
            fclose($controlSockets[0]);
            fclose($dataSockets[0]);

            $this->cPid = $pid;
            $this->isChild = FALSE;
            $this->startedAt = microtime(true);
            $this->started = TRUE;

            $controlSocket = new \SocketManager\Connection($controlSockets[1]);
            $controlSocket->state = TRUE;
            $controlSocket->name = $this->getName();
            $controlSocket->mode = \SocketManager\Connection::MODE_CLIENT;
            $controlSocket->sendKeepAlives = FALSE;
            $controlSocket->tag = 'forkedSocket';
            $controlSocket->timeoutOverride = $this->processTimeout;
            $this->controlSocket = $controlSocket;

            $dataSocket = new \SocketManager\Connection($dataSockets[1]);
            $dataSocket->state = TRUE;
            $dataSocket->name = $this->getName();
            $dataSocket->mode = \SocketManager\Connection::MODE_CLIENT;
            $dataSocket->sendKeepAlives = FALSE;
            $dataSocket->tag = 'forkedSocket';
            $dataSocket->timeoutOverride = $this->processTimeout;
            $dataSocket->userData = array("PID" => $pid);
            $this->dataSocket = $dataSocket;

            return $this->dataSocket;
        } else {
            $this->afterProcessChild();

            // Child Process
            // Close the sockets for the parent
            fclose($controlSockets[1]);
            fclose($dataSockets[1]);

            $controlSocket = new \SocketManager\Connection($controlSockets[0]);
            $controlSocket->state = TRUE;
            $controlSocket->name = $this->getName();
            $controlSocket->mode = \SocketManager\Connection::MODE_CLIENT;
            $controlSocket->sendKeepAlives = FALSE;
            $controlSocket->tag = 'forkedSocket';
            $controlSocket->timeoutOverride = $this->processTimeout;
            $this->controlSocket = $controlSocket;

            $dataSocket = new \SocketManager\Connection($dataSockets[0]);
            $dataSocket->state = TRUE;
            $dataSocket->name = $this->getName();
            $dataSocket->mode = \SocketManager\Connection::MODE_CLIENT;
            $dataSocket->sendKeepAlives = FALSE;
            $dataSocket->tag = 'forkedSocket';
            $dataSocket->timeoutOverride = $this->processTimeout;
            $this->dataSocket = $dataSocket;

            $sockMan = \SocketManager::GetInstance();
            $sockMan->addSocket($dataSocket);

            $this->cPid = getmypid();
            $this->isChild = TRUE;
            $this->startedAt = microtime(true);
            $this->started = TRUE;
            try {
                // Some sanity checking
                if (is_callable($this->function)) {
                    $f = $this->function;
					self::$CurrentProcess = $this;
                    $this->returnValue = $f(array("socket" => $this->dataSocket, "parameters" => $arguments));
                    if ($this->returnValue instanceof \Exception) {
                        \Utility::SmartDump($this->returnValue);
                        throw $this->returnValue;
                    } else {
                        if (!is_array($this->returnValue)) {
                            $this->returnValue = array($this->returnValue);
                        }
                    }
                } else {
                    throw new \Exception("Argument passed to Process is not callable!");
                }
            } catch (\Exception $e) {
                // Processes can simply throw to get an error to the receiver
                // TODO: Instead of relying on Brightfish Logging, make it possible so there's a callback handler on ChildManager where the overlying application
                // (in this case Brightfish) can set up a logging/error handler.
                \Brightfish\Daemon\Logging::Log(-1, "ChildProcess: " . $e->getMessage() . "(" . $e->getCode() . ") at " . $e->getFile() . " line " . $e->getLine() . "\n" . $e->getTraceAsString() . "\n");
                $this->returnValue = new \Brightfish\Error($e);
            }

            $write = array(
                'type' => 'process-return'
            );

            // Dirty hacky code.
            // TODO: Set custom callbacks from descendant classes to handle the returnvalue writing
            if ($this instanceof \Brightfish\Daemon\ExecutionUnit) {

                if ($this->deferred) {
                    $write['response'] = serialize($this->returnValue);
                    \Utility::Log("EUnit is deferred, closing");

					// Try to write. Most likely BatchProcess is gone already, unless the deferred process instantly failed.
                    $this->writeToSocket($write, FALSE);
                    $this->done = TRUE;
                    $this->dispose();
                    return;
                }

                $write['benchmarking'] = serialize(self::$Benchmarking);
                if (count(self::$Errors))
                    $write['customerrors'] = self::$Errors;
            }

            // See if the returnvalue is an error, or even needs to be considered
            if ($this->returnValue instanceof \Brightfish\Error) {
                $write['error'] = serialize($this->returnValue);
                $write['response'] = 0;
            } else if (!$this->writeReturnValue) {
                $write['response'] = serialize(1);
            } else {
                $write['response'] = serialize($this->returnValue);
            }

            $this->done = TRUE;
            $this->doneAt = microtime(true);
            if (!$this->writeToSocket($write, FALSE)) {
                \Brightfish\Daemon\Logging::Log(-1, "Failed to write return value to socket!");
            } else {
                //\Brightfish\Daemon\Logging::Log(3, "Successfully wrote return value.");
            }

            $this->dispose();
        }
    }

    public function getName() {
        if ($this->name)
            return $this->name;
        else
            return $this->cPid;
    }

    /**
     * Override this in childclasses to set a specific handling which is executed right after forking.
     * Remember to also call parent::afterProcessChild() !
     */
    protected function afterProcessChild() {
        \Utility::SetTimeColor(\Utility::DB_COLOR_RED);
        \Utility::SetLogPrefix('C' . getmypid());
    }

    /**
     *
     * @param type $msg
     * @param type $mode TRUE = Data, FALSE = ReturnValue/Code
     */
    private function writeToSocket($msg, $mode = TRUE) {

        if ($this->isChild) {
            $sock = ($mode ? $this->dataSocket : $this->controlSocket);

            if ($this->isChild) {

                $sock->write($msg);

                // pseudo socket-manager implementation - as this will only be used from a child-process it can 'block' while waiting for the socket to come free
                $done = FALSE;
                $socketResource = $sock->getSocketResource();

                $eCount = 0;

                while ($sock->write_hasBuffer()) {

                    $read = $ex = NULL;
                    $write = array($socketResource);

                    $mt1 = microtime(TRUE);
                    $select = @stream_select($read, $write, $ex, 0, 500000);
                    $len = microtime(TRUE) - $mt1;

                    if ($select === FALSE) {
                        \Utility::Log("Error occured in selecting write socket.");
                    } else if ($select === 1) {
                        $part = $sock->write_getCurrentMessage();
                        $writtenLength = $sock->write_doWrite($part);

                        if (!$writtenLength && feof($socketResource)) {
                            \Utility::Log("Socket FEOF'd while attempting returnvalue write. Canceling.");
                            $done = FALSE;
                            break;
                        }

                        if ($writtenLength === FALSE || $writtenLength === 0) {
                            $eCount++;
                        } else if ($sock->write_checkDone()) {
                            $done = TRUE;
                        }

                        if ($eCount == 4) {
                            \Utility::Log("Failed to write return value to socket.");
                            $done = FALSE;
                            break;
                        }
                    }
                }

                return $done;
            }
        }
        \Utility::Log("Called writeToSocket on parent?", 0, 31);
        return false;
    }

    /**
     * Parent: Attempts to kill child by sending SIGINT
     * Child: Kills own process by exit;
     * @param boolean $force Disposes even if process is still running by sending signal 9 (SIGKILL)
     */
    public function dispose($force = FALSE) {

        if (!$this->done && !$force) {
            throw new \Exception("Child process is not done yet! If you want to forcefully kill the child you'll have to specify '\$force' as true.");
        }

        if ($this->done || $force) {
            if ($this->isChild) {
                $this->controlSocket->destroy();
                $this->dataSocket->destroy();
                exit;
            } else {
                pcntl_signal_dispatch();
                $this->done = TRUE;

                $this->controlSocket->destroy();
                $this->dataSocket->destroy();
            }
        }
    }

}