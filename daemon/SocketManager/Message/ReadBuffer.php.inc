<?php

namespace SocketManager\Message;

class ReadBuffer {

    private $totalBuffer;
    private $expectedSize = 0;
    public $readSize = 4;
    public $currentMessageSize;

    public function __construct() {
        $this->totalBuffer = '';
    }

    public function isComplete() {
        if (strlen($this->totalBuffer) == $this->expectedSize && $this->expectedSize > 0) {
            return TRUE;
        }
    }

    public function add($in) {

        $this->totalBuffer .= $in;

        if (!$this->expectedSize && strlen($this->totalBuffer) > 3) {
            $part = substr($this->totalBuffer, 0, 4);
            $arr = unpack('VSize', $part);

            $this->expectedSize = $arr['Size'] + 4;
        }

        $this->currentMessageSize = strlen($this->totalBuffer);

        $remainder = $this->expectedSize - $this->currentMessageSize;

        if($remainder < 0)
            \Utility::Log("Remainder less than 0! remainder = $remainder, expectedSize = {$this->expectedSize}, currentSize = {$this->currentMessageSize}", 0, 31);

        $this->readSize = $remainder;
        return $remainder;
    }

    public function getMessage() {
        if ($this->isComplete()) {
            $message = \SocketManager\Message::Decode($this->totalBuffer);
            if ($message) {
                return $message;
            }
        }
    }

}