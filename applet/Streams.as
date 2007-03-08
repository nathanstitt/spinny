


class Streams {

        var clips:Array = new Array;
        var sounds:Array = new Array;
        var stream_url:String;
        var current:Boolean = false;
        var volume:Number = 100;
	var interval;
	var playTime=600;
	var bufferTime = 5;

	var stoppedSlash;
        var playing:Boolean = true;

	function Streams( strm, stopped_obj ){
		stream_url = strm;
		stoppedSlash = stopped_obj;
		stoppedSlash._visible = false;

		clips[ false ] = _root.createEmptyMovieClip("snd1", 1);
 		clips[ true ] = _root.createEmptyMovieClip("snd2", 2);

 		sounds[ current ] = new Sound( clips[ current ] );
		sounds[ current ].setVolume( volume );

 		sounds[ current ].loadSound( _root.stream_url+";stream"+Math.random()+".mp3", true );

		interval = setInterval( this, 'deleteSound',1000 );
		debug("initialized");
	}
	

	function debug( msg ){
		_root.tra = msg;
	}

	function stop(){
		stoppedSlash._visible = playing;
		clearInterval( interval );
		if ( playing ){
			debug( "stopping" );
			for( var s in sounds ){
				sounds[ s ] = null;
 				clips[  s ].removeMovieClip();
 				clips[  s ] = _root.createEmptyMovieClip("snd"+Number(s), Number(s)+1 );
			}
		} else {
			debug( "starting" );
			sounds[ current ] = new Sound( clips[ current ] );
			sounds[ current ].setVolume( volume );
			sounds[ current ].loadSound( _root.stream_url+";stream"+Math.random()+".mp3", true );
			interval = setInterval( this, 'deleteSound',1000 );
		}
		playing=!playing;
	}

	function deleteSound () {
		debug( "delete " + Number( ! current ) );

		delete( sounds[ ! current ] );
		clips[  ! current ].removeMovieClip();
		clips[  ! current ] = _root.createEmptyMovieClip("snd" + Number(!current), Number(!current)+1 );

		clearInterval( interval );

		interval = setInterval( this, 'loadSound', playTime*1000 );
	}

	function loadSound() {
		debug( "load " + Number( ! current ) );

		sounds[ ! current ] = new Sound( clips[ ! current ] );
		sounds[ ! current ].setVolume( volume );
		sounds[ ! current ].loadSound( stream_url + ";stream"+Math.random()+".mp3", true );
		sounds[ ! current ].setVolume(0);
		clearInterval( interval );

		interval = setInterval( this, 'swap', bufferTime*1000 );
	}

	function setVolume( new_volume ){
		volume += new_volume;
		if ( volume > 100 ){
			volume = 100;
		}
		if ( volume < 0 ){
			stop();
		} else {
			debug( "Vol: " + volume + " : " + new_volume );
			sounds[ current ].setVolume( volume );
		}
		return volume;
	}
	function getVolume(){
		return volume;
	}

	function swap() {
		debug( "swapping" );
		clearInterval( interval );
		current = ! current;
		sounds[ ! current ].setVolume( 0 );
		sounds[ current ].setVolume(volume);
		interval = setInterval( this, 'deleteSound', 1000 );
	}

}


