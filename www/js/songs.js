function Songs() {

	this.dm = new YAHOO.ext.grid.JSONDataModel( {
	root: 'Songs',
 		id: 'id',
 		fields: ['tr','tt','at','al','ln']
	} );

	this.cm=new YAHOO.ext.grid.DefaultColumnModel( [
 	                                             {header: "Track",  sortable: true}, 
 		                                     {header: "Title",  sortable: true}, 
 		                                     {header: "Artist", sortable: true},
 		                                     {header: "Album",  sortable: true}, 
 	                                             {header: "Length", sortable: true}
	] );
 	this.dm.addListener('loadException', function( grid,err ){
 			YAHOO.log( "Song Load excp Msgs:" + err.message );
 			YAHOO.log( "Song Load excp Desc:" + err.description );
 			YAHOO.log( "Song Load excp Numb:" + err.number );
 			for ( var item in err){
// //				YAHOO.log( "Song Load excp:" + item );
 			}
 		});


	this.dm.addListener('load', function(){
		    YAHOO.log( "Loaded dyna Playlist data");

	});

}


Songs.prototype.init = function( el ){
	this.grid = new SpinnyGrid( el, this.dm, this.cm );
	YAHOO.log("PLaylists Grid id: " + this.grid.id);
	this.grid.enableDragDrop = true;
	this.grid.selfTarget = true;
	this.grid.autoSizeColumns = false;
	this.grid.selectAfterDrop = true;
	this.dm.load('/pl/songs');
	this.grid.render();
	this.grid.on('rowdblclick', function( grid, row ){
			YAHOO.log("Dblclick: " + row );
		}
	);
}

