<?php

ini_set( 'display_errors', 'stderr' );

declare( ticks = 1 );

include_once( 'conf/daemon.conf' );
include_once( 'daemon/error.php.inc' );
include_once( 'daemon/daemon.php.inc' );

DaemonProcess::StartServer( $SFCONFIG );
