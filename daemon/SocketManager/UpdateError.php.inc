<?php

namespace SocketManager;

class UpdateError extends AUpdate {

    public function __construct($socket, $result, $code = NULL) {
        $this->type = self::UPDATE_TYPE_ERROR;

        $this->socket = $socket;
        $this->result = $result;
        $this->code = $code;
    }

}