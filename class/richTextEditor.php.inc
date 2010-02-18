
<?php

class XULRichTextEditor extends XULEditor
{
	public $remoteConstructor = 'RichTextEditor';
	
	public function setBodyStyle($prop, $value)
	{
		$this->callMethod( 'setBodyStyle', array($prop, $value) );
	}
	
	public function yieldCollect( $e, $val = NULL ) {
		$this->value = $val;
	}
}