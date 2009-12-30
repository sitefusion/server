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
		var val;
		if( typeof(this.element.value) == 'undefined' )
			val = this.serverValue;
		else
			val = this.element.value
		
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
		
		this.setEventHost( [ 'yield' ] );
		
		this.eventHost.yield.msgType = 1;
	},
	
	yield: function() {
		var item, idx;

		idx = (typeof(this.element.selectedIndex) == 'undefined' || this.element.selectedIndex == -1)  ? 0 : this.element.selectedIndex;
		item = this.element.childNodes[0].childNodes[idx].sfNode;
		
		this.fireEvent( 'yield', new Array( item ) );
	},

	selectedItem: function( item ) {
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

