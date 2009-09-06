<?php

function authorizeLogin( $args, $user, $pass ) {
	include( 'sfadminPasswd.php.inc' );
	return ($user == 'admin' && $pass == $password);
}

function getApplication( $args ) {
	include_once( 'main_admin.php.inc' );
	return 'SitefusionAdmin';
}
