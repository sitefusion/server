<?php

/**
 * @package SiteFusion
 * @subpackage Node
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
	
	final public function createClientEvent( $event, $type = -1 ) {
		if(! $this->isRegistered )
			Error::CallerFatal( "Can't create a client-side event on an unregistered node" );
		
		$this->callMethod( 'createEvent', array( $event, $type ) );
	}
	
	final public function setEvent( $event, $type, $obj, $func, $yielders = NULL ) {
		$this	-> setEventType( $event, $type )
			-> setEventHandler( $event, $obj, $func );
		
		if( $yielders != NULL )
			$this->setEventYielder( $event, $yielders );
		
		return $this;
	}

	final public function setEventHandler( $event, $obj, $func ) {
		$handler = array( $obj, $func );
		
		if( isset( $this->eventHandler[$event] ) )
			array_push( $this->eventHandler[$event], $handler );
		else
			$this->eventHandler[$event] = array( $handler );
		
		if( is_object($obj) )
			array_push( $obj->handlerFor, array($event,$this,$func) );
		
		return $this;
	}
	
	final public function removeEventHandler( $event, $obj, $func ) {
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
	
	final public function setEventYielder( $event, $obj ) {
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
	
	final public function removeEventYielder( $event, $obj ) {
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
	
	final public function setEventType( $event, $type ) {
		if( ! $this->isRegistered )
			Error::CallerFatal( 'Can`t set event type because the hosting node is not registered' );
		
		$this->eventType[$event] = $type;
		
		$this->callMethod( 'setEventType', array($event,$type) );
		
		return $this;
	}
	
	final public function setEventReflex( $event, $code ) {
		if( ! $this->isRegistered )
			Error::CallerFatal( 'Can`t set event reflex because the node is not registered' );
		
		$this->callMethod( 'addReflex', array($event,$code) );
		
		return $this;
	}
	
	final public function removeEventReflex( $event ) {
		if( ! $this->isRegistered )
			Error::CallerFatal( 'Can`t remove event reflex because the node is not registered' );
		
		$this->callMethod( 'removeReflex', array($event) );
		
		return $this;
	}

	final public function commandHandler( $event, $data ) {
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
	
	final public function fireLocalEvent( $event, $args = NULL ) {
		if( $args === NULL )
			$args = array();
		
		$eventobj = $this->commandHandler( $event, $args );
		
		return $eventobj;
	}
	
	final public function fireClientEvent( $event, $args = NULL ) {
		if(! $this->isRegistered )
			Error::CallerFatal( 'Unregistered node cannot fire a ClientEvent' );
		
		$this->rootApplication->sendCommand(
			Comm::CallMethod( $this, 'fireEvent', array($event,$args) )
		);
	}
	
	final public function yield() {
		if(! $this->isRegistered )
			Error::CallerFatal( 'Unregistered node cannot yield' );
		
		$this->rootApplication->sendCommand(
			Comm::CallMethod( $this, 'yield' )
		);
		
		return $this;
	}
	
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
	
	public function createRemoteObject( $args = NULL ) {
		if(! $this->isRegistered )
			Error::CallerFatal( 'Can\'t call createRemoteObject on unregistered node' );
		
		$this->rootApplication->sendCommand(
			Comm::CreateRemoteObject( $this, $args )
		);
		
		return $this;
	}

	public function insertElement() {
		if(! $this->isRegistered )
			Error::CallerFatal( 'Can\'t call insertElement on unregistered node' );
		
		$this->rootApplication->sendCommand(
			Comm::InsertElement( $this )
		);
		
		return $this;
	}

	public function removeElement() {
		if(! $this->isRegistered )
			Error::CallerFatal( 'Can\'t call removeElement on unregistered node' );
		
		$this->rootApplication->sendCommand(
			Comm::RemoveElement( $this )
		);
		
		return $this;
	}

	public function unRegister() {
		if(! $this->isRegistered )
			Error::CallerFatal( 'Can\'t call unRegister on unregistered node' );
		
		$this->rootApplication->sendCommand(
			Comm::UnRegister( $this )
		);
		
		return $this;
	}

	public function callMethod( $name, $args = NULL ) {
		if(! $this->isRegistered )
			Error::CallerFatal( 'Can\'t call callMethod on unregistered node' );
		
		$this->rootApplication->sendCommand(
			Comm::CallMethod( $this, $name, $args )
		);
		
		return $this;
	}

	public function setProperty( $name, $value ) {
		if(! $this->isRegistered )
			Error::CallerFatal( 'Can\'t call setProperty on unregistered node' );
		
		$this->rootApplication->sendCommand(
			Comm::SetProperty( $this, $name, $value )
		);
		
		return $this;
	}
	
	public function setMethod( $method, $args, $code ) {
		if(! $this->isRegistered )
			Error::CallerFatal( 'Can\'t call setMethod on unregistered node' );
		
		$this->rootApplication->sendCommand(
			Comm::SetMethod( $this, $method, $args, $code )
		);
		
		return $this;
	}

	public function setAttribute( $name, $value ) {
		if(! $this->isRegistered )
			Error::CallerFatal( 'Can\'t call setAttribute on unregistered node' );
		
		$this->rootApplication->sendCommand(
			Comm::SetAttribute( $this, $name, $value )
		);
		
		return $this;
	}

	public function removeAttribute( $name ) {
		if(! $this->isRegistered )
			Error::CallerFatal( 'Can\'t call removeAttribute on unregistered node' );
		
		$this->rootApplication->sendCommand(
			Comm::RemoveAttribute( $this, $name )
		);
		
		return $this;
	}
	
	public function setStyle( $name, $value ) {
		if(! $this->isRegistered )
			Error::CallerFatal( 'Can\'t call setStyle on unregistered node' );
		
		$this->rootApplication->sendCommand(
			Comm::SetProperty( $this, 'element.style.'.$name, $value )
		);
		
		return $this;
	}



	/* Generic methods */
	
	public function attach() {
		$this->createRemoteObject( array( $this->hostWindow ) );
		$this->insertElement();
	}

	public function detach() {
		$this->removeElement();
		$this->unRegister();
	}
	
	public function focus() {
		$this->callMethod( 'focus' );
		
		return $this;
	}
	
	public function blur() {
		$this->callMethod( 'blur' );
		
		return $this;
	}
	
	public function removeNode() {
		if(! $this->isChild )
			Error::CallerFatal( 'removeNode: Node is not a child' );
		
		$this->parent->removeChild( $this );
	}
	
	public function extractNode() {
		if(! $this->isChild )
			Error::CallerFatal( 'extractNode: Node is not a child' );
		
		$this->parent->extractChild( $this );
	}
	
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
	
	
	/* Attribute setter */
	
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
	
	public function size( $width, $height ) {
		$this->width( $width );
		$this->height( $height );
		
		return $this;
	}
	
	public function width( $width = NULL ) {
		if( $width === NULL )
			return (isset($this->width) ? $this->width : NULL);
		
		$this->width = $width;
		
		if( $this->isRegistered )
			$this->setAttribute( 'width', $width );
		
		return $this;
	}
	
	public function height( $height = NULL ) {
		if( $height === NULL )
			return (isset($this->height) ? $this->height : NULL);
		
		$this->height = $height;
		
		if( $this->isRegistered )
			$this->setAttribute( 'height', $height );
		
		return $this;
	}
	
	
	/* Padding */
	
	public function padding( $pix = NULL ) {
		if( $pix === NULL )
			return (isset($this->padding) ? $this->padding : NULL);
		
		$this->padding = $pix;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.style.padding', $this->padding );
		
		return $this;
	}
	
	public function paddingTop( $pix = NULL ) {
		if( $pix === NULL )
			return (isset($this->paddingTop) ? $this->paddingTop : NULL);
		
		$this->paddingTop = $pix;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.style.paddingTop', $this->paddingTop );
		
		return $this;
	}

	public function paddingRight( $pix = NULL ) {
		if( $pix === NULL )
			return (isset($this->paddingRight) ? $this->paddingRight : NULL);
		
		$this->paddingRight = $pix;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.style.paddingRight', $this->paddingRight );
		
		return $this;
	}

	public function paddingBottom( $pix = NULL ) {
		if( $pix === NULL )
			return (isset($this->paddingBottom) ? $this->paddingBottom : NULL);
		
		$this->paddingBottom = $pix;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.style.paddingBottom', $this->paddingBottom );
		
		return $this;
	}

	public function paddingLeft( $pix = NULL ) {
		if( $pix === NULL )
			return (isset($this->paddingLeft) ? $this->paddingLeft : NULL);
		
		$this->paddingLeft = $pix;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.style.paddingLeft', $this->paddingLeft );
		
		return $this;
	}



	/* Margin */
	
	public function margin( $pix = NULL ) {
		if( $pix === NULL )
			return (isset($this->margin) ? $this->margin : NULL);
		
		$this->margin = $pix;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.style.margin', $this->margin );
		
		return $this;
	}
	
	public function marginTop( $pix = NULL ) {
		if( $pix === NULL )
			return (isset($this->marginTop) ? $this->marginTop : NULL);
		
		$this->marginTop = $pix;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.style.marginTop', $this->marginTop );
		
		return $this;
	}

	public function marginRight( $pix = NULL ) {
		if( $pix === NULL )
			return (isset($this->marginRight) ? $this->marginRight : NULL);
		
		$this->marginRight = $pix;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.style.marginRight', $this->marginRight );
		
		return $this;
	}

	public function marginBottom( $pix = NULL ) {
		if( $pix === NULL )
			return (isset($this->marginBottom) ? $this->marginBottom : NULL);
		
		$this->marginBottom = $pix;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.style.marginBottom', $this->marginBottom );
		
		return $this;
	}

	public function marginLeft( $pix = NULL ) {
		if( $pix === NULL )
			return (isset($this->marginLeft) ? $this->marginLeft : NULL);
		
		$this->marginLeft = $pix;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.style.marginLeft', $this->marginLeft );
		
		return $this;
	}
	
	
	/* Text and Background Properties */
	
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


	public function className( $className = NULL ) {
		if( $className === NULL )
			return (isset($this->className) ? $this->className : NULL);
		
		$this->className = (string) $className;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.className', $this->className );
		
		return $this;
	}
	
	public function flex( $val = NULL ) {
		if( $val === NULL )
			return (isset($this->flex) ? $this->flex : NULL);
		
		$this->flex = (int) $val;
		
		if( $this->isRegistered )
			$this->callMethod( 'flex', array($this->flex) );
		
		return $this;
	}
	
	public function disabled( $state = NULL ) {
		if( $state === NULL )
			return (isset($this->disabled) ? $this->disabled : NULL);
		
		$this->disabled = (bool) $state;
		
		if( $this->isRegistered )
			$this->callMethod( 'disabled', array($this->disabled) );
		
		return $this;
	}
	
	public function label( $text = NULL ) {
		if( $text === NULL )
			return (isset($this->label) ? $this->label : NULL);
		
		$this->label = $text;
		
		if( $this->isRegistered )
			$this->callMethod( 'label', $text );
		
		return $this;
	}
	
	public function value( $val = NULL ) {
		if( $val === NULL )
			return (isset($this->value) ? $this->value : NULL);
		
		$this->value = $val;
		
		if( $this->isRegistered )
			$this->callMethod( 'value', array($this->value) );
		
		return $this;
	}
	
	public function orient( $dir = NULL ) {
		if( $dir === NULL )
			return (isset($this->orient) ? $this->orient : NULL);
		
		$this->orient = ($dir == 'h' || $dir == 'horizontal' ? 'horizontal':'vertical');
		
		if( $this->isRegistered )
			$this->callMethod( 'orient', array($this->orient) );
		
		return $this;
	}

	public function pack( $pack = NULL ) {
		if( $pack === NULL )
			return (isset($this->pack) ? $this->pack : NULL);
		
		$this->pack = $pack;
		
		if( $this->isRegistered )
			$this->callMethod( 'pack', array($this->pack) );
		
		return $this;
	}

	public function align( $align = NULL ) {
		if( $align === NULL )
			return (isset($this->align) ? $this->align : NULL);
		
		$this->align = $align;
		
		if( $this->isRegistered )
			$this->callMethod( 'align', array($this->align) );
		
		return $this;
	}

	public function crop( $crop = NULL ) {
		if( $crop === NULL )
			return (isset($this->crop) ? $this->crop : NULL);
		
		$this->crop = $crop;
		
		if( $this->isRegistered )
			$this->callMethod( 'crop', array($this->crop) );
		
		return $this;
	}

	public function accessKey( $key = NULL ) {
		if( $key === NULL )
			return (isset($this->accessKey) ? $this->accessKey : NULL);
		
		$this->accessKey = $key;
		
		if( $this->isRegistered )
			$this->callMethod( 'accessKey', array($this->accessKey) );
		
		return $this;
	}
	
	public function visible( $vis = NULL ) {
		if( $vis === NULL )
			return (isset($this->visible) ? $this->visible : NULL);
		
		$this->visible = (bool) $vis;
		
		if( $this->isRegistered )
			$this->setAttribute( 'visible', $this->visible );
		
		return $this;
	}

	public function hidden( $hid = NULL ) {
		return $this->attributeMethod( 'hidden', $hid );
	}

	public function equalsize( $state = NULL ) {
		return $this->attributeMethod( 'equalsize', $state );
	}
	
	public function tooltiptext( $text = NULL ) {
		return $this->attributeMethod( 'tooltiptext', $text );
	}
	
	public function context( $node = NULL ) {
		if( $node === NULL )
			return (isset($this->context) ? $this->context : NULL);
		
		$this->context = $node;
		
		if( $this->isRegistered && $this->context->isRegistered )
			$this->setAttribute( 'context', 'cid'.$this->context->id );
		
		return $this;
	}
	
	public function popup( $node = NULL ) {
		if( $node === NULL )
			return (isset($this->popup) ? $this->popup : NULL);
		
		$this->popup = $node;
		
		if( $this->isRegistered && $this->popup->isRegistered )
			$this->setAttribute( 'popup', 'cid'.$this->popup->id );
		
		return $this;
	}
	
	public function tooltip( $node = NULL ) {
		if( $node === NULL )
			return (isset($this->tooltip) ? $this->tooltip : NULL);
		
		$this->tooltip = $node;
		
		if( $this->isRegistered && $this->tooltip->isRegistered )
			$this->setAttribute( 'tooltip', 'cid'.$this->tooltip->id );
		
		return $this;
	}
	
	
	/* Drag & Drop */
	
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
	
	public function setDroppable( $flavours, $eventObj = NULL, $eventHandler = NULL ) {
		if( ! $this->isRegistered )
			Error::Fatal( "Can't call setDroppable() on unregistered node" );
		if( (! is_array($flavours)) || count($flavours) < 1 )
			Error::Fatal( "Can't declare node as Droppable with empty flavour list" );
		
		$this->callMethod( 'setDroppable', $flavours );
		
		if( $eventObj && $eventHandler ) {
			$this->setEvent( 'sfdragdrop', sendMessage, $eventObj, $eventHandler );
		}
	}
	
	
	/* Layout initializing on attach() */
	
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

