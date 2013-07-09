<?php

namespace SocketManager;
include_once('UpdateRead.php.inc');
include_once('UpdateNewConnection.php.inc');
include_once('UpdateError.php.inc');

abstract class AUpdate {

    const UPDATE_TYPE_READ = 1,
            UPDATE_TYPE_NEWCONN = 2,
            UPDATE_TYPE_ERROR = 3;

    protected $type;
    public $socket;
    public $result;
    public $code = 0;

    public function getType() {
        return $this->type;
    }

}