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


SiteFusion.Classes.BasicWindow = Class.create( SiteFusion.Classes.Node, {
	lastWindowState: STATE_NORMAL,

	maximize: function() {
		var oThis = this;
		this.windowObject.setTimeout( function() { oThis.windowObject.maximize(); }, 100 );
	},

	minimize: function() {
		var oThis = this;
		this.windowObject.setTimeout( function() { oThis.windowObject.minimize(); }, 100 );
	},
	
	fullScreen: function( state ) {
		var oThis = this;
		this.windowObject.setTimeout( function() { oThis.windowObject.fullScreen = state; }, 100 );
	},
	
	center: function() {
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
	},

	sizeToContent: function() {
		var oThis = this;
		SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.windowObject.sizeToContent(); } );
	},

	addStyleSheetRule: function( ruleText ) {
		var sheet = this.windowObject.document.styleSheets[0];
		sheet.insertRule( ruleText, sheet.cssRules.length );
	},

	openLink: function( url ) {
		var ioservice = Components.classes["@mozilla.org/network/io-service;1"].getService(Components.interfaces.nsIIOService);
		var uriToOpen = ioservice.newURI(url, null, null);
		var extps = Components.classes["@mozilla.org/uriloader/external-protocol-service;1"].getService(Components.interfaces.nsIExternalProtocolService);
		extps.loadURI(uriToOpen, null);
	},

	createElement: function( tagName ) {
		return this.windowObject.document.createElement( tagName );
	},

	createElementNS: function( nsName, tagName ) {
		return this.windowObject.document.createElementNS( nsName, tagName );
	},

	createTextNode: function( text ) {
		return this.windowObject.document.createTextNode( text );
	},

	alert: function( text ) {
        //apply the check for the window being hidden.
        var targetWindow = this.windowObject;

        winUtils = this.windowObject.QueryInterface(Ci.nsIInterfaceRequestor).getInterface(Ci.nsIDOMWindowUtils);
        if (winUtils && !winUtils.isParentWindowMainWidgetVisible) {
            targetWindow = null;
        }
        PromptService.alert(targetWindow,"",text+'');
	},

	setTitle: function( title ) {
		this.windowObject.document.title = title;
	},
	
	initMenuBar: function() {
		var menubar = this.windowObject.document.getElementById('systemMenuBar');
		if( navigator.platform.match(/mac/i) ) {
			this.systemMenuBar = menubar;
			var quitItem = this.windowObject.document.getElementById('menu_FileQuitItem');
			quitItem.setAttribute( 'oncommand', 'sfRootWindow.onClose(event);' );
		}
		else {
			menubar.parentNode.removeChild( menubar );
			this.systemMenuBar = null;
		}
	},
	
	openUrlWindow: function(url, options) {
		var win = this.windowObject.open(url,'', options);
		SiteFusion.Interface.RegisterChildWindow(win);
		win.onclose = function() {SiteFusion.Interface.UnregisterChildWindow(win); };
	},
	
	openUrlDialog: function(url, options) {
		var win = this.windowObject.openDialog(url,'', options);
		SiteFusion.Interface.RegisterChildWindow(win);
		win.onclose = function() {SiteFusion.Interface.UnregisterChildWindow(win); };
		setTimeout(function(){SiteFusion.Interface.CursorIdle();},50);
	},

	addBasicEvents: function() {

		var oThis = this;

		var onSizeModeChanged = function(event) { oThis.onSizeModeChanged(event); };
		this.windowObject.addEventListener( 'sizemodechange', onSizeModeChanged, true );
		this.eventHost.sizemodechange.msgType = SiteFusion.Comm.MSG_SEND;

		var onActivate = function(event) { oThis.onActivate(event); };
		this.windowObject.addEventListener( 'activate', onActivate, true );

		var onDeactivate = function(event) { oThis.onDeactivate(event); };
		this.windowObject.addEventListener( 'deactivate', onDeactivate, true );

	},

	onSizeModeChanged: function(event) {
		if (this.lastWindowState != this.windowObject.windowState) { // See mozilla bug https://bugzilla.mozilla.org/show_bug.cgi?id=715867
			this.fireEvent( 'sizemodechange', [this.windowObject.windowState] );
			this.lastWindowState = this.windowObject.windowState;
		}
	},

	onActivate: function(event) {
		this.fireEvent( 'activate' );
	},

	onDeactivate: function(event) {
		this.fireEvent( 'deactivate' );
	}
} );


SiteFusion.Classes.Window = Class.create( SiteFusion.Classes.BasicWindow, {
	sfClassName: 'XULWindow',
	
	initialize: function( winobj ) {
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
	},
	
	onResize: function( event ) {
		this.fireEvent( 'resize' );
	},
	
	onClose: function( event ) {
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
	},
	
	close: function() {
		for( var n = 0; n < SiteFusion.Interface.ChildWindows.length; n++ ) {
			if( SiteFusion.Interface.ChildWindows[n] )
				SiteFusion.Interface.ChildWindows[n].close();
		}
		
		this.windowObject.close();
	}
} );


SiteFusion.Classes.ChildWindow = Class.create( SiteFusion.Classes.BasicWindow, {
	sfClassName: 'XULChildWindow',
	
	initialize: function( win ) {
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
	},

	open: function() {
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
	},
	
	openWindow: function() {
		this.parentHostWindow.windowObject.open( "chrome://sitefusion/content/childwindow.xul?" + this.cid, "", this.features );
	},
	
	initWindow: function( win ) {
		SiteFusion.Interface.RegisterChildWindow( win );
		this.windowObject = win;
		win.sfNode = this;
		this.element = win.document.getElementById( 'sitefusion-dialog' );
		this.element.sfNode = this;
		win.sfRootWindow = SiteFusion.RootWindow;
		
		this.attachEvents();
		
		this.initMenuBar();
		this.fireEvent( 'initialized' );
	},
	
	attachEvents: function() {
		var oThis = this;
		var onClose = function(event) { oThis.onClose(event); };
		this.windowObject.addEventListener( 'close', onClose, true );
		
		var onResize = function(event) { oThis.onResize(event); };
		this.windowObject.addEventListener( 'resize', onResize, true );

		this.addBasicEvents();
	},

	close: function() {
		this.windowObject.close();
	},
	
	onResize: function( event ) {
		this.fireEvent( 'resize' );
	},

	onClose: function( event ) {
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
	}
	
} );


SiteFusion.Classes.Dialog = Class.create( SiteFusion.Classes.ChildWindow, {
	sfClassName: 'XULDialog',
	
	initialize: function( win ) {
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
	},
	
	openWindow: function() {
		this.parentHostWindow.windowObject.openDialog( "chrome://sitefusion/content/dialog.xul?" + this.cid, "", this.features );
	},
	
	attachEvents: function() {
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
	},
	
	onDialogButton: function( event ) {
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
	}
} );


SiteFusion.Classes.PrefWindow = Class.create( SiteFusion.Classes.Dialog, {
	sfClassName: 'XULPrefWindow',
	
	openWindow: function() {
		this.parentHostWindow.windowObject.openDialog( "chrome://sitefusion/content/prefwindow.xul?" + this.cid, "", this.features );
	},
	
	showPane: function( pane ) {
		this.element.showPane( pane.element );
	}
} );


SiteFusion.Classes.PrefPane = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULPrefPane',
	
	initialize: function( win ) {
		this.element = win.createElement( 'prefpane' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.PromptService = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'PromptService',
	
	initialize: function( win ) {
		this.hostWindow = win;
		this.element = win.createElement( 'box' );
		this.element.hidden = true;
	
		this.promptService = Components.classes["@mozilla.org/embedcomp/prompt-service;1"]
		                    .getService(Components.interfaces.nsIPromptService);

		this.setEventHost();
	},
	
	getTargetWindow: function() {
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
	},

	alert: function( title, text ) {
		this.promptService.alert( this.getTargetWindow(), title+'', text+'' );
	},
	
	alertCheck: function( title, text, checkMsg, checkState ) {
		checkState = { value: checkState };
		this.promptService.alertCheck( this.getTargetWindow(), title+'', text+'', checkMsg+'', checkState );
		this.fireEvent( 'yield', [ true, checkState.value, null, null, null ] );
	},
	
	confirm: function( title, text ) {
		var result = this.promptService.confirm( this.getTargetWindow(), title+'', text+'' );
		this.fireEvent( 'yield', [ result, null, null, null, null ] );
	},
	
	confirmCheck: function( title, text, checkMsg, checkState ) {
		checkState = { value: checkState };
		var result = this.promptService.confirmCheck( this.getTargetWindow(), title+'', text+'', checkMsg+'', checkState );
		this.fireEvent( 'yield', [ result, checkState.value, null, null, null ] );
	},
	
	confirmEx: function( title, text, buttonFlags, button0Title, button1Title, button2Title, checkMsg, checkState ) {
		checkState = { value: checkState };
		var result = this.promptService.confirmEx( this.getTargetWindow(), title+'', text+'', buttonFlags, button0Title, button1Title, button2Title, checkMsg, checkState );
		this.fireEvent( 'yield', [ result, checkState.value, null, null, null ] );
	},
	
	prompt: function( title, text, textValue, checkMsg, checkState ) {
		if( checkMsg !== null ) checkMsg = checkMsg+'';
		checkState = { value: checkState };
		textValue = { value: textValue+'' };
		var result = this.promptService.prompt( this.getTargetWindow(), title+'', text+'', textValue, checkMsg, checkState );
		this.fireEvent( 'yield', [ result, checkState.value, textValue.value, null, null ] );
	},
	
	promptUsernameAndPassword: function( title, text, username, password, checkMsg, checkState ) {
		if( checkMsg !== null ) checkMsg = checkMsg+'';
		checkState = { value: checkState };
		username = { value: username+'' };
		password = { value: password+'' };
		var result = this.promptService.promptUsernameAndPassword( this.getTargetWindow(), title+'', text+'', username, password, checkMsg, checkState );
		this.fireEvent( 'yield', [ result, checkState.value, null, username.value, password.value ] );
	},
	
	promptPassword: function( title, text, password, checkMsg, checkState ) {
		if( checkMsg !== null ) checkMsg = checkMsg+'';
		checkState = { value: checkState };
		password = { value: password+'' };
		var result = this.promptService.promptPassword( this.hostWindow.windowObject, title+'', text+'', password, checkMsg, checkState );
		this.fireEvent( 'yield', [ result, checkState.value, null, null, password.value ] );
	},
	
	select: function( title, text, list, index ) {
		index = { value: index };
		for( var n = 0; n < list.length; n++ ) {
			list[n] = list[n]+'';
		}
		var result = this.promptService.select( this.hostWindow.windowObject, title+'', text+'', list.length, list, index );
		this.fireEvent( 'yield', [ result, null, index.value, null, null ] );
	}
} );


SiteFusion.Classes.AlertNotification = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'AlertNotification',
	
	initialize: function( win ) {
		this.hostWindow = win;
		this.element = win.createElement( 'box' );
		this.element.hidden = true;
		
		this.setEventHost( [ 'finished' ] );
	},
	
	showAlertNotification: function( imageUrl, title, text, name, textClickable ) {
		name = (name == null ? 'SiteFusionAlertNotification' : name);
		try {
			var alertsService = Cc["@mozilla.org/alerts-service;1"].getService(Ci.nsIAlertsService);
			
			var oThis = this;
			var observer = {
				observe: function( subject, topic, data ) {
					switch ( topic ) {
						case 'alertfinished':
							oThis.fireEvent( 'finished' );
						break;
						
						case 'alertclickcallback':
							oThis.fireEvent( 'command' );
						break;
					}
				}
			};
	        
	        alertsService.showAlertNotification( (imageUrl != '' ? this.parseImageURL(imageUrl):''), title, text, textClickable, '', observer, name );
	    }
	    catch( e ) {
	    	this.fireEvent( 'finished' );
	    }
	}
} );
