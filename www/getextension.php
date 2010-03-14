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

ob_implicit_flush();

include( '../conf/webfrontend.conf' );
include( 'functions.php' );

/*try {
	if( ! (isset($_GET['sid']) && isset($_GET['ident']) ) )
		throw new Exception( 'No parameters' );
	
	$db = mysql_connect( $WEBCONFIG['databaseHost'], $WEBCONFIG['databaseUsername'], $WEBCONFIG['databasePassword'] );
	mysql_select_db( 'sitefusion' );
	$res = mysql_query( "SELECT * FROM `processes` WHERE `id` = '".mysql_escape_string($_GET['sid'])."'" );
	if(! $res )
		throw new Exception( mysql_error() );
	
	if( ! mysql_num_rows($res) )
		throw new Exception( 'No session' );
	
	$dbSession = mysql_fetch_assoc( $res );
	
	if( $dbSession['ident'] != $_GET['ident'] )
		throw new Exception( 'Not authorized' );
	
	$port = (int) $dbSession['port'];
}
catch ( Exception $ex ) {
	echo $ex->getMessage();
	exit(1);
}*/

$extension = $_GET['extension'];
if( strpos($extension,'/') || strpos($extension,"\\") or !file_exists($path = $WEBCONFIG['sitefusionPath'].'/extensions/'.$extension) ) {
	echo "Invalid extension: $extension";
	exit(1);
}

header( 'Content-type: application/octet-stream' );
header( 'Content-length: ' . filesize($path) );

readfile( $path );
