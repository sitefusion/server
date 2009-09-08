<?php

/**
 * @package SiteFusion
 * @subpackage Daemon
*/



class SFException extends Exception {
	public function	__construct( $message, $code = NULL, $file = NULL, $line = NULL ) {
		parent::__construct( $message, $code );
		$this->file = $file;
		$this->line = $line;
	}
}


function errorHandler( $no, $str, $file, $line, $context = NULL ) {
		throw new SFException( strip_tags($str), $no, $file, $line );
		return;
}


set_error_handler( 'errorHandler' );



class Error
{
	public static $Suppress = FALSE;
	
	public static function CallerFatal( $msg, $level = 1 ) {
		if( self::$Suppress ) return;
		
		if( ob_get_length() )
			ob_clean();
		
		$bt = debug_backtrace();
		
		$msg = addslashes($msg);
		$file = ( isset($bt[$level]['file']) ? $bt[$level]['file'] : '' );
		$line = ( isset($bt[$level]['line']) ? $bt[$level]['line'] : '' );
		
		$text = "ERROR: $msg\\n\\nFile: $file\\nLine: $line\\n";

		for( $n = $level + 1; isset($bt[$n]); $n++ ) {
			$file = ( isset($bt[$n]['file']) ? $bt[$n]['file'] : '' );
			$line = ( isset($bt[$n]['line']) ? $bt[$n]['line'] : '' );
			$text .= "\\n\\nBacktrace $n:\\nFile: $file\\nLine: $line";
		}

		$text = str_replace( "\\n", "\n", $text );
		echo $text;
		
		die();
	}
	
	public static function Fatal( $msg ) {
		if( self::$Suppress ) return;
		
		if( ob_get_length() )
			ob_clean();
		
		$bt = debug_backtrace();
		
		$msg = addslashes($msg);
		$file = ( isset($bt[0]['file']) ? $bt[0]['file'] : '' );
		$line = ( isset($bt[0]['line']) ? $bt[0]['line'] : '' );
		
		$text = "ERROR: $msg\\n\\nFile: $file\\nLine: $line\\n";

		for( $n = 1; isset($bt[$n]); $n++ ) {
			$file = ( isset($bt[$n]['file']) ? $bt[$n]['file'] : '' );
			$line = ( isset($bt[$n]['line']) ? $bt[$n]['line'] : '' );
			$text .= "\\n\\nBacktrace $n:\\nFile: $file\\nLine: $line";
		}

		$text = str_replace( "\\n", "\n", $text );
		echo $text;
		
		die();
	}
}