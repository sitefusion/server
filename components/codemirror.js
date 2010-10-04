SiteFusion.Classes.CodeMirror = Class.create( SiteFusion.Classes.Browser, {
	sfClassName: 'XULCodeEditor',
	
	initialize: function( win ) {
		this.element = win.createElement( 'browser' );
		this.element.sfNode = this;
		this.element.setAttribute('disablehistory', true);
		var oThis = this;
		
		this.hostWindow = win;
		
		this.setEventHost( [
			'initialized',
			'ready',
			'yield'
		] );
		
		this.eventHost.initialized.msgType = 0;
		this.eventHost.ready.msgType = 0;
		this.eventHost.yield.msgType = 1;
	},
	yield: function() {
		this.fireEvent( 'yield', [ this.element.contentWindow.editor.getCode() ] );
	},
	search: function() {
	    var text = this.hostWindow.windowObject.prompt("Voer zoekterm in:", "");
	    if (!text) return;
	
	    var first = true;
	    do {
	      var cursor = this.element.contentWindow.editor.getSearchCursor(text, first);
	      first = false;
	      while (cursor.findNext()) {
	        cursor.select();
	        if (!this.hostWindow.windowObject.confirm("nogmaals zoeken?"))
	          return;
	      }
	    } while (this.hostWindow.windowObject.confirm("Einde bereikt. Verder zoeken?"));
	},
	replace: function() {
	    // This is a replace-all, but it is possible to implement a
	    // prompting replace.
	    var from = this.hostWindow.windowObject.prompt("Enter search string:", ""), to;
	    if (from) to = this.hostWindow.windowObject.prompt("What should it be replaced with?", "");
	    if (to == null) return;
	
	    var cursor = this.element.contentWindow.editor.getSearchCursor(from, false);
	    while (cursor.findNext())
	      cursor.replace(to);
	  },
	
	jump: function() {
	    var line = this.hostWindow.windowObject.prompt("Jump to line:", "");
	    if (line && !isNaN(Number(line)))
	      this.element.contentWindow.editor.jumpToLine(Number(line));
	},
	
	line: function() {
	    this.hostWindow.windowObject.alert("The cursor is currently at line " + this.element.contentWindow.editor.currentLine());
	    this.element.contentWindow.editor.focus();
	},
	
	macro: function() {
	    var name = this.hostWindow.windowObject.prompt("Name your constructor:", "");
	    if (name)
	      this.element.contentWindow.editor.replaceSelection("function " + name + "() {\n  \n}\n\n" + name + ".prototype = {\n  \n};\n");
	},
	
	reindent: function() {
	    this.element.contentWindow.editor.reindent();
	}
  
});