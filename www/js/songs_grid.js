var SongsGrid = function(){

    var ds;
    var cm;
    var grid;
    var cssOver;
    var paging;
    var currentPage;
	


    onSongDblClick = function(grid, rowIndex, colIndex){
	if ( 0 == colIndex ){
	    var song = SongsGrid.ds.getAt( rowIndex )
            params='pl_id='+PlaylistsGrid.getCurrentId()+'&song_id=' + song.id;
	    var url = '/pl/songs/rm';
	    var cb = {
                scope: this,
        	argument: { song: song },
		success: function(o){ 
		    this.removeSong( o.argument.song );
		},
		failure: function(o){ alert("Song rm req failed"); }
	    }
	    Ext.lib.Ajax.request( 'POST', url, cb, params );
	} else {
	    var mp = Layout.getApplet();
	    var pos = SongsGrid.rangeShowing().start + rowIndex-1;
	    var res = mp.setPlaylistPos( pos );
//	    console.log( "Change to song " + pos );
	}
    }
    
    return {
	rangeShowing : function(){
	    return { start: SongsGrid.paging.cursor+1, end: 1 + SongsGrid.paging.cursor + SongsGrid.ds.data.getCount() };
	},
	hiLightRow : function( num ){
	    var el_num = ( num % SongsGrid.ds.baseParams.limit );
	    r=SongsGrid.rangeShowing();
//	    console.log( "On Track: " + el_num + ' / ' + num + ' : ' + r.start + ' ~ ' + r.end );
	    if ( num >= end ){
		SongsGrid.ds.load( { params:{ start: num } } );
	    } else if ( num < r.start ){
		SongsGrid.ds.load({ params:{ start: 1+(num-( r.end-r.start )) } });
	    } else {
		SongsGrid.grid.getSelectionModel().selectRow( el_num );
	    }
	},
	clearDNDcss : function(){
	    Ext.util.CSS.updateRule(  ".x-grid-row-over td", "border-top", "0px" );
	    Ext.util.CSS.updateRule(  ".x-grid-row-over td, .x-grid-locked .x-grid-row-over td", "background-color", '#d9e8fb' );
	},

	setHeight :function( height ){
	    this.ds.baseParams.limit=Math.round((height-50)/28);
//	    console.log("Height: " + height );
	},
	removeSong : function( song ){
	    SongsGrid.ds.remove( song );//data.removeAt( rowIndex );
	},
	refresh: function(){
	    SongsGrid.ds.baseParams.pl_id = PlaylistsGrid.getCurrentId();
	    this.ds.baseParams.limit=Math.round( (Layout.height()-50) / 28 );
	    SongsGrid.ds.load( { params: {start:0 } }  );
	},
	randomize: function(){
//	    console.log( "Randomize" );
	},
	onLoad:function(){
//	    var mp = document.getElementById("flash_applet");
	    var mp = Layout.getApplet();
	    if ( mp.getPlaylistId() != PlaylistsGrid.getCurrentId() ){
		mp.loadNewPlaylist( PlaylistsGrid.getCurrentId() )
		SongsGrid.grid.getSelectionModel().selectRow(0 );
	    } else {
		var el_num = ( mp.getPlaylistPos() % SongsGrid.ds.baseParams.limit );
		SongsGrid.grid.getSelectionModel().selectRow( el_num );
	    }
	},
	init : function(){

	    var jsonreader =  new Ext.data.JsonReader( {
		    root: 'Songs',
		    id: 'id',
		    totalProperty: 'Size'
		}, [
		   {name: 'track', mapping: 'tr', type: 'int'},
		   {name: 'title', mapping: 'tt'},
		   {name: 'artist', mapping: 'at'},
		   {name: 'album', mapping: 'al'},
		   {name: 'length', mapping: 'ln', type: 'int'}
		   ] );
// 	    jsonreader.addListener('load',function(o,arg ){
// 		console.log("Loaded");
// 	    });
            // create the Data Store
	    this.ds = new Ext.data.Store({
                // load using HTTP
		proxy: new Ext.data.HttpProxy({url: '/pl/songs/list'}),
                // the return will be XML, so lets set up a reader
		reader: jsonreader,
		remoteSort: true
	    });
	    this.ds.on('load', this.onLoad, this);
	    this.setHeight( Layout.height() );

	    this.currentPage = 1;
	    cm = new Ext.grid.ColumnModel([
					       {header: "Del", fixed:true, width: 30,renderer:
						   function(){ return '<img src="/images/custom/delete.gif" class="link">'; } },
					       {header: "Track", fixed:true, width: 120, dataIndex: 'track'},
					       {header: "Title", width: 180, dataIndex: 'title'},
					       {header: "Artist", width: 115, dataIndex: 'artist'},
					       {header: "Album", width: 100, dataIndex: 'album'},
					       {header: "Length", fixed:true, width: 120, dataIndex: 'length'}
					       ]);
	    cm.defaultSortable = true;

            // create the grid
	    this.grid = new Ext.grid.Grid('songsGrid', {
		ds: this.ds,
		cm: cm,
		autoSizeColumns:true,
		enableColLock:false,
		monitorWindowResize: true,
		autoWidth:true,
		enableDragDrop:true,
		ddGroup : 'songsDD',
		loadMask: true
	    });
	    this.grid.autoWidth = true;
	    this.grid.addListener('celldblclick', onSongDblClick, this, true );
	    this.grid.on( 'startdrag', function(grid,dd,e){
		var rows=SongsGrid.grid.getSelectionModel().getSelections();
		var ids = [];
		for( var i = 0, len = rows.length; i < len; i++) {
		    ids.push( rows[i].id );
		}
		Layout.beginDrag( 'songsGrid', ids );
	    });
	    Layout.addSongsGrid( this.grid );


	    var drop = new Ext.dd.DropTarget(this.grid.container, {
		ddGroup : 'songsDD',
		notifyDrop : function( dd, e, data){
		    SongsGrid.clearDNDcss();
		    var drag_data = Layout.getDragData();
		    if ( "songsGrid" == drag_data.type ){
			for( var i = 0, len = data.selections.length; i < len; i++) {
       			    SongsGrid.ds.remove(data.selections[i]);
			}
			SongsGrid.grid.selModel.clearSelections();
			var dragData = dd.getDragData(e);
			var pos = dragData.rowIndex;
			SongsGrid.ds.insert(pos, data.selections);
			var params='pl_id='+PlaylistsGrid.getCurrentId();
			for ( var x = 0, len = SongsGrid.ds.data.getCount(); x < len; x++ ) {
			    params+="&" + SongsGrid.ds.getAt( x ).id.toString() + "=" + (x+1).toString();
			}
			Ext.lib.Ajax.request( 'POST', '/pl/songs/reorder', null, params );
		    } else {
			var cb = {
 			    success: function(o){ alert( "Song add req success" );
				SongsGrid.refresh();
 			    },
 		    	    failure: function(o){ alert("Song add req failed"); }
 			}
			var t = e.getTarget();
			var row = SongsGrid.grid.view.findRowIndex(t)+1;
			if ( ! row ){
			    row = SongsGrid.ds.data.items.length+1;
			}
			Ext.lib.Ajax.request( 'POST', '/pl/songs/add/' + drag_data.type, cb,
					     'pl_id='+PlaylistsGrid.getCurrentId()+'&id='+drag_data.ids+'&pos='+row );
		    }
	    	    return true;
		},
 		notifyEnter: function( src, e, data ){
		    SongsGrid.cssOver = Ext.util.CSS.getRule( ".x-grid-row-over td",true );
		    Ext.util.CSS.updateRule(  ".x-grid-row-over td", "border-top", "1px solid red" );
		    Ext.util.CSS.updateRule(  ".x-grid-row-over td, .x-grid-locked .x-grid-row-over td", "background-color", 'white' );
 		    return drop.dropAllowed;
 		},
 		notifyOut: function( src, e, data ){
		    SongsGrid.clearDNDcss();
 		    return drop.dropAllowed;
 		}
	    });

 	    this.grid.render();
	    var gridFoot = this.grid.getView().getHeaderPanel(true);

	    this.paging = new Ext.PagingToolbar( gridFoot, this.ds, {
		pageSize: this.ds.baseParams.limit,
		displayInfo: true,
		displayMsg: 'Displaying songs {0} - {1} of {2}',
		emptyMsg: "No songs to display"
	    });
	    this.paging.add( 'separator' );
	    this.paging.addButton( new Ext.Toolbar.Button( { icon:'/images/custom/shuffle.gif', cls:"x-btn-text-icon", handler: SongsGrid.randomize } ) );
 //	    SongsGrid.refresh();

	}
    };
}();

Ext.EventManager.onDocumentReady( SongsGrid.init, SongsGrid, true);
