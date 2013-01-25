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
 * Update 23-1-2013:  Added wheel event
 */

SiteFusion.Comm = {
	MSG_NONE: -1,
	MSG_SEND: 0,
	MSG_QUEUE: 1,
	
	ShowProgressCursorDelay: 500,
	CommTransmission: null,
	RevCommTransmission: null,
	TransmissionQueue: [],
	IdleHandlers: [],
	BusyHandlers: [],
	ProgressTimer: null,
	Queue: [],
	XULEvents: [
		'blur', 'broadcast', 'change', 'click', 'command', 'commandupdate', 'resize', 'transitionend',
		'contextmenu', 'dblclick', /*'dragdrop', 'dragenter', 'dragexit', 'draggesture',
		'dragover',*/ 'focus', 'input', 'keydown', 'keypress', 'keyup', 'load', 'mousedown',
		'mousemove', 'mouseout', 'mouseover', 'mouseup', 'overflow', 'overflowchanged',
		'popuphidden', 'popuphiding', 'popupshowing', 'popupshown', 'select',
		'syncfrompreference', 'synctopreference', 'underflow', 'unload', 'sfdragstart', 'sfdragover', 'sfdragdrop','dragexit','scroll','wheel'
	],
	
	
	RevComm: function() {
		new SiteFusion.Comm.Transmission();
	},

	SendCommand: function( sfNode, evt, args ) {
		this.QueueCommand( sfNode, evt, args );
		return this.QueueFlush( sfNode.eventHost[evt].blocking );
	},

	QueueCommand: function( sfNode, evt, args ) {
		args.unshift( evt );
		args.unshift( sfNode.cid );
		
		this.Queue.push( args.toJSON() );
	},
	
	QueueFlush: function( blocking ) {
		if( ! this.Queue.length ) return;
		
		var queue = this.Queue;
		this.Queue = [];
		
		var tr = new SiteFusion.Comm.Transmission( blocking ? true:false, '['+queue.join(',')+']' );
		return tr;
	},
	
	StartProgressTimer: function() {
		if( ! this.ProgressTimer ) {
			var oThis = this;
			this.ProgressTimer = setTimeout( function() { oThis.ProgressTimer = null; SiteFusion.Interface.CursorBackground(); }, this.ShowProgressCursorDelay );
		}
	},
	
	StopProgressTimer: function() {
		if( this.ProgressTimer ) {
			clearTimeout( this.ProgressTimer );
			this.ProgressTimer = null;
		}
		SiteFusion.Interface.CursorIdle();
	},
	
	AddToRegistry: function( id, node ) {
		SiteFusion.Registry[parseInt(id)] = node;
		node.cid = id;
		
		node.isPainted = false;
		node.sfChildren = [];
		node.deferredSFChildren = [];
		
		if( 'element' in node )
			node.element.setAttribute( 'id', 'cid'+id );

		return node;
	}
};

SiteFusion.Comm.Transmission = Class.create( {
	STATE_NEW: 0,
	STATE_PROCESSING: 1,
	STATE_CONNECTED: 2,
	STATE_FINISHED: 3,
	
	state: null,
	blocking: null,
	payload: null,
	payloadEncoding: null,
	reverseInitiative: false,
	request: null,
	onstatechange: null,
	
	initialize: function( blocking, payload ) {
		if( typeof(payload) == 'undefined' )
			this.reverseInitiative = true;
		else
			this.payload = payload;
		
		this.blocking = (blocking && !this.reverseInitiative) ? true:false;
		
		this.state = this.STATE_NEW;
		if( typeof(this.onstatechange) == 'function' )
			this.onstatechange();
		
		if( this.reverseInitiative ) {
			SiteFusion.Comm.RevCommTransmission = this;
			this.send();
		}
		else if( this.blocking || !SiteFusion.Comm.CommTransmission ) {
			SiteFusion.Comm.CommTransmission = this;
			this.send();
		}
		else
			SiteFusion.Comm.TransmissionQueue.push( this );
	},
	
	send: function() {
		// RevComm connections can proceed right away
		if( this.reverseInitiative ) {
			this.openHttpRequest();
			return;
		}
		
		if( this.blocking ) {
			for( var n = 0; n < SiteFusion.Comm.BusyHandlers.length; n++ ) {
				SiteFusion.Comm.BusyHandlers[n]();
			}
		}
		else SiteFusion.Comm.StartProgressTimer();
		
		// Compress payload if larger than 128 bytes and asynchronous
		if( this.payload.length > 128 && !this.blocking ) {
			this.payloadEncoding = 'application/x-gzip';
			new SiteFusion.Comm.Transmission.Compressor( this );
			this.state = this.STATE_PROCESSING;
			if( typeof(this.onstatechange) == 'function' )
				this.onstatechange();
		}
		else {
			this.payloadEncoding = 'application/octet-stream';
			this.openHttpRequest();
		}
	},
	
	openHttpRequest: function() {
		this.request = new XMLHttpRequest;
		
		var aSync = (this.reverseInitiative || !this.blocking);
		
		this.state = this.STATE_CONNECTED;
		if( typeof(this.onstatechange) == 'function' )
			this.onstatechange();
		
		try {
			this.request.open( 'POST',
				SiteFusion.Address + '/' + (this.reverseInitiative ? 'revcomm':'comm') + '.php'
				+ '?app=' + SiteFusion.Application
				+ '&args=' + SiteFusion.Arguments
				+ '&sid=' + SiteFusion.SID
				+ '&ident=' + SiteFusion.Ident
				+ '&clientid=' + SiteFusion.ClientID,
				aSync
			);
			
			if( this.payloadEncoding )
				this.request.setRequestHeader( 'Content-Type', this.payloadEncoding );
		
			if( aSync ) {
				var transmission = this;
				this.request.onreadystatechange = function( event ) {
					if( this.readyState == 4 )
						transmission.handleResponse();
				};
			}
			
			this.request[this.payloadEncoding == 'application/x-gzip' ? 'sendAsBinary':'send']( this.payload );
		}
		catch ( e ) {
			var oThis = this;
			setTimeout( function() { oThis.openHttpRequest(); }, 1000 );
			return;
		}
			
		
		if( !aSync )
			this.handleResponse();
	},
	
	handleResponse: function() {
		if( this.request.status != 200 ) {
			if( !this.reverseInitiative ) {
				var oThis = this;
				setTimeout( function() { oThis.openHttpRequest(); }, 1000 );
			}
			return;
		}
		
		var contentType = this.request.getResponseHeader( 'Content-type' );
		
		if( contentType.match( /sitefusion\/error/ ) ) {
			try {
				var error = eval( '('+this.request.responseText+')' );
				SiteFusion.HandleError( error );
			}
			catch ( ex ) {
				SiteFusion.HandleError( { 'type': null, 'message': this.request.responseText.replace( /<.+?>/g, '' ) } );
			}
			return;
		}
		
		SiteFusion.Execute( this.request.responseText );
		SiteFusion.Interface.HandleDeferredChildAdditions();
		
		if( this.blocking ) {
			for( var n = 0; n < SiteFusion.Comm.IdleHandlers.length; n++ ) {
				SiteFusion.Comm.IdleHandlers[n]();
			}
		}
		
		if( this.reverseInitiative ) {
			this.send();
		}
		else if( SiteFusion.Comm.TransmissionQueue.length ) {
			SiteFusion.Comm.CommTransmission = SiteFusion.Comm.TransmissionQueue.shift();
			SiteFusion.Comm.CommTransmission.send();
		}
		else {
			SiteFusion.Comm.CommTransmission = null;
			SiteFusion.Comm.StopProgressTimer();
		}
		
		this.state = this.STATE_FINISHED;
		if( typeof(this.onstatechange) == 'function' )
			this.onstatechange();
	},
	
	abort: function() {
		if( this.state >= this.STATE_CONNECTED ) {
			this.request.abort();
			SiteFusion.Comm.StopProgressTimer();
		}
	}
} );

SiteFusion.Comm.Transmission.Compressor = Class.create( {
	buffer: '',
	transmission: null,
	
	initialize: function( transmission ) {
		this.transmission = transmission;
		
		var utfConv = Cc["@mozilla.org/intl/scriptableunicodeconverter"].createInstance(Ci.nsIScriptableUnicodeConverter);
		utfConv.charset = "UTF-8";
		
		var utfStream = utfConv.convertToInputStream( this.transmission.payload );
		
		var scs = Cc["@mozilla.org/streamConverters;1"].getService( Ci.nsIStreamConverterService );
		var compressor = scs.asyncConvertData( "uncompressed", "deflate", this, null );
		
		var pump = Cc["@mozilla.org/network/input-stream-pump;1"].createInstance( Ci.nsIInputStreamPump );
		pump.init( utfStream, -1, -1, 0, 0, true );
		pump.asyncRead( compressor, null );
	},
	
	onStartRequest: function( request, context ) {},
	
	onStopRequest: function( request, context, statuscode ) {
		this.transmission.payload = this.buffer;
		this.transmission.openHttpRequest();
	},
	
	onDataAvailable: function( request, context, inputStream, offset, count ) {
		var stream = Cc["@mozilla.org/binaryinputstream;1"].createInstance( Ci.nsIBinaryInputStream );
		stream.setInputStream( inputStream );
		this.buffer += stream.readBytes( count );
	}
} );

