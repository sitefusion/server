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

SiteFusion.Classes.Box = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULBox';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Box.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Box.prototype.constructor = SiteFusion.Classes.Box;

    SiteFusion.Classes.Box.prototype.initialize = function( win ) {
        this.element = win.createElement( 'box' );
        this.element.sfNode = this;

        this.setEventHost();
    };


SiteFusion.Classes.HBox = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULHBox';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.HBox.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.HBox.prototype.constructor = SiteFusion.Classes.HBox;

    SiteFusion.Classes.HBox.prototype.initialize = function( win ) {
        this.element = win.createElement( 'hbox' );
        this.element.sfNode = this;

        this.setEventHost();
    };


SiteFusion.Classes.VBox = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULVBox';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.VBox.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.VBox.prototype.constructor = SiteFusion.Classes.VBox;

    SiteFusion.Classes.VBox.prototype.initialize = function( win ) {
        this.element = win.createElement( 'vbox' );
        this.element.sfNode = this;

        this.setEventHost();
    };


SiteFusion.Classes.BBox = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULBBox';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.BBox.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.BBox.prototype.constructor = SiteFusion.Classes.BBox;

    SiteFusion.Classes.BBox.prototype.initialize = function( win ) {
        this.element = win.createElement( 'bbox' );
        this.element.sfNode = this;

        this.setEventHost();
    };


SiteFusion.Classes.ScrollBox = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULScrollBox';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.ScrollBox.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.ScrollBox.prototype.constructor = SiteFusion.Classes.ScrollBox;

    SiteFusion.Classes.ScrollBox.prototype.initialize = function( win ) {
        this.element = win.createElement( 'scrollbox' );
        this.element.sfNode = this;

        this.setEventHost();
    };

    SiteFusion.Classes.ScrollBox.prototype.scrollTo = function( x, y ) {
        var oThis = this;

        SiteFusion.Interface.DeferredCallbacks.push(function() {
            try {
                /* The old way for retrieving the scroll object */
                var xpcomInterface = oThis.element.boxObject.QueryInterface(Ci.nsIScrollBoxObject);
            } catch(e) {
                /* The new way for retrieving the scroll object */
                var xpcomInterface = oThis.element.boxObject;
            }

            xpcomInterface.scrollTo(x, y);
        });
    };

    SiteFusion.Classes.ScrollBox.prototype.ensureElementIsVisible = function( node ) {
        var oThis = this;
        SiteFusion.Interface.DeferredCallbacks.push(function() {
            try {
                /* The old way for retrieving the scroll object */
                var xpcomInterface = oThis.element.boxObject.QueryInterface(Ci.nsIScrollBoxObject);
            } catch(e) {
                /* The new way for retrieving the scroll object */
                var xpcomInterface = oThis.element.boxObject;
            }

            xpcomInterface.ensureElementIsVisible(node.element);
        });
    };


SiteFusion.Classes.HTMLBox = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULHTMLBox';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.HTMLBox.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.HTMLBox.prototype.constructor = SiteFusion.Classes.HTMLBox;

    SiteFusion.Classes.HTMLBox.prototype.initialize = function( win ) {
        this.element = win.createElement( 'browser' );
        this.element.sfNode = this;
        this.element.setAttribute('disablehistory', true);
        this.element.setAttribute( 'src', 'about:blank' );

        this.setEventHost();
    };

    SiteFusion.Classes.HTMLBox.prototype.setContent = function( html ) {
        if( ! this.element.contentDocument ) {
            var oThis = this;
            setTimeout( function() { oThis.setContent( html ); }, 10 );
            return;
        }

        this.element.contentDocument.open();
        this.element.contentDocument.write( html );
        this.element.contentDocument.close();
    };


SiteFusion.Classes.Stack = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULStack';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Stack.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Stack.prototype.constructor = SiteFusion.Classes.Stack;

    SiteFusion.Classes.Stack.prototype.initialize = function( win ) {
        this.element = win.createElement( 'stack' );
        this.element.sfNode = this;

        this.setEventHost();
    };


SiteFusion.Classes.Deck = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULDeck';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Deck.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Deck.prototype.constructor = SiteFusion.Classes.Deck;

    SiteFusion.Classes.Deck.prototype.initialize = function( win ) {
        this.element = win.createElement( 'deck' );
        this.element.sfNode = this;

        this.setEventHost();
    };

    SiteFusion.Classes.Deck.prototype.selectedPanel = function( panel ) {
        var oThis = this;
        SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.element.selectedPanel = panel.element; } );
    };


SiteFusion.Classes.Grid = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULGrid';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Grid.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Grid.prototype.constructor = SiteFusion.Classes.Grid;

    SiteFusion.Classes.Grid.prototype.initialize = function( win ) {
        this.element = win.createElement( 'grid' );
        this.element.sfNode = this;

        this.setEventHost();
    };


SiteFusion.Classes.Columns = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULColumns';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Columns.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Columns.prototype.constructor = SiteFusion.Classes.Columns;

    SiteFusion.Classes.Columns.prototype.initialize = function( win ) {
        this.element = win.createElement( 'columns' );
        this.element.sfNode = this;

        this.setEventHost();
    };


SiteFusion.Classes.Rows = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULRows';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Rows.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Rows.prototype.constructor = SiteFusion.Classes.Rows;

    SiteFusion.Classes.Rows.prototype.initialize = function( win ) {
        this.element = win.createElement( 'rows' );
        this.element.sfNode = this;

        this.setEventHost();
    };


SiteFusion.Classes.Column = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULColumn';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Column.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Column.prototype.constructor = SiteFusion.Classes.Column;

    SiteFusion.Classes.Column.prototype.initialize = function( win ) {
        this.element = win.createElement( 'column' );
        this.element.sfNode = this;

        this.setEventHost();
    };


SiteFusion.Classes.Row = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULRow';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Row.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Row.prototype.constructor = SiteFusion.Classes.Row;

    SiteFusion.Classes.Row.prototype.initialize = function( win ) {
        this.element = win.createElement( 'row' );
        this.element.sfNode = this;

        this.setEventHost();
    };
