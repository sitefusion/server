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

SiteFusion.Registry = [];
SiteFusion.Classes = {};
SiteFusion.ClientComponents = {};
SiteFusion.CommandLineArguments = {};

SiteFusion.Initialize = function() {
    SiteFusion.RootWindow = new SiteFusion.Classes.Window( window );
    
    SiteFusion.RootWindow.fireEvent( 'initialized' );
};

SiteFusion.Interface = {
    ChildWindows: [],
    DeferredCallbacks: [],
    
    HandleDeferredChildAdditions: function() {
        var root = SiteFusion.Registry[0];
        this.HandleDeferredChildAdditionsRecursive( root );
        this.HandleDeferredCallbacks();
    },

    HandleDeferredChildAdditionsRecursive: function( node ) {
        if ( ! node ) {
            return;
        }
        
        node.isPainted = true;
        
        for ( var n = 0; n < node.deferredSFChildren.length; n++ ) {
            var item = node.deferredSFChildren[n];
            this.HandleDeferredChildAdditionsRecursive( item[1] );

            if( item[0] == 'addChild' ) {
                node.directAddChild( item[1] );
            } else if( item[0] == 'addChildBefore' ) {
                node.directAddChildBefore( item[1], item[2] );
            }
        }

        for ( var n = 0; n < node.sfChildren.length; n++ ) {
            this.HandleDeferredChildAdditionsRecursive( node.sfChildren[n] );
        }

        for ( var n = 0; n < node.deferredSFChildren.length; n++ ) {
            var item = node.deferredSFChildren[n];

            if ( item[0] == 'addChild' ) {
                node.sfChildren.push( item[1] );
            } else if( item[0] == 'addChildBefore' ) {
                for( var m = 0; m < node.sfChildren.length; m++ ) {
                    if( node.sfChildren[m] == item[2] ) {
                        node.sfChildren.splice( m, 0, item[1] );
                        break;
                    }
                }
            }
        }

        node.deferredSFChildren = [];
    },

    HandleDeferredCallbacks: function() {
        for( var n = 0; n < this.DeferredCallbacks.length; n++ ) {
            this.DeferredCallbacks[n]();
        }

        this.DeferredCallbacks = [];
    },
    
    SetCursor: function( cursor ) {
        var wm = Cc["@mozilla.org/appshell/window-mediator;1"].getService(Ci.nsIWindowMediator);
        var enumerator = wm.getEnumerator(null);
        while(enumerator.hasMoreElements()) {
            var win = enumerator.getNext();
            var func = function() {
                try { win.setCursor(cursor); } catch ( e ) { win.cursorTimer = win.setTimeout(func,10); }
            };
            if( win.cursorTimer ) win.clearTimeout(win.cursorTimer);
            win.cursorTimer = win.setTimeout( func, 1 );
        }
    },
    
    CursorBusy: function() {
        SiteFusion.Interface.SetCursor( 'wait' );
    },

    CursorBackground: function() {
        SiteFusion.Interface.SetCursor( 'progress' );
    },

    CursorIdle: function() {
        SiteFusion.Interface.SetCursor( 'auto' );
    },
    
    SetClipboardText: function(copytext) {
        var str = Components.classes["@mozilla.org/supports-string;1"].createInstance(Components.interfaces.nsISupportsString);
        if (!str) return false;
        str.data=copytext;
        
        var trans = Components.classes["@mozilla.org/widget/transferable;1"].createInstance(Components.interfaces.nsITransferable);
        if (!trans) return false;
        
        trans.addDataFlavor("text/unicode");
        trans.setTransferData("text/unicode",str,copytext.length*2);
        
        var clip = Components.classes["@mozilla.org/widget/clipboard;1"].createInstance(Components.interfaces.nsIClipboard);
        if (!clip) return false;
        
        clip.emptyClipboard(clip.kGlobalClipboard);
        clip.setData(trans,null,clip.kGlobalClipboard);
    },
    
    RegisterChildWindow: function( win ) {
        SiteFusion.Interface.ChildWindows.push( win );
    },
    
    UnregisterChildWindow: function( win ) {
        for( var n = 0; n < SiteFusion.Interface.ChildWindows.length; n++ ) {
            if( SiteFusion.Interface.ChildWindows[n] === win ) {
                SiteFusion.Interface.ChildWindows.splice( n, 1 );
                return;
            }
        }
    }
};

