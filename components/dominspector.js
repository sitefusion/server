SiteFusion.ClientComponents.DOMInspector = {
	AssertSelf: function() {
		try {
			var tmpNameSpace = {}; 
			var sl = Components.classes["@mozilla.org/moz/jssubscript-loader;1"].createInstance(Components.interfaces.mozIJSSubScriptLoader);
			sl.loadSubScript("chrome://inspector/content/hooks.js", tmpNameSpace);            
		}
		catch(e)
		{
			return false;
		}
		return true;
	},
	
	DOMInspector: function() {
		var windowDS = Components.classes["@mozilla.org/rdf/datasource;1?name=window-mediator"].getService(Components.interfaces.nsIWindowDataSource);
		var tmpNameSpace = {};                         
		var sl = Components.classes["@mozilla.org/moz/jssubscript-loader;1"].createInstance(Components.interfaces.mozIJSSubScriptLoader);
		sl.loadSubScript("chrome://inspector/content/hooks.js", tmpNameSpace);
		tmpNameSpace.inspectDOMDocument(document);
	}
}