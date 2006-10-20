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
	var ary=eval('('+o.responseText+')');
//	alert(o.responseText);
	for ( var indx in ary ){
		var obj=ary[indx];
		obj.iconMode = 0;
		var newnode=new YAHOO.widget.TextNode( obj, node, false );
		if ( obj.ch ){
			newnode.label = obj.label + " <sup>(" + String(obj.ch) + ")</sup>"
			newnode.setDynamicLoad(loadDataForNode,1);
		}
	}
	o.argument.callback()
};

function handleFailure(o) {
	alert("Tree Failed AJAX Req");
	o.argument.callback()
};
