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
// theFrontDoor.
// Portions created by the Initial Developer are Copyright (C) 2009
// the Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Nikki Auburger <nikki@thefrontdoor.nl> (original author)
//   Tom Peeters <tom@thefrontdoor.nl>
//   Pieter Janssen <pieter.janssen@thefrontdoor.nl>
//
// - - - - - - - - - - - - - - END LICENSE BLOCK - - - - - - - - - - - - -

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
        'contextmenu', 'dblclick', 'sfdragstart', 'sfdragover', 'sfdragdrop', 'dragdrop', 'dragend', 
        'dragenter', 'dragexit', 'draggesture','dragover', 'focus', 'input', 'keydown', 'keypress', 
        'keyup', 'load', 'mousedown', 'mousemove', 'mouseout', 'mouseover', 'mouseup', 'overflow', 
        'overflowchanged', 'popuphidden', 'popuphiding', 'popupshowing', 'popupshown', 'select',
        'syncfrompreference', 'synctopreference', 'underflow', 'unload', 'scroll','wheel'
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

        if (args.toJSON) {
            args = args.toJSON();
        } else {
            args = JSON.stringify(args);
        }

        this.Queue.push( args );
    },
    
    QueueFlush: function( blocking ) {
        if ( ! this.Queue.length ) {
            return;
        }
        
        var queue = this.Queue;
        this.Queue = [];

        queue = '[' + queue.join(',') + ']';
        
        var tr = new SiteFusion.Comm.Transmission( blocking ? true : false, queue );
        return tr;
    },
    
    StartProgressTimer: function() {
        if ( ! this.ProgressTimer ) {
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
        
        if ( 'element' in node ) {
            node.element.setAttribute( 'id', 'cid' + id );
        }

        return node;
    }
};

SiteFusion.Comm.Transmission = function() {
    this.STATE_NEW = 0;
    this.STATE_PROCESSING = 1;
    this.STATE_CONNECTED = 2;
    this.STATE_FINISHED = 3;
    
    this.state = null;
    this.blocking = null;
    this.payload = null;
    this.payloadEncoding = null;
    this.reverseInitiative = false;
    this.request = null;
    this.onstatechange = null;

    this.initialize.apply(this, arguments);
};
    
SiteFusion.Comm.Transmission.prototype.initialize = function( blocking, payload ) {
    if ( typeof(payload) == 'undefined' ) {
        this.reverseInitiative = true;
    } else {
        this.payload = payload;
    }
    
    this.blocking = (blocking && !this.reverseInitiative) ? true : false;
    
    this.state = this.STATE_NEW;
    if ( typeof(this.onstatechange) == 'function' ) {
        this.onstatechange();
    }
    
    if ( this.reverseInitiative ) {
        SiteFusion.Comm.RevCommTransmission = this;
        this.send();
    } else if( this.blocking || !SiteFusion.Comm.CommTransmission ) {
        SiteFusion.Comm.CommTransmission = this;
        this.send();
    } else {
        SiteFusion.Comm.TransmissionQueue.push( this );
    }
};

SiteFusion.Comm.Transmission.prototype.send = function() {
    
    // RevComm connections can proceed right away
    if ( this.reverseInitiative ) {
        this.openHttpRequest();
        return;
    }
    
    if ( this.blocking ) {
        for( var n = 0; n < SiteFusion.Comm.BusyHandlers.length; n++ ) {
            SiteFusion.Comm.BusyHandlers[n]();
        }
    } else {
        SiteFusion.Comm.StartProgressTimer();
    }
    
    // Compress payload if larger than 128 bytes and asynchronous
    if ( this.payload.length > 128 && !this.blocking ) {
        this.payloadEncoding = 'application/x-gzip';
        new SiteFusion.Comm.Transmission.Compressor( this );
        this.state = this.STATE_PROCESSING;
        if ( typeof(this.onstatechange) == 'function' ) {
            this.onstatechange();
        }
    } else {
        this.payloadEncoding = 'application/octet-stream';
        this.openHttpRequest();
    }
};

SiteFusion.Comm.Transmission.prototype.openHttpRequest = function() {
    this.request = new XMLHttpRequest();

    var aSync = (this.reverseInitiative || !this.blocking);

    this.state = this.STATE_CONNECTED;
    if ( typeof(this.onstatechange) == 'function' ) {
        this.onstatechange();
    }
    
    try {
        var url = SiteFusion.Address + '/' + (this.reverseInitiative ? 'revcomm':'comm') + '.php'
            + '?app=' + SiteFusion.Application
            + '&args=' + SiteFusion.Arguments
            + '&sid=' + SiteFusion.SID
            + '&ident=' + SiteFusion.Ident
            + '&clientid=' + SiteFusion.ClientID

        this.request.open( 'POST',
            url,
            aSync
        );
        
        if ( this.payloadEncoding ) {
            this.request.setRequestHeader( 'Content-Type', this.payloadEncoding );
        }
    
        if ( aSync ) {
            var transmission = this;
            this.request.onreadystatechange = function( event ) {
                if( this.readyState == 4 ) {
                    transmission.handleResponse(url);
                }
            };
        }

        this.request.send( this.payload );
    } catch ( e ) {
        var oThis = this;
        SiteFusion.consoleError(e);
        setTimeout( function() { oThis.openHttpRequest(); }, 1000 );
        return;
    }
    
    if ( !aSync ) {
        this.handleResponse();
    }
};

SiteFusion.Comm.Transmission.prototype.handleResponse = function(url) {
    if ( this.request.status != 200 ) {

        SiteFusion.consoleError("request to " + url + " status: " + this.request.status + " reponseText: " + this.request.responseText);

        if( !this.reverseInitiative ) {
            var oThis = this;
            setTimeout( function() { oThis.openHttpRequest(); }, 1000 );
        }
        return;
    }
    
    var contentType = this.request.getResponseHeader( 'Content-type' );
    
    if ( contentType.match( /sitefusion\/error/ ) ) {
        try {
            var error = eval( '(' + this.request.responseText + ')' );
            SiteFusion.HandleError( error );
        } catch ( ex ) {
            SiteFusion.HandleError( { 'type': null, 'message': this.request.responseText.replace( /<.+?>/g, '' ) } );
        }
        return;
    }
    
    SiteFusion.Execute( this.request.responseText );
    SiteFusion.Interface.HandleDeferredChildAdditions();
    
    if ( this.blocking ) {
        for( var n = 0; n < SiteFusion.Comm.IdleHandlers.length; n++ ) {
            SiteFusion.Comm.IdleHandlers[n]();
        }
    }
    
    if ( this.reverseInitiative ) {
        this.send();
    } else if( SiteFusion.Comm.TransmissionQueue.length ) {
        SiteFusion.Comm.CommTransmission = SiteFusion.Comm.TransmissionQueue.shift();
        SiteFusion.Comm.CommTransmission.send();
    } else {
        SiteFusion.Comm.CommTransmission = null;
        SiteFusion.Comm.StopProgressTimer();
    }
    
    this.state = this.STATE_FINISHED;
    if ( typeof(this.onstatechange) == 'function' ) {
        this.onstatechange();
    }
};

SiteFusion.Comm.Transmission.prototype.abort = function() {
    if ( this.state >= this.STATE_CONNECTED ) {
        this.request.abort();
        SiteFusion.Comm.StopProgressTimer();
    }
};

SiteFusion.Comm.Transmission.Compressor = function() {
    this.buffer = '';
    this.transmission = null;

    this.initialize.apply(this, arguments);
};
    
SiteFusion.Comm.Transmission.Compressor.prototype.initialize = function( transmission ) {
    this.transmission = transmission;
    
    var utfConv = Cc["@mozilla.org/intl/scriptableunicodeconverter"].createInstance(Ci.nsIScriptableUnicodeConverter);
    utfConv.charset = "UTF-8";
    
    var utfStream = utfConv.convertToInputStream( this.transmission.payload );
    
    var scs = Cc["@mozilla.org/streamConverters;1"].getService( Ci.nsIStreamConverterService );
    var compressor = scs.asyncConvertData( "uncompressed", "deflate", this, null );
    
    var pump = Cc["@mozilla.org/network/input-stream-pump;1"].createInstance( Ci.nsIInputStreamPump );
    pump.init( utfStream, -1, -1, 0, 0, true );
    pump.asyncRead( compressor, null );
};

SiteFusion.Comm.Transmission.Compressor.prototype.onStartRequest = function( request, context ) {};

SiteFusion.Comm.Transmission.Compressor.prototype.onStopRequest = function( request, context, statuscode ) {
    var sData = this.buffer;
    var nBytes = sData.length;
    var ui8Data = new Uint8Array(nBytes);
    for (var nIdx = 0; nIdx < nBytes; nIdx++) {
        ui8Data[nIdx] = sData.charCodeAt(nIdx) & 0xff;
    }

    this.transmission.payload = ui8Data;
    this.transmission.openHttpRequest();
};

SiteFusion.Comm.Transmission.Compressor.prototype.onDataAvailable = function( request, context, inputStream, offset, count ) {
    var stream = Cc["@mozilla.org/binaryinputstream;1"].createInstance( Ci.nsIBinaryInputStream );
    stream.setInputStream( inputStream );
    this.buffer += stream.readBytes( count );
};
