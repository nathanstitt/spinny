// create the LayoutExample application (single instance)
var UserAdminDlg = function(){
    // everything in this space is private and only accessible in the HelloWorld block
    
    // define some private variables
    var dialog, showBtn;
    var grid, gridForm, ds;

    saveAccounts = function(){

	jsonData = "[";
	
	for(i=0;i<UserAdminDlg.ds.getCount();i++) {
	    record = UserAdminDlg.ds.getAt(i);
	    if( record.data.newRecord || record.dirty)  {
		if ( jsonData.length > 1 ){
		    jsonData +=  + ",";
		}
		jsonData += Ext.util.JSON.encode(record.data);
	    }
	}
	jsonData += "]";
	if ( jsonData.length > 2 ){
	    console.info(jsonData);
	    UserAdminDlg.gridForm.submit( {
		waitMsg: 'Saving changes, please wait...',
		url:'/users/save',
		params:{t:'save',data:jsonData},
		success:function(form, action) {
		    UserAdminDlg.dialog.hide();
		    UserAdminDlg.ds.load();
		},
		failure: function(form, action) {
		    errorMsg.update('Save Failed: ' + o.responseText);
		}
	    } );
	}
    };
    createUser = function(){
	UserAdminDlg.ds.add( new Ext.data.Record({
	    id: 0,
	    login: 'New User',
	    password: '******',
	    role: 0,
	    last_visit:'',
	    newRecord:true
	}));
    };

    // return a public interface
    return {
        init : function(){
            showBtn = Ext.get('show-user-admin-btn');
            showBtn.on('click', this.showDialog, this, true);
	    errorMsg = Ext.get('user-admin-dlg-error-msg');
	    saveWait = Ext.get('user-admin-dlg-save-wait');
	    this.showDialog();
        },
        onClick: function( grid,row,col ){
	    if ( col ){
		    return;
	    }
	    var cb = {
		    success: function(o){ o.argument[0].getDataModel().removeRow(o.argument[1]); },
		    failure: function(o){ console.log("User del failed","error"); },
		    argument: [grid,row]
	    };
	    dm=grid.getDataModel();
	    uid=dm.getRowId(row);
	    console.log( "Deleting user id: " + uid,"info" );
	    if ( uid ){
		Ext.lib.Ajax.request( 'POST', '/users/del', cb, "uid="+uid );
	    } else {
		    dm.removeRow( row );
	    }
	},
	hideDialog:function(){
	    this.dialog.hide();
	},
        showDialog : function(){
            if(!this.dialog){ // lazy initialize the dialog and only create it once
                this.dialog = new Ext.LayoutDialog("user-edit-dlg", { 
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
                this.dialog.addKeyListener(27,  UserAdminDlg.hideDialog,   UserAdminDlg );
		this.dialog.addButton('Cancel', UserAdminDlg.hideDialog,   UserAdminDlg );
                this.dialog.addButton('Save',   saveAccounts );

		layout = this.dialog.getLayout();

		this.ds = new Ext.data.Store({
		    proxy: new Ext.data.HttpProxy({url: '/users/list'}),
		    reader: new Ext.data.JsonReader( {
			root: 'Users',
			id: 'id',
		    },[
		       { name: 'login' },
		       { name: 'id' },
		       { name: 'password'},
		       { name: 'role' },
		       { name: 'last_visit' }
		       ] )
		    } );


		var cm=new Ext.grid.ColumnModel( [ {
		    header: "Rm",
		    width:25,
		    fixed:true,
		    renderer: function(){ return '<img src="/images/custom/delete.gif" class="link">'; }
		},{
		    header: "Login",
		    sortable:true,
		    dataIndex: 'login',
		    editor: new Ext.grid.GridEditor( new Ext.form.TextField({ allowBlank:false }) )
		},{
		    header: "Password",
		    width: 80,
		    dataIndex: 'password',
		    editor: new Ext.grid.GridEditor( new Ext.form.TextField({ allowBlank:false }) ),
		    tooltip:'Click to sort'
		},{
		    header: "Role",
		    width: 80,
		    dataIndex: 'role',
		    renderer:  function(data, cell, record, rowIndex, columnIndex, store) {
			switch(data) {
			    case 0:
			    return "ReadOnly";
			    case 1:
			    return "Modify Playlists";
			    case 2:
			    return "Admin";
			}
		    },
		    editor: new Ext.grid.GridEditor( new Ext.form.ComboBox( {
			typeAhead:true,
			triggerAction:'all',
			lazyRender:true,
			transform:'role-selector' })),
		    sortable:true
		},{
		    header: "Last Visited",
		    sortable:true,
		    align:'center',
		    dataIndex: 'last_visit',
		    fixed:true,
		    width: 80 
		} ] );

		this.grid = new Ext.grid.EditorGrid('users-grid', {
		    ds: this.ds,
		    cm: cm,
		    selModel: new Ext.grid.RowSelectionModel({singleSelect:true}),
		    autoSizeColumns:true,
		    monitorWindowResize: true,
		    autoWidth:true,
		    enableDragDrop:false
		});
		this.gridForm = new Ext.BasicForm( Ext.get("updategrid"), { } );

		var gp=new Ext.GridPanel(this.grid);
		layout.add('center',gp  );

		this.grid.render();

		var gridFoot = this.grid.getView().getFooterPanel(true);

                // add a paging toolbar to the grid's footer
		this.paging = new Ext.PagingToolbar( gridFoot, this.ds, {
		    pageSize: 25,
		    displayInfo: true,
		    displayMsg: 'Displaying Users {0} - {1} of {2}',
		    emptyMsg: "No users to display"
		});
		this.paging.add('-', {
		    text: 'Add User',
		    handler: createUser
		});
		this.ds.load( { params: { start:0, limit:25  } } );

            }
            this.dialog.show(showBtn.dom);
	    saveWait.setVisible(false);
	    errorMsg.update('');

       },


    };
}();


Ext.EventManager.onDocumentReady( UserAdminDlg.init, UserAdminDlg, true );