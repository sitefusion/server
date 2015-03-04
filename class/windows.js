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

SiteFusion.Classes.BasicWindow = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.lastWindowState = STATE_NORMAL;
};
SiteFusion.Classes.BasicWindow.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.BasicWindow.prototype.constructor = SiteFusion.Classes.BasicWindow;

    SiteFusion.Classes.BasicWindow.prototype.maximize = function() {
        var oThis = this;
        this.windowObject.setTimeout( function() { oThis.windowObject.maximize(); }, 100 );
    };

    SiteFusion.Classes.BasicWindow.prototype.minimize = function() {
        var oThis = this;
        this.windowObject.setTimeout( function() { oThis.windowObject.minimize(); }, 100 );
    };

    SiteFusion.Classes.BasicWindow.prototype.fullScreen = function( state ) {
        var oThis = this;
        this.windowObject.setTimeout( function() { oThis.windowObject.fullScreen = state; }, 100 );
    };

    SiteFusion.Classes.BasicWindow.prototype.center = function() {
        var oThis = this;
        this.windowObject.setTimeout( function() {
            oThis.windowObject
                .QueryInterface(Ci.nsIInterfaceRequestor)
                .getInterface(Ci.nsIWebNavigation)
                .QueryInterface(Ci.nsIDocShellTreeItem)
                .treeOwner
                .QueryInterface(Ci.nsIInterfaceRequestor)
                .getInterface(Ci.nsIXULWindow).center(null,true,true);
        }, 10 );
    };

    SiteFusion.Classes.BasicWindow.prototype.sizeToContent = function() {
        var oThis = this;
        SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.windowObject.sizeToContent(); } );
    };

    SiteFusion.Classes.BasicWindow.prototype.addStyleSheetRule = function( ruleText ) {
        var sheet = this.windowObject.document.styleSheets[0];
        sheet.insertRule( ruleText, sheet.cssRules.length );
    };

    SiteFusion.Classes.BasicWindow.prototype.openLink = function( url ) {
        var ioservice = Components.classes["@mozilla.org/network/io-service;1"].getService(Components.interfaces.nsIIOService);
        var uriToOpen = ioservice.newURI(url, null, null);
        var extps = Components.classes["@mozilla.org/uriloader/external-protocol-service;1"].getService(Components.interfaces.nsIExternalProtocolService);
        extps.loadURI(uriToOpen, null);
    };

    SiteFusion.Classes.BasicWindow.prototype.createElement = function( tagName ) {
        return this.windowObject.document.createElement( tagName );
    };

    SiteFusion.Classes.BasicWindow.prototype.createElementNS = function( nsName, tagName ) {
        return this.windowObject.document.createElementNS( nsName, tagName );
    };

    SiteFusion.Classes.BasicWindow.prototype.createTextNode = function( text ) {
        return this.windowObject.document.createTextNode( text );
    };

    SiteFusion.Classes.BasicWindow.prototype.alert = function( text ) {
        //apply the check for the window being hidden.
        var targetWindow = this.windowObject;

        winUtils = this.windowObject.QueryInterface(Ci.nsIInterfaceRequestor).getInterface(Ci.nsIDOMWindowUtils);
        if (winUtils && !winUtils.isParentWindowMainWidgetVisible) {
            targetWindow = null;
        }
        PromptService.alert(targetWindow,"",text+'');
    };

    SiteFusion.Classes.BasicWindow.prototype.setTitle = function( title ) {
        this.windowObject.document.title = title;
    };

    SiteFusion.Classes.BasicWindow.prototype.initMenuBar = function() {
        //if we find the old-style systemMenuBar, delete it.
        var menubar = this.windowObject.document.getElementById('systemMenuBar');
        this.systemMenuBar = null;

        //if there's no systemMenuBar found, return. We don't have to do anything since it's a recent client
        if (!menubar) return;

        if( navigator.platform.match(/mac/i)) {
            this.systemMenuBar = menubar;
            var quitItem = this.windowObject.document.getElementById('menu_FileQuitItem');
            if (quitItem) {
                quitItem.setAttribute( 'oncommand', 'sfRootWindow.onClose(event);' );
            }
        }
        else {
            menubar.parentNode.removeChild( menubar );
        }
    };

    SiteFusion.Classes.BasicWindow.prototype.openUrlWindow = function(url, options) {
        var win = this.windowObject.open(url,'', options);
        SiteFusion.Interface.RegisterChildWindow(win);
        win.onclose = function() {SiteFusion.Interface.UnregisterChildWindow(win); };
    };

    SiteFusion.Classes.BasicWindow.prototype.openUrlDialog = function(url, options) {
        var win = this.windowObject.openDialog(url,'', options);
        SiteFusion.Interface.RegisterChildWindow(win);
        win.onclose = function() {SiteFusion.Interface.UnregisterChildWindow(win); };
        setTimeout(function(){SiteFusion.Interface.CursorIdle();},50);
    };

    SiteFusion.Classes.BasicWindow.prototype.addBasicEvents = function() {

        var oThis = this;

        var onSizeModeChanged = function(event) { oThis.onSizeModeChanged(event); };
        this.windowObject.addEventListener( 'sizemodechange', onSizeModeChanged, true );
        this.eventHost.sizemodechange.msgType = SiteFusion.Comm.MSG_SEND;

        var onActivate = function(event) { oThis.onActivate(event); };
        this.windowObject.addEventListener( 'activate', onActivate, true );

        var onDeactivate = function(event) { oThis.onDeactivate(event); };
        this.windowObject.addEventListener( 'deactivate', onDeactivate, true );

    };

    SiteFusion.Classes.BasicWindow.prototype.onSizeModeChanged = function(event) {
        if (this.lastWindowState != this.windowObject.windowState) { // See mozilla bug https://bugzilla.mozilla.org/show_bug.cgi?id=715867
            this.fireEvent( 'sizemodechange', [this.windowObject.windowState] );
            this.lastWindowState = this.windowObject.windowState;
        }
    };

    SiteFusion.Classes.BasicWindow.prototype.onActivate = function(event) {
        this.fireEvent( 'activate' );
    };

    SiteFusion.Classes.BasicWindow.prototype.onDeactivate = function(event) {
        this.fireEvent( 'deactivate' );
    };


SiteFusion.Classes.Window = function() {
    SiteFusion.Classes.BasicWindow.apply(this, arguments);

    this.sfClassName = 'XULWindow';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Window.prototype = Object.create(SiteFusion.Classes.BasicWindow.prototype);
SiteFusion.Classes.Window.prototype.constructor = SiteFusion.Classes.Window;

    SiteFusion.Classes.Window.prototype.initialize = function( winobj ) {
        this.isPainted = true;

        this.windowObject = winobj;
        this.element = winobj.document.documentElement;

        this.element.sfNode = this;
        winobj.sfNode = this;
        winobj.sfRootWindow = this;

        this.setEventHost( [ 'initialized', 'close', 'sizemodechange', 'activate', 'deactivate' ] );

        this.eventHost.initialized.msgType = SiteFusion.Comm.MSG_SEND;
        this.eventHost.initialized.blocking = true;
        this.eventHost.close.msgType = SiteFusion.Comm.MSG_SEND;
        this.eventHost.close.blocking = true;

        var oThis = this;
        var onClose = function(event) { oThis.onClose(event); };
        this.windowObject.addEventListener( 'close', onClose, true );

        var onResize = function(event) { oThis.onResize(event); };
        this.windowObject.addEventListener( 'resize', onResize, true );

        this.addBasicEvents();

        SiteFusion.Comm.AddToRegistry( 0, this );

        SiteFusion.Comm.BusyHandlers.push( SiteFusion.Interface.CursorBusy );
        SiteFusion.Comm.IdleHandlers.push( SiteFusion.Interface.CursorIdle );

        this.element.setAttribute( 'id', 'sitefusion-window' );

        this.initMenuBar();

        var obsService = Cc["@mozilla.org/observer-service;1"].getService(Ci.nsIObserverService);

        var wakeObserver = {
            observe: function( subject, topic, data ) {

                if (topic == 'wake_notification') {
                    //suppress all (session) errors after this message
                    if (typeof(SiteFusion.wakeTitle) != 'undefined') {
                        SiteFusion.WakeOccurred = true;
                    }
                    var promptService = Cc["@mozilla.org/embedcomp/prompt-service;1"].getService(Ci.nsIPromptService);
                    var wakeTitle = 'SiteFusion connection interrupted';
                    var wakeMessage = 'This application has lost connection to the SiteFusion server because of a system sleep. The application will now shut down.';
                    if (typeof(SiteFusion.wakeTitle) != 'undefined' && typeof(SiteFusion.wakeMessage) != 'undefined') {
                        wakeTitle = SiteFusion.wakeTitle;
                        wakeMessage = SiteFusion.wakeMessage;
                    }
                    promptService.alert(oThis.windowObject, wakeTitle, wakeMessage);
                    System.Shutdown();
                    return;
                }
                if( !oThis.onClose() )
                    subject.data = true;
            }
        };

        obsService.addObserver( wakeObserver, 'wake_notification', false );
        //Addition for xulrunner 2.0 addonmanager restart cycle
        obsService.addObserver( {observe: function( subject, topic, data){ oThis.onClose(); }}, 'quit-application-requested', false );

        SiteFusion.Comm.RevComm();
    };

    SiteFusion.Classes.Window.prototype.onResize = function( event ) {
        this.fireEvent( 'resize' );
    };

    SiteFusion.Classes.Window.prototype.onClose = function( event ) {
        this.preventClose = false;

        this.fireEvent( 'close' );

        if( this.preventClose ) {
            if( event ) {
                event.preventDefault();
                event.stopPropagation();
            }
            return false;
        }

        if( SiteFusion.Comm.RevCommTransmission ) {
            try {
                SiteFusion.Comm.RevCommTransmission.abort();
            }
            catch ( ex ) {}
        }

        this.close();

        return true;
    };

    SiteFusion.Classes.Window.prototype.close = function() {
        for( var n = 0; n < SiteFusion.Interface.ChildWindows.length; n++ ) {
            if( SiteFusion.Interface.ChildWindows[n] )
                SiteFusion.Interface.ChildWindows[n].close();
        }

        this.windowObject.close();
    };


SiteFusion.Classes.ChildWindow = function() {
    SiteFusion.Classes.BasicWindow.apply(this, arguments);

    this.sfClassName = 'XULChildWindow';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.ChildWindow.prototype = Object.create(SiteFusion.Classes.BasicWindow.prototype);
SiteFusion.Classes.ChildWindow.prototype.constructor = SiteFusion.Classes.ChildWindow;

    SiteFusion.Classes.ChildWindow.prototype.initialize = function( win ) {
        this.isPainted = true;
        this.hostWindow = win;

        this.alwaysLowered = false;
        this.alwaysRaised = false;
        this.modal = false;
        this.centerscreen = true;
        this.dependent = false;
        this.dialog = true;
        this.resizable = false;

        this.isClosing = false;

        this.setEventHost( [ 'initialized', 'close', 'hasClosed', 'sizemodechange', 'activate', 'deactivate' ] );

        this.eventHost.initialized.msgType = 0;
        this.eventHost.initialized.blocking = true;
        this.eventHost.close.msgType = 0;
        this.eventHost.close.blocking = true;
        this.eventHost.hasClosed.msgType = 0;
    };

    SiteFusion.Classes.ChildWindow.prototype.open = function() {
        var feat = 'chrome';

        if( this.centerscreen )
            feat += ',centerscreen';
        if( this.alwaysLowered )
            feat += ',alwaysLowered';
        if( this.alwaysRaised )
            feat += ',alwaysRaised';
        if( this.modal )
            feat += ',modal';
        if( this.dependent )
            feat += ',dependent';
        if( this.dialog )
            feat += ',dialog';
        if( this.resizable )
            feat += ',resizable';

        this.features = feat;

        this.openWindow();
    };

    SiteFusion.Classes.ChildWindow.prototype.openWindow = function() {
        this.parentHostWindow.windowObject.open( "chrome://sitefusion/content/childwindow.xul?" + this.cid, "", this.features );
    };

    SiteFusion.Classes.ChildWindow.prototype.initWindow = function( win ) {
        SiteFusion.Interface.RegisterChildWindow( win );
        this.windowObject = win;
        win.sfNode = this;
        this.element = win.document.getElementById( 'sitefusion-dialog' );
        this.element.sfNode = this;
        win.sfRootWindow = SiteFusion.RootWindow;

        this.attachEvents();

        this.initMenuBar();
        this.fireEvent( 'initialized' );
    };

    SiteFusion.Classes.ChildWindow.prototype.attachEvents = function() {
        var oThis = this;
        var onClose = function(event) { oThis.onClose(event); };
        this.windowObject.addEventListener( 'close', onClose, true );

        var onResize = function(event) { oThis.onResize(event); };
        this.windowObject.addEventListener( 'resize', onResize, true );

        this.addBasicEvents();
    };

    SiteFusion.Classes.ChildWindow.prototype.close = function() {
        this.windowObject.close();
    };

    SiteFusion.Classes.ChildWindow.prototype.onResize = function( event ) {
        this.fireEvent( 'resize' );
    };

    SiteFusion.Classes.ChildWindow.prototype.onClose = function( event ) {
        this.preventClose = false;

        this.fireEvent( 'close' );

        if( this.preventClose ) {
            event.preventDefault();
            event.stopPropagation();
            return false;
        }

        this.isClosing = true;
        this.fireEvent( 'hasClosed' );

        SiteFusion.Interface.UnregisterChildWindow( this.windowObject );

        return true;
    };



SiteFusion.Classes.Dialog = function() {
    SiteFusion.Classes.ChildWindow.apply(this, arguments);

    this.sfClassName = 'XULDialog';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Dialog.prototype = Object.create(SiteFusion.Classes.ChildWindow.prototype);
SiteFusion.Classes.Dialog.prototype.constructor = SiteFusion.Classes.Dialog;

    SiteFusion.Classes.Dialog.prototype.initialize = function( win ) {
        this.isPainted = true;
        this.hostWindow = win;

        this.alwaysLowered = false;
        this.alwaysRaised = false;
        this.modal = false;
        this.centerscreen = true;
        this.dependent = false;
        this.dialog = true;
        this.resizable = false;

        this.isClosing = false;

        this.setEventHost( [ 'initialized', 'accept', 'cancel', 'close', 'help', 'disclosure', 'extra1', 'extra2', 'hasClosed', 'sizemodechange', 'activate', 'deactivate' ] );

        this.eventHost.initialized.msgType = 0;
        this.eventHost.initialized.blocking = true;
        this.eventHost.close.msgType = -1;
        this.eventHost.accept.msgType = 0;
        this.eventHost.accept.blocking = true;
        this.eventHost.cancel.msgType = -1;
        this.eventHost.help.msgType = 0;
        this.eventHost.disclosure.msgType = 0;
        this.eventHost.extra1.msgType = 0;
        this.eventHost.extra2.msgType = 0;
        this.eventHost.close.msgType = 0;
        this.eventHost.close.blocking = true;
        this.eventHost.hasClosed.msgType = 0;
    };

    SiteFusion.Classes.Dialog.prototype.openWindow = function() {
        this.parentHostWindow.windowObject.openDialog( "chrome://sitefusion/content/dialog.xul?" + this.cid, "", this.features );
    };

    SiteFusion.Classes.Dialog.prototype.attachEvents = function() {
        var oThis = this;
        var onClose = function(event) { oThis.onClose(event); };
        this.windowObject.addEventListener( 'close', onClose, true );

        var onDialogButton = function(event) { oThis.onDialogButton(event); };
        this.windowObject.addEventListener( 'dialogaccept', onDialogButton, true );
        this.windowObject.addEventListener( 'dialogcancel', onDialogButton, true );
        this.windowObject.addEventListener( 'dialoghelp', onDialogButton, true );
        this.windowObject.addEventListener( 'dialogdisclosure', onDialogButton, true );
        this.windowObject.addEventListener( 'dialogextra1', onDialogButton, true );
        this.windowObject.addEventListener( 'dialogextra2', onDialogButton, true );

        this.addBasicEvents();
    };

    SiteFusion.Classes.Dialog.prototype.onDialogButton = function( event ) {
        var ename = event.type.substr(6);

        if( ename == 'help' || ename == 'disclosure' )
            this.preventClose = true;
        else
            this.preventClose = false;

        this.fireEvent( ename );

        if( this.preventClose ) {
            event.preventDefault();
            event.stopPropagation();
            return false;
        }

        this.fireEvent( 'hasClosed' );

        this.windowObject.close();
    };


SiteFusion.Classes.PrefWindow = function() {
    SiteFusion.Classes.Dialog.apply(this, arguments);

    this.sfClassName = 'XULPrefWindow';
};
SiteFusion.Classes.PrefWindow.prototype = Object.create(SiteFusion.Classes.Dialog.prototype);
SiteFusion.Classes.PrefWindow.prototype.constructor = SiteFusion.Classes.PrefWindow;

    SiteFusion.Classes.PrefWindow.prototype.openWindow = function() {
        this.parentHostWindow.windowObject.openDialog( "chrome://sitefusion/content/prefwindow.xul?" + this.cid, "", this.features );
    };

    SiteFusion.Classes.PrefWindow.prototype.showPane = function( pane ) {
        this.element.showPane( pane.element );
    };


SiteFusion.Classes.PrefPane = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULPrefPane';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.PrefPane.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.PrefPane.prototype.constructor = SiteFusion.Classes.PrefPane;

    SiteFusion.Classes.PrefPane.prototype.initialize = function( win ) {
        this.element = win.createElement( 'prefpane' );
        this.element.sfNode = this;

        this.setEventHost();
    };


SiteFusion.Classes.PromptService = function() {
    SiteFusion.Classes.Node.apply(this, arguments);
    this.sfClassName = 'PromptService';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.PromptService.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.PromptService.prototype.constructor = SiteFusion.Classes.PromptService;

    SiteFusion.Classes.PromptService.prototype.initialize = function( win ) {
        this.hostWindow = win;
        this.element = win.createElement( 'box' );
        this.element.hidden = true;

        this.promptService = Components.classes["@mozilla.org/embedcomp/prompt-service;1"]
                            .getService(Components.interfaces.nsIPromptService);

        this.setEventHost();
    };

    SiteFusion.Classes.PromptService.prototype.getTargetWindow = function() {
        //apply the check for the window being hidden.
        var targetWindow = this.hostWindow.windowObject;

        if (targetWindow) {
            winUtils = targetWindow.QueryInterface(Ci.nsIInterfaceRequestor).getInterface(Ci.nsIDOMWindowUtils);

            if (winUtils && !winUtils.isParentWindowMainWidgetVisible) {
                targetWindow = null;
            }
        }
        else {
            targetWindow = null;
        }
        return targetWindow;
    };

    SiteFusion.Classes.PromptService.prototype.alert = function( title, text ) {
        this.promptService.alert( this.getTargetWindow(), title+'', text+'' );
    };

    SiteFusion.Classes.PromptService.prototype.alertCheck = function( title, text, checkMsg, checkState ) {
        checkState = { value: checkState };
        this.promptService.alertCheck( this.getTargetWindow(), title+'', text+'', checkMsg+'', checkState );
        this.fireEvent( 'yield', [ true, checkState.value, null, null, null ] );
    };

    SiteFusion.Classes.PromptService.prototype.confirm = function( title, text ) {
        var result = this.promptService.confirm( this.getTargetWindow(), title+'', text+'' );
        this.fireEvent( 'yield', [ result, null, null, null, null ] );
    };

    SiteFusion.Classes.PromptService.prototype.confirmCheck = function( title, text, checkMsg, checkState ) {
        checkState = { value: checkState };
        var result = this.promptService.confirmCheck( this.getTargetWindow(), title+'', text+'', checkMsg+'', checkState );
        this.fireEvent( 'yield', [ result, checkState.value, null, null, null ] );
    };

    SiteFusion.Classes.PromptService.prototype.confirmEx = function( title, text, buttonFlags, button0Title, button1Title, button2Title, checkMsg, checkState ) {
        checkState = { value: checkState };
        var result = this.promptService.confirmEx( this.getTargetWindow(), title+'', text+'', buttonFlags, button0Title, button1Title, button2Title, checkMsg, checkState );
        this.fireEvent( 'yield', [ result, checkState.value, null, null, null ] );
    };

    SiteFusion.Classes.PromptService.prototype.prompt = function( title, text, textValue, checkMsg, checkState ) {
        if( checkMsg !== null ) checkMsg = checkMsg+'';
        checkState = { value: checkState };
        textValue = { value: textValue+'' };
        var result = this.promptService.prompt( this.getTargetWindow(), title+'', text+'', textValue, checkMsg, checkState );
        this.fireEvent( 'yield', [ result, checkState.value, textValue.value, null, null ] );
    };

    SiteFusion.Classes.PromptService.prototype.promptUsernameAndPassword = function( title, text, username, password, checkMsg, checkState ) {
        if( checkMsg !== null ) checkMsg = checkMsg+'';
        checkState = { value: checkState };
        username = { value: username+'' };
        password = { value: password+'' };
        var result = this.promptService.promptUsernameAndPassword( this.getTargetWindow(), title+'', text+'', username, password, checkMsg, checkState );
        this.fireEvent( 'yield', [ result, checkState.value, null, username.value, password.value ] );
    };

    SiteFusion.Classes.PromptService.prototype.promptPassword = function( title, text, password, checkMsg, checkState ) {
        if( checkMsg !== null ) checkMsg = checkMsg+'';
        checkState = { value: checkState };
        password = { value: password+'' };
        var result = this.promptService.promptPassword( this.hostWindow.windowObject, title+'', text+'', password, checkMsg, checkState );
        this.fireEvent( 'yield', [ result, checkState.value, null, null, password.value ] );
    };

    SiteFusion.Classes.PromptService.prototype.select = function( title, text, list, index ) {
        index = { value: index };
        for( var n = 0; n < list.length; n++ ) {
            list[n] = list[n]+'';
        }
        var result = this.promptService.select( this.hostWindow.windowObject, title+'', text+'', list.length, list, index );
        this.fireEvent( 'yield', [ result, null, index.value, null, null ] );
    };


SiteFusion.Classes.AlertNotification = function() {
    SiteFusion.Classes.Node.apply(this, arguments);
    this.sfClassName = 'AlertNotification';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.AlertNotification.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.AlertNotification.prototype.constructor = SiteFusion.Classes.AlertNotification;

    SiteFusion.Classes.AlertNotification.prototype.initialize = function( win ) {
        this.hostWindow = win;
        this.element = win.createElement( 'box' );
        this.element.hidden = true;

        this.setEventHost( [ 'finished' ] );
    };

    SiteFusion.Classes.AlertNotification.prototype.showAlertNotification = function( imageUrl, title, text, name, textClickable, alertOrigin ) {

        name = (name == null ? 'SiteFusion' : name);
        imageUrl = (imageUrl != '' ? this.parseImageURL(imageUrl) : '');

        var observer = {
            observe: function( subject, topic, data ) {
                switch ( topic ) {
                    case 'alertshow':
                        data.fireEvent( 'focus' );
                    break;
                    case 'alertclickcallback':
                        data.fireEvent( 'command' );
                    break;
                    case 'alertfinished':
                        data.fireEvent( 'finished' );
                    break;
                }
            }
        };

        var win = Components.classes['@mozilla.org/embedcomp/window-watcher;1'].getService(Components.interfaces.nsIWindowWatcher).openWindow(null, 'chrome://global/content/alerts/alert.xul', '_blank', 'chrome,titlebar=no,popup=yes', null);
        win.arguments = [imageUrl, title, text, textClickable, this, alertOrigin, null, null, null, observer];

    };

