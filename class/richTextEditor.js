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

SiteFusion.Classes.RichTextEditor = Class.create( SiteFusion.Classes.Editor, {
	sfClassName: 'XULRichTextEditor',
	
	initialize: function( win ) {
		this.element = win.createElement( 'editor' );
		this.element.sfNode = this;
		var oThis = this;
		
		this.hostWindow = win;
		
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
			'attr_span',
			'madeEditable'
		] );
		
		this.eventHost.initialized.msgType = 0;
		this.eventHost.madeEditable.msgType = 0;
		this.eventHost.yield.msgType = 1;
	},
	
	editorLoaded: function() {

		var oThis = this;
		var func = function() { oThis.checkDocumentState(); };
		
		if (!this.editorElement)
			this.editorElement = this.element;
		
		this.element.contentDocument.onmouseup = function() { window.setTimeout( func, 1 ); };
		this.element.contentDocument.onkeyup = func;
		
		this.htmlEditor = this.element.getHTMLEditor( this.element.contentWindow );
		this.textEditor = this.element.getEditor( this.element.contentWindow );
		
		this.htmlEditor.returnInParagraphCreatesNewParagraph = false;
		
		this.editorElement.focus();
		if (this.editorElement == this.element)
		{
			var body = this.element.contentDocument.getElementsByTagName("body")[0];
			this.textEditor.selection.collapse( body, 1 );
		}
		else this.textEditor.selection.collapse( this.editorElement, 1 );
			
		this.element.contentWindow.onfocus= function() {
			var myKey = oThis.hostWindow.createElement( 'key' );
			oThis.element.myKeySet = oThis.hostWindow.createElement( 'keyset' );
			myKey.setAttribute('key', "V");
			myKey.setAttribute('modifiers', "accel");
	
			myKey.sfNode = oThis;
			
			myKey.setAttribute("oncommand", "sfRootWindow.windowObject.SiteFusion.Registry[" + oThis.cid + "].textEditor.QueryInterface(Components.interfaces.nsIHTMLEditor).pasteNoFormatting(1)");
	
			oThis.element.myKeySet.appendChild(myKey);
			oThis.element.appendChild(oThis.element.myKeySet);
		}
		this.element.contentWindow.onblur= function() {
			oThis.element.removeChild(oThis.element.myKeySet);
		}
		
		this.checkDocumentState();
	
		this.fireEvent( 'after_loaddata' );
	},
	setValue: function( html ) {
		var textEditor = this.textEditor;
	    textEditor.enableUndo(false);
	    textEditor.selectAll();
	    textEditor.deleteSelection(textEditor.eNone);
	    textEditor.enableUndo(true);
	    textEditor.resetModificationCount();
	    textEditor.document.title = "";
	    if (html)
			this.element.contentDocument.execCommand("inserthtml", false, html);
	},
  	setBodyStyle: function (prop, value) {
		var oThis = this;
		setTimeout(function() { oThis.element.contentDocument.body.style[prop] = value; }, 100);
	}
});