<?php

class Timer {

    private $callback;
    private $timeout;
    private $object;
    private $parameters;
    private $autoReset;
    private $lastTick = 0;
    private $callbackIsFunction = FALSE;
    
    public $isRunning;

    public function __construct($autoReset = FALSE, $timeout = NULL, $callback = NULL, $object = NULL, $parameters = NULL) {

        $typeCallback = gettype($callback);
        if ($typeCallback == 'object') {
            if (get_class($callback) == 'Closure') {
                $this->callbackIsFunction = TRUE;
            } else {
                throw new Exception("Invalid object given. Timer needs a Closure object, or a string and a object. " . get_class($callback) . " given.");
            }
        } else if ($object === NULL) {
            throw new Exception("No object for function call.");
        }

        $this->autoReset = $autoReset;
        $this->timeout = $timeout;
        $this->callback = $callback;
        $this->object = $object;
        $this->parameters = $parameters;

        $this->lastTick = microtime(TRUE);
    }

    public function setCallback($callback) {
        $this->callback = $callback;
    }

    public function setTimeout($time) {
        $this->timeout = $time;
    }

    public function setObject($object) {
        $this->object = $object;
    }

    public function setParameters($parameters) {
        $this->parameters = $parameters;
    }
    
    /**
     * (re-)starts the timer. 
     */
    public function start() {
        $this->isRunning = TRUE;
        $this->lastTick = microtime(TRUE);
    }
    
    public function stop() {
        $this->isRunning = FALSE;
    }

    public function tick($time = NULL) {
        if ($this->isRunning) {

            if ($time === NULL) {
                $time = microtime(TRUE);
            }
            
            //$bla = ($time - $this->lastTick);
            //\Utility::Log($bla . ' remaining on timer');
            
            if (($time - $this->lastTick) >= $this->timeout) {
                $this->lastTick = $time;
                $this->fireEvent();
                
                $this->isRunning = $this->autoReset;
                return TRUE;
            }
        }
        
        return FALSE;
    }

    public function fireEvent() {

        if ($this->callbackIsFunction) {
            $cb = $this->callback;

            $cb($this->parameters);
        } else {
            // lol php
            $this->object->{$this->callback}($this->parameters);
        }
    }

}