<?php
class SiteFusionTimer
{
	private $timeout;
	private $object;
	private $function;
	private $interval;
	private $cancelled = FALSE;
	
	public function __construct( $timeout, $obj, $func, $interval = FALSE ) {
		$this->timeout = $timeout;
		$this->object = $obj;
		$this->function = $func;
		$this->interval = $interval;
		
		ExtendedSiteFusionProcess::StoreTimer( $this );
	}
	
	public function timeout() {
		return $this->timeout;
	}
	
	public function trigger() {
		if( is_object($this->object) )
			call_user_func_array( array(&$this->object,$this->function), array($this) );
		else
			call_user_func_array( array($this->object,$this->function), array($this) );
		
		if( $this->interval && ! $this->cancelled )
			ExtendedSiteFusionProcess::StoreTimer( $this );
	}
	
	public function cancel() {
		ExtendedSiteFusionProcess::RemoveTimer( $this );
		$this->cancelled = TRUE;
	}
}