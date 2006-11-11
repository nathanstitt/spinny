

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
	var parent = o.argument.node;
	var treecallback = o.argument.callback;
	var ary=eval('('+o.responseText+')');
	for ( var indx in ary ){
		var obj=ary[indx];
		obj.iconMode = 0;
		addTreeNode( obj, parent );
	}
	o.argument.callback()
};

function handleFailure(o) {
	alert("Tree Failed AJAX Req");
	o.argument.callback()
};


function addTreeNode( obj, parent ){
	var newnode=new YAHOO.widget.TextNode( obj, parent , false );
	if ( obj.type == 's' ){
		newnode.label = '<img src="/img/song_add.png"> ' + obj.label
	} else if ( obj.ch ){
		newnode.label = obj.label + " <sup>" + String(obj.ch) + "</sup>"
			newnode.setDynamicLoad(loadDataForNode,1);
	}
}