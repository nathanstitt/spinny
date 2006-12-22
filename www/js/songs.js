Songs = function() {

    var grid,dm,cm;

    var lastHighlighted;

    onSongDblClick = function(grid, rowIndex, colIndex){
	if ( 0 == colIndex ){
	    params='pl_id='+Playlists.selected()+'&song_id='+dm.getRowId( rowIndex );
	    var cb = {
		success: function(o){ YAHOO.log( "Song rm req success" ); 
		    Songs.refreshListing();	      
		},
		failure: function(o){ YAHOO.log("Song rm req failed"); }
	    }
	    YAHOO.log( "Removing Song: " + dm.getRowId( rowIndex )  );
	    YAHOO.util.Connect.asyncRequest( 'POST', '/pl/songs/rm', cb, params );
	}
    }

    // return a public interface
    return {
	refreshListing : function(){
	    YAHOO.log( "Loaded dyna Playlist data for pl id " + Playlists.selected() );
	    dm.load('/pl/songs/list', "pl_id=" + Playlists.selected() );
	},

	elementOver : function( event ){
	    row=grid.getTargetRow( event );
	    if ( row != lastHighlighted ){
		if ( lastHighlighted == null ){
		    if ( dm.getRowCount() ){
			grid.getRow( dm.getRowCount()-1 ).style.borderBottom = '';
		    }
		} else {
		    old_row = grid.getRow( lastHighlighted );
		    if ( old_row ){
			old_row.style.borderTop = '';
		    }
		}
		if ( row == null ){
		    new_row = grid.getRow( dm.getRowCount()-1 );
		    new_row.style.borderBottom = '1px solid red';

		} else {
		    new_row = grid.getRow( row );
		    if ( new_row ){
			new_row.style.borderTop = '1px solid red';
		    }
		}
		lastHighlighted = row;
	    }
	},
	dragOver : function( event ){
	    rownum=grid.getTargetRow( event );
	    style='borderTop';
	    YAHOO.log( "Apply to " + rownum );
	    if ( rownum == null ) {
		rownum = dm.getRowCount()-1;
		style='borderBottom';
	    }
	    if ( rownum != -1 ){
		row = grid.getRow( rownum );
		HighlightEvents.start( row, style );
	    }
	},
        init : function(){
	    YAHOO.log("Initializing songs grid","info");
	    dm = new YAHOO.ext.grid.JSONDataModel( {
		root: 'Songs',
		id: 'id',
		fields: ['','tr','tt','at','al','ln']
	    } );
	    
	    cm=new YAHOO.ext.grid.DefaultColumnModel
		( [
		   {header: "Rm",resizable:false, width: 30,renderer: function(){ return '<img src="/img/delete.gif" class="link">'; } },
		   {header: "Track",resizable:false, width:50,  sortable: true}, 
		   {header: "Title",  sortable: true}, 
		   {header: "Artist", sortable: true},
		   {header: "Album",  sortable: true}, 
		   {header: "Length", sortable: true}
		   ]
		 );
	    
	    dm.addListener('loadException', function( grid,err ){
		YAHOO.log( "Song Load excp Msgs:" + err.message,"error" );
	    });


	    grid = new SpinnyGrid( 'songs', dm, cm );
	    grid.reOrderUrl='/pl/songs/reorder';
	    grid.reqParams="pl_id="+Playlists.selected();
	    grid.ddText="%0 songs selected";


	    grid.addListener('celldblclick', onSongDblClick, this, true );

	    grid.enableDragDrop = true;
	    grid.selfTarget = true;
	    grid.autoSize = function(){
		SpinnyGrid.superclass.autoSize.call(this);
		model=this.colModel;
		view=this.getView();
		width = view.wrap.clientWidth;

		YAHOO.log("Autosize songs, width="+width);
		view.fitColumns.defer( 50, view );
// 		model.setColumnWidth(1, width*0.05 );
// 		model.setColumnWidth(2, width*0.40 );
// 		model.setColumnWidth(3, width*0.20 );
// 		model.setColumnWidth(4, width*0.20 );
// 		model.setColumnWidth(3, width*0.20 );
	    };
	    grid.autoSizeColumns = false;

	    grid.selectAfterDrop = true;
	    dm.load('/pl/songs/list', "pl_id=" + Playlists.selected() );
	    grid.render();
	    
	},
	getGrid : function(){
	    return grid;
	},

	songDropped : function( event, song_ids ){
	    YAHOO.log("Song ids " + song_ids + " Dropped on row " + lastHighlighted );
	    position = grid.getTargetRow( event );
	    if ( position == null ){
		position=dm.getRowCount();
	    }
	    params='pl_id='+Playlists.selected()+'&position='+position;
	    for (i = 0; i < song_ids.length; i += 1) {
		params+="&song_id="+song_ids[i];
	    }
	    var cb = {
		success: function(o){ YAHOO.log( "Song add req success" ); 
		    Songs.refreshListing();	      
		},
		failure: function(o){ YAHOO.log("Song add req failed"); }
	    }
	    YAHOO.log( "About to send song additions: " + song_ids + " to " + params );
	    YAHOO.util.Connect.asyncRequest( 'POST', '/pl/songs/addsong', cb, params );
	},
	playlistDropped : function( event, ids ){
	    rownum = grid.getTargetRow( event );
	    if ( rownum == null ){
		rownum = dm.getRowCount()-1;
	    }
	    YAHOO.log( "PL " + ids + " dropped on songs row " + rownum );
	    params='pl_id='+Playlists.selected()+'&position='+rownum;
	    for (i = 0; i < ids.length; i += 1) {
		params+="&pl_id="+ids[i];
	    }
	    var cb = {
		success: function(o){ YAHOO.log( "Song add playlists req success" ); 
		    Songs.refreshListing();	      
		},
		failure: function(o){ YAHOO.log("Song add playlists req failed"); }
	    }
	    YAHOO.log( "About to send pl additions to songs " + params );
	    YAHOO.util.Connect.asyncRequest( 'POST', '/pl/songs/addpl', cb, params ); 
	}

	
    };
}();

//YAHOO.ext.EventManager.onDocumentReady( Songs.init, Songs, true );


