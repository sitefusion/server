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

SiteFusion.Classes.CustomTree = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULCustomTree';

    this.DragObserver = {
        onDragStart: function( event ) {
            var tree = event.target.parentNode.sfNode;
            var selection = tree.getSelection();
            var dragAllowed;

            if( tree.isDraggable ) {
                dragAllowed = true;
                for( var n = 0; n < selection.length; n++ ) {
                    var row = tree.view.idToRow[selection[n]];
                    if( typeof(row.isDraggable) != 'undefined' && row.isDraggable === false ) {
                        dragAllowed = false;
                        break;
                    }
                }
            } else {
                var countAllowed = 0;
                for( var n = 0; n < selection.length; n++ ) {
                    var row = tree.view.idToRow[selection[n]];
                    if( typeof(row.isDraggable) != 'undefined' && row.isDraggable !== false )
                        countAllowed++;
                }
                dragAllowed = countAllowed == selection.length;
            }

            if( !dragAllowed )
                return;

            event.dataTransfer.setData( 'sfNode/XULTreeItem', selection.join(',') );
            tree.draggedSelection = selection;

            var action = tree.isDraggable;
            if( tree.view.idToRow[selection[0]].isDraggable ) action = tree.view.idToRow[selection[0]].isDraggable;

            switch ( action ) {
                case 'move':
                    dragAction.action = Ci.nsIDragService.DRAGDROP_ACTION_MOVE;
                    break;

                case 'copy':
                    dragAction.action = Ci.nsIDragService.DRAGDROP_ACTION_COPY;
                    break;

                case 'link':
                    dragAction.action = Ci.nsIDragService.DRAGDROP_ACTION_LINK;
                    break;

                case 'moveOrCopy':
                    dragAction.action = Ci.nsIDragService.DRAGDROP_ACTION_MOVE + Ci.nsIDragService.DRAGDROP_ACTION_COPY;
                    break;
            }
        },
        onDrop: function (aEvent, aXferData, aDragSession) { },
        onDragExit: function (aEvent, aDragSession) { },
        onDragOver: function (aEvent, aFlavour, aDragSession) { },
        getSupportedFlavours: function() { return null; }
    };

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.CustomTree.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.CustomTree.prototype.constructor = SiteFusion.Classes.CustomTree;

    SiteFusion.Classes.CustomTree.prototype.initialize = function( win ) {
        this.element = win.createElement( 'tree' );
        this.element.sfNode = this;
        this.hostWindow = win;

        this.rightDown = false;

        this.isSparse = false;
        this.isDraggable = false;
        this.allowDrop = false;
        this.allowForeignDrop = false;
        this.allowFileDrop = false;
        this.sortable = false;

        this.element.boxObject.QueryInterface(Components.interfaces.nsITreeBoxObject);

        this.setEventHost( [ 'yield', 'openStateChange', 'cellValueChange', 'treeDrop', 'nodeDrop', 'fileDrop', 'sortColumn', 'itemClick', 'itemDoubleClick', 'itemContextClick' ] );
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

    SiteFusion.Classes.CustomTree.prototype.setView = function() {
        var oThis = this;
        this.view = new SiteFusion.Classes.CustomTree.ViewConstructor( oThis );
        this.hostWindow.windowObject.setTimeout( function() {
            oThis.element.view = oThis.view;
            var tc = oThis.element.getElementsByTagName('treechildren')[0];

            oThis.element.addEventListener('mousedown', (e) => {
                if (e.button == 2) {
                    if (oThis.mouseUpTimer) {
                        oThis.hostWindow.windowObject.clearTimeout(oThis.mouseUpTimer);
                    }
                    oThis.rightDown = true;
                }
            });

            oThis.element.addEventListener('mouseup', (e) => {
                oThis.mouseUpTimer = oThis.hostWindow.windowObject.setTimeout( function() {
                    oThis.rightDown = false;
                }, 500);

            });

            tc.setAttribute( 'ondragstart', 'sfRootWindow.windowObject.SiteFusion.Registry['+oThis.cid+'].onDragStartEvent(event);' );

            oThis.element.setAttribute( 'ondblclick', 'sfRootWindow.windowObject.SiteFusion.Registry['+oThis.cid+'].onItemEvent(event, \'itemDoubleClick\');' );

            oThis.element.setAttribute( 'onclick', 'sfRootWindow.windowObject.SiteFusion.Registry['+oThis.cid+'].onItemEvent(event, \'itemClick\');' );

            oThis.element.setAttribute( 'oncontextmenu', 'sfRootWindow.windowObject.SiteFusion.Registry['+oThis.cid+'].onItemEvent(event, \'itemContextClick\');' );
        }, 1 );
        this.hostWindow.windowObject.setTimeout(function() {if (oThis.element.columns){oThis.element.columns.restoreNaturalOrder();}},200);
    };

    SiteFusion.Classes.CustomTree.prototype.onItemEvent = function ( eventObj, eventName ) {
        var row = {}, column = {}, part = {};
        this.element.boxObject.getCellAt(eventObj.clientX, eventObj.clientY, row, column, part);

        if(row.value != -1) {
            var rowId = this.view.visibleData[row.value].id;
            var colIndex = column.value.index;

            if (eventName == 'itemDoubleClick') {
                //filter false double clicks that include other buttons than just the primary one
                var oThis = this;

                //timer is needed in order to make sure
                this.hostWindow.windowObject.setTimeout( function() {
                    if (oThis.rightDown == false) {
                        oThis.fireEvent( eventName, [ rowId, colIndex, part.value, eventObj.clientX, eventObj.clientY ] );
                    }
                }, 100);
                eventObj.preventDefault();
                return;
            }
            this.fireEvent( eventName, [ rowId, colIndex, part.value, eventObj.clientX, eventObj.clientY ] );
        }
    };

    SiteFusion.Classes.CustomTree.prototype.onDragStartEvent = function( event ) {
        var tree = event.target.parentNode.sfNode;
        var selection = tree.getSelection();
        if (selection.length == 0 || typeof selection[0] == 'undefined') {
            return;
        }

        var dragAllowed;

        if( tree.isDraggable ) {
            dragAllowed = true;
            for( var n = 0; n < selection.length; n++ ) {
                var row = tree.view.idToRow[selection[n]];
                if( typeof(row.isDraggable) != 'undefined' && row.isDraggable === false ) {
                    dragAllowed = false;
                    break;
                }
            }
        } else {
            var countAllowed = 0;
            for( var n = 0; n < selection.length; n++ ) {
                var row = tree.view.idToRow[selection[n]];
                if( typeof(row.isDraggable) != 'undefined' && row.isDraggable !== false ) {
                    countAllowed++;
                }
            }
            dragAllowed = countAllowed == selection.length;
        }

        if( !dragAllowed ) {
            return;
        }

        event.dataTransfer.setData( 'sfNode/XULTreeItem', selection.join(',') );
        tree.draggedSelection = selection;

        var action = tree.isDraggable;
        if( tree.view.idToRow[selection[0]].isDraggable ) action = tree.view.idToRow[selection[0]].isDraggable;

        event.dataTransfer.effectAllowed = action;
    };

    SiteFusion.Classes.CustomTree.prototype.getSelection = function() {
        var rows = [];
        var tree = this.element;
        var start = {};
        var end = {};

        if( this.view.selection ) {
            var numRanges = this.view.selection.getRangeCount();

            for( var t = 0; t < numRanges; t++ ) {
                this.view.selection.getRangeAt( t, start, end );

                for( var v = start.value; v <= end.value; v++ ) {
                    if( v < 0 ) v = 0;
                    rows.push( this.view.visibleData[v].id );
                }
            }
        }

        return rows;
    };

    SiteFusion.Classes.CustomTree.prototype.yield = function() {
        this.fireEvent( 'yield', this.getSelection() );
    };

    SiteFusion.Classes.CustomTree.prototype.select = function( itemJSON ) {
        if (!this.view.selection) {
            var oThis = this;
            this.hostWindow.windowObject.setTimeout(function() { oThis.select( itemJSON ); }, 10);
            return;
        }

        this.view.selection.selectEventsSuppressed = true;

        var idx, ids = eval('(' + itemJSON + ')');
        this.view.selection.clearSelection();

        for (var n = 0; n < ids.length; n++) {
            idx = this.view.getRowIndex(this.view.idToRow[ids[n]]);
            if (idx !== false) {
                this.view.selection.toggleSelect(idx);
            }
        }

        this.view.selection.selectEventsSuppressed = false;
    };


SiteFusion.Classes.CustomTree.ViewConstructor = function( tree ) {
    this.sfTree = tree;

    this.treeBox = null;
    this.selection = null;

    this.invalidateTableTimer = null;

    this.data = [];
    this.visibleData = [];
    this.idToRow = [];
    this.rowCount = 0;
    this.columnProperties = [];
};

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.setTree = function( treebox ) { this.treeBox = treebox; };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.updateDataSetRow = function( rowJSON ) {
        var update = eval('(' + rowJSON + ')');
        var row = this.idToRow[update.id];

        var idx = this.getRowIndex( row );

        if( update.columns ) {
            for( var col in update.columns ) {
                row.columns[col] = update.columns[col];
            }
        }

        if( typeof(update.isContainer) != 'undefined' ) {
            row.isContainer = update.isContainer;
        }
        if( typeof(update.isEmpty) != 'undefined' ) {
            row.isEmpty = update.isEmpty;
        }
        if( typeof(update.isOpen) != 'undefined' ) {
            if( row.isOpen != update.isOpen ) {
                this.toggleOpenState( idx );
            }
        }
        if( typeof(update.primaryImage) != 'undefined' ) {
            row.primaryImage = update.primaryImage;
        }
        if( typeof(update.images) != 'undefined' ) {
            row.images = update.images;
        }
        if( typeof(update.progressModes) != 'undefined' ) {
            row.progressModes = update.progressModes;
        }
        if( typeof(update.editableCells) != 'undefined' ) {
            row.editableCells = update.editableCells;
        }
        if( typeof(update.isDraggable) != 'undefined' ) {
            if( update.isDraggable === null ) {
                delete row.isDraggable;
            } else {
                row.isDraggable = update.isDraggable;
            }
        }
        if( typeof(update.allowDrop) != 'undefined' ) {
            if( update.allowDrop === null ) {
                delete row.allowDrop;
            } else {
                row.allowDrop = update.allowDrop;
            }
        }
        if( typeof(update.properties) != 'undefined' ) {
            row.properties = update.properties;
        }
        if( typeof(update.cellProperties) != 'undefined' ) {
            row.cellProperties = update.cellProperties;
        }

        this.invalidateRow( idx );
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.addDataSetRow = function( rowJSON, pos ) {
        var row = eval('(' + rowJSON + ')');
        if( row.isContainer ) row.children = [];
        var visiblePos;
        var updateRowCount = false;

        if( row.parentId === null ) {
            var idx;

            if( pos === null || pos == this.data.length ) {
                this.data.push( row );
                this.visibleData.push( row );
                visiblePos = this.visibleData.length - 1;
            } else {
                visiblePos = this.getRowIndex( this.data[pos] );
                this.data.splice( pos, 0, row );
                this.visibleData.splice( visiblePos, 0, row );
            }

            updateRowCount = true;
        } else {
            var parentRow = this.idToRow[row.parentId];
            if( parentRow.children.length == 0 ) {
                parentRow.children.push( row );
                var parentIndex = this.getRowIndex( parentRow );

                if( this.isOpenRecursive(parentRow) ) {
                    visiblePos = parentIndex + 1;
                    this.visibleData.splice( visiblePos, 0, row );
                    updateRowCount = true;
                }
                this.invalidateRow( parentIndex );
            } else {
                if( pos === null || pos >= parentRow.children.length ) {
                    if( this.isOpenRecursive( parentRow ) ) {
                        var lastSibling = parentRow.children[parentRow.children.length-1];
                        visiblePos = this.getRowIndex(lastSibling) + this.getVisibleRowCount(lastSibling);
                        this.visibleData.splice( visiblePos, 0, row );
                        updateRowCount = true;
                    }
                    parentRow.children.push( row );
                } else {
                    if( this.isOpenRecursive( parentRow ) ) {
                        visiblePos = this.getRowIndex( parentRow.children[pos] );
                        this.visibleData.splice( visiblePos, 0, row );
                        updateRowCount = true;
                    }
                    parentRow.children.splice( pos, 0, row );
                }
            }
        }

        this.idToRow[row.id] = row;

        this.rowCount = this.visibleData.length;

        if( updateRowCount )
            this.rowCountChanged( visiblePos, 1 );
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.removeDataSetRow = function( rowId ) {
        var row = this.idToRow[rowId];

        var idx = this.getRowIndex( row );

        var visibleCount;
        if( idx !== false ){
            visibleCount = this.getVisibleRowCount( row );
            var ret = this.visibleData.splice( idx, visibleCount );
        }

        var sets = [];
        if( row.isContainer && row.children.length > 0 ) sets.push( row.children );

        for( var n = 0; n < sets.length; n++ ) {
            for( var m = 0; m < sets[n].length; m++ ) {
                if( sets[n][m].isContainer && sets[n][m].children.length > 0 )
                    sets.push( sets[n][m].children );

                delete this.idToRow[sets[n][m].id];
            }
        }

        var parentSet = row.parentId === null ? this.data:this.idToRow[row.parentId].children;

        for( var n = 0; n < parentSet.length; n++ ) {
            if( parentSet[n] === row ) {
                parentSet.splice( n, 1 );
                break;
            }
        }

        if( row.parentId !== null && parentSet.length == 0 )
            this.invalidateRow( this.getRowIndex( this.idToRow[row.parentId] ) );

        delete this.idToRow[rowId];

        this.rowCount = this.visibleData.length;

        this.rowCountChanged( idx, -visibleCount );
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.applyChildOrder = function( rowId, orderJSON ) {
        var childSet = rowId === null ? this.data : this.idToRow[rowId].children;
        var order = eval('(' + orderJSON + ')');
        var reorderVisible = rowId === null ? true : this.isOpenRecursive( this.idToRow[rowId] );
        var visData;
        var visPos = 0;

        if( reorderVisible ) {
            var startAt = rowId === null ? 0 : this.getRowIndex(this.idToRow[rowId]);
            visData = [];
            visPos = rowId === null ? startAt : startAt + 1;

            for( var n = 0; n < childSet.length; n++ ) {
                var idx = this.getRowIndex( childSet[n], startAt );
                var len = this.getVisibleRowCount( childSet[n] );
                visData[childSet[n].id] = this.visibleData.splice( idx, len );
            }
        }

        for( var n = 0; n < childSet.length; n++ ) {
            childSet[n] = this.idToRow[order[n]];

            if( reorderVisible ) {
                for( var v = 0; v < visData[childSet[n].id].length; v++ ) {
                    this.visibleData.splice( visPos, 0, visData[childSet[n].id][v] );
                    visPos++;
                }
            }
        }

        this.invalidateTable();
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.toggleOpenState = function( idx ) {
        var row = this.visibleData[idx];

        if(! row.isContainer ) return;

        if( row.isOpen ) {
            this.sfTree.fireEvent( 'openStateChange', [ false, row.id ] );

            var visCount = this.getVisibleRowCount( row ) - 1;
            this.visibleData.splice( idx + 1, visCount );

            row.isOpen = false;
            this.rowCount = this.visibleData.length;
            this.rowCountChanged( idx + 1, -visCount );
            this.invalidateRow( idx );
        } else {
            this.sfTree.fireEvent( 'openStateChange', [ true, row.id ] );

            var added = this.openRecursive( idx, row );

            row.isOpen = true;
            this.rowCount = this.visibleData.length;
            this.rowCountChanged( idx + 1, added );
            this.invalidateRow( idx );
        }
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.openRecursive = function( idx, row ) {
        var added = 0;

        for( var n = 0; n < row.children.length; n++ ) {
            this.visibleData.splice( idx + 1 + added, 0, row.children[n] );

            if( row.children[n].isContainer && row.children[n].isOpen ) {
                added += this.openRecursive( idx + 1 + added, row.children[n] );
            }

            added++;
        }

        this.rowCount = this.visibleData.length;

        return added;
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.getVisibleRowCount = function( row ) {
        var count = 1;

        if( row.isContainer && row.isOpen ) {
            for( var n = 0; n < row.children.length; n++ )
                count += this.getVisibleRowCount( row.children[n] );
        }

        return count;
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.getActualRowCount = function( row ) {
        var count = 1;

        if( row.isContainer ) {
            for( var n = 0; n < row.children.length; n++ )
                count += this.getActualRowCount( row.children[n] );
        }

        return count;
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.getRowIndex = function( row, startAt ) {
        if( ! startAt ) startAt = 0;
        for( var n = startAt; n < this.visibleData.length; n++ ) {
            if( this.visibleData[n] === row ) {
                return n;
            }
        }

        return false;
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.getParentIndex = function( idx ) {
        if (typeof(this.visibleData[idx]) == 'undefined') {
            return -1;
        }

        if (this.visibleData[idx].parentId === null) {
            return -1;
        }

        for(var n = 0; n < this.visibleData.length; n++) {
            if(this.visibleData[n].id == this.visibleData[idx].parentId) {
                return n;
            }
        }
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.getLevel = function( idx ) {
        var level = 0;
        if (typeof(this.visibleData[idx]) != 'undefined') {
            var row = this.visibleData[idx];
            while( row.parentId !== null ) {
                row = this.idToRow[row.parentId];
                level++;
            }
        }

        return level;
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.getCellText = function( idx, column ) {
        if (typeof(this.visibleData[idx]) == 'undefined') {
            return null;
        }

        if (this.visibleData[idx].isSeparator) {
            return null;
        }

        return this.visibleData[idx].columns[column.index];
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.getCellValue = function( idx, column ) {
        if (typeof(this.visibleData[idx]) == 'undefined') {
            return null;
        }

        return this.visibleData[idx].columns[column.index];
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.isContainer = function( idx ) {
        if (typeof(this.visibleData[idx]) == 'undefined') {
            return false;
        }

        return this.visibleData[idx].isContainer;
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.isContainerOpen = function( idx ) {
        if (typeof(this.visibleData[idx]) == 'undefined') {
            return false;
        }

        return this.visibleData[idx].isOpen;
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.isContainerEmpty = function( idx ) {
        if (typeof(this.visibleData[idx]) == 'undefined') {
            return true;
        }

        return this.visibleData[idx].isEmpty;
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.getImageSrc = function( idx, col ) {
        if (typeof(this.visibleData[idx]) == 'undefined') {
            return '';
        } else if (col.primary && this.visibleData[idx].primaryImage) {
            return this.sfTree.parseImageURL(this.visibleData[idx].primaryImage);
        } else if (this.visibleData[idx].images && this.visibleData[idx].images[col.index]) {
            return this.sfTree.parseImageURL(this.visibleData[idx].images[col.index]);
        } else {
            return '';
        }
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.isSeparator = function( idx ) {
        if (typeof(this.visibleData[idx]) == 'undefined') {
            return false;
        }

        return (this.visibleData[idx].isSeparator ? true : false);
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.getProgressMode = function( idx, col ) {
        var Ci = Components.interfaces;
        if (typeof(this.visibleData[idx]) == 'undefined') {
            return Ci.nsITreeView.PROGRESS_NONE;
        }

        if (this.visibleData[idx].progressModes && this.visibleData[idx].progressModes[col.index]) {
            switch ( this.visibleData[idx].progressModes[col.index] ) {
                case 'normal':          return Ci.nsITreeView.PROGRESS_NORMAL;
                case 'undetermined':    return Ci.nsITreeView.PROGRESS_UNDETERMINED;
                default:                return Ci.nsITreeView.PROGRESS_NONE;
            }
        }
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.isEditable = function( idx, col ) {
        if (typeof(this.visibleData[idx]) == 'undefined') {
            return false;
        }

        return (this.visibleData[idx].editableCells && this.visibleData[idx].editableCells[col.index]) ? true : false;
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.setCellText = function( idx, col, value ) {
        if (typeof(this.visibleData[idx]) != 'undefined') {
            this.visibleData[idx].columns[col.index] = value;
            this.invalidateRow(idx);

            var id = this.visibleData[idx].id;
            var oThis = this;
            this.sfTree.hostWindow.windowObject.setTimeout(function() {
                oThis.sfTree.fireEvent('cellValueChange', [oThis.visibleData[idx].id, col.index, value]);
            }, 1);
        }
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.setCellValue = function( idx, col, value ) {
        if (typeof(this.visibleData[idx]) != 'undefined') {
            this.visibleData[idx].columns[col.index] = value;
            this.invalidateRow(idx);

            var id = this.visibleData[idx].id;
            var oThis = this;
            this.sfTree.hostWindow.windowObject.setTimeout(function() {
                oThis.sfTree.fireEvent('cellValueChange', [oThis.visibleData[idx].id, col.index, value]);
            }, 1);
        }
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.canDrop = function( idx, orientation ) {
        if (typeof(this.visibleData[idx]) == 'undefined') {
            return false;
        }

        var row = this.visibleData[idx];
        var Cc = Components.classes;
        var Ci = Components.interfaces;
        var allowedOrient;

        if (this.sfTree.allowDrop) {
            if (typeof(row.allowDrop) != 'undefined') {
                if (row.allowDrop === false) {
                    return false;
                }

                allowedOrient = row.allowDrop;
            } else {
                allowedOrient = this.sfTree.allowDrop;
            }
        } else {
            if (typeof(row.allowDrop) == 'undefined' || row.allowDrop === false) {
                return false;
            }

            allowedOrient = row.allowDrop;
        }

        var ds = Cc["@mozilla.org/widget/dragservice;1"].getService(Ci.nsIDragService);
        var session = ds.getCurrentSession();

        if (session.isDataFlavorSupported("text/x-moz-url") || session.isDataFlavorSupported("application/x-moz-file")) {
            return session.canDrop = this.sfTree.allowFileDrop;
        }

        if (this.sfTree.preventCircularHeritage || !this.sfTree.allowForeignDrop) {
            if (this.sfTree.preventCircularHeritage) {
                if (session.sourceNode && session.sourceNode.tagName == 'treechildren' && session.sourceNode.parentNode.sfNode == this.sfTree) {
                    var selection = session.sourceNode.parentNode.sfNode.draggedSelection;
                    for (var n = 0; n < selection.length; n++) {
                        var obj = row;
                        while(1) {
                            if (obj.id == selection[n]) {
                                return false;
                            }

                            if(obj.parentId === null) {
                                break;
                            }

                            obj = this.idToRow[obj.parentId];
                        }
                    }
                }
            }

            if (!this.sfTree.allowForeignDrop) {
                if (!(session.sourceNode && session.sourceNode.tagName == 'treechildren' && session.sourceNode.parentNode.sfNode == this.sfTree)) {
                    return false;
                }
            }
        }

        switch (allowedOrient) {
            case 'any':     return true;
            case 'on':      return (orientation == Ci.nsITreeView.DROP_ON);
            case 'between': return (orientation != Ci.nsITreeView.DROP_ON);
        }
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.drop = function( idx, orientation ) {
        if (idx != -1 && typeof(this.visibleData[idx]) == 'undefined') {
            return;
        }

        var ds = Cc["@mozilla.org/widget/dragservice;1"].getService(Ci.nsIDragService);
        var session = ds.getCurrentSession();
        var id = (idx == -1 ? null : this.visibleData[idx].id);

        if( session.sourceNode ) {
            switch ( session.sourceNode.tagName ) {
                case 'treechildren':
                    this.sfTree.fireEvent( 'treeDrop', [
                        session.sourceNode.parentNode.sfNode,
                        session.sourceNode.parentNode.sfNode.draggedSelection.join(','),
                        id,
                        orientation
                    ] );
                break;

                default:
                    this.sfTree.fireEvent( 'nodeDrop', [
                        session.sourceNode.sfNode,
                        id,
                        orientation
                    ] );
                break;
            }
        } else {
            var _ios = Cc['@mozilla.org/network/io-service;1'].getService(Ci.nsIIOService);
            var fileProtHandler = _ios.getProtocolHandler("file").QueryInterface(Components.interfaces.nsIFileProtocolHandler);
            var uris = [];
            var fileService = new SiteFusion.Classes.FileService( window.sfNode );

            // Setup a transfer item to retrieve the file data
            var trans = Cc["@mozilla.org/widget/transferable;1"].createInstance(Ci.nsITransferable);
            trans.addDataFlavor("text/x-moz-url");
            trans.addDataFlavor("application/x-moz-file");

            for (var i = 0; i < session.numDropItems; i++) {
                var uri = null;

                session.getData(trans, i);
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
                        if (file)
                            uri = [file.path, fileService.resultFromFile(file)];
                    }
                }

                if (uri) {
                    if( typeof(uri) == 'string' && uri.substr(0,7) == 'file://' ) {
                        var file = fileProtHandler.getFileFromURLSpec( uri );
                        uri = [file.path, fileService.resultFromFile(file)];
                    }

                    uris.push(uri);
                }
            }

            this.sfTree.fireEvent( 'fileDrop', [
                uris,
                id,
                orientation
            ] );
        }
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.getRowProperties = function( idx, prop ) {
        var propStr = '';
        if (typeof(this.visibleData[idx]) != 'undefined') {
            var row = this.visibleData[idx];
            if( row.properties ) {
                var aserv = Cc["@mozilla.org/atom-service;1"].getService(Ci.nsIAtomService);
                for( var n = 0; n < row.properties.length; n++ ) {
                    if (typeof prop === 'undefined') {
                        propStr += (propStr.length ? " " : "") + row.properties[n];
                    }
                    else {
                        prop.AppendElement(aserv.getAtom(row.properties[n]));
                    }
                }
            }
        }

        if (propStr.length) {
            return propStr;
        }
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.getColumnProperties = function( column, prop ) {
        var propStr = '';
        if( this.columnProperties[column.index] ) {
            var aserv = Cc["@mozilla.org/atom-service;1"].getService(Ci.nsIAtomService);
            for( var n = 0; n < this.columnProperties[column.index].length; n++ ) {
                if (typeof prop === 'undefined') {
                    propStr += (propStr.length ? " " : "") + this.columnProperties[column.index][n];
                }
                else {
                    prop.AppendElement(aserv.getAtom(this.columnProperties[column.index][n]));
                }
            }
        }
        if (propStr.length) {
            return propStr;
        }
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.getCellProperties = function( idx, column, prop ) {
        var propStr = '';
        if (typeof(this.visibleData[idx]) != 'undefined') {
            var row = this.visibleData[idx];
            if( row.cellProperties && row.cellProperties[column.index] ) {
                var aserv = Cc["@mozilla.org/atom-service;1"].getService(Ci.nsIAtomService);
                for( var n = 0; n < row.cellProperties[column.index].length; n++ ) {
                    if (typeof prop === 'undefined') {
                        propStr += (propStr.length ? " " : "") + row.cellProperties[column.index][n];
                    }
                    else {
                        prop.AppendElement(aserv.getAtom(row.cellProperties[column.index][n]));
                    }
                }
            }
        }

        if (propStr.length) {
            return propStr;
        }
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.setColumnProperties = function( properties ) {
        this.columnProperties = eval('('+properties+')');
        this.invalidateTable();
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.cycleHeader = function( col ) {
        if ( !this.sfTree.sortable ) {
            return;
        }
        this.sfTree.fireEvent( 'sortColumn', [ col.index ] );
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.isSorted = function() { return true; };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.isOpenRecursive = function( row ) {
        var par = row;

        while( par.isOpen ) {
            if( par.parentId === null ) {
                return true;
            }

            par = this.idToRow[par.parentId];
        }

        return false;
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.invalidateRow = function( idx ) {
        if( this.treeBox ) {
            this.treeBox.invalidateRow( idx );
        }
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.invalidateTable = function() {
        if( this.invalidateTableTimer ) {
            this.sfTree.hostWindow.windowObject.clearTimeout( this.invalidateTableTimer );
        }
        var oThis = this;
        this.invalidateTableTimer = this.sfTree.hostWindow.windowObject.setTimeout( function() { oThis.treeBox.invalidate(); oThis.invalidateTableTimer = null; }, 1 );
    };

    SiteFusion.Classes.CustomTree.ViewConstructor.prototype.rowCountChanged = function( idx, count ) {
        if( this.treeBox ) {
            this.treeBox.rowCountChanged( idx, count );
        }
    };
