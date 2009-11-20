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
 * @subpackage Node
*/


/**
 * This is the base class of all nodes. It provides all methods necessary for manipulating XUL attributes,
 * event firing, handling and yielding, reflexes and styling.
 * 
 * @property int			$id					Registry ID for the node (only set when the node is registered)
 * @property Node			$parent				Parent node for this node (only set when the node is registered)
 * @property bool			$isRegistered		Indicates whether the node is registered and thus part of the XUL tree on the client side
 * @property bool			$isChild			Indicates whether the node is a child of another node
 * @property XULWindow		$rootWindow			Reference to the root window object (only set when the node is registered)
 * @property Application	$rootApplication	Reference to the application object (only set when the node is registered)
 * @property array			$eventHandler		[INTERNAL] Keeps track of event handlers
 * @property array			$eventYielder		[INTERNAL] Keeps track of event yielders
 * @property array			$handlerFor			[INTERNAL] Keeps track of event handlers of other nodes
 * @property array			$eventHandler		[INTERNAL] Keeps track of event yielders of other nodes
*/

class Node
{
	public $id;
	public $parent;
	public $isRegistered		= FALSE;
	public $isChild			= FALSE;
	public $rootWindow;
	public $rootApplication;
	public $eventHandler		= array();
	public $eventYielder		= array();
	public $eventType		= array();
	public $handlerFor		= array();
	public $yielderFor		= array();
	
	
	/**
	 * Dynamic Constructor
	 *
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( $this instanceof BranchNode && func_num_args() ) {
			$args = func_get_args();
			call_user_func_array( array(&$this,'addChild'), $args );
		}
	}
	
	
	/* Event Handling */
	
	/**
	 * Creates a new event on the client side
	 * 
	 * NOTE: this is not about firing an event. This method actually creates a new addition to
	 * the set of fireable client events of this node.
	 * 
	 * @param string	$event		Name of the new event
	 * @param int		$type		Message type for the new event (one of MSG_SEND, MSG_QUEUE or MSG_NONE)
	*/
	
	public function createClientEvent( $event, $type = -1 ) {
		if(! $this->isRegistered )
			Error::CallerFatal( "Can't create a client-side event on an unregistered node" );
		
		$this->callMethod( 'createEvent', array( $event, $type ) );
	}
	
	
	/**
	 * Shorthand version for setEventType, setEventHandler and setEventYielder
	 * 
	 * This method sets an event type, handler and yielder(s) in one convenient call.
	 * 
	 * @param string		$event		Event name
	 * @param int			$type		Message type (one of MSG_SEND, MSG_QUEUE or MSG_NONE)
	 * @param object|string	$obj		Object or class name to call the handler method on
	 * @param string		$func		Name of the handler method
	 * @param mixed			$yielders	Node or array of nodes to yield when this event fires
	 * @return Node	Returns reference to self ($this) for easy chaining of method calls
	*/
	
	public function setEvent( $event, $type, $obj, $func, $yielders = NULL ) {
		$this	-> setEventType( $event, $type )
			-> setEventHandler( $event, $obj, $func );
		
		if( $yielders != NULL )
			$this->setEventYielder( $event, $yielders );
		
		return $this;
	}
	
	
	/**
	 * Sets an event handler
	 * 
	 * This method sets an event handler to call when a certain event fires.
	 * 
	 * @param string		$event		Event name
	 * @param object|string	$obj		Object or class name to call the handler method on
	 * @param string		$func		Name of the handler method
	 * @return Node	Returns reference to self ($this) for easy chaining of method calls
	*/

	public function setEventHandler( $event, $obj, $func ) {
		$handler = array( $obj, $func );
		
		if( isset( $this->eventHandler[$event] ) )
			array_push( $this->eventHandler[$event], $handler );
		else
			$this->eventHandler[$event] = array( $handler );
		
		if( is_object($obj) )
			array_push( $obj->handlerFor, array($event,$this,$func) );
		
		return $this;
	}
	
	
	/**
	 * Removes an event handler
	 * 
	 * This method removes a previously set event handler for a certain event.
	 * 
	 * @param string		$event		Event name
	 * @param object|string	$obj		Object or class name to call the handler method on
	 * @param string		$func		Name of the handler method
	 * @return Node	Returns reference to self ($this) for easy chaining of method calls
	*/
	
	public function removeEventHandler( $event, $obj, $func ) {
		$handler = array( $obj, $func );
		
		if(! isset( $this->eventHandler[$event] ) )
			Error::CallerFatal( 'The given handler is not in the node`s eventHandler list' );

		for( $n = 0; $n < count($this->eventHandler[$event]); $n++ ) {
			if( $this->eventHandler[$event][$n] === $handler )
				array_splice( $this->eventHandler[$event], $n--, 1 );
		}
		
		for( $n = 0; $n < count($obj->handlerFor); $n++ ) {
			$h = $obj->handlerFor[$n];
			if( $obj->handlerFor[$n] === array($event,$this,$func) )
				array_splice( $obj->handlerFor, $n--, 1 );
		}
		
		return $this;
	}
	
	
	/**
	 * Sets a node to yield when an event on this node fires
	 * 
	 * Yielding means that a node transfers its current input on the client side to the server
	 * side. For example, a textbox the user just entered text in transfers its content to
	 * the server, synchronising both sides. The transfer is done through the 'yield' event,
	 * Which is usually handled automatically on the server side, placing the transferred contents
	 * in the designated property on the node. This usually occurs when another event fires, for 
	 * example the pressing of a button (the 'command' event). To make the button trigger the 
	 * yield of the textbox, the setEventYielder() method should be called on the button with the
	 * event name ('command') as the first argument and the textbox node as the second argument, 
	 * indicating that the button should yield the textbox when its command event fires.
	 * 
	 * @param string		$event		Event name
	 * @param Node			$obj		Node to yield when this event fires
	 * @return Node	Returns reference to self ($this) for easy chaining of method calls
	*/
	
	public function setEventYielder( $event, $obj ) {
		if( ! $this->isRegistered )
			Error::CallerFatal( 'Can`t set eventYielder because the hosting node is not registered' );
		
		if(! is_array($obj) )
			$obj = array($obj);
		
		foreach ( $obj as $node ) {
			if( ! property_exists( $node, 'isRegistered' ) )
				Error::CallerFatal( 'Object cannot be yielded because it is not a Node class object' );
			if( ! $node->isRegistered )
				Error::CallerFatal( 'Given yielder node is not registered' );
			if( isset($this->eventYielder[$event]) && in_array( $node, $this->eventYielder[$event], TRUE ) )
				return $this;
			
			if( isset( $this->eventYielder[$event] ) )
				array_push( $this->eventYielder[$event], $node );
			else
				$this->eventYielder[$event] = array( $node );
			
			array_push( $node->yielderFor, array($event,$this) );
		}
		
		array_unshift( $obj, $event );
		
		$this->rootApplication->sendCommand(
			Comm::CallMethod( $this, 'addYielder', $obj )
		);
		
		return $this;
	}
	
	
	/**
	 * Removed a node this is yielded when an event on this node fires
	 * 
	 * This method is the opposite of setEventYielder(), and removes a previously set yielder
	 * 
	 * @param string		$event		Event name
	 * @param Node			$obj		Node to yield when this event fires
	 * @return Node	Returns reference to self ($this) for easy chaining of method calls
	*/
	
	public function removeEventYielder( $event, $obj ) {
		if( ! $this->isRegistered )
			Error::CallerFatal( 'Can`t remove eventYielder because the hosting node is not registered' );
		
		if(! is_array($obj) )
			$obj = array($obj);
		
		foreach ( $obj as $node ) {
			if( ! $node->isRegistered )
				Error::CallerFatal( 'Given yielder node is not registered' );
			if(! (is_array($this->eventYielder[$event]) && in_array( $node, $this->eventYielder[$event], TRUE )) )
				Error::CallerFatal( 'This node does not yield the given node' );
			
			$index = array_search( $node, $this->eventYielder[$event], TRUE );
			array_splice( $this->eventYielder[$event], $index, 1 );
			
			for( $n = 0; $n < count($node->yielderFor); $n++ ) {
				if( $node->yielderFor[$n] === array($event,$this) )
					array_splice( $node->yielderFor, $n, 1 );
			}
		}
		
		array_unshift( $obj, $event );
		
		$this->rootApplication->sendCommand(
			Comm::CallMethod( $this, 'removeYielder', $obj )
		);
		
		return $this;
	}
	
	
	/**
	 * Sets the event type for a certain event
	 * 
	 * The event type determines what happens to the event when it fires. There are three event
	 * types: MSG_SEND, MSG_QUEUE and MSG_NONE.
	 * 
	 * When an event fires, a series of actions unfold:
	 * 1. The event reflex is executed (if any)
	 * 2. If there are yielder nodes set, their 'yield' events are fired
	 * 3. The message for the current event is produced if the event type is not MSG_NONE
	 * 
	 * If the event type is MSG_QUEUE, the message is appended to the queue. If the event type is
	 * MSG_SEND, the queue is flushed with the current event message tailing the flush. Yield events
	 * are always of the type MSG_QUEUE, so that they don't get sent before the event message that
	 * triggered them is produced. This way the whole event triggering, yielding and sending only
	 * requires one communication cycle.
	 * 
	 * @param string		$event		Event name
	 * @param int			$type		Event type
	 * @return Node	Returns reference to self ($this) for easy chaining of method calls
	*/
	
	public function setEventType( $event, $type ) {
		if( ! $this->isRegistered )
			Error::CallerFatal( 'Can`t set event type because the hosting node is not registered' );
		
		$this->eventType[$event] = $type;
		
		$this->callMethod( 'setEventType', array($event,$type) );
		
		return $this;
	}
	
	
	/**
	 * Sets a reflex for a certain event
	 * 
	 * A reflex is a piece of JavaScript that gets executed immediately upon firing an event,
	 * without requiring server communication. This can be used for simple actions, such as
	 * coloring a button on a mouseover event. If this would have to be done through an event
	 * handler on the server side, there would be a delay which is not desirable for such
	 * simple actions. The JavaScript code is supplied through a string, in which references
	 * to a node can be made with the Node->js property. This property refers to the SiteFusion
	 * JavaScript node object, which also contains a reference to the XUL element in the element
	 * JavaScript property. Setting a reflex that colors a button red when the mouse hovers it
	 * would be done like:
	 * 
	 * $button->setEventReflex( "mouseover", "$button->js.element.style.backgroundColor = 'red';" );
	 * 
	 * A node can only hold one reflex per event.
	 * 
	 * @param string		$event		Event name
	 * @param int			$code		JavaScript code to execute when the event fires
	 * @return Node	Returns reference to self ($this) for easy chaining of method calls
	*/
	
	public function setEventReflex( $event, $code ) {
		if( ! $this->isRegistered )
			Error::CallerFatal( 'Can`t set event reflex because the node is not registered' );
		
		$this->callMethod( 'addReflex', array($event,$code) );
		
		return $this;
	}
	
	
	/**
	 * Removes a previously set reflex for a certain event
	 * 
	 * This function removes the reflex set for the given event.
	 *
	 * @param string	$event	Event name
	 * @return Node	Returns reference to self ($this) for easy chaining of method calls
	*/
	
	public function removeEventReflex( $event ) {
		if( ! $this->isRegistered )
			Error::CallerFatal( 'Can`t remove event reflex because the node is not registered' );
		
		$this->callMethod( 'removeReflex', array($event) );
		
		return $this;
	}

	
	/**
	 * [INTERNAL FUNCTION] Handles an incoming event for this node
	 *
	 * @param Event	$event	Event object
	 * @param array	$data	Event arguments
	 * @return Event	Event object
	*/
	
	public function commandHandler( $event, $data ) {
		if(! is_array($data) )
			$data = array($data);
		
		if( $event instanceof Event ) {
			$eventobj = $event;
			$event = $eventobj->name;
		}
		else $eventobj = new Event( $event, $this );

		array_unshift( $data, $eventobj );
		
		if( isset($this->eventHandler[$event]) ) {
			foreach ( $this->eventHandler[$event] as $handler ) {
				if( is_object($handler[0]) )
					call_user_func_array( array(&$handler[0], $handler[1]), $data );
				else
					call_user_func_array( array($handler[0], $handler[1]), $data );
				
				if( $eventobj->cancel == TRUE )
					break;
			}
		}
		
		return $eventobj;
	}
	
	
	/**
	 * Fires a local (server) event
	 * 
	 * This method fires an event on the server side, which does not go through the client.
	 * Events fired through this method do not have to be valid clientside events, and can
	 * be used to build event-driven applications.
	 * 
	 * @param string|Event	$event		Event object or name
	 * @param array			$args		Event arguments
	 * @return Event		Event object
	*/
	
	public function fireLocalEvent( $event, $args = NULL ) {
		if( $args === NULL )
			$args = array();
		
		$eventobj = $this->commandHandler( $event, $args );
		
		return $eventobj;
	}
	
	
	/**
	 * Fires a remote (client) event
	 * 
	 * This method fires an event on the client side, as if it was the result of a user action.
	 * Events fired through this method can only be valid clientside events, either pre-existing
	 * standard XUL or SiteFusion events or an event created with the createClientEvent() method.
	 * 
	 * @param string		$event		Event name
	 * @param array			$args		Event arguments
	*/
	
	public function fireClientEvent( $event, $args = NULL ) {
		if(! $this->isRegistered )
			Error::CallerFatal( 'Unregistered node cannot fire a ClientEvent' );
		
		$this->rootApplication->sendCommand(
			Comm::CallMethod( $this, 'fireEvent', array($event,$args) )
		);
	}
	
	
	/**
	 * Manually yield this node
	 * 
	 * This method calls the 'yield' method on the node, which usually results in a yield event
	 * being triggered.
	 * 
	 * @return Node	Returns reference to self ($this) for easy chaining of method calls
	*/
	
	public function yield() {
		if(! $this->isRegistered )
			Error::CallerFatal( 'Unregistered node cannot yield' );
		
		$this->rootApplication->sendCommand(
			Comm::CallMethod( $this, 'yield' )
		);
		
		return $this;
	}
	
	
	/**
	 * [INTERNAL FUNCTION] Removes event handler and yielder links to other nodes
	 * 
	 * This method is called before a node is removed to clear all links to other nodes, enabling
	 * the PHP garbage collector to free up the memory.
	*/
	
	public function unlink() {
		foreach ( $this->eventHandler as $event => $hs ) {
			$hsc = $hs;
			foreach ( $hsc as $h ) {
				$this->removeEventHandler( $event, $h[0], $h[1] );
			}
		}
		foreach ( $this->eventYielder as $event => $ys ) {
			$ysc = $ys;
			foreach ( $ysc as $y ) {
				$this->removeEventYielder( $event, $y );
			}
		}
		
		$handlerFor = $this->handlerFor;
		foreach ( $handlerFor as $h ) {
			$h[1]->removeEventHandler( $h[0], $this, $h[2] );
		}
		$yielderFor = $this->yielderFor;
		foreach ( $yielderFor as $y ) {
			$y[1]->removeEventYielder( $y[0], $this );
		}
	}
	
	
	/**
	 * [INTERNAL FUNCTION] Removes links to child and parent nodes
	 * 
	 * This method is called before a node is removed to clear all links its child and parent
	 * nodes, enabling the PHP garbage collector to free up the memory.
	*/
	
	public function unbind() {
		if( $this instanceof BranchNode ) {
			foreach ( $this->children as $child ) {
				$child->unbind();
			}
			$this->children = array();
		}
		unset( $this->parent );
		$this->isChild = FALSE;
	}



	/* Comm methods */
	
	
	/**
	 * [INTERNAL FUNCTION] Calls the remote constructor of this class
	*/
	
	public function createRemoteObject( $args = NULL ) {
		if(! $this->isRegistered )
			Error::CallerFatal( 'Can\'t call createRemoteObject on unregistered node' );
		
		$this->rootApplication->sendCommand(
			Comm::CreateRemoteObject( $this, $args )
		);
		
		return $this;
	}
	
	
	/**
	 * [INTERNAL FUNCTION] Inserts the newly constructed node's element into the DOM tree on the client side
	*/
	
	public function insertElement() {
		if(! $this->isRegistered )
			Error::CallerFatal( 'Can\'t call insertElement on unregistered node' );
		
		$this->rootApplication->sendCommand(
			Comm::InsertElement( $this )
		);
		
		return $this;
	}
	
	
	/**
	 * [INTERNAL FUNCTION] Removes the node's element from the client DOM tree
	*/
	
	public function removeElement() {
		if(! $this->isRegistered )
			Error::CallerFatal( 'Can\'t call removeElement on unregistered node' );
		
		$this->rootApplication->sendCommand(
			Comm::RemoveElement( $this )
		);
		
		return $this;
	}

	
	/**
	 * [INTERNAL FUNCTION] Removes this node from the client registry
	*/
	
	public function unRegister() {
		if(! $this->isRegistered )
			Error::CallerFatal( 'Can\'t call unRegister on unregistered node' );
		
		$this->rootApplication->sendCommand(
			Comm::UnRegister( $this )
		);
		
		return $this;
	}
	
	
	/**
	 * Call a JavaScript method on the client-side node
	 * 
	 * @param string	$name		Name of the method (may also contain dots to traverse to properties containing other objects)
	 * @param array		$args		Method call arguments (may contain references to nodes)
	 * @return Node	Returns reference to self ($this) for easy chaining of method calls
	*/
	
	public function callMethod( $name, $args = NULL ) {
		if(! $this->isRegistered )
			Error::CallerFatal( 'Can\'t call callMethod on unregistered node' );
		
		$this->rootApplication->sendCommand(
			Comm::CallMethod( $this, $name, $args )
		);
		
		return $this;
	}
	
	/**
	 * Call a JavaScript method on the client-side node in a delayed fashion
	 * 
	 * @param string	$name		Name of the method (may also contain dots to traverse to properties containing other objects)
	 * @param array		$args		Method call arguments (may contain references to nodes)
	 * @param int		$delay		Delay of function call in ms
	 * @return Node	Returns reference to self ($this) for easy chaining of method calls
	*/
	
	public function callMethodDelayed( $name, $args = NULL, $delay = 1 ) {
		if(! $this->isRegistered )
			Error::CallerFatal( 'Can\'t call callMethod on unregistered node' );
		
		$this->rootApplication->sendCommand(
			Comm::CallMethodDelayed( $this, $name, $args, $delay )
		);
		
		return $this;
	}
	
	/**
	 * Set a JavaScript property on the client-side node
	 * 
	 * @param string	$name		Name of the property (may also contain dots to traverse to properties containing other objects)
	 * @param mixed		$value		Value to set the property to (may also be a reference to a node)
	 * @return Node	Returns reference to self ($this) for easy chaining of method calls
	*/
	
	public function setProperty( $name, $value ) {
		if(! $this->isRegistered )
			Error::CallerFatal( 'Can\'t call setProperty on unregistered node' );
		
		$this->rootApplication->sendCommand(
			Comm::SetProperty( $this, $name, $value )
		);
		
		return $this;
	}
	
	
	/**
	 * Set a JavaScript method on the client-side node
	 * 
	 * @param string	$name		Name of the new method (may also contain dots to traverse to properties containing other objects)
	 * @param array		$args		Array of argument names for the JavaScript method to accept
	 * @param string	$code		JavaScript code for the body of the method
	 * @return Node	Returns reference to self ($this) for easy chaining of method calls
	*/
	
	public function setMethod( $method, $args, $code ) {
		if(! $this->isRegistered )
			Error::CallerFatal( 'Can\'t call setMethod on unregistered node' );
		
		$this->rootApplication->sendCommand(
			Comm::SetMethod( $this, $method, $args, $code )
		);
		
		return $this;
	}

	
	/**
	 * Set an attribute on the client-side XUL node
	 * 
	 * @param string	$name		Name of the attribute
	 * @param string	$value		New value for the attribute
	 * @return Node	Returns reference to self ($this) for easy chaining of method calls
	*/
	
	public function setAttribute( $name, $value ) {
		if(! $this->isRegistered )
			Error::CallerFatal( 'Can\'t call setAttribute on unregistered node' );
		
		$this->rootApplication->sendCommand(
			Comm::SetAttribute( $this, $name, $value )
		);
		
		return $this;
	}

	
	/**
	 * Remove an attribute on the client-side XUL node
	 * 
	 * @param string	$name		Name of the attribute
	 * @return Node	Returns reference to self ($this) for easy chaining of method calls
	*/
	
	public function removeAttribute( $name ) {
		if(! $this->isRegistered )
			Error::CallerFatal( 'Can\'t call removeAttribute on unregistered node' );
		
		$this->rootApplication->sendCommand(
			Comm::RemoveAttribute( $this, $name )
		);
		
		return $this;
	}
	
	
	/**
	 * Set a CSS style property on the client-side XUL node
	 * 
	 * @param string	$name		Name of the CSS property (camelized, so backgroundColor instead of background-color)
	 * @param string	$value		New value for the style property
	 * @return Node	Returns reference to self ($this) for easy chaining of method calls
	*/
	
	public function setStyle( $name, $value ) {
		if(! $this->isRegistered )
			Error::CallerFatal( 'Can\'t call setStyle on unregistered node' );
		
		$this->rootApplication->sendCommand(
			Comm::SetProperty( $this, 'element.style.'.$name, $value )
		);
		
		return $this;
	}



	/* Generic methods */
	
	
	/**
	 * [INTERNAL FUNCTION] Default attach method
	 * 
	 * The default action on attach is to call createRemoteObject() and insertElement().
	*/
	
	public function attach() {
		$this->createRemoteObject( array( $this->hostWindow ) );
		$this->insertElement();
	}
	
	
	/**
	 * [INTERNAL FUNCTION] Default detach method
	 * 
	 * The default action on detach is to call removeElement() and unRegister().
	*/
	
	public function detach() {
		$this->removeElement();
		$this->unRegister();
	}
	
	
	/**
	 * Give this node focus
	 * 
	 * Equivalent of the JavaScript focus() method.
	*/
	
	public function focus() {
		$this->callMethod( 'focus' );
		
		return $this;
	}
	
	
	/**
	 * Make this node loose focus
	 * 
	 * Equivalent of the JavaScript blur() method.
	*/
	
	public function blur() {
		$this->callMethod( 'blur' );
		
		return $this;
	}
	
	
	/**
	 * Removes and unregisters this node and its childnodes
	 *
	 * This method removes and unlinks this node and its child nodes from each other.
	 * Use this method when you intend to discard the node(s) entirely. If you want
	 * to reuse the node and its childnodes, use the method extractNode().
	*/
	
	public function removeNode() {
		if(! $this->isChild )
			Error::CallerFatal( 'removeNode: Node is not a child' );
		
		$this->parent->removeChild( $this );
	}
	
	
	/**
	 * Extracts this node and its childnodes
	 * 
	 * This method extracts this node and its childnodes without breaking their links.
	 * Use this method if you intend to reuse this node or the subbranch that it contains.
	 * 
	 * @return Node	The extracted node
	*/
	
	public function extractNode() {
		if(! $this->isChild )
			Error::CallerFatal( 'extractNode: Node is not a child' );
		
		return $this->parent->extractChild( $this );
	}
	
	
	/**
	 * Return the first ancestor of a certain class of this node
	 * 
	 * This method goes down the parent lineage and returns the first ancestor that is of 
	 * the given class. Returns NULL if no such ancestor was found.
	 * 
	 * @param string	$className	Name of the class
	 * @return Node|NULL	The found node or NULL if none was found
	*/
	
	public function findAncestor( $className ) {
		if( !$this->isRegistered )
			Error::CallerFatal( 'Node is not registered' );
		
		$par = $this->parent;
		
		while( ! $par instanceof $className ) {
			if( isset($par->parent) )
				$par = $par->parent;
			else return NULL;
		}
		
		return ($par instanceof $className ? $par : NULL);
	}
	
	
	/**
	 * Convenient attribute setter
	 * 
	 * This method sets both the property and attribute (when the node is registered).
	 * If value is NULL, returns the current value. Otherwise returns the node.
	 * 
	 * @param string	$name	Name of the attribute and property
	 * @param mixed		$value	Value (if NULL returns current value)
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function attributeMethod( $name, $value ) {
		if( $value === NULL )
			return (isset($this->$name) ? $this->$name : NULL);
		
		$this->$name = $value;
		
		if( is_bool($value) )
			$value = ($value ? 'true':'false');
		
		if( $this->isRegistered )
			$this->setAttribute( $name, $value );
		
		return $this;
	}



	/* Sizing */
	
	/**
	 * Set the size of the element
	 * 
	 * Sets the width and height (in pixels).
	 * 
	 * @param int	$width		Width in pixels
	 * @param int	$height		Height in pixels
	 * @return Node	Returns reference to self ($this) for easy chaining of method calls
	*/
	
	public function size( $width, $height ) {
		$this->width( $width );
		$this->height( $height );
		
		return $this;
	}
	
	
	/**
	 * Set the width of the element
	 * 
	 * @link https://developer.mozilla.org/en/XUL/Attribute/width
	 * @param int	$width		Width in pixels
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function width( $width = NULL ) {
		if( $width === NULL )
			return (isset($this->width) ? $this->width : NULL);
		
		$this->width = $width;
		
		if( $this->isRegistered )
			$this->setAttribute( 'width', $width );
		
		return $this;
	}
	
	
	/**
	 * Set the height of the element
	 * 
	 * @link https://developer.mozilla.org/en/XUL/Attribute/height
	 * @param int	$height		Height in pixels
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function height( $height = NULL ) {
		if( $height === NULL )
			return (isset($this->height) ? $this->height : NULL);
		
		$this->height = $height;
		
		if( $this->isRegistered )
			$this->setAttribute( 'height', $height );
		
		return $this;
	}
	
	
	/* Padding */
	
	
	/**
	 * Set the padding of the element
	 * 
	 * @param int	$pix		Padding in pixels
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function padding( $pix = NULL ) {
		if( $pix === NULL )
			return (isset($this->padding) ? $this->padding : NULL);
		
		$this->padding = $pix;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.style.padding', $this->padding );
		
		return $this;
	}
	
	
	/**
	 * Set the top padding of the element
	 * 
	 * @param int	$pix		Padding in pixels
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function paddingTop( $pix = NULL ) {
		if( $pix === NULL )
			return (isset($this->paddingTop) ? $this->paddingTop : NULL);
		
		$this->paddingTop = $pix;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.style.paddingTop', $this->paddingTop );
		
		return $this;
	}

	
	/**
	 * Set the right padding of the element
	 * 
	 * @param int	$pix		Padding in pixels
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function paddingRight( $pix = NULL ) {
		if( $pix === NULL )
			return (isset($this->paddingRight) ? $this->paddingRight : NULL);
		
		$this->paddingRight = $pix;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.style.paddingRight', $this->paddingRight );
		
		return $this;
	}
	
	
	/**
	 * Set the bottom padding of the element
	 * 
	 * @param int	$pix		Padding in pixels
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function paddingBottom( $pix = NULL ) {
		if( $pix === NULL )
			return (isset($this->paddingBottom) ? $this->paddingBottom : NULL);
		
		$this->paddingBottom = $pix;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.style.paddingBottom', $this->paddingBottom );
		
		return $this;
	}

	
	/**
	 * Set the left padding of the element
	 * 
	 * @param int	$pix		Padding in pixels
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function paddingLeft( $pix = NULL ) {
		if( $pix === NULL )
			return (isset($this->paddingLeft) ? $this->paddingLeft : NULL);
		
		$this->paddingLeft = $pix;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.style.paddingLeft', $this->paddingLeft );
		
		return $this;
	}



	/* Margin */
	
	
	/**
	 * Set the margin of the element
	 * 
	 * @param int	$pix		Margin in pixels
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function margin( $pix = NULL ) {
		if( $pix === NULL )
			return (isset($this->margin) ? $this->margin : NULL);
		
		$this->margin = $pix;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.style.margin', $this->margin );
		
		return $this;
	}
	
	
	/**
	 * Set the top margin of the element
	 * 
	 * @param int	$pix		Margin in pixels
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function marginTop( $pix = NULL ) {
		if( $pix === NULL )
			return (isset($this->marginTop) ? $this->marginTop : NULL);
		
		$this->marginTop = $pix;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.style.marginTop', $this->marginTop );
		
		return $this;
	}

	
	/**
	 * Set the right margin of the element
	 * 
	 * @param int	$pix		Margin in pixels
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function marginRight( $pix = NULL ) {
		if( $pix === NULL )
			return (isset($this->marginRight) ? $this->marginRight : NULL);
		
		$this->marginRight = $pix;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.style.marginRight', $this->marginRight );
		
		return $this;
	}

	
	/**
	 * Set the bottom margin of the element
	 * 
	 * @param int	$pix		Margin in pixels
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function marginBottom( $pix = NULL ) {
		if( $pix === NULL )
			return (isset($this->marginBottom) ? $this->marginBottom : NULL);
		
		$this->marginBottom = $pix;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.style.marginBottom', $this->marginBottom );
		
		return $this;
	}

	
	/**
	 * Set the left margin of the element
	 * 
	 * @param int	$pix		Margin in pixels
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function marginLeft( $pix = NULL ) {
		if( $pix === NULL )
			return (isset($this->marginLeft) ? $this->marginLeft : NULL);
		
		$this->marginLeft = $pix;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.style.marginLeft', $this->marginLeft );
		
		return $this;
	}
	
	
	/* Text and Background Properties */
	
	
	/**
	 * Set the text size for the element
	 * 
	 * @param string	$size		Text size in CSS form (#pt #px etc)
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function textSize( $size = NULL ) {
		if( $size === NULL )
			return (isset($this->textSize) ? $this->textSize : NULL);
		
		$this->textSize = $size;
		
		if( $this->isRegistered )
			$this->rootApplication->sendCommand(
				Comm::SetProperty( $this, 'element.style.fontSize', $this->textSize )
			);
		
		return $this;
	}
	
	
	/**
	 * Set the text color for the element
	 * 
	 * @param string	$color		Color in HTML format (#XXXXXX)
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function textColor( $color = NULL ) {
		if( $color === NULL )
			return (isset($this->textColor) ? $this->textColor : NULL);
		
		$this->textColor = $color;
		
		if( $this->isRegistered )
			$this->rootApplication->sendCommand(
				Comm::SetProperty( $this, 'element.style.color', $this->textColor )
			);
		
		return $this;
	}

	
	/**
	 * Set the text style for the element
	 * 
	 * You can supply one or more of the following attributes in a space-separated string
	 * 
	 * - bold
	 * - italic
	 * - underline
	 * - strike
	 * - blink
	 * - overline
	 * 
	 * @param string	$style		Style string
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function textStyle( $style = NULL ) {
		if( $style === NULL )
			return (isset($this->textStyle) ? $this->textStyle : NULL);
		
		$this->textStyle = $style;
		
		if( $this->isRegistered ) {
			$deco = array();
			foreach( explode(' ',$style) as $spart ) {
				switch( trim($spart) ) {
					case "bold": $this->setProperty( 'element.style.fontWeight', 'bold' ); break;
					case "italic": $this->setProperty( 'element.style.fontStyle', 'italic' ); break;
					case "underline": $deco[] = 'underline'; break;
					case "strike": $deco[] = 'line-through'; break;
					case "blink": $deco[] = 'blink'; break;
					case "overline": $deco[] = 'overline'; break;
				}
			}
			
			if( count($deco) )
				$this->setProperty( 'element.style.textDecoration', implode( ',', $deco ) );
		}
		
		return $this;
	}
	
	
	/**
	 * Set the background color for the element
	 * 
	 * @param string	$color	Color in HTML format (#XXXXXX)
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function backgroundColor( $color = NULL ) {
		if( $color === NULL )
			return (isset($this->backgroundColor) ? $this->backgroundColor : NULL);
		
		$this->backgroundColor = $color;
		
		if( $this->isRegistered )
			$this->rootApplication->sendCommand(
				Comm::SetProperty( $this, 'element.style.backgroundColor', $this->backgroundColor )
			);
		
		return $this;
	}

	
	
	/* Standard XUL attributes */


	
	/**
	 * Set the CSS classname for the element
	 * 
	 * @param string	$className		Class name
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function className( $className = NULL ) {
		if( $className === NULL )
			return (isset($this->className) ? $this->className : NULL);
		
		$this->className = (string) $className;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.className', $this->className );
		
		return $this;
	}
	
	
	/**
	 * Set the flex attribute for this element
	 * 
	 * @link https://developer.mozilla.org/en/XUL/Attribute/flex
	 * @param int	$val		Flex value
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function flex( $val = NULL ) {
		if( $val === NULL )
			return (isset($this->flex) ? $this->flex : NULL);
		
		$this->flex = (int) $val;
		
		if( $this->isRegistered )
			$this->callMethod( 'flex', array($this->flex) );
		
		return $this;
	}
	
	
	/**
	 * Set the disabled attribute for this element
	 * 
	 * @link https://developer.mozilla.org/en/XUL/Attribute/disabled
	 * @param bool	$state		Disabled (true or false)
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function disabled( $state = NULL ) {
		if( $state === NULL )
			return (isset($this->disabled) ? $this->disabled : NULL);
		
		$this->disabled = (bool) $state;
		
		if( $this->isRegistered )
			$this->callMethod( 'disabled', array($this->disabled) );
		
		return $this;
	}
	
	
	/**
	 * Set the label attribute for this element
	 * 
	 * @link https://developer.mozilla.org/en/XUL/Attribute/label
	 * @param string	$text		Label text
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function label( $text = NULL ) {
		if( $text === NULL )
			return (isset($this->label) ? $this->label : NULL);
		
		$this->label = $text;
		
		if( $this->isRegistered )
			$this->callMethod( 'label', $text );
		
		return $this;
	}
	
	
	/**
	 * Set the value attribute for this element
	 * 
	 * @link https://developer.mozilla.org/en/XUL/Attribute/value
	 * @param mixed	$val		Value attribute content
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function value( $val = NULL ) {
		if( $val === NULL )
			return (isset($this->value) ? $this->value : NULL);
		
		$this->value = $val;
		
		if( $this->isRegistered )
			$this->callMethod( 'value', array($this->value) );
		
		return $this;
	}
	
	
	/**
	 * Set the orient attribute for this element
	 * 
	 * @link https://developer.mozilla.org/en/XUL/Attribute/orient
	 * @param string	$dir		Orient value ('horizontal' or 'vertical')
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function orient( $dir = NULL ) {
		if( $dir === NULL )
			return (isset($this->orient) ? $this->orient : NULL);
		
		$this->orient = ($dir == 'h' || $dir == 'horizontal' ? 'horizontal':'vertical');
		
		if( $this->isRegistered )
			$this->callMethod( 'orient', array($this->orient) );
		
		return $this;
	}

	
	/**
	 * Set the pack attribute for this element
	 * 
	 * @link https://developer.mozilla.org/en/XUL/Attribute/pack
	 * @param string	$pack		Pack value
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function pack( $pack = NULL ) {
		if( $pack === NULL )
			return (isset($this->pack) ? $this->pack : NULL);
		
		$this->pack = $pack;
		
		if( $this->isRegistered )
			$this->callMethod( 'pack', array($this->pack) );
		
		return $this;
	}

	
	/**
	 * Set the align attribute for this element
	 * 
	 * @link https://developer.mozilla.org/en/XUL/Attribute/align
	 * @param string	$align		Align value
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function align( $align = NULL ) {
		if( $align === NULL )
			return (isset($this->align) ? $this->align : NULL);
		
		$this->align = $align;
		
		if( $this->isRegistered )
			$this->callMethod( 'align', array($this->align) );
		
		return $this;
	}

	
	/**
	 * Set the crop attribute for this element
	 * 
	 * @link https://developer.mozilla.org/en/XUL/Attribute/crop
	 * @param string	$crop		Crop value
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function crop( $crop = NULL ) {
		if( $crop === NULL )
			return (isset($this->crop) ? $this->crop : NULL);
		
		$this->crop = $crop;
		
		if( $this->isRegistered )
			$this->callMethod( 'crop', array($this->crop) );
		
		return $this;
	}

	
	/**
	 * Set the accessKey attribute for this element
	 * 
	 * @link https://developer.mozilla.org/en/XUL/Attribute/accesskey
	 * @param string	$key		accessKey value
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function accessKey( $key = NULL ) {
		if( $key === NULL )
			return (isset($this->accessKey) ? $this->accessKey : NULL);
		
		$this->accessKey = $key;
		
		if( $this->isRegistered )
			$this->callMethod( 'accessKey', array($this->accessKey) );
		
		return $this;
	}
	
	
	/**
	 * Set the hidden attribute for this element
	 * 
	 * @link https://developer.mozilla.org/en/XUL/Attribute/hidden
	 * @param bool	$hid		Hidden value (true or false)
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function hidden( $hid = NULL ) {
		return $this->attributeMethod( 'hidden', $hid );
	}

	
	/**
	 * Set the equalsize attribute for this element
	 * 
	 * @link https://developer.mozilla.org/en/XUL/Attribute/equalsize
	 * @param string	$state		Equalsize value
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function equalsize( $state = NULL ) {
		return $this->attributeMethod( 'equalsize', $state );
	}
	
	
	/**
	 * Set the tooltiptext attribute for this element
	 * 
	 * @link https://developer.mozilla.org/en/XUL/Attribute/tooltiptext
	 * @param string	$text		Text to show in a tooltip when hovering the element
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function tooltiptext( $text = NULL ) {
		return $this->attributeMethod( 'tooltiptext', $text );
	}
	
	
	/**
	 * Set a contextmenu to show when clicking the context (right) mouse or keyboard button on this element
	 * 
	 * @link https://developer.mozilla.org/en/XUL/Attribute/context
	 * @param XULPopup|XULPanel	$node		Popup node to show as context menu
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function context( $node = NULL ) {
		if( $node === NULL )
			return (isset($this->context) ? $this->context : NULL);
		
		$this->context = $node;
		
		if( $this->isRegistered && $this->context->isRegistered )
			$this->setAttribute( 'context', 'cid'.$this->context->id );
		
		return $this;
	}
	
	
	/**
	 * Set a popup menu to show when clicking on this element
	 * 
	 * @link https://developer.mozilla.org/en/XUL/Attribute/popup
	 * @param XULPopup|XULPanel	$node		Popup node to show as popup menu
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function popup( $node = NULL ) {
		if( $node === NULL )
			return (isset($this->popup) ? $this->popup : NULL);
		
		$this->popup = $node;
		
		if( $this->isRegistered && $this->popup->isRegistered )
			$this->setAttribute( 'popup', 'cid'.$this->popup->id );
		
		return $this;
	}
	
	
	/**
	 * Set a tooltip to show when hovering over this element
	 * 
	 * @link https://developer.mozilla.org/en/XUL/Attribute/tooltip
	 * @param XULToolTip	$node		Tooltip node to show as tooltip
	 * @return mixed	Returns the current value or reference to self ($this) for easy chaining of method calls
	*/
	
	public function tooltip( $node = NULL ) {
		if( $node === NULL )
			return (isset($this->tooltip) ? $this->tooltip : NULL);
		
		$this->tooltip = $node;
		
		if( $this->isRegistered && $this->tooltip->isRegistered )
			$this->setAttribute( 'tooltip', 'cid'.$this->tooltip->id );
		
		return $this;
	}
	
	
	/* Drag & Drop */
	
	
	/**
	 * Set this element as draggable
	 * 
	 * This enables the user to drag the element and drop it on another element that was indicates as dropzone 
	 * by the setDroppable() method. By default it supports one droppable flavour called 'sfNode/classname' in 
	 * which 'classname' is the PHP classname of the node. The data carried by this flavour is a reference to 
	 * the dragged node itself. Additional flavours with their accompanying data can be set through the 
	 * $flavours parameter in array with the flavour type as the key and the data as the value.
	 * 
	 * @see setDroppable()
	 * @param array	$flavours	Array of additional flavours
	*/
	
	public function setDraggable( $flavours = NULL ) {
		if( ! $this->isRegistered )
			Error::Fatal( "Can't call setDraggable() on unregistered node" );
		
		if( $flavours === NULL ) $flavours = array();
		$send = array( get_class($this) );
		
		foreach ( $flavours as $type => $content ) {
			$send[] = $type;
			$send[] = $content;
		}
		
		$this->callMethod( 'setDraggable', $send );
	}
	
	
	/**
	 * Set this element as a dropzone
	 * 
	 * Sets this element as a dropzone for draggable elements. The flavours specified in the $flavours array
	 * dicate which elements can be dropped on it. If specified, the handler method on the handler object is
	 * called when an element is dropped.
	 * 
	 * @see setDraggable()
	 * @param array			$flavours		Array of acceptable data flavours
	 * @param object|string	$eventObj		Handler object or classname
	 * @param string		$eventHandler	Method to call on the handler object or class
	*/
	
	public function setDroppable( $flavours, $eventObj = NULL, $eventHandler = NULL ) {
		if( ! $this->isRegistered )
			Error::Fatal( "Can't call setDroppable() on unregistered node" );
		if( (! is_array($flavours)) || count($flavours) < 1 )
			Error::Fatal( "Can't declare node as Droppable with empty flavour list" );
		
		$this->callMethod( 'setDroppable', $flavours );
		
		if( $eventObj && $eventHandler ) {
			$this->setEvent( 'sfdragdrop', MSG_SEND, $eventObj, $eventHandler );
		}
	}
	
	
	/* Layout initializing on attach() */
	
	/**
	 * [INTERNAL FUNCTION] Initializes the attributes of a node when it is registered
	*/
	
	public function initNodeLayout() {
		
		if( isset($this->textSize) )
			$this->textSize( $this->textSize );
		if( isset($this->textColor) )
			$this->textColor( $this->textColor );
		if( isset($this->backgroundColor) )
			$this->backgroundColor( $this->backgroundColor );
		if( isset($this->textStyle) )
			$this->textStyle( $this->textStyle );
		
		if( isset($this->width) )
			$this->width( $this->width );
		if( isset($this->height) )
			$this->height( $this->height );
		
		if( isset($this->className) )
			$this->className( $this->className );
		
		if( isset($this->flex) )
			$this->flex( $this->flex );
		if( isset($this->label) )
			$this->label( $this->label );
		if( isset($this->value) )
			$this->value( $this->value );
		if( isset($this->disabled) )
			$this->disabled( $this->disabled );
		
		if( isset($this->orient) )
			$this->orient( $this->orient );
		if( isset($this->pack) )
			$this->pack( $this->pack );
		if( isset($this->align) )
			$this->align( $this->align );
		if( isset($this->crop) )
			$this->crop( $this->crop );
		if( isset($this->equalsize) )
			$this->equalsize( $this->equalsize );
		if( isset($this->tooltiptext) )
			$this->tooltiptext( $this->tooltiptext );
		if( isset($this->accessKey) )
			$this->accessKey( $this->accessKey );
		if( isset($this->visible) )
			$this->visible( $this->visible );
		if( isset($this->hidden) )
			$this->hidden( $this->hidden );
		
		if( isset($this->paddingTop) )
			$this->paddingTop( $this->paddingTop );
		if( isset($this->paddingLeft) )
			$this->paddingLeft( $this->paddingLeft );
		if( isset($this->paddingBottom) )
			$this->paddingBottom( $this->paddingBottom );
		if( isset($this->paddingRight) )
			$this->paddingRight( $this->paddingRight );
		
		if( isset($this->marginTop) )
			$this->marginTop( $this->marginTop );
		if( isset($this->marginLeft) )
			$this->marginLeft( $this->marginLeft );
		if( isset($this->marginBottom) )
			$this->marginBottom( $this->marginBottom );
		if( isset($this->marginRight) )
			$this->marginRight( $this->marginRight );
		
		if( isset($this->context) )
			$this->context( $this->context );
		if( isset($this->popup) )
			$this->popup( $this->popup );
		if( isset($this->tooltip) )
			$this->tooltip( $this->tooltip );
		
		if( isset($this->initAttributes) && is_array($this->initAttributes) ) {
			foreach ( $this->initAttributes as $attr ) {
				if( isset($this->$attr) )
					$this->$attr( $this->$attr );
			}
		}
	}
}

