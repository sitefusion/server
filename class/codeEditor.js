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

Language={};

Language.css = "body {padding-left:25px;padding-top:9px;background:white;margin-left:32px;font-family:monospace;font-size:13px;background-repeat:repeat-y;background-position:0 3px;	line-height:16px;	height:100%;}P {margin:0;padding:0;border:0;outline:0;display:block;}b, i, s, u, a, em, tt, ins, big, cite, strong, var, dfn {text-decoration:none;font-weight:normal;font-style:normal;font-size:13px;}b {color:#000080;}big, big b, big em, big ins, big s, strong i, strong i b, strong i s, strong i u, strong i a, strong i a u, strong i s u {color:gray;font-weight:normal;}s, s b, strong s u, strong s cite {color:#5656fa;font-weight:normal;}strong a, strong a u {color:#006700;font-weight:bold;}em {color:#800080;font-style:normal;}ins {color:#800000;}strong u {color:#7F0055;font-weight:bold;}cite, s cite {color:red;font-weight:bold;}";

Language.syntax = [
	{ input : /(&lt;[^!\?]*?&gt;)/g, output : '<b>$1</b>' },
	{ input : /(&lt;style.*?&gt;)(.*?)(&lt;\/style&gt;)/g, output : '<em>$1</em><em>$2</em><em>$3</em>' },
	{ input : /(&lt;script.*?&gt;)(.*?)(&lt;\/script&gt;)/g, output : '<ins>$1</ins><ins>$2</ins><ins>$3</ins>' },
	{ input : /\"(.*?)(\"|<br>|<\/P>)/g, output : '<s>"$1$2</s>' },
	{ input : /\'(.*?)(\'|<br>|<\/P>)/g, output : '<s>\'$1$2</s>'},
	{ input : /(&lt;\?)/g, output : '<strong>$1' },
	{ input : /(\?&gt;)/g, output : '$1</strong>' },
	{ input : /(&lt;\?php|&lt;\?=|&lt;\?|\?&gt;)/g, output : '<cite>$1</cite>' },
	{ input : /(\$[\w\.]*)/g, output : '<a>$1</a>' },
	{ input : /\b(false|true|and|or|xor|__FILE__|exception|__LINE__|array|as|break|case|class|const|continue|declare|default|die|do|echo|else|elseif|empty|enddeclare|endfor|endforeach|endif|endswitch|endwhile|eval|exit|extends|for|foreach|function|global|if|include|include_once|isset|list|new|print|require|require_once|return|static|switch|unset|use|while|__FUNCTION__|__CLASS__|__METHOD__|final|php_user_filter|interface|implements|extends|public|private|protected|abstract|clone|try|catch|throw|this)\b/g, output : '<u>$1</u>' },
	{ input : /([^:])\/\/(.*?)(<br|<\/P)/g, output : '$1<i>//$2</i>$3' },
	{ input : /([^:])#(.*?)(<br|<\/P)/g, output : '$1<i>#$2</i>$3' },
	{ input : /\/\*(.*?)\*\//g, output : '<i>/*$1*/</i>' },
	{ input : /(&lt;!--.*?--&gt.)/g, output : '<big>$1</big>' }
]
	
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
		//if (this.element.contentWindow)
		//	this.applySyntaxHighlighting();
	},
	
	makeEditable: function() {
		this.element.makeEditable("plaintext", false);
		this.fireEvent( 'madeEditable' );
	},
	editorLoaded: function() {
		
		if (!this.editorElement)
			this.editorElement = this.element;
		
		this.htmlEditor = this.element.getHTMLEditor( this.element.contentWindow );
		 
		this.textEditor = this.element.getEditor(this.element.contentWindow).QueryInterface(Ci.nsIPlaintextEditor);
		this.textEditor.enableUndo(true);
		this.textEditor.rootElement.style.fontFamily = "-moz-fixed";
		this.textEditor.rootElement.style.fontSize = "10pt";
		this.textEditor.rootElement.style.backgroundColor = "white";
		this.textEditor.rootElement.style.whiteSpace = "nowrap";
		this.textEditor.rootElement.style.margin = 5;
		
		var myKey = this.hostWindow.createElement( 'key' );
		var myKeySet = this.hostWindow.createElement( 'keyset' );
		myKey.setAttribute('key', "V");
		myKey.setAttribute('modifiers', "accel");

		myKey.sfNode = this;
		
		myKey.setAttribute("oncommand", "sfRootWindow.windowObject.SiteFusion.Registry[" + this.cid + "].textEditor.QueryInterface(Components.interfaces.nsIHTMLEditor).pasteNoFormatting(1)");

		myKeySet.appendChild(myKey);
		this.element.appendChild(myKeySet);
		
	    //set language css
	    this.applyLanguageCSS();
    
		EditorListener.attach(this.textEditor);
			
		this.fireEvent( 'after_loaddata' );
	},
  clearSource : function() {
    var textEditor = this.textEditor;
    textEditor.enableUndo(false);
    textEditor.selectAll();
    textEditor.deleteSelection(textEditor.eNone);
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
  
  applyLanguageCSS: function () {
  	this.htmlEditor.replaceHeadContentsWithHTML('<style>' + Language.css + '</style>');	
  	this.element.contentDocument.body.style.backgroundImage = 'url(' + this.parseImageURL('/class/res/line-numbers.png') + ')';
  },
  
  applySyntaxHighlighting: function() {
  	this.element.getEditor( this.element.contentWindow ).setShouldTxnSetSelection(true);
  	this.htmlEditor.setShouldTxnSetSelection(false);
		this.htmlEditor.beginTransaction();
		this.preserveSelection();
		var textEditor = this.textEditor;
    var source = this.element.contentDocument.body.innerHTML + '';
    
		for(i=0;i<Language.syntax.length;i++) 
			source = source.replace(Language.syntax[i].input,Language.syntax[i].output);

		this.element.contentDocument.body.innerHTML = source;
		this.htmlEditor.setShouldTxnSetSelection(true);
		this.htmlEditor.endTransaction();
		this.restoreSelection();
  },
  
  preserveSelection: function preserveSelection()
  {
    // let's preserve the caret's position ; after edit changes,
    // the selection is always collapsed

    // we're going to preserve it counting the chars from the
    // beginning of the document up to the caret, just like in
    // a plaintext editor ; BRs count for "\n" so 1 char.
    var range = this.textEditor.selection.getRangeAt(0);
    this.mSelEndContainer   = range.endContainer;
    this.mSelEndOffset      = range.endOffset;
    this.mGlobalOffset = 0; 

    if (this.mSelEndContainer.nodeType == Node.TEXT_NODE)
    {
      if (this.mSelEndContainer.parentNode.nodeName.toLowerCase() != "body")
        this.mSelEndContainer = this.mSelEndContainer.parentNode;
      this.mGlobalOffset = this.mSelEndOffset;
    }
    else
    {
      var children = this.mSelEndContainer.childNodes;
      var l = children.length;
      for (var i = 0; i < l && i < this.mSelEndOffset; i++)
      {
        var child = children.item(i);
        switch (child.nodeType)
        {
          case Node.TEXT_NODE:
            this.mGlobalOffset += child.data.length;
            break;
          case Node.ELEMENT_NODE:
            if (child.nodeName.toLowerCase() == "br")
              this.mGlobalOffset++;
            else
              this.mGlobalOffset += child.textContent.length;
            break;
          default:
            break;
        }
      }
      if (this.mSelEndContainer.nodeName.toLowerCase() == "body")
        return;
    }

    var node = this.mSelEndContainer.previousSibling;
    while (node)
    {
      if (node.nodeName.toLowerCase() == "br")
        this.mGlobalOffset += 1;
      else
        this.mGlobalOffset += node.textContent.length;
      node = node.previousSibling;
    }
  },

  restoreSelection: function restoreSelection()
  {
    // let's restore the caret at our previous position using the
    // char offset we stored in restoreSelection()
    var node = this.element.contentDocument.body.firstChild;
    var offset = 0;

    while (node &&
           ((node.nodeName.toLowerCase() == "br") ? 1 : node.textContent.length) < this.mGlobalOffset)
    {
      //if (node.nodeName.toLowerCase() == "br")
      //  this.mGlobalOffset--;
      //else
      //  this.mGlobalOffset -= node.textContent.length;
      node = node.nextSibling;
    }
    if (node)
    {
      if (node.nodeName.toLowerCase() == "span") 
        this.htmlEditor.selection.collapse(node.firstChild, this.mGlobalOffset);
      else if (node.nodeType == Node.TEXT_NODE) 
      {
        this.htmlEditor.selection.collapse(node, this.mGlobalOffset);
      }
      else // this is a BR element
        if (this.mGlobalOffset)
          this.htmlEditor.setCaretAfterElement(node);
        else
          this.htmlEditor.beginningOfDocument();
    }
  },
  
  getRangeAndCaret : function() {	
		var range = this.element.contentWindow.getSelection().getRangeAt(0);
		var range2 = range.cloneRange();
		var node = range.endContainer;			
		var caret = range.endOffset;
		range2.selectNode(node);	
		return [range2.toString(),caret];
	},
	
  insertText : function(aText) {
    this.textEditor.insertText(aText);
  },
  
	yield: function() {
		this.fireEvent( 'yield', [ this.getSource() ] );
	}
});

SiteFusion.Classes.DiavoloCodeEditor = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULDiavoloCodeEditor',
	
		initialize: function( win ) {
		this.element = win.createElement( 'sourceeditor' );
		this.element.sfNode = this;
		
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
	
	init: function() {
		this.element.setAttribute( 'grammarURL', 'http://sfdev.donnie.lan.thefrontdoor.nl/inc/diavolo/css.xml' );
		this.fireEvent( 'initialized' );
	},
	
	initDiavolo : function() {
		this.element.init();
	}
});