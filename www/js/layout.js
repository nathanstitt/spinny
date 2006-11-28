

Layout = function(){
	var layout;
	var songsGrid;
	var plGrid;
	return {
	init : function(){
			this.layout = new YAHOO.ext.BorderLayout(document.body, {
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
							title: '<img style="vertical-align: middle; cursor: pointer;" id="playlists-show-btn" onClick="PlaylistsDlg.showDialog();" src="/img/add.gif"> Playlists'

							},
						center: {
					split:true,
							autoScroll:true,
							titlebar: false,
							collapsible: false
							},
						east: {
					split:true,
							titlebar: false,
							autoScroll:false,
							titlebar: true,
							collapsible: true,
							autoScroll:true,
							initialSize: 200,
							minSize: 175

							}
				});
	                var innerLayout = new YAHOO.ext.BorderLayout('center-content', {
				north: {
					autoScroll:true,
							tabPosition: 'top',
							split: false,
							initialSize: 300,
							split:true
							},
						center: {
					minSize: 100,
							autoScroll:true,
							titlebar: false
							}
				
				});
			var westLayout = new YAHOO.ext.BorderLayout('west-content', {
				north: {
					autoScroll:true,
							tabPosition: 'top',
							split: false,
							initialSize: 100,
							split:true
							},
						center: {
					minSize: 100,
							autoScroll:true,
							titlebar: false
							}
				});
			innerLayout.add( 'north', new YAHOO.ext.ContentPanel('center1', {title: 'Info Panel #1', closable: false}));
			innerLayout.add( 'north', new YAHOO.ext.ContentPanel('center2', {title: 'Info Panel #2', closable: false}));

			this.songsGrid = new Songs;
			this.songsGrid.init('songs');
			this.plgp=new YAHOO.ext.GridPanel(this.songsGrid.grid);

			innerLayout.add( 'center', this.plgp );

			this.layout.beginUpdate();
			this.layout.add('north', new YAHOO.ext.ContentPanel('north', 'North'));

	                this.layout.add('east', new YAHOO.ext.ContentPanel('console', {title: 'Message Console', closable: true}));

	                westLayout.add('center', new YAHOO.ext.ContentPanel( 'searchSongs', {title: 'Find'}));
	                westLayout.add('center', new YAHOO.ext.ContentPanel( 'albumsTree', {title: 'Album' }));
	                westLayout.add('center', new YAHOO.ext.ContentPanel('artistsTree', {title: 'Artist' }));
	                westLayout.add('center', new YAHOO.ext.ContentPanel( 'dirsTree', {title: 'Dir'}));
			
			this.plGrid = new Playlists;
			this.plGrid.init('playlists');
			westLayout.add('north',  new YAHOO.ext.GridPanel( this.plGrid.grid ) );


	                this.layout.add('center', new YAHOO.ext.NestedLayoutPanel(innerLayout) );
			this.layout.add('west',  new YAHOO.ext.NestedLayoutPanel(westLayout), {title:'Playlists'} );


	                this.layout.getRegion('center').showPanel('center1');

	                this.layout.endUpdate();

/* 			setTimeout('Layout.plGrid.autoSize();', 500); */
// 			setTimeout('Layout.songsGrid.reload();', 500); 

		       this.trees = new Trees();
		       this.trees.init();
		       this.trees.draw();
		},

			toggleWest : function(link){
	                var west = this.layout.getRegion('west');
	                if(west.isVisible()){
				west.hide();
				link.innerHTML = 'Show West Region '
					}else{
				west.show();
				link.innerHTML = 'Hide West Region';
	                }
		}
	}
}();

YAHOO.ext.EventManager.onDocumentReady(Layout.init, Layout, true);


