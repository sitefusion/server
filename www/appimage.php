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

include( '../conf/webfrontend.conf' );
include( 'functions.php' );

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
	
	$dbSession = $dbSession = GetSessionFromSID($sid, $dbUsername, $dbPassword, $dbDSN, $dbHost, $dbName);
	
	if( !is_array($dbSession) || $_GET['ident'] == '' || $dbSession['ident'] != $_GET['ident'] )
		throw new Exception( 'Not authorized' );
}
catch ( Exception $ex ) {
	echo $ex->getMessage();
	exit(1);
}

try {
	$name = $_GET['name'];
	
	if( preg_match( '/^[a-zA-Z0-9\-_\/]+\.[a-zA-Z]{3}$/i', $name ) && is_file($WEBCONFIG['sitefusionPath'].'/'.$name) ) {
		if( substr($name,-8) == '.php.inc' || substr($name,-4) == '.php' )
			throw new Exception( 'Unauthorized file access' );
		
		switch ( strtolower(substr($name,-3)) ) {
			case 'jpg': header( 'Content-Type: image/jpeg' ); break;
			case 'gif': header( 'Content-Type: image/gif' ); break;
			case 'png': header( 'Content-Type: image/png' ); break;
			default: header( 'Content-Type: application/octet-stream' );
		}
		
		readfile( $WEBCONFIG['sitefusionPath'].'/'.$name );
	}
	else {
		throw new Exception( 'No such file' );
	}
}
catch ( Exception $ex ) {
	echo $ex->getMessage();
	exit(1);
}
