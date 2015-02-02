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

SiteFusion.Classes.ListBox = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULListBox';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.ListBox.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.ListBox.prototype.constructor = SiteFusion.Classes.ListBox;

    SiteFusion.Classes.ListBox.prototype.initialize = function( win ) {
        this.element = win.createElement( 'listbox' );
        this.element.sfNode = this;
        
        this.setEventHost( [ 'yield' ] );
        
        this.eventHost.yield.msgType = 1;
    };
    
    SiteFusion.Classes.ListBox.prototype.selectItem = function( items ) {
                var oThis = this;
                window.setTimeout(function(){oThis.element.clearSelection();},100);
        
        for( var n = 0; n < items.length; n++ ) {
            var item = items[n];
            // tommyfix 23-07-2012
                        
            if (item) {
                window.setTimeout(function() { oThis.element.addItemToSelection( item.element ); }, 100);
            }
        }
    };
    
    SiteFusion.Classes.ListBox.prototype.yield = function() {
        var items = this.element.selectedItem ? new Array( this.element.selectedItem.sfNode ) : new Array( null );
        
        if( this.element.selectedItem ) {
            var selectedItems = this.element.selectedItems;
            
            for( var t = 0; t < selectedItems.length; t++ ) {
                items.push( selectedItems[t].sfNode );
            }
        }
        
        this.fireEvent( 'yield', items );
    };

    SiteFusion.Classes.ListBox.prototype.clearSelection = function() {
        this.element.clearSelection();
    };


SiteFusion.Classes.ListItem = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULListItem';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.ListItem.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.ListItem.prototype.constructor = SiteFusion.Classes.ListItem;

    SiteFusion.Classes.ListItem.prototype.initialize = function( win ) {
        this.element = win.createElement( 'listitem' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };


SiteFusion.Classes.ListCols = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULListCols';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.ListCols.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.ListCols.prototype.constructor = SiteFusion.Classes.ListCols;

    SiteFusion.Classes.ListCols.prototype.initialize = function( win ) {
        this.element = win.createElement( 'listcols' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };


SiteFusion.Classes.ListCol = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULListCol';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.ListCol.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.ListCol.prototype.constructor = SiteFusion.Classes.ListCol;

    SiteFusion.Classes.ListCol.prototype.initialize = function( win ) {
        this.element = win.createElement( 'listcol' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };


SiteFusion.Classes.ListHead = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULListHead';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.ListHead.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.ListHead.prototype.constructor = SiteFusion.Classes.ListHead;

    SiteFusion.Classes.ListHead.prototype.initialize = function( win ) {
        this.element = win.createElement( 'listhead' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };


SiteFusion.Classes.ListHeader = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULListHeader';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.ListHeader.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.ListHeader.prototype.constructor = SiteFusion.Classes.ListHeader;

    SiteFusion.Classes.ListHeader.prototype.initialize = function( win ) {
        this.element = win.createElement( 'listheader' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };


SiteFusion.Classes.ListCell = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULListCell';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.ListCell.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.ListCell.prototype.constructor = SiteFusion.Classes.ListCell;
    
    SiteFusion.Classes.ListCell.prototype.initialize = function( win ) {
        this.element = win.createElement( 'listcell' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };


SiteFusion.Classes.RichListBox = function() {
    SiteFusion.Classes.ListBox.apply(this, arguments);

    this.sfClassName = 'XULRichListBox';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.RichListBox.prototype = Object.create(SiteFusion.Classes.ListBox.prototype);
SiteFusion.Classes.RichListBox.prototype.constructor = SiteFusion.Classes.ListBox;

    SiteFusion.Classes.RichListBox.prototype.initialize = function( win ) {
        this.element = win.createElement( 'richlistbox' );
        this.element.sfNode = this;
        
        this.setEventHost( [ 'yield' ] );
        
        this.eventHost.yield.msgType = 1;
    };

SiteFusion.Classes.RichListItem = function() {
    SiteFusion.Classes.ListItem.apply(this, arguments);

    this.sfClassName = 'XULRichListItem';
    
    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.RichListItem.prototype = Object.create(SiteFusion.Classes.ListItem.prototype);
SiteFusion.Classes.RichListItem.prototype.constructor = SiteFusion.Classes.ListItem;

    SiteFusion.Classes.RichListItem.prototype.initialize = function( win ) {
        this.element = win.createElement( 'richlistitem' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };

