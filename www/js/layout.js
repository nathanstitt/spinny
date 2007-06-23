var flash_applet;

Layout = function(){
    var layout;
    var innerLayout;
    var westLayout;

    var DD=[];
    return {
	setTrack :function( num ){
	    SongsGrid.hiLightRow( num );
	},
	init : function(){
	    flash_applet = new SWFObject("/player.swf", "flash_applet", "17", "17", "9", "#FFFFFF");
	    flash_applet.write("flash_applet");

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

	    layout.beginUpdate();
	    layout.add('north', new Ext.ContentPanel('north', 'North'));

	    westLayout.add('center', new Ext.ContentPanel( 'tagsTree', {title: 'Artists' }));

	    westLayout.add('center', new Ext.ContentPanel( 'dirsTree', {title: 'Dir'}));

	    layout.add('west',  new Ext.NestedLayoutPanel(westLayout), {title:'Playlists'} );

	    PlaylistsGrid.init();



	    layout.endUpdate();

	},
	getApplet : function(){
	    flash_applet = document.getElementById("flash_applet");
	    return flash_applet;
	},
	height : function(){
	    return layout.getViewSize().height;
	},
	addSongsGrid : function( grid ){
	    var gp = new Ext.GridPanel( grid );
	    layout.add( 'center', gp  );
	    layout.addListener('regionresized', function( region,newSize ) { SongsGrid.setHeight( innerLayout.getViewSize().height-newSize ) } );
	},
	addPlGrid : function( grid ){
	    var gp = new Ext.GridPanel( grid );
	    westLayout.add( 'north', gp  );
	    westLayout.addListener('regionresized', function(region,newSize) { this.getView().autoSizeColumns(); }, grid );
	},
	beginDrag : function( type, ids ){
	    DD = { type: type, ids: ids };
	},
	getDragData :function(){
	    return DD;
	}
	

	
    }
}();

Ext.EventManager.onDocumentReady(Layout.init, Layout, true);


