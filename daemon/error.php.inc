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
 * @package Daemon
 * @subpackage Extension
*/

define('ERR_REPORT_APP', 1);
define('ERR_REPORT_SF', 0);

class SFException extends Exception {

    public function __construct($message, $type = 0, $file = NULL, $line = NULL) {
        parent::__construct($message);

        $this->type = $type;

        if ($file === NULL) {
            $this->file = $this->getFile();
        }

        if ($line === NULL) {
            $this->line = $this->getLine();
        }
    }

    public function getFormattedReport() {
        $msg = $this->getMessage();
        $file = $this->file;
        $line = $this->line;

        if (class_exists('ExtendedSiteFusionProcess') && $this->type == ERR_REPORT_APP) {
            $sfPath = ExtendedSiteFusionProcess::GetOption( 'sitefusionPath' );
            $trace = $this->getTrace();
            $appDir = $sfPath.'/app/';

            for ($stackLevel = 0; $stackLevel < count($trace); $stackLevel++) {
                if (isset($trace[$stackLevel]['file']) && substr( $trace[$stackLevel]['file'], 0, strlen($appDir) ) == $appDir) {
                    $file = $trace[$stackLevel]['file'];
                    $line = $trace[$stackLevel]['line'];
                    break;
                }
            }
        }

        return $msg . ' in ' . $file . ' on line ' . $line . "\n\n" . $this->getTraceAsString();
    }
}

function errorHandler($no, $str, $file, $line) {
    /* Check if error code is not included in error_reporting */
    if (!(error_reporting() && $no)) {
        return FALSE;
    }

    /* Trow the error */
    throw new SFException($str, ERR_REPORT_APP, $file, $line);
}

set_error_handler('errorHandler');

if (version_compare(phpversion(), '7.0.0', '>=')) {
    function exceptionHandler(Throwable $ex) {
        /* Create some default values */
        $logfile = NULL;
        $parentType = NULL;

        /* Get log file and parent tpye */
        if (class_exists('ApplicationProcess')) {
            $parentType = ApplicationProcess::GetProcessType();
            if ($parentType === NULL) {
                if (class_exists('DaemonProcess')) {
                    $parentType = DaemonProcess::GetProcessType();
                    if ($parentType !== NULL) {
                        $logfile = DaemonProcess::GetOption('logfile');
                    }
                }
            } else {
                $logfile = ApplicationProcess::GetOption('logfile');
            }
        } elseif (class_exists('ServiceProcess')) {
            $parentType = ServiceProcess::GetProcessType();
            if ($parentType === NULL) {
                if (class_exists('DaemonProcess')) {
                    $parentType = DaemonProcess::GetProcessType();
                    if ($parentType !== NULL) {
                        $logfile = DaemonProcess::GetOption('logfile');
                    }
                }
            } else {
                $logfile = ServiceProcess::GetOption('logfile');
            }
        } elseif (class_exists('DaemonProcess')) {
            $parentType = DaemonProcess::GetProcessType();
            if ($parentType !== NULL) {
                $logfile = DaemonProcess::GetOption('logfile');
            }
        }

        /* Get the message */
        if ($ex instanceof SFException) {
            $message = $ex->getFormattedReport();
        } else {
            $message = $ex;
        }

        /* Check if logfile and parent type is not NULL */
        if ($logfile !== NULL && $parentType !== NULL && file_exists($logfile)) {
            $text = date('d-m-Y H:i:s') . ': ' . $message . PHP_EOL;
            file_put_contents($logfile, $text, FILE_APPEND | LOCK_EX);
        }

        /* Show the message */
        echo $message . PHP_EOL;
    }

    set_exception_handler('exceptionHandler');
}