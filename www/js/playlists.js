

var PlaylistsGrid = function(){

    var grid;
    var ds;
    var cm;
    var currentId;
    var cssOver;

    onDblClick = function(grid, rowIndex, colIndex){
	if ( 1 == colIndex ){
	    var pl = this.ds.getAt( rowIndex );
	    this.currentId = pl.id;
	    SongsGrid.refresh();
	 //   PlaylistsGrid.grid.getView().refresh();//nderRows( 0rowIndex, rowIndex );
	}
    }
    afterEdit = function( edit ){
	var params = 'pl_id=' + PlaylistsGrid.ds.getAt( edit.row ).id.toString() + '&text=' + encodeURIComponent(edit.value);
	Ext.lib.Ajax.request( 'POST', '/pl/update/' + edit.field, null, params );
	PlaylistsGrid.ds.commitChanges();
    }
    onDrop = function( dd, e, data){
	PlaylistsGrid.clearDNDcss();
	var t = e.getTarget();
	var row = PlaylistsGrid.grid.view.findRowIndex(t);
	if ( ! row ){
	    row = PlaylistsGrid.ds.data.items.length-1;
	}

	var pl_id = PlaylistsGrid.ds.getAt( row ).id.toString()
	var drag_data = Layout.getDragData();
	var cb = ( pl_id == PlaylistsGrid.getCurrentId() ) ? { success: function(o){ SongsGrid.refresh(); } } : null;
	if ( "songsGrid" == drag_data.type ){
	    var params='pos=0&pl_id='+ pl_id;
	    for( var i=0; i < drag_data.ids.length; ++i ){
		params+="&song_id=" + drag_data.ids[i];
	    }
	    Ext.lib.Ajax.request( 'POST', '/pl/songs/addsongs', cb, params );
	} else if ( "pl" == drag_data.type ){
	    var dragData = dd.getDragData(e);
	    var params='rdr=1';
	    for( var i = 0, len = data.selections.length; i < len; i++) {
       		PlaylistsGrid.ds.remove(data.selections[i]);
	    }
	    PlaylistsGrid.grid.selModel.clearSelections();
	    var pos = dragData.rowIndex;
	    if ( ! pos ){
		pos = PlaylistsGrid.ds.data.items.length;
	    }
	    PlaylistsGrid.ds.insert(pos, data.selections);
	    for ( var x = 0, len = PlaylistsGrid.ds.data.getCount(); x < len; x++ ) {
		params+="&" + PlaylistsGrid.ds.getAt( x ).id.toString() + "=" + (x+1).toString();
	    }
	    Ext.lib.Ajax.request( 'POST', '/pl/reorder', null, params );
	} else {
	    Ext.lib.Ajax.request( 'POST', '/pl/songs/add/' + drag_data.type, cb,
				 'pl_id='+pl_id+'&id='+drag_data.ids+'&pos='+0 );
	}
	return true;
    }
    return {
	clearDNDcss : function() {
	    Ext.util.CSS.updateRule( ".x-grid-row-over td", "border", "0px" );
	    Ext.util.CSS.updateRule( ".x-grid-row-over td", "border-bottom" , "1px solid #c3daf9" );
	    Ext.util.CSS.updateRule( ".x-grid-row-over td, .x-grid-locked .x-grid-row-over td", "background-color", '#d9e8fb' );
	},
	refresh : function(){
	    console.log("Refresh PL");
	    PlaylistsGrid.ds.load( { params: {start:0 } }  );
	},
	addNew : function(){
	    PlaylistsGrid.ds.add(
				 new Ext.data.Record( {
				     description:'Unnamed Playlist',
				     name: 'New Playlist',
				     newRecord: true
				 }, 0 ) );
	},
	init : function(){
	    this.ds = new Ext.data.Store({
		proxy: new Ext.data.HttpProxy({ 
		    url: '/pl/list'
		}),
		reader: new Ext.data.JsonReader({
		    root: 'Playlists',
		    id: 'id'
		}, [
		    {name: 'name' },
		    {name: 'description' }
		    ])
	    });
	    
	    this.cm = new Ext.grid.ColumnModel([ {
		dataIndex:'name',
		editor: new Ext.grid.GridEditor( new Ext.form.TextField( { allowBlank: false } ) )
	    }, { fixed:true,
		width: 30,
		renderer: function( val, cell, rec, row, col, ds ){
		    return '<img src="/images/custom/play.png" class="link">'; 
		}
	    } ] );

            // create the grid
	    this.grid = new Ext.grid.EditorGrid('playlistsGrid', {
		ds: this.ds,
		cm: this.cm,
		selModel: new Ext.grid.RowSelectionModel({singleSelect:true}),
		autoSizeColumns:true,
		monitorWindowResize: true,
		trackMouseOver: true,
		autoWidth:true,
		enableDragDrop:true,
		ddGroup : 'songsDD'
	    } );

	    this.grid.autoWidth = true;
	    this.grid.addListener('afteredit', afterEdit, this, true );
	    this.grid.autoWidth = true;
	    this.grid.addListener('celldblclick', onDblClick, this, true );

	    this.grid.on( 'startdrag', function(grid,dd,e){
		Layout.beginDrag( 'pl', PlaylistsGrid.grid.getSelectionModel().getSelected().id );
	    });

	    Layout.addPlGrid( this.grid );

	    var drop = new Ext.dd.DropTarget( this.grid.container, {
		ddGroup : 'songsDD',
		notifyDrop : PlaylistsGrid.onDrop,
		notifyEnter: function( src, e, data ){
		    PlaylistsGrid.cssOver = Ext.util.CSS.getRule( ".x-grid-row-over td",true );
		    Ext.util.CSS.updateRule( ".x-grid-row-over td", "border-top" , "1px solid red" );
		    if ( 'pl' != Layout.getDragData().type ){
			Ext.util.CSS.updateRule( ".x-grid-row-over td", "border-bottom" , "1px solid red" );
		    } else {
			Ext.util.CSS.updateRule(  ".x-grid-row-over td, .x-grid-locked .x-grid-row-over td", "background-color", 'white' );
		    }
 		    return drop.dropAllowed;
		},
		notifyOut: function( src, e, data ){
		    PlaylistsGrid.clearDNDcss();
 		    return drop.dropAllowed;
		}
	    });

	    this.grid.render();
	    this.ds.load();
//	    this.refresh();

	},

	getCurrentId : function(){
	    return ( this.currentId || 1 );
	}

    };
}();

//Ext.EventManager.onDocumentReady( PlaylistsGrid.init, PlaylistsGrid, true);
