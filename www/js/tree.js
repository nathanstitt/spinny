


TreeDDNode = function( obj ) {
    if ( obj ) {
        this.init( obj.type + obj.id, 'GridDD');
        this.initFrame();
    }
    var s = this.getDragEl().style;
    s.borderColor = "transparent";
    s.backgroundColor = "#f6f5e5";
    s.opacity = 0.76;
    s.filter = "alpha(opacity=76)";
};

// extend proxy so we don't move the whole object around
TreeDDNode.prototype = new YAHOO.util.DDProxy();


TreeDDNode.prototype.onDragDrop = function(e, id, foo ) {
    if ( id == "playlists" ){
	Playlists.songDropped( e, this.id.substring(2) );
    } else {
	Songs.songDropped( e, this.id.substring(2) );
    }
}

TreeDDNode.prototype.startDrag = function(x, y) {
    // called when source object first selected for dragging
    // draw a red border around the drag object we create
    var dragEl = this.getDragEl();
    var clickEl = this.getEl();
    dragEl.innerHTML = clickEl.innerHTML;
    dragEl.style.width = '30em';
    dragEl.style.padding = '0.4em';
    dragEl.style.border = "1px solid red";
}



TreeDDNode.prototype.onDragOver = function(e, id) {
	if ( id == "playlists" ){
		Playlists.elementOver( e );
	} else {
		Songs.elementOver( e );

	}
}

TreeDDNode.prototype.onDragOut = function(e, id) {
	if ( this.currentRow != null ){
		old_row = Layout.plGrid.getRow( this.currentRow );
		old_row.style.borderTop = '';
	}
}

TreeDDNode.prototype.endDrag = function(e) {
		// override so source object doesn't move when we are done
}






function Trees() {

}




Trees.loadNodeData=function(node, completeCallback ){
	var cb = { 
	success: function(o){
			YAHOO.log("AJAX Tree node load: " + o.responseText );
			var parent = o.argument.node;
			var treecallback = o.argument.callback;
			var ary=eval('('+o.responseText+')');
			for ( var indx in ary ){
				var obj=ary[indx];
				obj.iconMode = 0;
				Trees.addNode( obj, parent );
			}
			o.argument.callback();
		},
	failure: function(){
			alert("Tree Failed AJAX Req");
			o.argument.callback()
		},
	argument: {'node':node,'callback': completeCallback }
	};
	url="/tree/" + node.data.type + "/" + node.data.id
	var transaction = YAHOO.util.Connect.asyncRequest('GET', url, cb, null); 
}

Trees.addNode=function(obj,parent){
//	obj.type = parent.type;
	var newnode=new YAHOO.widget.TextNode( obj, parent , false );
	if ( obj.type == 's' ){
		newnode.label = '<span id="dd'+obj.id+'"><img src="/img/tree/song_add.png"> '+obj.label+'</span>';
		new TreeDDNode( obj );
	} else if ( obj.ch ){
		newnode.label = obj.label + " <sup>" + String(obj.ch) + "</sup>";
		newnode.setDynamicLoad( Trees.loadNodeData,1 );
	}
}






