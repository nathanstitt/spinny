
// create the HelloWorld application (single instance)
var PlaylistsDlg = function(){
    // everything in this space is private and only accessible in the HelloWorld block
    
    // define some private variables
    var dialog, showBtn;
    
    // return a public interface
    return {
        init : function(){
		    txtDesc = getEl('playlist-dlg-desc');
		    txtName = getEl('playlist-dlg-name');
		    wait = getEl('dlg-save-wait');
		    error = getEl('dlg-save-error');
		    errorMsg = getEl('dlg-error-msg');
        },
        showDialog : function(){
             showBtn = getEl('playlists-show-btn');

            if(!dialog){ // lazy initialize the dialog and only create it once
                dialog = new YAHOO.ext.BasicDialog("playlists-dlg", { 
                        //modal:true,
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
	    wait.setVisible(false);
	    error.setVisible(false);
            dialog.show(showBtn.dom);
	    txtDesc.dom.value = '';
        },
        save : function(){
		    wait.setVisible(true);
		    YAHOO.util.Connect.setForm(document.getElementById('playlist-dlg-form'));
		    var success = function(o){
			    wait.setVisible(false);
			    dialog.hide;
			    Layout.plGrid.add( eval('(' + o.responseText + ')') );

		    };
		    var failure = function(o){
			    wait.setVisible(false);
			    error.setVisible(true);
			    errorMsg.update('Save Failed: ' + o.responseText);
		    };
		    YAHOO.util.Connect.asyncRequest('POST', '/pl/create',
						    {success: success, failure: failure});
        }
    };
}();

YAHOO.ext.EventManager.onDocumentReady(PlaylistsDlg.init, PlaylistsDlg, true);

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
		{header: "Name", width: 150 }
	] );
	this.cm.setColumnWidth( 0,24 );
}


Playlists.prototype.onDblClick = function(grid, rowIndex, colIndex){
	YAHOO.log( "Cell DBL Click on " + colIndex  );
}

Playlists.prototype.add = function( pl ){
	YAHOO.log( "Begin Adding pl" );
	this.reload();
}

Playlists.prototype.reload = function(){
	this.dm.load('/pl/list');
}

Playlists.prototype.init = function( el ){

	this.grid = new SpinnyGrid( el, this.dm, this.cm );
	this.grid.reOrderUrl='/pl/reorder';

	this.grid.addListener('celldblclick', this.onDblClick, this,true );

	YAHOO.log("PLaylists Grid id: " + this.grid.id);
	this.grid.autoSize = function(){
		SpinnyGrid.superclass.autoSize.call(this);
		this.colModel.setColumnWidth(1, this.getView().wrap.clientWidth - 30 );
	};

	this.grid.enableDragDrop = true;
	this.grid.selfTarget = true;
	this.grid.autoSizeColumns = false;
	this.grid.selectAfterDrop = true;
	this.dm.load('/pl/list');
	this.grid.render();

}



