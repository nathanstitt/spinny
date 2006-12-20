// create the LayoutExample application (single instance)
var UserAdminDlg = function(){
    // everything in this space is private and only accessible in the HelloWorld block
    
    // define some private variables
    var dialog, showBtn;
    var grid, gp;

    saveAccounts = function(){
	    saveWait.setVisible(true);
	    errorMsg.update('');
	    YAHOO.log("Saving Accounts","info");
	    dm=grid.getDataModel();
	    var users=[];
	    for ( i=0; i< dm.getRowCount(); ++i ){
		    user=new Object;
		    row=dm.getRow(i)
		    user.id=dm.getRowId(i);
		    user.login=row[1];
		    user.password = row[2];
		    user.role=row[3]-0;
		    users[i]=user;
	    }
	    json=YAHOO.ext.util.JSON.encode(users);
	    var cb = {
	    success: function(o){ 
			    o.argument.dm.loadPage( o.argument.dm.loadedPage );
			    saveWait.setVisible(false);
			    dialog.hide();
		    },
	    failure: function(o){ 
			    o.argument.dm.loadPage( o.argument.dm.loadedPage );
			    errorMsg.update('Save Failed: ' + o.responseText);
		    },
	    argument: { 'dm':dm,'dialog':dialog }
	    }
	    YAHOO.util.Connect.asyncRequest( 'POST', '/users/save', cb, "json="+json );

	    //  dialog.hide();
    };
    createUser = function(){
	    YAHOO.log("Create new User", "info");
	    dm=grid.getDataModel();
	    nr=[ '', 'Login', '' ];
	    nr.id=0;
	    dm.addRows( [nr] );
    };

    // return a public interface
    return {
        init : function(){
             showBtn = getEl('show-user-admin-btn');
             showBtn.on('click', this.showDialog, this, true);
	     errorMsg = getEl('user-admin-dlg-error-msg');
	     saveWait = getEl('user-admin-dlg-save-wait');
        },
        onClick: function( grid,row,col ){
	    if ( col ){
		    return;
	    }
	    var cb = {
		    success: function(o){ o.argument[0].getDataModel().removeRow(o.argument[1]); },
		    failure: function(o){ YAHOO.log("User del failed","error"); },
		    argument: [grid,row]
	    };
	    dm=grid.getDataModel();
	    uid=dm.getRowId(row);
	    YAHOO.log( "Deleting user id: " + uid,"info" );
	    if ( uid ){
		    YAHOO.util.Connect.asyncRequest( 'POST', '/users/del', cb, "uid="+uid );
	    } else {
		    dm.removeRow( row );
	    }
	},
        showDialog : function(){
            if(!dialog){ // lazy initialize the dialog and only create it once
                dialog = new YAHOO.ext.LayoutDialog("user-edit-dlg", { 
                        modal:true,
                        width:550,
			animateTarget: showBtn,
                        height:400,
                        shadow:true,
                        minWidth:300,
			autoTabls:false,
                        minHeight:300,
			center: {
	                       autoScroll:true
	                }
                });
                dialog.addKeyListener(27, dialog.hide, dialog);
                dialog.addButton('Cancel', dialog.hide, dialog);
                dialog.addButton('Save', saveAccounts );

		layout = dialog.getLayout();
		layout.beginUpdate();
		function renderDel(val){
			return '<img src="/img/delete.gif" class="link">';
		}
		function renderType(val){
			sel=getEl('role-selector');
			options = sel.getChildrenByTagName('option')
			for ( option in options ){
				dom =  options[option].dom;
				if ( dom.value == val ){
					return dom.text;
				}
			}
			return '';
		}
		var roleEditor = new YAHOO.ext.grid.SelectEditor('role-selector'); 
		var sort = YAHOO.ext.grid.DefaultColumnModel.sortTypes;
		cm=new YAHOO.ext.grid.DefaultColumnModel( [
		    {header: "Rm", width:25,renderer:renderDel},
		{header: "Login", sortable:true,sortType: sort.asUCString, width: 180, editor: new YAHOO.ext.grid.TextEditor() },
		{header: "Password", width: 80, editor: new YAHOO.ext.grid.TextEditor(),tooltip:'Click to sort'},
		{header: "Role", width: 80, editor: roleEditor,sortable:true,sortType: sort.asInt, renderer: renderType },
		{header: "Last Visited", sortable:true,sortType: sort.asFloat, width: 80 }
        	] );
		sm = new YAHOO.ext.grid.SingleSelectionModel();
		//	sm.addListener('selectionchange', onSelection);

		dm = new YAHOO.ext.grid.JSONDataModel( {
			root: 'Users',
			id: 'id',
			fields: ['','login','password','role','last_visit' ]
		} );
		dm.addListener('load', function( dm ){
				YAHOO.log( "Loaded dyna User data", "info" );
			}, this, true );
		dm.initPaging('/users/list',30 );
		dm.setDefaultSort(cm, 3, 'Name');

		grid = new YAHOO.ext.grid.EditorGrid('users-grid', dm, cm, sm );
		grid.render();

		grid.addListener('cellclick', this.onClick, this,true );
		gp=new YAHOO.ext.GridPanel(grid);
		layout.add('center',gp  );

		view = grid.getView();
		var toolbar = view.getPageToolbar();
		toolbar.addSeparator();
		toolbar.addButton({
			className: 'new-topic-button',
					text: "New User",
					click: createUser
					});
		layout.endUpdate();

            }
            dialog.show(showBtn.dom);
	    saveWait.setVisible(false);
	    errorMsg.update('');
	    dm.loadPage(1);
	    cm.setColumnWidth( 0, 25 );
//	    gp.setSize( 500, 400 );
       },


    };
}();

// using onDocumentReady instead of window.onload initializes the application
// when the DOM is ready, without waiting for images and other resources to load
YAHOO.ext.EventManager.onDocumentReady( UserAdminDlg.init, UserAdminDlg, true );