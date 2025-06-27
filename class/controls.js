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
// theFrontDoor.
// Portions created by the Initial Developer are Copyright (C) 2009
// the Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Nikki Auburger <nikki@thefrontdoor.nl> (original author)
//   Tom Peeters <tom@thefrontdoor.nl>
//   Pieter Janssen <pieter.janssen@thefrontdoor.nl>
//
// - - - - - - - - - - - - - - END LICENSE BLOCK - - - - - - - - - - - - -

SiteFusion.Classes.Button = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULButton';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Button.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Button.prototype.constructor = SiteFusion.Classes.Button;

    SiteFusion.Classes.Button.prototype.initialize = function( win ) {
        this.element = win.createElement( 'button' );
        this.element.sfNode = this;

        this.setEventHost();
    };

SiteFusion.Classes.ProgressMeter = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULProgressMeter';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.ProgressMeter.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.ProgressMeter.prototype.constructor = SiteFusion.Classes.ProgressMeter;

    SiteFusion.Classes.ProgressMeter.prototype.initialize = function( win, mode ) {
        this.element = win.createElement( 'progressmeter' );
        this.element.sfNode = this;

        this.value = this.numericValue;
        this.mode( mode );

        this.setEventHost();
    };

SiteFusion.Classes.Splitter = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULSplitter';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Splitter.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Splitter.prototype.constructor = SiteFusion.Classes.Splitter;

    SiteFusion.Classes.Splitter.prototype.initialize = function( win ) {
        this.element = win.createElement( 'splitter' );
        this.element.sfNode = this;

        this.setEventHost();
    };

SiteFusion.Classes.ColorPicker = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULColorPicker';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.ColorPicker.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.ColorPicker.prototype.constructor = SiteFusion.Classes.ColorPicker;

    SiteFusion.Classes.ColorPicker.prototype.initialize = function( win ) {
        this.element = win.createElement( 'colorpicker' );
        this.element.sfNode = this;

        this.setEventHost( [ 'yield' ] );
    };

    SiteFusion.Classes.ColorPicker.prototype.yield = function() {
        this.fireEvent( 'yield', [ this.element.color ] );
    };

SiteFusion.Classes.Scale = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULScale';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Scale.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Scale.prototype.constructor = SiteFusion.Classes.Scale;

    SiteFusion.Classes.Scale.prototype.initialize = function( win ) {
        this.element = win.createElement( 'scale' );
        this.element.sfNode = this;

        this.setEventHost( [ 'yield' ] );
    };

    SiteFusion.Classes.Scale.prototype.yield = function() {
        this.fireEvent( 'yield', [ this.element.value ] );
    };

SiteFusion.Classes.TextBox = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULTextBox';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.TextBox.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.TextBox.prototype.constructor = SiteFusion.Classes.TextBox;

    SiteFusion.Classes.TextBox.prototype.initialize = function( win ) {
        this.element = win.createElement( 'textbox' );
        this.element.sfNode = this;

        this.setEventHost( [ 'yield', 'return' ] );
        var oThis = this;
        this.eventHost.yield.msgType = 1;
    };

    SiteFusion.Classes.TextBox.prototype._keypressHandler = function( event ) {
        if( event.keyCode == 13 ) {
            var oThis = this;
            setTimeout( function() { oThis.sfNode.fireEvent( 'return' ); }, 10 );
        }
    };

    SiteFusion.Classes.TextBox.prototype.value = function( text ) {
        if (this.element.type !== 'number') {
            this.element.setAttribute( 'value', text );
            this.element.value = text + '';

            if ( this.element.inputField ) {
                this.element.inputField.value = text;
            }
        } else {
            this.element.valueNumber = text;
        }
    };

    SiteFusion.Classes.TextBox.prototype.yield = function() {
        let val = '';
    	let validityObj = {};

    	if (this.element.type === 'number') {
            val = this.element.valueNumber;
    	} else if (this.element.inputField && typeof this.element.inputField.value !== 'undefined') {
            val = this.element.inputField.value + '';

            if (this.element.inputField.validity) {
            	for (let prop in this.element.inputField.validity) {
                    validityObj[prop] = this.element.inputField.validity[prop];
            	}
            }
    	} else if (typeof this.element.value !== 'undefined') {
            val = this.element.value + '';
    	}	

    	this.fireEvent('yield', [val, JSON.stringify(validityObj)]);
    };

SiteFusion.Classes.TimePicker = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULTimePicker';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.TimePicker.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.TimePicker.prototype.constructor = SiteFusion.Classes.TimePicker;

    SiteFusion.Classes.TimePicker.prototype.initialize = function( win ) {
        this.element = win.createElement( 'timepicker' );
        this.element.sfNode = this;

        var isMac = navigator.platform.toUpperCase().indexOf('MAC')>=0;

        if (isMac) {
            this.element.addEventListener('popuphiding', function(e) {
                var origEl = this;
                var par =  origEl.parentNode;
                var el = par.removeChild(origEl);
                par.appendChild(el);
            });
        }

        this.setEventHost( [ 'yield' ] );

        this.eventHost.yield.msgType = 1;
    };

    SiteFusion.Classes.TimePicker.prototype.value = function( text ) {
        if (typeof(text) == "number") {
            var dateval = new Date(Math.round(text * 1000));
            var strVal = dateval.getHours() + ":" + dateval.getMinutes() + ":" + dateval.getSeconds();
            var oThis = this;
            window.setTimeout(function() {
                oThis.element.value = strVal;
            }, 10);
        } else {
            if (text.length && text.match(/\d\d?\:\d\d?\:\d\d?/)) {
                this.element.value = text;
            }
        }
    };

    SiteFusion.Classes.TimePicker.prototype.reset = function() {
        var dateval = new Date();
        this.element.value = dateval.getHours() + ":" + dateval.getMinutes() + ":" + dateval.getSeconds();
    };

    SiteFusion.Classes.TimePicker.prototype.yield = function() {
        var val;

        val = this.element.value + '';
        var hour = this.element.hour;
        var minute = this.element.minute;
        var second = this.element.second;
        var timestamp = Math.round(this.element.dateValue.getTime()/1000);
        this.fireEvent( 'yield', [ val, hour, minute, second, timestamp] );
    };

SiteFusion.Classes.DatePicker = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULDatePicker';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.DatePicker.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.DatePicker.prototype.constructor = SiteFusion.Classes.DatePicker;

    SiteFusion.Classes.DatePicker.prototype.initialize = function( win ) {
        this.element = win.createElement( 'datepicker' );
        this.element.sfNode = this;

        var isMac = navigator.platform.toUpperCase().indexOf('MAC')>=0;

        if (isMac) {
            this.element.addEventListener('popuphiding', function(e) {
                var origEl = this;
                var par =  origEl.parentNode;
                var el = par.removeChild(origEl);
                par.appendChild(el);
            });
        }

        this.element.timeoutloop = null;

        this.setEventHost( [ 'yield' ] );

        this.eventHost.yield.msgType = 1;
    };

    SiteFusion.Classes.DatePicker.prototype.value = function( text ) {
        this.initialValue = text;

        var dateVal;
        if (typeof(text) == "number") {
            dateVal = new Date(Math.round(text * 1000));
        } else if (text.length && text.length == 10 && text.match(/\d{1,4}(\/|-)\d{1,4}(\/|-)\d{1,4}/)) {
            dateVal = new Date(text);
        } else {
            SiteFusion.consoleError('XULDatePicker::value : Value \'' + text + '\' is not a valid date.');
        }

        if (typeof dateVal !== 'undefined') {
            var obThis = this;
            window.setTimeout(function() {
                obThis.setDateValue(dateVal);
            }, 1);

        }
    };

    SiteFusion.Classes.DatePicker.prototype.setDateValue = function(dateVal) {
        // Workaround start
        if (!(dateVal instanceof Date))
            throw "Invalid Date";

        if (this.isBound()) {
            this.element._setValueNoSync(dateVal);
            if (this.element.attachedControl) {
                this.element.attachedControl._setValueNoSync(dateVal);
            }
        } else {
            if (this.element.timeoutloop) {
                window.clearTimeout(this.element.timeoutloop);
                this.element.timeoutloop = null;
            }

            var self = this;
            this.element.timeoutloop = window.setTimeout(function() {
                self.setDateValue(dateVal);
            }, 200);

        }
        // Workaround end

    };

    SiteFusion.Classes.DatePicker.prototype.isBound = function() {
        return (typeof this.element._setValueNoSync !== 'undefined');
    };

    SiteFusion.Classes.DatePicker.prototype.reset = function() {
        this.setDateValue(new Date());
    };

    SiteFusion.Classes.DatePicker.prototype.yield = function() {
        var val;
        if (this.isBound()) {
            val = this.element.value;
            if (typeof(val) == 'undefined') {
                val = this.serverValue;
            }
        } else {
            // element binding has not kicked in, so return initial value
            val = this.initialValue;
        }
        this.fireEvent( 'yield', [ val ] );
    };

SiteFusion.Classes.CheckBox = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULCheckBox';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.CheckBox.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.CheckBox.prototype.constructor = SiteFusion.Classes.CheckBox;

    SiteFusion.Classes.CheckBox.prototype.initialize = function( win ) {
        this.element = win.createElement( 'checkbox' );
        this.element.sfNode = this;

        this.setEventHost( [ 'yield' ] );

        this.eventHost.yield.msgType = 1;
    };

    SiteFusion.Classes.CheckBox.prototype.yield = function() {
        this.fireEvent( 'yield', [ this.element.checked ] );
    };

SiteFusion.Classes.RadioGroup = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULRadioGroup';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.RadioGroup.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.RadioGroup.prototype.constructor = SiteFusion.Classes.RadioGroup;

    SiteFusion.Classes.RadioGroup.prototype.initialize = function( win ) {
        this.element = win.createElement( 'radiogroup' );
        this.element.sfNode = this;

        this.setEventHost( [ 'yield' ] );

        this.eventHost.yield.msgType = 1;
    };

    SiteFusion.Classes.RadioGroup.prototype.yield = function() {
        var sel = this.element.selectedItem;
        if( sel !== null ) {
            sel = sel.sfNode;
        }

        this.fireEvent( 'yield', [ sel ] );
    };

    SiteFusion.Classes.RadioGroup.prototype.selectItem = function( item ) {
        var oThis = this;
        SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.element.selectedItem = item.element; });
    };

SiteFusion.Classes.Radio = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULRadio';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Radio.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Radio.prototype.constructor = SiteFusion.Classes.Radio;

    SiteFusion.Classes.Radio.prototype.initialize = function( win ) {
        this.element = win.createElement( 'radio' );
        this.element.sfNode = this;

        this.setEventHost();
    };

SiteFusion.Classes.MenuList = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULMenuList';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.MenuList.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.MenuList.prototype.constructor = SiteFusion.Classes.MenuList;

    SiteFusion.Classes.MenuList.prototype.initialize = function( win ) {
        this.element = win.createElement( 'menulist' );
        this.element.sfNode = this;
        this.hostWindow = win;

        this.setEventHost( [ 'yield' ] );

        this.eventHost.yield.msgType = 1;

        // FIXME:
        // This is the dirtiest workaround ever, but it seems to be the only way to
        // get the menulist to function properly in situations where it is constructed
        // while not yet visible. Problems include:
        // - fails to show first menuitem
        // - always one selection behind, selectedIndex indicates the last selected item
        //   instead of the currently selected one
        // - fails to programmatically preselect an item
        // - can only be constructed properly when it has become visible, hence the timer
        //   because there is no event to indicate the control became visible.
        var oThis = this;
        setTimeout( function() { oThis._fixElement(); }, 1000 );
        // /FIXME: end of workaround timer call
    };

    // FIXME:
    // This workaround function needs to be removed when Mozilla fixes the menulist bindings
    SiteFusion.Classes.MenuList.prototype._fixElement = function() {
        if( typeof(this.element.itemCount) == 'undefined' ) {
            // If itemCount is undefined, the menulist is broken
            var par = this.element.parentNode;
            if(! par ) return;  // element has been removed before timer hit

            // Replacing the menulist element with a new one is nescessary to rearm the bindings
            var before = this.element.nextSibling;
            par.removeChild( this.element );
            var popup = this.element.removeChild( this.element.childNodes[0] );
            this.element = this.hostWindow.createElement( 'menulist' );
            this.element.appendChild( popup );
            this.element.sfNode = this;

            if( before )
                par.insertBefore( this.element, before );
            else
                par.appendChild( this.element );

            var oThis = this;

            // Hook up all menuitems with an event handler for DOMMenuItemActive to manually
            // keep track of selected items
            for( var n = 0; n < popup.childNodes.length; n++ ) {
                var item = popup.childNodes[n];
                if( typeof(item._menulistHandlerSet) != 'undefined' )
                    continue;

                var func = function() {
                    for( var c = 0; c < this.parentNode.childNodes.length; c++ ) {
                        if( this.parentNode.childNodes[c] == this ) {
                            oThis.selectedIndex = c;
                            break;
                        }
                    }
                };

                item._menulistHandlerSet = true;
                item.addEventListener( 'DOMMenuItemActive', func, true );

                // Use the 'selected' attribute to preset the initial selection because
                // setting selectedIndex on the menulist here will still break it
                if( typeof(this.selectedIndex) != 'undefined' && this.selectedIndex == n )
                    item.setAttribute( 'selected', 'true' );
            }

            // Hook a function on the command event to set the element.selectedIndex to
            // the actual index in this.selectedIndex
            var setFunc = function() {
                oThis.element.selectedIndex = oThis.selectedIndex;
                delete oThis.selectedIndex;
            };

            // Hook a function to the keypress event to handle the up and down keys changing
            // the selected item when the popup is closed but the menulist has focus (windows only)
            var keyFunc = function(event) {
                if( oThis.element.childNodes[0].state == 'closed' && event.keyCode == 38 || event.keyCode == 40 ) {
                    var currentIndex = oThis.element.selectedIndex;
                    if( currentIndex == 0 && event.keyCode == 38 )
                        currentIndex = oThis.element.itemCount - 1;
                    else if( currentIndex == oThis.element.itemCount - 1 && event.keyCode == 40 )
                        currentIndex = 0;
                    else
                        currentIndex += (event.keyCode == 38 ? -1:1);

                    oThis.selectedIndex = currentIndex;
                }
            };

            this.element.addEventListener( 'command', setFunc, true );
            this.element.addEventListener( 'keypress', keyFunc, true );

            // Restore SiteFusion event listeners that were previously set on the discarded element
            for( var n = 0; n < SiteFusion.Comm.XULEvents.length; n++ ) {
                this.setEventListener( SiteFusion.Comm.XULEvents[n] );
            }

            // Repeat if this iteration didn't execute after the menulist became visible
            setTimeout( function() { oThis._fixElement(); }, 1000 );
        }
    };
    // /FIXME: end of workaround function

    SiteFusion.Classes.MenuList.prototype.yield = function() {
        var item, idx, elInputField;

        if( this.element.editable )
            elInputField = this.element.inputField.value;

        idx = (typeof(this.element.selectedIndex) == 'undefined' || this.element.selectedIndex == -1)  ? 0 : this.element.selectedIndex;
        if( typeof( this.element.childNodes[0].childNodes[idx] ) == 'undefined' ){ item = null; }
        else{   item = this.element.childNodes[0].childNodes[idx].sfNode; }

        this.fireEvent( 'yield', new Array( item, elInputField ) );
    };

    SiteFusion.Classes.MenuList.prototype.selectedItem = function( item ) {
        // FIXME: this property becomes obsolete when the workaround is removed
        this.selectedIndex = item;
        // /FIXME: end of workaround property

        var oThis = this;
        SiteFusion.Interface.DeferredCallbacks.push(
            function() {
                oThis.element.selectedIndex = item;
            }
        );
    };

    SiteFusion.Classes.MenuList.prototype.addItem = function( item, index ) {
        var oThis = this;
        SiteFusion.Interface.DeferredCallbacks.push(
            function() {
                var el = oThis.element.insertItemAt( index, item.element.getAttribute('label') );
                el.sfNode = item;
                item.element = el;
            }
        );
    };
