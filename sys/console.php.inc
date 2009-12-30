<?php

class ClientError
{
	public static function ConsoleMessage ($msg)
	{
		ApplicationProcess::$Application->sendCommand("SiteFusion.consoleMessage(".json_encode((string)$msg).");");
	}
	
	public static function NativeConsoleMessage ($msg)
	{
		ApplicationProcess::$Application->sendCommand("SiteFusion.nativeConsoleMessage(".json_encode((string)$msg).");");
	}
	
	public static function ConsoleError ($msg)
	{
		ApplicationProcess::$Application->sendCommand("SiteFusion.consoleError(".json_encode((string)$msg).");");
	}
}