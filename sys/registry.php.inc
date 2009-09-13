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
 * @subpackage Registry
*/


class Registry
{
	private $newId = 0;
	public $all = array();
	
	final public function add( $parent, $obj ) {
		$id = $this->newId++;

		$this->all[$id] = $obj;
		
		$rootwin = is_object($parent) ? $parent->rootWindow:$obj;

		$obj->id = $id;
		$obj->js = 'window.registry['.$id.']';
		$obj->isRegistered = TRUE;
		$obj->rootWindow = $rootwin;
		$obj->rootApplication = $rootwin->application;
		
		if( isset($obj->isRemoved) )
			unset( $obj->isRemoved );
		
		$down = $obj;
		
		while( $down->parent != NULL ) {
			if( ($down->parent instanceof Window || $down->parent instanceof Dialog || $down->parent instanceof XULWindow || $down->parent instanceof XULChildWindow) && $down->parent != $obj ) {
				$obj->hostWindow = $down->parent;
				break;
			}
			$down = $down->parent;
		}

		$obj->fireLocalEvent( 'onBeforeAttach' );

		if( method_exists( $obj, 'attach' ) )
			$obj->attach();
		
		if( method_exists( $obj, 'initNodeLayout' ) )
			$obj->initNodeLayout();
		
		if( is_object($parent) && method_exists( $parent, 'onAddChild' ) )
			$parent->onAddChild( $obj );
		
		return $id;
	}

	final public function addSilent( $parent, $obj ) {
		$id = $this->newId++;
		
		$this->all[$id] = $obj;

		$rootwin = is_object($parent) ? $parent->rootWindow:$obj;

		$obj->id = $id;
		$obj->js = 'window.registry['.$id.']';
		$obj->isRegistered = TRUE;
		$obj->rootWindow = $rootwin;
		$obj->rootApplication = $rootwin->application;
		
		if( isset($obj->isRemoved) )
			unset( $obj->isRemoved );
		
		return $id;
	}
	
	final public function remove( $node ) {
		if(! $node->isRegistered )
			Error::CallerFatal( 'Node is not registered' );
		
		if( is_object($node->parent) && method_exists( $node->parent, 'onRemoveChild' ) )
			$parent->onRemoveChild( $node );
		
		if( method_exists( $node, 'onBeforeDetach' ) )
			$node->onBeforeDetach();

		$node->fireLocalEvent( 'onBeforeDetach' );

		if( method_exists( $node, 'detach' ) )
			$node->detach();

		$node->fireLocalEvent( 'onAfterDetach' );
		
		$id = $node->id;
		unset( $node->id );
		unset( $node->js );
		unset( $node->rootWindow );
		unset( $node->rootApplication );
		$node->isRegistered = FALSE;
		
		unset( $this->all[$id] );
	}

	final public function removeSilent( $node ) {
		if(! $node->isRegistered )
			Error::CallerFatal( 'Node is not registered' );
		
		if( is_object($node->parent) && method_exists( $node->parent, 'onRemoveChild' ) )
			$parent->onRemoveChild( $node );

		if( method_exists( $node, 'onBeforeDetach' ) )
			$node->onBeforeDetach();

		$node->fireLocalEvent( 'onBeforeDetach' );
		
		$node->isRemoved = TRUE;
		
		if( method_exists( $node, 'detach' ) )
			$node->detach();

		$node->fireLocalEvent( 'onAfterDetach' );
		
		$id = $node->id;
		unset( $node->id );
		unset( $node->js );
		unset( $node->rootWindow );
		unset( $node->rootApplication );
		$node->isRegistered = FALSE;
		
		unset( $this->all[$id] );
	}
}

