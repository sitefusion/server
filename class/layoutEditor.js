SiteFusion.Classes.Editor = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULEditor';

    this.initialize.apply(this, arguments);
};

SiteFusion.Classes.Editor.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Editor.prototype.constructor = SiteFusion.Classes.Editor;

    SiteFusion.Classes.Editor.prototype.initialize = function( win ) {
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
    };
    
    SiteFusion.Classes.Editor.prototype.init = function() {
        this.element.setAttribute( 'src', 'about:blank' );
        this.fireEvent( 'initialized' );
    };
    
    SiteFusion.Classes.Editor.prototype.setValue = function( html ) {
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
    };
    
    SiteFusion.Classes.Editor.prototype.makeEditable = function() {
        if (typeof this.element.contentDocument == 'undefined' ) {
            SiteFusion.consoleMessage('this.element.contentDocument is undefined in makeEditable()');
            return;
        }
        this.element.contentDocument.designMode = "on";
        this.fireEvent( 'madeEditable' );
    };
    
    SiteFusion.Classes.Editor.prototype.editorLoaded = function() {
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
    };
    
    SiteFusion.Classes.Editor.prototype.disableInput = function(state)
    {
        if (typeof this.element.contentDocument == 'undefined' ){
            SiteFusion.consoleMessage('this.element.contentDocument is undefined in disableInput()');
            return;
        }
        this.element.contentDocument.execCommand('contentReadOnly',false, state);
    };
    
    SiteFusion.Classes.Editor.prototype.yield = function() {
        if(! this.editorElement ){
            SiteFusion.consoleMessage('this.editorElement is not set in in yield()');
            return;
        }
        if (typeof this.element.contentDocument == 'undefined' ){
            SiteFusion.consoleMessage('this.element.contentDocument is undefined in in yield()');
            return;
        }
        this.fireEvent( 'yield', [ this.element.contentDocument.body.innerHTML + '' ] );
    };
    
    SiteFusion.Classes.Editor.prototype.storeSelection = function() {
        if (typeof this.textEditor == 'undefined') {
                SiteFusion.consoleMessage('this.textEditor is undefined in storeSelection()');
                return;
        }
        var sel = this.textEditor.selection;
        this.storedSelection = [];
        
        for( var n = 0; n < sel.rangeCount; n++ ) {
            this.storedSelection.push( sel.getRangeAt(n) );
        }
    };

    SiteFusion.Classes.Editor.prototype.restoreSelection = function() {
        if (typeof this.textEditor == 'undefined') {
                SiteFusion.consoleMessage('this.textEditor is undefined in restoreSelection()');
                return;
        }
        var sel = this.textEditor.selection;
        sel.removeAllRanges();
        
        for( var n = 0; n < this.storedSelection.length; n++ ) {
            sel.addRange( this.storedSelection[n] );
        }
    };

    SiteFusion.Classes.Editor.prototype.createLink = function( href, target, className ) {
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
    };

    SiteFusion.Classes.Editor.prototype.removeElement = function( tagName ) {
        this.restoreSelection();
        
        var el;
        
        if(! (el = this.htmlEditor.getSelectedElement( tagName )) )
            return;
        
        while( el.hasChildNodes() ) {
            var c = el.removeChild( el.lastChild );
            el.parentNode.insertBefore( c, el );
        }
        
        el.parentNode.removeChild( el );
    };

    SiteFusion.Classes.Editor.prototype.insertImage = function( src, width, height, align, alt ) {
        this.restoreSelection();

        if ( width === parseInt( width ) )
            width += 'px';

        if ( height === parseInt( height ) )
            height += 'px';
        
        var img, newel = false;
        if ( img = this.htmlEditor.getSelectedElement( 'IMG' ) ) {
            if( src !== null )
                img.src = src;
            
            if( width !== null ) {
                img.style.width = width;
                img.setAttribute( 'width', width );
            }

            if( height !== null ) {
                img.style.height = height;
                img.setAttribute( 'height', height );
            }

            if( align !== null )
                img.align = align;
            if( alt !== null )
                img.alt = alt;
        
            img.setAttribute( 'border', '0' );
        } else {
            var html = '<img src="' + src + '" border="0" style="width: ' + width + '; height: ' + height + ';"' + (align !== null ? ' align="' + align + '"' : '') + ' alt="' + (alt !== null ? alt : '') + '">';
            this.htmlEditor.insertHTML( html );
        }

        var oThis = this;
        window.setTimeout( function() { oThis.checkDocumentState(); }, 200 );
    };

    SiteFusion.Classes.Editor.prototype.removeTableRow = function() {
        this.restoreSelection();
        
        var currentRow = this.getNearestElement( 'TR' );
        var table = this.getNearestElement( 'TABLE' );
        table.deleteRow( currentRow.rowIndex );
    };

    SiteFusion.Classes.Editor.prototype.insertTableRow = function() {
        this.restoreSelection();
        
        var currentRow = this.getNearestElement( 'TR' );
        var table = this.getNearestElement( 'TABLE' );
        
        var row = table.insertRow( currentRow.rowIndex );
        var cell;
        
        for( var n = 0; n < currentRow.cells.length; n++ ) {
            cell = row.insertCell(-1);
            cell.innerHTML = '&nbsp;';
        }
    };

    SiteFusion.Classes.Editor.prototype.removeTable = function() {
        this.restoreSelection();
        
        var table = this.getNearestElement( 'TABLE' );
        table.parentNode.removeChild( table );
    };

    SiteFusion.Classes.Editor.prototype.insertHeading = function(elementName) {
        this.htmlEditor.insertHTML( "<" + elementName + ">heading</" + elementName + ">");
    };
    
    SiteFusion.Classes.Editor.prototype.insertHTML = function( html ) {
        this.restoreSelection();
        
        this.htmlEditor.insertHTML( html );
    };

    SiteFusion.Classes.Editor.prototype.elementSetAttribute = function( element, attr, value ) {
        var el;
        
        if(! (el = this.getNearestElement( element )) )
            return;
        
        el.setAttribute( attr, value );
    };

    SiteFusion.Classes.Editor.prototype.elementSetStyle = function( element, prop, value ) {
        var el;
        
        if(! (el = this.getNearestElement( element )) )
            return;
        
        el.style[prop] = value;
    };

    SiteFusion.Classes.Editor.prototype.setTextClass = function( className ) {
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
    };
    
    SiteFusion.Classes.Editor.prototype.setTextID = function( id ) {
        var el = this.getNearestElement( 'SPAN' );
        
        if( id == '' ) {
            if( (!el) || el.tagName != 'SPAN' )
                return;
            
            while( el.hasChildNodes() ) {
                var c = el.removeChild( el.lastChild );
                el.parentNode.insertBefore( c, el );
            }
            
            el.parentNode.removeChild( el );
        } else {
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
    };
    
    SiteFusion.Classes.Editor.prototype.getNearestElement = function( tag ) {
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
    };

    SiteFusion.Classes.Editor.prototype.elementIsEditable = function( el ) {
        var found = false;
        
        while( el && el.tagName != "BODY" ) {
            if( el == this.editorElement ) {
                found = true;
                break;
            }
            el = el.parentNode;
        }
        
        return found;
    };

    SiteFusion.Classes.Editor.prototype.elementSetClassname = function( tag, className ) {
        var el = this.getNearestElement( tag );
        
        if (! el ) {
            return;
        }
        
        el.className = className;
    };
    
    SiteFusion.Classes.Editor.prototype.checkDocumentState = function() {
        this.storeSelection();
        
        if ( ! this.documentState ) {
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
        } catch( e ) {
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
        } catch( e ) {
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
    };
    
    SiteFusion.Classes.Editor.prototype.pasteHandler = function (e) {
        var xferableplain = Components.classes["@mozilla.org/widget/transferable;1"].createInstance(Components.interfaces.nsITransferable);
        var xferablehtml = Components.classes["@mozilla.org/widget/transferable;1"].createInstance(Components.interfaces.nsITransferable);
        var xferablemshtml = Components.classes["@mozilla.org/widget/transferable;1"].createInstance(Components.interfaces.nsITransferable);
        
        var unicodestring = Components.classes["@mozilla.org/supports-string;1"].createInstance(Components.interfaces.nsISupportsString);
        var clipboard = Components.classes["@mozilla.org/widget/clipboard;1"].getService(Components.interfaces.nsIClipboard);

        var str       = {};
        var strLength = {};
        var strHtml       = {};
        var strHtmlLength = {};
        var strMSHtml       = {};
        var strMSHtmlLength = {};
                
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
        } catch(e) {
            SiteFusion.consoleMessage("Microsoft XML not found on clipboard\n");
            hasMicrosoftHtml = false;
        }
        
        try {
            xferablehtml.getTransferData("text/html", strHtml, strHtmlLength);
            SiteFusion.consoleMessage("parsing html...\n");
        } catch(e) {
            SiteFusion.consoleMessage("html not found on clipboard\n");
            hasHtml = false;
        }
        
        
        try {
            xferableplain.getTransferData("text/unicode", str, strLength);
            SiteFusion.consoleMessage("parsing plaintext...\n");
        } catch(e) {
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
            } catch(ex) {
                SiteFusion.consoleMessage(e); 
            }
            
            var tempmshtml = strMSHtml;
            
            if (tempmshtml.indexOf("urn:schemas-microsoft-com:office:office") == -1) {
                var input = tempmshtml.split( /\r?\n/ );
                var data = {};
                while( input[0].match( /[a-z]\:.+/i ) ) {
                    var part = input[0].split( ':' );
                    data[part[0]] = part[1];
                    input.shift();
                }
                if (data.StartFragment && data.EndFragment) {
                    var selStart = parseInt(data.StartFragment.replace(/^0+/,''));
                    var selEnd = parseInt(data.EndFragment.replace(/^0+/,''));
                
                    var utfConv = Components.classes["@mozilla.org/intl/scriptableunicodeconverter"].createInstance(Components.interfaces.nsIScriptableUnicodeConverter);
                        utfConv.charset = "UTF-8";
                    selectedHtml = utfConv.ConvertToUnicode(tempmshtml.substr( selStart, selEnd-selStart ));
                }
            }
        }

        var processedHtml = '';
        if (selectedHtml) {
            //check for incomplete table structure
            
            if (selectedHtml.substr(0,3).toLowerCase() == "<tr" && selectedHtml.substr(selectedHtml.length - 5, 5).toLowerCase() == "</tr>")
                processedHtml = "<table>" + selectedHtml + "</table>";
            else
                processedHtml = selectedHtml;
            
            unicodestring.data = processedHtml;
            xferablehtml.setTransferData("text/html", unicodestring, processedHtml.length * 2);

            clipboard.setData(xferablehtml, null, Components.interfaces.nsIClipboard.kGlobalClipboard);
            dump ("setData() mshtml: " + unicodestring + "\n");
        } else if (strHtml && hasHtml && !htmlIsOfficeHtml) {
            //check for incomplete table structure
            if (pastehtml.substr(0,3).toLowerCase() == "<tr" && pastehtml.substr(pastehtml.length - 5, 5).toLowerCase() == "</tr>")
                processedHtml = "<table>" + pastehtml + "</table>";
            else
                processedHtml = pastehtml;
            
            unicodestring.data = processedHtml;
            xferablehtml.setTransferData("text/html", unicodestring, processedHtml.length * 2);

            clipboard.setData(xferablehtml, null, Components.interfaces.nsIClipboard.kGlobalClipboard);
            dump ("setData() html: " + processedHtml + "\n");
        } else {
            unicodestring.data = pastetext;
            xferableplain.setTransferData("text/unicode ", unicodestring, pastetext.length * 2);
            clipboard.setData(xferableplain, null, Components.interfaces.nsIClipboard.kGlobalClipboard);
            dump ("setData() plaintext\n");
        }
        
    };

SiteFusion.Classes.LayoutEditor = function() {
    SiteFusion.Classes.Editor.apply(this, arguments);

    this.sfClassName = 'XULLayoutEditor';
};

SiteFusion.Classes.LayoutEditor.prototype = Object.create(SiteFusion.Classes.Editor.prototype);
SiteFusion.Classes.LayoutEditor.prototype.constructor = SiteFusion.Classes.LayoutEditor;

    SiteFusion.Classes.LayoutEditor.prototype.loadLayoutHTML = function( html, frameid ) {
        if (typeof this.element.contentDocument == 'undefined' ) {
            SiteFusion.consoleMessage('this.element.contentDocument is undefined in loadLayoutHTML()');
            return;
        }
        this.element.contentWindow.document.write( html );  
        
        
        this.editorElement = this.element.contentDocument.getElementById( frameid );
    };

    SiteFusion.Classes.LayoutEditor.prototype.loadBody = function( html ) {
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
    };

    SiteFusion.Classes.LayoutEditor.prototype.makeEditable = function() {
        if (typeof this.element.contentDocument == 'undefined' ) {
            SiteFusion.consoleMessage('this.element.contentDocument is undefined in makeEditable()');
            return;
        }
        this.element.contentDocument.body.contentEditable = false;
        this.editorElement.contentEditable = true;
        this.fireEvent( 'madeEditable' );
    };
    
    SiteFusion.Classes.LayoutEditor.prototype.disableInput = function(state)
    {
        this.editorElement.contentEditable = !state;
    };
    
    SiteFusion.Classes.LayoutEditor.prototype.yield = function() {
        if(! this.editorElement ) {
            SiteFusion.consoleMessage('this.element.contentDocument is undefined in yield()');
            return;
        }
        this.fireEvent( 'yield', [ this.editorElement.innerHTML + '' ] );
    };
