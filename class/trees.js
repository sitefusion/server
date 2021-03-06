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

SiteFusion.Classes.Tree =  function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULTree';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Tree.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Tree.prototype.constructor = SiteFusion.Classes.Tree;
    
    SiteFusion.Classes.Tree.prototype.initialize = function( win ) {
        this.element = win.createElement( 'tree' );
        this.element.sfNode = this;
        this.setEventHost( [ 'yield' ] );
        var oThis = this;
        win.windowObject.setTimeout(function() { oThis.element.columns.restoreNaturalOrder(); }, 100);
        
        this.eventHost.yield.msgType = 1;
        
        var isMac = navigator.platform.toUpperCase().indexOf('MAC')>=0;

        if (isMac) {
            this.element.addEventListener('popuphiding', function(e) {
                var origEl = e.target;
                var par =  origEl.parentNode;
                var el = par.removeChild(origEl);
                par.appendChild(el);
            });
        }
    };
    
    SiteFusion.Classes.Tree.prototype.yield = function() {
        var rows = [];
        var tree = this.element;
        var start = {};
        var end = {};
        
        var numRanges = tree.view.selection.getRangeCount();
        
        for( var t = 0; t < numRanges; t++ ) {
            tree.view.selection.getRangeAt( t, start, end );
            
            for( var v = start.value; v <= end.value; v++ ) {
                if( v < 0 ) v = 0;
                rows.push( tree.view.getItemAtIndex(v).sfNode );
            }
        }
        
        this.fireEvent( 'yield', rows );
    };

    SiteFusion.Classes.Tree.prototype.select = function( item ) {
        var oThis = this;
        SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.element.view.selection.select( oThis.element.view.getIndexOfItem( item.element ) ); } );
    };


SiteFusion.Classes.TreeCols = function() {
    SiteFusion.Classes.Node.apply(this, arguments);
    this.sfClassName = 'XULTreeCols';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.TreeCols.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.TreeCols.prototype.constructor = SiteFusion.Classes.TreeCols;
    
    SiteFusion.Classes.TreeCols.prototype.initialize = function( win ) {
        this.element = win.createElement( 'treecols' );
        this.element.sfNode = this;
        this.setEventHost();
    };


SiteFusion.Classes.TreeCol = function() {
    SiteFusion.Classes.Node.apply(this, arguments);
    this.sfClassName = 'XULTreeCol';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.TreeCol.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.TreeCol.prototype.constructor = SiteFusion.Classes.TreeCol;
    
    SiteFusion.Classes.TreeCol.prototype.initialize = function( win ) {
        this.element = win.createElement( 'treecol' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };


SiteFusion.Classes.TreeChildren = function() {
    SiteFusion.Classes.Node.apply(this, arguments);
    this.sfClassName = 'XULTreeChildren';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.TreeChildren.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.TreeChildren.prototype.constructor = SiteFusion.Classes.TreeChildren;
    
    SiteFusion.Classes.TreeChildren.prototype.initialize = function( win ) {
        this.element = win.createElement( 'treechildren' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };

SiteFusion.Classes.TreeItem = function() {
    SiteFusion.Classes.Node.apply(this, arguments);
    this.sfClassName = 'XULTreeItem';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.TreeItem.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.TreeItem.prototype.constructor = SiteFusion.Classes.TreeItem;
    
    SiteFusion.Classes.TreeItem.prototype.initialize = function( win ) {
        this.element = win.createElement( 'treeitem' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };

SiteFusion.Classes.TreeRow = function() {
    SiteFusion.Classes.Node.apply(this, arguments);
    this.sfClassName = 'XULTreeRow';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.TreeRow.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.TreeRow.prototype.constructor = SiteFusion.Classes.TreeRow;
    
    SiteFusion.Classes.TreeRow.prototype.initialize = function( win ) {
        this.element = win.createElement( 'treerow' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };


SiteFusion.Classes.TreeCell = function() {
    SiteFusion.Classes.Node.apply(this, arguments);
    this.sfClassName = 'XULTreeCell';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.TreeCell.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.TreeCell.prototype.constructor = SiteFusion.Classes.TreeCell;
    
    SiteFusion.Classes.TreeCell.prototype.initialize = function( win ) {
        this.element = win.createElement( 'treecell' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };


SiteFusion.Classes.TreeSeparator = function() {
    SiteFusion.Classes.Node.apply(this, arguments);
    this.sfClassName = 'XULTreeSeparator';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.TreeSeparator.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.TreeSeparator.prototype.constructor = SiteFusion.Classes.TreeSeparator;
    
    SiteFusion.Classes.TreeSeparator.prototype.initialize = function( win ) {
        this.element = win.createElement( 'treeseparator' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };
