<?php

namespace SocketManager;

class UpdateNewConnection extends AUpdate {

    public function __construct($socket, $result, $code = NULL) {
        $this->type = self::UPDATE_TYPE_NEWCONN;

        $this->socket = $socket;
        $this->result = $result;
        $this->code = $code;
    }

    public function accept($newName = NULL) {
        $sockMan = \SocketManager::GetInstance();
        $name = (($newName) ? $newName : $this->result->name);
        if ($name == "_TMP_ACCEPT") {
            throw new \Exception("Change the name on an accepted connection!");
        }
        $sockMan->acceptClient($this->result, $name);
    }

    public function deny() {
        $sockMan = \SocketManager::GetInstance();
        $sockMan->denyClient($this->result);
    }

}