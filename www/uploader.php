<?php
// - - - - - - - - - - - - - BEGIN LICENSE BLOCK - - - - - - - - - - - - -
// Version: MPL 1.1/GPL 2.0/LGPL 2.1
//
// The contents of this file are subject to the Mozilla Public License Version
// 1.1 (the "License"); you may not use this file except in compliance with
// the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/
//
// Software distributed under the License is distributed on an "AS IS" basis,
// WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
// for the specific language governing rights and limitations under the
// License.
//
// The Original Code is sitefusion.sourceforge.net code.
//
// The Initial Developer of the Original Code is
// FrontDoor Media Group.
// Portions created by the Initial Developer are Copyright (C) 2009
// the Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Nikki Auburger <nikki@thefrontdoor.nl> (original author)
//   Tom Peeters <tom@thefrontdoor.nl>
//
// - - - - - - - - - - - - - - END LICENSE BLOCK - - - - - - - - - - - - -

/**
 * @package Webfrontend
*/

require_once '../conf/webfrontend.conf';
require_once 'functions.php';

try {
    /* Check the input */
    $input = fopen('php://input', 'r');
    if (!$input) {
        throw new Exception('No input');
    }

    /* Check the parms */
    if (empty($_GET['cid']) || empty($_GET['sid']) || empty($_GET['ident'])) {
        throw new Exception('No parameters');
    }

    /* Get the parms */
    $cid = $_GET['cid'];
    $sid = $_GET['sid'];
    $ident = $_GET['ident'];

    /* Get the address */
    $address = isset($WEBCONFIG['address']) ? $WEBCONFIG['address'] : NULL;

    /* Get the database settings */
    $dbDSN = isset($WEBCONFIG['databaseDSN']) ? $WEBCONFIG['databaseDSN'] : NULL;
    $dbHost = isset($WEBCONFIG['databaseHost']) ? $WEBCONFIG['databaseHost'] : NULL;
    $dbName = isset($WEBCONFIG['databaseName']) ? $WEBCONFIG['databaseName'] : NULL;
    $dbUsername = isset($WEBCONFIG['databaseUsername']) ? $WEBCONFIG['databaseUsername'] : NULL;
    $dbPassword = isset($WEBCONFIG['databasePassword']) ? $WEBCONFIG['databasePassword'] : NULL;

    /* Check the session */
    $dbSession = GetSessionFromSID($sid, $dbUsername, $dbPassword, $dbDSN, $dbHost, $dbName);
    if ($dbSession['ident'] != $ident) {
        throw new Exception('Not authorized');
    }

    /* Set the port */
    $port = intval($dbSession['port']);
} catch (Exception $ex) {
    echo $ex->getMessage();
    exit(1);
}

function getConnection($address, $port) {
    $socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
    if ($socket === FALSE) {
        throw new Exception('socket_create() failed: reason: ' . socket_strerror(socket_last_error()));
    }

    $result = socket_connect($socket, $address, $port);
    if ($result === FALSE) {
        throw new Exception('socket_connect() failed.\nReason: ($result) ' . socket_strerror(socket_last_error($socket)));
    }

    return $socket;
}

try {
    /* Set unlimited execution time */
    set_time_limit(0);

    /* Send the start command */
    $socket = getConnection($address, $port);
    WriteCommand($socket, 'WRITE', array('cid' => $cid, 'action' => 'start'));
    socket_close($socket);

    /* Get the input data */
    $output = '';
    $maxOutput = (5 * 1024 * 1024);
    while ($data = fread($input, 1024)) {
        /* Set the ouput */
        $output .= $data;

        /* Check the size */
        if (strlen($output) >= $maxOutput) {
            /* Send the data command */
            $socket = getConnection($address, $port);
            WriteCommand($socket, 'WRITE', array('cid' => $cid, 'action' => 'data'), $output);
            socket_close($socket);

            /* Reset the output */
            $output = '';
        }
    }

    /* Check the output */
    if ($output != '') {
        /* Send the data command */
        $socket = getConnection($address, $port);
        WriteCommand($socket, 'WRITE', array('cid' => $cid, 'action' => 'data'), $output);
        socket_close($socket);

        /* Reset the output */
        $output = '';
    }

    /* Send the end command */
    $socket = getConnection($address, $port);
    WriteCommand($socket, 'WRITE', array('cid' => $cid, 'action' => 'end'));
    socket_close($socket);

    /* Close the input */
    fclose($input);
} catch (Exception $ex) {
    try {
        /* Send the end command */
        $socket = getConnection($address, $port);
        WriteCommand($socket, 'WRITE', array('cid' => $cid, 'action' => 'end'));
        socket_close($socket);
    } catch (Exception $e) {

    }

    /* Clear the output */
    $output = '';

    /* Close the input */
    fclose($input);

    /* Stop here */
    echo $ex->getMessage();
    exit(1);
}
