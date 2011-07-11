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


SiteFusion.Classes.Button = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULButton',
	
	initialize: function( win ) {
		this.element = win.createElement( 'button' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.ProgressMeter = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULProgressMeter',
	
	initialize: function( win, mode ) {
		this.element = win.createElement( 'progressmeter' );
		this.element.sfNode = this;
		
		this.value = this.numericValue;
		this.mode( mode );
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.Splitter = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULSplitter',
	
	initialize: function( win ) {
		this.element = win.createElement( 'splitter' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.ColorPicker = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULColorPicker',
	
	initialize: function( win ) {
		this.element = win.createElement( 'colorpicker' );
		this.element.sfNode = this;
		
		this.setEventHost( [ 'yield' ] );
	},
	
	yield: function() {
		this.fireEvent( 'yield', [ this.element.color ] );
	}
} );


SiteFusion.Classes.Scale = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULScale',
	
	initialize: function( win ) {
		this.element = win.createElement( 'scale' );
		this.element.sfNode = this;
		
		this.setEventHost( [ 'yield' ] );
	},
	
	yield: function() {
		//alert(this.element.value);
		this.fireEvent( 'yield', [ this.element.value ] );
	}
} );


SiteFusion.Classes.TextBox = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULTextBox',
	
	initialize: function( win ) {
		this.element = win.createElement( 'textbox' );
		this.element.sfNode = this;
		
	//	this.element.onkeypress = this._keypressHandler;
		
		this.setEventHost( [ 'yield', 'return' ] );
		
		this.eventHost.yield.msgType = 1;
	},
	
	_keypressHandler: function( event ) {
		if( event.keyCode == 13 ) {
			var oThis = this;
			setTimeout( function() { oThis.sfNode.fireEvent( 'return' ); }, 10 );
		}
	},
	
	value: function( text ) {
		if( this.element.type == 'number' )
		{
			this.element.valueNumber = text;
		}
		else
		{
			this.element.setAttribute( 'value', '' + text );
			this.element.value = '' + text;
			if( this.element.inputField )
				this.element.inputField.value = text;
		}
	},

	yield: function() {
		var val;
		if( this.element.type == 'number' )
			val = this.element.valueNumber;
		else
			val = this.element.inputField.value + '';
				
		this.fireEvent( 'yield', [ val ] );
	}
} );


SiteFusion.Classes.TimePicker = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULTimePicker',
	
	initialize: function( win ) {
		this.element = win.createElement( 'timepicker' );
		this.element.sfNode = this;
		
		this.setEventHost( [ 'yield' ] );
	
		this.eventHost.yield.msgType = 1;
	},
	
	value: function( text ) {
		if (typeof(text) == "number") {
			var dateval = new Date(Math.round(text * 1000));
			this.element.value = dateval.getHours() + ":" + dateval.getMinutes() + ":" + dateval.getSeconds();
		}
		else {
			if (text.length && text.match(/\d\d?\:\d\d?\:\d\d?/))
			{
				this.element.setAttribute( 'value', text );
			}
		}
	},
	
	reset: function() {
			var dateval = new Date();
			this.element.value = dateval.getHours() + ":" + dateval.getMinutes() + ":" + dateval.getSeconds();
	},

	yield: function() {
		var val;
		
		val = this.element.value + '';
		var hour = this.element.hour;
		var minute = this.element.minute;
		var second = this.element.second;
		var timestamp = Math.round(this.element.dateValue.getTime()/1000);
		this.fireEvent( 'yield', [ val, hour, minute, second, timestamp] );
	}
} );


SiteFusion.Classes.DatePicker = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULDatePicker',
	
	initialize: function( win ) {
		this.element = win.createElement( 'datepicker' );
		this.element.sfNode = this;
		
		this.setEventHost( [ 'yield' ] );
		
		this.eventHost.yield.msgType = 1;
	},
	
	yield: function() {
		var val = this.element.value;
		if( typeof(val) == 'undefined' )
			val = this.serverValue;
		this.fireEvent( 'yield', [ val ] );
	}
} );


SiteFusion.Classes.CheckBox = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULCheckBox',
	
	initialize: function( win ) {
		this.element = win.createElement( 'checkbox' );
		this.element.sfNode = this;
	
		this.setEventHost( [ 'yield' ] );
		
		this.eventHost.yield.msgType = 1;
	},
	
	yield: function() {
		this.fireEvent( 'yield', [ this.element.checked ] );
	}
} );


SiteFusion.Classes.RadioGroup = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULRadioGroup',
	
	initialize: function( win ) {
		this.element = win.createElement( 'radiogroup' );
		this.element.sfNode = this;
		
		this.setEventHost( [ 'yield' ] );
	
		this.eventHost.yield.msgType = 1;
	},
	
	yield: function() {
		var sel = this.element.selectedItem;
		if( sel != null )
			sel = sel.sfNode;
		
		this.fireEvent( 'yield', [ sel ] );
	},

	selectItem: function( item ) {
		var oThis = this;
		SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.element.selectedItem = item.element; } );
	}
} );


SiteFusion.Classes.Radio = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULRadio',
	
	initialize: function( win ) {
		this.element = win.createElement( 'radio' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.MenuList = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULMenuList',
	
	initialize: function( win ) {
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
	},
	
	// FIXME:
	// This workaround function needs to be removed when Mozilla fixes the menulist bindings
	_fixElement: function() {
		if( typeof(this.element.itemCount) == 'undefined' ) {
			// If itemCount is undefined, the menulist is broken
			var par = this.element.parentNode;
			if(! par ) return;	// element has been removed before timer hit
			
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
	},
	// /FIXME: end of workaround function
	
	yield: function() {
		var item, idx, elInputField;
		
		if( this.element.editable )
			elInputField = this.element.inputField.value;

		idx = (typeof(this.element.selectedIndex) == 'undefined' || this.element.selectedIndex == -1)  ? 0 : this.element.selectedIndex;
		if( typeof( this.element.childNodes[0].childNodes[idx] ) == 'undefined' ){ item = null; }
		else{	item = this.element.childNodes[0].childNodes[idx].sfNode; }
		
		this.fireEvent( 'yield', new Array( item, elInputField ) );
	},

	selectedItem: function( item ) {
		// FIXME: this property becomes obsolete when the workaround is removed
		this.selectedIndex = item;
		// /FIXME: end of workaround property
		
		var oThis = this;
		SiteFusion.Interface.DeferredCallbacks.push(
			function() {
				oThis.element.selectedIndex = item;
			}
		);
	},
	
	addItem: function( item, index ) {
		var oThis = this;
		SiteFusion.Interface.DeferredCallbacks.push(
			function() {
				var el = oThis.element.insertItemAt( index, item.element.getAttribute('label') );
				el.sfNode = item;
				item.element = el;
			}
		);
	}
} );

