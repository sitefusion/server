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

SiteFusion.Classes.Editor = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULEditor',
	
	initialize: function( win ) {
		this.element = win.createElement( 'editor' );
		this.element.sfNode = this;
		
		this.hostWindow = win;
		this.storedSelection = [];
		
		this.setEventHost( [
			'before_initialize',
			'after_loaddata',
			'initialized',
			'madeEditable',
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
		this.eventHost.madeEditable.msgType = 0;
		this.eventHost.yield.msgType = 1;
		this.eventHost.document_state.msgType = 0;
	},
	
	init: function() {
		this.element.setAttribute( 'src', 'about:blank' );
		this.fireEvent( 'initialized' );
	},
	
	setValue: function( html ) {
		if (!this.textEditor) {
			SiteFusion.consoleMessage('this.textEditor is undefined in setValue()');
			return;
		}
		
		var textEditor = this.textEditor;
	    textEditor.enableUndo(false);
	    textEditor.selectAll();
	    textEditor.deleteSelection(textEditor.eNone);
	    textEditor.enableUndo(true);
	    textEditor.resetModificationCount();
		if (html && typeof this.element.contentDocument != 'undefined' )
			this.element.contentDocument.execCommand("inserthtml", false, html);
	},
	
	makeEditable: function() {
		if (typeof this.element.contentDocument == 'undefined' ) {
			SiteFusion.consoleMessage('this.element.contentDocument is undefined in makeEditable()');
			return;
		}
		this.element.contentDocument.designMode = "on";
		this.fireEvent( 'madeEditable' );
	},
	
	editorLoaded: function() {
		var oThis = this;
		var func = function() { oThis.checkDocumentState(); };
		
		if (!this.editorElement)
			this.editorElement = this.element;
			
		if (typeof this.element.contentDocument == 'undefined' ){
			SiteFusion.consoleMessage('this.element.contentDocument is undefined in editorLoaded()');
			return;
		}
			
		this.element.contentDocument.onmouseup = function() { window.setTimeout( func, 1 ); };
		this.element.contentDocument.onkeyup = func;
		this.element.contentDocument.onpaste = this.pasteHandler;
		
		//dirty hack for Mozilla scroll-in-contentEditable-bug	
		//todo: keep track of Mozilla development at: https://developer.mozilla.org/en/DOM/Selection/modify
		this.element.contentDocument.onkeypress = function(event) {
				if (event.keyCode >= 37 && event.keyCode <= 40) {
					var selection = oThis.element.contentWindow.getSelection();
					switch (event.keyCode) {
						case 37:
						//left arrow
						selection.modify((event.shiftKey ? "extend": "move"), "backward", "character");
						break;
						case 38:
						//up arrow
						selection.modify((event.shiftKey ? "extend": "move"), "backward", "line");
						break;
						case 39:
						//right arrow
						selection.modify((event.shiftKey ? "extend": "move"), "forward", "character");
						break;
						case 40:
						//down arrow
						selection.modify((event.shiftKey ? "extend": "move"), "forward", "line");
						break;
					}
					return false;
				}
		};
		

		this.htmlEditor = this.element.getHTMLEditor( this.element.contentWindow );
		this.textEditor = this.element.getEditor( this.element.contentWindow );
		
		this.htmlEditor.returnInParagraphCreatesNewParagraph = false;
		
		this.editorElement.focus();
		if (this.editorElement == this.element)
		{
			if (typeof this.textEditor == 'undefined') {
		 	 	SiteFusion.consoleMessage('this.textEditor is undefined in editorLoaded()');
		 	 	return;
		     }
			var body = this.element.contentDocument.getElementsByTagName("body")[0];
			this.textEditor.selection.collapse( body, 1 );
		}
		else {
		 	 if (typeof this.textEditor == 'undefined') {
		 	 	SiteFusion.consoleMessage('this.textEditor is undefined in editorLoaded()');
		 	 	return;
		     }
			 this.textEditor.selection.collapse( this.editorElement, 1 );
		}
			
		this.checkDocumentState();
	
		this.fireEvent( 'after_loaddata' );
	},
	
	disableInput: function(state)
	{
		if (typeof this.element.contentDocument == 'undefined' ){
			SiteFusion.consoleMessage('this.element.contentDocument is undefined in disableInput()');
			return;
		}
		this.element.contentDocument.execCommand('contentReadOnly',false, state);
	},
	
	yield: function() {
		if(! this.editorElement ){
			SiteFusion.consoleMessage('this.editorElement is not set in in yield()');
			return;
		}
		if (typeof this.element.contentDocument == 'undefined' ){
			SiteFusion.consoleMessage('this.element.contentDocument is undefined in in yield()');
			return;
		}
		this.fireEvent( 'yield', [ this.element.contentDocument.body.innerHTML + '' ] );
	},
	
	storeSelection: function() {
		if (typeof this.textEditor == 'undefined') {
		 	 	SiteFusion.consoleMessage('this.textEditor is undefined in storeSelection()');
		 	 	return;
		}
		var sel = this.textEditor.selection;
		this.storedSelection = [];
		
		for( var n = 0; n < sel.rangeCount; n++ ) {
			this.storedSelection.push( sel.getRangeAt(n) );
		}
	},

	restoreSelection: function() {
		if (typeof this.textEditor == 'undefined') {
		 	 	SiteFusion.consoleMessage('this.textEditor is undefined in restoreSelection()');
		 	 	return;
		}
		var sel = this.textEditor.selection;
		sel.removeAllRanges();
		
		for( var n = 0; n < this.storedSelection.length; n++ ) {
			sel.addRange( this.storedSelection[n] );
		}
	},

	createLink: function( href, target, className ) {
		this.restoreSelection();
		
		var a, newel = false;
		
		if(! (a = this.htmlEditor.getSelectedElement( 'href' )) ) {
			a = this.element.contentDocument.createElement( 'a' );
			newel = true;
		}
		
		a.href = href;
		a.target = target;
		a.className = className;
		
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

		if ( width === parseInt( width ) )
			width += 'px';

		if ( height === parseInt( height ) )
			height += 'px';
		
		var img, newel = false;
		if ( img = this.htmlEditor.getSelectedElement( 'IMG' ) ) {
			if( src != null )
				img.src = src;
			
			if( width != null ) {
				img.style.width = width;
				img.setAttribute( 'width', width );
			}

			if( height != null ) {
				img.style.height = height;
				img.setAttribute( 'height', height );
			}

			if( align != null )
				img.align = align;
			if( alt != null )
				img.alt = alt;
		
			img.setAttribute( 'border', '0' );
		} else {
			var html = '<img src="'+src+'" border="0" style="width: '+width+'; height: '+height+';"'+(align != null ? ' align="'+align+'"' : '')+' alt="'+(alt != null ? alt:'')+'">';
			this.htmlEditor.insertHTML( html );
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

	insertHeading: function(elementName) {
		this.htmlEditor.insertHTML( "<" + elementName + ">heading</" + elementName + ">");
	},
	
	insertHTML: function( html ) {
		this.restoreSelection();
		
		this.htmlEditor.insertHTML( html );
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
				if (typeof this.textEditor == 'undefined') {
		 	 		SiteFusion.consoleMessage('this.textEditor is undefined in setTextClass()');
		 	 		return;
				}
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
	
	setTextID: function( id ) {
		var el = this.getNearestElement( 'SPAN' );
		
		if( id == '' ) {
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
				if (typeof this.textEditor == 'undefined') {
		 	 		SiteFusion.consoleMessage('this.textEditor is undefined in setTextClass()');
		 	 		return;
				}
				var content = this.textEditor.selection.toString();
				if(! content.length )
					return;
				
				el = this.element.contentDocument.createElement( 'SPAN' );
				el.innerHTML = content;
				this.htmlEditor.insertElementAtSelection( el, true );
			}
			
			el.id = id;
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
		
		if (typeof this.element.contentDocument == 'undefined' ){
			SiteFusion.consoleMessage('this.element.contentDocument is undefined in in checkDocumentState()');
			return;
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
				spanAttr = [ el.className, el.id ];
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
				imageAttr = [ img.getAttribute('src'), img.width, img.height, img.align, img.className, img.getAttribute('alt') ];
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
	
	pasteHandler: function (e) {
		SiteFusion.consoleMessage('init paste!\n');

		var xferableplain = Components.classes["@mozilla.org/widget/transferable;1"].createInstance(Components.interfaces.nsITransferable);
		var xferablehtml = Components.classes["@mozilla.org/widget/transferable;1"].createInstance(Components.interfaces.nsITransferable);
		var xferablemshtml = Components.classes["@mozilla.org/widget/transferable;1"].createInstance(Components.interfaces.nsITransferable);
		
		var unicodestring = Components.classes["@mozilla.org/supports-string;1"].createInstance(Components.interfaces.nsISupportsString);
		var clipboard = Components.classes["@mozilla.org/widget/clipboard;1"].getService(Components.interfaces.nsIClipboard);

		var str       = new Object();
		var strLength = new Object();
		var strHtml       = new Object();
		var strHtmlLength = new Object();
		var strMSHtml       = new Object();
		var strMSHtmlLength = new Object();
				
		var pastetext = "";
		var pastehtml = "";
		var selectedHtml = "";
		
		var hasText = true;
		var hasMicrosoftHtml = true;
		var hasHtml = true;
		var htmlIsOfficeHtml = false;
		
		//get text
		xferableplain.addDataFlavor("text/unicode");
		clipboard.getData(xferableplain, Components.interfaces.nsIClipboard.kGlobalClipboard);
		
		//get html
		xferablehtml.addDataFlavor("text/html");
		clipboard.getData(xferablehtml, Components.interfaces.nsIClipboard.kGlobalClipboard);
		
		//get microsoft html (windows only)
		xferablemshtml.addDataFlavor("application/x-moz-nativehtml");
		clipboard.getData(xferablemshtml, Components.interfaces.nsIClipboard.kGlobalClipboard);

		try {
			xferablemshtml.getTransferData("application/x-moz-nativehtml", strMSHtml, strMSHtmlLength);
			SiteFusion.consoleMessage("parsing Microsoft XML...\n");
		}
		catch(e)
		{
			SiteFusion.consoleMessage("Microsoft XML not found on clipboard\n");
			hasMicrosoftHtml = false;
		}
		
		try {
			xferablehtml.getTransferData("text/html", strHtml, strHtmlLength);
			SiteFusion.consoleMessage("parsing html...\n");
		}
		catch(e)
		{
			SiteFusion.consoleMessage("html not found on clipboard\n");
			hasHtml = false;
		}
		
		
		try {
			xferableplain.getTransferData("text/unicode", str, strLength);
			SiteFusion.consoleMessage("parsing plaintext...\n");
		}
		catch(e)
		{
			SiteFusion.consoleMessage("plaintext not found on clipboard\n");
			hasText = false;
		}
		
	    if (str && hasText) {
	    	str = str.value.QueryInterface(Components.interfaces.nsISupportsString);
	    	pastetext = str.data.substring(0, strLength.value / 2);
	    }

	    if (strHtml && hasHtml) {
	    	strHtml = strHtml.value.QueryInterface(Components.interfaces.nsISupportsString);
	    	pastehtml = strHtml.data.substring(0, strHtmlLength.value / 2);
	    	
			//office 2004 mac
	    	htmlIsOfficeHtml = ((pastehtml.indexOf("Version:1.0") != -1) && (pastehtml.indexOf("StartHTML:") != -1) && (pastehtml.indexOf("EndHTML:") != -1));
			//office 2008 mac
			if (!htmlIsOfficeHtml) htmlIsOfficeHtml = (pastehtml.indexOf("urn:schemas-microsoft-com:office:office") != -1);
	    }
	
		if (strMSHtml && strMSHtmlLength && strMSHtml.value && !hasHtml) {
			
			try {
				
			strMSHtml = new String(strMSHtml.value.QueryInterface(Components.interfaces.nsISupportsCString));
			
			//SiteFusion.consoleMessage (utfConv.ConvertToUnicode(strMSHtml));
			//var utfStream = utfConv.convertToInputStream( strMSHtml );
			
			//var avb = utfStream.available();
			//var str = utfStream.read(avb);
			//SiteFusion.consoleMessage(str);
			/*
			var scs = Components.classes["@mozilla.org/streamConverters;1"].getService( Components.interfaces.nsIStreamConverterService );
			var compressor = scs.asynComponents.classesonvertData( "uncompressed", "deflate", this, null );
			
			var pump = Components.classes["@mozilla.org/network/input-stream-pump;1"].createInstance( Components.interfaces.nsIInputStreamPump );
			pump.init( utfStream, -1, -1, 0, 0, true );
			pump.asyncRead( compressor, null );
			*/
			
			//strMSHtml = utfConv.ConvertToUnicode(strMSHtml);
			}
			catch(e) {SiteFusion.consoleMessage(e); }
			
	    	var tempmshtml = strMSHtml;
	    	
	    	if (tempmshtml.indexOf("urn:schemas-microsoft-com:office:office") == -1) {
		    	var input = tempmshtml.split( /\r?\n/ );
		    	var data = {};
		    	while( input[0].match( /[a-z]\:.+/i ) ) {
		    		var part = input[0].split( ':' );
		    		data[part[0]] = part[1];
		    		input.shift();
		    	}
		    	if (data.StartFragment && data.EndFragment)
		    	{
		    		var selStart = parseInt(data.StartFragment.replace(/^0+/,''));
		    		var selEnd = parseInt(data.EndFragment.replace(/^0+/,''));
		    	
		   	 		var utfConv = Components.classes["@mozilla.org/intl/scriptableunicodeconverter"].createInstance(Components.interfaces.nsIScriptableUnicodeConverter);
						utfConv.charset = "UTF-8";
		    		selectedHtml = utfConv.ConvertToUnicode(tempmshtml.substr( selStart, selEnd-selStart ));
		    	}
		    }
		}

		if (selectedHtml) {
			//check for incomplete table structure
		    if (selectedHtml.substr(0,3).toLowerCase() == "<tr" && selectedHtml.substr(selectedHtml.length - 5, 5).toLowerCase() == "</tr>")
		    	var processedHtml = "<table>" + selectedHtml + "</table>";
		    else
		    	var processedHtml = selectedHtml;
	    	
			unicodestring.data = processedHtml;
			xferablehtml.setTransferData("text/html", unicodestring, processedHtml.length * 2);

			clipboard.setData(xferablehtml, null, Components.interfaces.nsIClipboard.kGlobalClipboard);
			dump ("setData() mshtml: " + unicodestring + "\n");
		}
		else if (strHtml && hasHtml && !htmlIsOfficeHtml)
		{
			//check for incomplete table structure
		    if (pastehtml.substr(0,3).toLowerCase() == "<tr" && pastehtml.substr(pastehtml.length - 5, 5).toLowerCase() == "</tr>")
		    	var processedHtml = "<table>" + pastehtml + "</table>";
		    else
		    	var processedHtml = pastehtml;
	    	
			unicodestring.data = processedHtml;
			xferablehtml.setTransferData("text/html", unicodestring, processedHtml.length * 2);

			clipboard.setData(xferablehtml, null, Components.interfaces.nsIClipboard.kGlobalClipboard);
			dump ("setData() html: " + processedHtml + "\n");
		}
		else
		{
			//pastetext = pastetext.replace(new RegExp("([^a-zA-Z0-9\t\n\v\f\r \x21\x22\x23\x24\x25\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f\x3a\x3b\x3d\x3f\x40\x5b\x5c\x5d\x5e\x5f\x60\x7b\x7c\x7d\x7e])", "g"), this.editorObj.convertHelper);
			unicodestring.data = pastetext;
			xferableplain.setTransferData("text/unicode ", unicodestring, pastetext.length * 2);
			clipboard.setData(xferableplain, null, Components.interfaces.nsIClipboard.kGlobalClipboard);
			dump ("setData() plaintext\n");
		}
		SiteFusion.consoleMessage("End of clipboard parsing!\n");
	}
});

SiteFusion.Classes.LayoutEditor = Class.create( SiteFusion.Classes.Editor, {
	sfClassName: 'XULLayoutEditor',

	loadLayoutHTML: function( html, frameid ) {
		if (typeof this.element.contentDocument == 'undefined' ) {
			SiteFusion.consoleMessage('this.element.contentDocument is undefined in loadLayoutHTML()');
			return;
		}
		this.element.contentWindow.document.write( html );	
		
		
		this.editorElement = this.element.contentDocument.getElementById( frameid );
	},

	loadBody: function( html ) {
		if( ! this.element ) {
			this._loadBody = html;
			return;
		}
		if (typeof this.element.contentDocument == 'undefined' ) {
			SiteFusion.consoleMessage('this.element.contentDocument is undefined in loadBody()');
			return;
		}
		if (!this.editorElement ) {
			SiteFusion.consoleMessage('this.editorElement is not set in loadBody()');
			return;
		}
				
		this.editorElement.innerHTML = html;
		
		this.makeEditable();
	},

	makeEditable: function() {
		if (typeof this.element.contentDocument == 'undefined' ) {
			SiteFusion.consoleMessage('this.element.contentDocument is undefined in makeEditable()');
			return;
		}
		this.element.contentDocument.body.contentEditable = false;
		this.editorElement.contentEditable = true;
		this.fireEvent( 'madeEditable' );
	},
	
	disableInput: function(state)
	{
		this.editorElement.contentEditable = !state;
	},
	
	yield: function() {
		if(! this.editorElement ) {
			SiteFusion.consoleMessage('this.element.contentDocument is undefined in yield()');
			return;
		}
		this.fireEvent( 'yield', [ this.editorElement.innerHTML + '' ] );
	}
} );
