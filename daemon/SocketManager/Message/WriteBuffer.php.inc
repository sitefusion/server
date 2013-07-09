<?php

namespace SocketManager\Message;

class WriteBuffer {

    private $totalMessage;
    private $currentlyAt = 0;
    private $messageSize;
    private $timestamp;

    public function __construct($toSend) {
        $this->totalMessage = $toSend . '';
        $this->messageSize = strlen($toSend);
        $this->timestamp = microtime(TRUE);
    }

    public function __toString() {
        //return "MesBufWrite - Contents:" . bin2hex($this->totalMessage) . ");cAt($this->currentlyAt);";
    }

    public function isDone() {
        return ($this->currentlyAt === $this->messageSize);
    }

    public function getMessagePart() {
        return substr($this->totalMessage, $this->currentlyAt, ($this->messageSize - $this->currentlyAt));
    }

    public function movePointer($steps) {
        $this->currentlyAt += $steps;
    }

}