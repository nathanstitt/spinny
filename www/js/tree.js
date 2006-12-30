


TreeDDNode = function( obj ) {
    this.init( obj.p + obj.id, 'GridDD');
    this.initFrame();
    var s = this.getDragEl().style;
    s.borderColor = "transparent";
    s.backgroundColor = "#f6f5e5";
    s.opacity = 0.76;
    s.filter = "alpha(opacity=76)";
};

// extend proxy so we don't move the whole object around
TreeDDNode.prototype = new YAHOO.util.DDProxy();


TreeDDNode.prototype.onDragDrop = function(e, id, foo ) {
    Layout.handleDrop( id, 'song', e, [ this.id.substring(1) ] );
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
    Layout.handleDrag( this, id, 'song', e );
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

Trees.labelClick = function( node ){
    YAHOO.log( "Node " + node.data.label + " clicked" );
    var cb = { 
	success: function(o){
	    Layout.showSongInfo( o.argument.song_id, o.responseText );
	},
	failure: function(){
	    alert("Tree Label Click Failed AJAX Req");
	},
	argument: {'song_id':node.data.id }
    };
    url="/info/song/" + node.data.id + "/artist";
    YAHOO.util.Connect.asyncRequest('GET', url, cb, null);
}


// 

Trees.loadNodeData=function(node, completeCallback ){
    var cb = { 
	success: function(o){
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
YAHOO.log("Load Node: " + node.data.p + ' : ' + node.label  );
    url="/tree/" + node.data.type + "/" + node.data.id
    var transaction = YAHOO.util.Connect.asyncRequest('GET', url, cb, null); 
}

Trees.addNode=function(obj,parent){
 //   obj.data.p = parent.data.p;
    var newnode=new YAHOO.widget.TextNode( obj, parent , false );
    if ( obj.type == 's' ){
	newnode.label = '<span id="'+obj.p+obj.id+'"><img src="/img/tree/song_add.png"> '+obj.type + ' : ' + obj.p + ' : ' + obj.label+'</span>';
	newnode.onLabelClick = Trees.labelClick;
	new TreeDDNode( obj );
    } else if ( obj.ch ){
	newnode.label = obj.label + " <sup>" + String(obj.ch) + "</sup>";
	newnode.setDynamicLoad( Trees.loadNodeData,1 );
    }
}






