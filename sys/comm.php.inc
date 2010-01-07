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
 * @package API
 * @subpackage Comm
*/


define( "MSG_NONE", -1 );
define( "MSG_SEND", 0 );
define( "MSG_QUEUE", 1 );


class Comm
{
	public static function CreateRemoteObject( $obj, $args = NULL ) {
		if( ! $obj->isRegistered )
			throw new SFException( 'Need a registered node for Comm::CreateRemoteObject()', ERR_REPORT_APP );
		
		if( $args === NULL )
			$args = '';
		else
			$args = Comm::EncodeArguments( $args );
		
		return "CM.AddToRegistry($obj->id,new C.".$obj->remoteConstructor."($args));";
	}
	
	public static function InsertElement( $obj ) {
		if( ! $obj->isRegistered )
			throw new SFException( 'Need a registered node for Comm::InsertElement()', ERR_REPORT_APP );
		
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
			throw new SFException( 'Need a registered node for Comm::RemoveElement()', ERR_REPORT_APP );
		
		if( (! $obj->hostWindow->hasClosed) && (! isset($obj->isRemoved)) )
			return "R[".$obj->parent->id."].removeChild(R[".$obj->id."]);";
		
		return '';
	}
	
	public static function UnRegister( $obj ) {
		if( ! $obj->isRegistered )
			throw new SFException( 'Need a registered node for Comm::UnRegister()', ERR_REPORT_APP );
		
		return "delete R[".$obj->id."];";
	}
	
	public static function CallMethod( $obj, $method, $args = NULL ) {
		if( ! $obj->isRegistered )
			throw new SFException( 'Need a registered node for Comm::CallMethod()', ERR_REPORT_APP );

		if( $obj->hostWindow->hasClosed )
			return '';
		
		if( $args === NULL )
			$args = '';
		else
			$args = Comm::EncodeArguments( $args );
		
		return "R[".$obj->id."].".$method."($args);";
	}
	
	public static function CallMethodDelayed( $obj, $method, $args = NULL, $delay = 1 ) {
		if( ! $obj->isRegistered )
			throw new SFException( 'Need a registered node for Comm::CallMethodDelayed()', ERR_REPORT_APP );

		if( $obj->hostWindow->hasClosed )
			return '';
		
		if( $args === NULL )
			$args = '';
		else
			$args = Comm::EncodeArguments( $args );
		
		return "window.setTimeout( function() {R[".$obj->id."].".$method."($args);}, $delay);";
	}

	public static function SetProperty( $obj, $prop, $value ) {
		if( ! $obj->isRegistered )
			throw new SFException( 'Need a registered node for Comm::SetProperty()', ERR_REPORT_APP );

		if( $obj->hostWindow->hasClosed )
			return '';

		$args = array($value);

		$args = Comm::EncodeArguments( $args );
		
		return "R[".$obj->id."].".$prop."=".$args.";";
	}
	
	public static function SetMethod( $obj, $method, $args, $code ) {
		if( ! $obj->isRegistered )
			throw new SFException( 'Need a registered node for Comm::SetMethod()', ERR_REPORT_APP );

		if( $obj->hostWindow->hasClosed )
			return '';
		
		return "R[".$obj->id."].".$method." = function(".implode(',',$args).") {".$code."};";
	}
	
	public static function SetAttribute( $obj, $attr, $value ) {
		if( ! $obj->isRegistered )
			throw new SFException( 'Need a registered node for Comm::SetAttribute()', ERR_REPORT_APP );

		if( $obj->hostWindow->hasClosed )
			return '';
		
		return "R[".$obj->id."].element.setAttribute(".json_encode($attr).",".json_encode((string)$value).");";
	}

	public static function RemoveAttribute( $obj, $attr ) {
		if( ! $obj->isRegistered )
			throw new SFException( 'Need a registered node for Comm::RemoveAttribute()', ERR_REPORT_APP );

		if( $obj->hostWindow->hasClosed )
			return '';

		return "R[".$obj->id."].element.removeAttribute(".json_encode($attr).");";
	}
	
	public static function EncodeArguments( $args ) {
		if(! is_array($args) )
			$args = array($args);
		
		if (!count($args))
			return json_encode($args);
			
		for( $n = 0; $n < count($args); $n++ ) {
			if( is_array($args[$n]) ) {
				$args[$n] = '['.self::EncodeArguments( $args[$n] ).']';
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

