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
	
//this callback object is used to determine when the editor is not busy anymore	
var EditorListener = {
  _handler : null,
  _enabled : true,
  _timeout : -1,
  _batch : false,
  _editor : null,

  init : function(aHandler) {
    this._handler = aHandler;
  },

  attach : function(aEditor) {
    if (this._editor) {
      this._editor.transactionManager.RemoveListener(this);
    }

    this._editor = aEditor;

    if (this._editor) {
      this._editor.transactionManager.AddListener(this);
    }
  },

  get enabled() {
    return this._enabled;
  },
  set enabled(aEnabled) {
    this._enabled = aEnabled;
  },

  refresh : function() {
    if (!this._enabled || this._batch)
      return;

    if (this._timeout != -1)
      window.clearTimeout(this._timeout);

    var self = this;
    this._timeout = window.setTimeout(function() { self._handler(); }, 1000);
  },

  didDo : function(aManager, aTransaction, aResult) {
    this.refresh();
  },

  didUndo : function(aManager, aTransaction, aResult) {
    this.refresh();
  },

  didRedo : function(aManager, aTransaction, aResult) {
    this.refresh();
  },

  didBeginBatch : function(aManager, aResult) {
    this.batch = true;
  },

  didEndBatch : function(aManager, aResult) {
    this.batch = false;
    this.refresh();
  },

  didMerge : function(aManager, aTopTransaction, aTransactionToMerge, aDidMerge, aResult) {
    this.refresh();
  },

  willDo : function(aManager, aTransaction) { return false; },
  willUndo : function(aManager, aTransaction) { return false; },
  willRedo : function(aManager, aTransaction) { return false; },
  willBeginBatch : function(aManager) { return false; },
  willEndBatch : function(aManager) { return false; },
  willMerge : function(aManager, aTopTransaction, aTransactionToMerge) { return false; }
};

SiteFusion.Classes.CodeEditor = Class.create( SiteFusion.Classes.Editor, {
	sfClassName: 'XULCodeEditor',
	
	initialize: function( win ) {
		this.element = win.createElement( 'editor' );
		this.element.sfNode = this;
		var oThis = this;
		EditorListener.init(function() { oThis.handleEditorChange(); });
		
		this.hostWindow = win;
		
		this.setEventHost( [
			'before_initialize',
			'after_loaddata',
			'initialized',
			'madeEditable',
			'yield'
		] );
		
		this.eventHost.initialized.msgType = 0;
		this.eventHost.madeEditable.msgType = 0;
		this.eventHost.yield.msgType = 1;
	},
		
	handleEditorChange: function()
	{
		
	},
	
	makeEditable: function() {
		this.element.makeEditable("plaintext", false);
		this.fireEvent( 'madeEditable' );
	},
	editorLoaded: function(contents) {

		if (!this.editorElement)
			this.editorElement = this.element;

		var editor = null;
		var textEditor = null;

		var oThis = this;
		
		var getEditorFunc = function() {
			setTimeout(function() {

				if (oThis.element.editingSession) {
					oThis.element.editingSession.makeWindowEditable(oThis.element.contentWindow, "plaintext", false, true, false);
					textEditor = oThis.element.editingSession.getEditorForWindow(oThis.element.contentWindow);
					if (textEditor)
						editor = textEditor.QueryInterface(Components.interfaces.nsIHTMLEditor);

					if (editor && textEditor) {			
				        oThis.htmlEditor = editor;
						
						oThis.textEditor = textEditor.QueryInterface(Ci.nsIPlaintextEditor);;
						oThis.textEditor.enableUndo(true);
						oThis.textEditor.rootElement.style.fontFamily = "-moz-fixed";
						oThis.textEditor.rootElement.style.fontSize = "10pt";
						oThis.textEditor.rootElement.style.backgroundColor = "white";
						oThis.textEditor.rootElement.style.whiteSpace = "nowrap";
						oThis.textEditor.rootElement.style.marginLeft = 5;
						
						oThis.element.contentWindow.onfocus= function() {
							var myKey = oThis.hostWindow.createElement( 'key' );
							oThis.element.myKeySet = oThis.hostWindow.createElement( 'keyset' );
							myKey.setAttribute('key', "V");
							myKey.setAttribute('modifiers', "accel");
					
							myKey.sfNode = oThis;
							
							myKey.setAttribute("oncommand", "sfRootWindow.windowObject.SiteFusion.Registry[" + oThis.cid + "].textEditor.QueryInterface(Components.interfaces.nsIHTMLEditor).pasteNoFormatting(1)");
					
							oThis.element.myKeySet.appendChild(myKey);
							oThis.element.appendChild(oThis.element.myKeySet);
						}
						oThis.element.contentWindow.onblur= function() {
							oThis.element.removeChild(oThis.element.myKeySet);
						}
						
						//dirty hack for Mozilla scroll-in-contentEditable-bug	
						//todo: keep track of Mozilla development at: https://developer.mozilla.org/en/DOM/Selection/modify
						oThis.element.contentWindow.onkeypress = function(event) {
								if (event.keyCode >= 37 && event.keyCode <= 40) {
									event.preventDefault();
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
								}
						};
						
					    //set language css
					    oThis.applyStyling();
				    
						EditorListener.attach(oThis.textEditor);
						oThis.setSource(contents);
						oThis.fireEvent( 'after_loaddata' );
					} 
				}
				//are we completely ready yet?
				if (!oThis.element.editingSession || !editor || !textEditor) {
					getEditorFunc();
				}
			}
			,20);
			
		};
		getEditorFunc();
	},
  clearSource : function() {
    var textEditor = this.textEditor;
    textEditor.enableUndo(false);
    textEditor.selectAll();
    
    try {
    	textEditor.deleteSelection(textEditor.eNone, true);
    }
    catch (e) {
    	textEditor.deleteSelection(textEditor.eNone);
    }
    
    textEditor.enableUndo(true);
    textEditor.resetModificationCount();

    textEditor.document.title = "";

    this.element.contentWindow.focus();
  },

  getSource : function() {
    var textEditor = this.textEditor;
    var source = textEditor.outputToString("text/plain", 1028);
    return source;
  },

  setSource : function(aText) {
    this.clearSource();

    this.insertText(aText);
  },
  
  applyStyling: function () {
  	this.htmlEditor.replaceHeadContentsWithHTML('<style>body {padding-left:25px;padding-top:0px;background:white;margin-left:32px;font-family:monospace;font-size:13px;background-repeat:repeat-y;background-position:0 3px;	line-height:16px;	height:95%;}P {margin:0;padding:0;border:0;outline:0;display:block;}</style>');	
  	this.element.contentDocument.body.style.backgroundImage = "url('" + this.parseImageURL('/class/res/line-numbers.png') + "')";
  },
	
  insertText : function(aText) {
    this.textEditor.insertText(aText);
  },
  
	yield: function() {
		this.fireEvent( 'yield', [ this.getSource() ] );
	}
});