<?php

namespace SocketManager;

class UpdateRead extends AUpdate {

    public function __construct($socket, $result, $code = NULL) {
        $this->type = self::UPDATE_TYPE_READ;

        $this->socket = $socket;
        $this->result = $result;
        $this->code = $code;
    }

}