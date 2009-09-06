
SiteFusion.Interface = {
	ChildWindows: [],
	DeferredCallbacks: [],
	
	HandleDeferredChildAdditions: function() {
		var root = SiteFusion.Registry[0];
		this.HandleDeferredChildAdditionsRecursive( root );
		this.HandleDeferredCallbacks();
	},

	HandleDeferredChildAdditionsRecursive: function( node ) {
		node.isPainted = true;
		
		for( var n = 0; n < node.deferredSFChildren.length; n++ ) {
			var item = node.deferredSFChildren[n];
			this.HandleDeferredChildAdditionsRecursive( item[1] );

			if( item[0] == 'addChild' )
				node.directAddChild( item[1] );
			else if( item[0] == 'addChildBefore' )
				node.directAddChildBefore( item[1], item[2] );
		}

		for( var n = 0; n < node.sfChildren.length; n++ ) {
			this.HandleDeferredChildAdditionsRecursive( node.sfChildren[n] );
		}

		for( var n = 0; n < node.deferredSFChildren.length; n++ ) {
			var item = node.deferredSFChildren[n];

			if( item[0] == 'addChild' )
				node.sfChildren.push( item[1] );
			else if( item[0] == 'addChildBefore' ) {
				for( var m = 0; m < node.sfChildren.length; m++ ) {
					if( node.sfChildren[m] == item[2] ) {
						node.sfChildren.splice( m, 0, item[1] );
						break;
					}
				}
			}
		}

		node.deferredSFChildren = new Array();
	},

	HandleDeferredCallbacks: function() {
		for( var n = 0; n < this.DeferredCallbacks.length; n++ ) {
			this.DeferredCallbacks[n]();
		}

		this.DeferredCallbacks = [];
	},
	
	CursorBusy: function() {
		document.documentElement.style.cursor = 'wait';
		
		for( var n = 0; n < SiteFusion.Interface.ChildWindows.length; n++ ) {
			if( SiteFusion.Interface.ChildWindows[n] ) {
				if( SiteFusion.Interface.ChildWindows[n].document ) {
					if( SiteFusion.Interface.ChildWindows[n].document.documentElement )
						SiteFusion.Interface.ChildWindows[n].document.documentElement.style.cursor = 'wait';
				}
			}
		}
	},

	CursorIdle: function() {
		document.documentElement.style.cursor = '';

		for( var n = 0; n < SiteFusion.Interface.ChildWindows.length; n++ ) {
			if( SiteFusion.Interface.ChildWindows[n] ) {
				if( SiteFusion.Interface.ChildWindows[n].document ) {
					if( SiteFusion.Interface.ChildWindows[n].document.documentElement )
						SiteFusion.Interface.ChildWindows[n].document.documentElement.style.cursor = '';
				}
			}
		}
	},
	
	RegisterChildWindow: function( win ) {
		SiteFusion.Interface.ChildWindows.push( win );
	},
	
	UnregisterChildWindow: function( win ) {
		for( var n = 0; n < SiteFusion.Interface.ChildWindows.length; n++ ) {
			if( SiteFusion.Interface.ChildWindows[n] === win ) {
				SiteFusion.Interface.ChildWindows.splice( n, 1 );
				return;
			}
		}
	}
};

