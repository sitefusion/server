<?php

/**
 * @package SiteFusion
 * @subpackage Node
*/


class BranchNode extends Node
{
	public $children = array();
	
	public function addChild() {
		$args = func_get_args();
		
		for( $n = 0; $n < sizeof($args); $n++ ) {
			if( is_array($args[$n]) ) {
				if( count($args[$n]) )
					array_splice( $args, $n, 1, $args[$n] );
				else continue;
			}

			$node = $args[$n];
			
			if(! $node instanceof Node )
				Error::CallerFatal( 'Object is not a Node' );

			if( $node->isChild )
				Error::CallerFatal( 'Node is already a child' );
			
			$this->fireLocalEvent( 'onBeforeAddChild', array($node) );
			
			array_push( $this->children, $node );
			$node->isChild = TRUE;
			$node->parent = $this;
		
			if( $this->isRegistered ) {
				$this->rootApplication->registry->add( $this, $node );
				
				if( $node instanceof BranchNode )
					$node->registerChildren();

				$node->fireLocalEvent( 'onAfterAttach' );
			}
			
			$this->fireLocalEvent( 'onAfterAddChild', array($node) );
		}
		
		return $this;
	}
	
	public function addChildBefore( $node, $childnode ) {
		if( $node->isChild ) {
			Error::CallerFatal( 'Node is already a child' );
			return FALSE;
		}
		if(! in_array( $childnode, $this->children, TRUE ) ) {
			Error::CallerFatal( 'Given childnode is not a child of this node' );
			return FALSE;
		}
		
		$this->fireLocalEvent( 'onBeforeAddChild', array($node) );
		
		$index = array_search( $childnode, $this->children, TRUE );
		array_splice( $this->children, $index, 0, array($node) );
		$obj->isChild = TRUE;
		$obj->parent = $this;
		
		if( $this->isRegistered ) {
			$obj->insertBefore = $childnode;
			
			$this->rootApplication->registry->add( $this, $node );
			
			if( $node instanceof BranchNode )
				$node->registerChildren();

			$node->fireLocalEvent( 'onAfterAttach' );
		}
		
		$this->fireLocalEvent( 'onAfterAddChild', array($node) );
		
		return $this;
	}

	public function addChildAfter( $node, $childnode ) {
		if( $node->isChild ) {
			Error::CallerFatal( 'Node is already a child' );
			return FALSE;
		}
		if(! in_array( $childnode, $this->children, TRUE ) ) {
			Error::CallerFatal( 'Given childnode is not a child of this node' );
			return FALSE;
		}
		
		$index = array_search( $childnode, $this->children, TRUE );
		
		if( $index == (count($this->children) - 1) )
			$this->addChild( $node );
		else
			$this->addChildBefore( $node, $this->children[$index+1] );
		
		return $this;
	}
	
	public function addChildFirst() {
		$args = func_get_args();
		
		if( count($this->children) ) {
			for( $n = count($args)-1; $n >= 0; $n-- ) {
				if( is_array($args[$n]) ) {
					$num = count($args[$n]);
					array_splice( $args, $n, 1, $args[$n] );
					$n += $num - 1;
				}
	
				$obj = $args[$n];
				$this->addChildBefore( $obj, $this->children[0] );
			}
		}
		else {
			$this->addChild( $args );
		}
		
		return $this;
	}
	
	public function registerChildren() {
		
		foreach( $this->children as $child ) {
			$event = FALSE;
			
			if(! $child->isRegistered ) {
				$this->rootApplication->registry->add( $this, $child );
				$event = TRUE;
			}

			if( method_exists( $child, 'registerChildren' ) )
				$child->registerChildren();
			
			if( $event )
				$child->fireLocalEvent( 'onAfterAttach' );
		}
		
	}
	
	public function removeChild() {
		
		$args = func_get_args();
		$return = array();
		
		for( $n = 0; $n < sizeof($args); $n++ ) {
			if( is_array($args[$n]) )
				array_splice( $args, $n, 1, $args[$n] );
			
			$obj = $args[$n];

			if( ( $index = array_search($obj,$this->children,TRUE) ) === FALSE )
				Error::CallerFatal( 'Node is not a child of this BranchNode' );
			
			$this->fireLocalEvent( 'onBeforeRemoveChild', array($obj) );
			
			if( $obj instanceof BranchNode && $this->isRegistered )
				$obj->unregisterChildren();

			$obj->unlink();
			
			array_splice( $this->children, $index, 1 );
			
			if( $this->isRegistered )
				$this->rootApplication->registry->remove( $obj );
			
			$obj->unbind();
			
			$this->fireLocalEvent( 'onAfterRemoveChild', array($obj) );
		
			array_push( $return, $obj );
		}
		
		return TRUE;
	}
	
	public function extractChild() {
		
		$args = func_get_args();
		$return = array();
		
		for( $n = 0; $n < sizeof($args); $n++ ) {
			if( is_array($args[$n]) )
				array_splice( $args, $n, 1, $args[$n] );
			
			$obj = $args[$n];

			if( ( $index = array_search($obj,$this->children,TRUE) ) === FALSE )
				Error::CallerFatal( 'Node is not a child of this BranchNode' );
			
			$this->fireLocalEvent( 'onBeforeRemoveChild', array($obj) );
			
			if( $obj instanceof BranchNode && $this->isRegistered )
				$obj->unregisterChildren();

			$obj->unlink();
			
			array_splice( $this->children, $index, 1 );
			
			if( $this->isRegistered )
				$this->rootApplication->registry->remove( $obj );
			
			$obj->isChild = FALSE;
			unset( $obj->parent );
			
			$this->fireLocalEvent( 'onAfterRemoveChild', array($obj) );
		
			array_push( $return, $obj );
		}
		
		return ( sizeof($return) == 1 ? $return[0]:$return );
	}
	
	public function unregisterChildren() {
		
		foreach( $this->children as $child ) {
			if( $child instanceof BranchNode )
				$child->unregisterChildren();
			
			$child->unlink();
			
			$this->rootApplication->registry->removeSilent( $child );
		}
		
	}
	
	public function removeChildren() {
		if( count($this->children) )
			return $this->removeChild( $this->children );
	}
	
	public function extractChildren() {
		if( count($this->children) )
			return $this->extractChild( $this->children );
	}
	
	public function findChild( $child ) {
		for( $n = 0; $n < count($this->children); $n++ ) {
			if( $child === $this->children[$n] )
				return $n;
		}
		
		return NULL;
	}
}
