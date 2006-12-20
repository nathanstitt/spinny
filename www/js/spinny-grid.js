


HighlightEvents = function(){
    var current;
    var timeout;
    var row,effect;
}

HighlightEvents.start = function( row, effect ){
    if ( row != this.row || effect != this.effect ){
	this.effect = effect;
	this.cancel();
	this.row = row;
	this.current = new Highlight( row, effect );
	this.timeout = setTimeout( 'HighlightEvents.cancel()', 5000 );
    }
}

HighlightEvents.cancel = function(){
    if ( this.current ){
	this.current.revert();
	clearTimeout( this.timeout );
	this.row = this.timeout = this.current = null;
    }
}

function Highlight( row, style ){
    this.style = style;
    this.row = row;
    this.old_style_value = YAHOO.util.Dom.getStyle( this.row, style );
    YAHOO.util.Dom.setStyle( this.row, this.style, '1px solid red' );
}


Highlight.prototype.revert = function(){
    YAHOO.util.Dom.setStyle( this.row, this.style, this.old_style_value );
}



SpinnyGrid = function(){
    SpinnyGrid.superclass.constructor.apply(this, arguments);
    this.enableDragDrop = true;
    this.addListener('dragdrop', this.onRowsDropped, this, true);
    this.addListener('enddrag',  function(){ HighlightEvents.cancel() } );
    this.addListener('dragover', this.onDrag, this ,true );

    // enables dropping rows from this grid on this grid
    // required for row ordering via D&D
    this.selfTarget = true;
};


YAHOO.extendX(SpinnyGrid, YAHOO.ext.grid.Grid, {
    render : function(){
        SpinnyGrid.superclass.render.call(this);
        if(this.selfTarget){
            this.target = new YAHOO.util.DDTarget(this.container.id, 'GridDD');
        }
    },
    
    transferRows : function(indexes, targetGrid, targetRow, copy){
        var dm = this.dataModel;
        var targetDm = targetGrid.getDataModel();
        var rowData = dm.getRows(indexes);
        if(!copy){
            indexes.sort();
            for(var i = 0, len = indexes.length; i < len; i++) {
           	    dm.removeRow(indexes[i]-i);
            }
        }
        this.selModel.unlock();
        this.selModel.clearSelections();
        this.selModel.lock();
        var selStart;
        if(targetRow != null && targetRow < targetDm.getRowCount()){
            targetDm.insertRows(targetRow, rowData);
            selStart = targetRow;
        }else{
            targetDm.addRows(rowData);
            selStart = targetDm.getRowCount()-indexes.length;
        }
	var params=this.reqParams+"&";
	for( var i=0; i<targetDm.getRowCount();++i){
		params+=targetDm.getRowId(i)+"="+i+"&";
	}
	var cb = {
	success: function(o){ YAHOO.log( "RE-Order req success" ); },
        failure: function(o){ YAHOO.log("RE-Order req failed"); }
	}
	YAHOO.log( "About to conn to: " + this.reOrderUrl );
	YAHOO.util.Connect.asyncRequest( 'POST', this.reOrderUrl, cb, params );
    },
    
    getTargetRow : function(e){
        YAHOO.util.Event.stopEvent(e);
        var xy = YAHOO.util.Event.getXY(e);
        var cell = this.getView().getCellAtPoint(xy[0], xy[1]);
        return cell ? Math.max(0, cell[1]) : null;
    },
    onDrag :function(grid, dd, dest_id, e ){
	Layout.handleDrag( dd, dest_id, this.id, e );
    },
    onRowsDropped : function(grid, dd, dest_id, e){
	if( dest_id == this.id ) {
           var indexes = this.getSelectedRowIndexes();
           this.transferRows(indexes, this, this.getTargetRow(e));
       } else {
	   Layout.handleDrop( dest_id, this.id, e, this.getSelectedIds() );
       }
    },
    getSelectedIds : function(){
	var rows = this.getSelectedRows();
	ret=[];
	for( var i=0; i<rows.length; ++i ){
	    ret.push( this.dataModel.getRowId( rows[i].rowIndex ) );
	}
	YAHOO.log("GET SEL IDS RET: " + ret );
	return ret;
    }
});