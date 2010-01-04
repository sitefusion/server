<?php

class DOMInspector
{
	static public function Open() {
		ApplicationProcess::$Application->sendCommand('SiteFusion.ClientComponents.DOMInspector.DOMInspector();');
	}
}