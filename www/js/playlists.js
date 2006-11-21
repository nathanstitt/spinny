


function Playlists() {
	function renderEdit(val){
		return '<img src="/img/edit.gif">';
	}

	this.dm = new YAHOO.ext.grid.JSONDataModel( {
	  root: 'Playlists',
	  id: 'id',
	  fields: ['description','name']
	} );

	this.cm=new YAHOO.ext.grid.DefaultColumnModel( [
		{header: "Edit", width: 20,renderer:renderEdit},
		{header: "Name", width: 300 }
	] );
	this.cm.setColumnWidth( 0,24 );

	this.dm.addListener('load', function(){
		    YAHOO.log( "Loaded dyna Playlist data");

	});
}

Playlists.prototype.showDialog = function( index ){
                this.editDlg.render();
}

Playlists.prototype.init = function( el ){

	this.grid = new SpinnyGrid( el, this.dm, this.cm );
	YAHOO.log("PLaylists Grid id: " + this.grid.id);
	this.grid.enableDragDrop = true;
	this.grid.selfTarget = true;
	this.grid.autoSizeColumns = false;
	this.grid.selectAfterDrop = true;
	this.dm.load('/pl/list');
	this.grid.render();
	this.grid.on('rowdblclick', function( grid, row ){
			
		}
	);

}



