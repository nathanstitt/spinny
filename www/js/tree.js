function loadDataForNode( node, onCompleteCallback) { 
	var conCallback = { 
	    success: handleSuccess,
	    failure: handleFailure,
	    argument: { "node": node, "callback": onCompleteCallback }
	}
	url="/tree/" + node.data.type + "/" + node.data.id
	var transaction = YAHOO.util.Connect.asyncRequest('GET', url, conCallback, null); 
}

function handleSuccess(o) {
	var node = o.argument.node;
	var treecallback = o.argument.callback;
	var artists = new YAHOO.widget.TextNode( node, false);
	for ( obj in JSON.parse(o.responseText) ){
		var node=new YAHOO.widget.TextNode( obj, node, false );
	}
	treecallback();
};

function handleFailure(o) {
	alert("Failed AJAX");
};
