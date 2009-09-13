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

$db = mysql_connect( $WEBCONFIG['databaseHost'], $WEBCONFIG['databaseUsername'], $WEBCONFIG['databasePassword'] );
mysql_select_db( 'sitefusion' );
$res = mysql_query( "SELECT `ident` FROM `processes` WHERE `id` = '".mysql_escape_string($_GET['sid'])."'" );
if(! $res )
	die( mysql_error() );

if( ! mysql_num_rows($res) )
	die( 'No session' );

$dbSession = mysql_fetch_assoc( $res );

if( $dbSession['ident'] != $_GET['ident'] )
	die( 'Not authorized' );


$name = $_GET['name'];

if( preg_match( '/^[a-zA-Z0-9\-_\/]+\.[a-zA-Z]{3}$/i', $name ) && is_file($WEBCONFIG['sitefusionPath'].'/'.$name) ) {
	if( substr($name,-8) == '.php.inc' || substr($name,-4) == '.php' )
		die( 'Unauthorized file access' );
	
	switch ( strtolower(substr($name,-3)) ) {
		case 'jpg': header( 'Content-Type: image/jpeg' ); break;
		case 'gif': header( 'Content-Type: image/gif' ); break;
		case 'png': header( 'Content-Type: image/png' ); break;
		default: header( 'Content-Type: application/octet-stream' );
	}
	
	readfile( $WEBCONFIG['sitefusionPath'].'/'.$name );
}
else {
	die( 'No such file' );
}