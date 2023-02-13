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

ignore_user_abort( TRUE );
include( '../conf/webfrontend.conf' );
include( 'functions.php' );

try {
    if( substr_count($_SERVER['HTTP_ACCEPT_ENCODING'], 'gzip') ) ob_start("ob_gzhandler");

    $input = fopen( 'php://input', 'r' );
    $content = '';
    while( strlen($content) < $_SERVER['CONTENT_LENGTH'] ) {
        $content .= fread( $input, 8192 );
    }
    fclose( $input );

    if( $_SERVER['CONTENT_TYPE'] == 'application/x-gzip' )
        $content = gzuncompress($content);
}
catch ( Exception $ex ) {
    ReturnError( 'input_error', $ex->getMessage() );
}


try {
    $sid = isset($_GET['sid']) ? $_GET['sid'] : NULL;
    if (!$sid) {
        throw new Exception( 'Not authorized' );
    }

    $dbDSN = (isset($WEBCONFIG['databaseDSN']) ? $WEBCONFIG['databaseDSN'] : NULL);
    $dbHost = (isset($WEBCONFIG['databaseHost']) ? $WEBCONFIG['databaseHost'] : NULL);
    $dbName = (isset($WEBCONFIG['databaseName']) ? $WEBCONFIG['databaseName'] : NULL);
    $dbUsername = (isset($WEBCONFIG['databaseUsername']) ? $WEBCONFIG['databaseUsername'] : NULL);
    $dbPassword = (isset($WEBCONFIG['databasePassword']) ? $WEBCONFIG['databasePassword'] : NULL);

    for ($n = 0; $n < 20; $n++) {
        $dbSession = $dbSession = GetSessionFromSID($sid, $dbUsername, $dbPassword, $dbDSN, $dbHost, $dbName);
        if (empty($dbSession['ident'])) {
            usleep(200);
        } else {
            break;
        }
    }

    if( empty($dbSession) || $dbSession['ident'] != $_GET['ident'] )
        throw new Exception( 'Not authorized');

    $port = (int) $dbSession['port'];
}
catch ( Exception $ex ) {
    ReturnError( 'session_error', $ex->getMessage() );
}

try {
    $socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
    if ($socket === FALSE) {
        throw new Exception( "socket_create() failed: reason: " . socket_strerror(socket_last_error()) );
    }

    $result = socket_connect($socket, $WEBCONFIG['address'], $port );
    if ($result === FALSE) {
        throw new Exception( "socket_connect() failed.\nReason: ($result) " . socket_strerror(socket_last_error($socket)) );
    }
} catch (Exception $ex) {
    ReturnError('server_offline', $ex->getMessage());
}

try {
    WriteCommand($socket, 'COMM', NULL, $content);
    $cmd = ReadCommand($socket);
    socket_close($socket);
} catch (Exception $ex) {
    /* Give the daemon some time to collect error output */
    usleep(500000);

    try {
        $socket = @socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
        if ($socket === FALSE) {
            throw new Exception('socket_create() failed: reason: ' . socket_strerror(socket_last_error()));
        }

        $result = @socket_connect($socket, $WEBCONFIG['address'], $WEBCONFIG['port']);
        if ($result === FALSE) {
            throw new Exception("socket_connect() failed.\nReason: ($result) " . socket_strerror(socket_last_error($socket)));
        }

        WriteCommand($socket, 'GETERROR', array('clientid' => $_GET['clientid']));
        $cmd = ReadCommand($socket);

        if ($cmd->found) {
            ReturnError('php_error', $cmd->data);
        } else {
            ReturnError('php_error', 'webfrontend comm: ReadCommand failed');
        }
    } catch (Exception $ex) {
        ReturnError('unspecified_error', $ex->getMessage());
    }
}

if (substr($cmd->data,-16) != '"EXEC_COMPLETE";') {
    ReturnError('php_error', $cmd->data);
} else {
    ReturnResult($cmd->data);
}
