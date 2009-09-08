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
 * @package SiteFusion
 * @subpackage Daemon
*/



class JobProcess extends SiteFusionProcess
{
	static public $Arguments;
	static public $Started;
	static private $MailTo;
	static private $Job;
	static private $JobStarted = FALSE;
	
	static public function StartServer( $job, $args, $mailTo = NULL ) {
		if( self::$JobStarted || self::$ProcessType )
			return;
		
		self::$ProcessType = 'job';
		self::$JobStarted = TRUE;
		self::$Job = $job;
		self::$Arguments = explode( ',', $args );
		self::$MailTo = $mailTo;
		self::ExecuteJob();
		exit();
	}
	
	static private function ExecuteJob() {
		try {
			$time = microtime(TRUE);
			ob_start();
			
			include( 'app/'.self::$Job.'.job.php.inc' );
			
			$output = ob_get_clean();
			$time = microtime(TRUE) - $time;
			$output = "Job execution took $time seconds. Output:\n\n" . $output;
		}
		catch ( SFException $ex ) {
			$output = "Job execution failed:\n\n" . $ex->getMessage() . ' in file ' . $ex->getFile() . ' at line ' . $ex->getLine() . "\n\n" . $ex->getTraceAsString();
		}
		
		if( self::$MailTo ) {
			mail( self::$MailTo, 'SiteFusion Daemon Job: '.self::$Job.' '.join(',',self::$Arguments), $output );
		}
	}
}
