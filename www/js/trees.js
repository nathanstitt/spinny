var SongsTree = function(){
    // shorthand
    var Tree = Ext.tree;

    var dirsTree;
    var tagsTree;

    var createTree = function( url, elName ){

        var tree = new Tree.TreePanel( Ext.get(elName), {
            animate:true, 
            enableDrag:true,
            containerScroll: true,
	    loader: new Tree.TreeLoader( {dataUrl:url}),
            ddGroup: 'songsDD',
            rootVisible:false
        });
 
            // set the root node
	var root = new Tree.AsyncTreeNode({
	    text: 'yui-ext',
	    draggable:false,
	    id:'0'
	});
	tree.setRootNode(root);
	tree.on('startdrag', function(tree, node, e ){
	    Layout.beginDrag( node.attributes.cls, node.id );
	});
        tree.render();
	root.expand();
	return tree;
    }

    return {
        init : function(){
	    dirsTree = createTree('/tree/dir', 'dirsTree' );
	    tagsTree = createTree('/tree/tag', 'tagsTree' );
	}
    };
}();

//Ext.EventManager.onDocumentReady( SongsTree.init, SongsTree, true);

