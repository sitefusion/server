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


SiteFusion.Comm = {
	CommConnection: null,
	RevCommConnection: null,
	IdleHandlers: [],
	BusyHandlers: [],
	Buffer: [],
	BufferSend: false,
	UseBuffer: false,
	Queue: '',
	Timer: null,
	CommandId: 1,
	XULEvents: [
		'blur', 'broadcast', 'change', 'click', 'command', 'commandupdate',
		'contextmenu', 'dblclick', /*'dragdrop', 'dragenter', 'dragexit', 'draggesture',
		'dragover',*/ 'focus', 'input', 'keydown', 'keypress', 'keyup', 'load', 'mousedown',
		'mousemove', 'mouseout', 'mouseover', 'mouseup', 'overflow', 'overflowchanged',
		'popuphidden', 'popuphiding', 'popupshowing', 'popupshown', 'select',
		'syncfrompreference', 'synctopreference', 'underflow', 'unload', 'sfdragstart', 'sfdragover', 'sfdragdrop'
	],
	EventReturnValue: null,
	
	RevComm: function() {
		var x = new XMLHttpRequest;
		x.open( 'GET', SiteFusion.Address + '/revcomm.php?app=' + SiteFusion.Application + '&args=' + SiteFusion.Arguments + '&sid=' + SiteFusion.SID + '&ident=' + SiteFusion.Ident, true );
		x.onreadystatechange = function( evt ) {
			if( x.readyState == 4 ) {
				if( x.status == 200 ) {
					if( x.responseText != '' && x.responseText.substr( -16 ) != '"EXEC_COMPLETE";' ) {
						SiteFusion.Error( x.responseText.replace( /<.+?>/g, '' ) );
						return;
					}
					
					SiteFusion.Execute( x.responseText );
					SiteFusion.Interface.HandleDeferredChildAdditions();
					SiteFusion.Comm.RevComm();
				}
			}
		};
		x.send( null );
		SiteFusion.Comm.RevCommConnection = x;
	},
	
	Idle: function() {
		for( var n = 0; n < this.IdleHandlers.length; n++ ) {
			if(! this.IdleHandlers[n] ) {
				this.IdleHandlers.splice( n--, 1 );
				continue;
			}
			this.IdleHandlers[n]();
		}

		if( this.Buffer.length ) {
			for( var n = 0; n < this.Buffer.length; n++ ) {
				var cmd = this.Buffer[n];
				if( cmd[0] )
					this.QueueCommand( cmd[1], cmd[2], cmd[3] );
				else
					this.SendCommand( cmd[1], cmd[2], cmd[3] );
			}
		}

		this.UseBuffer = false;

		this.Timer = setTimeout( this.Inactive, 4 * 60 * 1000 );
	},

	Inactive: function() {
		SiteFusion.RootWindow.fireEvent( 'idle' );
	},

	Busy: function() {
		this.UseBuffer = true;

		for( var n = 0; n < this.BusyHandlers.length; n++ ) {
			if(! this.BusyHandlers[n] ) {
				this.BusyHandlers.splice( n--, 1 );
				continue;
			}
			this.BusyHandlers[n]();
		}
	},

	BufferCommand: function( type, cid, evt, args ) {
		this.Buffer.push( [ type, cid, evt, args ] );
	},
	
	RawQueueCommand: function( cid, evt, args ) {
		args.unshift( evt );
		args.unshift( cid );
		var data = args.toJSON();
		
		this.Queue += (this.Queue == '' ? '':'&') + this.CommandId++ + '=' + data;
	},

	SendCommand: function( cid, evt, args ) {
		if( this.UseBuffer ) {
			this.BufferCommand( 0, cid, evt, args );
			return;
		}

		this.RawQueueCommand( cid, evt, args );

		this.QueueFlush();
	},

	QueueCommand: function( cid, evt, args ) {
		if( this.UseBuffer ) {
			this.BufferCommand( 1, cid, evt, args );
			return;
		}

		this.RawQueueCommand( cid, evt, args );
	},

	QueueFlush: function() {
		if( this.Queue == '' ) return;

		if( this.Timer )
			clearTimeout( this.Timer );

		this.UseBuffer = true;

		for( var n = 0; n < this.BusyHandlers.length; n++ ) {
			this.BusyHandlers[n]();
		}


		this.SendQueue();

		while( this.Buffer.length ) {
			var cmd = this.Buffer.shift();
			this.RawQueueCommand( cmd[1], cmd[2], cmd[3] );
			if( cmd[0] == 0 )
				this.SendQueue();
		}

		for( var n = 0; n < this.IdleHandlers.length; n++ ) {
			this.IdleHandlers[n]();
		}

		this.UseBuffer = false;

		this.Timer = setTimeout( this.Inactive, 4 * 60 * 1000 );
	},

	SendQueue: function() {
		var queue = this.Queue;
		this.Queue = '';
		this.CommandId = 1;

		var x = new XMLHttpRequest;
		x.open( 'POST', SiteFusion.Address + '/comm.php?app=' + SiteFusion.Application + '&args=' + SiteFusion.arguments + '&sid=' + SiteFusion.SID + '&ident=' + SiteFusion.Ident, false );
		x.setRequestHeader( 'Content-Type', 'application/x-www-form-urlencoded' );
		x.send( queue );
		
		this.CommConnection = x;
		
		if( x.status != 200 ) {
			var oThis = this;
			setTimeout( function() { oThis.QueueFlush() }, 10000 );
			return;
		}

		if( x.responseText.substr( -16 ) != '"EXEC_COMPLETE";' ) {
			SiteFusion.Error( x.responseText.replace( /<.+?>/g, '' ) );
			return;
		}

		SiteFusion.Execute( x.responseText );
		SiteFusion.Interface.HandleDeferredChildAdditions();
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


