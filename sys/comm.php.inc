<?php

/**
 * @package SiteFusion
 * @subpackage Comm
*/


define( "MSG_NONE", -1 );
define( "MSG_SEND", 0 );
define( "MSG_QUEUE", 1 );


class Comm
{
	public static function CreateRemoteObject( $obj, $args = NULL ) {
		if( ! $obj->isRegistered )
			Error::CallerFatal( 'Need a registered node for Comm::CreateRemoteObject()' );
		
		if( $args === NULL )
			$args = '';
		else
			$args = Comm::EncodeArguments( $args );
		
		return "CM.AddToRegistry($obj->id,new C.".$obj->remoteConstructor."($args));";
	}
	
	public static function InsertElement( $obj ) {
		if( ! $obj->isRegistered )
			Error::CallerFatal( 'Need a registered node for Comm::InsertElement()' );
		
		if( isset( $obj->insertBefore ) ) {
			$id = $obj->insertBefore->id;
			unset( $obj->insertBefore );
			
			return "R[".$obj->parent->id."].addChildBefore(R[".$obj->id."],R[".$id."]);";
		}
		else {
			return "R[".$obj->parent->id."].addChild(R[".$obj->id."]);";
		}
	}

	public static function RemoveElement( $obj ) {
		if( ! $obj->isRegistered )
			Error::CallerFatal( 'Need a registered node for Comm::RemoveElement()' );
		
		if( (! $obj->hostWindow->hasClosed) && (! isset($obj->isRemoved)) )
			return "R[".$obj->parent->id."].removeChild(R[".$obj->id."]);";
		
		return '';
	}
	
	public static function UnRegister( $obj ) {
		if( ! $obj->isRegistered )
			Error::CallerFatal( 'Need a registered node for Comm::UnRegister()' );
		
		return "delete R[".$obj->id."];";
	}
	
	public static function CallMethod( $obj, $method, $args = NULL ) {
		if( ! $obj->isRegistered )
			Error::CallerFatal( 'Need a registered node for Comm::CallMethod()' );

		if( $obj->hostWindow->hasClosed )
			return '';
		
		if( $args === NULL )
			$args = '';
		else
			$args = Comm::EncodeArguments( $args );
		
		return "R[".$obj->id."].".$method."($args);";
	}

	public static function SetProperty( $obj, $prop, $value ) {
		if( ! $obj->isRegistered )
			Error::CallerFatal( 'Need a registered node for Comm::SetProperty()' );

		if( $obj->hostWindow->hasClosed )
			return '';

		$args = array($value);

		$args = Comm::EncodeArguments( $args );
		
		return "R[".$obj->id."].".$prop."=".$args.";";
	}
	
	public static function SetMethod( $obj, $method, $args, $code ) {
		if( ! $obj->isRegistered )
			Error::CallerFatal( 'Need a registered node for Comm::SetMethod()' );

		if( $obj->hostWindow->hasClosed )
			return '';
		
		return "R[".$obj->id."].".$method." = function(".implode(',',$args).") {".$code."};";
	}
	
	public static function SetAttribute( $obj, $attr, $value ) {
		if( ! $obj->isRegistered )
			Error::CallerFatal( 'Need a registered node for Comm::SetAttribute()' );

		if( $obj->hostWindow->hasClosed )
			return '';
		
		return "R[".$obj->id."].element.setAttribute(".json_encode($attr).",".json_encode((string)$value).");";
	}

	public static function RemoveAttribute( $obj, $attr ) {
		if( ! $obj->isRegistered )
			Error::CallerFatal( 'Need a registered node for Comm::RemoveAttribute()' );

		if( $obj->hostWindow->hasClosed )
			return '';

		return "R[".$obj->id."].element.removeAttribute(".json_encode($attr).");";
	}
	
	public static function EncodeArguments( $args ) {
		if(! is_array($args) )
			$args = array($args);
		
		for( $n = 0; $n < count($args); $n++ ) {
			if( is_array($args[$n]) ) {
				$args[$n] = self::EncodeArguments( $args[$n] );
				continue;
			}
			if( $args[$n] instanceof Node )
				$args[$n] = ($args[$n]->isRegistered ? 'R['.$args[$n]->id.']' : 'null');
			else
				$args[$n] = json_encode( $args[$n] );
		}
		
		return join( ',', $args );
	}
}

