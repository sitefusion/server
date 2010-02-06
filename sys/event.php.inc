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
 * @subpackage Application
*/


/**
 * Represents events on the client and server side
 * 
 * Objects of the Event class represent events at both the client and the server side.
 * They can be constructed manually, for example to add additional properties for the 
 * handler(s), with a name and sourceObject parameter. They can then be fired on a node
 * through the fireLocalEvent method. The event handler(s) set on that node will then
 * be called, and the Event object is passed as the first argument.  An event handler 
 * can cancel the propagation of the event by setting the 'cancel' property to TRUE.
 * 
 * For events originating from the client, the sourceObject property will always be set
 * to the original source node of the event.
 * 
 * @property string	$name			Name of the event
 * @property Node	$sourceObject	Source node
 * @property bool	$cancel			Set to TRUE to cancel the event handling
*/

class Event
{
	public $name;
	public $sourceObject;
	public $cancel = FALSE;
	
	
	/**
	 * Constructor
	 * 
	 * @param string	$name			Name of the event
	 * @param Node		$sourceObject	Source node
	*/
	
	public function __construct( $name = NULL, $sourceObject = NULL ) {
		$this->name = $name;
		$this->sourceObject = $sourceObject;
	}
}
