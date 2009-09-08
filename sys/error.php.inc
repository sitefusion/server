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


class SFException extends Exception {
	public function	__construct( $message, $code = NULL, $file = NULL, $line = NULL ) {
		parent::__construct( $message, $code );
		$this->file = $file;
		$this->line = $line;
	}
}

function errorHandler( $no, $str, $file, $line, $context = NULL ) {
	if( $no == E_NOTICE || $no == E_WARNING || $no == E_USER_NOTICE || $no == E_USER_WARNING || $no == E_RECOVERABLE_ERROR ) {
		throw new SFException( strip_tags($str), $no, $file, $line );
		return;
	}
	
	ob_clean();
	$str = addslashes($str);
	$text = "ERROR $no: $str\\n\\nFile: $file\\nLine: $line\\n";
	
	
	$bt = debug_backtrace();
	
	for( $n = 1; isset($bt[$n]); $n++ ) {
		$file = ( isset($bt[$n]['file']) ? $bt[$n]['file'] : '' );
		$line = ( isset($bt[$n]['line']) ? $bt[$n]['line'] : '' );
		$text .= "\\n\\nBacktrace $n:\\nFile: $file\\nLine: $line";
	}
	
	
	if( MODE_HTA ) {
		?>
		<html>
		<head>
		<script language="Javascript">
		showError = false;
		alert( "<?= $text ?>" );
		</script>
		</head>
		<body></body>
		</html>
		<?php
	}
	elseif( MODE_XUL ) {
		$text = str_replace( "\\n", "\n", $text );
		echo $text;
	}
	
	die();
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

		if( MODE_HTA ) {
			?>
			<html>
			<head>
			<script language="Javascript">
			showError = false;
			alert( "<?= $text ?>" );
			</script>
			</head>
			<body></body>
			</html>
			<?php
		}
		elseif( MODE_XUL ) {
			$text = str_replace( "\\n", "\n", $text );
			echo $text;
		}
		
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

		if( MODE_HTA ) {
			?>
			<html>
			<head>
			<script language="Javascript">
			showError = false;
			alert( "<?= $text ?>" );
			</script>
			</head>
			<body></body>
			</html>
			<?php
		}
		elseif( MODE_XUL ) {
			$text = str_replace( "\\n", "\n", $text );
			echo $text;
		}
		
		die();
	}
}