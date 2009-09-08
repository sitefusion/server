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


SiteFusion.Classes.CustomTree = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULCustomTree',
	
	isSparse: false,
	isDraggable: false,
	allowDrop: false,
	allowForeignDrop: false,
	sortable: false,
	
	initialize: function( win ) {
		this.element = win.createElement( 'tree' );
		this.element.sfNode = this;
		
		this.setEventHost( [ 'yield', 'openStateChange', 'cellValueChange', 'treeDrop', 'nodeDrop', 'sortColumn' ] );
		
		this.eventHost.yield.msgType = 1;
	},
	
	DragObserver: {
		onDragStart: function( event, transferData, dragAction ) {
			var tree = event.target.parentNode.sfNode;
			var selection = tree.getSelection();
			var Ci = Components.interfaces;
			var dragAllowed;
			
			if( tree.isDraggable ) {
				dragAllowed = true;
				for( index in selection ) {
					var row = tree.view.idToRow[selection[index]];
					if( typeof(row.isDraggable) != 'undefined' && row.isDraggable === false ) {
						dragAllowed = false;
						break;
					}
				}
			}
			else {
				var countAllowed = 0;
				for( index in selection ) {
					var row = tree.view.idToRow[selection[index]];
					if( typeof(row.isDraggable) != 'undefined' && row.isDraggable !== false )
						countAllowed++;
				}
				dragAllowed = countAllowed == selection.length;
			}
			
			if( !dragAllowed )
				return;
			
			transferData.data = new TransferData();
			transferData.data.addDataForFlavour( 'sfNode/XULTreeItem', selection.join(',') );
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
	},
	
	setView: function() {
		var oThis = this;
		this.view = new this.ViewConstructor( oThis );
		setTimeout( function() {
			oThis.element.view = oThis.view;
			var tc = oThis.element.getElementsByTagName('treechildren')[0];
			tc.setAttribute( 'ondraggesture', 'SiteFusion.Registry['+oThis.cid+'].onDragGesture(event);' );
		}, 1 );
	},

	onDragGesture: function( event ) {
		nsDragAndDrop.startDrag( event, this.DragObserver );
	},
	
	getSelection: function() {
		var rows = new Array();
		var tree = this.element;
		var start = new Object();
		var end = new Object();
		
		var numRanges = this.view.selection.getRangeCount();
		
		for( var t = 0; t < numRanges; t++ ) {
			this.view.selection.getRangeAt( t, start, end );
			
			for( var v = start.value; v <= end.value; v++ ) {
				if( v < 0 ) v = 0;
				rows.push( this.view.visibleData[v].id );
			}
		}
		
		return rows;
	},
	
	yield: function() {
		this.fireEvent( 'yield', this.getSelection() );
	},

	select: function( itemJSON ) {
		var idx, ids = eval('('+itemJSON+')');
		this.view.selection.clearSelection();
		
		for( var n = 0; n < ids.length; n++ ) {
			idx = this.view.getRowIndex( this.view.idToRow[ids[n]] );
			if( idx !== false )
				this.view.selection.toggleSelect( idx );
		}
	}
} );


SiteFusion.Classes.CustomTree.ViewConstructor = function( tree ) {
	this.sfTree = tree;
	
	this.treeBox = null;
	this.selection = null;
	
	this.invalidateTableTimer = null;
	
	this.data = [];
	this.visibleData = [];
	this.idToRow = [];
	this.rowCount = 0;
	
	this.setTree = function( treebox ) { this.treeBox = treebox; };
	
	this.updateDataSetRow = function( rowJSON ) {
		var update = eval('('+rowJSON+')');
		var row = this.idToRow[update.id];
		
		var idx = this.getRowIndex( row );
		
		if( update.columns ) {
			for( col in update.columns ) {
				row.columns[col] = update.columns[col];
			}
		}
		
		if( typeof(update.isContainer) != 'undefined' )
			row.isContainer = update.isContainer;
		if( typeof(update.isEmpty) != 'undefined' )
			row.isEmpty = update.isEmpty;
		if( typeof(update.isOpen) != 'undefined' ) {
			if( row.isOpen != update.isOpen )
				this.toggleOpenState( idx );
		}
		if( typeof(update.primaryImage) != 'undefined' )
			row.primaryImage = update.primaryImage;
		if( typeof(update.progressModes) != 'undefined' )
			row.progressModes = update.progressModes;
		if( typeof(update.editableCells) != 'undefined' )
			row.editableCells = update.editableCells;
		if( typeof(update.isDraggable) != 'undefined' ) {
			if( update.isDraggable === null )
				delete row.isDraggable;
			else
				row.isDraggable = update.isDraggable;
		}
		if( typeof(update.allowDrop) != 'undefined' ) {
			if( update.allowDrop === null )
				delete row.allowDrop;
			else
				row.allowDrop = update.allowDrop;
		}
		
		this.invalidateRow( idx );
	};
	
	this.addDataSetRow = function( rowJSON, pos ) {
		var row = eval('('+rowJSON+')');
		if( row.isContainer ) row.children = [];
		var visiblePos;
		var updateRowCount = false;
		
		if( row.parentId === null ) {
			var idx;
			
			if( pos === null || pos == this.data.length ) {
				this.data.push( row );
				this.visibleData.push( row );
				visiblePos = this.visibleData.length - 1;
			}
			else {
				visiblePos = this.getRowIndex( this.data[pos] );
				this.data.splice( pos, 0, row );
				this.visibleData.splice( visiblePos, 0, row );
			}
			
			updateRowCount = true;
		}
		else {
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
			}
			else {
				if( pos === null || pos >= parentRow.children.length ) {
					if( this.isOpenRecursive( parentRow ) ) {
						var lastSibling = parentRow.children[parentRow.children.length-1];
						visiblePos = this.getRowIndex(lastSibling) + this.getVisibleRowCount(lastSibling);
						this.visibleData.splice( visiblePos, 0, row );
						updateRowCount = true;
					}
					parentRow.children.push( row );
				}
				else {
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
		
		if( updateRowCount && this.treeBox )
			this.treeBox.rowCountChanged( visiblePos, 1 );
	};
	
	this.removeDataSetRow = function( rowId ) {
		var row = this.idToRow[rowId];
		
		var idx = this.getRowIndex( row );
		var visibleCount = this.getVisibleRowCount( row );
		var ret = this.visibleData.splice( idx, visibleCount );
		
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
		
		if( this.treeBox )
			this.treeBox.rowCountChanged( idx, -visibleCount );
	};
	
	this.applyChildOrder = function( rowId, orderJSON ) {
		var childSet = rowId === null ? this.data : this.idToRow[rowId].children;
		var order = eval('('+orderJSON+')');
		var reorderVisible = rowId === null ? true : this.isOpenRecursive( this.idToRow[rowId] );
		if( reorderVisible ) {
			var startAt = rowId === null ? 0 : this.getRowIndex(this.idToRow[rowId]);
			var visData = [];
			var visPos = rowId === null ? startAt : startAt+1;
		}
		
		if( reorderVisible ) {
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
	
	this.toggleOpenState = function( idx ) {
		var row = this.visibleData[idx];
		
		if(! row.isContainer ) return;
		
		if( row.isOpen ) {
			this.sfTree.fireEvent( 'openStateChange', [ false, row.id ] );
			
			var visCount = this.getVisibleRowCount( row ) - 1;
			this.visibleData.splice( idx + 1, visCount );
			
			row.isOpen = false;
			this.rowCount = this.visibleData.length;
			this.treeBox.rowCountChanged( idx + 1, -visCount );
			this.invalidateRow( idx );
		}
		else {
			this.sfTree.fireEvent( 'openStateChange', [ true, row.id ] );
			
			var added = this.openRecursive( idx, row );
			
			row.isOpen = true;
			this.rowCount = this.visibleData.length;
			this.treeBox.rowCountChanged( idx + 1, added );
			this.invalidateRow( idx );
		}
	};
	
	this.openRecursive = function( idx, row ) {
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
	
	this.getVisibleRowCount = function( row ) {
		var count = 1;
		
		if( row.isContainer && row.isOpen ) {
			for( var n = 0; n < row.children.length; n++ )
				count += this.getVisibleRowCount( row.children[n] );
		}
		
		return count;
	};
	
	this.getActualRowCount = function( row ) {
		var count = 1;
		
		if( row.isContainer ) {
			for( var n = 0; n < row.children.length; n++ )
				count += this.getActualRowCount( row.children[n] );
		}
		
		return count;
	};
	
	this.getRowIndex = function( row, startAt ) {
		if( ! startAt ) startAt = 0;
		for( var n = startAt; n < this.visibleData.length; n++ ) {
			if( this.visibleData[n] === row ) {
				return n;
			}
		}
		
		return false;
	};
	
	this.getParentIndex = function( idx ) {
		if( this.visibleData[idx].parentId === null ) return -1;
		
		for( var n = 0; n < this.visibleData.length; n++ ) {
			if( this.visibleData[n].id == this.visibleData[idx].parentId ) {
				return n;
			}
		}
	};
	
	this.getLevel = function( idx ) {
		var level = 0;
		var row = this.visibleData[idx];
		
		while( row.parentId !== null ) {
			row = this.idToRow[row.parentId];
			level++;
		}
		
		return level;
	};
	
	this.getCellText = function( idx, column ) {
		return this.visibleData[idx].columns[column.index];
	};
	
	this.getCellValue = function( idx, column ) {
		return this.visibleData[idx].columns[column.index];
	};
	
	this.isContainer = function( idx ) {
		return this.visibleData[idx].isContainer;
	};
	
	this.isContainerOpen = function( idx ) {
		return this.visibleData[idx].isOpen;
	};
	
	this.isContainerEmpty = function( idx ) {
		return this.visibleData[idx].isEmpty;
	};

	this.getImageSrc = function( idx, col ) {
		if( col.primary && this.visibleData[idx].primaryImage ) {
			var url = this.visibleData[idx].primaryImage;
			if( url.substr(0,1) == '/' ) url = SiteFusionBaseURI + url;
			return url;
		}
		else return null;
	};
	
	this.isSeparator = function( idx ) {
		return (this.visibleData[idx].isSeparator ? true:false);
	};
	
	this.getProgressMode = function( idx, col ) {
		var Ci = Components.interfaces;
		if( this.visibleData[idx].progressModes && this.visibleData[idx].progressModes[col.index] ) {
			switch ( this.visibleData[idx].progressModes[col.index] ) {
				case 'normal':			return Ci.nsITreeView.PROGRESS_NORMAL;
				case 'undetermined':	return Ci.nsITreeView.PROGRESS_UNDETERMINED;
				default:				return Ci.nsITreeView.PROGRESS_NONE;
			}
		}
	};
	
	this.isEditable = function( idx, col ) {
		if( this.visibleData[idx].editableCells && this.visibleData[idx].editableCells[col.index] )
			return true;
		
		return false;
	};
	
	this.setCellText = function( idx, col, value ) {
		this.visibleData[idx].columns[col.index] = value;
		this.invalidateRow( idx );
		var id = this.visibleData[idx].id;
		var oThis = this;
		setTimeout( function() {
			oThis.sfTree.fireEvent( 'cellValueChange', [ oThis.visibleData[idx].id, col.index, value ] );
		}, 1 );
	};
	
	this.setCellValue = function( idx, col, value ) {
		this.visibleData[idx].columns[col.index] = value;
		this.invalidateRow( idx );
		var id = this.visibleData[idx].id;
		var oThis = this;
		setTimeout( function() {
			oThis.sfTree.fireEvent( 'cellValueChange', [ oThis.visibleData[idx].id, col.index, value ] );
		}, 1 );
	};
	
	this.canDrop = function( idx, orientation ) {
		var row = this.visibleData[idx];
		var Cc = Components.classes;
		var Ci = Components.interfaces;
		var allowedOrient;
		
		if( this.sfTree.allowDrop ) {
			if( typeof(row.allowDrop) != 'undefined' ) {
				if( row.allowDrop === false ) return false;
				allowedOrient = row.allowDrop;
			}
			else allowedOrient = this.sfTree.allowDrop;
		}
		else {
			if( typeof(row.allowDrop) == 'undefined' || row.allowDrop === false )
				return false;
			
			allowedOrient = row.allowDrop;
		}
		
		if( this.sfTree.preventCircularHeritage || !this.sfTree.allowForeignDrop ) {
			var ds = Cc["@mozilla.org/widget/dragservice;1"].getService(Ci.nsIDragService);
			var session = ds.getCurrentSession();

			if( this.sfTree.preventCircularHeritage ) {
				if( session.sourceNode && session.sourceNode.tagName == 'treechildren' && session.sourceNode.parentNode.sfNode == this.sfTree ) {
					var selection = session.sourceNode.parentNode.sfNode.draggedSelection;
					for( var n = 0; n < selection.length; n++ ) {
						var obj = row;
						while( 1 ) {
							if( obj.id == selection[n] )
								return false;
							if( obj.parentId === null )
								break;
							
							obj = this.idToRow[obj.parentId];
						}
					}
				}
			}
			if( !this.sfTree.allowForeignDrop ) {
				if( !(session.sourceNode && session.sourceNode.tagName == 'treechildren' && session.sourceNode.parentNode.sfNode == this.sfTree) )
					return false;
			}
		}
		
		switch ( allowedOrient ) {
			case 'any':		return true;
			case 'on':		return ( orientation == Ci.nsITreeView.DROP_ON );
			case 'between':	return ( orientation != Ci.nsITreeView.DROP_ON );
		}
	};
	
	this.drop = function( idx, orientation ) {
		var ds = Cc["@mozilla.org/widget/dragservice;1"].getService(Ci.nsIDragService);
		var session = ds.getCurrentSession();
		
		if( session.sourceNode ) {
			switch ( session.sourceNode.tagName ) {
				case 'treechildren':
				//	session.sourceNode.parentNode.widgetObj.draggedSelection;
					this.sfTree.fireEvent( 'treeDrop', [
						session.sourceNode.parentNode.sfNode,
						session.sourceNode.parentNode.sfNode.draggedSelection.join(','),
						this.visibleData[idx].id,
						orientation
					] );
					break;
				
				default:
					this.sfTree.fireEvent( 'nodeDrop', [
						session.sourceNode.sfNode,
						this.visibleData[idx].id,
						orientation
					] );
					break;
			}
		}
	};
	
	this.cycleHeader = function( col ) {
		if( !this.sfTree.sortable ) return;
		this.sfTree.fireEvent( 'sortColumn', [ col.index ] );
	};
	
	this.isSorted = function() { return true; },
	this.getRowProperties = function( row, props ) {},
	this.getCellProperties = function( row, col, props ) {},
	this.getColumnProperties = function( colid, col, props ) {},
	
	this.isOpenRecursive = function( row ) {
		var par = row;
		
		while( par.isOpen ) {
			if( par.parentId === null )
				return true;
			
			par = this.idToRow[par.parentId];
		}
		
		return false;
	};
	
	this.invalidateRow = function( idx ) {
		if( this.treeBox )
			this.treeBox.invalidateRow( idx );
	};
	
	this.invalidateTable = function() {
		if( this.invalidateTableTimer ) {
			clearTimeout( this.invalidateTableTimer );
		}
		var oThis = this;
		this.invalidateTableTimer = setTimeout( function() { oThis.treeBox.invalidate(); oThis.invalidateTableTimer = null; }, 1 );
	};
};

