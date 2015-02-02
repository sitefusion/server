SiteFusion.Classes.CodeMirror = function() {
    SiteFusion.Classes.Browser.apply(this, arguments);

    this.sfClassName = 'XULCodeMirror';
    
    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.CodeMirror.prototype = Object.create(SiteFusion.Classes.Browser.prototype);
SiteFusion.Classes.CodeMirror.prototype.constructor = SiteFusion.Classes.CodeMirror;

    SiteFusion.Classes.CodeMirror.prototype.initialize = function( win ) {
        this.element = win.createElement( 'browser' );
        this.element.sfNode = this;
        this.element.setAttribute('disablehistory', true);
        var oThis = this;
        
        this.v2 = new this.v2func(this);
        this.v1 = new this.v1func(this);
        
        this.hostWindow = win;
        
        this.setEventHost([
            'initialized',
            'ready',
            'yield',
            'yieldThemes',
            'themesLoaded',
            'noMoreResults',
            'replaceResultCount'
        ]);
        
        this.eventHost.initialized.msgType = 0;
        this.eventHost.ready.msgType = 0;
        this.eventHost.yield.msgType = 1;
        this.eventHost.yieldThemes.msgType = 0;
        this.eventHost.themesLoaded.msgType = 0;
        this.eventHost.noMoreResults.msgType = 0;
        this.eventHost.replaceResultCount.msgType = 0;
    };

     /*v1 functions*/
    SiteFusion.Classes.CodeMirror.prototype.v1func = function() {
        this.initialize.apply(this, arguments);
    };

        SiteFusion.Classes.CodeMirror.prototype.v1func.prototype.initialize = function( par ) {
            this.parent = par;
        };
        
        SiteFusion.Classes.CodeMirror.prototype.v1func.prototype.yield = function() {
            this.parent.fireEvent( 'yield', [ this.parent.element.contentWindow.editor.getCode() ] );
        };

        SiteFusion.Classes.CodeMirror.prototype.v1func.prototype.search = function() {
            var text = this.parent.hostWindow.windowObject.prompt("Voer zoekterm in:", "");
            if (!text) {
                return;
            }
        
            var first = true;
            do {
                var cursor = this.parent.element.contentWindow.editor.getSearchCursor(text, first);
                first = false;
                while (cursor.findNext()) {
                    cursor.select();
                    if (!this.parent.hostWindow.windowObject.confirm("nogmaals zoeken?")) {
                        return;
                    }
                }
            } while (this.parent.hostWindow.windowObject.confirm("Einde bereikt. Verder zoeken?"));
        };

        SiteFusion.Classes.CodeMirror.prototype.v1func.prototype.replace = function() {
            // This is a replace-all, but it is possible to implement a
            // prompting replace.
            var from = this.parent.hostWindow.windowObject.prompt("Enter search string:", ""), to;
            if (from) {
                to = this.parent.hostWindow.windowObject.prompt("What should it be replaced with?", "");
            }
            if (to === null) {
                return;
            }
        
            var cursor = this.parent.element.contentWindow.editor.getSearchCursor(from, false);
            while (cursor.findNext()) {
                cursor.replace(to);
            }
        };
    
        SiteFusion.Classes.CodeMirror.prototype.v1func.prototype.jump = function() {
            var line = this.parent.hostWindow.windowObject.prompt("Jump to line:", "");
            if (line && !isNaN(Number(line))) {
                this.parent.element.contentWindow.editor.jumpToLine(Number(line));
            }
        };
        
        SiteFusion.Classes.CodeMirror.prototype.v1func.prototype.line = function() {
            this.parent.hostWindow.windowObject.alert("The cursor is currently at line " + this.parent.element.contentWindow.editor.currentLine());
            this.parent.element.contentWindow.editor.focus();
        };
        
        SiteFusion.Classes.CodeMirror.prototype.v1func.prototype.macro = function() {
            var name = this.parent.hostWindow.windowObject.prompt("Name your constructor:", "");
            if (name) {
                this.parent.element.contentWindow.editor.replaceSelection("function " + name + "() {\n  \n}\n\n" + name + ".prototype = {\n  \n};\n");
            }
        };
        
        SiteFusion.Classes.CodeMirror.prototype.v1func.prototype.reindent = function() {
            this.parent.element.contentWindow.editor.reindent();
        };

  
    /*v2 functions*/
    SiteFusion.Classes.CodeMirror.prototype.v2func = function() {
        this.lastPos = null;
        this.lastQuery = null;
        this.marked = [];
        this.from = null;
        this.to = null;
        this.lastDirection = -1;

        this.initialize.apply(this, arguments);
    };
        
        SiteFusion.Classes.CodeMirror.prototype.v2func.prototype.initialize = function( par ) {
            this.parent = par;
        };
        
        SiteFusion.Classes.CodeMirror.prototype.v2func.prototype.yield = function() { 
            this.parent.fireEvent( 'yield', [ this.parent.element.contentWindow.editor.getValue(), this.parent.element.contentWindow.editor.getSelection() ] );
        };
        
        SiteFusion.Classes.CodeMirror.prototype.v2func.prototype.getAvailableThemes = function() {
            try {
                var a =[];
                var links = this.parent.element.contentDocument.getElementsByTagName("link"); 
                for (var i = 0; i < links.length; i++) { 
                    var rel = links[i].getAttribute("rel"); 
                    var href = links[i].getAttribute("href");
                    if ( rel == "stylesheet" && href.indexOf('theme/') != -1 ) { 
                        var theme = href.substring(6,href.length-4);
                        a.push(theme);
                    }
                }
                this.parent.fireEvent( 'yieldThemes', a );
            } catch (e) {
                alert(e);
            }
        };
        
        SiteFusion.Classes.CodeMirror.prototype.v2func.prototype.unmark = function () {
            for (var i = 0; i < this.marked.length; ++i) {
                if (this.marked[i].clear === 'function' ) {
                    this.marked[i].clear();
                } else {
                    this.marked[i];
                }
            }
            this.marked.length = 0;
        };
    
        SiteFusion.Classes.CodeMirror.prototype.v2func.prototype.search = function (query, caseSensitive, reverseOrder) {
            try {
                this.unmark();
                var text = query;
                if (!text) return;
              
                if (this.lastDirection != -1 && this.lastDirection != reverseOrder) {
                    //direction has changed since last search. Toggle lastPos accordingly!
                    if (!reverseOrder)
                        this.lastPos = this.to;
                    else this.lastPos = this.from;
                }
                this.lastDirection = reverseOrder;
              
                for (var cursor = this.parent.element.contentWindow.editor.getSearchCursor(text, {'line': 0, 'ch': 0}, caseSensitive); (reverseOrder ? cursor.findPrevious() : cursor.findNext());) {
                    this.marked.push(this.parent.element.contentWindow.editor.markText(cursor.from(), cursor.to(), "searched"));
                }
            
                if (this.lastQuery != text) this.lastPos = null;
                var cursor = this.parent.element.contentWindow.editor.getSearchCursor(text, this.lastPos || this.parent.element.contentWindow.editor.getCursor(), caseSensitive);
              
                if (!(reverseOrder ? cursor.findPrevious() : cursor.findNext())) {
                    cursor = this.parent.element.contentWindow.editor.getSearchCursor(text, {'line': 0, 'ch': 0}, caseSensitive);
                    if (!(reverseOrder ? cursor.findPrevious() : cursor.findNext())) {
                        this.parent.fireEvent('noMoreResults');
                        return false;
                    }
                }
                this.from = cursor.from();
                this.to = cursor.to();
              
                this.parent.element.contentWindow.editor.setSelection(cursor.from(), cursor.to());
                this.lastQuery = text;
                if (!reverseOrder) {
                    this.lastPos = cursor.to();
                } else {
                    this.lastPos = cursor.from(); 
                }
            } catch (e) {
                alert(e);
            }
            return true;
        };
    
        SiteFusion.Classes.CodeMirror.prototype.v2func.prototype.replaceAll = function (query, replace, caseSensitive) {
            this.unmark();
            if (!query) {
                this.parent.fireEvent('noMoreResults');
                return;
            }
            var counter = 0;
            for (var cursor = this.parent.element.contentWindow.editor.getSearchCursor(query,{ 'line': 0, 'ch': 0 }, caseSensitive); cursor.findNext();) {
                this.parent.element.contentWindow.editor.replaceRange(replace, cursor.from(), cursor.to());
                counter++;
            }
            if (!counter) {
                this.parent.fireEvent('noMoreResults');
                return;
            } else {
                this.parent.fireEvent('replaceResultCount', [counter]);
                return;
            }
        };
        
        SiteFusion.Classes.CodeMirror.prototype.v2func.prototype.replace = function (query, replace, caseSensitive, reverseOrder) {
            this.unmark();
            var cursor;
            var text = query;
            if (!text) {
                this.parent.fireEvent('noMoreResults');
                return;
            }

            if (this.parent.element.contentWindow.editor.getSelection() != query) {
                this.search(query, caseSensitive,reverseOrder);
                return;
            }
                
            this.parent.element.contentWindow.editor.replaceRange(replace, this.from, this.to);
            
            var tempcursor = this.parent.element.contentWindow.editor.getSearchCursor(replace, this.from, caseSensitive);
            if (tempcursor.findNext()) {
                this.parent.element.contentWindow.editor.setSelection(tempcursor.from(), tempcursor.to());
            
                this.from = tempcursor.from();
                this.to = tempcursor.to();
              
                this.lastQuery = query;
                this.lastPos = tempcursor.to();
            
                this.search(query, caseSensitive, reverseOrder);
            } else {
                this.parent.fireEvent('noMoreResults');
                return;
            }
        };
