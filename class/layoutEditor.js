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


SiteFusion.Classes.LayoutEditor = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULLayoutEditor',
	
	initialize: function( win ) {
		this.element = win.createElement( 'editor' );
		this.element.sfNode = this;
		
		this.hostWindow = win;
		this.storedSelection = [];
		
		this.setEventHost( [
			'before_initialize',
			'after_loaddata',
			'initialized',
			'yield',
			'document_state',
			'state_bold',
			'state_italic',
			'state_underline',
			'state_justifyleft',
			'state_justifycenter',
			'state_justifyright',
			'state_justifyfull',
			'state_insertorderedlist',
			'state_insertunorderedlist',
			'state_indent',
			'state_outdent',
			'state_link',
			'attr_link',
			'state_img',
			'attr_img',
			'state_table',
			'attr_table',
			'state_tr',
			'attr_tr',
			'state_td',
			'attr_td',
			'state_span',
			'attr_span'
		] );
	
		this.eventHost.initialized.msgType = 0;
		this.eventHost.yield.msgType = 1;
		this.eventHost.document_state.msgType = 0;
	},
	
	init: function() {
		this.element.setAttribute( 'src', 'about:blank' );
		this.fireEvent( 'initialized' );
	},

	loadLayoutHTML: function( html, frameid ) {
		this.element.contentWindow.document.write( html );
		
		this.editorElement = this.element.contentDocument.getElementById( frameid );
	
		this.element.contentDocument.body.contentEditable = false;
		this.editorElement.contentEditable = true;
	},

	loadBody: function( html ) {
		if( ! this.editorElement ) {
			this._loadBody = html;
			return;
		}
		
		this.editorElement.innerHTML = html;
		
		this.editorLoaded();
	},

	yield: function() {
		if(! this.editorElement )
			return;
		
		this.fireEvent( 'yield', [ this.editorElement.innerHTML ] );
	},

	editorLoaded: function() {
		var oThis = this;
		var func = function() { oThis.checkDocumentState(); };
		
		this.element.contentDocument.onmouseup = function() { window.setTimeout( func, 1 ); };
		this.element.contentDocument.onkeyup = func;
		
		this.htmlEditor = this.element.getHTMLEditor( this.element.contentWindow );
		this.textEditor = this.element.getEditor( this.element.contentWindow );
		
		this.htmlEditor.returnInParagraphCreatesNewParagraph = false;
		
		this.editorElement.focus();
		this.textEditor.selection.collapse( this.editorElement, 1 );
		this.checkDocumentState();
	
		this.fireEvent( 'after_loaddata' );
	},

	checkDocumentState: function() {
		this.storeSelection();
		
		
		if( ! this.documentState ) {
			this.documentState = {
				'tBold': false,
				'tItalic': false,
				'tUnderline': false,
				'tJustifyLeft': false,
				'tJustifyCenter': false,
				'tJustifyRight': false,
				'tJustifyFull': false,
				'tOrderedList': false,
				'tUnorderedList': false,
				'tIndent': false,
				'tOutdent': false,
				'spanState': false,
				'spanElement': null,
				'linkState': false,
				'linkElement': null,
				'imageState': false,
				'imageElement': null,
				'tableState': false,
				'tableElement': null,
				'tableRowState': false,
				'tableRowElement': null,
				'tableCellState': false,
				'tableCellElement': null
			};
		}
		
		var doc = this.element.contentDocument;
		
		
		
		var currentDocState = {};
		var tBold = false, tItalic = false, tUnderline = false, tJustifyLeft = false, tJustifyCenter = false, tJustifyRight = false, tJustifyFull = false, tOrderedList = false, tUnorderedList = false, tIndent = false, tOutdent = false;
		
		try {
			tBold = doc.queryCommandState( 'bold' );
			tItalic = doc.queryCommandState( 'italic' );
			tUnderline = doc.queryCommandState( 'underline' );
			tJustifyLeft = doc.queryCommandState( 'justifyleft' );
			tJustifyCenter = doc.queryCommandState( 'justifycenter' );
			tJustifyRight = doc.queryCommandState( 'justifyright' );
			tJustifyFull = doc.queryCommandState( 'justifyfull' );
			tOrderedList = doc.queryCommandState( 'insertorderedlist' );
			tUnorderedList = doc.queryCommandState( 'insertunorderedlist' );
			tIndent = doc.queryCommandState( 'indent' );
			tOutdent = doc.queryCommandState( 'outdent' );
		}
		catch( e ) {
		}
		
		
		
		var spanState = false, spanAttr = [], spanElement = null, linkState = false, linkElement = null, linkAttr = [], imageState = false, imageElement = null, imageAttr = [];
		var tableState = false, tableElement = null, tableAttr = [], tableRowState = false, tableRowElement = null, tableRowAttr = [], tableCellState = false, tableCellElement = null, tableCellAttr = [];
		
		try {
			var el = this.htmlEditor.getSelectionContainer();
			
			if( el.tagName == 'SPAN' && this.elementIsEditable(el) ) {
				spanState = true;
				spanElement = el;
				spanAttr = [ el.className ];
			}
			
			
			var a = this.htmlEditor.getSelectedElement( 'href' );
			
			if( a && this.elementIsEditable(a) ) {
				linkState = true;
				linkElement = a;
				linkAttr = [ a.getAttribute('href'), a.target, a.className ];
			}
	
	
			var img = this.htmlEditor.getSelectedElement( 'IMG' );
			
			if( img && this.elementIsEditable(img) ) {
				imageState = true;
				imageElement = img;
				imageAttr = [ img.getAttribute('src'), img.width, img.height, img.align, img.className, img.alt ];
			}
			
	
			var el = this.htmlEditor.getSelectionContainer();
			var table = null, tr = null, td = null;
			
			while( el && el.tagName != 'BODY' ) {
				if( el.tagName == 'TABLE' && ! table )
					table = el;
				if( el.tagName == 'TR' && ! tr )
					tr = el;
				if( el.tagName == 'TD' && ! td )
					td = el;
				
				el = el.parentNode;
			}
			
			if( table && this.elementIsEditable(table) ) {
				tableState = true;
				tableElement = table;
				tableAttr = [ table.style.width, table.align, table.className ];
			}
	
			if( tr && this.elementIsEditable(tr) ) {
				tableRowState = true;
				tableRowElement = tr;
				tableRowAttr = [ tr.style.height, tr.className ];
			}
			
			if( td && this.elementIsEditable(td) ) {
				tableCellState = true;
				tableCellElement = td;
				tableCellAttr = [ td.style.width, td.style.height, td.vAlign, td.className ];
			}
		}
		catch( e ) {
		}
	
		
		if( tBold != this.documentState.tBold ) {
			this.fireEvent( 'state_bold', [ tBold ] );
		}
		this.documentState.tBold = tBold;
		
		if( tItalic != this.documentState.tItalic ) {
			this.fireEvent( 'state_italic', [ tItalic ] );
		}
		this.documentState.tItalic = tItalic;
		
		if( tUnderline != this.documentState.tUnderline ) {
			this.fireEvent( 'state_underline', [ tUnderline ] );
		}
		this.documentState.tUnderline = tUnderline;
		
		if( tJustifyLeft != this.documentState.tJustifyLeft ) {
			this.fireEvent( 'state_justifyleft', [ tJustifyLeft ] );
		}
		this.documentState.tJustifyLeft = tJustifyLeft;
		
		if( tJustifyCenter != this.documentState.tJustifyCenter ) {
			this.fireEvent( 'state_justifycenter', [ tJustifyCenter ] );
		}
		this.documentState.tJustifyCenter = tJustifyCenter;
		
		if( tJustifyRight != this.documentState.tJustifyRight ) {
			this.fireEvent( 'state_justifyright', [ tJustifyRight ] );
		}
		this.documentState.tJustifyRight = tJustifyRight;
		
		if( tJustifyFull != this.documentState.tJustifyFull ) {
			this.fireEvent( 'state_justifyfull', [ tJustifyFull ] );
		}
		this.documentState.tJustifyFull = tJustifyFull;
		
		if( tOrderedList != this.documentState.tOrderedList ) {
			this.fireEvent( 'state_insertorderedlist', [ tOrderedList ] );
		}
		this.documentState.tOrderedList = tOrderedList;
		
		if( tUnorderedList != this.documentState.tUnorderedList ) {
			this.fireEvent( 'state_insertunorderedlist', [ tUnorderedList ] );
		}
		this.documentState.tUnorderedList = tUnorderedList;
		
		if( tIndent != this.documentState.tIndent ) {
			this.fireEvent( 'state_indent', [ tIndent ] );
		}
		this.documentState.tIndent = tIndent;
		
		if( tOutdent != this.documentState.tOutdent ) {
			this.fireEvent( 'state_outdent', [ tOutdent ] );
		}
		this.documentState.tOutdent = tOutdent;
	
		if( spanState != this.documentState.spanState || spanElement != this.documentState.spanElement ) {
			this.fireEvent( 'state_span', [ spanState ] );
			this.fireEvent( 'attr_span', spanAttr );
		}
		this.documentState.spanState = spanState;
		this.documentState.spanElement = spanElement;
	
		if( linkState != this.documentState.linkState || linkElement != this.documentState.linkElement ) {
			this.fireEvent( 'state_link', [ linkState ] );
			this.fireEvent( 'attr_link', linkAttr );
		}
		this.documentState.linkState = linkState;
		this.documentState.linkElement = linkElement;
	
		if( imageState != this.documentState.imageState || imageElement != this.documentState.imageElement ) {
			this.fireEvent( 'state_img', [ imageState ] );
			this.fireEvent( 'attr_img', imageAttr );
		}
		this.documentState.imageState = imageState;
		this.documentState.imageElement = imageElement;
	
		if( tableState != this.documentState.tableState || tableElement != this.documentState.tableElement ) {
			this.fireEvent( 'state_table', [ tableState ] );
			this.fireEvent( 'attr_table', tableAttr );
		}
		this.documentState.tableState = tableState;
		this.documentState.tableElement = tableElement;
	
		if( tableRowState != this.documentState.tableRowState || tableRowElement != this.documentState.tableRowElement ) {
			this.fireEvent( 'state_tr', [ tableRowState ] );
			this.fireEvent( 'attr_tr', tableRowAttr );
		}
		this.documentState.tableRowState = tableRowState;
		this.documentState.tableRowElement = tableRowElement;
	
		if( tableCellState != this.documentState.tableCellState || tableCellElement != this.documentState.tableCellElement ) {
			this.fireEvent( 'state_td', [ tableCellState ] );
			this.fireEvent( 'attr_td', tableCellAttr );
		}
		this.documentState.tableCellState = tableCellState;
		this.documentState.tableCellElement = tableCellElement;
		
		SiteFusion.Comm.QueueFlush();
	},

	storeSelection: function() {
		var sel = this.textEditor.selection;
		this.storedSelection = [];
		
		for( var n = 0; n < sel.rangeCount; n++ ) {
			this.storedSelection.push( sel.getRangeAt(n) );
		}
	},

	restoreSelection: function() {
		var sel = this.textEditor.selection;
		sel.removeAllRanges();
		
		for( var n = 0; n < this.storedSelection.length; n++ ) {
			sel.addRange( this.storedSelection[n] );
		}
	},

	createLink: function( href, target, class ) {
		this.restoreSelection();
		
		var a, newel = false;
		
		if(! (a = this.htmlEditor.getSelectedElement( 'href' )) ) {
			a = this.element.contentDocument.createElement( 'a' );
			newel = true;
		}
		
		a.href = href;
		a.target = target;
		a.className = class;
		
		if( newel )
			this.htmlEditor.insertLinkAroundSelection( a );
	},

	removeElement: function( tagName ) {
		this.restoreSelection();
		
		var el;
		
		if(! (el = this.htmlEditor.getSelectedElement( tagName )) )
			return;
		
		while( el.hasChildNodes() ) {
			var c = el.removeChild( el.lastChild );
			el.parentNode.insertBefore( c, el );
		}
		
		el.parentNode.removeChild( el );
	},

	insertImage: function( src, width, height, align, alt ) {
		this.restoreSelection();
		
		var img, newel = false;
		
		if(! (img = this.htmlEditor.getSelectedElement( 'IMG' )) ) {
			var html = '<img src="'+src+'" border="0" style="width: '+width+'px; height: '+height+'px;"'+(align != null ? ' align="'+align+'"' : '')+' alt="'+(alt != null ? alt:'')+'">';
			this.htmlEditor.insertHTML( html );
		}
		else {
			if( src != null )
				img.src = src;
			if( width != null )
				img.style.width = width + 'px';
			if( height != null )
				img.style.height = height + 'px';
			if( align != null )
				img.align = align;
			if( alt != null )
				img.alt = alt;
		
			img.setAttribute( 'border', '0' );
		}
		
		var oThis = this;
		window.setTimeout( function() { oThis.checkDocumentState(); }, 200 );
	},

	removeTableRow: function() {
		this.restoreSelection();
		
		var currentRow = this.getNearestElement( 'TR' );
		var table = this.getNearestElement( 'TABLE' );
		table.deleteRow( currentRow.rowIndex );
	},

	insertTableRow: function() {
		this.restoreSelection();
		
		var currentRow = this.getNearestElement( 'TR' );
		var table = this.getNearestElement( 'TABLE' );
		
		var row = table.insertRow( currentRow.rowIndex );
		var cell;
		
		for( var n = 0; n < currentRow.cells.length; n++ ) {
			cell = row.insertCell(-1);
			cell.innerHTML = '&nbsp;';
		}
	},

	removeTable: function() {
		this.restoreSelection();
		
		var table = this.getNearestElement( 'TABLE' );
		table.parentNode.removeChild( table );
	},

	insertHTML: function( html ) {
		this.restoreSelection();
		
		this.htmlEditor.insertHTML( ctrlUnHtml(html) );
	},

	elementSetAttribute: function( element, attr, value ) {
		var el;
		
		if(! (el = this.getNearestElement( element )) )
			return;
		
		el.setAttribute( attr, value );
	},

	elementSetStyle: function( element, prop, value ) {
		var el;
		
		if(! (el = this.getNearestElement( element )) )
			return;
		
		el.style[prop] = value;
	},

	setTextClass: function( className ) {
		var el = this.getNearestElement( 'SPAN' );
		
		if( className == '' ) {
			if( (!el) || el.tagName != 'SPAN' )
				return;
			
			while( el.hasChildNodes() ) {
				var c = el.removeChild( el.lastChild );
				el.parentNode.insertBefore( c, el );
			}
			
			el.parentNode.removeChild( el );
		}
		else {
			if( (!el) || el.tagName != 'SPAN' ) {
				var content = this.textEditor.selection.toString();
				if(! content.length )
					return;
				
				el = this.element.contentDocument.createElement( 'SPAN' );
				el.innerHTML = content;
				this.htmlEditor.insertElementAtSelection( el, true );
			}
			
			el.className = className;
		}
	},

	getNearestElement: function( tag ) {
		this.restoreSelection();
		
		tag = tag.toUpperCase();
		var el;
		
		if(! (el = this.htmlEditor.getSelectedElement( tag )) ) {
			el = this.htmlEditor.getSelectionContainer();
			
			while( el && el.tagName != 'BODY' ) {
				if( el.tagName == tag )
					break;
				
				el = el.parentNode;
			}
		}
		
		return el ? el : null;
	},

	elementIsEditable: function( el ) {
		var found = false;
		
		while( el && el.tagName != "BODY" ) {
			if( el == this.editorElement ) {
				found = true;
				break;
			}
			el = el.parentNode;
		}
		
		return found;
	},

	elementSetClassname: function( tag, className ) {
		var el = this.getNearestElement( tag );
		
		if(! el )
			return;
		
		el.className = className;
	}
} );
