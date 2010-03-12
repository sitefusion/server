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


SiteFusion.Classes.Node = Class.create( {
	preventDeferredInsertion: false,
	
	setEventHost: function( events, exclude ) {
		this.eventHost = {};
		
		for( var n = 0; n < SiteFusion.Comm.XULEvents.length; n++ ) {
			this.createEvent( SiteFusion.Comm.XULEvents[n] );

			if( exclude ) {
				for( var m = 0; m < exclude.length; m++ ) {
					if( SiteFusion.Comm.XULEvents[n] == exclude[m] ) {
						this.eventHost[SiteFusion.Comm.XULEvents[n]].noAutoBind = true;
						break;
					}
				}
			}
		}

		if( events ) {
			if( events.indexOf('yield') == -1 )
				events.push( 'yield' );
		}
		else events = [ 'yield' ];

		for( var n = 0; n < events.length; n++ ) {
			this.createEvent( events[n] );
		}
	},
	
	createEvent: function( evt, type ) {
		if( evt in this.eventHost )
			SiteFusion.Error( 'Event ' + evt + ' is already defined' );

		this.eventHost[evt] = new Array();
		this.eventHost[evt].msgType = type;
		this.eventHost[evt].reflex = null;
		this.eventHost[evt].blocking = false;
	},
	
	setEventListener: function( evt ) {
		var xulEvt = (SiteFusion.Comm.XULEvents.indexOf(evt) != -1);
		
		if( ! xulEvt ) return;
		if( this.eventHost[evt].noAutoBind ) return;

		if( this.eventHost[evt].msgType == -1 && this.eventHost[evt].reflex == null && this.eventHost[evt].length == 0 )
			this.element.removeEventListener( evt, this.eventHandler, true );
		else
			this.element.addEventListener( evt, this.eventHandler, true );
	},

	setEventType: function( evt, type ) {
		if( this.eventHost[evt] == null )
			SiteFusion.Error( 'Widget ' + this.sfClassName + ' does not support event ' + evt );
		
		this.eventHost[evt].msgType = type;

		var oThis = this;
		SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.setEventListener( evt ); } );
	},
	
	setEventBlocking: function( evt, blocking ) {
		if( this.eventHost[evt] == null )
			SiteFusion.Error( 'Widget ' + this.sfClassName + ' does not support event ' + evt );
		
		this.eventHost[evt].blocking = blocking;
	},
	
	eventHandler: function ( event ) {
		var obj = event.currentTarget;

		if( ! obj.sfNode ) return;
		
		var tr = obj.sfNode.fireEvent( event );
		return tr;
	},

	fireEvent: function( e, args ) {
		var event = typeof(e) == 'string' ? null:e;
		var eventName = typeof(e) == 'string' ? e:e.type;

		if( this.eventHost[eventName] == null )
			SiteFusion.Error( 'Widget ' + this.sfClassName + ' does not support event ' + e );

		var sfEvent = this.eventHost[eventName];
		
		if( ! args )
			args = new Array();

		if( sfEvent.reflex != null ) {
			this._dummy = sfEvent.reflex;
			var ret = this._dummy( eventName, args, event );
			delete this._dummy;
			if( ret === false ) return false;
		}

		for( var n = 0; n < sfEvent.length; n++ ) {
			if( sfEvent[n].yield )
				sfEvent[n].yield();
		}

		if( sfEvent.msgType == 0 )
			return SiteFusion.Comm.SendCommand( this, eventName, args );
		else if( sfEvent.msgType == 1 )
			SiteFusion.Comm.QueueCommand( this, eventName, args );

		return true;
	},

	addYielder: function() {
		var e = arguments[0];

		if( this.eventHost[e] == null )
			SiteFusion.Error( 'Widget ' + this.sfClassName + ' does not support event ' + e );

		for( var n = 1; n < arguments.length; n++ ) {
			this.eventHost[e].push( arguments[n] );
		}

		var oThis = this;
		SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.setEventListener( e ); } );
	},

	removeYielder: function() {
		var e = arguments[0];

		if( this.eventHost[e] == null )
			SiteFusion.Error( 'Widget ' + this.sfClassName + ' does not support event ' + e );

		for( var n = 1; n < arguments.length; n++ ) {
			var list = this.eventHost[e];
			for( var m = 0; m < list.length; m++ ) {
				if( arguments[n] == list[m] )
					list.splice( m--, 1 );
			}
		}

		var oThis = this;
		SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.setEventListener( e ); } );
	},

	addReflex: function( e, code ) {
		if( this.eventHost[e] == null )
			SiteFusion.Error( 'Widget ' + this.sfClassName + ' does not support event ' + e );
		
		var func;
		
		try {
			eval( "func = function( eventName, eventArguments, eventObject ) { " + code + " }" );
		}
		catch ( e ) {
			SiteFusion.Error( 'Reflex contains an error: ' + e );
		}
		
		this.eventHost[e].reflex = func;

		var oThis = this;
		SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.setEventListener( e ); } );
	},

	removeReflex: function( e ) {
		if( this.eventHost[e] == null )
			SiteFusion.Error( 'Widget ' + this.sfClassName + ' does not support event ' + e );

		this.eventHost[e].reflex = null;

		var oThis = this;
		SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.setEventListener( e ); } );
	},
	
	addChild: function( childSFNode ) {
		try {
			if( childSFNode.parentNode )
				return;
		
			if( this.isPainted ) {
				this.deferredSFChildren.push( [ 'addChild', childSFNode ] );
			}
			else {
				this.directAddChild( childSFNode );
				this.sfChildren.push( childSFNode );
			}
		}
		catch (e) { SiteFusion.consoleMessage( 'Exception in addChild: '+e ); }
	},

	addChildBefore: function( childSFNode, beforeSFNode ) {
		try {
			if( childSFNode.parentNode )
				return;
			
			if( this.isPainted ) {
				this.deferredSFChildren.push( [ 'addChildBefore', childSFNode, beforeSFNode ] );
			}
			else {
				this.directAddChildBefore( childSFNode, beforeSFNode );
	
				for( var n = 0; n < this.sfChildren.length; n++ ) {
					if( this.sfChildren[n] == beforeSFNode ) {
						this.sfChildren.splice( n, 0, childSFNode );
						break;
					}
				}
			}
		}
		catch (e) { SiteFusion.consoleMessage( 'Exception in addChildBefore: '+e ); }
	},
	
	removeChild: function( childSFNode ) {
		try {
			this.element.removeChild( childSFNode.element );
			
			for( var n = 0; n < this.sfChildren.length; n++ ) {
				if( this.sfChildren[n] == childSFNode ) {
					this.sfChildren.splice( n, 1 );
					break;
				}
			}
		}
		catch (e) { SiteFusion.consoleMessage( 'Exception in removeChild: '+e ); }
	},

	removeChildRecursive: function( childSFNode ) {
		childSFNode.isRemoved = true;
		for( var n = 0; n < childSFNode.sfChildren.length; n++ ) {
			childSFNode.removeChildRecursive( childSFNode.sfChildren[n] );
		}
	},

	directAddChild: function( childSFNode ) {
		try {
			this.element.appendChild( childSFNode.element );
		}
		catch (e) { SiteFusion.consoleMessage( 'Exception in directAddChild: '+e ); }
	},

	directAddChildBefore: function( childSFNode, beforeSFNode ) {
		try {
			if( ! beforeSFNode )
				this.directAddChild( childSFNode );
			else
				this.element.insertBefore( childSFNode.element, beforeSFNode.element );
		}
		catch (e) { SiteFusion.consoleMessage( 'Exception in directAddChildBefore: '+e ); }
	},

	addChildSilent: function( childSFNode ) {
		this.sfChildren.push( childSFNode );
	},

	removeChildSilent: function( childSFNode ) {
		for( var n = 0; n < this.sfChildren.length; n++ ) {
			if( this.sfChildren[n] == childSFNode ) {
				this.sfChildren.splice( n, 1 );
				return;
			}
		}
	},

	width: function( val ) {
		this.element.setAttribute( 'width', val + 0 );
	},

	height: function( val ) {
		this.element.setAttribute( 'height', val + 0 );
	},

	flex: function( val ) {
		this.element.setAttribute( 'flex', val + 0 );
	},

	label: function( text ) {
		this.element.setAttribute( 'label', text );
	},

	disabled: function( state ) {
		this.element.setAttribute( 'disabled', (state ? 'true':'false') );
	},

	textValue: function( text ) {
		this.element.setAttribute( 'value', '' + text );
		this.element.value = '' + text;
	},

	numericValue: function( num ) {
		this.element.setAttribute( 'value', num );
		this.element.value = num;
	},

	mode: function( mode ) {
		this.element.setAttribute( 'mode', '' + mode );
	},

	orient: function( text ) {
		this.element.setAttribute( 'orient', '' + text );
	},

	pack: function( text ) {
		this.element.setAttribute( 'pack', '' + text );
	},

	align: function( text ) {
		this.element.setAttribute( 'align', '' + text );
	},

	crop: function( text ) {
		this.element.setAttribute( 'crop', '' + text );
		this.element.crop = text;
	},

	src: function( text ) {
		this.element.setAttribute( 'src', this.parseImageURL(text) );
	},

	accessKey: function( key ) {
		this.element.setAttribute( 'accessKey', key );
	},

	image: function( text ) {
		this.element.setAttribute( 'image', this.parseImageURL(text) );
	},
	
	value: function( val ) {
		this.textValue( val );
	},

	focus: function() {
		var oThis = this;
		SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.element.focus(); } );
	},

	blur: function() {
		var oThis = this;
		SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.element.blur(); } );
	},
	
	parseImageURL: function( text ) {
		if( text.substr(0,1) == '/' ) {
			var d = new Date();
			text = SiteFusion.Address + '/appimage.php?name=' + text.substr(1) + '&app=' + SiteFusion.Application + '&args=' + SiteFusion.Arguments + '&sid=' + SiteFusion.SID + '&ident=' + SiteFusion.Ident /*+ '&cycle=' + d.getTime()*/;
		}
		return text;
	},

	backgroundImage: function ( url ) {
		this.element.style.backgroundImage = 'url(' + this.parseImageURL(url) + ')';
	},
	
	setDraggable: function() {
		var clsName = arguments[0];
		var flavours = [];

		for( var n = 1; n < arguments.length; n++ ) {
			flavours.push( arguments[n] );
		}

		this.element.addEventListener( 'draggesture', this.onDragGestureEvent, true );
		this.draggable = true;
		this.draggableClassname = clsName;
		this.draggableFlavours = flavours;
	},

	setDroppable: function() {
		this.dropObserver = new this.DropObserver( this, arguments );
		this.droppable = true;
	},

	onDragGestureEvent: function( event ) {
		nsDragAndDrop.startDrag( event, this.DragObserver );
	},

	DragObserver: {
		onDragStart: function( event, transferData, action ) {
			var obj = event.target.sfNode;
			transferData.data = new TransferData();
			transferData.data.addDataForFlavour( 'sfNode/'+obj.draggableClassname, obj.cid );

			for( var n = 0; n < obj.draggableFlavours.length; n += 2 ) {
				transferData.data.addDataForFlavour( obj.draggableFlavours[n], obj.draggableFlavours[n+1] );
			}

			obj.fireEvent( 'sfdragstart' );
		}
	},
	
	DropObserver: Class.create( {
		initialize: function( sfNode, flavours ) {
			this.sfNode = sfNode;

			var oThis = this;
			this.sfNode.element.addEventListener( 'dragover', function(event) { nsDragAndDrop.dragOver(event,oThis); }, true );
			this.sfNode.element.addEventListener( 'dragdrop', function(event) { nsDragAndDrop.drop(event,oThis); }, true );

			var flavourSet = new FlavourSet();

			for( var n = 0; n < flavours.length; n++ ) {
				flavourSet.appendFlavour( flavours[n] );
			}

			this.flavours = flavourSet;
		},
		
		getSupportedFlavours: function() {
			return this.flavours;
		},

		onDragOver: function( event, flavour, session ) {
			this.sfNode.fireEvent( 'sfdragover', [ flavour ] );
		},

		onDrop: function( event, dropdata, session ) {
			var data = dropdata.data;
			if( dropdata.flavour.contentType.substr( 0, 7 ) == 'sfNode/' )
				data = window.registry[data];
			this.sfNode.fireEvent( 'sfdragdrop', [ data ] );
		}
	} ),
	
	toJSON: function() {
		return Object.toJSON( { '__sfNode': this.cid } );
	}
} );

