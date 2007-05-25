
Layout = function(){
    var layout;
    var innerLayout;
    var westLayout;
    var DD=[];
    return {

	init : function(){
	    layout = new Ext.BorderLayout( document.body, {
		hideOnLayout: true,
		north: {
		    split:false,
		    initialSize: 25,
		    titlebar: false
		},
		west: {
		    split:true,
		    initialSize: 200,
		    minSize: 175,
		    maxSize: 400,
		    titlebar: true,
		    collapsible: true,
		    autoScroll:true,
		    title: '<img style="vertical-align: middle; cursor: pointer;" id="playlists-show-btn" onClick="PlaylistsGrid.addNew();" src="/images/custom/add.gif"> Playlists'

		},
		center: {
		    split:true,
		    autoScroll:true,
		    titlebar: false,
		    collapsible: false
		}
	    });
	    innerLayout = new Ext.BorderLayout('center-content', {
		north: {
		    autoScroll:true,
		    tabPosition: 'top',
		    split: false,
		    initialSize: 300,
		    split:true,
		    closeOnTab:true
		},
		center: {
		    minSize: 100,
		    autoScroll:true,
		    titlebar: false
		}
		
	    });
	    westLayout = new Ext.BorderLayout('west-content', {
		north: {
		    autoScroll:true,
		    tabPosition: 'top',
		    split: false,
		    initialSize: 105,
		    split:true
		},
		center: {
		    minSize: 100,
		    autoScroll:true,
		    titlebar: false
		}
	    });
	    lyricsPanel = new Ext.ContentPanel('center1', {title: 'Lyrics', closable: false});
	    innerLayout.add( 'north', lyricsPanel );
	    artistPanel = new Ext.ContentPanel('center2', {title: 'Artist Info', closable: true})
	    innerLayout.add( 'north', artistPanel );

	    layout.beginUpdate();
	    layout.add('north', new Ext.ContentPanel('north', 'North'));



	    westLayout.add('center', new Ext.ContentPanel( 'tagsTree', {title: 'Artists' }));

	    westLayout.add('center', new Ext.ContentPanel( 'dirsTree', {title: 'Dir'}));

//	    SongSearch.init();

//	    westLayout.add('center', new Ext.GridPanel( SongSearch.getGrid(), {title: 'Find'}));
	    
//	    westLayout.add('north',  new Ext.GridPanel( Playlists.getGrid() ) );
	    layout.add('center', new Ext.NestedLayoutPanel(innerLayout) );
	    layout.add('west',  new Ext.NestedLayoutPanel(westLayout), {title:'Playlists'} );


	    layout.getRegion('center').showPanel('center1');

	    layout.endUpdate();

/* 			setTimeout('Layout.plGrid.autoSize();', 500); */
// 			setTimeout('Layout.songsGrid.reload();', 500); 

//	    this.trees = new Trees();
//	    this.trees.init();

	},
	addSongsGrid : function( grid ){
	    var gp = new Ext.GridPanel( grid );
	    innerLayout.add( 'center', gp  );
	    innerLayout.addListener('layout', function(region,newSize) { this.getView().autoSizeColumns(); }, grid );
	},
	addPlGrid : function( grid ){
	    var gp = new Ext.GridPanel( grid );
	    westLayout.add( 'north', gp  );
	    westLayout.addListener('layout', function(region,newSize) { this.getView().autoSizeColumns(); }, grid );
	},
	beginDrag : function( type, ids ){
	    console.log( "Begin drag " + type + " : " + ids );
	    DD = { type: type, ids: ids };
	},
	getDragData :function(){
	    return DD;
	}
	

// 	toggleWest : function(link){
// 	    var west = layout.getRegion('west');
// 	    if(west.isVisible()){
// 		west.hide();
// 		link.innerHTML = 'Show Playlists'
// 	    }else{
// 		west.show();
// 		link.innerHTML = 'Hide Playlists';
// 	    }
// 	},
// 	handleDrag : function( dd, dest_id, src_id, event ){
// 	    if ( dest_id == 'playlists' ){
// 		Playlists.dragOver( event, src_id );
// 	    } else if ( dest_id == 'songs' ){
// 		Songs.dragOver( event, src_id );
// 	    } else {
// 		dd.setDropStatus(false);
// 	    }
// 	},
// 	handleDrop : function( dst_id, src_id, event, ids ){
// 	    HighlightEvents.cancel();
// 	    if ( dst_id == 'playlists' ){
// 		Playlists.songsDropped( event, ids );
// 	    } else if ( dst_id == 'songs' && src_id == 'playlists' ){
// 		Songs.playlistDropped( event, ids );
// 	    } else if ( dst_id == 'songs' ) {
// 		Songs.songDropped( event, ids );
// 	    }
// 	},
// 	showSongInfo : function( song_id, artist ){
// 	    link = 'http://en.wikipedia.org/wiki/' + encodeURIComponent( artist );

// 	    id='art-'+song_id;	    
//   	    var iframe = Ext.DomHelper.append( document.body, 
//     		            {tag: 'iframe', frameBorder: 0, src: link, id: 'frm'+id } );

//  	    panel = new Ext.ContentPanel(iframe,{
// 		title: artist, fitToFrame:true, closable:true } );
// 	    innerLayout.add( 'north', panel );
// 	}	
    }
}();

Ext.EventManager.onDocumentReady(Layout.init, Layout, true);


