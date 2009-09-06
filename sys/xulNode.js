
SiteFusion.Classes.Node = Class.create( {
	
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
		this.eventHost[evt].msgType = type ? type : -1;
		this.eventHost[evt].reflex = null;
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
	
	eventHandler: function ( e ) {
		var obj = e.currentTarget;

		if( ! obj.sfNode ) return;
		
		window.event = e;
		return obj.sfNode.fireEvent( e.type );
	},

	fireEvent: function( e, args ) {
		if( this.eventHost[e] == null )
			SiteFusion.Error( 'Widget ' + this.sfClassName + ' does not support event ' + e );

		var sfEvent = this.eventHost[e];
		
		if( ! args )
			args = new Array();

		if( sfEvent.reflex != null ) {
			if( sfEvent.reflex( e, args ) === false )
				return false;
		}

		for( var n = 0; n < sfEvent.length; n++ ) {
			if( sfEvent[n].yield )
				sfEvent[n].yield();
		}

		if( sfEvent.msgType == 0 )
			SiteFusion.Comm.SendCommand( this.cid, e, args );
		else if( sfEvent.msgType == 1 )
			SiteFusion.Comm.QueueCommand( this.cid, e, args );

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
			eval( "func = function( eventName, eventArguments ) { " + ctrlUnHtml(code) + " }" );
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
		if( this.isPainted ) {
			this.deferredSFChildren.push( [ 'addChild', childSFNode ] );
		}
		else {
			this.directAddChild( childSFNode );
			this.sfChildren.push( childSFNode );
		}
	},

	addChildBefore: function( childSFNode, beforeSFNode ) {
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
	},
	
	removeChild: function( childSFNode ) {
		this.element.removeChild( childSFNode.element );
		
		for( var n = 0; n < this.sfChildren.length; n++ ) {
			if( this.sfChildren[n] == childSFNode ) {
				this.sfChildren.splice( n, 1 );
				break;
			}
		}
	},

	removeChildRecursive: function( childSFNode ) {
		childSFNode.isRemoved = true;
		for( var n = 0; n < childSFNode.sfChildren.length; n++ ) {
			childSFNode.removeChildRecursive( childSFNode.sfChildren[n] );
		}
	},

	directAddChild: function( childSFNode ) {
		this.element.appendChild( childSFNode.element );
	},

	directChildBefore: function( childSFNode, beforeSFNode ) {
		this.element.insertBefore( childSFNode.element, beforeSFNode.element );
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
		this.element.setAttribute( 'value', num + 0 );
		this.element.value = num + 0;
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
		if( text.substr(0,1) == '/' ) {
			var d = new Date();
			text = SiteFusion.Address + '/appimage.php?name=' + text.substr(1) + '&app=' + SiteFusion.Application + '&args=' + SiteFusion.Arguments + '&sid=' + SiteFusion.SID + '&ident=' + SiteFusion.Ident + '&cycle=' + d.getTime();
		}
		
		this.element.setAttribute( 'src', text );
	},

	accessKey: function( key ) {
		this.element.setAttribute( 'accessKey', key );
	},

	image: function( text ) {
		if( text.substr(0,1) == '/' ) {
			var d = new Date();
			text = SiteFusion.Address + '/appimage.php?name=' + text.substr(1) + '&app=' + SiteFusion.Application + '&args=' + SiteFusion.Arguments + '&sid=' + SiteFusion.SID + '&ident=' + SiteFusion.Ident + '&cycle=' + d.getTime();
		}
		
		this.element.setAttribute( 'image', text );
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

