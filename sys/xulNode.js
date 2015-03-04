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

SiteFusion.Classes.Node = function() {
    this.preventDeferredInsertion = false;
    this.observer = null;
    this.isRemoved = false;
};

    SiteFusion.Classes.Node.prototype.addObserver = function(topic) {
        if (!this.observer) {
            this.observer = new this.Observer( this );
            this.createEvent('observe', 0);
        }
        this.observer.register(topic);
    };

    SiteFusion.Classes.Node.prototype.removeObserver = function(topic) {
        if (!this.observer) {
            return;
        }
        this.observer.unregister(topic);
    };

    SiteFusion.Classes.Node.prototype.setEventHost = function( events, exclude ) {
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
            if( events.indexOf('yield') == -1 ) {
                events.push( 'yield' );
            }
        } else {
            events = [ 'yield' ];
        }

        for( var n = 0; n < events.length; n++ ) {
            this.createEvent( events[n] );
        }
    };

    SiteFusion.Classes.Node.prototype.createEvent = function( evt, type ) {
        if( evt in this.eventHost )
            SiteFusion.Error( 'Event ' + evt + ' is already defined' );

        this.eventHost[evt] = [];
        this.eventHost[evt].msgType = type;
        this.eventHost[evt].reflex = null;
        this.eventHost[evt].blocking = false;
    };

    SiteFusion.Classes.Node.prototype.setEventListener = function( evt ) {
        var xulEvt = (SiteFusion.Comm.XULEvents.indexOf(evt) != -1);
        
        if( ! xulEvt ) return;
        if( this.eventHost[evt].noAutoBind ) return;

        if( this.eventHost[evt].msgType == -1 && this.eventHost[evt].reflex === null && this.eventHost[evt].length === 0 )
            this.element.removeEventListener( evt, this.eventHandler, true );
        else
            this.element.addEventListener( evt, this.eventHandler, true );
    };

    SiteFusion.Classes.Node.prototype.setEventType = function( evt, type ) {
        if( this.eventHost[evt] === null )
            SiteFusion.Error( 'Widget ' + this.sfClassName + ' does not support event ' + evt );
        
        this.eventHost[evt].msgType = type;

        var oThis = this;
        SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.setEventListener( evt ); } );
    };

    SiteFusion.Classes.Node.prototype.setEventBlocking = function( evt, blocking ) {
        if( this.eventHost[evt] === null )
            SiteFusion.Error( 'Widget ' + this.sfClassName + ' does not support event ' + evt );
        
        this.eventHost[evt].blocking = blocking;
    };

    SiteFusion.Classes.Node.prototype.eventHandler = function ( event ) {
        var obj = event.currentTarget;

        if( ! obj.sfNode ) return;
        
        var tr = obj.sfNode.fireEvent( event );
        return tr;
    };

    SiteFusion.Classes.Node.prototype.fireEvent = function( e, args ) {
        var event = typeof(e) == 'string' ? null:e;
        var eventName = typeof(e) == 'string' ? e:e.type;
        
        if( this.eventHost[eventName] === null )
            SiteFusion.Error( 'Widget ' + this.sfClassName + ' does not support event ' + e );

        var sfEvent = this.eventHost[eventName];
        
        if( ! args )
            args = [];
        
        if( sfEvent.reflex !== null ) {
            this._dummy = sfEvent.reflex;
            var ret = this._dummy( eventName, args, event );
            delete this._dummy;
            if( ret === false ) return false;
        }

        for( var n = 0; n < sfEvent.length; n++ ) {
            if( sfEvent[n].yield )
                sfEvent[n].yield();
        }
        
        if( sfEvent.msgType == 0 ) {

            return SiteFusion.Comm.SendCommand( this, eventName, args );
        }
        else if( sfEvent.msgType == 1 )
            SiteFusion.Comm.QueueCommand( this, eventName, args );

        return true;
    };

    SiteFusion.Classes.Node.prototype.addYielder = function() {
        var e = arguments[0];

        if( this.eventHost[e] === null )
            SiteFusion.Error( 'Widget ' + this.sfClassName + ' does not support event ' + e );

        for( var n = 1; n < arguments.length; n++ ) {
            this.eventHost[e].push( arguments[n] );
        }

        var oThis = this;
        SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.setEventListener( e ); } );
    };

    SiteFusion.Classes.Node.prototype.removeYielder = function() {
        var e = arguments[0];

        if( this.eventHost[e] === null )
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
    };

    SiteFusion.Classes.Node.prototype.addReflex = function( e, code ) {
        if( this.eventHost[e] === null )
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
    };

    SiteFusion.Classes.Node.prototype.removeReflex = function( e ) {
        if( this.eventHost[e] === null )
            SiteFusion.Error( 'Widget ' + this.sfClassName + ' does not support event ' + e );

        this.eventHost[e].reflex = null;

        var oThis = this;
        SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.setEventListener( e ); } );
    };

    SiteFusion.Classes.Node.prototype.addChild = function( childSFNode ) {
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
    };

    SiteFusion.Classes.Node.prototype.addChildBefore = function( childSFNode, beforeSFNode ) {
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
    };

    SiteFusion.Classes.Node.prototype.removeChild = function( childSFNode ) {
        try {
            this.element.removeChild( childSFNode.element );
            this.removeChildRecursive( childSFNode );

            for( var n = 0; n < this.sfChildren.length; n++ ) {
                if( this.sfChildren[n] == childSFNode ) {
                    this.sfChildren.splice( n, 1 );
                    break;
                }
            }
        }
        catch (e) { SiteFusion.consoleMessage( 'Exception in removeChild: '+e ); }
    };

    SiteFusion.Classes.Node.prototype.removeChildRecursive = function( childSFNode ) {
        childSFNode.isRemoved = true;

        for( var n = 0; n < childSFNode.sfChildren.length; n++ ) {
            childSFNode.removeChildRecursive( childSFNode.sfChildren[n] );
        }
    };

    SiteFusion.Classes.Node.prototype.directAddChild = function( childSFNode ) {
        try {
            this.element.appendChild( childSFNode.element );
        }
        catch (e) { SiteFusion.consoleMessage( 'Exception in directAddChild: '+e ); }
    };

    SiteFusion.Classes.Node.prototype.directAddChildBefore = function( childSFNode, beforeSFNode ) {
        try {
            if( ! beforeSFNode )
                this.directAddChild( childSFNode );
            else
                this.element.insertBefore( childSFNode.element, beforeSFNode.element );
        }
        catch (e) { SiteFusion.consoleMessage( 'Exception in directAddChildBefore: '+e ); }
    };

    SiteFusion.Classes.Node.prototype.addChildSilent = function( childSFNode ) {
        this.sfChildren.push( childSFNode );
    };

    SiteFusion.Classes.Node.prototype.removeChildSilent = function( childSFNode ) {
        for( var n = 0; n < this.sfChildren.length; n++ ) {
            if( this.sfChildren[n] == childSFNode ) {
                this.sfChildren.splice( n, 1 );
                return;
            }
        }
    };

    SiteFusion.Classes.Node.prototype.width = function( val ) {
        this.element.setAttribute( 'width', parseInt(val) );
    };

    SiteFusion.Classes.Node.prototype.height = function( val ) {
        this.element.setAttribute( 'height', parseInt(val) );
    };

    SiteFusion.Classes.Node.prototype.flex = function( val ) {
        this.element.setAttribute( 'flex', parseInt(val) );
    };

    SiteFusion.Classes.Node.prototype.label = function( text ) {
        this.element.setAttribute( 'label', text );
    };

    SiteFusion.Classes.Node.prototype.disabled = function( state ) {
        if( state )
            this.element.setAttribute( 'disabled', 'true' );
        else
            this.element.removeAttribute( 'disabled' );
    };

    SiteFusion.Classes.Node.prototype.textValue = function( text ) {
        this.element.setAttribute( 'value', '' + text );
        this.element.value = '' + text;
    };

    SiteFusion.Classes.Node.prototype.numericValue = function( num ) {
        this.element.setAttribute( 'value', num );
        this.element.value = num;
    };

    SiteFusion.Classes.Node.prototype.mode = function( mode ) {
        this.element.setAttribute( 'mode', '' + mode );
    };

    SiteFusion.Classes.Node.prototype.orient = function( text ) {
        this.element.setAttribute( 'orient', '' + text );
    };

    SiteFusion.Classes.Node.prototype.pack = function( text ) {
        this.element.setAttribute( 'pack', '' + text );
    };

    SiteFusion.Classes.Node.prototype.align = function( text ) {
        this.element.setAttribute( 'align', '' + text );
    };

    SiteFusion.Classes.Node.prototype.crop = function( text ) {
        this.element.setAttribute( 'crop', '' + text );
        this.element.crop = text;
    };

    SiteFusion.Classes.Node.prototype.src = function( text ) {
        this.element.setAttribute( 'src', this.parseImageURL(text) );
    };

    SiteFusion.Classes.Node.prototype.accessKey = function( key ) {
        this.element.setAttribute( 'accessKey', key );
    };

    SiteFusion.Classes.Node.prototype.image = function( text ) {
        this.element.setAttribute( 'image', this.parseImageURL(text) );
    };

    SiteFusion.Classes.Node.prototype.value = function( val ) {
        this.textValue( val );
    };

    SiteFusion.Classes.Node.prototype.focus = function() {
        var oThis = this;
        SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.element.focus(); } );
    };

    SiteFusion.Classes.Node.prototype.blur = function() {
        var oThis = this;
        SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.element.blur(); } );
    };

    SiteFusion.Classes.Node.prototype.parseImageURL = function( text ) {
        if( text.substr(0,1) == '/' ) {
            var d = new Date();
            text = SiteFusion.Address + '/appimage.php?name=' + text.substr(1) + '&app=' + SiteFusion.Application + '&args=' + SiteFusion.Arguments + '&sid=' + SiteFusion.SID + '&ident=' + SiteFusion.Ident /*+ '&cycle=' + d.getTime()*/;
        }
        return text;
    };

    SiteFusion.Classes.Node.prototype.backgroundImage = function ( url ) {
        this.element.style.backgroundImage = 'url(' + this.parseImageURL(url) + ')';
    };

    SiteFusion.Classes.Node.prototype.setDraggable = function() {
        var clsName = arguments[0];
        var flavours = [];

        for( var n = 1; n < arguments.length; n++ ) {
            flavours.push( arguments[n] );
        }
        
        this.draggable = true;
        this.draggableClassname = clsName;
        this.draggableFlavours = flavours;

        this.element.addEventListener('dragstart', this.onDragStartEvent.bind(this), true);
    };

    SiteFusion.Classes.Node.prototype.unsetDraggable = function() {
        this.element.removeEventListener('dragstart', this.onDragStartEvent.bind(this), true);

        this.draggable = false;
        this.draggableClassname = '';
        this.draggableFlavours = [];
    };

    SiteFusion.Classes.Node.prototype.setDroppable = function() {
        this.dropObserver = new this.DropObserver( this, arguments );
        this.droppable = true;
    };

    SiteFusion.Classes.Node.prototype.onDragStartEvent = function( event ) {
        event.dataTransfer.setData('sfNode/' + this.draggableClassname, this.cid);
        
        for (var n = 0; n < this.draggableFlavours.length; n += 2) {
            event.dataTransfer.setData(this.draggableFlavours[n], this.draggableFlavours[n + 1]);
        }
        
        this.fireEvent( 'sfdragstart' );
    };

    SiteFusion.Classes.Node.prototype.toJSON = function() {
        return JSON.stringify({ '__sfNode': this.cid });
    };

SiteFusion.Classes.Node.prototype.Observer = function() {
    this.topicIds = [];

    this.initialize.apply(this, arguments);
};

    SiteFusion.Classes.Node.prototype.Observer.prototype.initialize = function( sfNode ) {
        this.sfNode = sfNode;
        this.topicIds = [];
        this.observerService = Components.classes["@mozilla.org/observer-service;1"].getService(Components.interfaces.nsIObserverService);
        //window.setTimeout( function() { alert('observer initialize'); }, 100);
    };

    SiteFusion.Classes.Node.prototype.Observer.prototype.observe = function(aSubject, aTopic, aData) {
        var data = '';
        var subject = '';
        if (aSubject) { 
            try {
                aSubject.QueryInterface(Ci.nsISupportsString);
                if (aSubject && aSubject.data) {
                    subject = aSubject.data + '';
                }
            }
            catch (e) {}
        }
        if (aData) {
            data = aData + '';
        }
        topic = aTopic + '';
        SiteFusion.consoleMessage(sfNode);
        //note: we reverse the order since topic is our primary identifier
            this.sfNode.fireEvent( 'observe', [ topic, subject, data] );
    };

    SiteFusion.Classes.Node.prototype.Observer.prototype.register = function(topicId) {
        //we can only register 1 observer per topicId per SFNode
        this.unregister(topicId);
        
        this.topicIds.push(topicId);
        this.observerService.addObserver(this, topicId, false);
    };

    SiteFusion.Classes.Node.prototype.Observer.prototype.unregister = function(topicId) {
        var removeIndex=-1;
        for(var i in this.topicIds){
            if(this.topicIds[i]==topicId){
                removeIndex = i;
                break;
            }
        }
        if (removeIndex != -1) {
            this.observerService.removeObserver(this, topicId);
            this.topicIds.splice(removeIndex,1);
        }
    };

    SiteFusion.Classes.Node.prototype.Observer.prototype.destroy = function() {
        for(var i in this.topicIds){
            this.unregister(this.topicIds[i]);
        }
    };

SiteFusion.Classes.Node.prototype.DropObserver = function() {
    this.initialize.apply(this, arguments);
};

    SiteFusion.Classes.Node.prototype.DropObserver.prototype.initialize = function( sfNode, flavors ) {
        this.sfNode = sfNode;
        
        var oThis = this;
        this.sfNode.element.addEventListener( 'dragover', function(event) { oThis.onFileDragOver(event) || oThis.onDragOver(event); }, true );
        this.sfNode.element.addEventListener( 'drop', function(event) { oThis.onFileDrop(event) || oThis.onDrop(event); }, true );

        this.flavorNames = [];
        
        for( var n = 0; n < flavors.length; n++ ) {
            this.flavorNames.push( (flavors[n]+'').toLowerCase() );
        }
    };

    SiteFusion.Classes.Node.prototype.DropObserver.prototype.onDragOver = function( event ) {
        var types = event.dataTransfer.types;
        types = this.flavorNames.filter(function (value) { return types.contains(value); });
        if (types.length) {
            event.stopPropagation();
            event.preventDefault();
            var newEvent = {};
            newEvent.type = 'sfdragover';
            newEvent.dataTransfer = event.dataTransfer;
            newEvent.target = event.target;
            this.sfNode.fireEvent( newEvent, [ event.target.sfNode ] );
        }
    };

    SiteFusion.Classes.Node.prototype.DropObserver.prototype.onDrop = function( event ) {
        var types = event.dataTransfer.types;
        types = this.flavorNames.filter(function (value) { return types.contains(value); });
        if (types.length) {
            var d = event.dataTransfer.getData(types[0]);
            if (types[0].substr(0, 7) == 'sfnode/') {
                d = SiteFusion.Registry[d];
            }
            
            event.preventDefault();
            event.stopPropagation();
            
            this.sfNode.fireEvent( 'sfdragdrop', [ d ] );
        }
    };

    SiteFusion.Classes.Node.prototype.DropObserver.prototype.onFileDragOver = function( event ) {
        return false;
        var dragService = Cc["@mozilla.org/widget/dragservice;1"].getService(Ci.nsIDragService);
        var dragSession = dragService.getCurrentSession();
        var flavor;
        
        var supported = (dragSession.isDataFlavorSupported(flavor = "text/x-moz-url") && this.flavorNames.indexOf("text/x-moz-url") != -1);
        if (!supported)
            supported = (dragSession.isDataFlavorSupported(flavor = "application/x-moz-file") && this.flavorNames.indexOf("application/x-moz-file") != -1);

        if (supported) {
            event.preventDefault();
            event.stopPropagation();
            
            dragSession.canDrop = true;
            this.sfNode.fireEvent( 'sfdragover', [ flavor ] );
        }
        
        return supported;
    };

    SiteFusion.Classes.Node.prototype.DropObserver.prototype.onFileDrop = function( event ) {
        var dragService = Cc["@mozilla.org/widget/dragservice;1"].getService(Ci.nsIDragService);
        var dragSession = dragService.getCurrentSession();
        var _ios = Cc['@mozilla.org/network/io-service;1'].getService(Ci.nsIIOService);
        var fileProtHandler = _ios.getProtocolHandler("file").QueryInterface(Components.interfaces.nsIFileProtocolHandler);
        var uris = [];
        var fileService = new SiteFusion.Classes.FileService( window.sfNode );

        // If sourceNode is not null, then the drop was from inside the application
        if (dragSession.sourceNode)
            return false;

        // Setup a transfer item to retrieve the file data
        var trans = Cc["@mozilla.org/widget/transferable;1"].createInstance(Ci.nsITransferable);
        trans.addDataFlavor("text/x-moz-url");
        trans.addDataFlavor("application/x-moz-file");

        for (var i=0; i<dragSession.numDropItems; i++) {
            var uri = null;

            dragSession.getData(trans, i);
            var flavor = {}, data = {}, length = {};
            trans.getAnyTransferData(flavor, data, length);
            if (data) {
                var str = false;
                try {
                    str = data.value.QueryInterface(Components.interfaces.nsISupportsString);
                } catch(ex) {
                }
            
                if (str) {
                    uri = _ios.newURI(str.data.split("\n")[0], null, null).spec;
                } else {
                    var file = data.value.QueryInterface(Components.interfaces.nsIFile);
                    if (file) {
                        uri = [file.path, fileService.resultFromFile(file)];
                    }
                }
            }
            
            if (uri) {
                if ( typeof(uri) == 'string' && uri.substr(0,7) == 'file://' ) {
                    var file = fileProtHandler.getFileFromURLSpec( uri );
                    uri = [file.path, fileService.resultFromFile(file)];
                }
                
                uris.push(uri);
            }
        }
        event.preventDefault();
        event.stopPropagation();
        this.sfNode.fireEvent( 'sfdragdrop', [ uris ] );
        return true;
    };
