PlaylistsDlg = function(){

	// define some private variables
	var dialog, showBtn;
	var currentEditId;
	var wait,error,errorMsg;
        save = function(){
		wait.setVisible(true);
		params = YAHOO.util.Connect.setForm(document.getElementById('playlist-dlg-form'));
		var cb = {
		success: function(o){
				YAHOO.log( "Playlist save req success" ); 
				Playlists.refresh();
				o.argument.wait.setVisible(false);
				o.argument.dialog.hide();
			},
		failure: function(o){ 
				YAHOO.log("Playlist save req failed"); 
				o.argument.wait.setVisible(false);
				o.argument.errorMsg.update('Save Failed: ' + o.responseText);
				o.argument.error.setVisible(true);
			},
		argument: { 'wait':wait,'dialog':dialog,'error':error,'errorMsg': errorMsg }
		}
		if ( currentEditId == null ){
			url='/pl/create';
		} else {
			url='/pl/modify';
			params+='&pl_id='+currentEditId;
		}
		YAHOO.log( "Save params: " + params );
		YAHOO.util.Connect.asyncRequest('POST', url, cb, params );
        }
    initDialog  = function(){
	    if ( dialog ){
		    return null;
	    }
	    dialog = new YAHOO.ext.BasicDialog("playlists-dlg", { 
		    modal:true,
		    autoTabs:true,
		    width:350,
		    height:300,
		    shadow:true
	    });
	    dialog.addKeyListener(27, dialog.hide, dialog);
	    dialog.addButton('Close', dialog.hide, dialog);
	    dialog.addButton('Save', this.save, this );
	    var sizeTextBox = function(){
		    txtDesc.setSize(dialog.size.width-44, dialog.size.height-200);
		    txtName.setSize(dialog.size.width-44 );
	    };
	    dialog.on('resize', sizeTextBox);
	    sizeTextBox();
	    dialog.on('hide', function(){
			    wait.removeClass('active-msg');
			    error.removeClass('active-msg');
			    txtDesc.dom.value = '';
			    txtName.dom.value = '';
			    errorMsg.update('');
		    });
    }

    showDialog = function(title){
	    dialog.getTabs().getTab( 0 ).setText(title);
	    showBtn = getEl('playlists-show-btn');
	    wait.setVisible(false);
	    error.setVisible(false);
            dialog.show(showBtn.dom);
    }

    // return a public interface
    return {
        init : function(){
		    txtDesc = getEl('playlist-dlg-desc');
		    txtName = getEl('playlist-dlg-name');
		    wait = getEl('playlist-dlg-save-wait');
		    error = getEl('playlist-dlg-save-error');
		    errorMsg = getEl('playlist-dlg-error-msg');
        },

	showAddDialog : function(){
	    initDialog();
	    txtDesc.dom.value = '';
	    txtName.dom.value = '';
	    currentEditId=null;
	    showDialog("Create Playlist");
	},

	showEditDialog:function( pl ){
	    initDialog();
	    currentEditId	= pl.id;
	    txtDesc.dom.value	= pl.description;
	    txtName.dom.value	= pl.name;
	    showDialog("Edit Playlist");		    
	}
    };
}();

YAHOO.ext.EventManager.onDocumentReady(PlaylistsDlg.init, PlaylistsDlg, true);



Playlists = function(){ 

    var grid, dm, cm;
    var selectedRow, origColor;
    var descEl;


    renderEdit = function(val){
	return '<img src="/img/edit.gif">';
    }

    hiLightRow = function( rowIndex ){
	rowEl = grid.getRow( rowIndex );
	grid.clearSelections();
	YAHOO.util.Dom.setStyle( grid.getRow( selectedRow ), 'background-color', origColor );
	origColor=YAHOO.util.Dom.getStyle( rowEl, 'background-color' );
	YAHOO.util.Dom.setStyle( rowEl, 'background-color', '#f30f15' );

    }

    onPlDblClick = function(grid, rowIndex, colIndex){
 	YAHOO.log( "PL Cell DBL Click on " + colIndex  );
	row=dm.getRow( rowIndex );
	if ( colIndex == 0 ){
	    PlaylistsDlg.showEditDialog( { 'id': dm.getRowId( rowIndex ), 'description':row[0], 'name':row[1] } );
	} else if ( rowIndex != selectedRow ){
	    
	    hiLightRow( rowIndex );

	    selectedRow = rowIndex;
	    row=dm.getRow( rowIndex );
	    descEl.update( row[0] );
	    Songs.refreshListing();
	}
    }

    add = function( pl ){
	YAHOO.log( "Begin Adding pl" );
	reload();
    }

    onLoad = function(){
	row=dm.getRow( selectedRow );
	hiLightRow( selectedRow );

	descEl.update( row[0] );
	Songs.refreshListing();
    }

    onFirstLoad = function(){
	hiLightRow( 0 );

	row=dm.getRow( selectedRow );
	descEl.update( row[0] );
	Songs.init.call( Songs );
	Layout.init();
	dm.removeListener('load', onFirstLoad );
	dm.addListener('load', onLoad, this, true);
    }
    handleDrag = function( style, last_style, event ){
	rownum=grid.getTargetRow( event );
	if ( rownum == null ) {
	    rownum = dm.getRowCount()-1;
	    style=last_style;
	}
	row = grid.getRow( rownum );
	HighlightEvents.start( row, style );
    }
    return {

	refresh : function(){
	    dm.load('/pl/list');
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
			old_row.style.border = '';
		    }
		}
		if ( row == null ){
		    new_row = grid.getRow( dm.getRowCount()-1 );
		    new_row.style.border = '1px solid red';

		} else {
		    new_row = grid.getRow( row );
		    if ( new_row ){
			new_row.style.border = '1px solid red';
		    }
		}
		lastHighlighted = row;
	    }
	},

	dragOver : function( event, src_id ){
	    if ( src_id == 'playlists' ){
		handleDrag( 'borderTop', 'borderBottom', event );
	    } else {
		handleDrag( 'border','border',event );
	    }
	},

	init : function(){
	    lastHighlighted=0;
	    descEl=getEl('playlist-description');
	    dm = new YAHOO.ext.grid.JSONDataModel( {
		root: 'Playlists',
		id: 'id',
		fields: ['description','name']
	    } );
	    dm.addListener('load', onFirstLoad, this, true);

	    cm=new YAHOO.ext.grid.DefaultColumnModel( [
						       {header: "Edit", width: 20,renderer:renderEdit},
						       {header: "Name", width: 150 }
						       ] );
	    cm.setColumnWidth( 0,24 );
	    grid = new SpinnyGrid( 'playlists', dm, cm );
	    grid.reOrderUrl='/pl/reorder';
//		    grid.addListener('cellclick', onPlClick, this, true )
	    grid.addListener('celldblclick', onPlDblClick, this, true );

//	    grid.addListener("enddrag", function(grid,dd,id,e){ Layout.handlePlDrop( dd, id, e ) } );

	    YAHOO.log("PLaylists Grid id: " + grid.id);


	    grid.autoSize = function(){
		SpinnyGrid.superclass.autoSize.call(this);
		model=this.colModel;
		view=this.getView();
		width = view.wrap.clientWidth;
	    }

	    grid.enableDragDrop = true;
	    grid.selfTarget = true;
	    grid.autoSizeColumns = false;
	    grid.selectAfterDrop = true;
	    dm.load('/pl/list');
	    grid.render();
	    selectedRow=0;

	},
	getGrid : function(){
	    return grid;
	},
	selected : function(){
	    return dm.getRowId( selectedRow );
	},
	songDropped : function( event, song_ids ){
	    row=grid.getTargetRow( event );
	    YAHOO.log("Song id " + song_ids + " Dropped on PL row " + row );
	    pl_id=dm.getRowId( row );
	    params='pl_id='+pl_id+'&position=0'
	    for (x=0; x<song_ids.length;++x){
		YAHOO.log( "ADDING ID: " + song_ids[x] );
		params+='&song_id='+song_ids[x];
	    }
	    YAHOO.log("Song id " + song_ids + " Dropped on PL row " + row );

	    var cb = {
		success: function(o){ YAHOO.log( "PL Song add req success" ); 
		    if ( o.argument ){
			Songs.refreshListing();	      
		    }
		},
		argument: ( Playlists.selected() == pl_id ),
		failure: function(o){ YAHOO.log("PL Song add req failed"); }
	    }
	    YAHOO.log( "PL About to send song additions " + params );
//	    YAHOO.util.Connect.asyncRequest( 'POST', '/pl/songs/add', cb, params );
	},

    };

}();

YAHOO.ext.EventManager.onDocumentReady( Playlists.init, Playlists, Playlists, true);
