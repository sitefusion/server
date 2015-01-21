SiteFusion.Classes.SourceEditor = function() {
    SiteFusion.Classes.Node.call(this);

    this.sfClassName = 'XULSourceEditor';
    this.editor = {};

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.SourceEditor.prototype = Object.create(SiteFusion.Classes.Node);
SiteFusion.Classes.SourceEditor.prototype.constructor = SiteFusion.Classes.SourceEditor;

    SiteFusion.Classes.SourceEditor.prototype.initialize = function( win ) {
        this.element = win.createElement( 'box' );
        this.element.sfNode = this;
        
        this.hostWindow = win;
        
        this.setEventHost( [
            'initialized',
            'ready',
            'yield',
            'yieldSearch'
        ] );
        
        this.eventHost.initialized.msgType = 0;
        this.eventHost.ready.msgType = 0;
        this.eventHost.yield.msgType = 1;
        this.eventHost.yieldSearch.msgType = 0;
    };
    
    SiteFusion.Classes.SourceEditor.prototype.initSourceEditor = function(value, language) {
        var prefs = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);
                    
        prefs.setCharPref("devtools.editor.component", "orion");   
        prefs.setBoolPref("devtools.editor.expandtab", true); 
        prefs.setIntPref("devtools.editor.tabsize", 4); 
                 
        Components.utils.import("chrome://sourceeditor/content/source-editor.jsm");
        
        var config = {
            mode: language,
            showLineNumbers: true,
                theme: SourceEditor.THEMES.MOZILLA,
            initialText: value
        };
        this.editor = new SourceEditor();
        var oThis = this;
        
        try {
            this.editor.init(this.element,config, function() {oThis.fireEvent( 'ready');});
        } catch(e) {
            alert(e);
        }
    };
    
    SiteFusion.Classes.SourceEditor.prototype.yield = function() { 
        this.fireEvent( 'yield', [ this.editor.getText(), this.editor.getSelectedText(), this.editor.getSelection().start, this.editor.getSelection().end, this.editor.getCharCount(), this.editor.getLineCount(), this.editor.getMode()] );
    };
    
    SiteFusion.Classes.SourceEditor.prototype.setMode = function(mode) {
        this.editor.setMode(mode);
    };
    
    SiteFusion.Classes.SourceEditor.prototype.setText = function(text) {
        this.editor.setText(text);
    };
    
    SiteFusion.Classes.SourceEditor.prototype.find = function(val,caseSensitive, reverseOrder, start) {
        this.fireEvent( 'yieldSearch', [this.editor.find(val, {backwards: reverseOrder, ignoreCase: !caseSensitive, start: start}),this.editor.lastFind.str,this.editor.lastFind.index,this.editor.lastFind.lastFound,this.editor.lastFind.ignoreCase] );
    };
    
    SiteFusion.Classes.SourceEditor.prototype.findNext = function(wrap) {
        this.fireEvent( 'yieldSearch', [this.editor.findNext(wrap)] );
    };
    
    SiteFusion.Classes.SourceEditor.prototype.findPrevious = function(wrap) {
        this.fireEvent( 'yieldSearch',[this.editor.findPrevious(wrap)] );
    };