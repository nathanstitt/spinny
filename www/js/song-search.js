SongSearch = function() {
    var input;
    var dm,cm,grid;
    var initialText = 'Search Terms...';
    handleKey = function(){
	YAHOO.log("Key Press " + input.dom.value );
	dm.baseParams = { 'q':input.dom.value};
	if ( input.dom.value == "" ){
	    dm.removeAll();
	} else {
	    dm.loadPage( 1 );
	}
    }

    handleClick = function(){
	YAHOO.log("Click " + input.dom.value );
	if ( input.dom.value == initialText ){
	    input.dom.value = '';
	    input.removeListener('click', handleClick );
	}
    }

    return {

       init : function(){
	   
	   dm = new YAHOO.ext.grid.JSONDataModel( {
	       root: 'Songs',
	       totalProperty: 'TotalCount',
	       id: 'id',
	       fields: ['title']
	   } );
//	  

	   cm=new YAHOO.ext.grid.DefaultColumnModel
	       ( [
		   {header: "<input type='text' value='"+initialText+"' id='song-search-field'>",  sortable: false}, 
		   ]
		 );

	    grid = new SpinnyGrid( 'song-search-results', dm, cm );
	    grid.reOrderUrl='/pl/songs/reorder';
	    grid.ddText="%0 songs selected";

	    grid.autoSize = function(){
		SpinnyGrid.superclass.autoSize.call(this);
		model=this.colModel;
		view=this.getView();
		width = view.wrap.clientWidth;

	//	YAHOO.log("Autosize Find, width="+width);
		view.fitColumns.defer( 50, view );
	    }
	    grid.addListener('celldblclick', onSongDblClick, this, true );

	   grid.enableDragDrop = true;
	   grid.selfTarget = true;
	   pgv=new YAHOO.ext.grid.PagedGridView();
	   pgv.beforePageText='';
	   pgv.afterPageText='&nbsp;/&nbsp;%0';
	   grid.view = pgv;
	   dm.initPaging('/info/find',10 );
	   grid.render();

	   input = getEl('song-search-field');
	   YAHOO.log("Search el: " + input.dom.value );

	   input.addListener('click', handleClick,this,true );
	   input.addListener('keyup', handleKey,this,true );

//	   YAHOO.util.Event.addListener(oTextbox,"keyup",oSelf._onTextboxKeyUp,oSelf);

//	   var mySchema = ["ResultSet.Result","title"];
//	   var myDataSource = new YAHOO.widget.DS_XHR("/info/find", mySchema); 
//	   var myAutoComp = new YAHOO.widget.AutoComplete("song-search","song-search-results", myDataSource); 
//	   myAutoComp.alwaysShowContainer = true;

	   
       },
	getGrid : function(){
	    return grid;
	},

   }




}();



//YAHOO.ext.EventManager.onDocumentReady( SongSearch.init, SongSearch, true );