<?php


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
