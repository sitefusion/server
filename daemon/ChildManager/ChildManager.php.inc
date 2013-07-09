<?php

include_once 'Process.php.inc';

class ChildManager {

    static private $instance;

    /**
     *
     * @return \ChildManager
     */
    static public function GetInstance() {
        if (!self::$instance) {
            self::$instance = new self();
        }

        return self::$instance;
    }

    static public function ResetInstance() {
        if (self::$instance)
            self::$instance->forgetChildren();
        self::$instance = NULL;
    }

    static public function NewInstance() {
        return new self();
    }

    /**
     * @var \ChildManager\Process[]
     */
    public $children;

    /**
     *
     * @var \SocketManager
     */
    public $socketManager = NULL;
    private $lastSignal = NULL;

    public function __construct() {
        $this->children = array();

        pcntl_signal(SIGCHLD, array($this, 'signalHandler'));

        $this->socketManager = \SocketManager::NewInstance();
    }

    public function signalHandler($sig = NULL) {
        $this->lastSignal = $sig;
        if ($sig == SIGCHLD) {
            $status = -1;
            while (($pid = pcntl_waitpid(-1, $status, WNOHANG)) > 0) {

                $child = $this->getChildByPID($pid);
                if (!$child) {
                    continue;
                }

                \Utility::Log("Received SIGCHLD on child " . $child->cPid . "." . $child->name);

                $child->done = TRUE;
                $child->doneAt = microtime(TRUE);
                $child->signalHappened = TRUE;

                if ($status !== 0) {
                    $child->error = TRUE;
                }
            }
        } else {
            \Utility::Log("Unhandled SIG arrived at ChildManager: " . $sig);
        }
    }

    public function getChildBySocket($socket) {
        foreach ($this->children as $child) {
            if ($child->controlSocket === $socket)
                return $child;
        }
        return NULL;
    }

    /**
     *
     * @param int $pid
     * @return Process|null
     */
    public function getChildByPID($pid) {
        foreach ($this->children as $child) {
            if ($child->cPid == $pid) {
                return $child;
            }
        }
        return NULL;
    }

    /**
     *
     * @param Process $childProcess
     * @return Process
     * @throws Exception
     */
    public function addChild($childProcess) {
        if (!($childProcess instanceof \ChildManager\Process))
            throw new \Exception("Argument '\$childProcess' needs to be object of type 'Process', was '" . get_class($childProcess) . "'");
        $this->children[] = $childProcess;
        $childProcess->manager = $this;

        return $childProcess;
    }

    public function addChildControlSocket($pid, $socket) {
        $this->socketManager->addSocket($socket);
        $this->socketManager->renameSocket($socket, $pid);
    }

    /**
     *
     * @param boolean $remove
     * @return \ChildManager\Process[]
     */
    public function getDoneChildren($remove) {
        $children = array();

        if (count($this->children) > 0) {
            $keys = array_keys($this->children);
            foreach ($keys as $key) {
                if ($this->children[$key]->done && ($this->children[$key]->returnValueRead)) {
                    $children[] = $this->children[$key];

                    if ($remove) {
                        unset($this->children[$key]);
                    }
                }
            }
        }

        return $children;
    }

    /**
     *
     * @return int
     */
    public function getCountChildren() {
        return count($this->children);
    }

    /**
     *
     * @return \ChildManager\Process[]
     */
    public function update($timeout = 0) {

        pcntl_signal_dispatch();

        if ($timeout !== 0) {
            $s = floor($timeout);
            $u = ceil($timeout * 1000000);
        } else {
            $s = 0;
            $u = 0;
        }

        $childUpdate = $this->getDoneChildren(TRUE);
        $childSocketsRead = array();
        foreach ($this->children as $child) {
            if ($child->started && $child->controlSocket && $child->controlSocket->state) {
                $childSocketsRead[] = $child->controlSocket->getSocketResource();
            }
        }
        $write = $except = NULL;

        if (count($childSocketsRead) > 0) {
            $select = @stream_select($childSocketsRead, $write, $except, $s, $u);

            if ($select === FALSE && $this->lastSignal === NULL) {
                \Utility::Log("Unknown error occurred in ChildManager stream_select call", 0, 31);
            } else if ($select === FALSE && $this->lastSignal !== NULL) {
                \Utility::Log("Sig = " . $this->lastSignal);
            }
            $this->lastSignal = NULL;
        }

        // All done children should be gone at this point
        foreach ($this->children as $child) {

            if (!$child->started)
                continue;

            $time = $child->getWorkingTime();

            if ($time >= $child->processTimeout && !$child->ignoreTimeLimit) {
                posix_kill($child->cPid, SIGKILL);
                \Utility::Log("Killed child " . $child . ": Passed timelimit. (W=$time, L=$child->processTimeout)");
                $child->killed = TRUE;
                $child->doneAt = microtime(TRUE);
                $child->returnValue = new \Exception("Timeout ({$child->processTimeout})");
                $child->returnValueRead = TRUE;
                $child->dispose(TRUE);
				continue;
            }

            if ($child->controlSocket && $child->controlSocket->state && !$child->returnValueRead && in_array($child->controlSocket->getSocketResource(), $childSocketsRead)) {
                $read = $child->controlSocket->read_now();

                if (@$read['type'] == 'process-return') {
                    if (isset($read['error'])) {
                        $child->returnValue = unserialize($read['error']);
                        $child->error = TRUE;
                    } else {
                        $child->returnValue = unserialize($read['response']);
                    }

                    if (isset($read['benchmarking'])) {
                        $child->benchmarking = array_merge(unserialize($read['benchmarking']), $child->benchmarking);
                    }

                    if (isset($read['customerrors'])) {
                        $child->customErrors = $read['customerrors'];
                    }

                    $child->returnValueRead = TRUE;
                    $child->done = TRUE;
                    $child->doneAt = microtime(TRUE);

                    $child->dispose();
                }
            }
        }
        return $childUpdate;
    }

    /**
     *
     * @return boolean
     */
    public function dispose() {
        $allChildrenDead = TRUE;
        foreach ($this->children as $child) {
            if ($child->done || $child->ignoreTimeLimit || $child->getWorkingTime() > $child->processTimeout) {
                $child->dispose(TRUE);
            } else {
                // wait for the child to exit with a given time
                $allChildrenDead = FALSE;
            }
        }

        $this->getDoneChildren(TRUE);

        return $allChildrenDead;
    }

    public function forgetChildren() {
        $this->children = array();
    }

}