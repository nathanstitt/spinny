
/*
------------------------------------------------------------------
// File: \yutil.js
------------------------------------------------------------------
*/
YAHOO.namespace('ext');
YAHOO.namespace('ext.util');
YAHOO.namespace('ext.grid');
YAHOO.ext.Strict = (document.compatMode == 'CSS1Compat');
YAHOO.ext.SSL_SECURE_URL = 'javascript:false';

/**
 * Creates a callback that passes arguments[0], arguments[1], arguments[2], ...
 * Call directly on any function. Example: <code>myFunction.createCallback(myarg, myarg2)</code>
 * Will create a function that is bound to those 2 args.
 * @addon
 * @return {Function} The new function
*/
Function.prototype.createCallback = function(/*args...*/){
    // make args available, in function below
    var args = arguments;
    var method = this;
    return function() {
        return method.apply(window, args);
    }
};

/**
 * Creates a delegate (callback) that sets the scope to obj.
 * Call directly on any function. Example: <code>this.myFunction.createDelegate(this)</code>
 * Will create a function that is automatically scoped to this.
 * @addon
 * @param {Object} obj The object for which the scope is set
 * @param {<i>Array</i>} args (optional) Overrides arguments for the call. (Defaults to the arguments passed by the caller)
 * @param {<i>Boolean/Number</i>} appendArgs (optional) if True args are appended to call args instead of overriding, 
 *                                             if a number the args are inserted at the specified position
 * @return {Function} The new function
 */
Function.prototype.createDelegate = function(obj, args, appendArgs){
    var method = this;
    return function() {
        var callargs = args || arguments;
        if(appendArgs === true){
            callArgs = Array.prototype.slice.call(arguments, 0);
            callargs = callArgs.concat(args);
        }else if(typeof appendArgs == 'number'){
            callargs = Array.prototype.slice.call(arguments, 0); // copy arguments first
            var applyArgs = [appendArgs, 0].concat(args); // create method call params
            Array.prototype.splice.apply(callargs, applyArgs); // splice them in
        }
        return method.apply(obj || window, callargs);
    }
};

Function.prototype.defer = function(millis, obj, args, appendArgs){
    return setTimeout(this.createDelegate(obj, args, appendArgs), millis)
};

/**
 * Create a combined function call sequence of the original function + the passed function.
 * The resulting function returns the results of the original function.
 * The passed fcn is called with the parameters of the original function
 * @addon
 * @param {Function} fcn The function to sequence
 * @param {<i>Object</i>} scope (optional) The scope of the passed fcn (Defaults to scope of original function or window)
 * @return {Function} The new function
 */
Function.prototype.createSequence = function(fcn, scope){
    if(typeof fcn != 'function'){
        return this;
    }
    var method = this;
    return function() {
        var retval = method.apply(this || window, arguments);
        fcn.apply(scope || this || window, arguments);
        return retval;
    }
};

/**
 * Creates an interceptor function. The passed fcn is called before the original one. If it returns false, the original one is not called.
 * The resulting function returns the results of the original function.
 * The passed fcn is called with the parameters of the original function.
 * @addon
 * @param {Function} fcn The function to call before the original
 * @param {<i>Object</i>} scope (optional) The scope of the passed fcn (Defaults to scope of original function or window)
 * @return {Function} The new function
 */
Function.prototype.createInterceptor = function(fcn, scope){
    if(typeof fcn != 'function'){
        return this;
    }
    var method = this;
    return function() {
        fcn.target = this;
        fcn.method = method;
        if(fcn.apply(scope || this || window, arguments) === false) 
            return;
        return method.apply(this || window, arguments);;
    }
};

/**
 * @class
 * @constructor
 */
YAHOO.ext.util.Browser = new function(){
	var ua = navigator.userAgent.toLowerCase();
	/** @type Boolean */
	this.isOpera = (ua.indexOf('opera') > -1);
   	/** @type Boolean */
	this.isSafari = (ua.indexOf('webkit') > -1);
   	/** @type Boolean */
	this.isIE = (window.ActiveXObject);
   	/** @type Boolean */
	this.isIE7 = (ua.indexOf('msie 7') > -1);
   	/** @type Boolean */
	this.isGecko = !this.isSafari && (ua.indexOf('gecko') > -1);
	
	if(ua.indexOf("windows") != -1 || ua.indexOf("win32") != -1){
	    this.isWindows = true;
	}else if(ua.indexOf("macintosh") != -1){
		this.isMac = true;
	}
}();

/**
 * Enable custom handler signature and event cancelling. Using fireDirect() instead of fire() calls the subscribed event handlers 
 * with the exact parameters passed to fireDirect, instead of the usual (eventType, args[], obj). IMO this is more intuitive 
 * and promotes cleaner code. Also, if an event handler returns false, it is returned by fireDirect and no other handlers will be called.<br>
 * Example:<br><br><pre><code>
 * if(beforeUpdateEvent.fireDirect(myArg, myArg2) !== false){
 *     // do update
 * }</code></pre>
 * @addon
 */
YAHOO.util.CustomEvent.prototype.fireDirect = function(){
    var len=this.subscribers.length;
    for (var i=0; i<len; ++i) {
        var s = this.subscribers[i];
        if(s){
            var scope = (s.override) ? s.obj : this.scope;
            if(s.fn.apply(scope, arguments) === false){
                return false;
            }
        }
    }
    return true;
};

YAHOO.extendX = function(subclass, superclass, overrides){
    YAHOO.extend(subclass, superclass);
    subclass.override = function(o){
        YAHOO.override(subclass, o);
    };
    subclass.prototype.override = function(o){
        for(var method in o){
            this[method] = o[method];
        }  
    };
    if(overrides){
        subclass.override(overrides);
    }
};

YAHOO.override = function(origclass, overrides){
    if(overrides){
        var p = origclass.prototype;
        for(var method in overrides){
            p[method] = overrides[method];
        }
    }
};

/**
 * @class
 * Provides a convenient method of performing setTimeout where a new
 * timeout cancels the old timeout. An example would be performing validation on a keypress.
 * You can use this class to buffer
 * the keypress events for a certain number of milliseconds, and perform only if they stop
 * for that amount of time.
 * @constructor The parameters to this constructor serve as defaults and are not required.
 * @param {<i>Function</i>} fn (optional) The default function to timeout
 * @param {<i>Object</i>} scope (optional) The default scope of that timeout
 * @param {<i>Array</i>} args (optional) The default Array of arguments
 */
YAHOO.ext.util.DelayedTask = function(fn, scope, args){
    var timeoutId = null;
    
    /**
     * Cancels any pending timeout and queues a new one
     * @param {Number} delay The milliseconds to delay
     * @param {Function} newFn Overrides function passed to constructor
     * @param {Object} newScope Overrides scope passed to constructor
     * @param {Array} newArgs Overrides args passed to constructor
     */
    this.delay = function(delay, newFn, newScope, newArgs){
        if(timeoutId){
            clearTimeout(timeoutId);
        }
        fn = newFn || fn;
        scope = newScope || scope;
        args = newArgs || args;
        timeoutId = setTimeout(fn.createDelegate(scope, args), delay);
    };
    
    /**
     * Cancel the last queued timeout
     */
    this.cancel = function(){
        if(timeoutId){
            clearTimeout(timeoutId);
            timeoutId = null;
        }
    };
};

YAHOO.ext.util.Observable = function(){};
YAHOO.ext.util.Observable.prototype = {
    fireEvent : function(){
        var ce = this.events[arguments[0].toLowerCase()];
        return ce.fireDirect.apply(ce, Array.prototype.slice.call(arguments, 1));
    },
    
    addListener : function(eventName, fn, scope, override){
        eventName = eventName.toLowerCase();
        if(!this.events[eventName]){
            // added for a better message when subscribing to wrong event
            throw 'You are trying to listen for an event that does not exist: "' + eventName + '".';
        }
        this.events[eventName].subscribe(fn, scope, override);
    },
    
    delayedListener : function(eventName, fn, scope, delay){
        var newFn = function(){
            setTimeout(fn.createDelegate(scope, arguments), delay || 1);
        }
        this.addListener(eventName, newFn);
        return newFn;
    },
    
    removeListener : function(eventName, fn, scope){
        this.events[eventName.toLowerCase()].unsubscribe(fn, scope);
    }
};
YAHOO.ext.util.Observable.prototype.on = YAHOO.ext.util.Observable.prototype.addListener;

YAHOO.ext.util.Config = {
    apply : function(obj, config){
        if(config){
            for(var prop in config){
                obj[prop] = config[prop];
            }
        }
        return obj;
    }
};

if(!String.escape){
    /** @ignore */
    String.escape = function(string) {
        return string.replace(/('|\\)/g, "\\$1");
    };
};

/**
 * Left pads a string to size with ch
 */
String.leftPad = function (val, size, ch) {
    var result = new String(val);
    if (ch == null) {
        ch = " ";
    }
    while (result.length < size) {
        result = ch + result;
    }
    return result;
};

/*
------------------------------------------------------------------
// File: \MixedCollection.js
------------------------------------------------------------------
*/
/**
 * Collection class that maintains both numeric indexes and keys and exposes events
 */
YAHOO.ext.util.MixedCollection = function(allowFunctions){
    this.items = [];
    this.keys = [];
    this.events = {
        'clear' : new YAHOO.util.CustomEvent('clear'),
        'add' : new YAHOO.util.CustomEvent('add'),
        'replace' : new YAHOO.util.CustomEvent('replace'),
        'remove' : new YAHOO.util.CustomEvent('remove')
    }
    this.allowFunctions = allowFunctions === true;
};

YAHOO.extendX(YAHOO.ext.util.MixedCollection, YAHOO.ext.util.Observable, {
    allowFunctions : false,
    add : function(key, o){
        if(arguments.length == 1){
            o = arguments[0];
            key = this.getKey(o);
        }
        this.items.push(o);
        if(typeof key != 'undefined' && key != null){
            this.items[key] = o;
            this.keys.push(key);
        }
        this.fireEvent('add', this.items.length-1, o, key);
        return o;
    },
    
    getKey : function(o){
         return null;  
    },
    
    replace : function(key, o){
        if(typeof this.items[key] == 'undefined'){
            return this.add(key, o);
        }
        var old = this.items[key];
        if(typeof key == 'number'){ // array index key
            this.items[key] = o;
        }else{
            var index = this.indexOfKey(key);
            this.items[index] = o;
            this.items[key] = o;
        }
        this.fireEvent('replace', key, old, o);
        return o;
    },
    
    addAll : function(objs){
        if(arguments.length > 1 || objs instanceof Array){
            var args = arguments.length > 1 ? arguments : objs;
            for(var i = 0, len = args.length; i < len; i++){
                this.add(args[i]);
            }
        }else{
            for(var key in objs){
                if(this.allowFunctions || typeof objs[key] != 'function'){
                    this.add(objs[key], key);
                }
            }
        }
    },
    
    each : function(fn, scope){
        for(var i = 0, len = this.items.length; i < len; i++){
            fn.call(scope || window, this.items[i]);
        }
    },
    
    eachKey : function(fn, scope){
        for(var i = 0, len = this.keys.length; i < len; i++){
            fn.call(scope || window, this.keys[i], this.items[i]);
        }
    },
    
    find : function(fn, scope){
        for(var i = 0, len = this.items.length; i < len; i++){
            if(fn.call(scope || window, this.items[i])){
                return this.items[i];
            }
        }
        return null;
    },
    
    insert : function(index, key, o){
        if(arguments.length == 2){
            o = arguments[1];
            key = this.getKey(o);
        }
        if(index >= this.items.length){
            return this.add(o, key);
        }
        this.items.splice(index, 0, o);
        if(typeof key != 'undefined' && key != null){
            this.items[key] = o;
            this.keys.splice(index, 0, key);
        }
        this.fireEvent('add', index, o, key);
        return o;
    },
    
    remove : function(o){
        var index = this.indexOf(o);
        this.items.splice(index, 1);
        if(typeof this.keys[index] != 'undefined'){
            var key = this.keys[index];
            this.keys.splice(index, 1);
            delete this.items[key];
        }
        this.fireEvent('remove', o);
        return o;
    },
    
    removeAt : function(index){
        this.items.splice(index, 1);
        var key = this.keys[index];
        if(typeof key != 'undefined'){
             this.keys.splice(index, 1);
             delete this.items[key];
        }
        this.fireEvent('remove', o, key);
    },
    
    removeKey : function(key){
        var o = this.items[key];
        var index = this.indexOf(o);
        this.items.splice(index, 1);
        this.keys.splice(index, 1);
        delete this.items[key];
        this.fireEvent('remove', o, key);
    },
    
    getCount : function(){
        return this.items.length;  
    },
    
    indexOf : function(o){
        if(!this.items.indexOf){
            for(var i = 0, len = this.items.length; i < len; i++){
                if(this.items[i] == o) return i;
            }
            return -1;
        }else{
            return this.items.indexOf(o);
        }
    },
    
    indexOfKey : function(key){
        if(!this.keys.indexOf){
            for(var i = 0, len = this.key.length; i < len; i++){
                if(this.key[i] == o) return i;
            }
            return -1;
        }else{
            return this.keys.indexOf(o);
        }
    },
    
    item : function(key){
        return this.items[key];
    },
    
    contains : function(o){
        return this.indexOf(o) != -1;
    },
    
    containsKey : function(key){
        return typeof this.items[key] != 'undefined';
    },
    
    clear : function(o){
        this.items = [];
        this.keys = [];
        this.fireEvent('clear');
    },
    
    first : function(){
        return this.items[0];  
    },
    
    last : function(){
        return this.items[this.items.length];    
    }
});
YAHOO.ext.util.MixedCollection.prototype.get = YAHOO.ext.util.MixedCollection.prototype.item;

/*
------------------------------------------------------------------
// File: \JSON.js
------------------------------------------------------------------
*/
/**
 * @class
 * Modified version of Douglas Crockford's json.js that doesn't
 * mess with the Object prototype 
 * http://www.json.org/js.html
 */
YAHOO.ext.util.JSON = new function(){
    var useHasOwn = {}.hasOwnProperty ? true : false;
    var validRE = /^("(\\.|[^"\\\n\r])*?"|[,:{}\[\]0-9.\-+Eaeflnr-u \n\r\t])+?$/;
    
    var pad = function(n) {
        return n < 10 ? '0' + n : n;
    };
    
    var m = {
        '\b': '\\b',
        '\t': '\\t',
        '\n': '\\n',
        '\f': '\\f',
        '\r': '\\r',
        '"' : '\\"',
        '\\': '\\\\'
    };

    var encodeString = function(s){
        if (/["\\\x00-\x1f]/.test(s)) {
            return '"' + s.replace(/([\x00-\x1f\\"])/g, function(a, b) {
                var c = m[b];
                if(c){
                    return c;
                }
                c = b.charCodeAt();
                return '\\u00' +
                    Math.floor(c / 16).toString(16) +
                    (c % 16).toString(16);
            }) + '"';
        }
        return '"' + s + '"';
    };
    
    var encodeArray = function(o){
        var a = ['['], b, i, l = o.length, v;
            for (i = 0; i < l; i += 1) {
                v = o[i];
                switch (typeof v) {
                    case 'undefined':
                    case 'function':
                    case 'unknown':
                        break;
                    default:
                        if (b) {
                            a.push(',');
                        }
                        a.push(v === null ? "null" : YAHOO.ext.util.JSON.encode(v));
                        b = true;
                }
            }
            a.push(']');
            return a.join('');
    };
    
    var encodeDate = function(o){
        return '"' + o.getFullYear() + '-' +
                pad(o.getMonth() + 1) + '-' +
                pad(o.getDate()) + 'T' +
                pad(o.getHours()) + ':' +
                pad(o.getMinutes()) + ':' +
                pad(o.getSeconds()) + '"';
    };
    
    this.encode = function(o){
        if(typeof o == 'undefined' || o === null){
            return 'null';
        }else if(o instanceof Array){
            return encodeArray(o);
        }else if(o instanceof Date){
            return encodeDate(o);
        }else if(typeof o == 'string'){
            return encodeString(o);
        }else if(typeof o == 'number'){
            return isFinite(o) ? String(o) : "null";
        }else if(typeof o == 'boolean'){
            return String(o);
        }else {
            var a = ['{'], b, i, v;
            for (var i in o) {
                if(!useHasOwn || o.hasOwnProperty(i)) {
                    v = o[i];
                    switch (typeof v) {
                    case 'undefined':
                    case 'function':
                    case 'unknown':
                        break;
                    default:
                        if(b){
                            a.push(',');
                        }
                        a.push(this.encode(i), ':',
                                v === null ? "null" : this.encode(v));
                        b = true;
                    }
                }
            }
            a.push('}');
            return a.join('');
        }
    };
    
    this.decode = function(json){
        try{
            if(validRE.test(json)) {
                return eval('(' + json + ')');
            }
        }catch(e){
        }
        throw new SyntaxError("parseJSON");
    };
}();

/*
------------------------------------------------------------------
// File: \CSS.js
------------------------------------------------------------------
*/
/**
 * @class Class for manipulating CSS Rules
 */
YAHOO.ext.util.CSS = new function(){
	var rules = null;
   	
   	var toCamel = function(property) {
      var convert = function(prop) {
         var test = /(-[a-z])/i.exec(prop);
         return prop.replace(RegExp.$1, RegExp.$1.substr(1).toUpperCase());
      };
      while(property.indexOf('-') > -1) {
         property = convert(property);
      }
      return property;
   };
   
   /**
    * Gets all css rules for the document
    */
   this.getRules = function(refreshCache){
   		if(rules == null || refreshCache){
   			rules = {};
   			var ds = document.styleSheets;
   			for(var i =0, len = ds.length; i < len; i++){
   			    try{
    		        var ss = ds[i];
    		        var ssRules = ss.cssRules || ss.rules;
    		        for(var j = ssRules.length-1; j >= 0; --j){
    		        	rules[ssRules[j].selectorText] = ssRules[j];
    		        }
   			    }catch(e){} // try catch for cross domain access issue
	        }
   		}
   		return rules;
   	};
   	
   	/**
    * Searches for a rule by selector
    */
   this.getRule = function(selector, refreshCache){
   		var rs = this.getRules(refreshCache);
   		if(!(selector instanceof Array)){
   		    return rs[selector];
   		}
   		for(var i = 0; i < selector.length; i++){
			if(rs[selector[i]]){
				return rs[selector[i]];
			}
		}
		return null;
   	};
   	
   	
   	/**
    * Updates a rule property
    * @param {String/Array} selector If it's an array it tries each selector until it finds one. Stops immediately once one is found.
    */
   this.updateRule = function(selector, property, value){
   		if(!(selector instanceof Array)){
   			var rule = this.getRule(selector);
   			if(rule){
   				rule.style[property] = value;
   				return true;
   			}/*
   			var camel = toCamel(property);
   			if(rule && rule.style[camel]){
   				rule.style[camel] = value;
   				return true;
   			}*/
   		}else{
   			for(var i = 0; i < selector.length; i++){
   				if(this.updateRule(selector[i], property, value)){
   					return true;
   				}
   			}
   		}
   		return false;
   	};
   	
   	/**
    * Applies a rule to an element without adding the class
    * @param {String/Array} selector If it's an array it tries each selector until it finds one. Stops immediately once one is found.
    */
   this.apply = function(el, selector){
   		if(!(selector instanceof Array)){
   			var rule = this.getRule(selector);
   			if(rule){
   			    var s = rule.style;
   				for(var key in s){
   				    if(typeof s[key] != 'function'){
       					if(s[key] && String(s[key]).indexOf(':') < 0 && s[key] != 'false'){
       						try{el.style[key] = s[key];}catch(e){}
       					}
   				    }
   				}
   				return true;
   			}
   		}else{
   			for(var i = 0; i < selector.length; i++){
   				if(this.apply(el, selector[i])){
   					return true;
   				}
   			}
   		}
   		return false;
   	};
   	
   	this.applyFirst = function(el, id, selector){
   		var selectors = [
   			'#' + id + ' ' + selector,
   			selector
   		];
   		return this.apply(el, selectors);
   	};
   	
   	this.revert = function(el, selector){
   		if(!(selector instanceof Array)){
   			var rule = this.getRule(selector);
   			if(rule){
   				for(key in rule.style){
   					if(rule.style[key] && String(rule.style[key]).indexOf(':') < 0 && rule.style[key] != 'false'){
   						try{el.style[key] = '';}catch(e){}
   					}
   				}
   				return true;
   			}
   		}else{
   			for(var i = 0; i < selector.length; i++){
   				if(this.revert(el, selector[i])){
   					return true;
   				}
   			}
   		}
   		return false;
   	};
   	
   	this.revertFirst = function(el, id, selector){
   		var selectors = [
   			'#' + id + ' ' + selector,
   			selector
   		];
   		return this.revert(el, selectors);
   	};
}();

/*
------------------------------------------------------------------
// File: \Bench.js
------------------------------------------------------------------
*/
/**
 * @class
 * Very simple Benchmark class that supports multiple timers
 * @constructor
 */
YAHOO.ext.util.Bench = function(){
   this.timers = {};
   this.lastKey = null;
};
YAHOO.ext.util.Bench.prototype = {
   start : function(key){
       this.lastKey = key;
       this.timers[key] = {};
       this.timers[key].startTime = new Date().getTime(); 
   },
   
   stop : function(key){
       key = key || this.lastKey;
       this.timers[key].endTime = new Date().getTime(); 
   },
   
   getElapsed : function(key){
       key = key || this.lastKey;
       return this.timers[key].endTime - this.timers[key].startTime;
   },
   
   toString : function(html){
       var results = "";
       for(var key in this.timers){
           if(typeof this.timers[key] != 'function'){
               results += key + ":\t" + (this.getElapsed(key) / 1000) + " seconds\n";
           }
       }
       if(html){
           results = results.replace("\n", '<br>');
       }
       return results;
   },
   
   show : function(){
       alert(this.toString());
   }
};

/*
------------------------------------------------------------------
// File: \DomHelper.js
------------------------------------------------------------------
*/
/**
 * @class
 * Utility class for working with DOM
 */
YAHOO.ext.DomHelper = new function(){
    /**@private*/
    var d = document;
    var tempTableEl = null;
    /** True to force the use of DOM instead of html fragments @type Boolean */
    this.useDom = false;
    var emptyTags = /^(?:base|basefont|br|frame|hr|img|input|isindex|link|meta|nextid|range|spacer|wbr|audioscope|area|param|keygen|col|limittext|spot|tab|over|right|left|choose|atop|of)$/i;
    /**
     * Applies a style specification to an element
     * @param el String/HTMLElement - the element to apply styles to
     * @param styles - A style specification string eg "width:100px", or object in the form {width:"100px"}, or
     * a function which returns such a specification.
     */
    this.applyStyles = function(el, styles){
        if(styles){
           var D = YAHOO.util.Dom;
           if (typeof styles == "string"){
               var re = /\s?([a-z\-]*)\:([^;]*);?/gi;
               var matches;
               while ((matches = re.exec(styles)) != null){
                   D.setStyle(el, matches[1], matches[2]);
               }
            }else if (typeof styles == "object"){
               for (var style in styles){
                  D.setStyle(el, style, styles[style]);
               }
            }else if (typeof styles == "function"){
                YAHOO.ext.DomHelper.applyStyles(el, styles.call());
            }
        }
    }; 
    
    // build as innerHTML where available
    /** @ignore */
    function createHtml(o){
        var b = '';
        b += '<' + o.tag;
        for(var attr in o){
            if(attr == 'tag' || attr == 'children' || attr == 'html' || typeof o[attr] == 'function') continue;
            if(attr == 'style'){
                var s = o['style'];
                if(typeof s == 'function'){
                    s = s.call();
                }
                if(typeof s == 'string'){
                    b += ' style="' + s + '"';
                }else if(typeof s == 'object'){
                    b += ' style="';
                    for(var key in s){
                        if(typeof s[key] != 'function'){
                            b += key + ':' + s[key] + ';';
                        }
                    }
                    b += '"';
                }
            }else{
                if(attr == 'cls'){
                    b += ' class="' + o['cls'] + '"';
                }else if(attr == 'htmlFor'){
                    b += ' for="' + o['htmlFor'] + '"';
                }else{
                    b += ' ' + attr + '="' + o[attr] + '"';
                }
            }
        }
        if(emptyTags.test(o.tag)){
            b += ' />';
        }else{
            b += '>';
            if(o.children){
                for(var i = 0, len = o.children.length; i < len; i++) {
                    b += createHtml(o.children[i], b);
                }
            }
            if(o.html){
                b += o.html;
            }
            b += '</' + o.tag + '>';
        }
        return b;
    }
    
    // build as dom
    /** @ignore */
    function createDom(o, parentNode){
        var el = d.createElement(o.tag);
        var useSet = el.setAttribute ? true : false; // In IE some elements don't have setAttribute
        for(var attr in o){
            if(attr == 'tag' || attr == 'children' || attr == 'html' || attr == 'style' || typeof o[attr] == 'function') continue;
            if(attr=='cls'){
                el.className = o['cls'];
            }else{
                if(useSet) el.setAttribute(attr, o[attr]);
                else el[attr] = o[attr];
            }
        }
        this.applyStyles(el, o.style);
        if(o.children){
            for(var i = 0, len = o.children.length; i < len; i++) {
             	createDom(o.children[i], el);
            }
        }
        if(o.html){
            el.innerHTML = o.html;
        }
        if(parentNode){
           parentNode.appendChild(el);
        }
        return el;
    };
    
    /**
     * @ignore
     * Nasty code for IE's broken table implementation 
     */
    function insertIntoTable(tag, where, el, html){
        if(!tempTableEl){
            tempTableEl = document.createElement('div');
        }
        var node;
        if(tag == 'table' || tag == 'tbody'){
           tempTableEl.innerHTML = '<table><tbody>'+html+'</tbody></table>';
           node = tempTableEl.firstChild.firstChild.firstChild;
        }else{
           tempTableEl.innerHTML = '<table><tbody><tr>'+html+'</tr></tbody></table>';
           node = tempTableEl.firstChild.firstChild.firstChild.firstChild;
        }
        if(where == 'beforebegin'){
            el.parentNode.insertBefore(node, el);
            return node;
        }else if(where == 'afterbegin'){
            el.insertBefore(node, el.firstChild);
            return node;
        }else if(where == 'beforeend'){
            el.appendChild(node);
            return node;
        }else if(where == 'afterend'){
            el.parentNode.insertBefore(node, el.nextSibling);
            return node;
        }
    } 
    
    /**
     * Inserts an HTML fragment into the Dom
     * @param {String} where Where to insert the html in relation to el - beforeBegin, afterBegin, beforeEnd, afterEnd.
     * @param {HTMLElement} el The context element
     * @param {String} html The HTML fragmenet
     * @return {HTMLElement} The new node
     */
    this.insertHtml = function(where, el, html){
        where = where.toLowerCase();
        if(el.insertAdjacentHTML){
            var tag = el.tagName.toLowerCase();
            if(tag == 'table' || tag == 'tbody' || tag == 'tr'){
               return insertIntoTable(tag, where, el, html);
            }
            if(where == 'beforebegin'){
                el.insertAdjacentHTML(where, html);
                return el.previousSibling;
            }else if(where == 'afterbegin'){
                el.insertAdjacentHTML(where, html);
                return el.firstChild;
            }else if(where == 'beforeend'){
                el.insertAdjacentHTML(where, html);
                return el.lastChild;
            }else if(where == 'afterend'){
                el.insertAdjacentHTML(where, html);
                return el.nextSibling;
            }
            throw 'Illegal insertion point -> "' + where + '"';
        }
        var range = el.ownerDocument.createRange();
        var frag;
        if(where == 'beforebegin'){
            range.setStartBefore(el);
            frag = range.createContextualFragment(html);
            el.parentNode.insertBefore(frag, el);
            return el.previousSibling;
        }else if(where == 'afterbegin'){
            range.selectNodeContents(el);
            range.collapse(true);
            frag = range.createContextualFragment(html);
            el.insertBefore(frag, el.firstChild);
            return el.firstChild;
        }else if(where == 'beforeend'){
            range.selectNodeContents(el);
            range.collapse(false);
            frag = range.createContextualFragment(html);
            el.appendChild(frag);
            return el.lastChild;
        }else if(where == 'afterend'){
            range.setStartAfter(el);
            frag = range.createContextualFragment(html);
            el.parentNode.insertBefore(frag, el.nextSibling);
            return el.nextSibling;
        }else{
            throw 'Illegal insertion point -> "' + where + '"';
        } 
    };
    
    /**
     * Creates new Dom element(s) and inserts them before el
     * @param {HTMLElement} el The context element
     * @param {Object} o The Dom object spec (and children)
     * @param {<i>Boolean</i>} returnElement (optional) true to return a YAHOO.ext.Element
     * @return {HTMLElement} The new node
     */
    this.insertBefore = function(el, o, returnElement){
        el = YAHOO.util.Dom.get(el);
        var newNode;
        if(this.useDom){
            newNode = createDom(o, null);
            el.parentNode.insertBefore(newNode, el);
        }else{
            var html = createHtml(o);
            newNode = this.insertHtml('beforeBegin', el, html);
        }
        return returnElement ? YAHOO.ext.Element.get(newNode, true) : newNode;
    };
    
    /**
     * Creates new Dom element(s) and inserts them after el
     * @param {HTMLElement} el The context element
     * @param {Object} o The Dom object spec (and children)
     * @return {HTMLElement} The new node
     */
    this.insertAfter = function(el, o, returnElement){
        el = YAHOO.util.Dom.get(el);
        var newNode;
        if(this.useDom){
            newNode = createDom(o, null);
            el.parentNode.insertBefore(newNode, el.nextSibling);
        }else{
            var html = createHtml(o);
            newNode = this.insertHtml('afterEnd', el, html);
        }
        return returnElement ? YAHOO.ext.Element.get(newNode, true) : newNode;
    };
    
    /**
     * Creates new Dom element(s) and appends them to el
     * @param {HTMLElement} el The context element
     * @param {Object} o The Dom object spec (and children)
     * @param {<i>Boolean</i>} returnElement (optional) true to return a YAHOO.ext.Element
     * @return {HTMLElement} The new node
     */
    this.append = function(el, o, returnElement){
        el = YAHOO.util.Dom.get(el);
        var newNode;
        if(this.useDom){
            newNode = createDom(o, null);
            el.appendChild(newNode);
        }else{
            var html = createHtml(o);
            newNode = this.insertHtml('beforeEnd', el, html);
        }
        return returnElement ? YAHOO.ext.Element.get(newNode, true) : newNode;
    };
    
    /**
     * Creates new Dom element(s) and overwrites the contents of el with them
     * @param {HTMLElement} el The context element
     * @param {Object} o The Dom object spec (and children)
     * @param {<i>Boolean</i>} returnElement (optional) true to return a YAHOO.ext.Element
     * @return {HTMLElement} The new node
     */
    this.overwrite = function(el, o, returnElement){
        el = YAHOO.util.Dom.get(el);
        el.innerHTML = createHtml(o);
        return returnElement ? YAHOO.ext.Element.get(el.firstChild, true) : el.firstChild;
    };
    
    /**
     * Creates a new YAHOO.ext.DomHelper.Template from the Dom object spec 
     * @param {Object} o The Dom object spec (and children)
     * @param {<i>Boolean</i>} returnElement (optional) true to return a YAHOO.ext.Element
     * @return {YAHOO.ext.DomHelper.Template} The new template
     */
    this.createTemplate = function(o){
        var html = createHtml(o);
        return new YAHOO.ext.DomHelper.Template(html);
    };
}();

/**
* @class
* Represents an HTML fragment template
* @constructor
* @param {String} html The HTML fragment
*/
YAHOO.ext.DomHelper.Template = function(html){
    /**@private*/
    this.html = html;
    /**@private*/
    this.re = /\{(\w+)\}/g;
};
YAHOO.ext.DomHelper.Template.prototype = {
    /**
     * Returns an HTML fragment of this template with the specified values applied
     * @param {Object} values The template values. Can be an array if your params are numeric (i.e. {0}) or an object (i.e. {foo: 'bar'})
     * @return {String}
     */
    applyTemplate : function(values){
        if(this.compiled){
            return this.compiled(values);
        }
        var empty = '';
        var fn = function(match, index){
            if(typeof values[index] != 'undefined'){
                return values[index];
            }else{
                return empty;
            }
        }
        return this.html.replace(this.re, fn);
    },
    
    /**
     * Compiles the template into an internal function, eliminating the RegEx overhead
     */
    compile : function(){
        var html = this.html;
        var re = /\{(\w+)\}/g;
        var body = [];
        body.push("this.compiled = function(values){ return ");
        var result;
        var lastMatchEnd = 0;
        while ((result = re.exec(html)) != null){
            body.push("'", html.substring(lastMatchEnd, result.index), "' + ");
            body.push("values['", html.substring(result.index+1,re.lastIndex-1), "'] + ");
            lastMatchEnd = re.lastIndex;
        }
        body.push("'", html.substr(lastMatchEnd), "';};");
        eval(body.join(''));
    },
   
    /**
     * Applies the supplied values to the template and inserts the new node(s) before el
     * @param {HTMLElement} el The context element
     * @param {Object} values The template values. Can be an array if your params are numeric (i.e. {0}) or an object (i.e. {foo: 'bar'})
     * @param {<i>Boolean</i>} returnElement (optional) true to return a YAHOO.ext.Element
     * @return {HTMLElement} The new node
     */
    insertBefore: function(el, values, returnElement){
        el = YAHOO.util.Dom.get(el);
        var newNode = YAHOO.ext.DomHelper.insertHtml('beforeBegin', el, this.applyTemplate(values));
        return returnElement ? YAHOO.ext.Element.get(newNode, true) : newNode;
    },
    
    /**
     * Applies the supplied values to the template and inserts the new node(s) after el
     * @param {HTMLElement} el The context element
     * @param {Object} values The template values. Can be an array if your params are numeric (i.e. {0}) or an object (i.e. {foo: 'bar'})
     * @param {<i>Boolean</i>} returnElement (optional) true to return a YAHOO.ext.Element
     * @return {HTMLElement} The new node
     */
    insertAfter : function(el, values, returnElement){
        el = YAHOO.util.Dom.get(el);
        var newNode = YAHOO.ext.DomHelper.insertHtml('afterEnd', el, this.applyTemplate(values));
        return returnElement ? YAHOO.ext.Element.get(newNode, true) : newNode;
    },
    
    /**
     * Applies the supplied values to the template and append the new node(s) to el
     * @param {HTMLElement} el The context element
     * @param {Object} values The template values. Can be an array if your params are numeric (i.e. {0}) or an object (i.e. {foo: 'bar'})
     * @param {<i>Boolean</i>} returnElement (optional) true to return a YAHOO.ext.Element
     * @return {HTMLElement} The new node
     */
    append : function(el, values, returnElement){
        el = YAHOO.util.Dom.get(el);
        var newNode = YAHOO.ext.DomHelper.insertHtml('beforeEnd', el, this.applyTemplate(values));
        return returnElement ? YAHOO.ext.Element.get(newNode, true) : newNode;
    },
    
    /**
     * Applies the supplied values to the template and overwrites the content of el with the new node(s)
     * @param {HTMLElement} el The context element
     * @param {Object} values The template values. Can be an array if your params are numeric (i.e. {0}) or an object (i.e. {foo: 'bar'})
     * @param {<i>Boolean</i>} returnElement (optional) true to return a YAHOO.ext.Element
     * @return {HTMLElement} The new node
     */
    overwrite : function(el, values, returnElement){
        el = YAHOO.util.Dom.get(el);
        el.innerHTML = '';
        var newNode = YAHOO.ext.DomHelper.insertHtml('beforeEnd', el, this.applyTemplate(values));
        return returnElement ? YAHOO.ext.Element.get(newNode, true) : newNode;
    }
};

YAHOO.ext.Template = YAHOO.ext.DomHelper.Template;

/*
------------------------------------------------------------------
// File: \Element.js
------------------------------------------------------------------
*/
/**
 * @class Wraps around a DOM element and provides convenient access to Yahoo 
 * UI library functionality.<br><br>
 * Usage:<br>
 * <pre><code>
 * var el = YAHOO.ext.Element.get('myElementId');
 * // or the shorter
 * var el = getEl('myElementId');
 * </code></pre>
 * Using YAHOO.ext.Element.get() instead of calling the constructor directly ensures you get the same object 
 * each call instead of constructing a new one.<br>
 * @requires YAHOO.util.Dom
 * @requires YAHOO.util.Event
 * @requires YAHOO.util.CustomEvent 
 * @requires YAHOO.util.Anim (optional) to support animation
 * @requires YAHOO.util.Motion (optional) to support animation
 * @requires YAHOO.util.Easing (optional) to support animation
 * @constructor Create a new Element directly.
 * @param {String/HTMLElement} element
 * @param {<i>Boolean</i>} forceNew (optional) By default the constructor checks to see if there is already an instance of this element in the cache and if there is it returns the same instance. This will skip that check (useful for extending this class).
 */
YAHOO.ext.Element = function(element, forceNew){
    var dom = YAHOO.util.Dom.get(element);
    if(!dom){ // invalid id/element
        return null;
    }
    if(!forceNew && YAHOO.ext.Element.cache[dom.id]){ // element object already exists
        return YAHOO.ext.Element.cache[dom.id];
    }
    /**
     * The DOM element
     * @type HTMLElement
     */
    this.dom = dom;
    
    /**
     * The DOM element ID
     * @type String
     */
    this.id = this.dom.id;
    /**
     * @private the current visibility mode
     */
    this.visibilityMode = YAHOO.ext.Element.VISIBILITY;
    
    
    /**
     * @private the element's default display mode
     */
    this.originalDisplay = YAHOO.util.Dom.getStyle(this.dom, 'display') || '';
    if(this.autoDisplayMode){
        if(this.originalDisplay == 'none'){
            this.setVisibilityMode(YAHOO.ext.Element.DISPLAY);
        }
    }
    if(this.originalDisplay == 'none'){
        this.originalDisplay = YAHOO.ext.Element.blockElements.test(this.dom.tagName) ? 'block' : 'inline';
    }
    
    /**
     * The default unit to append to CSS values where a unit isn't provided (Defaults to px).
     * @type String
     */
    this.defaultUnit = 'px';
}

YAHOO.ext.Element.prototype = {    
    /**
     * Sets the elements visibility mode. When setVisible() is called it
     * will use this to determine whether to set the visibility or the display property.
     * @param visMode Element.VISIBILITY or Element.DISPLAY
     */
    setVisibilityMode : function(visMode){
        this.visibilityMode = visMode;
    },
    
    /**
     * Convenience method for setVisibilityMode(Element.DISPLAY)
     * @param {String} display (optional) What to set display to when visible
     */
    enableDisplayMode : function(display){
        this.setVisibilityMode(YAHOO.ext.Element.DISPLAY);
        if(typeof display != 'undefined') this.originalDisplay = display;
    },
    
    /**
     * Perform Yahoo UI animation on this element. 
     * @param {Object} args The YUI animation control args
     * @param {<i>Float</i>} duration (optional) How long the animation lasts. (Defaults to .35 seconds)
     * @param {<i>Function</i>} onComplete (optional) Function to call when animation completes.
     * @param {<i>Function</i>} easing (optional) YAHOO.util.Easing method to use. (Defaults to YAHOO.util.Easing.easeBoth)
     * @param {<i>Function</i>} animType (optional) YAHOO.util.Anim subclass to use. For example: YAHOO.util.Motion
     */
    animate : function(args, duration, onComplete, easing, animType){
        this.anim(args, duration, onComplete, easing, animType);
        return this;
    },
    
    /**
     * @private Internal animation call
     */
    anim : function(args, duration, onComplete, easing, animType){
        animType = animType || YAHOO.util.Anim;
        var anim = new animType(this.dom, args, duration || .35, 
                easing || YAHOO.util.Easing.easeBoth);
        if(onComplete){
            if(!(onComplete instanceof Array)){
                anim.onComplete.subscribe(onComplete, this, true);
            }else{
                for(var i = 0; i < onComplete.length; i++){
                    var fn = onComplete[i];
                    if(fn) anim.onComplete.subscribe(fn, this, true);
                }
            }
        }
        anim.animate();
    },
    
    /**
     * Scrolls this element into view. If the container element is static postioned it is changed to position relative.
     * @param {<i>String/HTMLElement/Element</i>} container (optional) The container element to scroll (defaults to document.body)
     */
    scrollIntoView : function(container){
        var c = getEl(container || document.body, true);
        var cp = c.getStyle('position');
        var restorePos = false;
        if(cp != 'relative' && cp != 'absolute'){
            c.setStyle('position', 'relative');
            restorePos = true;
        }
        var el = this.dom;
        var childTop = parseInt(el.offsetTop, 10);
        var childBottom = childTop + el.offsetHeight;
        var containerTop = parseInt(c.scrollTop, 10); // parseInt for safari bug
        var containerBottom = containerTop + c.clientHeight;
        if(childTop < containerTop){
        	c.scrollTop = childTop;
        }else if(childBottom > containerBottom){
            c.scrollTop = childBottom-c.clientHeight;
        }
        if(restorePos){
            c.setStyle('position', cp);
        }
        return this;
    },
        
    /** Measures the elements content height and updates height to match. */
    autoHeight : function(animate, duration, onComplete, easing){
        var oldHeight = this.getHeight();
        this.clip();
        this.setHeight(1); // force clipping
        setTimeout(function(){
            var height = parseInt(this.dom.scrollHeight, 10); // parseInt for Safari
            if(!animate){
                this.setHeight(height);
                this.unclip();
            }else{
                this.setHeight(oldHeight); // restore original height
                this.setHeight(height, animate, duration, function(){
                    this.unclip();
                    if(typeof onComplete == 'function') onComplete();
                }.createDelegate(this), easing);
            }
        }.createDelegate(this), 0);
        return this;
    },
    
    /**
     * Checks whether the element is currently visible using both visibility and display properties.
     * @param {<i>Boolean</i>} deep True to walk the dom and see if parent elements are hidden
     * @return {Boolean} Whether the element is currently visible 
     */
    isVisible : function(deep) {
        var vis = YAHOO.util.Dom.getStyle(this.dom, 'visibility') != 'hidden' 
               && YAHOO.util.Dom.getStyle(this.dom, 'display') != 'none';
        if(!deep || !vis){
            return vis;
        }
        var p = this.dom.parentNode;
        while(p && p.tagName.toLowerCase() != 'body'){
            if(YAHOO.util.Dom.getStyle(p, 'visibility') == 'hidden' || YAHOO.util.Dom.getStyle(p, 'display') == 'none'){
                return false;
            }
            p = p.parentNode;
        }
        return true;
    },
    
    /**
     * Selects child nodes based on the passed CSS selector
     * @param {String} selector The CSS selector
     * @param {Boolean} unique true to create a unique YAHOO.ext.Element for each child (defaults to a shared flyweight object)
     */
    select : function(selector, unique){
        return YAHOO.ext.Element.select('#' + this.dom.id + ' ' + selector, unique);  
    },
    
    /**
     * Initializes a YAHOO.util.DD object for this element.
     * @param {String} group The group the DD object is member of
     * @param {Object} config The DD config object
     * @param {Object} overrides An object containing methods to override/implement on the DD object
     * @return The DD object
     */
    initDD : function(group, config, overrides){
        var dd = new YAHOO.util.DD(YAHOO.util.Dom.generateId(this.dom), group, config);
        return YAHOO.ext.util.Config.apply(dd, overrides);
    },
   
    /**
     * Initializes a YAHOO.util.DDProxy object for this element.
     * @param {String} group The group the DDProxy object is member of
     * @param {Object} config The DDProxy config object
     * @param {Object} overrides An object containing methods to override/implement on the DDProxy object
     * @return The DDProxy object
     */
    initDDProxy : function(group, config, overrides){
        var dd = new YAHOO.util.DDProxy(YAHOO.util.Dom.generateId(this.dom), group, config);
        return YAHOO.ext.util.Config.apply(dd, overrides);
    },
   
    /**
     * Initializes a YAHOO.util.DDTarget object for this element.
     * @param {String} group The group the DDTarget object is member of
     * @param {Object} config The DDTarget config object
     * @param {Object} overrides An object containing methods to override/implement on the DDTarget object
     * @return The DDTarget object
     */
    initDDTarget : function(group, config, overrides){
        var dd = new YAHOO.util.DDTarget(YAHOO.util.Dom.generateId(this.dom), group, config);
        return YAHOO.ext.util.Config.apply(dd, overrides);
    },
   
    /**
     * Sets the visibility of the element (see details). If the visibilityMode is set to Element.DISPLAY, it will use 
     * the display property to hide the element, otherwise it uses visibility. The default is to hide and show using the visibility property.
     * @param {Boolean} visible Whether the element is visible
     * @param {<i>Boolean</i>} animate (optional) Fade the element in or out (Default is false)
     * @param {<i>Float</i>} duration (optional) How long the fade effect lasts. (Defaults to .35 seconds)
     * @param {<i>Function</i>} onComplete (optional) Function to call when animation completes.
     * @param {<i>Function</i>} easing (optional) YAHOO.util.Easing method to use. (Defaults to YAHOO.util.Easing.easeOut for hiding or YAHOO.util.Easing.easeIn for showing)
     */
     setVisible : function(visible, animate, duration, onComplete, easing){
        //if(this.isVisible() == visible) return; // nothing to do
        if(!animate || !YAHOO.util.Anim){
            if(this.visibilityMode == YAHOO.ext.Element.DISPLAY){
                this.setDisplayed(visible);
            }else{
                YAHOO.util.Dom.setStyle(this.dom, 'visibility', visible ? 'visible' : 'hidden');
            }
        }else{
            // make sure they can see the transition
            this.setOpacity(visible?0:1);
            YAHOO.util.Dom.setStyle(this.dom, 'visibility', 'visible');
            if(this.visibilityMode == YAHOO.ext.Element.DISPLAY){
                this.setDisplayed(true);
            }
            var args = {opacity: { from: (visible?0:1), to: (visible?1:0) }};
            var anim = new YAHOO.util.Anim(this.dom, args, duration || .35, 
                easing || (visible ? YAHOO.util.Easing.easeIn : YAHOO.util.Easing.easeOut));
            anim.onComplete.subscribe((function(){
                if(this.visibilityMode == YAHOO.ext.Element.DISPLAY){
                    this.setDisplayed(visible);
                }else{
                    YAHOO.util.Dom.setStyle(this.dom, 'visibility', visible ? 'visible' : 'hidden');
                }
            }).createDelegate(this));
            if(onComplete){
                anim.onComplete.subscribe(onComplete);
            }
            anim.animate();
        }
        return this;
    },
    
    /**
     *@private
     */
    isDisplayed : function() {
        return YAHOO.util.Dom.getStyle(this.dom, 'display') != 'none';
    },
    
    /**
     * Toggles the elements visibility or display, depending on visibility mode.
     * @param {<i>Boolean</i>} animate (optional) Fade the element in or out (Default is false)
     * @param {<i>float</i>} duration (optional) How long the fade effect lasts. (Defaults to .35 seconds)
     * @param {<i>Function</i>} onComplete (optional) Function to call when animation completes.
     * @param {<i>Function</i>} easing (optional) YAHOO.util.Easing method to use. (Defaults to YAHOO.util.Easing.easeOut for hiding or YAHOO.util.Easing.easeIn for showing)
     */
    toggle : function(animate, duration, onComplete, easing){
        this.setVisible(!this.isVisible(), animate, duration, onComplete, easing);
        return this;
    },
    
    /**
     *@private
     */
    setDisplayed : function(value) {
        YAHOO.util.Dom.setStyle(this.dom, 'display', value ? this.originalDisplay : 'none');
        return this;
    },
    
    /**
     * Tries to focus the element. Any exceptions are caught.
     */
    focus : function() {
        try{
            this.dom.focus();
        }catch(e){}
        return this;
    },
    
    /**
     * Add a CSS class to the element.
     * @param {String} className The CSS class to add
     */
    addClass : function(className){
        YAHOO.util.Dom.addClass(this.dom, className);
        return this;
    },
    
    /**
     * Adds the passed className to this element and removes the class from all siblings
     */
    radioClass : function(className){
        var siblings = this.dom.parentNode.childNodes;
        for(var i = 0; i < siblings.length; i++) {
        	var s = siblings[i];
        	if(s.nodeType == 1){
        	    YAHOO.util.Dom.removeClass(s, className);
        	}
        }
        YAHOO.util.Dom.addClass(this.dom, className);
        return this;
    },
    /**
     * Removes a CSS class from the element.
     * @param {String} className The CSS class to remove
     */
    removeClass : function(className){
        YAHOO.util.Dom.removeClass(this.dom, className);
        return this;
    },
    
    toggleClass : function(className){
        if(YAHOO.util.Dom.hasClass(this.dom, className)){
            YAHOO.util.Dom.removeClass(this.dom, className);
        }else{
            YAHOO.util.Dom.addClass(this.dom, className);
        }
        return this;
    },
    
    /**
     * Checks if a CSS class is in use by the element.
     * @param {String} className The CSS class to check
     * @return {Boolean} true or false
     */
    hasClass : function(className){
        return YAHOO.util.Dom.hasClass(this.dom, className);
    },
    
    /**
     * Replaces a CSS class on the element with another.
     * @param {String} oldClassName The CSS class to replace
     * @param {String} newClassName The replacement CSS class
     */
    replaceClass : function(oldClassName, newClassName){
        YAHOO.util.Dom.replaceClass(this.dom, oldClassName, newClassName);
        return this;
    },
    
    /**
       * Normalizes currentStyle and ComputedStyle.
       * @param {String} property The style property whose value is returned.
       * @return {String} The current value of the style property for this element.
       */
    getStyle : function(name){
        return YAHOO.util.Dom.getStyle(this.dom, name);
    },
    
    /**
       * Wrapper for setting style properties, also takes single object parameter of multiple styles
       * @param {String} property The style property to be set.
       * @param {String} val The value to apply to the given property.
       */
    setStyle : function(name, value){
        if(typeof name == 'string'){
            YAHOO.util.Dom.setStyle(this.dom, name, value);
        }else{
            var D = YAHOO.util.Dom;
            for(var style in name){
                if(typeof name[style] != 'function'){
                   D.setStyle(this.dom, style, name[style]);
                }
            }
        }
        return this;
    },
    
    /**
       * Gets the current X position of the element based on page coordinates.  Element must be part of the DOM tree to have page coordinates (display:none or elements not appended return false).
       @ return {String} The X position of the element
       */
    getX : function(){
        return YAHOO.util.Dom.getX(this.dom);
    },
    
    /**
       * Gets the current Y position of the element based on page coordinates.  Element must be part of the DOM tree to have page coordinates (display:none or elements not appended return false).
       @ return {String} The Y position of the element
       */
    getY : function(){
        return YAHOO.util.Dom.getY(this.dom);
    },
    
    /**
       * Gets the current position of the element based on page coordinates.  Element must be part of the DOM tree to have page coordinates (display:none or elements not appended return false).
       @ return {Array} The XY position of the element
       */
    getXY : function(){
        return YAHOO.util.Dom.getXY(this.dom);
    },
    
    /**
       * Sets the X position of the element based on page coordinates.  Element must be part of the DOM tree to have page coordinates (display:none or elements not appended return false).
       @param {String} The X position of the element
       */
    setX : function(x, animate, duration, onComplete, easing){
        if(!animate || !YAHOO.util.Anim){
            YAHOO.util.Dom.setX(this.dom, x);
        }else{
            this.setXY([x, this.getY()], animate, duration, onComplete, easing);
        }
        return this;
    },
    
    /**
       * Sets the Y position of the element based on page coordinates.  Element must be part of the DOM tree to have page coordinates (display:none or elements not appended return false).
       @param {String} The Y position of the element
       */
    setY : function(y, animate, duration, onComplete, easing){
        if(!animate || !YAHOO.util.Anim){
            YAHOO.util.Dom.setY(this.dom, y);
        }else{
            this.setXY([this.getX(), y], animate, duration, onComplete, easing);
        }
        return this;
    },
    
    /**
     * Set the element's X position directly using CSS style (instead of setX())
     * @param {String} left The left CSS property value
     */
    setLeft : function(left){
        YAHOO.util.Dom.setStyle(this.dom, 'left', this.addUnits(left));
        return this;
    },
    
    /**
     * Set the element's Y position directly using CSS style (instead of setY())
     * @param {String} top The top CSS property value
     */
    setTop : function(top){
        YAHOO.util.Dom.setStyle(this.dom, 'top', this.addUnits(top));
        return this;
    },
    
    /**
     * Set the element's css right style
     * @param {String} left The right CSS property value
     */
    setRight : function(right){
        YAHOO.util.Dom.setStyle(this.dom, 'right', this.addUnits(right));
        return this;
    },
    
    /**
     * Set the element's css bottom style
     * @param {String} top The bottom CSS property value
     */
    setBottom : function(bottom){
        YAHOO.util.Dom.setStyle(this.dom, 'bottom', this.addUnits(bottom));
        return this;
    },
    
    /**
     * Set the position of the element in page coordinates, regardless of how the element is positioned.
     * The element must be part of the DOM tree to have page coordinates (display:none or elements not appended return false).
     * @param {Array} pos Contains X & Y [x, y] values for new position (coordinates are page-based)
     * @param {<i>Boolean</i>} animate (optional) Animate the transition (Default is false)
     * @param {<i>float</i>} duration (optional) How long the animation lasts. (Defaults to .35 seconds)
     * @param {<i>Function</i>} onComplete (optional) Function to call when animation completes.
     * @param {<i>Function</i>} easing (optional) YAHOO.util.Easing method to use. (Defaults to YAHOO.util.Easing.easeBoth)
        */
    setXY : function(pos, animate, duration, onComplete, easing){
        if(!animate || !YAHOO.util.Anim){
            YAHOO.util.Dom.setXY(this.dom, pos);
        }else{
            this.anim({points: {to: pos}}, duration, onComplete, easing, YAHOO.util.Motion);
        }
        return this;
    },
    
    /**
     * Set the position of the element in page coordinates, regardless of how the element is positioned.
     * The element must be part of the DOM tree to have page coordinates (display:none or elements not appended return false).
     * @param {Number} x X value for new position (coordinates are page-based)
     * @param {Number} y Y value for new position (coordinates are page-based)
     * @param {<i>Boolean</i>} animate (optional) Animate the transition (Default is false)
     * @param {<i>float</i>} duration (optional) How long the animation lasts. (Defaults to .35 seconds)
     * @param {<i>Function</i>} onComplete (optional) Function to call when animation completes.
     * @param {<i>Function</i>} easing (optional) YAHOO.util.Easing method to use. (Defaults to YAHOO.util.Easing.easeBoth)
     */
    setLocation : function(x, y, animate, duration, onComplete, easing){
        this.setXY([x, y], animate, duration, onComplete, easing);
        return this;
    },
    
    /**
     * Set the position of the element in page coordinates, regardless of how the element is positioned.
     * The element must be part of the DOM tree to have page coordinates (display:none or elements not appended return false).
     * @param {Number} x X value for new position (coordinates are page-based)
     * @param {Number} y Y value for new position (coordinates are page-based)
     * @param {<i>Boolean</i>} animate (optional) Animate the transition (Default is false)
     * @param {<i>float</i>} duration (optional) How long the animation lasts. (Defaults to .35 seconds)
     * @param {<i>Function</i>} onComplete (optional) Function to call when animation completes.
     * @param {<i>Function</i>} easing (optional) YAHOO.util.Easing method to use. (Defaults to YAHOO.util.Easing.easeBoth)
     */
    moveTo : function(x, y, animate, duration, onComplete, easing){
        //YAHOO.util.Dom.setStyle(this.dom, 'left', this.addUnits(x));
        //YAHOO.util.Dom.setStyle(this.dom, 'top', this.addUnits(y));
        this.setXY([x, y], animate, duration, onComplete, easing);
        return this;
    },
    
    /**
       * Returns the region position of the given element.
       * The element must be part of the DOM tree to have a region (display:none or elements not appended return false).
       * @return {Region} A YAHOO.util.Region containing "top, left, bottom, right" member data.
       */
    getRegion : function(){
        return YAHOO.util.Dom.getRegion(this.dom);
    },
    
    /**
     * Returns the offset height of the element
     * @param {Boolean} contentHeight (optional) true to get the height minus borders and padding
     * @return {Number} The element's height
     */
    getHeight : function(contentHeight){
        var h = this.dom.offsetHeight;
        return contentHeight !== true ? h : h-this.getBorderWidth('tb')-this.getPadding('tb');
    },
    
    /**
     * Returns the offset width of the element
     * @param {Boolean} contentWidth (optional) true to get the width minus borders and padding
     * @return {Number} The element's width
     */
    getWidth : function(contentWidth){
        var w = this.dom.offsetWidth;
        return contentWidth !== true ? w : w-this.getBorderWidth('lr')-this.getPadding('lr');
    },
    
    /**
     * Returns the size of the element
     * @param {Boolean} contentSize (optional) true to get the width/size minus borders and padding
     * @return {Object} An object containing the element's size {width: (element width), height: (element height)}
     */
    getSize : function(contentSize){
        return {width: this.getWidth(contentSize), height: this.getHeight(contentSize)};
    },
    
    /** @private */
    adjustWidth : function(width){
        if(typeof width == 'number'){
            if(this.autoBoxAdjust && !this.isBorderBox()){
               width -= (this.getBorderWidth('lr') + this.getPadding('lr'));
            }
            if(width < 0){
                width = 0;
            }
        }
        return width;
    },
    
    /** @private */
    adjustHeight : function(height){
        if(typeof height == 'number'){
           if(this.autoBoxAdjust && !this.isBorderBox()){
               height -= (this.getBorderWidth('tb') + this.getPadding('tb'));
           }
           if(height < 0){
               height = 0;
           }
        }
        return height;
    },
    
    /**
     * Set the width of the element
     * @param {Number} width The new width
     * @param {<i>Boolean</i>} animate (optional) Animate the transition (Default is false)
     * @param {<i>float</i>} duration (optional) How long the animation lasts. (Defaults to .35 seconds)
     * @param {<i>Function</i>} onComplete (optional) Function to call when animation completes.
     * @param {<i>Function</i>} easing (optional) YAHOO.util.Easing method to use. (Defaults to YAHOO.util.Easing.easeOut if width is larger or YAHOO.util.Easing.easeIn if it is smaller)
     */
    setWidth : function(width, animate, duration, onComplete, easing){
        width = this.adjustWidth(width);
        if(!animate || !YAHOO.util.Anim){
            YAHOO.util.Dom.setStyle(this.dom, 'width', this.addUnits(width));
        }else{
            this.anim({width: {to: width}}, duration, onComplete, 
                easing || (width > this.getWidth() ? YAHOO.util.Easing.easeOut : YAHOO.util.Easing.easeIn));
        }
        return this;
    },
    
    /**
     * Set the height of the element
     * @param {Number} height The new height
     * @param {<i>Boolean</i>} animate (optional) Animate the transition (Default is false)
     * @param {<i>float</i>} duration (optional) How long the animation lasts. (Defaults to .35 seconds)
     * @param {<i>Function</i>} onComplete (optional) Function to call when animation completes.
     * @param {<i>Function</i>} easing (optional) YAHOO.util.Easing method to use. (Defaults to YAHOO.util.Easing.easeOut if height is larger or YAHOO.util.Easing.easeIn if it is smaller)
     */
     setHeight : function(height, animate, duration, onComplete, easing){
        height = this.adjustHeight(height);
        if(!animate || !YAHOO.util.Anim){
            YAHOO.util.Dom.setStyle(this.dom, 'height', this.addUnits(height));
        }else{
            this.anim({height: {to: height}}, duration, onComplete,  
                   easing || (height > this.getHeight() ? YAHOO.util.Easing.easeOut : YAHOO.util.Easing.easeIn));
        }
        return this;
    },
    
    /**
     * Set the size of the element. If animation is true, both width an height will be animated concurrently.
     * @param {Number} width The new width
     * @param {Number} height The new height
     * @param {<i>Boolean</i>} animate (optional) Animate the transition (Default is false)
     * @param {<i>float</i>} duration (optional) How long the animation lasts. (Defaults to .35 seconds)
     * @param {<i>Function</i>} onComplete (optional) Function to call when animation completes.
     * @param {<i>Function</i>} easing (optional) YAHOO.util.Easing method to use. (Defaults to YAHOO.util.Easing.easeBoth)
     */
     setSize : function(width, height, animate, duration, onComplete, easing){
        if(!animate || !YAHOO.util.Anim){
            this.setWidth(width);
            this.setHeight(height);
        }else{
            width = this.adjustWidth(width); height = this.adjustHeight(height);
            this.anim({width: {to: width}, height: {to: height}}, duration, onComplete, easing);
        }
        return this;
    },
    
    /**
     * Sets the element's position and size in one shot. If animation is true then width, height, x and y will be animated concurrently.
     * @param {Number} x X value for new position (coordinates are page-based)
     * @param {Number} y Y value for new position (coordinates are page-based)
     * @param {Number} width The new width
     * @param {Number} height The new height
     * @param {<i>Boolean</i>} animate (optional) Animate the transition (Default is false)
     * @param {<i>float</i>} duration (optional) How long the animation lasts. (Defaults to .35 seconds)
     * @param {<i>Function</i>} onComplete (optional) Function to call when animation completes.
     * @param {<i>Function</i>} easing (optional) YAHOO.util.Easing method to use. (Defaults to YAHOO.util.Easing.easeBoth)
     */
    setBounds : function(x, y, width, height, animate, duration, onComplete, easing){
        if(!animate || !YAHOO.util.Anim){
            this.setWidth(width);
            this.setHeight(height);
            this.setLocation(x, y);
        }else{
            width = this.adjustWidth(width); height = this.adjustHeight(height);
            this.anim({points: {to: [x, y]}, width: {to: width}, height: {to: height}}, duration, onComplete, easing, YAHOO.util.Motion);
        }
        return this;
    },
    
    /**
     * Sets the element's position and size the the specified region. If animation is true then width, height, x and y will be animated concurrently.
     * @param {YAHOO.util.Region} region The region to fill
     * @param {<i>Boolean</i>} animate (optional) Animate the transition (Default is false)
     * @param {<i>float</i>} duration (optional) How long the animation lasts. (Defaults to .35 seconds)
     * @param {<i>Function</i>} onComplete (optional) Function to call when animation completes.
     * @param {<i>Function</i>} easing (optional) YAHOO.util.Easing method to use. (Defaults to YAHOO.util.Easing.easeBoth)
     */
    setRegion : function(region, animate, duration, onComplete, easing){
        this.setBounds(region.left, region.top, region.right-region.left, region.bottom-region.top, animate, duration, onComplete, easing);
        return this;
    },
    
    /**
     * Appends an event handler to this element
     * @param {String}   eventName     The type of event to listen for
     * @param {Function} handler        The method the event invokes
     * @param {<i>Object</i>}   scope  (optional)  An arbitrary object that will be 
     *                             passed as a parameter to the handler
     * @param {<i>boolean</i>}  override (optional) If true, the obj passed in becomes
     *                             the execution scope of the listener
     */
    addListener : function(eventName, handler, scope, override){
        YAHOO.util.Event.addListener(this.dom, eventName, handler, scope || this, true);
        return this;
    },
    
    /**
     * Appends an event handler to this element and automatically prevents the default action, and if set stops propagation (bubbling) as well
     * @param {String}   eventName     The type of event to listen for
     * @param {Boolean}   stopPropagation     Whether to also stopPropagation (bubbling) 
     * @param {Function} handler        The method the event invokes
     * @param {<i>Object</i>}   scope  (optional)  An arbitrary object that will be 
     *                             passed as a parameter to the handler
     * @param {<i>boolean</i>}  override (optional) If true, the obj passed in becomes
     *                             the execution scope of the listener
     */
    addHandler : function(eventName, stopPropagation, handler, scope, override){
        var fn = YAHOO.ext.Element.createStopHandler(stopPropagation, handler, scope || this, true);
        YAHOO.util.Event.addListener(this.dom, eventName, fn);
        return this;
    },
    
    /** @private */
    
    /**
     * Appends an event handler to this element (Same as addListener)
     * @param {String}   eventName     The type of event to listen for
     * @param {Function} handler        The method the event invokes
     * @param {<i>Object</i>}   scope (optional)   An arbitrary object that will be 
     *                             passed as a parameter to the handler
     * @param {<i>boolean</i>}  override (optional) If true, the obj passed in becomes
     *                             the execution scope of the listener
     */
    on : function(eventName, handler, scope, override){
        YAHOO.util.Event.addListener(this.dom, eventName, handler, scope || this, true);
        return this;
    },
    
    /**
     * Append a managed listener - See {@link YAHOO.ext.EventObject} for more details.
     * @param {String}   eventName     The type of event to listen for
     * @param {Function} fn        The method the event invokes
     * @param {<i>Object</i>}   scope  (optional)  An arbitrary object that will be 
     *                             passed as a parameter to the handler
     * @param {<i>boolean</i>}  override (optional) If true, the obj passed in becomes
     *                             the execution scope of the listener
     */
    addManagedListener : function(eventName, fn, scope, override){
        return YAHOO.ext.EventManager.on(this.dom, eventName, fn, scope || this, true);
    },
    
    /** 
     * Append a managed listener (shorthanded for {@link #addManagedListener}) 
     * @param {String}   eventName     The type of event to listen for
     * @param {Function} fn        The method the event invokes
     * @param {<i>Object</i>}   scope  (optional)  An arbitrary object that will be 
     *                             passed as a parameter to the handler
     * @param {<i>boolean</i>}  override (optional) If true, the obj passed in becomes
     *                             the execution scope of the listener
     */
    mon : function(eventName, fn, scope, override){
        return YAHOO.ext.EventManager.on(this.dom, eventName, fn, scope || this, true);
    },
    /**
     * Removes an event handler from this element
     * @param {String} sType the type of event to remove
     * @param {Function} fn the method the event invokes
     * @param {Object} scope
     */
    removeListener : function(eventName, handler, scope){
        YAHOO.util.Event.removeListener(this.dom, eventName, handler, scope || this);
        return this;
    },
    
    /**
     * Removes all previous added listeners from this element
     */
    removeAllListeners : function(){
        YAHOO.util.Event.purgeElement(this.dom);
        return this;
    },
    
    
    /**
     * Set the opacity of the element
     * @param {Float} opacity The new opacity. 0 = transparent, .5 = 50% visibile, 1 = fully visible, etc
     * @param {<i>Boolean</i>} animate (optional) Animate (fade) the transition (Default is false)
     * @param {<i>Float</i>} duration (optional) How long the animation lasts. (Defaults to .35 seconds)
     * @param {<i>Function</i>} onComplete (optional) Function to call when animation completes.
     * @param {<i>Function</i>} easing (optional) YAHOO.util.Easing method to use. (Defaults to YAHOO.util.Easing.easeOut if height is larger or YAHOO.util.Easing.easeIn if it is smaller)
     */
     setOpacity : function(opacity, animate, duration, onComplete, easing){
        if(!animate || !YAHOO.util.Anim){
            YAHOO.util.Dom.setStyle(this.dom, 'opacity', opacity);
        }else{
            this.anim({opacity: {to: opacity}}, duration, onComplete, easing);
        }
        return this;
    },
    
    /**
     * Same as getX()
     */
    getLeft : function(local){
        if(!local){
            return this.getX();
        }else{
            return parseInt(this.getStyle('left'), 10) || 0;
        }
    },
    
    /**
     * Gets the right X coordinate of the element (element X position + element width)
     * @return {String} The left position of the element
     */
    getRight : function(local){
        if(!local){
            return this.getX() + this.getWidth();
        }else{
            return (this.getLeft(true) + this.getWidth()) || 0;
        }
    },
    
    /**
     * Same as getY()
     */
    getTop : function(local) {
        if(!local){
            return this.getY();
        }else{
            return parseInt(this.getStyle('top'), 10) || 0;
        }
    },
    
    /**
     * Gets the bottom Y coordinate of the element (element Y position + element height)
     * @return {String} The bottom position of the element
     */
    getBottom : function(local){
        if(!local){
            return this.getY() + this.getHeight();
        }else{
            return (this.getTop(true) + this.getHeight()) || 0;
        }
    },
    
    /**
    * Set the element as absolute positioned with the specified z-index
    * @param {<i>Number</i>} zIndex (optional)
    */
    setAbsolutePositioned : function(zIndex){
        this.setStyle('position', 'absolute');
        if(zIndex){
            this.setStyle('z-index', zIndex);
        }
        return this;
    },
    
    /**
    * Set the element as relative positioned with the specified z-index
    * @param {<i>Number</i>} zIndex (optional)
    */
    setRelativePositioned : function(zIndex){
        this.setStyle('position', 'relative');
        if(zIndex){
            this.setStyle('z-index', zIndex);
        }
        return this;
    },
    
    /**
    * Clear positioning back to the default when the document was loaded
    */
    clearPositioning : function(){
        this.setStyle('position', '');
        this.setStyle('left', '');
        this.setStyle('right', '');
        this.setStyle('top', '');
        this.setStyle('bottom', '');
        return this;
    },
    
    /**
    * Gets an object with all CSS positioning properties. Useful along with {@link #setPositioning} to get snapshot before performing an update and then restoring the element.
    */
    getPositioning : function(){
        return {
            'position' : this.getStyle('position'),
            'left' : this.getStyle('left'),
            'right' : this.getStyle('right'),
            'top' : this.getStyle('top'),
            'bottom' : this.getStyle('bottom')
        };
    },
    
    /**
     * Gets the width of the border(s) for the specified side(s)
     * @param {String} side Can be t, l, r, b or any combination of those to add multiple values. For example, 
     * passing lr would get the border (l)eft width + the border (r)ight width.
     * @return {Number} The width of the sides passed added together
     */
    getBorderWidth : function(side){
        return this.addStyles(side, YAHOO.ext.Element.borders);
    },
    
    /**
     * Gets the width of the padding(s) for the specified side(s)
     * @param {String} side Can be t, l, r, b or any combination of those to add multiple values. For example, 
     * passing lr would get the padding (l)eft + the padding (r)ight.
     * @return {Number} The padding of the sides passed added together
     */
    getPadding : function(side){
        return this.addStyles(side, YAHOO.ext.Element.paddings);
    },
    
    /**
    * Set positioning with an object returned by {@link #getPositioning}.
    */
    setPositioning : function(positionCfg){
        if(positionCfg.position)this.setStyle('position', positionCfg.position);
        if(positionCfg.left)this.setLeft(positionCfg.left);
        if(positionCfg.right)this.setRight(positionCfg.right);
        if(positionCfg.top)this.setTop(positionCfg.top);
        if(positionCfg.bottom)this.setBottom(positionCfg.bottom);
        return this;
    },
    
    setLeftTop : function(left, top){
        this.dom.style.left = this.addUnits(left);
        this.dom.style.top = this.addUnits(top);
        return this;
    },
    
    /**
     * Move this element relative to it's current position.
     * @param {String} direction Possible values are: 'left', 'right', 'up', 'down'.
     * @param {Number} distance How far to move the element in pixels
     * @param {<i>Boolean</i>} animate (optional) Animate the movement (Default is false)
     * @param {<i>Float</i>} duration (optional) How long the animation lasts. (Defaults to .35 seconds)
     * @param {<i>Function</i>} onComplete (optional) Function to call when animation completes.
     * @param {<i>Function</i>} easing (optional) YAHOO.util.Easing method to use. 
     */
     move : function(direction, distance, animate, duration, onComplete, easing){
        var xy = this.getXY();
        direction = direction.toLowerCase();
        switch(direction){
            case 'l':
            case 'left':
                this.moveTo(xy[0]-distance, xy[1], animate, duration, onComplete, easing);
                break;
           case 'r':
           case 'right':
                this.moveTo(xy[0]+distance, xy[1], animate, duration, onComplete, easing);
                break;
           case 't':
           case 'top':
           case 'up':
                this.moveTo(xy[0], xy[1]-distance, animate, duration, onComplete, easing);
                break;
           case 'b':
           case 'bottom':
           case 'down':
                this.moveTo(xy[0], xy[1]+distance, animate, duration, onComplete, easing);
                break;
        }
        return this;
    },
    
    /**
     *  Clip overflow on the element - use {@link #unclip} to remove
     */
    clip : function(){
        if(!this.isClipped){
           this.isClipped = true;
           this.originalClip = this.getStyle('overflow');
           this.setStyle('overflow', 'hidden');
        }
        return this;
    },
    
    /**
     *  Return clipping (overflow) to original clipping when the document loaded
     */
    unclip : function(){
        if(this.isClipped){
            this.isClipped = false;
            this.setStyle('overflow', this.originalClip);
        }
        return this;
    },
    
    /**
     * Align this element with another element.
     * @param {String/HTMLElement/YAHOO.ext.Element} element The element to align to.
     * @param {String} position The position to align to. Possible values are 'tl' - top left, 'tr' - top right, 'bl' - bottom left, and 'br' - bottom right. 
     * @param {<i>Array</i>} offsets (optional) Offset the positioning by [x, y]
     * @param {<i>Boolean</i>} animate (optional) Animate the movement (Default is false)
     * @param {<i>Float</i>} duration (optional) How long the animation lasts. (Defaults to .35 seconds)
     * @param {<i>Function</i>} onComplete (optional) Function to call when animation completes.
     * @param {<i>Function</i>} easing (optional) YAHOO.util.Easing method to use. 
     */
     alignTo : function(element, position, offsets, animate, duration, onComplete, easing){
        var otherEl = getEl(element);
        if(!otherEl){
            return this; // must not exist
        }
        offsets = offsets || [0, 0];
        var r = otherEl.getRegion();
        position = position.toLowerCase();
        switch(position){
           case 'bl':
                this.moveTo(r.left + offsets[0], r.bottom + offsets[1], 
                            animate, duration, onComplete, easing);
                break;
           case 'br':
                this.moveTo(r.right + offsets[0], r.bottom + offsets[1], 
                            animate, duration, onComplete, easing);
                break;
           case 'tl':
                this.moveTo(r.left + offsets[0], r.top + offsets[1], 
                            animate, duration, onComplete, easing);
                break;
           case 'tr':
                this.moveTo(r.right + offsets[0], r.top + offsets[1], 
                            animate, duration, onComplete, easing);
                break;
        }
        return this;
    },
    
    /**
    * Clears any opacity settings from this element. Required in some cases for IE.
    */
    clearOpacity : function(){
        if (window.ActiveXObject) {
            this.dom.style.filter = '';
        } else {
            this.dom.style.opacity = '';
            this.dom.style['-moz-opacity'] = '';
            this.dom.style['-khtml-opacity'] = '';
        }
        return this;
    },
    
    /**
    * Hide this element - Uses display mode to determine whether to use "display" or "visibility". See {@link #setVisible}.
    * @param {<i>Boolean</i>} animate (optional) Animate (fade) the transition (Default is false)
     * @param {<i>Float</i>} duration (optional) How long the animation lasts. (Defaults to .35 seconds)
     * @param {<i>Function</i>} onComplete (optional) Function to call when animation completes.
     * @param {<i>Function</i>} easing (optional) YAHOO.util.Easing method to use. (Defaults to YAHOO.util.Easing.easeBoth)
     */
    hide : function(animate, duration, onComplete, easing){
        this.setVisible(false, animate, duration, onComplete, easing);
        return this;
    },
    
    /**
    * Show this element - Uses display mode to determine whether to use "display" or "visibility". See {@link #setVisible}.
    * @param {<i>Boolean</i>} animate (optional) Animate (fade in) the transition (Default is false)
     * @param {<i>Float</i>} duration (optional) How long the animation lasts. (Defaults to .35 seconds)
     * @param {<i>Function</i>} onComplete (optional) Function to call when animation completes.
     * @param {<i>Function</i>} easing (optional) YAHOO.util.Easing method to use. (Defaults to YAHOO.util.Easing.easeBoth)
     */
    show : function(animate, duration, onComplete, easing){
        this.setVisible(true, animate, duration, onComplete, easing);
        return this;
    },
    
    /**
     * @private Test if size has a unit, otherwise appends the default 
     */
    addUnits : function(size){
        if(size === '' || size == 'auto' || typeof size == 'undefined'){
            return size;
        }
        if(typeof size == 'number' || !YAHOO.ext.Element.unitPattern.test(size)){
            return size + this.defaultUnit;
        }
        return size;
    },
    
    beginMeasure : function(){
        var el = this.dom;
        if(el.offsetWidth || el.offsetHeight){
            return this; // offsets work already
        }
        var changed = [];
        var p = this.dom; // start with this element
        while((!el.offsetWidth && !el.offsetHeight) && p && p.tagName && p.tagName.toLowerCase() != 'body'){
            if(YAHOO.util.Dom.getStyle(p, 'display') == 'none'){
                changed.push({el: p, visibility: YAHOO.util.Dom.getStyle(p, 'visibility')});
                p.style.visibility = 'hidden';
                p.style.display = 'block';
            }
            p = p.parentNode;
        }
        this._measureChanged = changed;
        return this;
               
    },
    
    endMeasure : function(){
        var changed = this._measureChanged;
        if(changed){
            for(var i = 0, len = changed.length; i < len; i++) {
            	var r = changed[i];
            	r.el.style.visibility = r.visibility;
                r.el.style.display = 'none';
            }
            this._measureChanged = null;
        }
        return this;
    },
    
    /**
    *   Update the innerHTML of this element, optionally searching for and processing scripts
    * @param {String} html The new HTML
    * @param {<i>Boolean</i>} loadScripts (optional) true to look for and process scripts
    */
    update : function(html, loadScripts){
        this.dom.innerHTML = html;
        if(!loadScripts) return this;
        
        var dom = this.dom;
        var _parseScripts = function(){
            var s = dom.getElementsByTagName("script");
            var docHead = document.getElementsByTagName("head")[0];
            
            //   For browsers which discard scripts when inserting innerHTML, extract the scripts using a RegExp
            if(s.length == 0){
                var re = /(?:<script.*(?:src=[\"\'](.*)[\"\']).*>.*<\/script>)|(?:<script.*>([\S\s]*?)<\/script>)/ig; // assumes HTML well formed and then loop through it.
                var match;
                while(match = re.exec(html)){
                     var s0 = document.createElement("script");
                     if (match[1])
                        s0.src = match[1];
                     else if (match[2])
                        s0.text = match[2];
                     else
                          continue;
                     docHead.appendChild(s0);
                }
            }else {
              for(var i = 0; i < s.length; i++){
                 var s0 = document.createElement("script");
                 s0.type = s[i].type;
                 if (s[i].text) {
                    s0.text = s[i].text;
                 } else {
                    s0.src = s[i].src;
                 }
                 docHead.appendChild(s0);
              }
            }
        }
        // set timeout to give DOM opportunity to catch up
        setTimeout(_parseScripts, 10);
        return this;
    },
    
    /**
     * Direct access to the UpdateManager update() method (takes the same parameters).
     */
    load : function(){
        var um = this.getUpdateManager();
        um.update.apply(um, arguments);
    },
    
    /**
    * Gets this elements UpdateManager
    * @return The UpdateManager
    * @type YAHOO.ext.UpdateManager 
    */
    getUpdateManager : function(){
        if(!this.updateManager){
            this.updateManager = new YAHOO.ext.UpdateManager(this);
        }
        return this.updateManager;
    },
    
    /**
    * Calculates the x, y to center this element on the screen
    * @return {Array} The x, y values [x, y]
    */
    getCenterXY : function(offsetScroll){
        var centerX = Math.round((YAHOO.util.Dom.getViewportWidth()-this.getWidth())/2);
        var centerY = Math.round((YAHOO.util.Dom.getViewportHeight()-this.getHeight())/2);
        if(!offsetScroll){
            return [centerX, centerY];
        }else{
            var scrollX = document.documentElement.scrollLeft || document.body.scrollLeft || 0;
            var scrollY = document.documentElement.scrollTop || document.body.scrollTop || 0;
            return[centerX + scrollX, centerY + scrollY];
        }
    },
    /**
    * Gets an array of child YAHOO.ext.Element objects by tag name
    * @param {String} tagName
    * @return {Array} The children
    */
    getChildrenByTagName : function(tagName){
        var children = this.dom.getElementsByTagName(tagName);
        var len = children.length;
        var ce = new Array(len);
        for(var i = 0; i < len; ++i){
            ce[i] = YAHOO.ext.Element.get(children[i], true);
        }
        return ce;
    },
    
    /**
    * Gets an array of child YAHOO.ext.Element objects by class name and optional tagName
    * @param {String} className
    * @param {<i>String</i>} tagName (optional)
    * @return {Array} The children
    */
    getChildrenByClassName : function(className, tagName){
        var children = YAHOO.util.Dom.getElementsByClassName(className, tagName, this.dom);
        var len = children.length;
        var ce = new Array(len);
        for(var i = 0; i < len; ++i){
            ce[i] = YAHOO.ext.Element.get(children[i], true);
        }
        return ce;
    },
    
    /**
     * Tests various css rules/browsers to determine if this element uses a border box
     */
    isBorderBox : function(){
        if(typeof this.bbox == 'undefined'){
            var el = this.dom;
            var b = YAHOO.ext.util.Browser;
            var strict = YAHOO.ext.Strict;
            this.bbox = ((b.isIE && !strict && el.style.boxSizing != 'content-box') || 
               (b.isGecko && YAHOO.util.Dom.getStyle(el, "-moz-box-sizing") == 'border-box') || 
               (!b.isSafari && YAHOO.util.Dom.getStyle(el, "box-sizing") == 'border-box'));
        }
        return this.bbox; 
    },
    
    /**
     * Return a box {x, y, width, height} that can be used to set another elements
     * size to match this element. If contentBox is true, a box for the content 
     * of the element is returned. If local is true, the element's left and top are returned instead of page x/y.
     */
    getBox : function(contentBox, local){
        var xy;
        if(!local){
            xy = this.getXY();
        }else{
            var left = parseInt(YAHOO.util.Dom.getStyle('left'), 10) || 0;
            var top = parseInt(YAHOO.util.Dom.getStyle('top'), 10) || 0;
            xy = [left, top];
        }
        var el = this.dom;
        var w = el.offsetWidth;
        var h = el.offsetHeight;
        if(!contentBox){
            return {x: xy[0], y: xy[1], width: w, height: h};
        }else{
            var l = this.getBorderWidth('l')+this.getPadding('l');
            var r = this.getBorderWidth('r')+this.getPadding('r');
            var t = this.getBorderWidth('t')+this.getPadding('t');
            var b = this.getBorderWidth('b')+this.getPadding('b');
            return {x: xy[0]+l, y: xy[1]+t, width: w-(l+r), height: h-(t+b)};
        }
    },
    
    /**
     * Sets the element's box. Use getBox() on another element to get a box obj. If animate is true then width, height, x and y will be animated concurrently.
     * @param {Object} box The box to fill {x, y, width, height}
     * @param {<i>Boolean</i>} adjust (optional) Whether to adjust for box-model issues automatically
     * @param {<i>Boolean</i>} animate (optional) Animate the transition (Default is false)
     * @param {<i>float</i>} duration (optional) How long the animation lasts. (Defaults to .35 seconds)
     * @param {<i>Function</i>} onComplete (optional) Function to call when animation completes.
     * @param {<i>Function</i>} easing (optional) YAHOO.util.Easing method to use. (Defaults to YAHOO.util.Easing.easeBoth)
     */
    setBox : function(box, adjust, animate, duration, onComplete, easing){
        var w = box.width, h = box.height;
        if((adjust && !this.autoBoxAdjust) && !this.isBorderBox()){
           w -= (this.getBorderWidth('lr') + this.getPadding('lr'));
           h -= (this.getBorderWidth('tb') + this.getPadding('tb'));
        }
        this.setBounds(box.x, box.y, w, h, animate, duration, onComplete, easing);
        return this;
    },
    
    repaint : function(){
        var dom = this.dom;
        YAHOO.util.Dom.addClass(dom, 'yui-ext-repaint');
        setTimeout(function(){
            YAHOO.util.Dom.removeClass(dom, 'yui-ext-repaint');
        }, 1);
        return this;
    },
    
    /**
     * Returns an object with properties top, left, right and bottom representing the margins of this element unless sides is passed, 
     * then it returns the calculated width of the sides (see getPadding)
     */
    getMargins : function(side){
        if(!side){
            return {
                top: parseInt(this.getStyle('margin-top'), 10) || 0,
                left: parseInt(this.getStyle('margin-left'), 10) || 0,
                bottom: parseInt(this.getStyle('margin-bottom'), 10) || 0,
                right: parseInt(this.getStyle('margin-right'), 10) || 0
            };
        }else{
            return this.addStyles(side, YAHOO.ext.Element.margins);
         }
    },
    
    addStyles : function(sides, styles){
        var val = 0;
        for(var i = 0, len = sides.length; i < len; i++){
             var w = parseInt(this.getStyle(styles[sides.charAt(i)]), 10);
             if(!isNaN(w)) val += w;
        }
        return val;
    },
    
    /**
     * Creates a proxy element of this element
     * @param {String} className The class name of the proxy element
     * @param {<i>String/HTMLElement</i>} renderTo (optional) The element or element id to render the proxy to (defaults to document.body)
     * @param {<i>Boolean</i>} matchBox (optional) True to align and size the proxy to this element now (defaults to false)
     * @return {YAHOO.ext.Element} The new proxy element
     */
    createProxy : function(className, renderTo, matchBox){
        if(renderTo){
            renderTo = YAHOO.util.Dom.get(renderTo);
        }else{
            renderTo = document.body;
        }
        var proxy = YAHOO.ext.DomHelper.append(renderTo, 
                {tag : 'div', cls: className, id: this.dom.id + '-proxy'}, true);
        if(matchBox){
           proxy.setBox(this.getBox());
        }
        return proxy;
    },
    
    /**
     * Removes this element from the DOM and deletes it from the cache
     */
    remove : function(){
        this.dom.parentNode.removeChild(this.dom);
        delete YAHOO.ext.Element.cache[this.dom.id];
    },
    
    /**
     * Adds and removes a css class when the mosue is over this element
     */
    addClassOnOver : function(className){
        this.on('mouseover', function(){
            this.addClass(className);
        }, this, true);
        this.on('mouseout', function(){
            this.removeClass(className);
        }, this, true);
        return this;
    },
    
    /**
     * Stops the specified event from bubbling and optionally prevent's the default action
     */
    swallowEvent : function(eventName, preventDefault){
        var fn = function(e){
            e.stopPropagation();
            if(preventDefault){
                e.preventDefault();
            }
        };
        this.mon(eventName, fn);
        return this;
    },
    
    /**
     * Sizes this element to it's parent element's dimensions performing 
     * neccessary box adjustments. 
     * @param {Boolean} monitorResize (optional) If true maintains the fit when the browser window is resized.
     */
    fitToParent : function(monitorResize){
        var p = getEl(this.dom.parentNode, true);
        p.beginMeasure(); // in case parent is display:none
        var box = p.getBox(true, true);
        p.endMeasure();
        this.setSize(box.width, box.height);
        if(monitorResize === true){
            YAHOO.ext.EventManager.onWindowResize(this.fitToParent, this, true);
        }
        return this;
    },
    
    /**
     * Appends the passed element(s) to this element
     * @param {String/HTMLElement/Array/Element/CompositeElement} el
     */
    appendChild: function(el){
        el = getEl(el);
        el.appendTo(this);
        return this;
    },
    
    /**
     * Creates the passed DomHelper config and appends it to this element or optionally inserts it before the passed child element.
     * @param {Object} config DomHelper element config object
     * @param {<i>HTMLElement</i>} insertBefore (optional) a child element of this element
     */
    createChild: function(config, insertBefore){
        var c;
        if(insertBefore){
            c = YAHOO.ext.DomHelper.insertBefore(insertBefore, config, true);
        }else{
            c = YAHOO.ext.DomHelper.append(this.dom, config, true);
        }
        return c;
    },
    
    /**
     * Appends this element to the passed element
     * @param {String/HTMLElement/Element} el The new parent element
     */
    appendTo: function(el){
        var node = getEl(el).dom;
        node.appendChild(this.dom);
        return this;
    },
    
    /**
     * Inserts this element before the passed element in the DOM
     * @param {String/HTMLElement/Element} el The element to insert before
     */
    insertBefore: function(el){
        var node = getEl(el).dom;
        node.parentNode.insertBefore(this.dom, node);
        return this;
    },
    
    /**
     * Inserts this element after the passed element in the DOM
     * @param {String/HTMLElement/Element} el The element to insert after
     */
    insertAfter: function(el){
        var node = getEl(el).dom;
        node.parentNode.insertBefore(this.dom, node.nextSibling);
        return this;
    },
    
    /**
     * Creates and wraps this element with another element
     * @param {Object} config DomHelper element config object for the wrapper element or null for an empty div
     * @return {Element} The newly created wrapper element
     */
    wrap: function(config){
        if(!config){
            config = {tag: 'div'};
        }
        var newEl = YAHOO.ext.DomHelper.insertBefore(this.dom, config, true);
        newEl.dom.appendChild(this.dom);
        return newEl;
    },
    
    /**
     * Replaces the passed element with this element
     * @param {String/HTMLElement/Element} el The element to replace
     */
    replace: function(el){
        el = getEl(el);
        this.insertBefore(el);
        el.remove();
        return this;
    },
    
    /**
     * Inserts an html fragment into this element
     * @param {String} where Where to insert the html in relation to the this element - beforeBegin, afterBegin, beforeEnd, afterEnd.
     * @param {String} html The HTML fragment
     */
    insertHtml : function(where, html){
        YAHOO.ext.DomHelper.insertHtml(where, this.dom, html);
        return this;
    },
    
    /**
     * Sets the passed attributes as attributes of this element (a style attribute can be a string, object or function)
     * @param {Object} o The object with the attributes
     */
    set : function(o){
        var el = this.dom;
        var useSet = el.setAttribute ? true : false;
        for(var attr in o){
            if(attr == 'style' || typeof o[attr] == 'function') continue;
            if(attr=='cls'){
                el.className = o['cls'];
            }else{
                if(useSet) el.setAttribute(attr, o[attr]);
                else el[attr] = o[attr];
            }
        }
        YAHOO.ext.DomHelper.applyStyles(el, o.style);
        return this;
    }
};

/**
 * Whether to automatically adjust width and height settings for box-model issues
 */
YAHOO.ext.Element.prototype.autoBoxAdjust = true;
YAHOO.ext.Element.prototype.autoDisplayMode = false;

/**
 * @private Used to check if a value has a unit
 */
YAHOO.ext.Element.unitPattern = /\d+(px|em|%|en|ex|pt|in|cm|mm|pc)$/i;
/**
 * Visibility mode constant - Use visibility to hide element
 * @type Number
 */
YAHOO.ext.Element.VISIBILITY = 1;
/**
 * Visibility mode constant - Use display to hide element
 * @type Number
 */
YAHOO.ext.Element.DISPLAY = 2;

YAHOO.ext.Element.blockElements = /^(?:address|blockquote|center|dir|div|dl|fieldset|form|h\d|hr|isindex|menu|ol|ul|p|pre|table|dd|dt|li|tbody|tr|td|thead|tfoot|iframe)$/i;

/** @ignore */
YAHOO.ext.Element.borders = {l: 'border-left-width', r: 'border-right-width', t: 'border-top-width', b: 'border-bottom-width'};
/** @ignore */
YAHOO.ext.Element.paddings = {l: 'padding-left', r: 'padding-right', t: 'padding-top', b: 'padding-bottom'};
YAHOO.ext.Element.margins = {l: 'margin-left', r: 'margin-right', t: 'margin-top', b: 'margin-bottom'};
        
/**
 * @private Call out to here so we make minimal closure
 */
YAHOO.ext.Element.createStopHandler = function(stopPropagation, handler, scope, override){
    return function(e){
        if(e){
            if(stopPropagation){
                YAHOO.util.Event.stopEvent(e);
            }else {
                YAHOO.util.Event.preventDefault(e);
            }
        }
        handler.call(override && scope ? scope : window, e, scope);
    };
};

/**
 * @private
 */
YAHOO.ext.Element.cache = {};

/**
 * Static method to retreive Element objects. Uses simple caching to consistently return the same object. 
 * Automatically fixes if an object was recreated with the same id via AJAX or DOM.
 * @param {String/HTMLElement/Element} el The id of the element or the element to wrap (must have an id). If you pass in an element, it is returned
 * @param {<i>Boolean</i>} autoGenerateId (optional) Set this flag to true if you are passing an element without an id (like document.body). It will auto generate an id if one isn't present. 
 * @return {Element} The element object
 */
YAHOO.ext.Element.get = function(el, autoGenerateId){
    if(!el){ return null; }
    autoGenerateId = true; // now generates id by default
    if(el instanceof YAHOO.ext.Element){
        el.dom = YAHOO.util.Dom.get(el.id); // refresh dom element in case no longer valid
        YAHOO.ext.Element.cache[el.id] = el; // in case it was created directly with Element(), let's cache it
        return el;
    }else if(el.isComposite){
        return el;
    }else if(el instanceof Array){
        return YAHOO.ext.Element.select(el);
    }
    var key = el;
    if(typeof el != 'string'){ // must be an element
        if(!el.id && !autoGenerateId){ return null; }
        YAHOO.util.Dom.generateId(el, 'elgen-');
        key = el.id;
    }
    var element = YAHOO.ext.Element.cache[key];
    if(!element){
        element = new YAHOO.ext.Element(key);
        if(!element.dom) return null;
        YAHOO.ext.Element.cache[key] = element;
    }else{
        element.dom = YAHOO.util.Dom.get(key);
    }
    return element;
};

/**
 * Shorthand function for YAHOO.ext.Element.get()
 */
var getEl = YAHOO.ext.Element.get;

// clean up refs
YAHOO.util.Event.addListener(window, 'unload', function(){ YAHOO.ext.Element.cache = null; });

/*
------------------------------------------------------------------
// File: \CompositeElement.js
------------------------------------------------------------------
*/
/**
 * Heavyweight composite class. Creates a YAHOO.ext.Element for every element.
 */
YAHOO.ext.CompositeElement = function(els){
    this.elements = [];
    this.addElements(els);
};
YAHOO.ext.CompositeElement.prototype = {
    isComposite: true,
    addElements : function(els){
        if(!els) return this;
        var yels = this.elements;
        var index = yels.length-1;
        for(var i = 0, len = els.length; i < len; i++) {
        	yels[++index] = getEl(els[i], true);
        }
        return this;
    },
    invoke : function(fn, args){
        var els = this.elements;
        for(var i = 0, len = els.length; i < len; i++) {
        	YAHOO.ext.Element.prototype[fn].apply(els[i], args);
        }
        return this;
    },
    /**
    * Adds elements to this composite
    * @param {String} els A string CSS selector, an array of elements or an element
    */
    add : function(els){
        if(typeof els == 'string'){
            this.addElements(YAHOO.ext.Element.selectorFunction(string));
        }else if(els instanceof Array){
            this.addElements(els);
        }else{
            this.addElements([els]);
        }
        return this;
    },
    /**
    * Calls the passed function passing (el, this, index) for each element in this composite.
    */
    each : function(fn, scope){
        var els = this.elements;
        for(var i = 0, len = els.length; i < len; i++){
            fn.call(scope || els[i], els[i], this, i);
        }
        return this;
    }
};
/**
 * Flyweight composite class. Reuses the same YAHOO.ext.Element for element operations.
 */
YAHOO.ext.CompositeElementLite = function(els){
    YAHOO.ext.CompositeElementLite.superclass.constructor.call(this, els);
    this.el = YAHOO.ext.Element.get(this.elements[0], true);
};
YAHOO.extendX(YAHOO.ext.CompositeElementLite, YAHOO.ext.CompositeElement, {
    addElements : function(els){
        if(els){
            this.elements = this.elements.concat(els);
        }
        return this;
    },
    invoke : function(fn, args){
        var els = this.elements;
        var el = this.el;
        for(var i = 0, len = els.length; i < len; i++) {
            el.dom = els[i];
        	YAHOO.ext.Element.prototype[fn].apply(el, args);
        }
        return this;
    }
});
YAHOO.ext.CompositeElement.createCall = function(proto, fnName){
    if(!proto[fnName]){
        proto[fnName] = function(){
            return this.invoke(fnName, arguments);  
        };
    }
};
for(var fnName in YAHOO.ext.Element.prototype){
    if(typeof YAHOO.ext.Element.prototype[fnName] == 'function'){
        YAHOO.ext.CompositeElement.createCall(YAHOO.ext.CompositeElement.prototype, fnName);
    }
}
if(typeof cssQuery == 'function'){// Dean Edwards cssQuery
    YAHOO.ext.Element.selectorFunction = cssQuery;
}else if(typeof document.getElementsBySelector == 'function'){ // Simon Willison's getElementsBySelector
    YAHOO.ext.Element.selectorFunction = document.getElementsBySelector.createDelegate(document);
}
/**
* Selects elements based on the passed CSS selector
* @param {String} selector The CSS selector
* @param {Boolean} unique true to create a unique YAHOO.ext.Element for each element (defaults to a shared flyweight object)
*/
YAHOO.ext.Element.select = function(selector, unique){
    var els;
    if(typeof selector == 'string'){
        els = YAHOO.ext.Element.selectorFunction(selector);
    }else if(selector instanceof Array){
        els = selector;
    }else{
        throw 'Invalid selector';
    }
    if(unique === true){
        return new YAHOO.ext.CompositeElement(els);
    }else{
        return new YAHOO.ext.CompositeElementLite(els);
    }
};

var getEls = YAHOO.ext.Element.select;

/*
------------------------------------------------------------------
// File: \State.js
------------------------------------------------------------------
*/
YAHOO.namespace('ext.state');

YAHOO.ext.state.Provider = function(){
    YAHOO.ext.state.Provider.superclass.constructor.call(this);
    this.events = {
        'statechange': new YAHOO.util.CustomEvent('statechange')  
    };
    this.state = {};
};
YAHOO.extendX(YAHOO.ext.state.Provider, YAHOO.ext.util.Observable, {
    get : function(name, defaultValue){
        return typeof this.state[name] == 'undefined' ?
            defaultValue : this.state[name];
    },
    
    clear : function(name){
        delete this.state[name];
        this.fireEvent('statechange', this, name, null);
    },
    
    set : function(name, value){
        this.state[name] = value;
        this.fireEvent('statechange', this, name, value);
    },
    
    decodeValue : function(cookie){
        var re = /^(a|n|d|b|s|o)\:(.*)$/;
        var matches = re.exec(unescape(cookie));
        if(!matches || !matches[1]) return; // non state cookie
        var type = matches[1];
        var v = matches[2];
        switch(type){
            case 'n':
                return parseFloat(v);
            case 'd':
                return new Date(Date.parse(v));
            case 'b':
                return (v == '1');
            case 'a':
                var all = [];
                var values = v.split('^');
                for(var i = 0, len = values.length; i < len; i++){
                    all.push(this.decodeValue(values[i]))
                }
                return all;
           case 'o':
                var all = {};
                var values = v.split('^');
                for(var i = 0, len = values.length; i < len; i++){
                    var kv = values[i].split('=');
                    all[kv[0]] = this.decodeValue(kv[1]);
                }
                return all;
           default:
                return v;
        }
    },
    
    encodeValue : function(v){
        var enc;
        if(typeof v == 'number'){
            enc = 'n:' + v;
        }else if(typeof v == 'boolean'){
            enc = 'b:' + (v ? '1' : '0');
        }else if(v instanceof Date){
            enc = 'd:' + v.toGMTString();
        }else if(v instanceof Array){
            var flat = '';
            for(var i = 0, len = v.length; i < len; i++){
                flat += this.encodeValue(v[i]);
                if(i != len-1) flat += '^';
            }
            enc = 'a:' + flat;
        }else if(typeof v == 'object'){
            var flat = '';
            for(var key in v){
                if(typeof v[key] != 'function'){
                    flat += key + '=' + this.encodeValue(v[key]) + '^';
                }
            }
            enc = 'o:' + flat.substring(0, flat.length-1);
        }else{
            enc = 's:' + v;
        }
        return escape(enc);        
    }
});

YAHOO.ext.state.Manager = new function(){
    var provider = new YAHOO.ext.state.Provider();
    
    return {
        setProvider : function(stateProvider){
            provider = stateProvider;
        },
        
        get : function(key, defaultValue){
            return provider.get(key, defaultValue);
        },
        
        set : function(key, value){
            provider.set(key, value);
        },
        
        clear : function(key){
            provider.clear(key);
        },
        
        getProvider : function(){
            return provider;
        }
    };
}();

YAHOO.ext.state.CookieProvider = function(config){
    YAHOO.ext.state.CookieProvider.superclass.constructor.call(this);
    this.path = '/';
    this.expires = new Date(new Date().getTime()+(1000*60*60*24*7)); //7 days
    this.domain = null;
    this.secure = false;
    YAHOO.ext.util.Config.apply(this, config);
    this.state = this.readCookies();
};

YAHOO.extendX(YAHOO.ext.state.CookieProvider, YAHOO.ext.state.Provider, {
    set : function(name, value){
        if(typeof value == 'undefined' || value === null){
            this.clear(name);
            return;
        }
        this.setCookie(name, value);
        YAHOO.ext.state.CookieProvider.superclass.set.call(this, name, value);
    },
        
    clear : function(name){
        this.clearCookie(name);
        YAHOO.ext.state.CookieProvider.superclass.clear.call(this, name);
    },
        
    readCookies : function(){
        var cookies = {};
        var c = document.cookie + ';';
        var re = /\s?(.*?)=(.*?);/g;
    	var matches;
    	while((matches = re.exec(c)) != null){
            var name = matches[1];
            var value = matches[2];
            if(name && name.substring(0,3) == 'ys-'){
                cookies[name.substr(3)] = this.decodeValue(value);
            }
        }
        return cookies;
    },
    
    setCookie : function(name, value){
        document.cookie = "ys-"+ name + "=" + this.encodeValue(value) +
           ((this.expires == null) ? "" : ("; expires=" + this.expires.toGMTString())) +
           ((this.path == null) ? "" : ("; path=" + this.path)) +
           ((this.domain == null) ? "" : ("; domain=" + this.domain)) +
           ((this.secure == true) ? "; secure" : "");
    },
    
    clearCookie : function(name){
        document.cookie = "ys-" + name + "=null; expires=Thu, 01-Jan-70 00:00:01 GMT" +
           ((this.path == null) ? "" : ("; path=" + this.path)) +
           ((this.domain == null) ? "" : ("; domain=" + this.domain)) +
           ((this.secure == true) ? "; secure" : "");
    }
});


/*
------------------------------------------------------------------
// File: \EventManager.js
------------------------------------------------------------------
*/

/**
 * @class 
 * Registers event handlers that want to receive an EventObject instead of the standard browser event
 * See {@link YAHOO.ext.EventObject} for more details on the usage of this object.
 */
YAHOO.ext.EventManager = new function(){
    var docReadyEvent;
    var docReadyProcId;
    var docReadyState = false;
    this.ieDeferSrc = null;
    var resizeEvent;
    var resizeTask;
    
    var fireDocReady = function(){
        if(!docReadyState){
            docReadyState = true;
            if(docReadyProcId){
                clearInterval(docReadyProcId);
            }
            if(docReadyEvent){
                docReadyEvent.fire();
            }
        }
    };
    
    var initDocReady = function(){
        docReadyEvent = new YAHOO.util.CustomEvent('documentready');
        if(document.addEventListener) {
            YAHOO.util.Event.on(document, "DOMContentLoaded", fireDocReady);
        }else if(YAHOO.ext.util.Browser.isIE){
            // inspired by  http://www.thefutureoftheweb.com/blog/2006/6/adddomloadevent
            document.write('<s'+'cript id="ie-deferred-loader" defer="defer" src="' +
                        (YAHOO.ext.EventManager.ieDeferSrc || YAHOO.ext.SSL_SECURE_URL) + '"></s'+'cript>');
            YAHOO.util.Event.on('ie-deferred-loader', 'readystatechange', function(){
                if(this.readyState == 'complete'){
                    fireDocReady();
                }
            });
        }else if(YAHOO.ext.util.Browser.isSafari){ 
            docReadyProcId = setInterval(function(){
                var rs = document.readyState;
                if(rs == 'loaded' || rs == 'complete') {
                    fireDocReady();     
                 }
            }, 10);
        }
        // no matter what, make sure it fires on load
        YAHOO.util.Event.on(window, 'load', fireDocReady);
    };
    /** 
     * Places a simple wrapper around an event handler to override the browser event 
     * object with a YAHOO.ext.EventObject
     */
    this.wrap = function(fn, scope, override){
        var wrappedFn = function(e){
            YAHOO.ext.EventObject.setEvent(e);
            fn.call(override ? scope || window : window, YAHOO.ext.EventObject, scope);
        };
        return wrappedFn;
    };
    
    /**
     * Appends an event handler
     *
     * @param {Object}   element        The html element to assign the 
     *                             event to
     * @param {String}   eventName     The type of event to append
     * @param {Function} fn        The method the event invokes
     * @param {Object}   scope    An arbitrary object that will be 
     *                             passed as a parameter to the handler
     * @param {boolean}  override If true, the obj passed in becomes
     *                             the execution scope of the listener
     * @return {Function} The wrapper function created (to be used to remove the listener if necessary)
     */
    this.addListener = function(element, eventName, fn, scope, override){
        var wrappedFn = this.wrap(fn, scope, override);
        YAHOO.util.Event.addListener(element, eventName, wrappedFn);
        return wrappedFn;
    };
    
    /**
     * Removes an event handler
     *
     * @param {Object}   element        The html element to remove the 
     *                             event from
     * @param {String}   eventName     The type of event to append
     * @param {Function} wrappedFn        The wrapper method returned when adding the listener
     * @return {Boolean} True if a listener was actually removed
     */
    this.removeListener = function(element, eventName, wrappedFn){
        return YAHOO.util.Event.removeListener(element, eventName, wrappedFn);
    };
    
    /**
     * Appends an event handler (shorthand for addListener)
     *
     * @param {Object}   element        The html element to assign the 
     *                             event to
     * @param {String}   eventName     The type of event to append
     * @param {Function} fn        The method the event invokes
     * @param {Object}   scope    An arbitrary object that will be 
     *                             passed as a parameter to the handler
     * @param {boolean}  override If true, the obj passed in becomes
     *                             the execution scope of the listener
     * @return {Function} The wrapper function created (to be used to remove the listener if necessary)
     */
    this.on = function(element, eventName, fn, scope, override){
        var wrappedFn = this.wrap(fn, scope, override);
        YAHOO.util.Event.addListener(element, eventName, wrappedFn);
        return wrappedFn;
    };
    
    /**
     * Fires when the document is ready (before onload and before images are loaded)
     * @param {Function} fn        The method the event invokes
     * @param {Object}   scope    An arbitrary object that will be 
     *                             passed as a parameter to the handler
     * @param {boolean}  override If true, the obj passed in becomes
     *                             the execution scope of the listener
     */
    this.onDocumentReady = function(fn, scope, override){
        if(!docReadyEvent){
            initDocReady();
        }
        docReadyEvent.subscribe(fn, scope, override);
    }
    
    /**
     * Fires when the window is resized and provides resize event buffering (100 milliseconds), passes new viewport width and height to handlers.
     * @param {Function} fn        The method the event invokes
     * @param {Object}   scope    An arbitrary object that will be 
     *                             passed as a parameter to the handler
     * @param {boolean}  override If true, the obj passed in becomes
     *                             the execution scope of the listener
     */
    this.onWindowResize = function(fn, scope, override){
        if(!resizeEvent){
            resizeEvent = new YAHOO.util.CustomEvent('windowresize');
            resizeTask = new YAHOO.ext.util.DelayedTask(function(){
                resizeEvent.fireDirect(YAHOO.util.Dom.getViewportWidth(), YAHOO.util.Dom.getViewportHeight());
            });
            YAHOO.util.Event.on(window, 'resize', function(){
                resizeTask.delay(50);
            });
        }
        resizeEvent.subscribe(fn, scope, override);
    }
};

/**
 * @class 
 * EventObject exposes the Yahoo! UI Event functionality directly on the object
 * passed to your event handler. It exists mostly for convenience. It also fixes the annoying null checks automatically to cleanup your code 
 * (All the YAHOO.util.Event methods throw javascript errors if the passed event is null).
 * To get an EventObject instead of the standard browser event,
 * your must register your listener thru the {@link YAHOO.ext.EventManager} or directly on an Element
 * with {@link YAHOO.ext.Element#addManagedListener} or the shorthanded equivalent {@link YAHOO.ext.Element#mon}.<br>
 * Example:
 * <pre><code>
 fu<>nction handleClick(e){ // e is not a standard event object, it is a YAHOO.ext.EventObject
    e.preventDefault();
    var target = e.getTarget();
    ...
 }
 var myDiv = getEl('myDiv');
 myDiv.mon('click', handleClick);
 //or
 YAHOO.ext.EventManager.on('myDiv', 'click', handleClick);
 YAHOO.ext.EventManager.addListener('myDiv', 'click', handleClick);
 </code></pre>
 */
YAHOO.ext.EventObject = new function(){
    /** The normal browser event */ 
    this.browserEvent = null;
    /** The button pressed in a mouse event */ 
    this.button = -1;
    /** True if the shift key was down during the event */ 
    this.shiftKey = false;
    /** True if the control key was down during the event */ 
    this.ctrlKey = false;
    /** True if the alt key was down during the event */ 
    this.altKey = false;
    
    /** Key constant @type Number */
    this.BACKSPACE = 8;
    /** Key constant @type Number */
    this.TAB = 9;
    /** Key constant @type Number */
    this.RETURN = 13;
    /** Key constant @type Number */
    this.ESC = 27;
    /** Key constant @type Number */
    this.SPACE = 32;
    /** Key constant @type Number */
    this.PAGEUP = 33;
    /** Key constant @type Number */
    this.PAGEDOWN = 34;
    /** Key constant @type Number */
    this.END = 35;
    /** Key constant @type Number */
    this.HOME = 36;
    /** Key constant @type Number */
    this.LEFT = 37;
    /** Key constant @type Number */
    this.UP = 38;
    /** Key constant @type Number */
    this.RIGHT = 39;
    /** Key constant @type Number */
    this.DOWN = 40;
    /** Key constant @type Number */
    this.DELETE = 46;
    /** Key constant @type Number */
    this.F5 = 116;

       /** @private */ 
    this.setEvent = function(e){
        this.browserEvent = e;
        if(e){
            this.button = e.button;
            this.shiftKey = e.shiftKey;
            this.ctrlKey = e.ctrlKey;
            this.altKey = e.altKey;
        }else{
            this.button = -1;
            this.shiftKey = false;
            this.ctrlKey = false;
            this.altKey = false;
        }
    };
    
    /**
     * Stop the event. Calls YAHOO.util.Event.stopEvent() if the event is not null.
     */ 
    this.stopEvent = function(){
        if(this.browserEvent){
            YAHOO.util.Event.stopEvent(this.browserEvent);
        }
    };
    
    /**
     * Prevents the browsers default handling of the event. Calls YAHOO.util.Event.preventDefault() if the event is not null.
     */ 
    this.preventDefault = function(){
        if(this.browserEvent){
            YAHOO.util.Event.preventDefault(this.browserEvent);
        }
    };
    
    /** @private */
    this.isNavKeyPress = function(){
        return (this.browserEvent.keyCode && this.browserEvent.keyCode >= 33 && this.browserEvent.keyCode <= 40);
    };
    
    /**
     * Cancels bubbling of the event. Calls YAHOO.util.Event.stopPropagation() if the event is not null.
     */ 
    this.stopPropagation = function(){
        if(this.browserEvent){
            YAHOO.util.Event.stopPropagation(this.browserEvent);
        }
    };
    
    /**
     * Gets the key code for the event. Returns value from YAHOO.util.Event.getCharCode() if the event is not null.
     * @return {Number}
     */ 
    this.getCharCode = function(){
        if(this.browserEvent){
            return YAHOO.util.Event.getCharCode(this.browserEvent);
        }
        return null;
    };
    
    /**
     * Returns a browsers key for a keydown event
     */
    this.getKey = function(){
        if(this.browserEvent){
            return this.browserEvent.charCode || this.browserEvent.keyCode;
        }
        return null;
    };
    
    /**
     * Gets the x coordinate of the event. Returns value from YAHOO.util.Event.getPageX() if the event is not null.
     * @return {Number}
     */ 
    this.getPageX = function(){
        if(this.browserEvent){
            return YAHOO.util.Event.getPageX(this.browserEvent);
        }
        return null;
    };
    
    /**
     * Gets the y coordinate of the event. Returns value from YAHOO.util.Event.getPageY() if the event is not null.
     * @return {Number}
     */ 
    this.getPageY = function(){
        if(this.browserEvent){
            return YAHOO.util.Event.getPageY(this.browserEvent);
        }
        return null;
    };
    
    /**
     * Gets the time of the event. Returns value from YAHOO.util.Event.getTime() if the event is not null.
     * @return {Number}
     */ 
    this.getTime = function(){
        if(this.browserEvent){
            return YAHOO.util.Event.getTime(this.browserEvent);
        }
        return null;
    };
    
    /**
     * Gets the page coordinates of the event. Returns value from YAHOO.util.Event.getXY() if the event is not null.
     * @return {Array} The xy values like [x, y]
     */ 
    this.getXY = function(){
        if(this.browserEvent){
            return YAHOO.util.Event.getXY(this.browserEvent);
        }
        return [];
    };
    
    /**
     * Gets the target for the event. Returns value from YAHOO.util.Event.getTarget() if the event is not null.
     * @return {HTMLelement}
     */ 
    this.getTarget = function(){
        if(this.browserEvent){
            return YAHOO.util.Event.getTarget(this.browserEvent);
        }
        return null;
    };
    
    /**
     * Walk up the DOM looking for a particular target - if the default target matches, it is returned.
     * @return {HTMLelement}
     */ 
    this.findTarget = function(className, tagName){
        if(tagName) tagName = tagName.toLowerCase();
        if(this.browserEvent){
            function isMatch(el){
               if(!el){
                   return false;
               }
               if(className && !YAHOO.util.Dom.hasClass(el, className)){
                   return false;
               }
               if(tagName && el.tagName.toLowerCase() != tagName){
                   return false;
               }
               return true;
            };
            
            var t = this.getTarget();
            if(!t || isMatch(t)){
    		    return t;
    	    }
    	    var p = t.parentNode;
    	    var b = document.body;
    	    while(p && p != b){
                if(isMatch(p)){
                	return p;
                }
                p = p.parentNode;
            }
    	}
        return null;
    };
    /**
     * Gets the related target. Returns value from YAHOO.util.Event.getRelatedTarget() if the event is not null.
     * @return {HTMLelement}
     */ 
    this.getRelatedTarget = function(){
        if(this.browserEvent){
            return YAHOO.util.Event.getRelatedTarget(this.browserEvent);
        }
        return null;
    };
    
    /**
     * Normalizes mouse wheel delta across browsers
     */
    this.getWheelDelta = function(){
        var e = this.browserEvent;
        var delta = 0;
        if(e.wheelDelta){ /* IE/Opera. */
            delta = e.wheelDelta/120;
            /** In Opera 9, delta differs in sign as compared to IE. */
            if(window.opera) delta = -delta;
        }else if(e.detail){ /** Mozilla case. */
            delta = -e.detail/3;
        }
        return delta;
    };
    
    /**
     * Returns true if the control, shift or alt key was pressed during this event.
     * @return {Boolean}
     */ 
    this.hasModifier = function(){
        return this.ctrlKey || this.altKey || this.shiftKey;
    };
}();
            
    

/*
------------------------------------------------------------------
// File: \UpdateManager.js
------------------------------------------------------------------
*/
/**
 * @class Provides AJAX-style update for Element object using Yahoo 
 * UI library YAHOO.util.Connect functionality.<br><br>
 * Usage:<br>
 * <pre><code>
 * // Get it from a YAHOO.ext.Element object
 * var mgr = myElement.getUpdateManager();
 * mgr.update('http://myserver.com/index.php', 'param1=1&amp;param2=2');
 * ...
 * mgr.formUpdate('myFormId', 'http://myserver.com/index.php');
 * <br>
 * // or directly (returns the same UpdateManager instance)
 * var mgr = new YAHOO.ext.UpdateManager('myElementId');
 * mgr.startAutoRefresh(60, 'http://myserver.com/index.php');
 * mgr.onUpdate.subscribe(myFcnNeedsToKnow);
 * <br>
 * </code></pre>
 * @requires YAHOO.ext.Element
 * @requires YAHOO.util.Dom
 * @requires YAHOO.util.Event
 * @requires YAHOO.util.CustomEvent 
 * @requires YAHOO.util.Connect
 * @constructor
 * Create new UpdateManager.
 * @param {String/HTMLElement/YAHOO.ext.Element} el The element to update 
 * @param {<i>Boolean</i>} forceNew (optional) By default the constructor checks to see if the passed element already has an UpdateManager and if it does it returns the same instance. This will skip that check (useful for extending this class).
 */
YAHOO.ext.UpdateManager = function(el, forceNew){
    el = YAHOO.ext.Element.get(el);
    if(!forceNew && el.updateManager){
        return el.updateManager;
    }
    /**
     * The Element object
     * @type YAHOO.ext.Element
     */
    this.el = el;
    /**
     * Cached url to use for refreshes. Overwritten every time update() is called unless 'discardUrl' param is set to true.
     * @type String
     */
    this.defaultUrl = null;
    /**
     * fired before update is made, return false from your handler and the update is cancelled. 
     * Uses fireDirect with signature: (oElement, url, params)
     * @type YAHOO.util.CustomEvent
     */
    this.beforeUpdate = new YAHOO.util.CustomEvent('UpdateManager.beforeUpdate');
    /**
     * Fired after successful update is made. Uses fireDirect with signature: (oElement, oResponseObject)
     * @type YAHOO.util.CustomEvent
     */
    this.onUpdate = new YAHOO.util.CustomEvent('UpdateManager.onUpdate');
    /**
     * Fired on update failure. Uses fireDirect with signature: (oElement, oResponseObject)
     * @type YAHOO.util.CustomEvent
     */
    this.onFailure = new YAHOO.util.CustomEvent('UpdateManager.onFailure');
    
    this.events = {
        'beforeupdate': this.beforeUpdate,
        'update': this.onUpdate,
        'failure': this.onFailure 
    };
    
    /**
     * Blank page URL to use with SSL file uploads (Defaults to YAHOO.ext.UpdateManager.defaults.sslBlankUrl or 'about:blank'). 
     * @type String
     */
    this.sslBlankUrl = YAHOO.ext.UpdateManager.defaults.sslBlankUrl;
    /**
     * Whether to append unique parameter on get request to disable caching (Defaults to YAHOO.ext.UpdateManager.defaults.disableCaching or false). 
     * @type Boolean
     */
    this.disableCaching = YAHOO.ext.UpdateManager.defaults.disableCaching;
    /**
     * Text for loading indicator (Defaults to YAHOO.ext.UpdateManager.defaults.indicatorText or '&lt;div class="loading-indicator"&gt;Loading...&lt;/div&gt;'). 
     * @type String
     */
    this.indicatorText = YAHOO.ext.UpdateManager.defaults.indicatorText;
    /**
     * Whether to show indicatorText when loading (Defaults to YAHOO.ext.UpdateManager.defaults.showLoadIndicator or true). 
     * @type String
     */
    this.showLoadIndicator = YAHOO.ext.UpdateManager.defaults.showLoadIndicator;
    /**
     * Timeout for requests or form posts in seconds (Defaults to YAHOO.ext.UpdateManager.defaults.timeout or 30 seconds). 
     * @type Number
     */
    this.timeout = YAHOO.ext.UpdateManager.defaults.timeout;
    
    /**
     * True to process scripts in the output (Defaults to YAHOO.ext.UpdateManager.defaults.loadScripts (false)). 
     * @type Number
     */
    this.loadScripts = YAHOO.ext.UpdateManager.defaults.loadScripts;
    
    /**
     * YAHOO.util.Connect transaction object of current executing transaction
     */
    this.transaction = null;
    
    /**
     * @private
     */
    this.autoRefreshProcId = null;
    /**
     * Delegate for refresh() prebound to 'this', use myUpdater.refreshDelegate.createCallback(arg1, arg2) to bind arguments
     * @type Function
     */
    this.refreshDelegate = this.refresh.createDelegate(this);
    /**
     * Delegate for update() prebound to 'this', use myUpdater.updateDelegate.createCallback(arg1, arg2) to bind arguments
     * @type Function
     */
    this.updateDelegate = this.update.createDelegate(this);
    /**
     * Delegate for formUpdate() prebound to 'this', use myUpdater.formUpdateDelegate.createCallback(arg1, arg2) to bind arguments
     * @type Function
     */
    this.formUpdateDelegate = this.formUpdate.createDelegate(this);
    /**
     * @private
     */
    this.successDelegate = this.processSuccess.createDelegate(this);
    /**
     * @private
     */
     this.failureDelegate = this.processFailure.createDelegate(this);
     
     /**
      * The renderer for this UpdateManager. Defaults to {@link YAHOO.ext.UpdateManager.BasicRenderer}. 
      */
      this.renderer = new YAHOO.ext.UpdateManager.BasicRenderer();
};

YAHOO.ext.UpdateManager.prototype = {
    fireEvent : YAHOO.ext.util.Observable.prototype.fireEvent,
    on : YAHOO.ext.util.Observable.prototype.on,
    addListener : YAHOO.ext.util.Observable.prototype.addListener,
    delayedListener : YAHOO.ext.util.Observable.prototype.delayedListener,
    removeListener : YAHOO.ext.util.Observable.prototype.removeListener,
    /**
     * Get the Element this UpdateManager is bound to
     * @return {YAHOO.ext.Element} The element
     */
    getEl : function(){
        return this.el;
    },
    
    /**
     * Performs an async request, updating this element with the response. If params are specified it uses POST, otherwise it uses GET.
     * @param {String/Function} url The url for this request or a function to call to get the url
     * @param {<i>String/Object</i>} params (optional) The parameters to pass as either a url encoded string "param1=1&amp;param2=2" or as an object {param1: 1, param2: 2}
     * @param {<i>Function</i>} callback (optional) Callback when transaction is complete - called with signature (oElement, bSuccess)
     * @param {<i>Boolean</i>} discardUrl (optional) By default when you execute an update the defaultUrl is changed to the last used url. If true, it will not store the url.
     */
    update : function(url, params, callback, discardUrl){
        if(this.beforeUpdate.fireDirect(this.el, url, params) !== false){
            this.showLoading();
            if(!discardUrl){
                this.defaultUrl = url;
            }
            if(typeof url == 'function'){
                url = url();
            }
            if(params && typeof params != 'string'){ // must be object
                var buf = [];
                for(var key in params){
                    if(typeof params[key] != 'function'){
                        buf.push(encodeURIComponent(key), '=', encodeURIComponent(params[key]), '&');
                    }
                }
                delete buf[buf.length-1];
                params = buf.join('');
            }
            var callback = {
                success: this.successDelegate,
                failure: this.failureDelegate,
                timeout: (this.timeout*1000),
                argument: {'url': url, 'form': null, 'callback': callback, 'params': params}
            };
            var method = params ? 'POST' : 'GET';
            if(method == 'GET'){
                url = this.prepareUrl(url);
            }
            this.transaction = YAHOO.util.Connect.asyncRequest(method, url, callback, params);
        }
    },
    
    /**
     * Performs an async form post, updating this element with the response. If the form has the attribute enctype="multipart/form-data", it assumes it's a file upload.
     * Uses this.sslBlankUrl for SSL file uploads to prevent IE security warning. See YUI docs for more info. 
     * @param {String/HTMLElement} form The form Id or form element
     * @param {<i>String</i>} url (optional) The url to pass the form to. If omitted the action attribute on the form will be used.
     * @param {<i>Boolean</i>} reset (optional) Whether to try to reset the form after the update
     * @param {<i>Function</i>} callback (optional) Callback when transaction is complete - called with signature (oElement, bSuccess)
     */
    formUpdate : function(form, url, reset, callback){
        if(this.beforeUpdate.fireDirect(this.el, form, url) !== false){
            this.showLoading();
            formEl = YAHOO.util.Dom.get(form);
            if(typeof url == 'function'){
                url = url();
            }
            url = url || formEl.action;
            var callback = {
                success: this.successDelegate,
                failure: this.failureDelegate,
                timeout: (this.timeout*1000),
                argument: {'url': url, 'form': form, 'callback': callback, 'reset': reset}
            };
            var isUpload = false;
            var enctype = formEl.getAttribute('enctype');
            if(enctype && enctype.toLowerCase() == 'multipart/form-data'){
                isUpload = true;
            }
            YAHOO.util.Connect.setForm(form, isUpload, this.sslBlankUrl);
            this.transaction = YAHOO.util.Connect.asyncRequest('POST', url, callback);
        }
    },
    
    /**
     * Refresh the element with the last used url or defaultUrl. If there is no url, it returns immediately
     * @param {Function} callback (optional) Callback when transaction is complete - called with signature (oElement, bSuccess)
     */
    refresh : function(callback){
        if(this.defaultUrl == null){
            return;
        }
        this.update(this.defaultUrl, null, callback, true);
    },
    
    /**
     * Set this element to auto refresh.
     * @param {Number} interval How often to update (in seconds).
     * @param {<i>String/Function</i>} url (optional) The url for this request or a function to call to get the url (Defaults to the last used url)
     * @param {<i>String/Object</i>} params (optional) The parameters to pass as either a url encoded string "&param1=1&param2=2" or as an object {param1: 1, param2: 2}
     * @param {<i>Function</i>} callback (optional) Callback when transaction is complete - called with signature (oElement, bSuccess)
     * @param {<i>Boolean</i>} refreshNow (optional) Whether to execute the refresh now, or wait the interval
     */
    startAutoRefresh : function(interval, url, params, callback, refreshNow){
        if(refreshNow){
            this.update(url || this.defaultUrl, params, callback, true);
        }
        if(this.autoRefreshProcId){
            clearInterval(this.autoRefreshProcId);
        }
        this.autoRefreshProcId = setInterval(this.update.createDelegate(this, [url || this.defaultUrl, params, callback, true]), interval*1000);
    },
    
    /**
     * Stop auto refresh on this element.
     */
     stopAutoRefresh : function(){
        if(this.autoRefreshProcId){
            clearInterval(this.autoRefreshProcId);
        }
    },
    
    /**
     * Called to update the element to "Loading" state. Override to perform custom action.
     */
    showLoading : function(){
        if(this.showLoadIndicator){
            this.el.update(this.indicatorText);
        }
    },
    
    /**
     * Adds unique parameter to query string if disableCaching = true
     * @private
     */
    prepareUrl : function(url){
        if(this.disableCaching){
            var append = '_dc=' + (new Date().getTime());
            if(url.indexOf('?') !== -1){
                url += '&' + append;
            }else{
                url += '?' + append;
            }
        }
        return url;
    },
    
    /**
     * @private
     */
    processSuccess : function(response){
        this.transaction = null;
        this.renderer.render(this.el, response, this);
        if(response.argument.form && response.argument.reset){
            try{ // put in try/catch since some older FF releases had problems with this
                response.argument.form.reset();
            }catch(e){}
        }
        this.onUpdate.fireDirect(this.el, response);
        if(typeof response.argument.callback == 'function'){
            response.argument.callback(this.el, true);
        }
    },
    
    /**
     * @private
     */
    processFailure : function(response){
        this.transaction = null;
        this.onFailure.fireDirect(this.el, response);
        if(typeof response.argument.callback == 'function'){
            response.argument.callback(this.el, false);
        }
    },
    
    /**
     * Set the content renderer for this UpdateManager. See {@link YAHOO.ext.UpdateManager.BasicRenderer#render} for more details.
     * @param {Object} renderer The object implementing the render() method
     */
    setRenderer : function(renderer){
        this.renderer = renderer;
    },
    
    getRenderer : function(){
       return this.renderer;  
    },
    
    /**
     * Set the defaultUrl used for updates
     * @param {String/Function} defaultUrl The url or a function to call to get the url
     */
    setDefaultUrl : function(defaultUrl){
        this.defaultUrl = defaultUrl;
    },
    
    /**
     * Aborts the executing transaction
     */
    abort : function(){
        if(this.transaction){
            YAHOO.util.Connect.abort(this.transaction);
        }
    },
    
    /**
     * Returns true if an update is in progress
     */
    isUpdating : function(){
        if(this.transaction){
            return YAHOO.util.Connect.isCallInProgress(this.transaction);
        }
        return false;
    }
};

/**
 * Static convenience method, Usage:
 * YAHOO.ext.UpdateManager.update('my-div', 'stuff.php');
 * @param {String/HTMLElement/YAHOO.ext.Element} el The element to update
 * @param {String} url The url
 * @param {<i>String/Object</i>} params (optional) Url encoded param string or an object of name/value pairs
 * @param {<i>Object</i>} options (optional) A config object with any of the UpdateManager properties you want to set - for example: {disableCaching:true, indicatorText: 'Loading data...'}
 */
YAHOO.ext.UpdateManager.update = function(el, url, params, options){
    var um = getEl(el, true).getUpdateManager();
    YAHOO.ext.util.Config.apply(um, options);
    um.update(url, params, options.callback);
}

/**
 * @class
 * Default Content renderer. Updates the elements innerHTML with the responseText.
 */ 
YAHOO.ext.UpdateManager.BasicRenderer = function(){};

YAHOO.ext.UpdateManager.BasicRenderer.prototype = {
    /**
     * This is called when the transaction is completed and it's time to update the element - The BasicRenderer 
     * updates the elements innerHTML with the responseText - To perform a custom render (i.e. XML or JSON processing), 
     * create an object with a "render(el, response)" method and pass it to setRenderer on the UpdateManager.
     * @param {YAHOO.ext.Element} el The element being rendered
     * @param {Object} response The YUI Connect response object
     */
     render : function(el, response, updateManager){
        el.update(response.responseText, updateManager.loadScripts);
    }
};

/**
 * The defaults collection enables customizing the default behavior of UpdateManager
 */
YAHOO.ext.UpdateManager.defaults = {};
/**
 * Timeout for requests or form posts in seconds (Defaults 30 seconds). 
 * @type Number
 */
 YAHOO.ext.UpdateManager.defaults.timeout = 30;
 
 /**
 * True to process scripts by default (Defaults to false). 
 * @type Number
 */
 YAHOO.ext.UpdateManager.defaults.loadScripts = false;
 
/**
* Blank page URL to use with SSL file uploads (Defaults to 'about:blank'). 
* @type String
*/
YAHOO.ext.UpdateManager.defaults.sslBlankUrl = YAHOO.ext.SSL_SECURE_URL;
/**
 * Whether to append unique parameter on get request to disable caching (Defaults to false). 
 * @type Boolean
 */
YAHOO.ext.UpdateManager.defaults.disableCaching = false;
/**
 * Whether to show indicatorText when loading (Defaults to true). 
 * @type String
 */
YAHOO.ext.UpdateManager.defaults.showLoadIndicator = true;
/**
 * Text for loading indicator (Defaults to '&lt;div class="loading-indicator"&gt;Loading...&lt;/div&gt;'). 
 * @type String
 */
YAHOO.ext.UpdateManager.defaults.indicatorText = '<div class="loading-indicator">Loading...</div>';

/*
------------------------------------------------------------------
// File: \Date.js
------------------------------------------------------------------
*/
/*
 * All the Date functions below are the excellent work of Baron Schwartz
 * They generate precompiled functions from date formats instead of parsing and processing
 * the format everytime you do something with a date.
 */
/** @ignore */
Date.parseFunctions = {count:0};
/** @ignore */
Date.parseRegexes = [];
/** @ignore */
Date.formatFunctions = {count:0};

/**
 * Formats a date given to the supplied format - the format syntax is the same as <a href="http://www.php.net/date">PHP's date() function</a>.
 */
Date.prototype.dateFormat = function(format) {
    if (Date.formatFunctions[format] == null) {
        Date.createNewFormat(format);
    }
    var func = Date.formatFunctions[format];
    return this[func]();
};

/**
 * Same as {@link #dateFormat}
 */
Date.prototype.format = Date.prototype.dateFormat;

/** @ignore */
Date.createNewFormat = function(format) {
    var funcName = "format" + Date.formatFunctions.count++;
    Date.formatFunctions[format] = funcName;
    var code = "Date.prototype." + funcName + " = function(){return ";
    var special = false;
    var ch = '';
    for (var i = 0; i < format.length; ++i) {
        ch = format.charAt(i);
        if (!special && ch == "\\") {
            special = true;
        }
        else if (special) {
            special = false;
            code += "'" + String.escape(ch) + "' + ";
        }
        else {
            code += Date.getFormatCode(ch);
        }
    }
    eval(code.substring(0, code.length - 3) + ";}");
};

/** @ignore */
Date.getFormatCode = function(character) {
    switch (character) {
    case "d":
        return "String.leftPad(this.getDate(), 2, '0') + ";
    case "D":
        return "Date.dayNames[this.getDay()].substring(0, 3) + ";
    case "j":
        return "this.getDate() + ";
    case "l":
        return "Date.dayNames[this.getDay()] + ";
    case "S":
        return "this.getSuffix() + ";
    case "w":
        return "this.getDay() + ";
    case "z":
        return "this.getDayOfYear() + ";
    case "W":
        return "this.getWeekOfYear() + ";
    case "F":
        return "Date.monthNames[this.getMonth()] + ";
    case "m":
        return "String.leftPad(this.getMonth() + 1, 2, '0') + ";
    case "M":
        return "Date.monthNames[this.getMonth()].substring(0, 3) + ";
    case "n":
        return "(this.getMonth() + 1) + ";
    case "t":
        return "this.getDaysInMonth() + ";
    case "L":
        return "(this.isLeapYear() ? 1 : 0) + ";
    case "Y":
        return "this.getFullYear() + ";
    case "y":
        return "('' + this.getFullYear()).substring(2, 4) + ";
    case "a":
        return "(this.getHours() < 12 ? 'am' : 'pm') + ";
    case "A":
        return "(this.getHours() < 12 ? 'AM' : 'PM') + ";
    case "g":
        return "((this.getHours() %12) ? this.getHours() % 12 : 12) + ";
    case "G":
        return "this.getHours() + ";
    case "h":
        return "String.leftPad((this.getHours() %12) ? this.getHours() % 12 : 12, 2, '0') + ";
    case "H":
        return "String.leftPad(this.getHours(), 2, '0') + ";
    case "i":
        return "String.leftPad(this.getMinutes(), 2, '0') + ";
    case "s":
        return "String.leftPad(this.getSeconds(), 2, '0') + ";
    case "O":
        return "this.getGMTOffset() + ";
    case "T":
        return "this.getTimezone() + ";
    case "Z":
        return "(this.getTimezoneOffset() * -60) + ";
    default:
        return "'" + String.escape(character) + "' + ";
    };
};

/**
 * Parses a date given the supplied format - the format syntax is the same as <a href="http://www.php.net/date">PHP's date() function</a>.
 */
Date.parseDate = function(input, format) {
    if (Date.parseFunctions[format] == null) {
        Date.createParser(format);
    }
    var func = Date.parseFunctions[format];
    return Date[func](input);
};

/** @ignore */
Date.createParser = function(format) {
    var funcName = "parse" + Date.parseFunctions.count++;
    var regexNum = Date.parseRegexes.length;
    var currentGroup = 1;
    Date.parseFunctions[format] = funcName;

    var code = "Date." + funcName + " = function(input){\n"
        + "var y = -1, m = -1, d = -1, h = -1, i = -1, s = -1;\n"
        + "var d = new Date();\n"
        + "y = d.getFullYear();\n"
        + "m = d.getMonth();\n"
        + "d = d.getDate();\n"
        + "var results = input.match(Date.parseRegexes[" + regexNum + "]);\n"
        + "if (results && results.length > 0) {"
    var regex = "";

    var special = false;
    var ch = '';
    for (var i = 0; i < format.length; ++i) {
        ch = format.charAt(i);
        if (!special && ch == "\\") {
            special = true;
        }
        else if (special) {
            special = false;
            regex += String.escape(ch);
        }
        else {
            obj = Date.formatCodeToRegex(ch, currentGroup);
            currentGroup += obj.g;
            regex += obj.s;
            if (obj.g && obj.c) {
                code += obj.c;
            }
        }
    }

    code += "if (y > 0 && m >= 0 && d > 0 && h >= 0 && i >= 0 && s >= 0)\n"
        + "{return new Date(y, m, d, h, i, s);}\n"
        + "else if (y > 0 && m >= 0 && d > 0 && h >= 0 && i >= 0)\n"
        + "{return new Date(y, m, d, h, i);}\n"
        + "else if (y > 0 && m >= 0 && d > 0 && h >= 0)\n"
        + "{return new Date(y, m, d, h);}\n"
        + "else if (y > 0 && m >= 0 && d > 0)\n"
        + "{return new Date(y, m, d);}\n"
        + "else if (y > 0 && m >= 0)\n"
        + "{return new Date(y, m);}\n"
        + "else if (y > 0)\n"
        + "{return new Date(y);}\n"
        + "}return null;}";

    Date.parseRegexes[regexNum] = new RegExp("^" + regex + "$");
    eval(code);
};

/** @ignore */
Date.formatCodeToRegex = function(character, currentGroup) {
    switch (character) {
    case "D":
        return {g:0,
        c:null,
        s:"(?:Sun|Mon|Tue|Wed|Thu|Fri|Sat)"};
    case "j":
    case "d":
        return {g:1,
            c:"d = parseInt(results[" + currentGroup + "], 10);\n",
            s:"(\\d{1,2})"};
    case "l":
        return {g:0,
            c:null,
            s:"(?:" + Date.dayNames.join("|") + ")"};
    case "S":
        return {g:0,
            c:null,
            s:"(?:st|nd|rd|th)"};
    case "w":
        return {g:0,
            c:null,
            s:"\\d"};
    case "z":
        return {g:0,
            c:null,
            s:"(?:\\d{1,3})"};
    case "W":
        return {g:0,
            c:null,
            s:"(?:\\d{2})"};
    case "F":
        return {g:1,
            c:"m = parseInt(Date.monthNumbers[results[" + currentGroup + "].substring(0, 3)], 10);\n",
            s:"(" + Date.monthNames.join("|") + ")"};
    case "M":
        return {g:1,
            c:"m = parseInt(Date.monthNumbers[results[" + currentGroup + "]], 10);\n",
            s:"(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)"};
    case "n":
    case "m":
        return {g:1,
            c:"m = parseInt(results[" + currentGroup + "], 10) - 1;\n",
            s:"(\\d{1,2})"};
    case "t":
        return {g:0,
            c:null,
            s:"\\d{1,2}"};
    case "L":
        return {g:0,
            c:null,
            s:"(?:1|0)"};
    case "Y":
        return {g:1,
            c:"y = parseInt(results[" + currentGroup + "], 10);\n",
            s:"(\\d{4})"};
    case "y":
        return {g:1,
            c:"var ty = parseInt(results[" + currentGroup + "], 10);\n"
                + "y = ty > Date.y2kYear ? 1900 + ty : 2000 + ty;\n",
            s:"(\\d{1,2})"};
    case "a":
        return {g:1,
            c:"if (results[" + currentGroup + "] == 'am') {\n"
                + "if (h == 12) { h = 0; }\n"
                + "} else { if (h < 12) { h += 12; }}",
            s:"(am|pm)"};
    case "A":
        return {g:1,
            c:"if (results[" + currentGroup + "] == 'AM') {\n"
                + "if (h == 12) { h = 0; }\n"
                + "} else { if (h < 12) { h += 12; }}",
            s:"(AM|PM)"};
    case "g":
    case "G":
    case "h":
    case "H":
        return {g:1,
            c:"h = parseInt(results[" + currentGroup + "], 10);\n",
            s:"(\\d{1,2})"};
    case "i":
        return {g:1,
            c:"i = parseInt(results[" + currentGroup + "], 10);\n",
            s:"(\\d{2})"};
    case "s":
        return {g:1,
            c:"s = parseInt(results[" + currentGroup + "], 10);\n",
            s:"(\\d{2})"};
    case "O":
        return {g:0,
            c:null,
            s:"[+-]\\d{4}"};
    case "T":
        return {g:0,
            c:null,
            s:"[A-Z]{3}"};
    case "Z":
        return {g:0,
            c:null,
            s:"[+-]\\d{1,5}"};
    default:
        return {g:0,
            c:null,
            s:String.escape(character)};
    }
};

Date.prototype.getTimezone = function() {
    return this.toString().replace(
        /^.*? ([A-Z]{3}) [0-9]{4}.*$/, "$1").replace(
        /^.*?\(([A-Z])[a-z]+ ([A-Z])[a-z]+ ([A-Z])[a-z]+\)$/, "$1$2$3");
};

Date.prototype.getGMTOffset = function() {
    return (this.getTimezoneOffset() > 0 ? "-" : "+")
        + String.leftPad(Math.floor(this.getTimezoneOffset() / 60), 2, "0")
        + String.leftPad(this.getTimezoneOffset() % 60, 2, "0");
};

Date.prototype.getDayOfYear = function() {
    var num = 0;
    Date.daysInMonth[1] = this.isLeapYear() ? 29 : 28;
    for (var i = 0; i < this.getMonth(); ++i) {
        num += Date.daysInMonth[i];
    }
    return num + this.getDate() - 1;
};

Date.prototype.getWeekOfYear = function() {
    // Skip to Thursday of this week
    var now = this.getDayOfYear() + (4 - this.getDay());
    // Find the first Thursday of the year
    var jan1 = new Date(this.getFullYear(), 0, 1);
    var then = (7 - jan1.getDay() + 4);
    document.write(then);
    return String.leftPad(((now - then) / 7) + 1, 2, "0");
};

Date.prototype.isLeapYear = function() {
    var year = this.getFullYear();
    return ((year & 3) == 0 && (year % 100 || (year % 400 == 0 && year)));
};

Date.prototype.getFirstDayOfMonth = function() {
    var day = (this.getDay() - (this.getDate() - 1)) % 7;
    return (day < 0) ? (day + 7) : day;
};

Date.prototype.getLastDayOfMonth = function() {
    var day = (this.getDay() + (Date.daysInMonth[this.getMonth()] - this.getDate())) % 7;
    return (day < 0) ? (day + 7) : day;
};

Date.prototype.getDaysInMonth = function() {
    Date.daysInMonth[1] = this.isLeapYear() ? 29 : 28;
    return Date.daysInMonth[this.getMonth()];
};

/** @ignore */
Date.prototype.getSuffix = function() {
    switch (this.getDate()) {
        case 1:
        case 21:
        case 31:
            return "st";
        case 2:
        case 22:
            return "nd";
        case 3:
        case 23:
            return "rd";
        default:
            return "th";
    }
};

/** @ignore */
Date.daysInMonth = [31,28,31,30,31,30,31,31,30,31,30,31];

/**
 * Override these values for international dates, for example...
 * Date.monthNames = ['JanInYourLang', 'FebInYourLang', ...];
 */
Date.monthNames =
   ["January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"];
    
/**
 * Override these values for international dates, for example...
 * Date.dayNames = ['SundayInYourLang', 'MondayInYourLang', ...];
 */
Date.dayNames =
   ["Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday"];

/** @ignore */
Date.y2kYear = 50;

/** @ignore */
Date.monthNumbers = {
    Jan:0,
    Feb:1,
    Mar:2,
    Apr:3,
    May:4,
    Jun:5,
    Jul:6,
    Aug:7,
    Sep:8,
    Oct:9,
    Nov:10,
    Dec:11};

/*
------------------------------------------------------------------
// File: widgets\TabPanel.js
------------------------------------------------------------------
*/
/*
	tabpanel.js, version .1
	Copyright(c) 2006, Jack Slocum.
	Code licensed under the BSD License
*/

/**
 * @class Creates a lightweight TabPanel component using Yahoo! UI.
 * <br><br>
 * Usage:
 * <pre><code>
    <font color="#008000">// basic tabs 1, built from existing content</font>
    var tabs = new YAHOO.ext.TabPanel('tabs1');
    tabs.addTab('script', "View Script");
    tabs.addTab('markup', "View Markup");
    tabs.activate('script');
    
    <font color="#008000">// more advanced tabs, built from javascript</font>
    var jtabs = new YAHOO.ext.TabPanel('jtabs');
    jtabs.addTab('jtabs-1', "Normal Tab", "My content was added during construction.");
    
    <font color="#008000">// set up the UpdateManager</font>
    var tab2 = jtabs.addTab('jtabs-2', "Ajax Tab 1");
    var updater = tab2.getUpdateManager();
    updater.setDefaultUrl('ajax1.htm');
    tab2.onActivate.subscribe(updater.refresh, updater, true);
    
    <font color="#008000">// Use setUrl for Ajax loading</font>
    var tab3 = jtabs.addTab('jtabs-3', "Ajax Tab 2");
    tab3.setUrl('ajax2.htm', null, true);
    
    <font color="#008000">// Disabled tab</font>
    var tab4 = jtabs.addTab('tabs1-5', "Disabled Tab", "Can't see me cause I'm disabled");
    tab4.disable();
    
    jtabs.activate('jtabs-1');
}
 * </code></pre>
 * @requires YAHOO.ext.Element
 * @requires YAHOO.ext.UpdateManager
 * @requires YAHOO.util.Dom
 * @requires YAHOO.util.Event
 * @requires YAHOO.util.CustomEvent 
 * @requires YAHOO.util.Connect (optional)
 * @constructor
 * Create new TabPanel.
 * @param {String/HTMLElement/Element} container The id, DOM element or YAHOO.ext.Element container where this TabPanel is to be rendered. 
 */
YAHOO.ext.TabPanel = function(container, onBottom){
    /**
    * The container element for this TabPanel.
    * @type YAHOO.ext.Element
    */
    this.el = getEl(container, true);
    
    if(onBottom){
        this.bodyEl = getEl(this.createBody(this.el.dom));
        this.el.addClass('ytabs-bottom');
    }
    /** @private */
    this.stripWrap = getEl(this.createStrip(this.el.dom), true);
    /** @private */
    this.stripEl = getEl(this.createStripList(this.stripWrap.dom), true);
    this.stripBody = getEl(this.stripWrap.dom.firstChild.firstChild, true);
    /** The body element that contains TabPaneItem bodies. 
     * @type YAHOO.ext.Element
     */
    if(!onBottom){
      this.bodyEl = getEl(this.createBody(this.el.dom));
      this.el.addClass('ytabs-top');
    }
    /** @private */
    this.items = [];
    
    // add indexOf to array if it isn't present
    if(!this.items.indexOf){
        this.items.indexOf = function(o){
            for(var i = 0, len = this.length; i < len; i++){
                if(this[i] == o) return i;
            }
            return -1;
        }
    }
    /** @private */
    this.active = null;
    /**
     * Fires when the active TabPanelItem changes. Uses fireDirect with signature: (TabPanel this, TabPanelItem activedTab).
     * @type CustomEvent
     */
    this.onTabChange = new YAHOO.util.CustomEvent('TabItem.onTabChange');
    /** @private */
    this.activateDelegate = this.activate.createDelegate(this);
    
    this.events = {
        'tabchange': this.onTabChange  
    };
    
    this.currentTabWidth = 0;
    this.minTabWidth = 40;
    this.maxTabWidth = 250;
    this.preferredTabWidth = 175;
    this.resizeTabs = false;
    this.monitorResize = true;
    YAHOO.ext.EventManager.onWindowResize(this.onResize, this, true);
    this.cpad = this.el.getPadding('lr');
    this.hiddenCount = 0;
}

YAHOO.ext.TabPanel.prototype = {
    fireEvent : YAHOO.ext.util.Observable.prototype.fireEvent,
    on : YAHOO.ext.util.Observable.prototype.on,
    addListener : YAHOO.ext.util.Observable.prototype.addListener,
    delayedListener : YAHOO.ext.util.Observable.prototype.delayedListener,
    removeListener : YAHOO.ext.util.Observable.prototype.removeListener,
    /**
     * Creates a new TabPanelItem by looking for an existing element with the provided id - if it's not found it creates one.
     * @param {String} id The id of the div to use or create
     * @param {String} text The text for the tab
     * @param {<i>String</i>} content (optional) Content to put in the TabPanelItem body
     * @return {YAHOO.ext.TabPanelItem} The created TabPanelItem
     */
    addTab : function(id, text, content, closable){
        var item = new YAHOO.ext.TabPanelItem(this, id, text, closable);
        this.addTabItem(item);
        if(content){
            item.setContent(content);
        }
        return item;
    },
    
    /**
     * Returns the TabPanelItem with the specified id
     * @param {String} id The id of the TabPanelItem to fetch.
     * @return {YAHOO.ext.TabPanelItem}
     */
    getTab : function(id){
        return this.items[id];
    },
    
    hideTab : function(id){
        var t = this.items[id];
        if(!t.isHidden()){
           t.setHidden(true);
           this.hiddenCount++;
           this.autoSizeTabs();
        }
    },
    
    unhideTab : function(id){
        var t = this.items[id];
        if(t.isHidden()){
           t.setHidden(false);
           this.hiddenCount--;
           this.autoSizeTabs();
        }
    },
    
    /**
     * Add an existing TabPanelItem.
     * @param {YAHOO.ext.TabPanelItem} item The TabPanelItem to add
     */
    addTabItem : function(item){
        this.items[item.id] = item;
        this.items[this.items.length] = item;
        if(this.resizeTabs){
           item.setWidth(this.currentTabWidth || this.preferredTabWidth)
           this.autoSizeTabs();
        }else{
            item.autoSize();
        }
    },
        
    /**
     * Remove a TabPanelItem.
     * @param {String} id The id of the TabPanelItem to remove.
     */
    removeTab : function(id){
        var items = this.items;
        var tab = items[id];
        if(!tab) return;
        var index = items.indexOf(tab);
        if(this.active == tab && items.length > 1){
            var newTab = this.getNextAvailable(index);
            if(newTab)newTab.activate();
        }
        this.stripEl.dom.removeChild(tab.pnode.dom);
        this.bodyEl.dom.removeChild(tab.bodyEl.dom);
        items.splice(index, 1);
        delete items[tab.id];
        this.autoSizeTabs();
    },
    
    getNextAvailable : function(start){
        var items = this.items;
        var index = start;
        // look for a next tab that will slide over to
        // replace the one being removed
        while(index < items.length){
            var item = items[++index];
            if(item && !item.isHidden()){
                return item;
            }
        }
        // if one isn't found select the previous tab (on the left)
        var index = start;
        while(index >= 0){
            var item = items[--index];
            if(item && !item.isHidden()){
                return item;
            }
        }
        return null;
    },
    
    /**
     * Disable a TabPanelItem. <b>It cannot be the active tab, if it is this call is ignored.</b>. 
     * @param {String} id The id of the TabPanelItem to disable.
     */
    disableTab : function(id){
        var tab = this.items[id];
        if(tab && this.active != tab){
            tab.disable();
        }
    },
    
    /**
     * Enable a TabPanelItem that is disabled.
     * @param {String} id The id of the TabPanelItem to enable.
     */
    enableTab : function(id){
        var tab = this.items[id];
        tab.enable();
    },
    
    /**
     * Activate a TabPanelItem. The currently active will be deactivated. 
     * @param {String} id The id of the TabPanelItem to activate.
     */
    activate : function(id){
        var tab = this.items[id];
        if(tab == this.active){
            return;
        } 
        if(!tab.disabled){
            if(this.active){
                this.active.hide();
            }
            this.active = this.items[id];
            this.active.show();
            this.onTabChange.fireDirect(this, this.active);
        }
    },
    
    /**
     * Get the active TabPanelItem
     * @return {YAHOO.ext.TabPanelItem} The active TabPanelItem or null if none are active.
     */
    getActiveTab : function(){
        return this.active;
    },
    
    /**
     * Updates the tab body element to fit the height of the container element
     * for scrolling
     * @param {Number} targetHeight (optional) Override the starting height from the elements height
     */
    syncHeight : function(targetHeight){
        var height = (targetHeight || this.el.getHeight())-this.el.getBorderWidth('tb')-this.el.getPadding('tb');
        var bm = this.bodyEl.getMargins();
        var newHeight = height-(this.stripWrap.getHeight())-(bm.top+bm.bottom);
        this.bodyEl.setHeight(newHeight);
        return newHeight; 
    },
    
    onResize : function(){
        if(this.monitorResize){
            this.autoSizeTabs();
        }
    },

    beginUpdate : function(){
        this.updating = true;    
    },
    
    endUpdate : function(){
        this.updating = false;
        this.autoSizeTabs();  
    },
    
    autoSizeTabs : function(){
        var count = this.items.length;
        var vcount = count - this.hiddenCount;
        if(!this.resizeTabs || count < 1 || vcount < 1 || this.updating) return;
        var w = Math.max(this.el.getWidth() - this.cpad, 10);
        var availWidth = Math.floor(w / vcount);
        var b = this.stripBody;
        if(b.getWidth() > w){
            var tabs = this.items;
            this.setTabWidth(Math.max(availWidth, this.minTabWidth));
            if(availWidth < this.minTabWidth){
                /*if(!this.sleft){    // incomplete scrolling code
                    this.createScrollButtons();
                }
                this.showScroll();
                this.stripClip.setWidth(w - (this.sleft.getWidth()+this.sright.getWidth()));*/
            }
        }else{
            if(this.currentTabWidth < this.preferredTabWidth){
                this.setTabWidth(Math.min(availWidth, this.preferredTabWidth));
            }
        }
    },
    
    setTabWidth : function(width){
        this.currentTabWidth = width;
        for(var i = 0, len = this.items.length; i < len; i++) {
        	if(!this.items[i].isHidden())this.items[i].setWidth(width);
        }
    }
};

YAHOO.ext.TabPanelItem = function(tabPanel, id, text, closable){
    /**
     * The TabPanel this TabPanelItem belongs to
     * @type YAHOO.ext.TabPanel
     */
    this.tabPanel = tabPanel;
    /**
     * The id for this TabPanelItem
     * @type String
     */
    this.id = id;
    /** @private */
    this.disabled = false;
    /** @private */
    this.text = text;
    /** @private */
    this.loaded = false;
    this.closable = closable;
    
    /** 
     * The body element for this TabPanelItem
     * @type YAHOO.ext.Element
     */
    this.bodyEl = getEl(tabPanel.createItemBody(tabPanel.bodyEl.dom, id));
    this.bodyEl.originalDisplay = 'block';
    this.bodyEl.setStyle('display', 'none');
    this.bodyEl.enableDisplayMode();
    
    var els = tabPanel.createStripElements(tabPanel.stripEl.dom, text, closable);
    /** @private */
    this.el = getEl(els.el, true);
    this.inner = getEl(els.inner, true);
    this.textEl = getEl(this.el.dom.firstChild.firstChild.firstChild, true);
    this.pnode = getEl(els.el.parentNode, true);
    this.el.mon('click', this.onTabClick, this, true);
    /** @private */
    if(closable){
        var c = getEl(els.close, true);
        c.dom.title = this.closeText;
        c.addClassOnOver('close-over');
        c.mon('click', this.closeClick, this, true);
     }
    
    /**
     * Fires when this TabPanelItem is activated. Uses fireDirect with signature: (TabPanel tabPanel, TabPanelItem this).
     * @type CustomEvent
     */
    this.onActivate = new YAHOO.util.CustomEvent('TabItem.onActivate');
    /**
     * Fires when this TabPanelItem is deactivated. Uses fireDirect with signature: (TabPanel tabPanel, TabPanelItem this).
     * @type CustomEvent
     */
    this.onDeactivate = new YAHOO.util.CustomEvent('TabItem.onDeactivate');
    
    this.events = {
         'activate': this.onActivate,
         'beforeclose': new YAHOO.util.CustomEvent('beforeclose'),
         'close': new YAHOO.util.CustomEvent('close'),
         'deactivate' : this.onDeactivate  
    };
    this.hidden = false;
};

YAHOO.ext.TabPanelItem.prototype = {
    fireEvent : YAHOO.ext.util.Observable.prototype.fireEvent,
    on : YAHOO.ext.util.Observable.prototype.on,
    addListener : YAHOO.ext.util.Observable.prototype.addListener,
    delayedListener : YAHOO.ext.util.Observable.prototype.delayedListener,
    removeListener : YAHOO.ext.util.Observable.prototype.removeListener,
    /**
     * Show this TabPanelItem - this <b>does not</b> deactivate the currently active TabPanelItem.
     */
    show : function(){
        this.pnode.addClass('on');
        this.bodyEl.show();
        if(YAHOO.ext.util.Browser.isOpera){
            this.tabPanel.stripWrap.repaint();
        }
        this.onActivate.fireDirect(this.tabPanel, this); 
    },
    
    setTooltip : function(text){
        this.titleEl.dom.title = text;
    },
    
    onTabClick : function(e){
        e.preventDefault();
        this.tabPanel.activate(this.id);
    },
    
    getWidth : function(){
        return this.inner.getWidth();  
    },
    
    setWidth : function(width){
        var iwidth = width - this.pnode.getPadding("lr");
        this.inner.setWidth(iwidth);
        this.textEl.setWidth(iwidth-this.inner.getPadding('lr'));
        this.pnode.setWidth(width);
    },
    
    setHidden : function(hidden){
        this.hidden = hidden;
        this.pnode.setStyle('display', hidden ? 'none' : 'block');  
    },
    
    isHidden : function(){
        return this.hidden;  
    },
    
    getText : function(){
        return this.text;
    },
    
    autoSize : function(){
        this.el.beginMeasure();
        this.textEl.setWidth(1);
        this.setWidth(this.textEl.dom.scrollWidth+this.pnode.getPadding("lr")+this.inner.getPadding('lr'));
        this.el.endMeasure();
    },
    
    setText : function(text){
        this.text = text;
        this.textEl.update(text);
        this.textEl.dom.title = text;
        if(!this.tabPanel.resizeTabs){
            this.autoSize();
        }
    },
    /**
     * Activate this TabPanelItem - this <b>does</b> deactivate the currently active TabPanelItem.
     */
    activate : function(){
        this.tabPanel.activate(this.id);
    },
    
    /**
     * Hide this TabPanelItem - if you don't activate another TabPanelItem this could look odd.
     */
    hide : function(){
        this.pnode.removeClass('on');
        this.bodyEl.hide();
        this.onDeactivate.fireDirect(this.tabPanel, this); 
    },
    
    /**
     * Disable this TabPanelItem - this call is ignore if this is the active TabPanelItem.
     */
    disable : function(){
        if(this.tabPanel.active != this){
            this.disabled = true;
            this.pnode.addClass('disabled');
        }
    },
    
    /**
     * Enable this TabPanelItem if it was previously disabled.
     */
    enable : function(){
        this.disabled = false;
        this.pnode.removeClass('disabled');
    },
    
    /**
     * Set the content for this TabPanelItem.
     * @param {String} content The content
     */
    setContent : function(content){
        this.bodyEl.update(content);
    },
    
    /**
     * Get the {@link YAHOO.ext.UpdateManager} for the body of this TabPanelItem. Enables you to perform Ajax updates.
     * @return {YAHOO.ext.UpdateManager} The UpdateManager
     */
    getUpdateManager : function(){
        return this.bodyEl.getUpdateManager();
    },
    
    /**
     * Set a URL to be used to load the content for this TabPanelItem.
     * @param {String/Function} url The url to load the content from or a function to call to get the url
     * @param {<i>String/Object</i>} params (optional) The string params for the update call or an object of the params. See {@link YAHOO.ext.UpdateManager#update} for more details. (Defaults to null)
     * @param {<i>Boolean</i>} loadOnce (optional) Whether to only load the content once. If this is false it makes the Ajax call every time this TabPanelItem is activated. (Defaults to false)
     * @return {YAHOO.ext.UpdateManager} The UpdateManager
     */
    setUrl : function(url, params, loadOnce){
        if(this.refreshDelegate){
            this.onActivate.unsubscribe(this.refreshDelegate);
        }
        this.refreshDelegate = this._handleRefresh.createDelegate(this, [url, params, loadOnce]);
        this.onActivate.subscribe(this.refreshDelegate);
    },
    
    /** @private */
    _handleRefresh : function(url, params, loadOnce){
        if(!loadOnce || !this.loaded){
            var updater = this.bodyEl.getUpdateManager();
            updater.update(url, params, this._setLoaded.createDelegate(this));
        }
    },
    
    /** @private */
    _setLoaded : function(){
        this.loaded = true;
    },
    
    /** @private */
    closeClick : function(e){
        var cancel = {};
        this.fireEvent('beforeclose', this, cancel);
        if(!cancel.cancel){
            this.tabPanel.removeTab(this.id);
            this.fireEvent('close', this);
        }
    },
    
    closeText : 'Close this tab'
};

/** @private */
YAHOO.ext.TabPanel.prototype.createStrip = function(container){
    var strip = document.createElement('div');
    strip.className = 'ytab-wrap';
    container.appendChild(strip);
    return strip;
};
/** @private */
YAHOO.ext.TabPanel.prototype.createStripList = function(strip){
    // div wrapper for retard IE
    strip.innerHTML = '<div class="ytab-strip-wrap"><table class="ytab-strip" cellspacing="0" cellpadding="0" border="0"><tbody><tr></tr></tbody></table></div>';
    return strip.firstChild.firstChild.firstChild.firstChild;
};
/** @private */
YAHOO.ext.TabPanel.prototype.createBody = function(container){
    var body = document.createElement('div');
    YAHOO.util.Dom.generateId(body, 'tab-body');
    YAHOO.util.Dom.addClass(body, 'yui-ext-tabbody');
    container.appendChild(body);
    return body;
};
/** @private */
YAHOO.ext.TabPanel.prototype.createItemBody = function(bodyEl, id){
    var body = YAHOO.util.Dom.get(id);
    if(!body){
        body = document.createElement('div');
        body.id = id;
    }
    YAHOO.util.Dom.addClass(body, 'yui-ext-tabitembody');
    bodyEl.appendChild(body);
    return body;
};
/** @private */
YAHOO.ext.TabPanel.prototype.createStripElements = function(stripEl, text, closable){
    var td = document.createElement('td');
    stripEl.appendChild(td);
    if(closable){
        td.className = "ytab-closable";
        if(!this.closeTpl){
            this.closeTpl = new YAHOO.ext.Template(
               '<a href="#" class="ytab-right"><span class="ytab-left"><em class="ytab-inner">' +
               '<span unselectable="on" title="{text}" class="ytab-text">{text}</span>' +
               '<div unselectable="on" class="close-icon">&nbsp;</div></em></span></a>'
            );
        }
        var el = this.closeTpl.overwrite(td, {'text': text});
        var close = el.getElementsByTagName('div')[0];
        var inner = el.getElementsByTagName('em')[0];
        return {'el': el, 'close': close, 'inner': inner};
    } else {
        if(!this.tabTpl){
            this.tabTpl = new YAHOO.ext.Template(
               '<a href="#" class="ytab-right"><span class="ytab-left"><em class="ytab-inner">' +
               '<span unselectable="on" title="{text}" class="ytab-text">{text}</span></em></span></a>'
            );
        }
        var el = this.tabTpl.overwrite(td, {'text': text});
        var inner = el.getElementsByTagName('em')[0];
        return {'el': el, 'inner': inner};
    }
};

/*
------------------------------------------------------------------
// File: anim\Actor.js
------------------------------------------------------------------
*/

/**
 * @class
 * Provides support for syncing and chaining of Element Yahoo! UI based animation and some common effects. Actors support "self-play" without an Animator.<br><br>
 * <b>Note: Along with the animation methods defined below, this class inherits and captures all of the "set" or animation methods of {@link YAHOO.ext.Element}. "get" methods are not captured and execute immediately.</b>
 * <br><br>Usage:<br>
 * <pre><code>
 * var actor = new YAHOO.ext.Actor('myElementId');
 * actor.startCapture(true);
 * actor.moveTo(100, 100, true);
 * actor.squish();
 * actor.play();
 * <br>
 * // or to start capturing immediately, with no Animator (the null second param)
 * <br>
 * var actor = new YAHOO.ext.Actor('myElementId', null, true);
 * actor.moveTo(100, 100, true);
 * actor.squish();
 * actor.play();
 * </code></pre>
 * @extends YAHOO.ext.Element
 * @requires YAHOO.ext.Element
 * @requires YAHOO.util.Dom
 * @requires YAHOO.util.Event
 * @requires YAHOO.util.CustomEvent 
 * @requires YAHOO.util.Anim
 * @requires YAHOO.util.ColorAnim
 * @requires YAHOO.util.Motion
 * @className YAHOO.ext.Actor
 * @constructor
 * Create new Actor.
 * @param {String/HTMLElement} el The dom element or element id 
 * @param {<i>YAHOO.ext.Animator</i>} animator (optional) The Animator that will capture this Actor's actions
 * @param {<i>Boolean</i>} selfCapture (optional) Whether this actor should capture it's own actions to support self playback without an animator (defaults to false)
 */
YAHOO.ext.Actor = function(element, animator, selfCapture){
    this.el = YAHOO.ext.Element.get(element, true); // cache el object for playback
    YAHOO.ext.Actor.superclass.constructor.call(this, element, true);
    this.onCapture = new YAHOO.util.CustomEvent('Actor.onCapture');
    if(animator){
        /**
        * The animator used to sync this actor with other actors
        * @member YAHOO.ext.Actor
        */
        animator.addActor(this);
    }
    /**
    * Whether this actor is currently capturing
    * @member YAHOO.ext.Actor
    */
    this.capturing = selfCapture;
    this.playlist = selfCapture ? new YAHOO.ext.Animator.AnimSequence() : null;
};

YAHOO.extendX(YAHOO.ext.Actor, YAHOO.ext.Element);

/**
 * Captures an action for this actor. Generally called internally but can be called directly.
 # @param {YAHOO.ext.Actor.Action} action
 */
YAHOO.ext.Actor.prototype.capture = function(action){
    if(this.playlist != null){
        this.playlist.add(action);
    }
    this.onCapture.fireDirect(this, action);
    return action;
};

/** @ignore */
YAHOO.ext.Actor.overrideAnimation = function(method, animParam, onParam){
    return function(){
        if(!this.capturing){
            return method.apply(this, arguments);
        }
        var args = Array.prototype.slice.call(arguments, 0);
        if(args[animParam] === true){
            return this.capture(new YAHOO.ext.Actor.AsyncAction(this, method, args, onParam));
        }else{
            return this.capture(new YAHOO.ext.Actor.Action(this, method, args));
        }
    };
}

/** @ignore */
YAHOO.ext.Actor.overrideBasic = function(method){
    return function(){
        if(!this.capturing){
            return method.apply(this, arguments);
        }
        var args = Array.prototype.slice.call(arguments, 0);
        return this.capture(new YAHOO.ext.Actor.Action(this, method, args));
    };
}

// All of these methods below are marked "ignore" because JSDoc treats them as fields, not function. How brilliant. The Element methods are documented anyway though.
/** Capturing override - See {@link YAHOO.ext.Element#setVisibilityMode} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.setVisibilityMode = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.setVisibilityMode);
/** Capturing override - See {@link YAHOO.ext.Element#enableDisplayMode} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.enableDisplayMode = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.enableDisplayMode);
/** Capturing override - See {@link YAHOO.ext.Element#focus} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.focus = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.focus);
/** Capturing override - See {@link YAHOO.ext.Element#addClass} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.addClass = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.addClass);
/** Capturing override - See {@link YAHOO.ext.Element#removeClass} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.removeClass = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.removeClass);
/** Capturing override - See {@link YAHOO.ext.Element#replaceClass} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.replaceClass = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.replaceClass);
/** Capturing override - See {@link YAHOO.ext.Element#setStyle} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.setStyle = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.setStyle);
/** Capturing override - See {@link YAHOO.ext.Element#setLeft} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.setLeft = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.setLeft);
/** Capturing override - See {@link YAHOO.ext.Element#setTop} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.setTop = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.setTop);
/** Capturing override - See {@link YAHOO.ext.Element#setAbsolutePositioned} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.setAbsolutePositioned = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.setAbsolutePositioned);
/** Capturing override - See {@link YAHOO.ext.Element#setRelativePositioned} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.setRelativePositioned = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.setRelativePositioned);
/** Capturing override - See {@link YAHOO.ext.Element#clearPositioning} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.clearPositioning = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.clearPositioning);
/** Capturing override - See {@link YAHOO.ext.Element#setPositioning} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.setPositioning = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.setPositioning);
/** Capturing override - See {@link YAHOO.ext.Element#clip} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.clip = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.clip);
/** Capturing override - See {@link YAHOO.ext.Element#unclip} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.unclip = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.unclip);
/** Capturing override - See {@link YAHOO.ext.Element#clearOpacity} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.clearOpacity = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.clearOpacity);
/** Capturing override - See {@link YAHOO.ext.Element#update} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.update = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.update);
/** Capturing override - See {@link YAHOO.ext.Element#remove} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.remove = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.remove);
YAHOO.ext.Actor.prototype.fitToParent = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.fitToParent);
YAHOO.ext.Actor.prototype.appendChild = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.appendChild);
YAHOO.ext.Actor.prototype.createChild = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.createChild);
YAHOO.ext.Actor.prototype.appendTo = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.appendTo);
YAHOO.ext.Actor.prototype.insertBefore = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.insertBefore);
YAHOO.ext.Actor.prototype.insertAfter = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.insertAfter);
YAHOO.ext.Actor.prototype.wrap = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.wrap);
YAHOO.ext.Actor.prototype.replace = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.replace);
YAHOO.ext.Actor.prototype.insertHtml = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.insertHtml);
YAHOO.ext.Actor.prototype.set = YAHOO.ext.Actor.overrideBasic(YAHOO.ext.Actor.superclass.set);

/**Capturing and animation syncing override - See {@link YAHOO.ext.Element#load} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.load = function(){
   if(!this.capturing){
        return YAHOO.ext.Actor.superclass.load.apply(this, arguments);
   }
   var args = Array.prototype.slice.call(arguments, 0);
   return this.capture(new YAHOO.ext.Actor.AsyncAction(this, YAHOO.ext.Actor.superclass.load, 
        args, 2));
};

/**Capturing and animation syncing override - See {@link YAHOO.ext.Element#animate} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.animate = function(args, duration, onComplete, easing, animType){
    if(!this.capturing){
        return YAHOO.ext.Actor.superclass.animate.apply(this, arguments);
    }
    return this.capture(new YAHOO.ext.Actor.AsyncAction(this, YAHOO.ext.Actor.superclass.animate, 
        [args, duration, onComplete, easing, animType], 2));
};

/** Capturing and animation syncing override - See {@link YAHOO.ext.Element#setVisible} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.setVisible = YAHOO.ext.Actor.overrideAnimation(YAHOO.ext.Actor.superclass.setVisible, 1, 3);
/**Capturing and animation syncing override - See {@link YAHOO.ext.Element#toggle} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.toggle = YAHOO.ext.Actor.overrideAnimation(YAHOO.ext.Actor.superclass.toggle, 0, 2);
/**Capturing and animation syncing override - See {@link YAHOO.ext.Element#setXY} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.setXY = YAHOO.ext.Actor.overrideAnimation(YAHOO.ext.Actor.superclass.setXY, 1, 3);
/**Capturing and animation syncing override - See {@link YAHOO.ext.Element#setLocation} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.setLocation = YAHOO.ext.Actor.overrideAnimation(YAHOO.ext.Actor.superclass.setLocation, 2, 4);
/**Capturing and animation syncing override - See {@link YAHOO.ext.Element#setWidth} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.setWidth = YAHOO.ext.Actor.overrideAnimation(YAHOO.ext.Actor.superclass.setWidth, 1, 3);
/**Capturing and animation syncing override - See {@link YAHOO.ext.Element#setHeight} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.setHeight = YAHOO.ext.Actor.overrideAnimation(YAHOO.ext.Actor.superclass.setHeight, 1, 3);
/**Capturing and animation syncing override - See {@link YAHOO.ext.Element#setSize} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.setSize = YAHOO.ext.Actor.overrideAnimation(YAHOO.ext.Actor.superclass.setSize, 2, 4);
/**Capturing and animation syncing override - See {@link YAHOO.ext.Element#setBounds} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.setBounds = YAHOO.ext.Actor.overrideAnimation(YAHOO.ext.Actor.superclass.setBounds, 4, 6);
/**Capturing and animation syncing override - See {@link YAHOO.ext.Element#setOpacity} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.setOpacity = YAHOO.ext.Actor.overrideAnimation(YAHOO.ext.Actor.superclass.setOpacity, 1, 3);
/**Capturing and animation syncing override - See {@link YAHOO.ext.Element#moveTo} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.moveTo = YAHOO.ext.Actor.overrideAnimation(YAHOO.ext.Actor.superclass.moveTo, 2, 4);
/**Capturing and animation syncing override - See {@link YAHOO.ext.Element#move} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.move = YAHOO.ext.Actor.overrideAnimation(YAHOO.ext.Actor.superclass.move, 2, 4);
/**Capturing and animation syncing override - See {@link YAHOO.ext.Element#alignTo} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.alignTo = YAHOO.ext.Actor.overrideAnimation(YAHOO.ext.Actor.superclass.alignTo, 3, 5);
/**Capturing and animation syncing override - See {@link YAHOO.ext.Element#hide} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.hide = YAHOO.ext.Actor.overrideAnimation(YAHOO.ext.Actor.superclass.hide, 0, 2);
/**Capturing and animation syncing override - See {@link YAHOO.ext.Element#show} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.show = YAHOO.ext.Actor.overrideAnimation(YAHOO.ext.Actor.superclass.show, 0, 2);

/**Capturing and animation syncing override - See {@link YAHOO.ext.Element#setBox} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.setBox = YAHOO.ext.Actor.overrideAnimation(YAHOO.ext.Actor.superclass.setBox, 2, 4);

/**Capturing and animation syncing override - See {@link YAHOO.ext.Element#autoHeight} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.autoHeight = YAHOO.ext.Actor.overrideAnimation(YAHOO.ext.Actor.superclass.autoHeight, 0, 2);
/** Capturing override - See {@link YAHOO.ext.Element#setX} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.setX = YAHOO.ext.Actor.overrideAnimation(YAHOO.ext.Actor.superclass.setX, 1, 3);
/** Capturing override - See {@link YAHOO.ext.Element#setY} for method details.
 * @type Function */
YAHOO.ext.Actor.prototype.setY = YAHOO.ext.Actor.overrideAnimation(YAHOO.ext.Actor.superclass.setY, 1, 3);

/**
 * Start self capturing calls on this Actor. All subsequent calls are captured and executed when play() is called.
 */
YAHOO.ext.Actor.prototype.startCapture = function(){
    this.capturing = true;
    this.playlist = new YAHOO.ext.Animator.AnimSequence();
 };
 
 /**
 * Stop self capturing calls on this Actor.
 */
 YAHOO.ext.Actor.prototype.stopCapture = function(){
     this.capturing = false;
 };

/**
 * Clears any calls that have been self captured.
 */
YAHOO.ext.Actor.prototype.clear = function(){
    this.playlist = new YAHOO.ext.Animator.AnimSequence();
};

/**
 * Starts playback of self captured calls.
 * @param {<i>Function</i>} oncomplete (optional) Callback to execute when playback has completed
 */
YAHOO.ext.Actor.prototype.play = function(oncomplete){
    this.capturing = false;
    if(this.playlist){
        this.playlist.play(oncomplete);
    }
 };

/**
 * Capture a function call.
 * @param {Function} fcn The function to call
 * @param {<i>Array</i>} args (optional) The arguments to call the function with
 * @param {<i>Object</i>} scope (optional) The scope of the function
 */
YAHOO.ext.Actor.prototype.addCall = function(fcn, args, scope){
    this.capture(new YAHOO.ext.Actor.Action(scope, fcn, args || []));
};

/**
 * Capture an async function call.
 * @param {Function} fcn The function to call
 * @param {Number} callbackIndex The index of the callback parameter on the passed function. A CALLBACK IS REQUIRED.
 * @param {<i>Array</i>} args The arguments to call the function with
 * @param {<i>Object</i>} scope (optional) The scope of the function
 */
YAHOO.ext.Actor.prototype.addAsyncCall = function(fcn, callbackIndex, args, scope){
    this.capture(new YAHOO.ext.Actor.AsyncAction(scope, fcn, args || [], callbackIndex));
 },
 
/**
 * Capture a pause (in seconds).
 * @param {Number} seconds The seconds to pause
 */
YAHOO.ext.Actor.prototype.pause = function(seconds){
    this.capture(new YAHOO.ext.Actor.PauseAction(seconds));
 };
 
/**
* Shake this element from side to side
*/
YAHOO.ext.Actor.prototype.shake = function(){
    this.move('left', 20, true, .05);
    this.move('right', 40, true, .05);
    this.move('left', 40, true, .05);
    this.move('right', 20, true, .05);
};

/**
* Bounce this element from up and down
*/
YAHOO.ext.Actor.prototype.bounce = function(){
    this.move('up', 20, true, .05);
    this.move('down', 40, true, .05);
    this.move('up', 40, true, .05);
    this.move('down', 20, true, .05);
};

/**
* Show the element using a "blinds" effect
* @param {String} anchor The part of the element that it should appear to exapand from. 
                        The short/long options currently are t/top, l/left
* @param {<i>Number</i>} newSize (optional) The size to animate to. (Default to current size)
* @param {<i>Float</i>} duration (optional) How long the effect lasts (in seconds)
* @param {<i>Function</i>} easing (optional) YAHOO.util.Easing method to use. (Defaults to YAHOO.util.Easing.easeOut)
*/
YAHOO.ext.Actor.prototype.blindShow = function(anchor, newSize, duration, easing){
    var size = newSize || this.getSize();
    this.clip();
    this.setVisible(true);
    anchor = anchor.toLowerCase();
    switch(anchor){
        case 't':
        case 'top':
            this.setHeight(1);
            this.setHeight(newSize, true, duration || .5, null, easing || YAHOO.util.Easing.easeOut);
        break;
        case 'l':
        case 'left':
            this.setWidth(1);
            this.setWidth(newSize, true, duration || .5, null, easing || YAHOO.util.Easing.easeOut);
        break;
    }
    this.unclip();
    return size;
};

/**
* Hide the element using a "blinds" effect
* @param {String} anchor The part of the element that it should appear to collapse to.
                        The short/long options are t/top, l/left, b/bottom, r/right.
* @param {<i>Float</i>} duration (optional) How long the effect lasts (in seconds)
* @param {<i>Function</i>} easing (optional) YAHOO.util.Easing method to use. (Defaults to YAHOO.util.Easing.easeIn)
*/
YAHOO.ext.Actor.prototype.blindHide = function(anchor, duration, easing){
    var size = this.getSize();
    this.clip();
    anchor = anchor.toLowerCase();
    switch(anchor){
        case 't':
        case 'top':
            this.setSize(size.width, 1, true, duration || .5, null, easing || YAHOO.util.Easing.easeIn);
            this.setVisible(false);
        break;
        case 'l':
        case 'left':
            this.setSize(1, size.height, true, duration || .5, null, easing || YAHOO.util.Easing.easeIn);
            this.setVisible(false);
        break;
        case 'r':
        case 'right':
            this.animate({width: {to: 1}, points: {by: [this.getWidth(), 0]}}, 
            duration || .5, null, YAHOO.util.Easing.easeIn, YAHOO.util.Motion);
            this.setVisible(false);
        break;
        case 'b':
        case 'bottom':
            this.animate({height: {to: 1}, points: {by: [0, this.getHeight()]}}, 
            duration || .5, null, YAHOO.util.Easing.easeIn, YAHOO.util.Motion);
            this.setVisible(false);
        break;
    }
    return size;
};

/**
* Show the element using a "slide in" effect - In order for this effect to work the element MUST have a child element container that can be "slid" otherwise a blindShow effect is rendered. 
* @param {String} anchor The part of the element that it should appear to slide from. 
                        The short/long options currently are t/top, l/left
* @param {<i>Number</i>} newSize (optional) The size to animate to. (Default to current size)
* @param {<i>Float</i>} duration (optional) How long the effect lasts (in seconds)
* @param {<i>Function</i>} easing (optional) YAHOO.util.Easing method to use. (Defaults to YAHOO.util.Easing.easeOuth)
*/
YAHOO.ext.Actor.prototype.slideShow = function(anchor, newSize, duration, easing, clearPositioning){
    var size = newSize || this.getSize();
    this.clip();
    var firstChild = this.dom.firstChild;
    if(!firstChild || (firstChild.nodeName && "#TEXT" == firstChild.nodeName.toUpperCase())) { // can't do a slide with only a textnode
        this.blindShow(anchor, newSize, duration, easing);
        return;
    }
    var child = YAHOO.ext.Element.get(firstChild, true);
    var pos = child.getPositioning();
    this.addCall(child.setAbsolutePositioned, null, child);
    this.setVisible(true);
    anchor = anchor.toLowerCase();
    switch(anchor){
        case 't':
        case 'top':
            this.addCall(child.setStyle, ['right', ''], child);
            this.addCall(child.setStyle, ['top', ''], child);
            this.addCall(child.setStyle, ['left', '0px'], child);
            this.addCall(child.setStyle, ['bottom', '0px'], child);
            this.setHeight(1);
            this.setHeight(newSize, true, duration || .5, null, easing || YAHOO.util.Easing.easeOut);
        break;
        case 'l':
        case 'left':
            this.addCall(child.setStyle, ['left', ''], child);
            this.addCall(child.setStyle, ['bottom', ''], child);
            this.addCall(child.setStyle, ['right', '0px'], child);
            this.addCall(child.setStyle, ['top', '0px'], child);
            this.setWidth(1);
            this.setWidth(newSize, true, duration || .5, null, easing || YAHOO.util.Easing.easeOut);
        break;
        case 'r':
        case 'right':
            this.addCall(child.setStyle, ['left', '0px'], child);
            this.addCall(child.setStyle, ['top', '0px'], child);
            this.addCall(child.setStyle, ['right', ''], child);
            this.addCall(child.setStyle, ['bottom', ''], child);
            this.setWidth(1);
            this.setWidth(newSize, true, duration || .5, null, easing || YAHOO.util.Easing.easeOut);
        break;
        case 'b':
        case 'bottom':
            this.addCall(child.setStyle, ['right', ''], child);
            this.addCall(child.setStyle, ['top', '0px'], child);
            this.addCall(child.setStyle, ['left', '0px'], child);
            this.addCall(child.setStyle, ['bottom', ''], child);
            this.setHeight(1);
            this.setHeight(newSize, true, duration || .5, null, easing || YAHOO.util.Easing.easeOut);
        break;
    }
    if(clearPositioning !== false){
      this.addCall(child.setPositioning, [pos], child);
    }
    this.unclip();
    return size;
};

/**
* Hide the element using a "slide in" effect - In order for this effect to work the element MUST have a child element container that can be "slid" otherwise a blindHide effect is rendered. 
* @param {String} anchor The part of the element that it should appear to slide to.
                        The short/long options are t/top, l/left, b/bottom, r/right.
* @param {<i>Float</i>} duration (optional) How long the effect lasts (in seconds)
* @param {<i>Function</i>} easing (optional) YAHOO.util.Easing method to use. (Defaults to YAHOO.util.Easing.easeIn)
*/
YAHOO.ext.Actor.prototype.slideHide = function(anchor, duration, easing){
    var size = this.getSize();
    this.clip();
    var firstChild = this.dom.firstChild;
    if(!firstChild || (firstChild.nodeName && "#TEXT" == firstChild.nodeName.toUpperCase())) { // can't do a slide with only a textnode
        this.blindHide(anchor, duration, easing);
        return;
    }
    var child = YAHOO.ext.Element.get(firstChild, true);
    var pos = child.getPositioning();
    this.addCall(child.setAbsolutePositioned, null, child);
    anchor = anchor.toLowerCase();
    switch(anchor){
        case 't':
        case 'top':
            this.addCall(child.setStyle, ['right', ''], child);
            this.addCall(child.setStyle, ['top', ''], child);
            this.addCall(child.setStyle, ['left', '0px'], child);
            this.addCall(child.setStyle, ['bottom', '0px'], child);
            this.setSize(size.width, 1, true, duration || .5, null, easing || YAHOO.util.Easing.easeIn);
            this.setVisible(false);
        break;
        case 'l':
        case 'left':
            this.addCall(child.setStyle, ['left', ''], child);
            this.addCall(child.setStyle, ['bottom', ''], child);
            this.addCall(child.setStyle, ['right', '0px'], child);
            this.addCall(child.setStyle, ['top', '0px'], child);
            this.setSize(1, size.height, true, duration || .5, null, easing || YAHOO.util.Easing.easeIn);
            this.setVisible(false);
        break;
        case 'r':
        case 'right':
            this.addCall(child.setStyle, ['right', ''], child);
            this.addCall(child.setStyle, ['bottom', ''], child);
            this.addCall(child.setStyle, ['left', '0px'], child);
            this.addCall(child.setStyle, ['top', '0px'], child);
            this.setSize(1, size.height, true, duration || .5, null, easing || YAHOO.util.Easing.easeIn);
            this.setVisible(false);
        break;
        case 'b':
        case 'bottom':
            this.addCall(child.setStyle, ['right', ''], child);
            this.addCall(child.setStyle, ['top', '0px'], child);
            this.addCall(child.setStyle, ['left', '0px'], child);
            this.addCall(child.setStyle, ['bottom', ''], child);
            this.setSize(size.width, 1, true, duration || .5, null, easing || YAHOO.util.Easing.easeIn);
            this.setVisible(false);
        break;
    }
    this.addCall(child.setPositioning, [pos], child);
    return size;
};

/**
* Hide the element by "squishing" it into the corner
* @param {<i>Float</i>} duration (optional) How long the effect lasts (in seconds)
*/
YAHOO.ext.Actor.prototype.squish = function(duration){
    var size = this.getSize();
    this.clip();
    this.setSize(1, 1, true, duration || .5);
    this.setVisible(false);
    return size;
};

/**
* Fade an element in
* @param {<i>Float</i>} duration (optional) How long the effect lasts (in seconds)
*/
YAHOO.ext.Actor.prototype.appear = function(duration){
    this.setVisible(true, true, duration);
};

/**
* Fade an element out
* @param {<i>Float</i>} duration (optional) How long the effect lasts (in seconds)
*/
YAHOO.ext.Actor.prototype.fade = function(duration){
    this.setVisible(false, true, duration);
};

/**
* Blink the element as if it was clicked and then collapse on it's center
* @param {<i>Float</i>} duration (optional) How long the effect lasts (in seconds)
*/
YAHOO.ext.Actor.prototype.switchOff = function(duration){
    this.clip();
    this.setVisible(false, true, .1);
    this.clearOpacity();
    this.setVisible(true);
    this.animate({height: {to: 1}, points: {by: [0, this.getHeight()/2]}}, 
            duration || .5, null, YAHOO.util.Easing.easeOut, YAHOO.util.Motion);
    this.setVisible(false);
};

/**
* Highlight the element using a background color (or passed attribute) animation
* @param {String} color (optional) The color to use for the highlight
* @param {<i>String</i>} fromColor (optional) If the element does not currently have a background color, you will need to pass in a color to animate from
* @param {<i>Float</i>} duration (optional) How long the effect lasts (in seconds)
* @param {<i>String</i>} attribute (optional) Specify a CSS attribute to use other than background color - camelCase
*/
YAHOO.ext.Actor.prototype.highlight = function(color, fromColor, duration, attribute){
    attribute = attribute || 'background-color';
    var original = this.getStyle(attribute);
    fromColor = fromColor || ((original && original != '' && original != 'transparent') ? original : '#FFFFFF');
    var cfg = {};
    cfg[attribute] = {to: color, from: fromColor};
    this.setVisible(true);
    this.animate(cfg, duration || .5, null, YAHOO.util.Easing.bounceOut, YAHOO.util.ColorAnim);
    this.setStyle(attribute, original);
};

/**
* Fade the element in and out the specified amount of times
* @param {<i>Number</i>} count (optional) How many times to pulse (Defaults to 3)
* @param {<i>Float</i>} duration (optional) How long the effect lasts (in seconds)
*/
YAHOO.ext.Actor.prototype.pulsate = function(count, duration){
    count = count || 3;
    for(var i = 0; i < count; i++){
        this.toggle(true, duration || .25);
        this.toggle(true, duration || .25);
    }
};

/**
* Fade the element as it is falling from it's current position
* @param {<i>Float</i>} duration (optional) How long the effect lasts (in seconds)
*/
YAHOO.ext.Actor.prototype.dropOut = function(duration){
    this.animate({opacity: {to: 0}, points: {by: [0, this.getHeight()]}}, 
            duration || .5, null, YAHOO.util.Easing.easeIn, YAHOO.util.Motion);
    this.setVisible(false);
};

/**
* Hide the element in a way that it appears as if it is flying off the screen
* @param {String} anchor The part of the page that the element should appear to move to. 
                        The short/long options are t/top, l/left, b/bottom, r/right, tl/top-left, 
                        tr/top-right, bl/bottom-left or br/bottom-right.
* @param {<i>Float</i>} duration (optional) How long the effect lasts (in seconds)
* @param {<i>Function</i>} easing (optional) YAHOO.util.Easing method to use. (Defaults to YAHOO.util.Easing.easeIn)
*/
YAHOO.ext.Actor.prototype.moveOut = function(anchor, duration, easing){
    var Y = YAHOO.util;
    var vw = Y.Dom.getViewportWidth();
    var vh = Y.Dom.getViewportHeight();
    var cpoints = this.getCenterXY()
    var centerX = cpoints[0];
    var centerY = cpoints[1];
    var anchor = anchor.toLowerCase();
    var p;
    switch(anchor){
        case 't':
        case 'top':
            p = [centerX, -this.getHeight()];
        break;
        case 'l':
        case 'left':
            p = [-this.getWidth(), centerY];
        break;
        case 'r':
        case 'right':
            p = [vw+this.getWidth(), centerY];
        break;
        case 'b':
        case 'bottom':
            p = [centerX, vh+this.getHeight()];
        break;
        case 'tl':
        case 'top-left':
            p = [-this.getWidth(), -this.getHeight()];
        break;
        case 'bl':
        case 'bottom':
            p = [-this.getWidth(), vh+this.getHeight()];
        break;
        case 'br':
        case 'bottom-right':
            p = [vw+this.getWidth(), vh+this.getHeight()];
        break;
        case 'tr':
        case 'top-right':
            p = [vw+this.getWidth(), -this.getHeight()];
        break;
    }
    this.moveTo(p[0], p[1], true, duration || .35, null, easing || Y.Easing.easeIn);
    this.setVisible(false);
};

/**
* Show the element in a way that it appears as if it is flying onto the screen
* @param {String} anchor The part of the page that the element should appear to move from. 
                        The short/long options are t/top, l/left, b/bottom, r/right, tl/top-left, 
                        tr/top-right, bl/bottom-left or br/bottom-right.
* @param {<i>Array</i>} to (optional) Array of x and y position to move to like [x, y] (Defaults to center screen)
* @param {<i>Float</i>} duration (optional) How long the effect lasts (in seconds)
* @param {<i>Function</i>} easing (optional) YAHOO.util.Easing method to use. (Defaults to YAHOO.util.Easing.easeOut)
*/
YAHOO.ext.Actor.prototype.moveIn = function(anchor, to, duration, easing){
    to = to || this.getCenterXY();
    this.moveOut(anchor, .01);
    this.setVisible(true);
    this.setXY(to, true, duration || .35, null, easing || YAHOO.util.Easing.easeOut);
};

/**
 * @class Used by {@link YAHOO.ext.Actor} to queue standard calls. Generally used internally. Documentation to come.<br><br>
 */
YAHOO.ext.Actor.Action = function(actor, method, args){
      this.actor = actor;
      this.method = method;
      this.args = args;
      //alert('queueing ' + actor.id + ' ' + method.toString());
  }
  
YAHOO.ext.Actor.Action.prototype = {
    play : function(onComplete){
        this.method.apply(this.actor || window, this.args);
        onComplete();
    }  
};


/**
 * @class Used by {@link YAHOO.ext.Actor} to queue animations. Generally used internally. Documentation to come.<br><br>
 */
YAHOO.ext.Actor.AsyncAction = function(actor, method, args, onIndex){
    YAHOO.ext.Actor.AsyncAction.superclass.constructor.call(this, actor, method, args);
    this.onIndex = onIndex;
    this.originalCallback = this.args[onIndex];
}
YAHOO.extendX(YAHOO.ext.Actor.AsyncAction, YAHOO.ext.Actor.Action);

YAHOO.ext.Actor.AsyncAction.prototype.play = function(onComplete){
    var callbackArg = this.originalCallback ? 
                        this.originalCallback.createSequence(onComplete) : onComplete;
    this.args[this.onIndex] = callbackArg;
    this.method.apply(this.actor, this.args);
};


/**
 * @class Used by {@link YAHOO.ext.Actor} to perform pauses. Generally used internally. Documentation to come.<br><br>
 */
YAHOO.ext.Actor.PauseAction = function(seconds){
    this.seconds = seconds;
};
YAHOO.ext.Actor.PauseAction.prototype = {
    play : function(onComplete){
        setTimeout(onComplete, this.seconds * 1000);
    }
};

/*
------------------------------------------------------------------
// File: anim\Animator.js
------------------------------------------------------------------
*/
/**
 * @class
 * Provides support for syncing animations for multiple {@link YAHOO.ext.Actor}s.<br><br>
* <br><br>This example can be seen in action <a href="http://jackslocum.blogspot.com/2006/08/splitbar-component-for-yahoo-ui.html" target="_new">here</a> by clicking on "Click here and I will point it out".<br>
 * <pre><code>
var animator = new YAHOO.ext.Animator();
var cursor = new YAHOO.ext.Actor('cursor-img', animator);
var click = new YAHOO.ext.Actor('click-img', animator);
var resize = new YAHOO.ext.Actor('resize-img', animator);

// start capturing
animator.startCapture();

// these animations will be run in sequence
cursor.show();
cursor.moveTo(500,400);
cursor.moveTo(20, getEl('navbar').getY()+10, true, .75);
click.show();
click.alignTo(cursor, 'tl', [-4, -4]);

// Add an async function call, pass callback to argument 1
animator.addAsyncCall(Blog.navbar.undockDelegate, 1);

// pause .5 seconds
animator.pause(.5);

// again, these animations will be run in sequence
click.hide(true, .7);
cursor.alignTo('splitter', 'tr', [0, +100], true, 1);
resize.alignTo('splitter', 'tr', [-12, +100]);

// start sync block: these animations will run at the same time
animator.beginSync();
cursor.hide();
resize.show();
animator.endSync();

// play the captured animation sequences, call myCallback when done
animator.play(myCallback);
 * </code></pre>
 * @extends YAHOO.ext.Element
 * @requires YAHOO.ext.Element
 * @requires YAHOO.util.Dom
 * @requires YAHOO.util.Event
 * @requires YAHOO.util.CustomEvent 
 * @requires YAHOO.util.Anim
 * @requires YAHOO.util.ColorAnim
 * @requires YAHOO.util.Motion
 * @constructor
 * @param {String/HTMLElement} el The dom element or element id 
 * @param {<i>YAHOO.ext.Animator</i>} animator (optional) The Animator that will capture this Actor's actions
 * @param {<i>Boolean</i>} selfCapture (optional) Whether this actor should capture it's own actions to support self playback without an animator (defaults to false)
 */ 
  YAHOO.ext.Animator = function(/*Actors...*/){
    /** @private */
    this.actors = [];
    /** @private */
    this.playlist = new YAHOO.ext.Animator.AnimSequence();
    /** @private */
    this.captureDelegate = this.capture.createDelegate(this);
    /** @private */
    this.playDelegate = this.play.createDelegate(this);
    /** @private */
    this.syncing = false;
    /** @private */
    this.stopping = false;
    /** @private */
    this.playing = false;
    for(var i = 0; i < arguments.length; i++){
        this.addActor(arguments[i]);
    }
 };
 
 YAHOO.ext.Animator.prototype = {
 
    /**
      * @private
      */
     capture : function(actor, action){
        if(this.syncing){
            if(!this.syncMap[actor.id]){
                this.syncMap[actor.id] = new YAHOO.ext.Animator.AnimSequence();
            }
            this.syncMap[actor.id].add(action);
        }else{
            this.playlist.add(action);
        }
    },
    
    /**
      * Add an actor. The actor is also set to capturing = true.
      * @param {YAHOO.ext.Actor} actor
      */
     addActor : function(actor){
        actor.onCapture.subscribe(this.captureDelegate);
        this.actors.push(actor);
    },
    
    
    /**
      * Start capturing actions on the added actors. 
      * @param {<i>Boolean</i>} clearPlaylist Whether to also create a new playlist
      */
     startCapture : function(clearPlaylist){
        for(var i = 0; i < this.actors.length; i++){
            var a = this.actors[i];
            if(!this.isCapturing(a)){
                a.onCapture.subscribe(this.captureDelegate);
            }
            a.capturing = true;
        }
        if(clearPlaylist){
            this.playlist = new YAHOO.ext.Animator.AnimSequence();
        }
     },
     
     /**
      * Checks whether this animator is listening to a specific actor.
      * @param {YAHOO.ext.Actor} actor
      */
     isCapturing : function(actor){
        var subscribers = actor.onCapture.subscribers;
        if(subscribers){
            for(var i = 0; i < subscribers.length; i++){
                if(subscribers[i] && subscribers[i].contains(this.captureDelegate)){
                    return true;
                }
            }
        }
        return false;
     },
     
     /**
      * Stop capturing on all added actors.
      */
     stopCapture : function(){
         for(var i = 0; i < this.actors.length; i++){
            var a = this.actors[i];
            a.onCapture.unsubscribe(this.captureDelegate);
            a.capturing = false;
         }
     },
     
     /**
     * Start a multi-actor sync block. By default all animations are run in sequence. While in the sync block
     * each actor's own animations will still be sequenced, but all actors will animate at the same time. 
     */
    beginSync : function(){
        this.syncing = true;
        this.syncMap = {};
     },
     
     /**
     * End the multi-actor sync block
     */
    endSync : function(){
         this.syncing = false;
         var composite = new YAHOO.ext.Animator.CompositeSequence();
         for(key in this.syncMap){
             if(typeof this.syncMap[key] != 'function'){
                composite.add(this.syncMap[key]);
             }
         }
         this.playlist.add(composite);
         this.syncMap = null;
     },
     
    /**
     * Starts playback of the playlist, also stops any capturing. To start capturing again call {@link #startCapture}.
     * @param {<i>Function</i>} oncomplete (optional) Callback to execute when playback has completed
     */
    play : function(oncomplete){
        if(this.playing) return; // can't play the same animation twice at once
        this.stopCapture();
        this.playlist.play(oncomplete);
    },
    
    /**
     * Stop at the next available stopping point
     */
    stop : function(){
        this.playlist.stop();
    },
    
    /**
     * Check if this animator is currently playing
     */
    isPlaying : function(){
        return this.playlist.isPlaying();
    },
    /**
     * Clear the playlist
     */
    clear : function(){
        this.playlist = new YAHOO.ext.Animator.AnimSequence();
     },
     
    /**
     * Add a function call to the playlist.
     * @param {Function} fcn The function to call
     * @param {<i>Array</i>} args The arguments to call the function with
     * @param {<i>Object</i>} scope (optional) The scope of the function
     */
     addCall : function(fcn, args, scope){
        this.playlist.add(new YAHOO.ext.Actor.Action(scope, fcn, args || []));
     },
     
     /**
     * Add an async function call to the playlist.
     * @param {Function} fcn The function to call
     * @param {Number} callbackIndex The index of the callback parameter on the passed function. A CALLBACK IS REQUIRED.
     * @param {<i>Array</i>} args The arguments to call the function with
     * @param {<i>Object</i>} scope (optional) The scope of the function
     */
    addAsyncCall : function(fcn, callbackIndex, args, scope){
        this.playlist.add(new YAHOO.ext.Actor.AsyncAction(scope, fcn, args || [], callbackIndex));
     },
     
     /**
     * Add a pause to the playlist (in seconds)
     * @param {Number} seconds The number of seconds to pause.
     */
    pause : function(seconds){
        this.playlist.add(new YAHOO.ext.Actor.PauseAction(seconds));
     }
     
  };

/**
 * Composite class with synchronized animations.
 */
YAHOO.ext.AnimatorComposite = function(els){
    this.animator = new YAHOO.ext.Animator();
    this.addElements(els);
    this.syncAnims = true;
};
YAHOO.ext.AnimatorComposite.prototype = {
    isComposite: true,
    addElements : function(els){
        if(!els) return this;
        var anim = this.animator;
        for(var i = 0, len = els.length; i < len; i++) {
        	anim.addActor(new YAHOO.ext.Actor(els[i]));
        }
        anim.startCapture();
        return this;
    },
    sequence : function(){
        this.syncAnims = false;
        return this;
    },
    sync : function(){
        this.syncAnims = true;
        return this;
    },
    invoke : function(fn, args){
        var els = this.animator.actors;
        if(this.syncAnims) this.animator.beginSync();
        for(var i = 0, len = els.length; i < len; i++) {
            YAHOO.ext.Actor.prototype[fn].apply(els[i], args);
        }
        if(this.syncAnims) this.animator.endSync();
        return this;
    },
    play : function(callback){
        this.animator.play(callback);
        return this;
    },
    reset : function(callback){
        this.animator.startCapture(true);
        return this;
    },
    pause : function(seconds){
        this.animator.pause(seconds);
        return this;
    },
    getAnimator : function(){
        return this.animator;
    },
    each : function(fn, scope){
        var els = this.animator.actors;
        if(this.syncAnims) this.animator.beginSync();
        for(var i = 0, len = els.length; i < len; i++){
            fn.call(scope || els[i], els[i], this, i);
        }
        if(this.syncAnims) this.animator.endSync();
        return this;
    },
    addCall : function(fcn, args, scope){
        this.animator.addCall(fcn, args, scope);
        return this;
    },
    addAsyncCall : function(fcn, callbackIndex, args, scope){
        this.animator.addAsyncCall(fcn, callbackIndex, args, scope);
        return this;
    }
};
for(var fnName in YAHOO.ext.Actor.prototype){
    if(typeof YAHOO.ext.Actor.prototype[fnName] == 'function'){
        YAHOO.ext.CompositeElement.createCall(YAHOO.ext.AnimatorComposite.prototype, fnName);
    }
}
YAHOO.ext.Animator.select = function(selector){
    var els;
    if(typeof selector == 'string'){
        els = YAHOO.ext.Element.selectorFunction(selector);
    }else if(els instanceof Array){
        els = selector;
    }else{
        throw 'Invalid selector';
    }
    return new YAHOO.ext.AnimatorComposite(els);
};
var getActors = YAHOO.ext.Animator.select;

/**
 * @class Used by {@link YAHOO.ext.Animator} to sequence animations. Generally used internally. Documentation to come.<br><br>
 */
YAHOO.ext.Animator.AnimSequence = function(){
    this.actions = [];
    this.nextDelegate = this.next.createDelegate(this);
    this.playDelegate = this.play.createDelegate(this);
    this.oncomplete = null;
    this.playing = false;
    this.stopping = false;
    this.actionIndex = -1;
 };
 
 YAHOO.ext.Animator.AnimSequence.prototype = {
 
    add : function(action){
        this.actions.push(action);
    },
    
    next : function(){
        if(this.stopping){
            this.playing = false;
            if(this.oncomplete){
                this.oncomplete(this, false);
            }
            return;
        }
        var nextAction = this.actions[++this.actionIndex];
        if(nextAction){
            nextAction.play(this.nextDelegate);
        }else{
            this.playing = false;
            if(this.oncomplete){
                this.oncomplete(this, true);
            }
        }
    },
    
    play : function(oncomplete){
        if(this.playing) return; // can't play the same sequence twice at once
        this.oncomplete = oncomplete;
        this.stopping = false;
        this.playing = true;
        this.actionIndex = -1;
        this.next();
    },
    
    stop : function(){
        this.stopping = true;
    },
    
    isPlaying : function(){
        return this.playing;
    },
    
    clear : function(){
        this.actions = [];
    },
     
    addCall : function(fcn, args, scope){
        this.actions.push(new YAHOO.ext.Actor.Action(scope, fcn, args || []));
     },
     
     /**
     * Add an async function call to the capture queue.
     * @param {Function} fcn The function to call
     * @param {Number} callbackIndex The index of the callback parameter on the passed function. A CALLBACK IS REQUIRED.
     * @param {<i>Array</i>} args The arguments to call the function with
     * @param {<i>Object</i>} scope (optional) The scope of the function
     */
    addAsyncCall : function(fcn, callbackIndex, args, scope){
        this.actions.push(new YAHOO.ext.Actor.AsyncAction(scope, fcn, args || [], callbackIndex));
     },
     
     pause : function(seconds){
        this.actions.push(new YAHOO.ext.Actor.PauseAction(seconds));
     }
     
  };

/**
 * @class Used by {@link YAHOO.ext.Animator} to run multiple animation sequences at once. Generally used internally. Documentation to come.<br><br>
 */
YAHOO.ext.Animator.CompositeSequence = function(){
    this.sequences = [];
    this.completed = 0;
    this.trackDelegate = this.trackCompletion.createDelegate(this);
}

YAHOO.ext.Animator.CompositeSequence.prototype = {
    add : function(sequence){
        this.sequences.push(sequence);
    },
    
    play : function(onComplete){
        this.completed = 0;
        if(this.sequences.length < 1){
            if(onComplete)onComplete();
            return;
        }
        this.onComplete = onComplete;
        for(var i = 0; i < this.sequences.length; i++){
            this.sequences[i].play(this.trackDelegate);
        }
    },
    
    trackCompletion : function(){
        ++this.completed;
        if(this.completed >= this.sequences.length && this.onComplete){
            this.onComplete();
        }
    },
    
    stop : function(){
        for(var i = 0; i < this.sequences.length; i++){
            this.sequences[i].stop();
        }
    },
    
    isPlaying : function(){
        for(var i = 0; i < this.sequences.length; i++){
            if(this.sequences[i].isPlaying()){
                return true;
            }
        }
        return false;
    }
};




/*
------------------------------------------------------------------
// File: widgets\Toolbar.js
------------------------------------------------------------------
*/
/**
 * @class
 * Basic Toolbar used by the Grid to create the paging toolbar. This class is reusable but functionality
 * is limited. Look for more functionality in a future version. 
 */
 YAHOO.ext.Toolbar = function(container){
    this.el = getEl(container, true);
    var div = document.createElement('div');
    div.className = 'ytoolbar';
    var tb = document.createElement('table');
    tb.border = 0;
    tb.cellPadding = 0;
    tb.cellSpacing = 0;
    div.appendChild(tb);
    var tbody = document.createElement('tbody');
    tb.appendChild(tbody);
    var tr = document.createElement('tr');
    tbody.appendChild(tr);
    this.el.dom.appendChild(div);
    this.tr = tr;
};

YAHOO.ext.Toolbar.prototype = {
    /**
     * Adds element(s) to the toolbar - this function takes a variable number of 
     * arguments of mixed type and adds them to the toolbar...
     * If an argument is a ToolbarButton, it is added. If the element is a string, it is wrapped 
     * in a ytb-text element and added unless the text is "separator" in which case a separator
     * is added. Otherwise, it is assumed the element is an HTMLElement and it is added directly.
     */
    add : function(){
        for(var i = 0; i < arguments.length; i++){
            var el = arguments[i];
            var td = document.createElement('td');
            this.tr.appendChild(td);
            if(el instanceof YAHOO.ext.ToolbarButton){
                el.init(td);
            }else if(el instanceof Array){
                this.addButton(el);
            }else if(typeof el == 'string'){
                var span = document.createElement('span');
                if(el == 'separator'){
                    span.className = 'ytb-sep';
                }else{
                    span.innerHTML = el;
                    span.className = 'ytb-text';
                }
                td.appendChild(span);
            }else if(typeof el == 'object'){ // must be element?
                td.appendChild(el);
            }
        }
    },
    
    getEl : function(){
        return this.el;  
    },
    
    addSeparator : function(){
        var td = document.createElement('td');
        this.tr.appendChild(td);
        var span = document.createElement('span');
        span.className = 'ytb-sep';
        td.appendChild(span);
    },
    
    /**
     * Add a button (or buttons), see {@link YAHOO.ext.ToolbarButton} for more info on the config
     * @param {Object/Array} config A button config or array of configs
     * @return {YAHOO.ext.ToolbarButton}
     */
    addButton : function(config){
        if(config instanceof Array){
            var buttons = [];
            for(var i = 0, len = config.length; i < len; i++) {
            	buttons.push(this.addButton(config[i]));
            }
            return buttons;
        }
        var b = new YAHOO.ext.ToolbarButton(config);
        this.add(b);
        return b;
    },
    
    addText : function(text){
        var td = document.createElement('td');
        this.tr.appendChild(td);
        var span = document.createElement('span');
        span.className = 'ytb-text';
        span.innerHTML = text;
        td.appendChild(span);
        return span;
    },
    
    insertButton : function(index, config){
        if(config instanceof Array){
            var buttons = [];
            for(var i = 0, len = config.length; i < len; i++) {
               buttons.push(this.insertButton(index + i, config[i]));
            }
            return buttons;
        }
        var b = new YAHOO.ext.ToolbarButton(config);
        var td = document.createElement('td');
        var nextSibling = this.tr.childNodes[index];
        if (nextSibling)
           this.tr.insertBefore(td, nextSibling);
       else
           this.tr.appendChild(td);
        b.init(td);
        return b;
    } 
};

/**
 * @class
 * A toolbar button. The config has the following options:
 * <ul>
 * <li>className - The CSS class for the button. Use this to attach a background image for an icon.</li>
 * <li>text - The button's text</li>
 * <li>tooltip - The buttons tooltip text</li>
 * <li>click - function to call when the button is clicked</li>
 * <li>mouseover - function to call when the mouse moves over the button</li>
 * <li>mouseout - function to call when the mouse moves off the button</li>
 * <li>scope - The scope of the above event handlers</li>
 * <li></li>
 * <li></li>
 */
YAHOO.ext.ToolbarButton = function(config){
    YAHOO.ext.util.Config.apply(this, config);
};

YAHOO.ext.ToolbarButton.prototype = {
    /** @private */
    init : function(appendTo){
        var element = document.createElement('span');
        element.className = 'ytb-button';
        element.unselectable = 'on';
        if(this.id){
            element.id = this.id;
        }
        this.disabled = (this.disabled === true);
        var inner = document.createElement('span');
        inner.className = 'ytb-button-inner ' + this.className;
        inner.unselectable = 'on';
        if(this.tooltip){
            element.setAttribute('title', this.tooltip);
        }
        if(this.style){
           YAHOO.ext.DomHelper.applyStyles(inner, this.style);
        } 
        element.appendChild(inner);
        appendTo.appendChild(element);
        this.el = getEl(element, true);
        inner.innerHTML = (this.text ? this.text : '&nbsp;');
        this.el.mon('click', this.onClick, this, true);    
        this.el.mon('mouseover', this.onMouseOver, this, true);    
        this.el.mon('mouseout', this.onMouseOut, this, true);
    },
    
    disable : function(){
        this.disabled = true;
        if(this.el){
            this.el.addClass('ytb-button-disabled');
        }
    },
    
    enable : function(){
        this.disabled = false;
        if(this.el){
            this.el.removeClass('ytb-button-disabled');
        }
    },
    
    isDisabled : function(){
        return this.disabled === true;
    },
    
    setDisabled : function(disabled){
        if(disabled){
            this.disable();
        }else{
            this.enable();
        }
    },
    
    /** @private */
    onClick : function(){
        if(!this.disabled && this.click){
            this.click.call(this.scope || window, this);
        }
    },
    
    /** @private */
    onMouseOver : function(){
        if(!this.disabled){
            this.el.addClass('ytb-button-over');
            if(this.mouseover){
                this.mouseover.call(this.scope || window, this);
            }
        }
    },
    
    /** @private */
    onMouseOut : function(){
        this.el.removeClass('ytb-button-over');
        if(!this.disabled){
            if(this.mouseout){
                this.mouseout.call(this.scope || window, this);
            }
        }
    }
};

/*
------------------------------------------------------------------
// File: widgets\Resizable.js
------------------------------------------------------------------
*/
/**
 * @class 
 * Makes an element resizable.
 */
YAHOO.ext.Resizable = function(el, config){
    // in case global fcn not defined
    var getEl = YAHOO.ext.Element.get;
    
    this.el = getEl(el, true);
    this.el.autoBoxAdjust = true;
    // if the element isn't positioned, make it relative
    if(this.el.getStyle('position') != 'absolute'){
        this.el.setStyle('position', 'relative');
    }
    
    // create the handles and proxy
    var dh = YAHOO.ext.DomHelper;
    var tpl = dh.createTemplate({tag: 'div', cls: 'yresizable-handle yresizable-handle-{0}', html: '&nbsp;'});
    this.east = getEl(tpl.append(this.el.dom, ['east']), true);
    this.south = getEl(tpl.append(this.el.dom, ['south']), true);
    if(config && config.multiDirectional){
        this.west = getEl(tpl.append(this.el.dom, ['west']), true);
        this.north = getEl(tpl.append(this.el.dom, ['north']), true);
    }
    this.corner = getEl(tpl.append(this.el.dom, ['southeast']), true);
    this.proxy = getEl(dh.insertBefore(document.body.firstChild, {tag: 'div', cls: 'yresizable-proxy', id: this.el.id + '-rzproxy'}), true);
    this.proxy.autoBoxAdjust = true;
    
    // wrapped event handlers to add and remove when sizing
    this.moveHandler = YAHOO.ext.EventManager.wrap(this.onMouseMove, this, true);
    this.upHandler = YAHOO.ext.EventManager.wrap(this.onMouseUp, this, true);
    this.selHandler = YAHOO.ext.EventManager.wrap(this.cancelSelection, this, true);
    
    // public events
    this.events = {
        'beforeresize' : new YAHOO.util.CustomEvent(),
        'resize' : new YAHOO.util.CustomEvent()
    };
    
    /** @private */
    this.dir = null;
    
    // properties
    this.resizeChild = false;
    this.adjustments = [0, 0];
    this.minWidth = 5;
    this.minHeight = 5;
    this.maxWidth = 10000;
    this.maxHeight = 10000;
    this.enabled = true;
    this.animate = false;
    this.duration = .35;
    this.dynamic = false;
    this.multiDirectional = false;
    this.disableTrackOver = false;
    this.easing = YAHOO.util.Easing ? YAHOO.util.Easing.easeOutStrong : null;
    
    YAHOO.ext.util.Config.apply(this, config);
    
    if(this.resizeChild){
        if(typeof this.resizeChild == 'boolean'){
            this.resizeChild = YAHOO.ext.Element.get(this.el.dom.firstChild, true);
        }else{
            this.resizeChild = YAHOO.ext.Element.get(this.resizeChild, true);
        }
    }
    
    // listen for mouse down on the handles
    var mdown = this.onMouseDown.createDelegate(this);
    this.east.mon('mousedown', mdown);
    this.south.mon('mousedown', mdown);
    if(this.multiDirectional){
        this.west.mon('mousedown', mdown);
        this.north.mon('mousedown', mdown);
    }
    this.corner.mon('mousedown', mdown);
    
    if(!this.disableTrackOver){
        // track mouse overs
        var mover = this.onMouseOver.createDelegate(this);
        // track mouse outs
        var mout = this.onMouseOut.createDelegate(this);
        
        this.east.mon('mouseover', mover);
        this.east.mon('mouseout', mout);
        this.south.mon('mouseover', mover);
        this.south.mon('mouseout', mout);
        if(this.multiDirectional){
            this.west.mon('mouseover', mover);
            this.west.mon('mouseout', mout);
            this.north.mon('mouseover', mover);
            this.north.mon('mouseout', mout);
        }
        this.corner.mon('mouseover', mover);
        this.corner.mon('mouseout', mout);
    }
    this.updateChildSize();
};

YAHOO.extendX(YAHOO.ext.Resizable, YAHOO.ext.util.Observable, {
    resizeTo : function(width, height){
        this.el.setSize(width, height);
        this.fireEvent('resize', this, width, height, null);
    },
    
    cancelSelection : function(e){
        e.preventDefault();
    },
    
    startSizing : function(e){
        this.fireEvent('beforeresize', this, e);
        if(this.enabled){ // 2nd enabled check in case disabled before beforeresize handler
            e.preventDefault();
            this.startBox = this.el.getBox();
            this.startPoint = e.getXY();
            this.offsets = [(this.startBox.x + this.startBox.width) - this.startPoint[0],
                            (this.startBox.y + this.startBox.height) - this.startPoint[1]];
            this.proxy.setBox(this.startBox);
            if(!this.dynamic){
                this.proxy.show();
            }
            YAHOO.util.Event.on(document.body, 'selectstart', this.selHandler);
            YAHOO.util.Event.on(document.body, 'mousemove', this.moveHandler);
            YAHOO.util.Event.on(document.body, 'mouseup', this.upHandler);
        }
    },
    
    onMouseDown : function(e){
        if(this.enabled){
            var t = e.getTarget();
            if(t == this.corner.dom){
                this.dir = 'both';
                this.proxy.setStyle('cursor', this.corner.getStyle('cursor'));
                this.startSizing(e);
            }else if(t == this.east.dom){
                this.dir = 'east';
                this.proxy.setStyle('cursor', this.east.getStyle('cursor'));
                this.startSizing(e);
            }else if(t == this.south.dom){
                this.dir = 'south';
                this.proxy.setStyle('cursor', this.south.getStyle('cursor'));
                this.startSizing(e);
            }else if(t == this.west.dom){
                this.dir = 'west';
                this.proxy.setStyle('cursor', this.west.getStyle('cursor'));
                this.startSizing(e);
            }else if(t == this.north.dom){
                this.dir = 'north';
                this.proxy.setStyle('cursor', this.north.getStyle('cursor'));
                this.startSizing(e);
            }
        }          
    },
    
    onMouseUp : function(e){
        YAHOO.util.Event.removeListener(document.body, 'selectstart', this.selHandler);
        YAHOO.util.Event.removeListener(document.body, 'mousemove', this.moveHandler);
        YAHOO.util.Event.removeListener(document.body, 'mouseup', this.upHandler);
        var size = this.resizeElement();
        this.fireEvent('resize', this, size.width, size.height, e);
    },
    
    updateChildSize : function(){
        if(this.resizeChild && this.el.dom.offsetWidth){
            var el = this.el;
            var child = this.resizeChild;
            var adj = this.adjustments;
            setTimeout(function(){
                var b = el.getBox(true);
                child.setSize(b.width+adj[0], b.height+adj[1]);
            }, 1);
        }
    },
    
    snap : function(value, inc){
        if(!inc || !value) return value;
        var newValue = value;
        var m = value % inc;
        if(m > 0){
            if(m > (inc/2)){
                newValue = value + (inc-m);
            }else{
                newValue = value - m;
            }
        }
        return newValue;
    },
    
    resizeElement : function(){
        var box = this.proxy.getBox();
        box.width = this.snap(box.width, this.widthIncrement);
        box.height = this.snap(box.height, this.heightIncrement);
        if(this.multiDirectional){
            this.el.setBox(box, false, this.animate, this.duration, null, this.easing);
        }else{
            this.el.setSize(box.width, box.height, this.animate, this.duration, null, this.easing);
        }
        this.updateChildSize();
        this.proxy.hide();
        return box;
    },
    
    onMouseMove : function(e){
        if(this.enabled){
            var xy = e.getXY();
            if(this.dir == 'both' || this.dir == 'east' || this.dir == 'south'){
                var w = Math.min(Math.max(this.minWidth, xy[0]-this.startBox.x+this.offsets[0]),this.maxWidth);
                var h = Math.min(Math.max(this.minHeight, xy[1]-this.startBox.y+this.offsets[1]), this.maxHeight);
                if(this.dir == 'both'){
                    this.proxy.setSize(w, h);
                }else if(this.dir == 'east'){
                    this.proxy.setWidth(w);
                }else if(this.dir == 'south'){
                    this.proxy.setHeight(h);
                }
            }else{
                var x = this.startBox.x + (xy[0]-this.startPoint[0]);
                var y = this.startBox.y + (xy[1]-this.startPoint[1]);
                var w = this.startBox.width+(this.startBox.x-x);
                var h = this.startBox.height+(this.startBox.y-y);
                if(this.dir == 'west' && w <= this.maxWidth && w >= this.minWidth){
                    this.proxy.setX(x);
                    this.proxy.setWidth(w);
                }else if(this.dir == 'north' && h <= this.maxHeight && h >= this.minHeight){
                    this.proxy.setY(y);
                    this.proxy.setHeight(h);
                }
            }
            if(this.dynamic){
                this.resizeElement();
            }
        }
    },
    
    onMouseOver : function(){
        if(this.enabled) this.el.addClass('yresizable-over');
    },
    
    onMouseOut : function(){
        this.el.removeClass('yresizable-over');
    }
});

/*
------------------------------------------------------------------
// File: widgets\SplitBar.js
------------------------------------------------------------------
*/
/*
 * splitbar.js, version .7
 * Copyright(c) 2006, Jack Slocum.
 * Code licensed under the BSD License
 */

YAHOO.util.DragDropMgr.clickTimeThresh = 350;

/**
 * @class Creates draggable splitter bar functionality from two elements.
 * <br><br>
 * Usage:
 * <pre><code>
 * var split = new YAHOO.ext.SplitBar('elementToDrag', 'elementToSize', 
 *                   YAHOO.ext.SplitBar.HORIZONTAL, YAHOO.ext.SplitBar.LEFT);
 * split.setAdapter(new YAHOO.ext.SplitBar.AbsoluteLayoutAdapter("container"));
 * split.minSize = 100;
 * split.maxSize = 600;
 * split.animate = true;
 * split.onMoved.subscribe(splitterMoved);
 * </code></pre>
 * @requires YAHOO.ext.Element
 * @requires YAHOO.util.Dom
 * @requires YAHOO.util.Event
 * @requires YAHOO.util.CustomEvent 
 * @requires YAHOO.util.DDProxy
 * @requires YAHOO.util.Anim (optional) to support animation
 * @requires YAHOO.util.Easing (optional) to support animation
 * @constructor
 * @param {String/HTMLElement/Element} dragElement The element to be dragged and act as the SplitBar. 
 * @param {String/HTMLElement/Element} resizingElement The element to be resized based on where the SplitBar element is dragged 
 * @param {Number} orientation (optional) Either YAHOO.ext.SplitBar.HORIZONTAL or YAHOO.ext.SplitBar.VERTICAL. (Defaults to HORIZONTAL)
 * @param {Number} placement (optional) Either YAHOO.ext.SplitBar.LEFT or YAHOO.ext.SplitBar.RIGHT for horizontal or  
                        YAHOO.ext.SplitBar.TOP or YAHOO.ext.SplitBar.BOTTOM for vertical. (By default, this is determined automatically by the intial position 
                        position of the SplitBar).
 */
YAHOO.ext.SplitBar = function(dragElement, resizingElement, orientation, placement){
    
    /** @private */
    this.el = YAHOO.ext.Element.get(dragElement, true);
    this.el.dom.unselectable = 'on';
    /** @private */
    this.resizingEl = YAHOO.ext.Element.get(resizingElement, true);
    
    /**
     * @private
     * The orientation of the split. Either YAHOO.ext.SplitBar.HORIZONTAL or YAHOO.ext.SplitBar.VERTICAL. (Defaults to HORIZONTAL)
     * Note: If this is changed after creating the SplitBar, the placement property must be manually updated
     * @type Number
     */
    this.orientation = orientation || YAHOO.ext.SplitBar.HORIZONTAL;
    
    /**
     * The minimum size of the resizing element. (Defaults to 0)
     * @type Number
     */
    this.minSize = 0;
    
    /**
     * The maximum size of the resizing element. (Defaults to 2000)
     * @type Number
     */
    this.maxSize = 2000;
    
    /**
     * Fires when the SplitBar is moved. Uses fireDirect with signature: (this, newSize)
     * @type CustomEvent
     */
    this.onMoved = new YAHOO.util.CustomEvent("SplitBarMoved", this);
    
    /**
     * Whether to animate the transition to the new size
     * @type Boolean
     */
    this.animate = false;
    
    /**
     * Whether to create a transparent shim that overlays the page when dragging, enables dragging across iframes.
     * @type Boolean
     */
    this.useShim = false;
    
    /** @private */
    this.shim = null;
    
    /** @private */
    this.proxy = YAHOO.ext.SplitBar.createProxy(this.orientation);
    
    /** @private */
    this.dd = new YAHOO.util.DDProxy(this.el.dom.id, "SplitBars", {dragElId : this.proxy.id});
    
    /** @private */
    this.dd.b4StartDrag = this.onStartProxyDrag.createDelegate(this);
    
    /** @private */
    this.dd.endDrag = this.onEndProxyDrag.createDelegate(this);
    
    /** @private */
    this.dragSpecs = {};
    
    /**
     * @private The adapter to use to positon and resize elements
     */
    this.adapter = new YAHOO.ext.SplitBar.BasicLayoutAdapter();
    this.adapter.init(this);
    
    if(this.orientation == YAHOO.ext.SplitBar.HORIZONTAL){
        /** @private */
        this.placement = placement || (this.el.getX() > this.resizingEl.getX() ? YAHOO.ext.SplitBar.LEFT : YAHOO.ext.SplitBar.RIGHT);
        this.el.setStyle('cursor', 'e-resize');
    }else{
        /** @private */
        this.placement = placement || (this.el.getY() > this.resizingEl.getY() ? YAHOO.ext.SplitBar.TOP : YAHOO.ext.SplitBar.BOTTOM);
        this.el.setStyle('cursor', 'n-resize');
    }
    
    this.events = {
        'resize' : this.onMoved,
        'moved' : this.onMoved,
        'beforeresize' : new YAHOO.util.CustomEvent('beforeresize')
    }
}

YAHOO.ext.SplitBar.prototype = {
    fireEvent : YAHOO.ext.util.Observable.prototype.fireEvent,
    on : YAHOO.ext.util.Observable.prototype.on,
    addListener : YAHOO.ext.util.Observable.prototype.addListener,
    delayedListener : YAHOO.ext.util.Observable.prototype.delayedListener,
    removeListener : YAHOO.ext.util.Observable.prototype.removeListener,
    /** 
     * @private Called before drag operation begins by the DDProxy
     */
    onStartProxyDrag : function(x, y){
        this.fireEvent('beforeresize', this);
        if(this.useShim){
            if(!this.shim){
                this.shim = YAHOO.ext.SplitBar.createShim();
            }
            this.shim.setVisible(true);
        }
        YAHOO.util.Dom.setStyle(this.proxy, 'display', 'block');
        var size = this.adapter.getElementSize(this);
        this.activeMinSize = this.getMinimumSize();;
        this.activeMaxSize = this.getMaximumSize();;
        var c1 = size - this.activeMinSize;
        var c2 = Math.max(this.activeMaxSize - size, 0);
        if(this.orientation == YAHOO.ext.SplitBar.HORIZONTAL){
            this.dd.resetConstraints();
            this.dd.setXConstraint(
                this.placement == YAHOO.ext.SplitBar.LEFT ? c1 : c2, 
                this.placement == YAHOO.ext.SplitBar.LEFT ? c2 : c1
            );
            this.dd.setYConstraint(0, 0);
        }else{
            this.dd.resetConstraints();
            this.dd.setXConstraint(0, 0);
            this.dd.setYConstraint(
                this.placement == YAHOO.ext.SplitBar.TOP ? c1 : c2, 
                this.placement == YAHOO.ext.SplitBar.TOP ? c2 : c1
            );
         }
        this.dragSpecs.startSize = size;
        this.dragSpecs.startPoint = [x, y];
        
        YAHOO.util.DDProxy.prototype.b4StartDrag.call(this.dd, x, y);
    },
    
    /** 
     * @private Called after the drag operation by the DDProxy
     */
    onEndProxyDrag : function(e){
        YAHOO.util.Dom.setStyle(this.proxy, 'display', 'none');
        var endPoint = YAHOO.util.Event.getXY(e);
        if(this.useShim){
            this.shim.setVisible(false);
        }
        var newSize;
        if(this.orientation == YAHOO.ext.SplitBar.HORIZONTAL){
            newSize = this.dragSpecs.startSize + 
                (this.placement == YAHOO.ext.SplitBar.LEFT ?
                    endPoint[0] - this.dragSpecs.startPoint[0] :
                    this.dragSpecs.startPoint[0] - endPoint[0]
                );
        }else{
            newSize = this.dragSpecs.startSize + 
                (this.placement == YAHOO.ext.SplitBar.TOP ?
                    endPoint[1] - this.dragSpecs.startPoint[1] :
                    this.dragSpecs.startPoint[1] - endPoint[1]
                );
        }
        newSize = Math.min(Math.max(newSize, this.activeMinSize), this.activeMaxSize);
        if(newSize != this.dragSpecs.startSize){
            this.adapter.setElementSize(this, newSize);
            this.onMoved.fireDirect(this, newSize);
        }
    },
    
    /**
     * Get the adapter this SplitBar uses
     * @return The adapter object
     */
    getAdapter : function(){
        return this.adapter;
    },
    
    /**
     * Set the adapter this SplitBar uses
     * @param {Object} adapter A SplitBar adapter object
     */
    setAdapter : function(adapter){
        this.adapter = adapter;
        this.adapter.init(this);
    },
    
    /**
     * Gets the minimum size for the resizing element
     * @return {Number} The minimum size
     */
    getMinimumSize : function(){
        return this.minSize;
    },
    
    /**
     * Sets the minimum size for the resizing element
     * @param {Number} minSize The minimum size
     */
    setMinimumSize : function(minSize){
        this.minSize = minSize;
    },
    
    /**
     * Gets the maximum size for the resizing element
     * @return {Number} The maximum size
     */
    getMaximumSize : function(){
        return this.maxSize;
    },
    
    /**
     * Sets the maximum size for the resizing element
     * @param {Number} maxSize The maximum size
     */
    setMaximumSize : function(maxSize){
        this.maxSize = maxSize;
    },
    
    /**
     * Sets the initialize size for the resizing element
     * @param {Number} size The initial size
     */
    setCurrentSize : function(size){
        var oldAnimate = this.animate;
        this.animate = false;
        this.adapter.setElementSize(this, size);
        this.animate = oldAnimate;
    }
};

/**
 * @private static Create the shim to drag over iframes
 */
YAHOO.ext.SplitBar.createShim = function(){
    var shim = document.createElement('div');
    YAHOO.util.Dom.generateId(shim, 'split-shim');
    YAHOO.util.Dom.setStyle(shim, 'width', '100%');
    YAHOO.util.Dom.setStyle(shim, 'height', '100%');
    YAHOO.util.Dom.setStyle(shim, 'position', 'absolute');
    YAHOO.util.Dom.setStyle(shim, 'background', 'white');
    YAHOO.util.Dom.setStyle(shim, 'z-index', 11000);
    shim.innerHTML = '&nbsp;';
    window.document.body.appendChild(shim);
    var shimEl = YAHOO.ext.Element.get(shim);
    shimEl.setOpacity(.01);
    shimEl.setXY([0, 0]);
    return shimEl;
};

/**
 * @private static Create our own proxy element element. So it will be the same same size on all browsers, we won't use borders. Instead we use a background color.
 */
YAHOO.ext.SplitBar.createProxy = function(orientation){
    var proxy = document.createElement('div');
    proxy.unselectable = 'on';
    YAHOO.util.Dom.generateId(proxy, 'split-proxy');
    YAHOO.util.Dom.setStyle(proxy, 'position', 'absolute');
    YAHOO.util.Dom.setStyle(proxy, 'visibility', 'hidden');
    YAHOO.util.Dom.setStyle(proxy, 'z-index', 11001);
    YAHOO.util.Dom.setStyle(proxy, 'background-color', "#aaa");
    if(orientation == YAHOO.ext.SplitBar.HORIZONTAL){
        YAHOO.util.Dom.setStyle(proxy, 'cursor', 'e-resize');
    }else{
        YAHOO.util.Dom.setStyle(proxy, 'cursor', 'n-resize');
    }
    // the next 2 fix IE abs position div height problem
    YAHOO.util.Dom.setStyle(proxy, 'line-height', '0px');
    YAHOO.util.Dom.setStyle(proxy, 'font-size', '0px');
    window.document.body.appendChild(proxy);
    return proxy;
};

/** 
 * @class
 * Default Adapter. It assumes the splitter and resizing element are not positioned
 * elements and only gets/sets the width of the element. Generally used for table based layouts.
 */
YAHOO.ext.SplitBar.BasicLayoutAdapter = function(){
};

YAHOO.ext.SplitBar.BasicLayoutAdapter.prototype = {
    // do nothing for now
    init : function(s){
    
    },
    /**
     * Called before drag operations to get the current size of the resizing element. 
     * @param {YAHOO.ext.SplitBar} s The SplitBar using this adapter
     */
     getElementSize : function(s){
        if(s.orientation == YAHOO.ext.SplitBar.HORIZONTAL){
            return s.resizingEl.getWidth();
        }else{
            return s.resizingEl.getHeight();
        }
    },
    
    /**
     * Called after drag operations to set the size of the resizing element.
     * @param {YAHOO.ext.SplitBar} s The SplitBar using this adapter
     * @param {Number} newSize The new size to set
     * @param {Function} onComplete A function to be invoke when resizing is complete
     */
    setElementSize : function(s, newSize, onComplete){
        if(s.orientation == YAHOO.ext.SplitBar.HORIZONTAL){
            if(!YAHOO.util.Anim || !s.animate){
                s.resizingEl.setWidth(newSize);
                if(onComplete){
                    onComplete(s, newSize);
                }
            }else{
                s.resizingEl.setWidth(newSize, true, .1, onComplete, YAHOO.util.Easing.easeOut);
            }
        }else{
            
            if(!YAHOO.util.Anim || !s.animate){
                s.resizingEl.setHeight(newSize);
                if(onComplete){
                    onComplete(s, newSize);
                }
            }else{
                s.resizingEl.setHeight(newSize, true, .1, onComplete, YAHOO.util.Easing.easeOut);
            }
        }
    }
};

/** 
 *@class
 * Adapter that  moves the splitter element to align with the resized sizing element. 
 * Used with an absolute positioned SplitBar.
 * @param {String/HTMLElement/Element} container The container that wraps around the absolute positioned content. If it's
 * document.body, make sure you assign an id to the body element.
 */
YAHOO.ext.SplitBar.AbsoluteLayoutAdapter = function(container){
    this.basic = new YAHOO.ext.SplitBar.BasicLayoutAdapter();
    this.container = getEl(container);
}

YAHOO.ext.SplitBar.AbsoluteLayoutAdapter.prototype = {
    init : function(s){
        this.basic.init(s);
        //YAHOO.util.Event.on(window, 'resize', this.moveSplitter.createDelegate(this, [s]));
    },
    
    getElementSize : function(s){
        return this.basic.getElementSize(s);
    },
    
    setElementSize : function(s, newSize, onComplete){
        this.basic.setElementSize(s, newSize, this.moveSplitter.createDelegate(this, [s]));
    },
    
    moveSplitter : function(s){
        var yes = YAHOO.ext.SplitBar;
        switch(s.placement){
            case yes.LEFT:
                s.el.setX(s.resizingEl.getRight());
                break;
            case yes.RIGHT:
                s.el.setStyle('right', (this.container.getWidth() - s.resizingEl.getLeft()) + 'px');
                break;
            case yes.TOP:
                s.el.setY(s.resizingEl.getBottom());
                break;
            case yes.BOTTOM:
                s.el.setY(s.resizingEl.getTop() - s.el.getHeight());
                break;
        }
    }
};

/**
 * Orientation constant - Create a vertical SplitBar
 * @type Number
 */
YAHOO.ext.SplitBar.VERTICAL = 1;

/**
 * Orientation constant - Create a horizontal SplitBar
 * @type Number
 */
YAHOO.ext.SplitBar.HORIZONTAL = 2;

/**
 * Placement constant - The resizing element is to the left of the splitter element
 * @type Number
 */
YAHOO.ext.SplitBar.LEFT = 1;

/**
 * Placement constant - The resizing element is to the right of the splitter element
 * @type Number
 */
YAHOO.ext.SplitBar.RIGHT = 2;

/**
 * Placement constant - The resizing element is positioned above the splitter element
 * @type Number
 */
YAHOO.ext.SplitBar.TOP = 3;

/**
 * Placement constant - The resizing element is positioned under splitter element
 * @type Number
 */
YAHOO.ext.SplitBar.BOTTOM = 4;


/*
------------------------------------------------------------------
// File: grid\Grid.js
------------------------------------------------------------------
*/
/**
 * @class
 * This class represents the primary interface of a component based grid control.
 * <br><br>Usage:<pre><code>
 * var grid = new YAHOO.ext.grid.Grid('my-container-id', dataModel, columnModel);
 * // set any options
 * grid.render();
 * </code></pre>
 * @requires YAHOO.util.Dom
 * @requires YAHOO.util.Event
 * @requires YAHOO.util.CustomEvent 
 * @requires YAHOO.ext.Element
 * @requires YAHOO.ext.util.Browser
 * @requires YAHOO.ext.util.CSS
 * @requires YAHOO.ext.SplitBar 
 * @requires YAHOO.ext.EventObject 
 * @constructor
 * @param {String/HTMLElement/YAHOO.ext.Element} container The element into which this grid will be rendered - 
 * The container MUST have some type of size defined for the grid to fill. The container will be 
 * automatically set to position relative if it isn't already.
 * @param {Object} dataModel The data model to bind to
 * @param {Object} colModel The column model with info about this grid's columns
 * @param {<i>Object</i>} selectionModel (optional) The selection model for this grid (defaults to DefaultSelectionModel)
 */
YAHOO.ext.grid.Grid = function(container, dataModel, colModel, selectionModel){
	/** @private */
	this.container = YAHOO.ext.Element.get(container);
	if(this.container.getStyle('position') != 'absolute'){
	    this.container.setStyle('position', 'relative');
	}
	//this.container.setStyle('overflow', 'hidden');
	/** @private */
	this.id = this.container.id;
	
    /** @private */
	this.rows = [];
    /** @private */
	this.rowCount = 0;
    /** @private */
	this.fieldId = null;
    /** @private */
	this.dataModel = dataModel;
    /** @private */
	this.colModel = colModel;
    /** @private */
	this.selModel = selectionModel;
	
	/** @private */
	this.activeEditor = null;
	
	/** @private */
	this.editingCell = null;
	
	/** The minimum width a column can be resized to. (Defaults to 25)
	 * @type Number */
	this.minColumnWidth = 25;
	
	/** True to automatically resize the columns to fit their content <b>on initial render</b>
	 * @type Boolean */
	this.autoSizeColumns = false;
	
	/** True to measure headers with column data when auto sizing columns
	 * @type Boolean */
	this.autoSizeHeaders = false;
	
	/**
	 * True to autoSize the grid when the window resizes - defaults to true
	 */
	this.monitorWindowResize = true;
	
	/** If autoSizeColumns is on, maxRowsToMeasure can be used to limit the number of
	 * rows measured to get a columns size - defaults to 0 (all rows).
	 * @type Number */
	this.maxRowsToMeasure = 0;
	
	/** True to highlight rows when the mouse is over (default is false)
	 * @type Boolean */
	this.trackMouseOver = false;
	
	/** True to enable drag and drop of rows
	 * @type Boolean */
	this.enableDragDrop = false;
	
	/** True to stripe the rows (default is true)
	 * @type Boolean */
	this.stripeRows = true;
	
	/** A regular expression defining tagNames 
     * allowed to have text selection (Defaults to <code>/INPUT|TEXTAREA/i</code>) */
    this.allowTextSelectionPattern = /INPUT|TEXTAREA|SELECT/i;
	
	/** @private */
	this.setValueDelegate = this.setCellValue.createDelegate(this);
	
	var CE = YAHOO.util.CustomEvent;
	/** @private */
	this.events = {
	    // raw events
	    'click' : new CE('click'),
	    'dblclick' : new CE('dblclick'),
	    'mousedown' : new CE('mousedown'),
	    'mouseup' : new CE('mouseup'),
	    'mouseover' : new CE('mouseover'),
	    'mouseout' : new CE('mouseout'),
	    'keypress' : new CE('keypress'),
	    'keydown' : new CE('keydown'),
	    // custom events
	    'cellclick' : new CE('cellclick'),
	    'celldblclick' : new CE('celldblclick'),
	    'rowclick' : new CE('rowclick'),
	    'rowdblclick' : new CE('rowdblclick'),
	    'headerclick' : new CE('headerclick'),
	    'rowcontextmenu' : new CE('rowcontextmenu'),
	    'headercontextmenu' : new CE('headercontextmenu'),
	    'beforeedit' : new CE('beforeedit'),
	    'afteredit' : new CE('afteredit'),
	    'bodyscroll' : new CE('bodyscroll'),
	    'columnresize' : new CE('columnresize'),
	    'startdrag' : new CE('startdrag'),
	    'enddrag' : new CE('enddrag'),
	    'dragdrop' : new CE('dragdrop'),
	    'dragover' : new CE('dragover'),
	    'dragenter' : new CE('dragenter'),
	    'dragout' : new CE('dragout')
	};
};

YAHOO.ext.grid.Grid.prototype = { 
    /**
     * Called once after all setup has been completed and the grid is ready to be rendered.
     */
    render : function(){
    	if(!this.view){
    	    if(this.dataModel.isPaged()){
    		    this.view = new YAHOO.ext.grid.PagedGridView();
    	    }else{
    	        this.view = new YAHOO.ext.grid.GridView();
    	    }
    	}
    	this.view.init(this);
        this.el = getEl(this.view.render(), true);
        var c = this.container;
        c.mon("click", this.onClick, this, true);
        c.mon("dblclick", this.onDblClick, this, true);
        c.mon("contextmenu", this.onContextMenu, this, true);
        c.mon("selectstart", this.cancelTextSelection, this, true);
        c.mon("mousedown", this.cancelTextSelection, this, true);
        c.mon("mousedown", this.onMouseDown, this, true);
        c.mon("mouseup", this.onMouseUp, this, true);
        if(this.trackMouseOver){
            this.el.mon("mouseover", this.onMouseOver, this, true);
            this.el.mon("mouseout", this.onMouseOut, this, true);
        }
        c.mon("keypress", this.onKeyPress, this, true);
        c.mon("keydown", this.onKeyDown, this, true);
        this.init();
    },
    
    setDataModel : function(dm, rerender){
        this.view.unplugDataModel(this.dataModel);
        this.dataModel = dm;
        this.view.plugDataModel(dm);
        if(rerender){
            dm.fireEvent('datachanged');
        }
    },
    
    /** @private */
    init : function(){
        this.rows = this.el.dom.rows;
        if(!this.disableSelection){
	        if(!this.selModel){
	            this.selModel = new YAHOO.ext.grid.DefaultSelectionModel(this);
	        }
	        this.selModel.init(this);
	        this.selModel.onSelectionChange.subscribe(this.updateField, this, true);
        }else{
            this.selModel = new YAHOO.ext.grid.DisableSelectionModel(this);
            this.selModel.init(this);
        }
        
        if(this.enableDragDrop){
            this.dd = new YAHOO.ext.grid.GridDD(this, this.container.dom);
        }
     },   

    /** @ignore */
    onMouseDown : function(e){
        this.fireEvent('mousedown', e);
    },
    
    /** @ignore */
    onMouseUp : function(e){
        this.fireEvent('mouseup', e);
    },
    
    /** @ignore */
    onMouseOver : function(e){
        this.fireEvent('mouseover', e);
    },
    
    /** @ignore */
    onMouseOut : function(e){
        this.fireEvent('mouseout', e);
    },
    
    /** @ignore */
    onKeyPress : function(e){
        this.fireEvent('keypress', e);
    },
    
    /** @ignore */
    onKeyDown : function(e){
        this.fireEvent('keydown', e);
    },
    
    /** Inherited from Observable */
    fireEvent : YAHOO.ext.util.Observable.prototype.fireEvent,
    /** Inherited from Observable */
    on : YAHOO.ext.util.Observable.prototype.on,
    /** Inherited from Observable */
    addListener : YAHOO.ext.util.Observable.prototype.addListener,
    /** Inherited from Observable */
    delayedListener : YAHOO.ext.util.Observable.prototype.delayedListener,
    /** Inherited from Observable */
    removeListener : YAHOO.ext.util.Observable.prototype.removeListener,
    
    /** @ignore */
    onClick : function(e){
        this.fireEvent('click', e);
        var target = e.getTarget();
        var row = this.getRowFromChild(target);
        var cell = this.getCellFromChild(target);
        var header = this.getHeaderFromChild(target);
        if(row){
            this.fireEvent('rowclick', this, row.rowIndex, e);
        }
        if(cell){
            this.fireEvent('cellclick', this, row.rowIndex, cell.columnIndex, e);
        }
        if(header){
            this.fireEvent('headerclick', this, header.columnIndex, e);
        }
    },

    /** @ignore */
    onContextMenu : function(e){
        var target = e.getTarget();
        var row = this.getRowFromChild(target);
        var header = this.getHeaderFromChild(target);
        if(row){
            this.fireEvent('rowcontextmenu', this, row.rowIndex, e);
        }
        if(header){
            this.fireEvent('headercontextmenu', this, header.columnIndex, e);
        }
        e.preventDefault();
    },

    /** @ignore */
    onDblClick : function(e){
        this.fireEvent('dblclick', e);
        var target = e.getTarget();
        var row = this.getRowFromChild(target);
        var cell = this.getCellFromChild(target);
        if(row){
            this.fireEvent('rowdblclick', this, row.rowIndex, e);
        }
        if(cell){
            this.fireEvent('celldblclick', this, row.rowIndex, cell.columnIndex, e);
        }
    },
    
    /**
     * Starts editing the specified for the specified row/column
     */
    startEditing : function(rowIndex, colIndex){
        var row = this.rows[rowIndex];
        var cell = row.childNodes[colIndex];
        this.stopEditing();
        setTimeout(this.doEdit.createDelegate(this, [row, cell]), 10);
    },
        
    /**
     * Stops any active editing
     */
    stopEditing : function(){
        if(this.activeEditor){
            this.activeEditor.stopEditing();
        }
    },
        
    /** @ignore */
    doEdit : function(row, cell){
        if(!row || !cell) return;
        var cm = this.colModel;
        var dm = this.dataModel;
        var colIndex = cell.columnIndex;
        var rowIndex = row.rowIndex;
        if(cm.isCellEditable(colIndex, rowIndex)){
           var ed = cm.getCellEditor(colIndex, rowIndex);
           if(ed){
               if(this.activeEditor){
                   this.activeEditor.stopEditing();
               }
               this.fireEvent('beforeedit', this, rowIndex, colIndex);
               this.activeEditor = ed;
               this.editingCell = cell;
               this.view.ensureVisible(row, true);
               try{
                   cell.focus();
               }catch(e){}
               ed.init(this, this.el.dom.parentNode, this.setValueDelegate);
               var value = dm.getValueAt(rowIndex, cm.getDataIndex(colIndex));
               // set timeout so firefox stops editing before starting a new edit
               setTimeout(ed.startEditing.createDelegate(ed, [value, row, cell]), 1);
           }   
        }  
    },
    
    setCellValue : function(value, rowIndex, colIndex){
         this.dataModel.setValueAt(value, rowIndex, this.colModel.getDataIndex(colIndex));
         this.fireEvent('afteredit', this, rowIndex, colIndex);
    },
    
    /** @ignore Called when text selection starts or mousedown to prevent default */
    cancelTextSelection : function(e){
        var target = e.getTarget();
        if(target && target != this.el.dom.parentNode && !this.allowTextSelectionPattern.test(target.tagName)){
            e.preventDefault();
        }
    },
    
    /**
     * Causes the grid to manually recalculate it's dimensions. Generally this is done automatically, 
     * but if manual update is required this method will initiate it.
     */
    autoSize : function(){
        this.view.updateWrapHeight();
        this.view.adjustForScroll();
    },
    
    /**
     * Scrolls the grid to the specified row
     * @param {Number/HTMLElement} row The row object or index of the row
     */
    scrollTo : function(row){
        if(typeof row == 'number'){
            row = this.rows[row];
        }
        this.view.ensureVisible(row, true);
    },
    
    /** @private */
    getEditingCell : function(){
        return this.editingCell;    
    },
    
    /**
     * Binds this grid to the field with the specified id. Initially reads and parses the comma 
     * delimited ids in the field and selects those items. All selections made in the grid
     * will be persisted to the field by their ids comma delimited.
     * @param {String} The id of the field to bind to
     */
    bindToField : function(fieldId){
        this.fieldId = fieldId;
        this.readField();
    },
    
    /** @private */
    updateField : function(){
        if(this.fieldId){
            var field = YAHOO.util.Dom.get(this.fieldId);
            field.value = this.getSelectedRowIds().join(',');
        }
    },
    
    /**
     * Causes the grid to read and select the ids from the bound field - See {@link #bindToField}.
     */
    readField : function(){
        if(this.fieldId){
            var field = YAHOO.util.Dom.get(this.fieldId);
            var values = field.value.split(',');
            var rows = this.getRowsById(values);
            this.selModel.selectRows(rows, false);
        }
    },
	
	/**
	 * Returns the table row at the specified index
	 * @return {HTMLElement} 
	 */
    getRow : function(index){
        return this.rows[index];
    },
	
	/**
	 * Returns the rows that have the specified id(s). The id value for a row is provided 
	 * by the DataModel. See {@link YAHOO.ext.grid.DefaultDataModel#getRowId}.
	 * @param {String/Array} An id to find or an array of ids
	 * @return {HtmlElement/Array} If one id was passed in, it returns one result. 
	 * If an array of ids was specified, it returns an Array of HTMLElements
	 */
    getRowsById : function(id){
        var dm = this.dataModel;
        if(!(id instanceof Array)){
            for(var i = 0; i < this.rows.length; i++){
                if(dm.getRowId(i) == id){
                    return this.rows[i];
                }
            }
            return null;
        }
        var found = [];
        var re = "^(?:";
        for(var i = 0; i < id.length; i++){
            re += id[i];
            if(i != id.length-1) re += "|";
        }
        var regex = new RegExp(re + ")$");
        for(var i = 0; i < this.rows.length; i++){
            if(regex.test(dm.getRowId(i))){
                found.push(this.rows[i]);
            }
        }
        return found;
    },
    
    /**
	 * Returns the row that comes after the specified row - text nodes are skipped.
	 * @param {HTMLElement} row
	 * @return {HTMLElement} 
	 */
    getRowAfter : function(row){
        return this.getSibling('next', row);
    },
    
    /**
	 * Returns the row that comes before the specified row - text nodes are skipped.
	 * @param {HTMLElement} row
	 * @return {HTMLElement} 
	 */
    getRowBefore : function(row){
        return this.getSibling('previous', row);
    },
    
    /**
	 * Returns the cell that comes after the specified cell - text nodes are skipped.
	 * @param {HTMLElement} cell
	 * @param {Boolean} includeHidden
	 * @return {HTMLElement} 
	 */
    getCellAfter : function(cell, includeHidden){
        var next = this.getSibling('next', cell);
        if(next && !includeHidden && this.colModel.isHidden(next.columnIndex)){
            return this.getCellAfter(next);
        }
        return next;
    },
    
    /**
	 * Returns the cell that comes before the specified cell - text nodes are skipped.
	 * @param {HTMLElement} cell
	 * @param {Boolean} includeHidden
	 * @return {HTMLElement} 
	 */
    getCellBefore : function(cell, includeHidden){
        var prev = this.getSibling('previous', cell);
        if(prev && !includeHidden && this.colModel.isHidden(prev.columnIndex)){
            return this.getCellBefore(prev);
        }
        return prev;
    },
    
    /**
	 * Returns the last cell for the row - text nodes and hidden columns are skipped.
	 * @param {HTMLElement} row
	 * @param {Boolean} includeHidden
	 * @return {HTMLElement} 
	 */
    getLastCell : function(row, includeHidden){
        var cell = this.getElement('previous', row.lastChild);
        if(cell && !includeHidden && this.colModel.isHidden(cell.columnIndex)){
            return this.getCellBefore(cell);
        }
        return cell;
    },
    
    /**
	 * Returns the first cell for the row - text nodes and hidden columns are skipped.
	 * @param {HTMLElement} row
	 * @param {Boolean} includeHidden
	 * @return {HTMLElement} 
	 */
    getFirstCell : function(row, includeHidden){
        var cell = this.getElement('next', row.firstChild);
        if(cell && !includeHidden && this.colModel.isHidden(cell.columnIndex)){
            return this.getCellAfter(cell);
        }
        return cell;
    },
    
    /**
     * Gets siblings, skipping text nodes
     * @param {String} type The direction to walk: 'next' or 'previous'
     * @private
     */
    getSibling : function(type, node){
        if(!node) return null;
        type += 'Sibling';
        var n = node[type];
        while(n && n.nodeType != 1){
            n = n[type];
        }
        return n;
    },
    
    /**
     * Returns node if node is an HTMLElement else walks the siblings in direction looking for 
     * a node that is an element
     * @param {String} direction The direction to walk: 'next' or 'previous'
     * @private
     */
    getElement : function(direction, node){
        if(!node || node.nodeType == 1) return node;
        else return this.getSibling(direction, node);
    },
    
    /**
     * @private
     */
    getElementFromChild : function(childEl, parentClass){
        if(!childEl || (YAHOO.util.Dom.hasClass(childEl, parentClass))){
		    return childEl;
	    }
	    var p = childEl.parentNode;
	    var b = document.body;
	    while(p && p != b){
            if(YAHOO.util.Dom.hasClass(p, parentClass)){
            	return p;
            }
            p = p.parentNode;
        }
	    return null;
    },
    
    /**
	 * Returns the row that contains the specified child element.
	 * @param {HTMLElement} childEl
	 * @return {HTMLElement} 
	 */
    getRowFromChild : function(childEl){
        return this.getElementFromChild(childEl, 'ygrid-row');
    },
    
    /**
	 * Returns the cell that contains the specified child element.
	 * @param {HTMLElement} childEl
	 * @return {HTMLElement} 
	 */
    getCellFromChild : function(childEl){
        return this.getElementFromChild(childEl, 'ygrid-col');
    },
    
    
    /**
     * Returns the header element that contains the specified child element.
     * @param {HTMLElement}  childEl
	 * @return {HTMLElement} 
	 */
     getHeaderFromChild : function(childEl){
        return this.getElementFromChild(childEl, 'ygrid-hd');
    },
    
    /**
     * Convenience method for getSelectionModel().getSelectedRows() - 
     * See <small>{@link YAHOO.ext.grid.DefaultSelectionModel#getSelectedRows}</small> for more details.
     */
    getSelectedRows : function(){
        return this.selModel.getSelectedRows();
    },
    
    /**
     * Convenience method for getSelectionModel().getSelectedRows()[0] - 
     * See <small>{@link YAHOO.ext.grid.DefaultSelectionModel#getSelectedRows}</small> for more details.
     */
    getSelectedRow : function(){
        if(this.selModel.hasSelection()){
            return this.selModel.getSelectedRows()[0];
        }
        return null;
    },
    
    /**
     * Get the selected row indexes
     * @return {Array} Array of indexes
     */
    getSelectedRowIndexes : function(){
        var a = [];
        var rows = this.selModel.getSelectedRows();
        for(var i = 0; i < rows.length; i++) {
        	a[i] = rows[i].rowIndex;
        }
        return a;
    },
    
    /**
     * Gets the first selected row or -1 if none are selected
     * @return {Number}
     */
    getSelectedRowIndex : function(){
        if(this.selModel.hasSelection()){
           return this.selModel.getSelectedRows()[0].rowIndex;
        }
        return -1;
    },
    
    /**
     * Convenience method for getSelectionModel().getSelectedRowIds()[0] - 
     * See <small>{@link YAHOO.ext.grid.DefaultSelectionModel#getSelectedRowIds}</small> for more details.
     */
    getSelectedRowId : function(){
        if(this.selModel.hasSelection()){
           return this.selModel.getSelectedRowIds()[0];
        }
        return null;
    },
    
    /**
     * Convenience method for getSelectionModel().getSelectedRowIds() - 
     * See <small>{@link YAHOO.ext.grid.DefaultSelectionModel#getSelectedRowIds}</small> for more details.
     */
    getSelectedRowIds : function(){
        return this.selModel.getSelectedRowIds();
    },
    
    /**
     * Convenience method for getSelectionModel().clearSelections() - 
     * See <small>{@link YAHOO.ext.grid.DefaultSelectionModel#clearSelections}</small> for more details.
     */
    clearSelections : function(){
        this.selModel.clearSelections();
    },
    
        
    /**
     * Convenience method for getSelectionModel().selectAll() - 
     * See <small>{@link YAHOO.ext.grid.DefaultSelectionModel#selectAll}</small> for more details.
     */
    selectAll : function(){
        this.selModel.selectAll();
    },
    
        
    /**
     * Convenience method for getSelectionModel().getCount() - 
     * See <small>{@link YAHOO.ext.grid.DefaultSelectionModel#getCount}</small> for more details.
     */
    getSelectionCount : function(){
        return this.selModel.getCount();
    },
    
    /**
     * Convenience method for getSelectionModel().hasSelection() - 
     * See <small>{@link YAHOO.ext.grid.DefaultSelectionModel#hasSelection}</small> for more details.
     */
    hasSelection : function(){
        return this.selModel.hasSelection();
    },
    
    /**
     * Returns the grid's SelectionModel.
     */
    getSelectionModel : function(){
        if(!this.selModel){
            this.selModel = new DefaultSelectionModel();
        }
        return this.selModel;
    },
    
    /**
     * Returns the grid's DataModel.
     */
    getDataModel : function(){
        return this.dataModel;
    },
    
    /**
     * Returns the grid's ColumnModel.
     */
    getColumnModel : function(){
        return this.colModel;
    },
    
    /**
     * Returns the grid's GridView object.
     */
    getView : function(){
        return this.view;
    },
    /**
     * Called to get grid's drag proxy text, by default returns this.ddText. 
     * @return {String}
     */
    getDragDropText : function(){
        return this.ddText.replace('%0', this.selModel.getCount());
    }
};
/**
 * Configures the text is the drag proxy (defaults to "%0 selected row(s)"). 
 * %0 is replaced with the number of selected rows.
 * @type String
 */
YAHOO.ext.grid.Grid.prototype.ddText = "%0 selected row(s)";

/*
------------------------------------------------------------------
// File: grid\GridDD.js
------------------------------------------------------------------
*/

// kill dependency issue
if(YAHOO.util.DDProxy){
/**
 * @class
 * Custom implementation of YAHOO.util.DDProxy used internally by the grid
 * @extends YAHOO.util.DDProxy
 */
YAHOO.ext.grid.GridDD = function(grid, bwrap){
    this.grid = grid;
    var ddproxy = document.createElement('div');
    ddproxy.id = grid.container.id + '-ddproxy';
    ddproxy.className = 'ygrid-drag-proxy';
    document.body.insertBefore(ddproxy, document.body.firstChild);
    YAHOO.util.Dom.setStyle(ddproxy, 'opacity', .80);
    var ddicon = document.createElement('span');
    ddicon.className = 'ygrid-drop-icon ygrid-drop-nodrop';
    ddproxy.appendChild(ddicon);
    var ddtext = document.createElement('span');
    ddtext.className = 'ygrid-drag-text';
    ddtext.innerHTML = "&nbsp;";
    ddproxy.appendChild(ddtext);
    
    this.ddproxy = ddproxy;
    this.ddtext = ddtext;
    this.ddicon = ddicon;
    YAHOO.util.Event.on(bwrap, 'click', this.handleClick, this, true);
    YAHOO.ext.grid.GridDD.superclass.constructor.call(this, bwrap.id, 'GridDD', 
          {dragElId : ddproxy.id, resizeFrame: false});
          
    this.unlockDelegate = grid.selModel.unlock.createDelegate(grid.selModel);
};
YAHOO.extendX(YAHOO.ext.grid.GridDD, YAHOO.util.DDProxy);

YAHOO.ext.grid.GridDD.prototype.handleMouseDown = function(e){
    var row = this.grid.getRowFromChild(YAHOO.util.Event.getTarget(e));
    if(!row) return;
    if(this.grid.selModel.isSelected(row)){
        YAHOO.ext.grid.GridDD.superclass.handleMouseDown.call(this, e);
    }else {
        this.grid.selModel.unlock();
        YAHOO.ext.EventObject.setEvent(e);
        this.grid.selModel.rowClick(this.grid, row.rowIndex, YAHOO.ext.EventObject);
        YAHOO.ext.grid.GridDD.superclass.handleMouseDown.call(this, e);
        this.grid.selModel.lock();
    }
};

YAHOO.ext.grid.GridDD.prototype.handleClick = function(e){
    if(this.grid.selModel.isLocked()){
         setTimeout(this.unlockDelegate, 1);
         YAHOO.util.Event.stopEvent(e);
    }
};

/**
 * Updates the DD visual element to allow/not allow a drop
 * @param {Boolean} dropStatus True if drop is allowed on the target
 */
YAHOO.ext.grid.GridDD.prototype.setDropStatus = function(dropStatus){
    if(dropStatus === true){
        YAHOO.util.Dom.replaceClass(this.ddicon, 'ygrid-drop-nodrop', 'ygrid-drop-ok');
    }else{
        YAHOO.util.Dom.replaceClass(this.ddicon, 'ygrid-drop-ok', 'ygrid-drop-nodrop');
    }
};

YAHOO.ext.grid.GridDD.prototype.startDrag = function(e){
    this.ddtext.innerHTML = this.grid.getDragDropText();
    this.setDropStatus(false);
    this.grid.selModel.lock();
    this.grid.fireEvent('startdrag', this.grid, this, e);
};
       
YAHOO.ext.grid.GridDD.prototype.endDrag = function(e){
    YAHOO.util.Dom.setStyle(this.ddproxy, 'visibility', 'hidden');
    this.grid.fireEvent('enddrag', this.grid, this, e);
};

YAHOO.ext.grid.GridDD.prototype.autoOffset = function(iPageX, iPageY) {
    this.setDelta(-12, -20);
};

YAHOO.ext.grid.GridDD.prototype.onDragEnter = function(e, id) {
    this.setDropStatus(true);
    this.grid.fireEvent('dragenter', this.grid, this, id, e);
};

YAHOO.ext.grid.GridDD.prototype.onDragDrop = function(e, id) {
    this.grid.fireEvent('dragdrop', this.grid, this, id, e);
};

YAHOO.ext.grid.GridDD.prototype.onDragOver = function(e, id) {
    this.grid.fireEvent('dragover', this.grid, this, id, e);
};

YAHOO.ext.grid.GridDD.prototype.onDragOut = function(e, id) {
    this.setDropStatus(false);
    this.grid.fireEvent('dragout', this.grid, this, id, e);
};
};

/*
------------------------------------------------------------------
// File: grid\GridView.js
------------------------------------------------------------------
*/
/**
 * @class
 * Default UI code used internally by the Grid. Documentation to come.
 * @constructor
 */
YAHOO.ext.grid.GridView = function(){
	/** @private */
	this.grid = null;
	
	/** @private */
	this.lastFocusedRow = null;
	/**
	 * Fires when the ViewPort is scrolled - fireDirect sig: (this, scrollLeft, scrollTop)
	 * @type YAHOO.util.CustomEvent
	 * @deprecated
	 */
	this.onScroll = new YAHOO.util.CustomEvent('onscroll');
	
	/**
	 * @private
	 */
	this.adjustScrollTask = new YAHOO.ext.util.DelayedTask(this._adjustForScroll, this);
	/**
	 * @private
	 */
	this.ensureVisibleTask = new YAHOO.ext.util.DelayedTask();
};

YAHOO.ext.grid.GridView.prototype = {
	init: function(grid){
		this.grid = grid;
	},
	
	fireScroll: function(scrollLeft, scrollTop){
		this.onScroll.fireDirect(this.grid, scrollLeft, scrollTop);
	},
	
	/**
	 * Utility method that gets an array of the cell renderers
	 */
	getColumnRenderers : function(){
    	var renderers = [];
    	var cm = this.grid.colModel;
        var colCount = cm.getColumnCount();
        for(var i = 0; i < colCount; i++){
            renderers.push(cm.getRenderer(i));
        }
        return renderers;
    },
    
    buildIndexMap : function(){
        var colToData = {};
        var dataToCol = {};
        var cm = this.grid.colModel;
        for(var i = 0, len = cm.getColumnCount(); i < len; i++){
            var di = cm.getDataIndex(i);
            colToData[i] = di;
            dataToCol[di] = i;
        }
        return {'colToData': colToData, 'dataToCol': dataToCol};
    },
    
    getDataIndexes : function(){
    	if(!this.indexMap){
            this.indexMap = this.buildIndexMap();
        }
        return this.indexMap.colToData;
    },
    
    getColumnIndexByDataIndex : function(dataIndex){
        if(!this.indexMap){
            this.indexMap = this.buildIndexMap();
        }
    	return this.indexMap.dataToCol[dataIndex];
    },
    
    updateHeaders : function(){
        var colModel = this.grid.colModel;
        var hcells = this.headers;
        var colCount = colModel.getColumnCount();
        for(var i = 0; i < colCount; i++){
            hcells[i].textNode.innerHTML = colModel.getColumnHeader(i);
        }
    },
    
    adjustForScroll : function(disableDelay){
        if(!disableDelay){
            this.adjustScrollTask.delay(50);
        }else{
            this._adjustForScroll();
        }
    },
    
    getCellAtPoint : function(x, y){
        var colIndex = null;        
        var rowIndex = null;
        
        // translate page coordinates to local coordinates
        var xy = YAHOO.util.Dom.getXY(this.wrap);
        x = (x - xy[0]) + this.wrap.scrollLeft;
        y = (y - xy[1]) + this.wrap.scrollTop;
        
        var colModel = this.grid.colModel;
        var pos = 0;
        var colCount = colModel.getColumnCount();
        for(var i = 0; i < colCount; i++){
            if(colModel.isHidden(i)) continue;
            var width = colModel.getColumnWidth(i);
            if(x >= pos && x < pos+width){
                colIndex = i;
                break;
            }
            pos += width;
        }
        if(colIndex != null){
            rowIndex = (y == 0 ? 0 : Math.floor(y / this.getRowHeight()));
            if(rowIndex >= this.grid.dataModel.getRowCount()){
                return null;
            }
            return [colIndex, rowIndex];
        }
        return null;
    },
    
    /** @private */
    _adjustForScroll : function(){
        this.forceScrollUpdate();
        if(this.scrollbarMode == YAHOO.ext.grid.GridView.SCROLLBARS_OVERLAP){
            var adjustment = 0;
            if(this.wrap.clientWidth && this.wrap.clientWidth != 0){
                adjustment = this.wrap.offsetWidth - this.wrap.clientWidth;
            }
            this.hwrap.setWidth(this.wrap.offsetWidth-adjustment);
        }else{
            this.hwrap.setWidth(this.wrap.offsetWidth);
        }
        this.bwrap.setWidth(Math.max(this.grid.colModel.getTotalWidth(), this.wrap.clientWidth));
    },

    focusRow : function(row){
        if(typeof row == 'number'){
            row = this.getBodyTable().childNodes[row];
        }
        if(!row) return;
    	var left = this.wrap.scrollLeft;
    	try{ // try catch for IE occasional focus bug
    	    row.childNodes.item(0).hideFocus = true;
        	row.childNodes.item(0).focus();
        }catch(e){}
        this.ensureVisible(row);
        this.wrap.scrollLeft = left;
        this.handleScroll();
        this.lastFocusedRow = row;
    },

    ensureVisible : function(row, disableDelay){
        if(!disableDelay){
            this.ensureVisibleTask.delay(50, this._ensureVisible, this, [row]);
        }else{
            this._ensureVisible(row);
        }
    },

    /** @ignore */
    _ensureVisible : function(row){
        if(typeof row == 'number'){
            row = this.getBodyTable().childNodes[row];
        }
        if(!row) return;
    	var left = this.wrap.scrollLeft;
    	var rowTop = parseInt(row.offsetTop, 10); // parseInt for safari bug
        var rowBottom = rowTop + row.offsetHeight;
        var clientTop = parseInt(this.wrap.scrollTop, 10); // parseInt for safari bug
        var clientBottom = clientTop + this.wrap.clientHeight;
        if(rowTop < clientTop){
        	this.wrap.scrollTop = rowTop;
        }else if(rowBottom > clientBottom){
            this.wrap.scrollTop = rowBottom-this.wrap.clientHeight;
        }
        this.wrap.scrollLeft = left;
        this.handleScroll();
    },
    
    updateColumns : function(){
        this.grid.stopEditing();
        var colModel = this.grid.colModel;
        var hcols = this.headers;
        var colCount = colModel.getColumnCount();
        var pos = 0;
        var totalWidth = colModel.getTotalWidth();
        for(var i = 0; i < colCount; i++){
            if(colModel.isHidden(i)) continue;
            var width = colModel.getColumnWidth(i);
            hcols[i].style.width = width + 'px';
            hcols[i].style.left = pos + 'px';
            hcols[i].split.style.left = (pos+width-3) + 'px';
            this.setCSSWidth(i, width, pos);
            pos += width;
        }
        this.lastWidth = totalWidth;
        this.bwrap.setWidth(Math.max(totalWidth, this.wrap.clientWidth));
        if(!YAHOO.ext.util.Browser.isIE){ // fix scrolling prob in gecko and opera
        	this.wrap.scrollLeft = this.hwrap.dom.scrollLeft;
        }
        this.syncScroll();
        this.forceScrollUpdate();
    },
    
    setCSSWidth : function(colIndex, width, pos){
        var selector = ["#" + this.grid.id + " .ygrid-col-" + colIndex, ".ygrid-col-" + colIndex];
        YAHOO.ext.util.CSS.updateRule(selector, 'width', width + 'px');
        if(typeof pos == 'number'){
            YAHOO.ext.util.CSS.updateRule(selector, 'left', pos + 'px');
        }
    },
    
    setCSSStyle : function(colIndex, name, value){
        var selector = ["#" + this.grid.id + " .ygrid-col-" + colIndex, ".ygrid-col-" + colIndex];
        YAHOO.ext.util.CSS.updateRule(selector, name, value);
    },
    
    handleHiddenChange : function(colModel, colIndex, hidden){
        if(hidden){
            this.hideColumn(colIndex);
        }else{
            this.unhideColumn(colIndex);
        }
        this.updateColumns();
    },
    
    hideColumn : function(colIndex){
        var selector = ["#" + this.grid.id + " .ygrid-col-" + colIndex, ".ygrid-col-" + colIndex];
        YAHOO.ext.util.CSS.updateRule(selector, 'position', 'absolute');
        YAHOO.ext.util.CSS.updateRule(selector, 'visibility', 'hidden');
        
        this.headers[colIndex].style.display = 'none';
        this.headers[colIndex].split.style.display = 'none';
    },
    
    unhideColumn : function(colIndex){
        var selector = ["#" + this.grid.id + " .ygrid-col-" + colIndex, ".ygrid-col-" + colIndex];
        YAHOO.ext.util.CSS.updateRule(selector, 'position', '');
        YAHOO.ext.util.CSS.updateRule(selector, 'visibility', 'visible');
        
        this.headers[colIndex].style.display = '';
        this.headers[colIndex].split.style.display = '';
    },
    
    getBodyTable : function(){
    	return this.bwrap.dom;
    },
    
    updateRowIndexes : function(firstRow, lastRow){
        var stripeRows = this.grid.stripeRows;
        var bt = this.getBodyTable();
        var nodes = bt.childNodes;
        firstRow = firstRow || 0;
        lastRow = lastRow || nodes.length-1;
        var re = /^(?:ygrid-row ygrid-row-alt|ygrid-row)/;
        for(var rowIndex = firstRow; rowIndex <= lastRow; rowIndex++){
            var node = nodes[rowIndex];
            if(stripeRows && (rowIndex+1) % 2 == 0){
        		node.className = node.className.replace(re, 'ygrid-row ygrid-row-alt');
        	}else{
        		node.className = node.className.replace(re, 'ygrid-row');
        	}
            node.rowIndex = rowIndex;
            nodes[rowIndex].style.top = (rowIndex * this.rowHeight) + 'px';
        }
    },

    insertRows : function(dataModel, firstRow, lastRow){
        this.updateBodyHeight();
        this.adjustForScroll(true);
        var renderers = this.getColumnRenderers();
        var dindexes = this.getDataIndexes();
        var colCount = this.grid.colModel.getColumnCount();
        var beforeRow = null;
        var bt = this.getBodyTable();
        if(firstRow < bt.childNodes.length){
            beforeRow = bt.childNodes[firstRow];
        }
        for(var rowIndex = firstRow; rowIndex <= lastRow; rowIndex++){
            var row = document.createElement('span');
            row.className = 'ygrid-row';
            row.style.top = (rowIndex * this.rowHeight) + 'px';
            this.renderRow(dataModel, row, rowIndex, colCount, renderers, dindexes);
            if(beforeRow){
            	bt.insertBefore(row, beforeRow);
            }else{
                bt.appendChild(row);
            }
        }
        this.updateRowIndexes(firstRow);
        this.adjustForScroll();
    },
    
    renderRow : function(dataModel, row, rowIndex, colCount, renderers, dindexes){
        for(var colIndex = 0; colIndex < colCount; colIndex++){
            var td = document.createElement('span');
            td.className = 'ygrid-col ygrid-col-' + colIndex + (colIndex == colCount-1 ? ' ygrid-col-last' : '');
            td.columnIndex = colIndex;
            td.tabIndex = 0;
            var span = document.createElement('span');
            span.className = 'ygrid-cell-text';
            td.appendChild(span);
            var val = renderers[colIndex](dataModel.getValueAt(rowIndex, dindexes[colIndex]), rowIndex, colIndex);
            if(typeof val == 'undefined' || val === '') val = '&nbsp;';
            span.innerHTML = val;
            row.appendChild(td);
        }
    },
    
    deleteRows : function(dataModel, firstRow, lastRow){
        this.updateBodyHeight();
        // first make sure they are deselected
        this.grid.selModel.deselectRange(firstRow, lastRow);
        var bt = this.getBodyTable();
        var rows = []; // get references because the rowIndex will change
        for(var rowIndex = firstRow; rowIndex <= lastRow; rowIndex++){
            rows.push(bt.childNodes[rowIndex]);
        }
        for(var i = 0; i < rows.length; i++){
            bt.removeChild(rows[i]);
            rows[i] = null;
        }
        rows = null;
        this.updateRowIndexes(firstRow);
        this.adjustForScroll();
    },
    
    updateRows : function(dataModel, firstRow, lastRow){
        var bt = this.getBodyTable();
        var dindexes = this.getDataIndexes();
        var renderers = this.getColumnRenderers();
        var colCount = this.grid.colModel.getColumnCount();
        for(var rowIndex = firstRow; rowIndex <= lastRow; rowIndex++){
            var row = bt.rows[rowIndex];
            var cells = row.childNodes;
            for(var colIndex = 0; colIndex < colCount; colIndex++){
                var td = cells[colIndex];
                var val = renderers[colIndex](dataModel.getValueAt(rowIndex, dindexes[colIndex]), rowIndex, colIndex);
                if(typeof val == 'undefined' || val === '') val = '&nbsp;';
                td.firstChild.innerHTML = val;
            }
        }
    },
    
    handleSort : function(dataModel, sortColumnIndex, sortDir, noRefresh){
        this.grid.selModel.syncSelectionsToIds();
        if(!noRefresh){
           this.updateRows(dataModel, 0, dataModel.getRowCount()-1);
        }
        this.updateHeaderSortState();
        if(this.lastFocusedRow){
            this.focusRow(this.lastFocusedRow);
        }
    },
    
    syncScroll : function(){
        this.hwrap.dom.scrollLeft = this.wrap.scrollLeft;
    },
    
    handleScroll : function(){
        this.syncScroll();
        this.fireScroll(this.wrap.scrollLeft, this.wrap.scrollTop);
        this.grid.fireEvent('bodyscroll', this.wrap.scrollLeft, this.wrap.scrollTop);
    },
    
    getRowHeight : function(){
        if(!this.rowHeight){
            var rule = YAHOO.ext.util.CSS.getRule(["#" + this.grid.id + " .ygrid-row", ".ygrid-row"]);
        	if(rule && rule.style.height){
        	    this.rowHeight = parseInt(rule.style.height, 10);
        	}else{
        	    this.rowHeight = 21;
        	}
        }
        return this.rowHeight;
    },
    
    renderRows : function(dataModel){
        if(this.grid.selModel){
            this.grid.selModel.clearSelections();
        }
    	var bt = this.getBodyTable();
    	bt.innerHTML = '';
    	this.rowHeight = this.getRowHeight();
    	this.insertRows(dataModel, 0, dataModel.getRowCount()-1);
    },
    
    updateCell : function(dataModel, rowIndex, dataIndex){
        var colIndex = this.getColumnIndexByDataIndex(dataIndex);
        if(typeof colIndex == 'undefined'){ // not present in grid
            return;
        }
        var bt = this.getBodyTable();
        var row = bt.childNodes[rowIndex];
        var cell = row.childNodes[colIndex];
        var renderer = this.grid.colModel.getRenderer(colIndex);
        var val = renderer(dataModel.getValueAt(rowIndex, dataIndex), rowIndex, colIndex);
        if(typeof val == 'undefined' || val === '') val = '&nbsp;';
        cell.firstChild.innerHTML = val;
    },
    
    calcColumnWidth : function(colIndex, maxRowsToMeasure){
        var maxWidth = 0;
        var bt = this.getBodyTable();
        var rows = bt.childNodes;
        var stopIndex = Math.min(maxRowsToMeasure || rows.length, rows.length);
        if(this.grid.autoSizeHeaders){
            var h = this.headers[colIndex];
            var curWidth = h.style.width;
            h.style.width = this.grid.minColumnWidth+'px';
            maxWidth = Math.max(maxWidth, h.scrollWidth);
            h.style.width = curWidth;
        }
        for(var i = 0; i < stopIndex; i++){
            var cell = rows[i].childNodes[colIndex].firstChild;
            maxWidth = Math.max(maxWidth, cell.scrollWidth);
        }
        return maxWidth + /*margin for error in IE*/ 5;
    },
    
    autoSizeColumn : function(colIndex, forceMinSize){
        if(forceMinSize){
           this.setCSSWidth(colIndex, this.grid.minColumnWidth);
        }
        var newWidth = this.calcColumnWidth(colIndex);
        this.grid.colModel.setColumnWidth(colIndex,
            Math.max(this.grid.minColumnWidth, newWidth));
        this.grid.fireEvent('columnresize', colIndex, newWidth);
    },
    
    autoSizeColumns : function(){
        var colModel = this.grid.colModel;
        var colCount = colModel.getColumnCount();
        var wrap = this.wrap;
        for(var i = 0; i < colCount; i++){
            this.setCSSWidth(i, this.grid.minColumnWidth);
            colModel.setColumnWidth(i, this.calcColumnWidth(i, this.grid.maxRowsToMeasure), true);
        }
        if(colModel.getTotalWidth() < wrap.clientWidth){
            var diff = Math.floor((wrap.clientWidth - colModel.getTotalWidth()) / colCount);
            for(var i = 0; i < colCount; i++){
                colModel.setColumnWidth(i, colModel.getColumnWidth(i) + diff, true);
            }
        }
        this.updateColumns();  
    },
    
    fitColumns : function(){
        var cm = this.grid.colModel;
        var colCount = cm.getColumnCount();
        var cols = [];
        var width = 0;
        var i, w;
        for (i = 0; i < colCount; i++){
            if(!cm.isHidden(i) && !cm.isFixed(i)){
                w = cm.getColumnWidth(i);
                cols.push(i);
                cols.push(w);
                width += w;
            }
        }
        var frac = (this.wrap.clientWidth - cm.getTotalWidth())/width;
        while (cols.length){
            w = cols.pop();
            i = cols.pop();
            cm.setColumnWidth(i, Math.floor(w + w*frac), true);
        }
        this.updateColumns();
    }, 
    
    onWindowResize : function(){
        if(this.grid.monitorWindowResize){
            this.updateWrapHeight();
            this.adjustForScroll();
        }  
    },
    
    updateWrapHeight : function(){
        this.grid.container.beginMeasure();
        var box = this.grid.container.getBox(true);
        this.wrapEl.setHeight(box.height-this.footerHeight-parseInt(this.wrap.offsetTop, 10));
        this.grid.container.endMeasure();
    },
    
    forceScrollUpdate : function(){
        var wrap = this.wrap;
        YAHOO.util.Dom.setStyle(wrap, 'width', (wrap.offsetWidth) +'px');
        setTimeout(function(){ // set timeout so FireFox works
            YAHOO.util.Dom.setStyle(wrap, 'width', '');
        }, 1);
    },
    
    updateHeaderSortState : function(){
        var state = this.grid.dataModel.getSortState();
        if(!state || typeof state.column == 'undefined') return;
        var sortColumn = this.getColumnIndexByDataIndex(state.column);
        var sortDir = state.direction;
        for(var i = 0, len = this.headers.length; i < len; i++){
            var h = this.headers[i];
            if(i != sortColumn){
                h.sortDesc.style.display = 'none';
                h.sortAsc.style.display = 'none';
            }else{
                h.sortDesc.style.display = sortDir == 'DESC' ? 'block' : 'none';
                h.sortAsc.style.display = sortDir == 'ASC' ? 'block' : 'none';
            }
        }
    },

    unplugDataModel : function(dm){
        dm.removeListener('cellupdated', this.updateCell, this);
        dm.removeListener('datachanged', this.renderRows, this);
        dm.removeListener('rowsdeleted', this.deleteRows, this);
        dm.removeListener('rowsinserted', this.insertRows, this);
        dm.removeListener('rowsupdated', this.updateRows, this);
        dm.removeListener('rowssorted', this.handleSort, this);
    },
    
    plugDataModel : function(dm){
        dm.on('cellupdated', this.updateCell, this, true);
        dm.on('datachanged', this.renderRows, this, true);
        dm.on('rowsdeleted', this.deleteRows, this, true);
        dm.on('rowsinserted', this.insertRows, this, true);
        dm.on('rowsupdated', this.updateRows, this, true);
        dm.on('rowssorted', this.handleSort, this, true);
    },
    
    render : function(){
        var grid = this.grid;
        var container = grid.container.dom;
        var dataModel = grid.dataModel;
        this.plugDataModel(dataModel);
    
        var colModel = grid.colModel;
        colModel.onWidthChange.subscribe(this.updateColumns, this, true);
        colModel.onHeaderChange.subscribe(this.updateHeaders, this, true);
        colModel.onHiddenChange.subscribe(this.handleHiddenChange, this, true);
        
        YAHOO.util.Event.on(window, 'resize', this.onWindowResize, this, true);
        
        var autoSizeDelegate = this.autoSizeColumn.createDelegate(this);
        
        var colCount = colModel.getColumnCount();
    
        var dh = YAHOO.ext.DomHelper;
        //create wrapper elements that handle offsets and scrolling
        var wrap = dh.append(container, {tag: 'div', cls: 'ygrid-wrap'});
        this.wrap = wrap;
        this.wrapEl = getEl(wrap, true);
        YAHOO.ext.EventManager.on(wrap, 'scroll', this.handleScroll, this, true);
        
        var hwrap = dh.append(container, {tag: 'div', cls: 'ygrid-wrap-headers'});
        this.hwrap = getEl(hwrap, true);
        
        var bwrap = dh.append(wrap, {tag: 'div', cls: 'ygrid-wrap-body', id: container.id + '-body'});
        this.bwrap = getEl(bwrap, true);
        this.bwrap.setWidth(colModel.getTotalWidth());
        bwrap.rows = bwrap.childNodes;
        
        this.footerHeight = 0;
        var foot = this.appendFooter(container);
        if(foot){
            this.footer = getEl(foot, true);
            this.footerHeight = this.footer.getHeight();
        }
        this.updateWrapHeight();
        
        var hrow = dh.append(hwrap, {tag: 'span', cls: 'ygrid-hrow'});
        this.hrow = hrow;
        
        if(!YAHOO.ext.util.Browser.isGecko){
            // IE doesn't like iframes, we will leave this alone
            var iframe = document.createElement('iframe');
            iframe.className = 'ygrid-hrow-frame';
            iframe.frameBorder = 0;
            iframe.src = YAHOO.ext.SSL_SECURE_URL;
            hwrap.appendChild(iframe);
        }
        this.headerCtrl = new YAHOO.ext.grid.HeaderController(this.grid);
        this.headers = [];
        this.cols = [];
        
        var htemplate = dh.createTemplate({
           tag: 'span', cls: 'ygrid-hd ygrid-header-{0}', children: [{
                tag: 'span', 
                cls: 'ygrid-hd-body', 
                html: '<table border="0" cellpadding="0" cellspacing="0">' +
                      '<tbody><tr><td><span>{1}</span></td>' +
                      '<td><span class="sort-desc"></span><span class="sort-asc"></span></td>' +
                      '</tr></tbody></table>'
           }]           
        });
        htemplate.compile();
        for(var i = 0; i < colCount; i++){
            var hd = htemplate.append(hrow, [i, colModel.getColumnHeader(i)]);
            var spans = hd.getElementsByTagName('span');
            hd.textNode = spans[1];
            hd.sortDesc = spans[2];
    	    hd.sortAsc = spans[3];
    	    hd.columnIndex = i;
            this.headers.push(hd);
            if(colModel.isSortable(i)){
                this.headerCtrl.register(hd);
            }
            var split = dh.append(hrow, {tag: 'span', cls: 'ygrid-hd-split'});
            hd.split = split;
        	
        	if(colModel.isResizable(i) && !colModel.isFixed(i)){
            	YAHOO.util.Event.on(split, 'dblclick', autoSizeDelegate.createCallback(i+0, true));
            	var sb = new YAHOO.ext.SplitBar(split, hd, null, YAHOO.ext.SplitBar.LEFT);
            	sb.columnIndex = i;
            	sb.minSize = grid.minColumnWidth;
            	sb.onMoved.subscribe(this.onColumnSplitterMoved, this, true);
            	YAHOO.util.Dom.addClass(sb.proxy, 'ygrid-column-sizer');
            	YAHOO.util.Dom.setStyle(sb.proxy, 'background-color', '');
            	sb.dd._resizeProxy = function(){
            	    var el = this.getDragEl();
            	    YAHOO.util.Dom.setStyle(el, 'height', (hwrap.clientHeight+wrap.clientHeight-2) +'px');
            	};
        	}else{
        	    split.style.cursor = 'default';
        	}
        }
       if(grid.autoSizeColumns){
            this.renderRows(dataModel);
            this.autoSizeColumns();
        }else{
            this.updateColumns();
            this.renderRows(dataModel);
        }
        
        for(var i = 0; i < colCount; i++){
            if(colModel.isHidden(i)){
                this.hideColumn(i);
            }
        }
        this.updateHeaderSortState();
        return this.bwrap;
    },
    
    onColumnSplitterMoved : function(splitter, newSize){
        this.grid.colModel.setColumnWidth(splitter.columnIndex, newSize);
        this.grid.fireEvent('columnresize', splitter.columnIndex, newSize);
    },
    
    appendFooter : function(parentEl){
        return null;  
    },
    
    updateBodyHeight : function(){
        YAHOO.util.Dom.setStyle(this.getBodyTable(), 'height', 
                             (this.grid.dataModel.getRowCount()*this.rowHeight)+'px');
    }
};
YAHOO.ext.grid.GridView.SCROLLBARS_UNDER = 0;
YAHOO.ext.grid.GridView.SCROLLBARS_OVERLAP = 1;
YAHOO.ext.grid.GridView.prototype.scrollbarMode = YAHOO.ext.grid.GridView.SCROLLBARS_UNDER;

YAHOO.ext.grid.GridView.prototype.fitColumnsToContainer = YAHOO.ext.grid.GridView.prototype.fitColumns;

/**
 * @class
 * Used internal by GridView to route header related events.
 * @constructor
 */
YAHOO.ext.grid.HeaderController = function(grid){
	/** @private */
	this.grid = grid;
	/** @private */
	this.headers = [];
};

YAHOO.ext.grid.HeaderController.prototype = {
	register : function(header){
		this.headers.push(header);
		YAHOO.ext.EventManager.on(header, 'selectstart', this.cancelTextSelection, this, true);
        YAHOO.ext.EventManager.on(header, 'mousedown', this.cancelTextSelection, this, true);
        YAHOO.ext.EventManager.on(header, 'mouseover', this.headerOver, this, true);
        YAHOO.ext.EventManager.on(header, 'mouseout', this.headerOut, this, true);
        YAHOO.ext.EventManager.on(header, 'click', this.headerClick, this, true);
	},
	
	headerClick : function(e){
	    var grid = this.grid, cm = grid.colModel, dm = grid.dataModel;
	    grid.stopEditing();
        var header = grid.getHeaderFromChild(e.getTarget());
        var state = dm.getSortState();
        var direction = header.sortDir || 'ASC';
        if(typeof state.column != 'undefined' && 
                 grid.getView().getColumnIndexByDataIndex(state.column) == header.columnIndex){
            direction = (state.direction == 'ASC' ? 'DESC' : 'ASC');
        }
        header.sortDir = direction;
        dm.sort(cm, cm.getDataIndex(header.columnIndex), direction);
    },
    
    headerOver : function(e){
        var header = this.grid.getHeaderFromChild(e.getTarget());
        YAHOO.util.Dom.addClass(header, 'ygrid-hd-over');
        //YAHOO.ext.util.CSS.applyFirst(header, this.grid.id, '.ygrid-hd-over');
    },
    
    headerOut : function(e){
        var header = this.grid.getHeaderFromChild(e.getTarget());
        YAHOO.util.Dom.removeClass(header, 'ygrid-hd-over');
        //YAHOO.ext.util.CSS.revertFirst(header, this.grid.id, '.ygrid-hd-over');
    },
    
    cancelTextSelection : function(e){
    	e.preventDefault();
    }
};

/*
------------------------------------------------------------------
// File: grid\PagedGridView.js
------------------------------------------------------------------
*/

YAHOO.ext.grid.PagedGridView = function(){
    YAHOO.ext.grid.PagedGridView.superclass.constructor.call(this);
    this.cursor = 1;
};
YAHOO.extendX(YAHOO.ext.grid.PagedGridView, YAHOO.ext.grid.GridView);

YAHOO.ext.grid.PagedGridView.prototype.appendFooter = function(parentEl){
    var fwrap = document.createElement('div');
    fwrap.className = 'ygrid-wrap-footer';
    var fbody = document.createElement('span');
    fbody.className = 'ygrid-footer';
    fwrap.appendChild(fbody);
    parentEl.appendChild(fwrap);
    this.createPagingToolbar(fbody);
    return fwrap;
};
YAHOO.ext.grid.PagedGridView.prototype.createPagingToolbar = function(container){
    var tb = new YAHOO.ext.Toolbar(container);
    this.pageToolbar = tb;
    this.first = tb.addButton({
        tooltip: this.firstText, 
        className: 'ygrid-page-first',
        disabled: true,
        click: this.onClick.createDelegate(this, ['first'])
    });
    this.prev = tb.addButton({
        tooltip: this.prevText, 
        className: 'ygrid-page-prev', 
        disabled: true,
        click: this.onClick.createDelegate(this, ['prev'])
    });
    tb.addSeparator();
    tb.add(this.beforePageText);
    var pageBox = document.createElement('input');
    pageBox.type = 'text';
    pageBox.size = 3;
    pageBox.value = '1';
    pageBox.className = 'ygrid-page-number';
    tb.add(pageBox);
    this.field = getEl(pageBox, true);
    this.field.mon('keydown', this.onEnter, this, true);
    this.field.on('focus', function(){pageBox.select();});
    this.afterTextEl = tb.addText(this.afterPageText.replace('%0', '1'));
    this.field.setHeight(18);
    tb.addSeparator();
    this.next = tb.addButton({
        tooltip: this.nextText, 
        className: 'ygrid-page-next', 
        disabled: true,
        click: this.onClick.createDelegate(this, ['next'])
    });
    this.last = tb.addButton({
        tooltip: this.lastText, 
        className: 'ygrid-page-last', 
        disabled: true,
        click: this.onClick.createDelegate(this, ['last'])
    });
    tb.addSeparator();
    this.loading = tb.addButton({
        tooltip: this.refreshText, 
        className: 'ygrid-loading',
        disabled: true,
        click: this.onClick.createDelegate(this, ['refresh'])
    });
    this.onPageLoaded(1, this.grid.dataModel.getTotalPages());
};
YAHOO.ext.grid.PagedGridView.prototype.getPageToolbar = function(){
    return this.pageToolbar;  
};

YAHOO.ext.grid.PagedGridView.prototype.onPageLoaded = function(pageNum, totalPages){
    this.cursor = pageNum;
    this.lastPage = totalPages;
    this.afterTextEl.innerHTML = this.afterPageText.replace('%0', totalPages);
    this.field.dom.value = pageNum;
    this.first.setDisabled(pageNum == 1);
    this.prev.setDisabled(pageNum == 1);
    this.next.setDisabled(pageNum == totalPages);
    this.last.setDisabled(pageNum == totalPages);
    this.loading.enable();
};

YAHOO.ext.grid.PagedGridView.prototype.onEnter = function(e){
    if(e.browserEvent.keyCode == e.RETURN){
        var v = this.field.dom.value;
        if(!v){
            this.field.dom.value = this.cursor;
            return;
        }
        var pageNum = parseInt(v, 10);
        if(isNaN(v)){
            this.field.dom.value = this.cursor;
            return;
        }
        pageNum = Math.min(Math.max(1, pageNum), this.lastPage);
        this.grid.dataModel.loadPage(pageNum);
        e.stopEvent();
    }
};

YAHOO.ext.grid.PagedGridView.prototype.beforeLoad = function(){
    if(this.loading){
        this.loading.disable();
    }  
};

YAHOO.ext.grid.PagedGridView.prototype.onClick = function(which){
    switch(which){
        case 'first':
            this.grid.dataModel.loadPage(1);
        break;
        case 'prev':
            this.grid.dataModel.loadPage(this.cursor -1);
        break;
        case 'next':
            this.grid.dataModel.loadPage(this.cursor + 1);
        break;
        case 'last':
            this.grid.dataModel.loadPage(this.lastPage);
        break;
        case 'refresh':
            this.grid.dataModel.loadPage(this.cursor);
        break;
    }
};

YAHOO.ext.grid.PagedGridView.prototype.unplugDataModel = function(dm){
    dm.removeListener('beforeload', this.beforeLoad, this);
    dm.removeListener('load', this.onPageLoaded, this);
    YAHOO.ext.grid.PagedGridView.superclass.unplugDataModel.call(this, dm);
};

YAHOO.ext.grid.PagedGridView.prototype.plugDataModel = function(dm){
    dm.on('beforeload', this.beforeLoad, this, true);
    dm.on('load', this.onPageLoaded, this, true);
    YAHOO.ext.grid.PagedGridView.superclass.plugDataModel.call(this, dm);
};

YAHOO.ext.grid.PagedGridView.prototype.beforePageText = "Page";
YAHOO.ext.grid.PagedGridView.prototype.afterPageText = "of %0";
YAHOO.ext.grid.PagedGridView.prototype.firstText = "First Page";
YAHOO.ext.grid.PagedGridView.prototype.prevText = "Previous Page";
YAHOO.ext.grid.PagedGridView.prototype.nextText = "Next Page";
YAHOO.ext.grid.PagedGridView.prototype.lastText = "Last Page";
YAHOO.ext.grid.PagedGridView.prototype.refreshText = "Refresh";


/*
------------------------------------------------------------------
// File: grid\EditorGrid.js
------------------------------------------------------------------
*/

YAHOO.ext.grid.EditorGrid = function(container, dataModel, colModel){
    YAHOO.ext.grid.EditorGrid.superclass.constructor.call(this, container, dataModel, 
                      colModel, new YAHOO.ext.grid.EditorSelectionModel());
    this.container.addClass('yeditgrid');
};
YAHOO.extendX(YAHOO.ext.grid.EditorGrid, YAHOO.ext.grid.Grid);

/*
------------------------------------------------------------------
// File: grid\AbstractColumnModel.js
------------------------------------------------------------------
*/
/**
 * @class
 * This abstract class defines the ColumnModel interface and provides default implementations of the events required by the Grid. 
 * @constructor
*/
YAHOO.ext.grid.AbstractColumnModel = function(){
	/** Fires when a column width is changed - fireDirect sig: (this, columnIndex, newWidth)
     * @type YAHOO.util.CustomEvent 
     * */
    this.onWidthChange = new YAHOO.util.CustomEvent('widthChanged');
    /** Fires when a header has changed - fireDirect sig: (this, columnIndex, newHeader)
     * @type YAHOO.util.CustomEvent 
     * */
    this.onHeaderChange = new YAHOO.util.CustomEvent('headerChanged');
	/** Fires when a column is hidden or unhidden - fireDirect sig: (this, columnIndex, hidden)
     * @type YAHOO.util.CustomEvent 
     * */
    this.onHiddenChange = new YAHOO.util.CustomEvent('hiddenChanged');
    
    this.events = {
        'widthchange': this.onWidthChange,
        'headerchange': this.onHeaderChange,
        'hiddenchange': this.onHiddenChange
    };
};

YAHOO.ext.grid.AbstractColumnModel.prototype = {
	/** Inherited from Observable */
    fireEvent : YAHOO.ext.util.Observable.prototype.fireEvent,
    /** Inherited from Observable */
    on : YAHOO.ext.util.Observable.prototype.on,
    /** Inherited from Observable */
    addListener : YAHOO.ext.util.Observable.prototype.addListener,
    /** Inherited from Observable */
    delayedListener : YAHOO.ext.util.Observable.prototype.delayedListener,
    /** Inherited from Observable */
    removeListener : YAHOO.ext.util.Observable.prototype.removeListener,
    
    fireWidthChange : function(colIndex, newWidth){
		this.onWidthChange.fireDirect(this, colIndex, newWidth);
	},
	
	fireHeaderChange : function(colIndex, newHeader){
		this.onHeaderChange.fireDirect(this, colIndex, newHeader);
	},
	
	fireHiddenChange : function(colIndex, hidden){
		this.onHiddenChange.fireDirect(this, colIndex, hidden);
	},
	
	/**
     * Interface method - Returns the number of columns.
     * @return {Number}
     */
    getColumnCount : function(){
        return 0;
    },
    
    /**
     * Interface method - Returns true if the specified column is sortable.
     * @param {Number} col The column index
     * @return {Boolean}
     */
    isSortable : function(col){
        return false;
    },
    
    /**
     * Interface method - Returns true if the specified column is hidden.
     * @param {Number} col The column index
     * @return {Boolean}
     */
    isHidden : function(col){
        return false;
    },
    
    /**
     * Interface method - Returns the sorting comparison function defined for the column (defaults to sortTypes.none).
     * @param {Number} col The column index
     * @return {Function}
     */
    getSortType : function(col){
        return YAHOO.ext.grid.DefaultColumnModel.sortTypes.none;
    },
    
    /**
     * Interface method - Returns the rendering (formatting) function defined for the column.
     * @param {Number} col The column index
     * @return {Function}
     */
    getRenderer : function(col){
        return YAHOO.ext.grid.DefaultColumnModel.defaultRenderer;
    },
    
    /**
     * Interface method - Returns the width for the specified column.
     * @param {Number} col The column index
     * @return {Number}
     */
    getColumnWidth : function(col){
        return 0;
    },
    
    /**
     * Interface method - Returns the total width of all columns.
     * @return {Number}
     */
    getTotalWidth : function(){
        return 0;
    },
    
    /**
     * Interface method - Returns the header for the specified column.
     * @param {Number} col The column index
     * @return {String}
     */
    getColumnHeader : function(col){
        return '';
    }
};


/*
------------------------------------------------------------------
// File: grid\DefaultColumnModel.js
------------------------------------------------------------------
*/

/**
 * @class
 * This is the default implementation of a ColumnModel used by the Grid. It defines
 * the columns in the grid.
 * <br>Usage:<br>
 * <pre><code>
 * var sort = YAHOO.ext.grid.DefaultColumnModel.sortTypes;
 * var myColumns = [
	{header: "Ticker", width: 60, sortable: true, sortType: sort.asUCString}, 
	{header: "Company Name", width: 150, sortable: true, sortType: sort.asUCString}, 
	{header: "Market Cap.", width: 100, sortable: true, sortType: sort.asFloat}, 
	{header: "$ Sales", width: 100, sortable: true, sortType: sort.asFloat, renderer: money}, 
	{header: "Employees", width: 100, sortable: true, sortType: sort.asFloat}
 * ];
 * var colModel = new YAHOO.ext.grid.DefaultColumnModel(myColumns);
 * </code></pre>
 * @extends YAHOO.ext.grid.AbstractColumnModel
 * @constructor
*/
YAHOO.ext.grid.DefaultColumnModel = function(config){
	YAHOO.ext.grid.DefaultColumnModel.superclass.constructor.call(this);
    /**
     * The config passed into the constructor
     */
    this.config = config;
    
    /**
     * The width of columns which have no width specified (defaults to 100)
     * @type Number
     */
    this.defaultWidth = 100;
    /**
     * Default sortable of columns which have no sortable specified (defaults to false)
     * @type Boolean
     */
    this.defaultSortable = false;
};
YAHOO.extendX(YAHOO.ext.grid.DefaultColumnModel, YAHOO.ext.grid.AbstractColumnModel);

/**
 * Returns the number of columns.
 * @return {Number}
 */
YAHOO.ext.grid.DefaultColumnModel.prototype.getColumnCount = function(){
    return this.config.length;
};
    
/**
 * Returns true if the specified column is sortable.
 * @param {Number} col The column index
 * @return {Boolean}
 */
YAHOO.ext.grid.DefaultColumnModel.prototype.isSortable = function(col){
    if(typeof this.config[col].sortable == 'undefined'){
        return this.defaultSortable;
    }
    return this.config[col].sortable;
};
    
/**
 * Returns the sorting comparison function defined for the column (defaults to sortTypes.none).
 * @param {Number} col The column index
 * @return {Function}
 */
YAHOO.ext.grid.DefaultColumnModel.prototype.getSortType = function(col){
    if(!this.dataMap){
        // build a lookup so we don't search every time
        var map = [];
        for(var i = 0, len = this.config.length; i < len; i++){
            map[this.getDataIndex(i)] = i;
        }
        this.dataMap = map;
    }
    col = this.dataMap[col];
    if(!this.config[col].sortType){
        return YAHOO.ext.grid.DefaultColumnModel.sortTypes.none;
    }
    return this.config[col].sortType;
};
    
/**
 * Sets the sorting comparison function for a column.
 * @param {Number} col The column index
 * @param {Function} fn
 */
YAHOO.ext.grid.DefaultColumnModel.prototype.setSortType = function(col, fn){
    this.config[col].sortType = fn;
};
    

/**
 * Returns the rendering (formatting) function defined for the column.
 * @param {Number} col The column index
 * @return {Function}
 */
YAHOO.ext.grid.DefaultColumnModel.prototype.getRenderer = function(col){
    if(!this.config[col].renderer){
        return YAHOO.ext.grid.DefaultColumnModel.defaultRenderer;
    }
    return this.config[col].renderer;
};
    
/**
 * Sets the rendering (formatting) function for a column.
 * @param {Number} col The column index
 * @param {Function} fn
 */
YAHOO.ext.grid.DefaultColumnModel.prototype.setRenderer = function(col, fn){
    this.config[col].renderer = fn;
};
    
/**
 * Returns the width for the specified column.
 * @param {Number} col The column index
 * @return {Number}
 */
YAHOO.ext.grid.DefaultColumnModel.prototype.getColumnWidth = function(col){
    return this.config[col].width || this.defaultWidth;
};
    
/**
 * Sets the width for a column.
 * @param {Number} col The column index
 * @param {Number} width The new width
 */
YAHOO.ext.grid.DefaultColumnModel.prototype.setColumnWidth = function(col, width, suppressEvent){
    this.config[col].width = width;
    this.totalWidth = null;
    if(!suppressEvent){
         this.onWidthChange.fireDirect(this, col, width);
    }
};
    
/**
 * Returns the total width of all columns.
 * @param {Boolean} includeHidden True to include hidden column widths
 * @return {Number}
 */
YAHOO.ext.grid.DefaultColumnModel.prototype.getTotalWidth = function(includeHidden){
    if(!this.totalWidth){
        this.totalWidth = 0;
        for(var i = 0; i < this.config.length; i++){
            if(includeHidden || !this.isHidden(i)){
                this.totalWidth += this.getColumnWidth(i);
            }
        }
    }
    return this.totalWidth;
};
    
/**
 * Returns the header for the specified column.
 * @param {Number} col The column index
 * @return {String}
 */
YAHOO.ext.grid.DefaultColumnModel.prototype.getColumnHeader = function(col){
    return this.config[col].header;
};
     
/**
 * Sets the header for a column.
 * @param {Number} col The column index
 * @param {String} header The new header
 */
YAHOO.ext.grid.DefaultColumnModel.prototype.setColumnHeader = function(col, header){
    this.config[col].header = header;
    this.onHeaderChange.fireDirect(this, col, header);
};
/**
 * Returns the dataIndex for the specified column.
 * @param {Number} col The column index
 * @return {Number}
 */
YAHOO.ext.grid.DefaultColumnModel.prototype.getDataIndex = function(col){
    if(typeof this.config[col].dataIndex != 'number'){
        return col;
    }
    return this.config[col].dataIndex;
};
     
/**
 * Sets the dataIndex for a column.
 * @param {Number} col The column index
 * @param {Number} dataIndex The new dataIndex
 */
YAHOO.ext.grid.DefaultColumnModel.prototype.setDataIndex = function(col, dataIndex){
    this.config[col].dataIndex = dataIndex;
};
/**
 * Returns true if the cell is editable.
 * @param {Number} colIndex The column index
 * @param {Number} rowIndex The row index
 * @return {Boolean}
 */
YAHOO.ext.grid.DefaultColumnModel.prototype.isCellEditable = function(colIndex, rowIndex){
    return this.config[colIndex].editable || (typeof this.config[colIndex].editable == 'undefined' && this.config[colIndex].editor);
};

/**
 * Returns the editor defined for the cell/column.
 * @param {Number} colIndex The column index
 * @param {Number} rowIndex The row index
 * @return {Object}
 */
YAHOO.ext.grid.DefaultColumnModel.prototype.getCellEditor = function(colIndex, rowIndex){
    return this.config[colIndex].editor;
};
   
/**
 * Sets if a column is editable.
 * @param {Number} col The column index
 * @param {Boolean} editable True if the column is editable
 */
YAHOO.ext.grid.DefaultColumnModel.prototype.setEditable = function(col, editable){
    this.config[col].editable = editable;
};


/**
 * Returns true if the column is hidden.
 * @param {Number} colIndex The column index
 * @return {Boolean}
 */
YAHOO.ext.grid.DefaultColumnModel.prototype.isHidden = function(colIndex){
    return this.config[colIndex].hidden;
};


/**
 * Returns true if the column width cannot be changed
 */
YAHOO.ext.grid.DefaultColumnModel.prototype.isFixed = function(colIndex){
    return this.config[colIndex].fixed;
};

/**
 * Returns true if the column cannot be resized
 */
YAHOO.ext.grid.DefaultColumnModel.prototype.isResizable = function(colIndex){
    return this.config[colIndex].resizable !== false;
};
/**
 * Sets if a column is hidden.
 * @param {Number} colIndex The column index
 */
YAHOO.ext.grid.DefaultColumnModel.prototype.setHidden = function(colIndex, hidden){
    this.config[colIndex].hidden = hidden;
    this.totalWidth = null;
    this.fireHiddenChange(colIndex, hidden);
};

/**
 * Sets the editor for a column.
 * @param {Number} col The column index
 * @param {Object} editor The editor object
 */
YAHOO.ext.grid.DefaultColumnModel.prototype.setEditor = function(col, editor){
    this.config[col].editor = editor;
};


/**
 * Default empty rendering function
 */
YAHOO.ext.grid.DefaultColumnModel.defaultRenderer = function(value){
	if(typeof value == 'string' && value.length < 1){
	    return '&nbsp;';
	}
	return value;
}

/**
 * Defines the default sorting (casting?) comparison functions used when sorting data:
 * <br>&nbsp;&nbsp;sortTypes.none - sorts data as it is without casting or parsing (the default)
 * <br>&nbsp;&nbsp;sortTypes.asUCString - case insensitive string
 * <br>&nbsp;&nbsp;sortTypes.asDate - attempts to parse data as a date
 * <br>&nbsp;&nbsp;sortTypes.asFloat
 * <br>&nbsp;&nbsp;sortTypes.asInt
 */
YAHOO.ext.grid.DefaultColumnModel.sortTypes = {};

YAHOO.ext.grid.DefaultColumnModel.sortTypes.none = function(s) {
	return s;
};

YAHOO.ext.grid.DefaultColumnModel.sortTypes.asUCString = function(s) {
	return String(s).toUpperCase();
};

YAHOO.ext.grid.DefaultColumnModel.sortTypes.asDate = function(s) {
    if(s instanceof Date){
        return s.getTime();
    }
	return Date.parse(String(s));
};

YAHOO.ext.grid.DefaultColumnModel.sortTypes.asFloat = function(s) {
	var val = parseFloat(String(s).replace(/,/g, ''));
    if(isNaN(val)) val = 0;
	return val;
};

YAHOO.ext.grid.DefaultColumnModel.sortTypes.asInt = function(s) {
    var val = parseInt(String(s).replace(/,/g, ''));
    if(isNaN(val)) val = 0;
	return val;
};

/*
------------------------------------------------------------------
// File: data\AbstractDataModel.js
------------------------------------------------------------------
*/
/**
 * @class
 * This abstract class provides default implementations of the events required by the Grid. 
 It takes care of the creating the CustomEvents and provides some convenient methods for firing the events. <br><br>
 * @constructor
*/
YAHOO.ext.grid.AbstractDataModel = function(){
    /** Fires when a cell is updated - fireDirect sig: (this, rowIndex, columnIndex)
     * @type YAHOO.util.CustomEvent
     * @deprecated Use addListener instead of accessing directly
     */
    this.onCellUpdated = new YAHOO.util.CustomEvent('onCellUpdated');
    /** Fires when all data needs to be revalidated - fireDirect sig: (thisd)
     * @type YAHOO.util.CustomEvent 
     * @deprecated Use addListener instead of accessing directly
     */
    this.onTableDataChanged = new YAHOO.util.CustomEvent('onTableDataChanged');
    /** Fires when rows are deleted - fireDirect sig: (this, firstRowIndex, lastRowIndex)
     * @type YAHOO.util.CustomEvent 
     * @deprecated Use addListener instead of accessing directly
     */
    this.onRowsDeleted = new YAHOO.util.CustomEvent('onRowsDeleted');
    /** Fires when a rows are inserted - fireDirect sig: (this, firstRowIndex, lastRowIndex)
     * @type YAHOO.util.CustomEvent 
     * @deprecated Use addListener instead of accessing directly
     */
    this.onRowsInserted = new YAHOO.util.CustomEvent('onRowsInserted');
    /** Fires when a rows are updated - fireDirect sig: (this, firstRowIndex, lastRowIndex)
     * @type YAHOO.util.CustomEvent 
     * @deprecated Use addListener instead of accessing directly
     */
    this.onRowsUpdated = new YAHOO.util.CustomEvent('onRowsUpdated');
    /** Fires when a sort has reordered the rows - fireDirect sig: (this, sortColumnIndex, 
     * sortDirection = 'ASC' or 'DESC')
     * @type YAHOO.util.CustomEvent 
     * @deprecated Use addListener instead of accessing directly
     */
    this.onRowsSorted = new YAHOO.util.CustomEvent('onRowsSorted');
    
    this.events = {
      'cellupdated' : this.onCellUpdated,
      'datachanged' : this.onTableDataChanged,
      'rowsdeleted' : this.onRowsDeleted,
      'rowsinserted' : this.onRowsInserted,
      'rowsupdated' : this.onRowsUpdated,
      'rowssorted' : this.onRowsSorted
    };
};

YAHOO.ext.grid.AbstractDataModel.prototype = {
    
    /** Inherited from Observable */
    fireEvent : YAHOO.ext.util.Observable.prototype.fireEvent,
    /** Inherited from Observable */
    on : YAHOO.ext.util.Observable.prototype.on,
    /** Inherited from Observable */
    addListener : YAHOO.ext.util.Observable.prototype.addListener,
    /** Inherited from Observable */
    delayedListener : YAHOO.ext.util.Observable.prototype.delayedListener,
    /** Inherited from Observable */
    removeListener : YAHOO.ext.util.Observable.prototype.removeListener,
    
    /**
     *  Notifies listeners that the value of the cell at [row, col] has been updated
     */
    fireCellUpdated : function(row, col){
        this.onCellUpdated.fireDirect(this, row, col);
    },
    
    /**
     *  Notifies listeners that all data for the grid may have changed - use as a last resort. This 
     * also wipes out all selections a user might have made.
     */
    fireTableDataChanged : function(){
        this.onTableDataChanged.fireDirect(this);
    },
    
    /**
     *  Notifies listeners that rows in the range [firstRow, lastRow], inclusive, have been deleted
     */
    fireRowsDeleted : function(firstRow, lastRow){
        this.onRowsDeleted.fireDirect(this, firstRow, lastRow);
    },
    
    /**
     *  Notifies listeners that rows in the range [firstRow, lastRow], inclusive, have been inserted
     */
    fireRowsInserted : function(firstRow, lastRow){
        this.onRowsInserted.fireDirect(this, firstRow, lastRow);
    },
    
    /**
     *  Notifies listeners that rows in the range [firstRow, lastRow], inclusive, have been updated
     */
    fireRowsUpdated : function(firstRow, lastRow){
        this.onRowsUpdated.fireDirect(this, firstRow, lastRow);
    },
    
    /**
     *  Notifies listeners that rows have been sorted and any indexes may be invalid
     */
    fireRowsSorted : function(sortColumnIndex, sortDir, noRefresh){
        this.onRowsSorted.fireDirect(this, sortColumnIndex, sortDir, noRefresh);
    },
    
    /**
     * Empty interface method - Classes which extend AbstractDataModel should implement this method.
     * See {@link YAHOO.ext.DefaultDataModel} for an example implementation.
     */
    sort : function(sortInfo, columnIndex, direction, suppressEvent){
    	
    },
    
    /**
     * Interface method to supply the view with info regarding the Grid's current sort state - if overridden,
     * this should return an object like this {column: this.sortColumn, direction: this.sortDir}.
     * @return {Object} 
     */
    getSortState : function(){
    	return {column: this.sortColumn, direction: this.sortDir};
    },
    
    /**
     * Empty interface method - Classes which extend AbstractDataModel should implement this method.
     * See {@link YAHOO.ext.DefaultDataModel} for an example implementation.
     */
    getRowCount : function(){
    	
    },
    
    /**
     * Empty interface method - Classes which extend AbstractDataModel should implement this method to support virtual row counts.
     */
    getTotalRowCount : function(){
    	return this.getRowCount();
    },
    
    
    /**
     * Empty interface method - Classes which extend AbstractDataModel should implement this method.
     * See {@link YAHOO.ext.DefaultDataModel} for an example implementation.
     */
    getRowId : function(rowIndex){
    	
    },
    
    /**
     * Empty interface method - Classes which extend AbstractDataModel should implement this method.
     * See {@link YAHOO.ext.DefaultDataModel} for an example implementation.
     */
    getValueAt : function(rowIndex, colIndex){
    	
    },
    
    /**
     * Empty interface method - Classes which extend AbstractDataModel should implement this method.
     * See {@link YAHOO.ext.DefaultDataModel} for an example implementation.
     */
    setValueAt : function(value, rowIndex, colIndex){
    	
    },
    
    isPaged : function(){
        return false;
    }
};

/*
------------------------------------------------------------------
// File: data\DefaultDataModel.js
------------------------------------------------------------------
*/

/**
 * @class
 * This is the default implementation of a DataModel used by the Grid. It works 
 * with multi-dimensional array based data. Using the event system in the base class 
 * {@link YAHOO.ext.grid.AbstractDataModel}, all updates to this DataModel are automatically
 * reflected in the user interface.
 * <br>Usage:<br>
 * <pre><code>
 * var myData = [
	["MSFT","Microsoft Corporation", "314,571.156", "32,187.000", "55000"],
	["ORCL", "Oracle Corporation", "62,615.266", "9,519.000", "40650"]
 * ];
 * var dataModel = new YAHOO.ext.grid.DefaultDataModel(myData);
 * </code></pre>
 * @extends YAHOO.ext.grid.AbstractDataModel
 * @constructor
*/
YAHOO.ext.grid.DefaultDataModel = function(data){
    YAHOO.ext.grid.DefaultDataModel.superclass.constructor.call(this);
    /**@private*/
    this.data = data;
};
YAHOO.extendX(YAHOO.ext.grid.DefaultDataModel, YAHOO.ext.grid.AbstractDataModel);

/**
 * Returns the number of rows in the dataset
 * @return {Number}
 */
YAHOO.ext.grid.DefaultDataModel.prototype.getRowCount = function(){
    return this.data.length;
};
    
/**
 * Returns the ID of the specified row. By default it return the value of the first column. 
 * Override to provide more advanced ID handling. 
 * @return {Number}
 */
YAHOO.ext.grid.DefaultDataModel.prototype.getRowId = function(rowIndex){
    return this.data[rowIndex][0];
};

/**
 * Returns the column data for the specified row. 
 * @return {Array}
 */
YAHOO.ext.grid.DefaultDataModel.prototype.getRow = function(rowIndex){
    return this.data[rowIndex];
};

/**
 * Returns the column data for the specified rows as a 
 * multi-dimensional array: rows[3][0] would give you the value of row 4, column 0. 
 * @param {Array} indexes The row indexes to fetch
 * @return {Array}
 */
YAHOO.ext.grid.DefaultDataModel.prototype.getRows = function(indexes){
    var data = this.data;
    var r = [];
    for(var i = 0; i < indexes.length; i++){
       r.push(data[indexes[i]]);
    }
    return r;
};

/**
 * Returns the value at the specified data position
 * @param {Number} rowIndex
 * @param {Number} colIndex
 * @return {Object}
 */
YAHOO.ext.grid.DefaultDataModel.prototype.getValueAt = function(rowIndex, colIndex){
	return this.data[rowIndex][colIndex];
};

/**
 * Sets the specified value at the specified data position
 * @param {Object} value The new value
 * @param {Number} rowIndex
 * @param {Number} colIndex
 */
YAHOO.ext.grid.DefaultDataModel.prototype.setValueAt = function(value, rowIndex, colIndex){
    this.data[rowIndex][colIndex] = value;
    this.fireCellUpdated(rowIndex, colIndex);
};

/**
 * @private
 * Removes the specified range of rows.
 * @param {Number} startIndex
 * @param {<i>Number</i>} endIndex (optional) Defaults to startIndex
 */
YAHOO.ext.grid.DefaultDataModel.prototype.removeRows = function(startIndex, endIndex){
    endIndex = endIndex || startIndex;
    this.data.splice(startIndex, endIndex-startIndex+1);
    this.fireRowsDeleted(startIndex, endIndex);
};

/**
 * Remove a row.
 * @param {Number} index
 */
YAHOO.ext.grid.DefaultDataModel.prototype.removeRow = function(index){
    this.data.splice(index, 1);
    this.fireRowsDeleted(index, index);
};

/**
 * @private
 * Removes all rows.
 */
YAHOO.ext.grid.DefaultDataModel.prototype.removeAll = function(){
	var count = this.getRowCount();
	if(count > 0){
    	this.removeRows(0, count-1);
	}
};

/**
 * Query the DataModel rows by the filters defined in spec, for example...
 * <pre><code>
 * // column 1 starts with Jack, column 2 filtered by myFcn, column 3 equals 'Fred'
 * dataModel.filter({1: /^Jack.+/i}, 2: myFcn, 3: 'Fred'});
 * </code></pre> 
 * @param {Object} spec The spec is generally an object literal consisting of
 * column index and filter type. The filter type can be a string/number (exact match),
 * a regular expression to test using String.search() or a function to call. If it's a function, 
 * it will be called with the value for the specified column and an array of the all column 
 * values for that row: yourFcn(value, columnData). If it returns anything other than true, 
 * the row is not a match. If you have modified Object.prototype this method may fail.
 * @param {Boolean} returnUnmatched True to return rows which <b>don't</b> match the query instead
 * of rows that do match
 * @return {Array} An array of row indexes that match
 */
YAHOO.ext.grid.DefaultDataModel.prototype.query = function(spec, returnUnmatched){
    var d = this.data;
    var r = [];
    for(var i = 0; i < d.length; i++){
        var row = d[i];
        var isMatch = true;
        for(var col in spec){
            //if(typeof spec[col] != 'function'){
                if(!isMatch) continue;
                var filter = spec[col];
                switch(typeof filter){
                    case 'string':
                    case 'number':
                    case 'boolean':
                      if(row[col] != filter){
                          isMatch = false;
                      }
                    break;
                    case 'function':
                      if(!filter(row[col], row)){
                          isMatch = false;
                      }
                    break;
                    case 'object':
                       if(filter instanceof RegExp){
                           if(String(row[col]).search(filter) === -1){
                               isMatch = false;
                           }
                       }
                    break;
                }
            //}
        }
        if(isMatch && !returnUnmatched){
            r.push(i);
        }else if(!isMatch && returnUnmatched){
            r.push(i);
        }
    }
    return r;
};

/**
 * Filter the DataModel rows by the query defined in spec, see {@link #query} for more details 
 * on the query spec.
 * @param {Object} query The query spec {@link #query}
 * @return {Number} The number of rows removed
 */
YAHOO.ext.grid.DefaultDataModel.prototype.filter = function(query){
    var matches = this.query(query, true);
    var data = this.data;
    // go thru the data setting matches to deleted
    // while not disturbing row indexes
    for(var i = 0; i < matches.length; i++){ 
        data[matches[i]]._deleted = true;
    }
    for(var i = 0; i < data.length; i++){
        while(data[i] && data[i]._deleted === true){
            this.removeRow(i);
        }
    }
    return matches.length;
};

/**
 * Adds a row to the dataset.
 * @param {Array} cellValues The array of values for the new row
 * @return {Number} The index of the added row
 */
YAHOO.ext.grid.DefaultDataModel.prototype.addRow = function(cellValues){
    this.data.push(cellValues);
    var newIndex = this.data.length-1;
    this.fireRowsInserted(newIndex, newIndex);
    this.applySort();
    return newIndex;
};

/**
 * @private
 * Adds a set of rows.
 * @param {Array} rowData This should be an array of arrays like the constructor takes
 */
YAHOO.ext.grid.DefaultDataModel.prototype.addRows = function(rowData){
    this.data = this.data.concat(rowData);
    var firstIndex = this.data.length-rowData.length;
    this.fireRowsInserted(firstIndex, firstIndex+rowData.length-1);
    this.applySort();
};

/**
 * Inserts a row a the specified location in the dataset.
 * @param {Number} index The index where the row should be inserted
 * @param {Array} cellValues The array of values for the new row
 * @return {Number} The index the row was inserted in
 */
YAHOO.ext.grid.DefaultDataModel.prototype.insertRow = function(index, cellValues){
    this.data.splice(index, 0, cellValues);
    this.fireRowsInserted(index, index);
    this.applySort();
    return index;
};

/**
 * @private
 * Inserts a set of rows.
 * @param {Number} index The index where the rows should be inserted
 * @param {Array} rowData This should be an array of arrays like the constructor takes
 */
YAHOO.ext.grid.DefaultDataModel.prototype.insertRows = function(index, rowData){
    /*
    if(index == this.data.length){ // try these two first since they are faster
        this.data = this.data.concat(rowData);
    }else if(index == 0){
        this.data = rowData.concat(this.data);
    }else{
        var newData = this.data.slice(0, index);
        newData.concat(rowData);
        newData.concat(this.data.slice(index));
        this.data = newData;
    }*/
    var args = rowData.concat();
    args.splice(0, 0, index, 0);
    this.data.splice.apply(this.data, args);
    this.fireRowsInserted(index, index+rowData.length-1);
    this.applySort();
};

/**
 * Applies the last used sort to the current data.
 */
YAHOO.ext.grid.DefaultDataModel.prototype.applySort = function(suppressEvent){
	if(typeof this.sortColumn != 'undefined'){
		this.sort(this.sortInfo, this.sortColumn, this.sortDir, suppressEvent);
	}
};

YAHOO.ext.grid.DefaultDataModel.prototype.setDefaultSort = function(sortInfo, columnIndex, direction){
    this.sortInfo = sortInfo;
    this.sortColumn = columnIndex;
    this.sortDir = direction;
};
/**
 * Sorts the data by the specified column - Uses the sortType specified for the column in the passed columnModel.
 * @param {Function/Object} sortInfo A sort comparison function or null to use the default or A object that has a method getSortType(index) that returns a function like 
 *                                               a grid column model. 
 * @param {Number} columnIndex The column index to sort by
 * @param {String} direction The direction of the sort ('DESC' or 'ASC')
 */
YAHOO.ext.grid.DefaultDataModel.prototype.sort = function(sortInfo, columnIndex, direction, suppressEvent){
    // store these so we can maintain sorting when we load new data
    this.sortInfo = sortInfo;
    this.sortColumn = columnIndex;
    this.sortDir = direction;
    
    var dsc = (direction && direction.toUpperCase() == 'DESC');
    var sortType = null;
    if(sortInfo != null){
        if(typeof sortInfo == 'function'){
            sortType = sortInfo;
        }else if(typeof sortInfo == 'object'){
            sortType = sortInfo.getSortType(columnIndex);;
        }
    }
    var fn = function(cells, cells2){
        var v1 = sortType ? sortType(cells[columnIndex], cells) : cells[columnIndex];
        var v2 = sortType ? sortType(cells2[columnIndex], cells2) : cells2[columnIndex];
        if(v1 < v2)
			return dsc ? +1 : -1;
		if(v1 > v2)
			return dsc ? -1 : +1;
	    return 0;
    };
    this.data.sort(fn);
    if(!suppressEvent){
       this.fireRowsSorted(columnIndex, direction);
    }
};

/**
 * Calls passed function with each rows data - if the function returns false it stops.
 */ 
YAHOO.ext.grid.DefaultDataModel.prototype.each = function(fn, scope){
    var d = this.data;
    for(var i = 0, len = d.length; i < len; i++){
        if(fn.call(scope || window, d[i], i) === false) break;
    }
};

/**
 * Alias to YAHOO.ext.grid.DefaultColumnModel.sortTypes
 */
YAHOO.ext.grid.DefaultDataModel.sortTypes = YAHOO.ext.grid.DefaultColumnModel.sortTypes;

/*
------------------------------------------------------------------
// File: data\LoadableDataModel.js
------------------------------------------------------------------
*/
/**
 * @class
 * This class extends DefaultDataModel and adds the core functionality to load data remotely. <br><br>
 * @extends YAHOO.ext.grid.DefaultDataModel
 * @constructor
 * @param {String} dataType YAHOO.ext.grid.LoadableDataModel.XML, YAHOO.ext.grid.LoadableDataModel.TEXT or YAHOO.ext.grid.JSON
*/
YAHOO.ext.grid.LoadableDataModel = function(dataType){
    YAHOO.ext.grid.LoadableDataModel.superclass.constructor.call(this, []);
    
    /** Fires when a successful load is completed - fireDirect sig: (this)
     * @type YAHOO.util.CustomEvent 
     * @deprecated Use addListener instead of accessing directly
     */
    this.onLoad = new YAHOO.util.CustomEvent('load');
    /** Fires when a load fails - fireDirect sig: (this, errorMsg, responseObj)
     * @type YAHOO.util.CustomEvent 
     * @deprecated Use addListener instead of accessing directly
     */
    this.onLoadException = new YAHOO.util.CustomEvent('loadException');
    
    this.events['load'] = this.onLoad;
    this.events['beforeload'] = new YAHOO.util.CustomEvent('beforeload');
    this.events['loadexception'] = this.onLoadException;
    
    /**@private*/
    this.dataType = dataType;
    /**@private*/
    this.preprocessors = [];
    /**@private*/
    this.postprocessors = [];
    
    // paging info
    /** The active page @type Number*/
    this.loadedPage = 1;
    /** True to use remote sorting, initPaging automatically sets this to true @type Boolean */
    this.remoteSort = false;
    /** The number of records per page @type Number*/
    this.pageSize = 0;
    /** The script/page to call to provide paged/sorted data @type String*/
    this.pageUrl = null;
    /** An object of key/value pairs to be passed as parameters
     * when loading pages/sorting @type Object*/
    this.baseParams = {};
    /** Maps named params to url parameters - Override to specify your own param names */
    this.paramMap = {'page':'page', 'pageSize':'pageSize', 'sortColumn':'sortColumn', 'sortDir':'sortDir'};
    
};
YAHOO.extendX(YAHOO.ext.grid.LoadableDataModel, YAHOO.ext.grid.DefaultDataModel);

/** @ignore */
YAHOO.ext.grid.LoadableDataModel.prototype.setLoadedPage = function(pageNum, userCallback){
    this.loadedPage = pageNum;
    if(typeof userCallback == 'function'){
        userCallback();
    }
};

/** Returns true if this model uses paging @type Boolean */
YAHOO.ext.grid.LoadableDataModel.prototype.isPaged = function(){
    return this.pageSize > 0;
};

/** Returns the total number of records available, override if needed @type Number */
YAHOO.ext.grid.LoadableDataModel.prototype.getTotalRowCount = function(){
    return this.totalCount || this.getRowCount();
};

/** Returns the number of records per page @type Number */
YAHOO.ext.grid.LoadableDataModel.prototype.getPageSize = function(){
    return this.pageSize;
};

/** Returns the total number of pages available @type Number */
YAHOO.ext.grid.LoadableDataModel.prototype.getTotalPages = function(){
    if(this.getPageSize() == 0 || this.getTotalRowCount() == 0){
        return 1;
    }
    return Math.ceil(this.getTotalRowCount()/this.getPageSize());
};

/** Initializes paging for this model. */
YAHOO.ext.grid.LoadableDataModel.prototype.initPaging = function(url, pageSize, baseParams){
    this.pageUrl = url;
    this.pageSize = pageSize;
    this.remoteSort = true;
    if(baseParams) this.baseParams = baseParams;
};

/** @ignore */
YAHOO.ext.grid.LoadableDataModel.prototype.createParams = function(pageNum, sortColumn, sortDir){
    var params = {}, map = this.paramMap;
    for(var key in this.baseParams){
        if(typeof this.baseParams[key] != 'function'){
            params[key] = this.baseParams[key];
        }
    }
    params[map['page']] = pageNum;
    params[map['pageSize']] = this.getPageSize();
    params[map['sortColumn']] = (typeof sortColumn == 'undefined' ? '' : sortColumn);
    params[map['sortDir']] = sortDir || '';
    return params;
};

YAHOO.ext.grid.LoadableDataModel.prototype.loadPage = function(pageNum, callback, keepExisting){
    var sort = this.getSortState();
    var params = this.createParams(pageNum, sort.column, sort.direction);
    this.load(this.pageUrl, params, this.setLoadedPage.createDelegate(this, [pageNum, callback]), 
               keepExisting ? (pageNum-1) * this.pageSize : null);
};

/** @ignore */
YAHOO.ext.grid.LoadableDataModel.prototype.applySort = function(suppressEvent){
	if(!this.remoteSort){
        YAHOO.ext.grid.LoadableDataModel.superclass.applySort.apply(this, arguments);
    }else if(!suppressEvent){
        var sort = this.getSortState();
        if(sort.column){
           this.fireRowsSorted(sort.column, sort.direction, true);
        }
    }
};

/** @ignore */
YAHOO.ext.grid.LoadableDataModel.prototype.resetPaging = function(){
	this.loadedPage = 1;
};

/** Overridden sort method to use remote sorting if turned on */
YAHOO.ext.grid.LoadableDataModel.prototype.sort = function(sortInfo, columnIndex, direction, suppressEvent){
    if(!this.remoteSort){
        YAHOO.ext.grid.LoadableDataModel.superclass.sort.apply(this, arguments);
    }else{
        this.sortInfo = sortInfo;
        this.sortColumn = columnIndex;
        this.sortDir = direction;
        var params = this.createParams(this.loadedPage, columnIndex, direction);
        this.load(this.pageUrl, params, this.fireRowsSorted.createDelegate(this, [columnIndex, direction, true]));
    }
}
/**
 * Initiates the loading of the data from the specified URL - Failed load attempts will 
 * fire the {@link #onLoadException} event.
 * @param {Object/String} url The url from which the data can be loaded
 * @param {<i>String/Object</i>} params (optional) The parameters to pass as either a url encoded string "param1=1&amp;param2=2" or as an object {param1: 1, param2: 2}
 * @param {<i>Function</i>} callback (optional) Callback when load is complete - called with signature (this, rowCountLoaded)
 * @param {<i>Number</i>} insertIndex (optional) if present, loaded data is inserted at the specified index instead of overwriting existing data
 */
YAHOO.ext.grid.LoadableDataModel.prototype.load = function(url, params, callback, insertIndex){
	this.fireEvent('beforeload');
	if(params && typeof params != 'string'){ // must be object
        var buf = [];
        for(var key in params){
            if(typeof params[key] != 'function'){
                buf.push(encodeURIComponent(key), '=', encodeURIComponent(params[key]), '&');
            }
        }
        delete buf[buf.length-1];
        params = buf.join('');
    }
    var cb = {
        success: this.processResponse,
        failure: this.processException,
        scope: this,
		argument: {callback: callback, insertIndex: insertIndex}
    };
    var method = params ? 'POST' : 'GET';
    YAHOO.util.Connect.asyncRequest(method, url, cb, params);
};

/**@private*/
YAHOO.ext.grid.LoadableDataModel.prototype.processResponse = function(response){
    var cb = response.argument.callback;
    var keepExisting = (typeof response.argument.insertIndex == 'number');
    var insertIndex = response.argument.insertIndex;
    switch(this.dataType){
    	case YAHOO.ext.grid.LoadableDataModel.XML:
    		this.loadData(response.responseXML, cb, keepExisting, insertIndex);
    	break;
    	case YAHOO.ext.grid.LoadableDataModel.JSON:
    		var rtext = response.responseText;
    		try { // this code is a modified version of Yahoo! UI DataSource JSON parsing
		        // Trim leading spaces
		        while(rtext.substring(0,1) == " ") {
		            rtext = rtext.substring(1, rtext.length);
		        }
		        // Invalid JSON response
		        if(rtext.indexOf("{") < 0) {
		            throw "Invalid JSON response";
		        }
		
		        // Empty (but not invalid) JSON response
		        if(rtext.indexOf("{}") === 0) {
		            this.loadData({}, response.argument.callback);
		            return;
		        }
		
		        // Turn the string into an object literal...
		        // ...eval is necessary here
		        var jsonObjRaw = eval("(" + rtext + ")");
		        if(!jsonObjRaw) {
		            throw "Error evaling JSON response";
		        }
				this.loadData(jsonObjRaw, cb, keepExisting, insertIndex);
		    } catch(e) {
		        this.fireLoadException(e, response);
				if(typeof callback == 'function'){
			    	callback(this, false);
			    }
		   	}
    	break;
    	case YAHOO.ext.grid.LoadableDataModel.TEXT:
    		this.loadData(response.responseText, cb, keepExisting, insertIndex);
    	break;
    };
};

/**@private*/
YAHOO.ext.grid.LoadableDataModel.prototype.processException = function(response){
    this.fireLoadException(null, response);
    if(typeof response.argument.callback == 'function'){
        response.argument.callback(this, false);
    }
};

YAHOO.ext.grid.LoadableDataModel.prototype.fireLoadException = function(e, responseObj){
    this.onLoadException.fireDirect(this, e, responseObj);
};

YAHOO.ext.grid.LoadableDataModel.prototype.fireLoadEvent = function(){
    this.fireEvent('load', this.loadedPage, this.getTotalPages());
};

/**
 * Adds a preprocessor function to parse data before it is added to the Model - ie. Date.parse to parse dates.
 */
YAHOO.ext.grid.LoadableDataModel.prototype.addPreprocessor = function(columnIndex, fn){
    this.preprocessors[columnIndex] = fn;
};

/**
 * Gets the preprocessor function for the specified column.
 */
YAHOO.ext.grid.LoadableDataModel.prototype.getPreprocessor = function(columnIndex){
    return this.preprocessors[columnIndex];
};

/**
 * Removes a preprocessor function.
 */
YAHOO.ext.grid.LoadableDataModel.prototype.removePreprocessor = function(columnIndex){
    this.preprocessors[columnIndex] = null;
};

/**
 * Adds a postprocessor function to format data before updating the underlying data source (ie. convert date to string before updating XML document).
 */
YAHOO.ext.grid.LoadableDataModel.prototype.addPostprocessor = function(columnIndex, fn){
    this.postprocessors[columnIndex] = fn;
};

/**
 * Gets the postprocessor function for the specified column.
 */
YAHOO.ext.grid.LoadableDataModel.prototype.getPostprocessor = function(columnIndex){
    return this.postprocessors[columnIndex];
};

/**
 * Removes a postprocessor function.
 */
YAHOO.ext.grid.LoadableDataModel.prototype.removePostprocessor = function(columnIndex){
    this.postprocessors[columnIndex] = null;
};
/**
 * Empty interface method - Called to process the data returned by the XHR - Classes which extend LoadableDataModel should implement this method.
 * See {@link YAHOO.ext.XMLDataModel} for an example implementation.
 */
YAHOO.ext.grid.LoadableDataModel.prototype.loadData = function(data, callback, keepExisting, insertIndex){
	
};

YAHOO.ext.grid.LoadableDataModel.XML = 'xml';
YAHOO.ext.grid.LoadableDataModel.JSON = 'json';
YAHOO.ext.grid.LoadableDataModel.TEXT = 'text';

/*
YAHOO.ext.grid.SparceDataset = function(bufferSize){
    this.stack = [];
    this.bufferSize = bufferSize || 1000;
    this.maxIndex = 0;
    
    this.events = {
        'rowsexpired' : new YAHOO.util.CustomEvent('rowsexpired')
    };
};

YAHOO.ext.grid.SparceDataset.prototype = {
    addListener : YAHOO.ext.grid.Grid.prototype.addListener,
    removeListener : YAHOO.ext.grid.Grid.prototype.removeListener,
    fireEvent : YAHOO.ext.grid.Grid.prototype.fireEvent,
    
    getRowAt : function(index){
        return this[String(index)];
    },
    
    splice : function(index, deleteCount){
        this.insertRowsAt(index, Array.prototype.slice.call(arguments, 2));
    },
    
    concat : function(){
        this.insertRowsAt(index, Array.prototype.slice.call(arguments, 2));
    },
    
    insertRowsAt: function(index, rowData){
        for(var i = 0; i < rowData.length; i++) {
        	var d = rowData[i];
        	var dataIndex = index + i;
        	this[dataIndex] = d;
        	this.stack.push(dataIndex);
        }
        this.maxIndex = Math.max(this.maxIndex, index+rowData.length);
        this.cleanup();
    },
    
    cleanup : function(){
        while(stack.length > this.bufferSize){
            var dataIndex = stack.shift();
            delete this[dataIndex];
            this.fireEvent('rowsexpired', dataIndex);
        }
    }
};*/







/*
------------------------------------------------------------------
// File: data\XMLDataModel.js
------------------------------------------------------------------
*/

/**
 * @class
 * This is an implementation of a DataModel used by the Grid. It works 
 * with XML data. 
 * <br>Example schema from Amazon search:
 * <pre><code>
 * var schema = {
 *     tagName: 'Item',
 *     id: 'ASIN',
 *     fields: ['Author', 'Title', 'Manufacturer', 'ProductGroup']
 * };
 * </code></pre>
 * @extends YAHOO.ext.grid.LoadableDataModel
 * @constructor
 * @param {Object} schema The schema to use
 * @param {XMLDocument} xml An XML document to load immediately
*/
YAHOO.ext.grid.XMLDataModel = function(schema, xml){
    YAHOO.ext.grid.XMLDataModel.superclass.constructor.call(this, YAHOO.ext.grid.LoadableDataModel.XML);
    /**@private*/
    this.schema = schema;
    this.xml = xml;
    if(xml){
        this.loadData(xml);
    }
};
YAHOO.extendX(YAHOO.ext.grid.XMLDataModel, YAHOO.ext.grid.LoadableDataModel);

YAHOO.ext.grid.XMLDataModel.prototype.getDocument = function(){
   return this.xml;    
};

/**
 * Overrides loadData in LoadableDataModel to process XML
 * @param {XMLDocument} doc The document to load
 * @param {<i>Function</i>} callback (optional) callback to call when loading is complete
 * @param {<i>Boolean</i>} keepExisting (optional) true to keep existing data
 * @param {<i>Number</i>} insertIndex (optional) if present, loaded data is inserted at the specified index instead of overwriting existing data
 */
YAHOO.ext.grid.XMLDataModel.prototype.loadData = function(doc, callback, keepExisting, insertIndex){
	this.xml = doc;
	var idField = this.schema.id;
	var fields = this.schema.fields;
	if(this.schema.totalTag){
	    this.totalCount = null;
	    var totalNode = doc.getElementsByTagName(this.schema.totalTag);
	    if(totalNode && totalNode.item(0) && totalNode.item(0).firstChild) {
            var v = parseInt(totalNode.item(0).firstChild.nodeValue, 10);
            if(!isNaN(v)){
                this.totalCount = v;
            }
    	}
	}
	var rowData = [];
	var nodes = doc.getElementsByTagName(this.schema.tagName);
    if(nodes && nodes.length > 0) {
	    for(var i = 0; i < nodes.length; i++) {
	        var node = nodes.item(i);
	        var colData = [];
	        colData.node = node;
	        colData.id = this.getNamedValue(node, idField, String(i));
	        for(var j = 0; j < fields.length; j++) {
	            var val = this.getNamedValue(node, fields[j], "");
	            if(this.preprocessors[j]){
	                val = this.preprocessors[j](val);
	            }
	            colData.push(val);
	        }
	        rowData.push(colData);
	    }
    }
    if(keepExisting !== true){
       YAHOO.ext.grid.XMLDataModel.superclass.removeAll.call(this);
	}
	if(typeof insertIndex != 'number'){
	    insertIndex = this.getRowCount();
	}
    YAHOO.ext.grid.XMLDataModel.superclass.insertRows.call(this, insertIndex, rowData);
    if(typeof callback == 'function'){
    	callback(this, true);
    }
    this.fireLoadEvent();
};

/**
 * Adds a row to this DataModel and syncs the XML document
 * @param {String} id The id of the row, if null the next row index is used
 * @param {Array} cellValues The cell values for this row
 * @return {Number} The index of the new row
 */
YAHOO.ext.grid.XMLDataModel.prototype.addRow = function(id, cellValues){
    var newIndex = this.getRowCount();
    var node = this.createNode(this.xml, id, cellValues);
    cellValues.id = id || newIndex;
    cellValues.node = node;
    YAHOO.ext.grid.XMLDataModel.superclass.addRow.call(this, cellValues);
    return newIndex;
};

/**
 * Inserts a row into this DataModel and syncs the XML document
 * @param {Number} index The index to insert the row
 * @param {String} id The id of the row, if null the next row index is used
 * @param {Array} cellValues The cell values for this row
 * @return {Number} The index of the new row
 */
YAHOO.ext.grid.XMLDataModel.prototype.insertRow = function(index, id, cellValues){
    var node = this.createNode(this.xml, id, cellValues);
    cellValues.id = id || this.getRowCount();
    cellValues.node = node;
    YAHOO.ext.grid.XMLDataModel.superclass.insertRow.call(this, index, cellValues);
    return index;
};

/**
 * Removes the row from DataModel and syncs the XML document
 * @param {Number} index The index of the row to remove
 */
YAHOO.ext.grid.XMLDataModel.prototype.removeRow = function(index){
    var node = this.data[index].node;
    node.parentNode.removeChild(node);
    YAHOO.ext.grid.XMLDataModel.superclass.removeRow.call(this, index, index);
};

YAHOO.ext.grid.XMLDataModel.prototype.getNode = function(rowIndex){
    return this.data[rowIndex].node;
};

/**
 * Override this method to define your own node creation routine for when new rows are added.
 * By default this method clones the first node and sets the column values in the newly cloned node.
 * @param {XMLDocument} xmlDoc The xml document being used by this model
 * @param {Array} colData The column data for the new node
 * @return {XMLNode} The created node
 */
YAHOO.ext.grid.XMLDataModel.prototype.createNode = function(xmlDoc, id, colData){
    var template = this.data[0].node;
    var newNode = template.cloneNode(true);
    var fields = this.schema.fields;
    for(var i = 0; i < fields.length; i++){
        var nodeValue = colData[i];
        if(this.postprocessors[i]){
            nodeValue = this.postprocessors[i](nodeValue);
        }
        this.setNamedValue(newNode, fields[i], nodeValue);
    }
    if(id){
        this.setNamedValue(newNode, this.schema.idField, id);
    }
    template.parentNode.appendChild(newNode);
    return newNode;
};

/**
 * Convenience function looks for value in attributes, then in children tags - also 
 * normalizes namespace matches (ie matches ns:tag, FireFox matches tag and not ns:tag).
 */
YAHOO.ext.grid.XMLDataModel.prototype.getNamedValue = function(node, name, defaultValue){
	if(!node || !name){
		return defaultValue;
	}
	var nodeValue = defaultValue;
    var attrNode = node.attributes.getNamedItem(name);
    if(attrNode) {
    	nodeValue = attrNode.value;
    } else {
        var childNode = node.getElementsByTagName(name);
        if(childNode && childNode.item(0) && childNode.item(0).firstChild) {
            nodeValue = childNode.item(0).firstChild.nodeValue;
    	}else{
    	    // try to strip namespace for FireFox
    	    var index = name.indexOf(':');
    	    if(index > 0){
    	        return this.getNamedValue(node, name.substr(index+1), defaultValue);
    	    }
    	}
    }
    return nodeValue;
};

/**
 * Convenience function set a value in the underlying xml node.
 */
YAHOO.ext.grid.XMLDataModel.prototype.setNamedValue = function(node, name, value){
	if(!node || !name){
		return;
	}
	var attrNode = node.attributes.getNamedItem(name);
    if(attrNode) {
    	attrNode.value = value;
    	return;
    }
    var childNode = node.getElementsByTagName(name);
    if(childNode && childNode.item(0) && childNode.item(0).firstChild) {
        childNode.item(0).firstChild.nodeValue = value;
    }else{
	    // try to strip namespace for FireFox
	    var index = name.indexOf(':');
	    if(index > 0){
	        this.setNamedValue(node, name.substr(index+1), value);
	    }
	}
};

/**
 * Overrides DefaultDataModel.setValueAt to update the underlying XML Document
 * @param {Object} value The new value
 * @param {Number} rowIndex
 * @param {Number} colIndex
 */
YAHOO.ext.grid.XMLDataModel.prototype.setValueAt = function(value, rowIndex, colIndex){
    var node = this.data[rowIndex].node;
    if(node){
        var nodeValue = value;
        if(this.postprocessors[colIndex]){
            nodeValue = this.postprocessors[colIndex](value);
        }
        this.setNamedValue(node, this.schema.fields[colIndex], nodeValue);
    }
    YAHOO.ext.grid.XMLDataModel.superclass.setValueAt.call(this, value, rowIndex, colIndex);
};

/**
 * Overrides getRowId in DefaultDataModel to return the ID value of the specified node. 
 * @param {Number} rowIndex
 * @return {Number}
 */
YAHOO.ext.grid.XMLDataModel.prototype.getRowId = function(rowIndex){
    return this.data[rowIndex].id;
};

/*
------------------------------------------------------------------
// File: data\JSONDataModel.js
------------------------------------------------------------------
*/

/**
 * @class
 * This is an implementation of a DataModel used by the Grid. It works 
 * with JSON data.
 * <br>Example schema:
 * <pre><code>
 * var schema = {
 *     root: 'Results.Result',
 *     id: 'ASIN',
 *     fields: ['Author', 'Title', 'Manufacturer', 'ProductGroup']
 * };
 * </code></pre>
 * @extends YAHOO.ext.grid.LoadableDataModel
 * @constructor
*/
YAHOO.ext.grid.JSONDataModel = function(schema){
    YAHOO.ext.grid.JSONDataModel.superclass.constructor.call(this, YAHOO.ext.grid.LoadableDataModel.JSON);
    /**@private*/
    this.schema = schema;
};
YAHOO.extendX(YAHOO.ext.grid.JSONDataModel, YAHOO.ext.grid.LoadableDataModel);

/**
 * Overrides loadData in LoadableDataModel to process JSON data
 * @param {Object} data The JSON object to load
 * @param {Function} callback
 */
YAHOO.ext.grid.JSONDataModel.prototype.loadData = function(data, callback, keepExisting){
	var idField = this.schema.id;
	var fields = this.schema.fields;
	if(this.schema.totalProperty){
        var v = parseInt(eval('data.' + this.schema.totalProperty), 10);
        if(!isNaN(v)){
            this.totalCount = v;
        }
    }
	var rowData = [];
	try{
	    var root = eval('data.' + this.schema.root);
	    for(var i = 0; i < root.length; i++){
			var node = root[i];
			var colData = [];
			colData.node = node;
			colData.id = (typeof node[idField] != 'undefined' && node[idField] !== '' ? node[idField] : String(i));
			for(var j = 0; j < fields.length; j++) {
			    var val = node[fields[j]];
			    if(typeof val == 'undefined'){
			        val = '';
			    }
	            if(this.preprocessors[j]){
	                val = this.preprocessors[j](val);
	            }
	            colData.push(val);
	        }
	        rowData.push(colData);
		}
		if(keepExisting !== true){
		  this.removeAll();
		}
        this.addRows(rowData);
    	if(typeof callback == 'function'){
	    	callback(this, true);
	    }
    	this.fireLoadEvent();
	}catch(e){
		this.fireLoadException(e, null);
		if(typeof callback == 'function'){
	    	callback(this, false);
	    }
	}
};

/**
 * Overrides getRowId in DefaultDataModel to return the ID value of the specified node. 
 * @param {Number} rowIndex
 * @return {Number}
 */
YAHOO.ext.grid.JSONDataModel.prototype.getRowId = function(rowIndex){
    return this.data[rowIndex].id;
};

/*
------------------------------------------------------------------
// File: grid\SelectionModel.js
------------------------------------------------------------------
*/
/**
 @class The default SelectionModel used by {@link YAHOO.ext.grid.Grid}. 
 It supports multiple selections and keyboard selection/navigation. <br><br>
 @constructor
 */
YAHOO.ext.grid.DefaultSelectionModel = function(){
    /** @private */
    this.selectedRows = [];
    /** @private */
    this.selectedRowIds = [];
    /** @private */
    this.lastSelectedRow = null;
    
    /** Fires when a row is selected or deselected - fireDirect sig: (this, row, isSelected)
     * @type YAHOO.util.CustomEvent 
     * @deprecated
     */
    this.onRowSelect = new YAHOO.util.CustomEvent('SelectionTable.rowSelected');
    /** Fires when the selection changes on the Grid - fireDirect sig: (this, selectedRows[], selectedRowIds[])
     * @type YAHOO.util.CustomEvent 
     * @deprecated
     */
    this.onSelectionChange = new YAHOO.util.CustomEvent('SelectionTable.selectionChanged');
    
    this.events = {
        'selectionchange' : this.onSelectionChange,
        'rowselect' : this.onRowSelect
    };
    
    this.locked = false;
};

YAHOO.ext.grid.DefaultSelectionModel.prototype = {
    /** @ignore Called by the grid automatically. Do not call directly. */
    init : function(grid){
        this.grid = grid;
        this.initEvents();
    },
    
    lock : function(){
        this.locked = true;
    },
    
    unlock : function(){
        this.locked = false;  
    },
    
    isLocked : function(){
        return this.locked;    
    },
    
    /** @ignore */
    initEvents : function(){
        if(this.grid.trackMouseOver){
        	this.grid.addListener("mouseover", this.handleOver, this, true);
        	this.grid.addListener("mouseout", this.handleOut, this, true);
        }
        this.grid.addListener("rowclick", this.rowClick, this, true);
        this.grid.addListener("keydown", this.keyDown, this, true);
    },
    
    /** Inherited from Observable */
    fireEvent : YAHOO.ext.util.Observable.prototype.fireEvent,
    /** Inherited from Observable */
    on : YAHOO.ext.util.Observable.prototype.on,
    /** Inherited from Observable */
    addListener : YAHOO.ext.util.Observable.prototype.addListener,
    /** Inherited from Observable */
    delayedListener : YAHOO.ext.util.Observable.prototype.delayedListener,
    /** Inherited from Observable */
    removeListener : YAHOO.ext.util.Observable.prototype.removeListener,
    
    /** @ignore Syncs selectedRows with the correct row by looking it up by id. 
      Used after a sort moves data around. */
    syncSelectionsToIds : function(){
        if(this.getCount() > 0){
            var ids = this.selectedRowIds.concat();
            this.clearSelections();
            this.selectRowsById(ids, true);
        }
    },
    
    /**
     * Set the selected rows by their ID(s). IDs must match what is returned by the DataModel getRowId(index).
     * @param {String/Array} id The id(s) to select 
     * @param {<i>Boolean</i>} keepExisting (optional) True to retain existing selections 
     */
    selectRowsById : function(id, keepExisting){
        var rows = this.grid.getRowsById(id);
        if (!(rows instanceof Array)){
            this.selectRow(rows, keepExisting);
            return;
        }
        this.selectRows(rows, keepExisting);
    },
    
    /**
     * Gets the number of selected rows.
     * @return {Number}
     */
    getCount : function(){
        return this.selectedRows.length;
    },
    
    /**
     * Selects the first row in the grid.
     */
    selectFirstRow : function(){
        for(var j = 0; j < this.grid.rows.length; j++){
            if(this.isSelectable(this.grid.rows[j])){
            	this.focusRow(this.grid.rows[j]);
                this.setRowState(this.grid.rows[j], true);
                return;
            }
        }
    },
    
    /**
     * Selects the row immediately following the last selected row.
     * @param {<i>Boolean</i>} keepExisting (optional) True to retain existing selections
     */
    selectNext : function(keepExisting){
        if(this.lastSelectedRow){
            for(var j = (this.lastSelectedRow.rowIndex+1); j < this.grid.rows.length; j++){
                var row = this.grid.rows[j];
                if(this.isSelectable(row)){
                    this.focusRow(row);
                    this.setRowState(row, true, keepExisting);
                    return;
                }
            }
        }
    },
    
    /**
     * Selects the row that precedes the last selected row.
     * @param {<i>Boolean</i>} keepExisting (optional) True to retain existing selections 
     */
    selectPrevious : function(keepExisting){
        if(this.lastSelectedRow){
            for(var j = (this.lastSelectedRow.rowIndex-1); j >= 0; j--){
                var row = this.grid.rows[j];
                if(this.isSelectable(row)){
                    this.focusRow(row);
                    this.setRowState(row, true, keepExisting);
                    return;
                }
            }
        }
    },
    
    /**
     * Returns the selected rows.
     * @return {Array} Array of DOM row elements
     */
    getSelectedRows : function(){
        return this.selectedRows;
    },
    
    /**
     * Returns the selected row ids.
     * @return {Array} Array of String ids
     */
    getSelectedRowIds : function(){
        return this.selectedRowIds;
    },
    
    /**
     * Clears all selections.
     */
    clearSelections : function(){
        if(this.isLocked()) return;
        var oldSelections = this.selectedRows.concat();
        for(var j = 0; j < oldSelections.length; j++){
            this.setRowState(oldSelections[j], false);
        }
        this.selectedRows = [];
        this.selectedRowIds = [];
    },
    
        
    /**
     * Selects all rows.
     */
    selectAll : function(){
        if(this.isLocked()) return;
        for(var j = 0; j < this.grid.rows.length; j++){
            this.setRowState(this.grid.rows[j], true, true);
        }
    },
    
    /**
     * Returns True if there is a selection.
     * @return {Boolean}
     */
    hasSelection : function(){
        return this.selectedRows.length > 0;
    },
    
    /**
     * Returns True if the specified row is selected.
     * @param {HTMLElement} row The row to check
     * @return {Boolean}
     */
    isSelected : function(row){
        return row && (row.selected === true || row.getAttribute('selected') == 'true');
    },
    
    /**
     * Returns True if the specified row is selectable.
     * @param {HTMLElement} row The row to check
     * @return {Boolean}
     */
    isSelectable : function(row){
        return row && row.getAttribute('selectable') != 'false';
    },
    
    /** @ignore */
    rowClick : function(grid, rowIndex, e){
        if(this.isLocked()) return;
        var row = grid.getRow(rowIndex);
        if(this.isSelectable(row)){
            if(e.shiftKey && this.lastSelectedRow){
                var lastIndex = this.lastSelectedRow.rowIndex;
                this.selectRange(this.lastSelectedRow, row, e.ctrlKey);
                this.lastSelectedRow = this.grid.el.dom.rows[lastIndex];
            }else{
                this.focusRow(row);
                var rowState = e.ctrlKey ? !this.isSelected(row) : true;
                this.setRowState(row, rowState, e.hasModifier());
            }
        }
    },
    
    /**
     * Deprecated. Tries to focus the row and scroll it into view - Use grid.scrollTo or grid.getView().focusRow() instead.
     * @deprecated
     * @param {HTMLElement} row The row to focus
     */
    focusRow : function(row){
    	this.grid.view.focusRow(row);
    },

    /**
     * Selects a row.
     * @param {Number/HTMLElement} row The row or index of the row to select
     * @param {<i>Boolean</i>} keepExisting (optional) True to retain existing selections 
     */
    selectRow : function(row, keepExisting){
        this.setRowState(this.getRow(row), true, keepExisting);
    },
    
    /**
     * Selects multiple rows.
     * @param {Array} rows Array of the rows or indexes of the row to select
     * @param {<i>Boolean</i>} keepExisting (optional) True to retain existing selections 
     */
    selectRows : function(rows, keepExisting){
        if(!keepExisting){
            this.clearSelections();
        }
        for(var i = 0; i < rows.length; i++){
            this.selectRow(rows[i], true);
        }
    },
    
    /**
     * Deselects a row.
     * @param {Number/HTMLElement} row The row or index of the row to deselect
     */
    deselectRow : function(row){
        this.setRowState(this.getRow(row), false);
    },
    
    /** @ignore */
    getRow : function(row){
        if(typeof row == 'number'){
            row = this.grid.rows[row];
        }
        return row;
    },
    
    /**
     * Selects a range of rows. All rows in between startRow and endRow are also selected.
     * @param {Number/HTMLElement} startRow The row or index of the first row in the range
     * @param {Number/HTMLElement} endRow The row or index of the last row in the range
     * @param {<i>Boolean</i>} keepExisting (optional) True to retain existing selections 
     */
    selectRange : function(startRow, endRow, keepExisting){
        startRow = this.getRow(startRow);
        endRow = this.getRow(endRow);
        this.setRangeState(startRow, endRow, true, keepExisting);
    },
    
    /**
     * Deselects a range of rows. All rows in between startRow and endRow are also deselected.
     * @param {Number/HTMLElement} startRow The row or index of the first row in the range
     * @param {Number/HTMLElement} endRow The row or index of the last row in the range
     */
    deselectRange : function(startRow, endRow){
        startRow = this.getRow(startRow);
        endRow = this.getRow(endRow);
        this.setRangeState(startRow, endRow, false, true);
    },
    
    /** @ignore */
    setRowStateFromChild : function(childEl, selected, keepExisting){
        var row = this.grid.getRowFromChild(childEl);
        this.setRowState(row, selected, keepExisting);
    },
    
    /** @ignore */
    setRangeState : function(startRow, endRow, selected, keepExisting){
        if(this.isLocked()) return;
        if(!keepExisting){
            this.clearSelections();
        }
        var curRow = startRow;
        while(curRow.rowIndex != endRow.rowIndex){
            this.setRowState(curRow, selected, true);
            curRow = (startRow.rowIndex < endRow.rowIndex ? 
                        this.grid.getRowAfter(curRow) : this.grid.getRowBefore(curRow))
        }
        this.setRowState(endRow, selected, true);
    },
    
    /** @ignore */
    setRowState : function(row, selected, keepExisting){
        if(this.isLocked()) return;
        if(this.isSelectable(row)){
            if(selected){
                if(!keepExisting){
                    this.clearSelections();
                }
                this.setRowClass(row, 'selected');
                row.selected = true;
                this.selectedRows.push(row);
                this.selectedRowIds.push(this.grid.dataModel.getRowId(row.rowIndex));
                this.lastSelectedRow = row;
            }else{
                this.setRowClass(row, '');
                row.selected = false;
                this._removeSelected(row);
            }
            this.fireEvent('rowselect', this, row, selected);
            this.fireEvent('selectionchange', this, this.selectedRows, this.selectedRowIds);
        }
    },

    /** @ignore */
    handleOver : function(e){
        var row = this.grid.getRowFromChild(e.getTarget());
        if(this.isSelectable(row) && !this.isSelected(row)){
            this.setRowClass(row, 'over');
        }
    },
    
    /** @ignore */
    handleOut : function(e){
        var row = this.grid.getRowFromChild(e.getTarget());
        if(this.isSelectable(row) && !this.isSelected(row)){
            this.setRowClass(row, '');
        }
    },
    
    /** @ignore */
    keyDown : function(e){
        if(e.browserEvent.keyCode == e.DOWN){
            this.selectNext(e.shiftKey);
            e.preventDefault();
        }else if(e.browserEvent.keyCode == e.UP){
            this.selectPrevious(e.shiftKey);
            e.preventDefault();
        }
    },

    /** @ignore */
    setRowClass : function(row, cssClass){
        if(this.isSelectable(row)){
            if(cssClass == 'selected'){
                YAHOO.util.Dom.removeClass(row, 'ygrid-row-over');
                YAHOO.util.Dom.addClass(row, 'ygrid-row-selected');
            }else if(cssClass == 'over'){
                YAHOO.util.Dom.removeClass(row, 'ygrid-row-selected');
                YAHOO.util.Dom.addClass(row, 'ygrid-row-over');
            }else if(cssClass == ''){
                YAHOO.util.Dom.removeClass(row, 'ygrid-row-selected');
                YAHOO.util.Dom.removeClass(row, 'ygrid-row-over');
            }
        }
    },
    
    /** @ignore */
    _removeSelected : function(row){
        var sr = this.selectedRows;
        for (var i = 0; i < sr.length; i++) {
          if (sr[i] === row){
              this.selectedRows.splice(i, 1);
              this.selectedRowIds.splice(i, 1);
              return;
          }
        }
    }
};

/**
 @class Extends {@link YAHOO.ext.grid.DefaultSelectionModel} to allow only one row to be selected at a time. <br><br> 
 @extends YAHOO.ext.grid.DefaultSelectionModel
 @constructor
 */
YAHOO.ext.grid.SingleSelectionModel = function(){
    YAHOO.ext.grid.SingleSelectionModel.superclass.constructor.call(this);
};

YAHOO.extendX(YAHOO.ext.grid.SingleSelectionModel, YAHOO.ext.grid.DefaultSelectionModel);

/** @ignore */
YAHOO.ext.grid.SingleSelectionModel.prototype.setRowState = function(row, selected){
    YAHOO.ext.grid.SingleSelectionModel.superclass.setRowState.call(this, row, selected, false);
};

/**
 @class Extends {@link YAHOO.ext.grid.DefaultSelectionModel} to disable row selection. <br><br> 
 @extends YAHOO.ext.grid.DefaultSelectionModel
 @constructor
 */
YAHOO.ext.grid.DisableSelectionModel = function(){
    YAHOO.ext.grid.DisableSelectionModel.superclass.constructor.call(this);
};

YAHOO.extendX(YAHOO.ext.grid.DisableSelectionModel, YAHOO.ext.grid.DefaultSelectionModel);

YAHOO.ext.grid.DisableSelectionModel.prototype.initEvents = function(){
};

/*
------------------------------------------------------------------
// File: grid\EditorSelectionModel.js
------------------------------------------------------------------
*/

/**
 @class Extends {@link YAHOO.ext.grid.DefaultSelectionModel} to enable cell navigation. <br><br> 
 @extends YAHOO.ext.grid.DefaultSelectionModel
 @constructor
 */
YAHOO.ext.grid.EditorSelectionModel = function(){
    YAHOO.ext.grid.EditorSelectionModel.superclass.constructor.call(this);
    /** Number of clicks to activate a cell (for editing) - valid values are 1 or 2
     * @type Number */
    this.clicksToActivateCell = 1;
    this.events['cellactivate'] = new YAHOO.util.CustomEvent('cellactivate');
};

YAHOO.extendX(YAHOO.ext.grid.EditorSelectionModel, YAHOO.ext.grid.DefaultSelectionModel);

YAHOO.ext.grid.EditorSelectionModel.prototype.disableArrowNavigation = false;
YAHOO.ext.grid.EditorSelectionModel.prototype.controlForArrowNavigation = false;

/** @ignore */
YAHOO.ext.grid.EditorSelectionModel.prototype.initEvents = function(){
    this.grid.addListener("cellclick", this.onCellClick, this, true);
    this.grid.addListener("celldblclick", this.onCellDblClick, this, true);
    this.grid.addListener("keydown", this.keyDown, this, true);
};

YAHOO.ext.grid.EditorSelectionModel.prototype.onCellClick = function(grid, rowIndex, colIndex){
    if(this.clicksToActivateCell == 1){
        var row = this.grid.getRow(rowIndex);
        var cell = row.childNodes[colIndex];
        if(cell){
            this.activate(row, cell);
        }
    }
};

YAHOO.ext.grid.EditorSelectionModel.prototype.activate = function(row, cell){
    this.fireEvent('cellactivate', this, row, cell);
    this.grid.doEdit(row, cell);
};

YAHOO.ext.grid.EditorSelectionModel.prototype.onCellDblClick = function(grid, rowIndex, colIndex){
    if(this.clicksToActivateCell == 2){
        var row = this.grid.getRow(rowIndex);
        var cell = row.childNodes[colIndex];
        if(cell){
            this.activate(row, cell);
        }
    }
};

/** @ignore */
YAHOO.ext.grid.EditorSelectionModel.prototype.setRowState = function(row, selected){
    YAHOO.ext.grid.EditorSelectionModel.superclass.setRowState.call(this, row, false, false);
};
/** @ignore */
YAHOO.ext.grid.EditorSelectionModel.prototype.focusRow = function(row, selected){
};

YAHOO.ext.grid.EditorSelectionModel.prototype.getEditorCellAfter = function(cell, spanRows){
    var g = this.grid;
    var next = g.getCellAfter(cell);
    while(next && !g.colModel.isCellEditable(next.columnIndex)){
        next = g.getCellAfter(next);
    }
    if(!next && spanRows){
        var row = g.getRowAfter(g.getRowFromChild(cell));
        if(row){
            next = g.getFirstCell(row);
            if(!g.colModel.isCellEditable(next.columnIndex)){
                next = this.getEditorCellAfter(next);
            }
        }
    }
    return next;
};

YAHOO.ext.grid.EditorSelectionModel.prototype.getEditorCellBefore = function(cell, spanRows){
    var g = this.grid;
    var prev = g.getCellBefore(cell);
    while(prev && !g.colModel.isCellEditable(prev.columnIndex)){
        prev = g.getCellBefore(prev);
    }
    if(!prev && spanRows){
        var row = g.getRowBefore(g.getRowFromChild(cell));
        if(row){
            prev = g.getLastCell(row);
            if(!g.colModel.isCellEditable(prev.columnIndex)){
               prev = this.getEditorCellBefore(prev);
            }
        }
    }
    return prev;
};

YAHOO.ext.grid.EditorSelectionModel.prototype.allowArrowNav = function(e){
    return (!this.disableArrowNavigation && (!this.controlForArrowNavigation || e.ctrlKey));
}
/** @ignore */
YAHOO.ext.grid.EditorSelectionModel.prototype.keyDown = function(e){
    var g = this.grid, cm = g.colModel, cell = g.getEditingCell();
    if(!cell) return;
    var newCell;
    switch(e.browserEvent.keyCode){
         case e.TAB:
             if(e.shiftKey){
                 newCell = this.getEditorCellBefore(cell, true);
             }else{
                 newCell = this.getEditorCellAfter(cell, true);
             }
             e.preventDefault();
         break;
         case e.DOWN:
             if(this.allowArrowNav(e)){
                 var next = g.getRowAfter(g.getRowFromChild(cell));
                 if(next){
                     newCell = next.childNodes[cell.columnIndex];
                 }
             }
         break;
         case e.UP:
             if(this.allowArrowNav(e)){
                 var prev = g.getRowBefore(g.getRowFromChild(cell));
                 if(prev){
                     newCell = prev.childNodes[cell.columnIndex];
                 }
             }
         break;
         case e.RETURN:
             if(e.shiftKey){
                 var prev = g.getRowBefore(g.getRowFromChild(cell));
                 if(prev){
                     newCell = prev.childNodes[cell.columnIndex];
                 }
             }else{
                 var next = g.getRowAfter(g.getRowFromChild(cell));
                 if(next){
                     newCell = next.childNodes[cell.columnIndex];
                 }
             }
         break;
         case e.RIGHT:
             if(this.allowArrowNav(e)){
                 newCell = this.getEditorCellAfter(cell);
             }
         break;
         case e.LEFT:
             if(this.allowArrowNav(e)){
                 newCell = this.getEditorCellBefore(cell);
             }
         break;
    };
    if(newCell){
        this.activate(g.getRowFromChild(newCell), newCell);
        e.stopEvent();
    }
};

YAHOO.ext.grid.EditorAndSelectionModel = function(){
    YAHOO.ext.grid.EditorAndSelectionModel.superclass.constructor.call(this);
    this.events['cellactivate'] = new YAHOO.util.CustomEvent('cellactivate');
};

YAHOO.extendX(YAHOO.ext.grid.EditorAndSelectionModel, YAHOO.ext.grid.DefaultSelectionModel);

YAHOO.ext.grid.EditorAndSelectionModel.prototype.initEvents = function(){
    YAHOO.ext.grid.EditorAndSelectionModel.superclass.initEvents.call(this);
    this.grid.addListener("celldblclick", this.onCellDblClick, this, true);
};

YAHOO.ext.grid.EditorAndSelectionModel.prototype.onCellDblClick = function(grid, rowIndex, colIndex){
    var row = this.grid.getRow(rowIndex);
    var cell = row.childNodes[colIndex];
    if(cell){
        this.fireEvent('cellactivate', this, row, cell);
        this.grid.doEdit(row, cell);
    }
}; 

/*
------------------------------------------------------------------
// File: grid\editor\CellEditor.js
------------------------------------------------------------------
*/
YAHOO.ext.grid.CellEditor = function(element){
    this.colIndex = null;
    this.rowIndex = null;
    this.grid = null;
    this.editing = false;
    this.originalValue = null;
    this.element = getEl(element, true);
    this.element.addClass('ygrid-editor');
    this.element.dom.tabIndex = 1;
    this.initialized = false;
    this.callback = null;
};

YAHOO.ext.grid.CellEditor.prototype = {
    init : function(grid, bodyElement, callback){
        // there's no way for the grid to know if multiple columns 
        // share the same editor so it will try to initialize the 
        // same one over and over
        if(this.initialized) return;
        this.initialized = true;
        this.callback = callback;
        this.grid = grid;
        bodyElement.appendChild(this.element.dom);
        this.initEvents();
    },
    
    initEvents : function(){
        var stopOnEnter = function(e){
            if(e.browserEvent.keyCode == e.RETURN){
                this.stopEditing(true);
            }else if(e.browserEvent.keyCode == e.ESC){
                this.setValue(this.originalValue);
                this.stopEditing(true);
            }
        }
        this.element.mon('keydown', stopOnEnter, this, true);
        this.element.on('blur', this.stopEditing, this, true);
    },

    startEditing : function(value, row, cell){
        this.originalValue = value;
        this.rowIndex = row.rowIndex;
        this.colIndex = cell.columnIndex;
        this.cell = cell;
        this.setValue(value);
        var cellbox = getEl(cell, true).getBox();
        this.fitToCell(cellbox);
        this.editing = true;
        this.show();
    },
     
    stopEditing : function(focusCell){
         if(this.editing){
             this.editing = false;
             var newValue = this.getValue();
             this.hide();
             //if(focusCell){try{this.cell.focus();}catch(e){}}; // try to give the cell focus so keyboard nav still works
             if(this.originalValue != newValue){
                this.callback(newValue, this.rowIndex, this.colIndex);
             }
         }
     },
     
    setValue : function(value){
        this.element.dom.value = value;
    },
    
    getValue : function(){
        return this.element.dom.value;
    },
    
    fitToCell : function(box){
        this.element.setBox(box, true);
    },
    
    show : function(){
        this.element.show();
        this.element.focus();
    },
    
    hide : function(){
        try{
            this.element.dom.blur();
        }catch(e){}
        this.element.hide();
    }
};

/*
------------------------------------------------------------------
// File: grid\editor\CheckboxEditor.js
------------------------------------------------------------------
*/

YAHOO.ext.grid.CheckboxEditor = function(){
    var div = document.createElement('span');
    div.className = 'ygrid-editor ygrid-checkbox-editor';
    var cb = document.createElement('input');
    cb.type = 'checkbox';
    cb.setAttribute('autocomplete', 'off');
    div.appendChild(cb);
    document.body.appendChild(div);
    YAHOO.ext.grid.CheckboxEditor.superclass.constructor.call(this, div);
    div.tabIndex = '';
    cb.tabIndex = 1;
    this.cb = getEl(cb, true);
};

YAHOO.extendX(YAHOO.ext.grid.CheckboxEditor, YAHOO.ext.grid.CellEditor);

YAHOO.ext.grid.CheckboxEditor.prototype.fitToCell = function(box){
    this.element.setBox(box, true);
};

YAHOO.ext.grid.CheckboxEditor.prototype.setValue = function(value){
     this.cb.dom.checked = (value === true || value === 'true' || value === 1 || value === '1');
};

YAHOO.ext.grid.CheckboxEditor.prototype.getValue = function(){
     return this.cb.dom.checked;
};

YAHOO.ext.grid.CheckboxEditor.prototype.show = function(){
    this.element.show();
    this.cb.focus();
};

YAHOO.ext.grid.CheckboxEditor.prototype.initEvents = function(){
    var stopOnEnter = function(e){
        if(e.browserEvent.keyCode == e.RETURN){
            this.stopEditing(true);
        }else if(e.browserEvent.keyCode == e.ESC){
            this.setValue(this.originalValue);
            this.stopEditing(true);
        }
    }
    this.cb.mon('keydown', stopOnEnter, this, true);
    this.cb.on('blur', this.stopEditing, this, true);
};

YAHOO.ext.grid.CheckboxEditor.prototype.hide = function(){
    try{
        this.cb.dom.blur();
    }catch(e){}
    this.element.hide();
};

/*
------------------------------------------------------------------
// File: grid\editor\DateEditor.js
------------------------------------------------------------------
*/
YAHOO.ext.grid.DateEditor = function(config){
    var div = document.createElement('span');
    div.className = 'ygrid-editor ygrid-editor-container';
    
    var element = document.createElement('input');
    element.type = 'text';
    element.tabIndex = 1;
    element.setAttribute('autocomplete', 'off');
    div.appendChild(element);
    
    var pick = document.createElement('span');
    pick.className = 'pick-button';
    div.appendChild(pick);
    
    document.body.appendChild(div);
    
    this.div = getEl(div, true);
    this.element = getEl(element, true);
    this.pick = getEl(pick, true);
    
    this.colIndex = null;
    this.rowIndex = null;
    this.grid = null;
    this.editing = false;
    this.originalValue = null;
    this.initialized = false;
    this.callback = null;
    
    this.cal = null;
    this.mouseDownHandler = YAHOO.ext.EventManager.wrap(this.handleMouseDown, this, true);
    
    YAHOO.ext.util.Config.apply(this, config);
    if(typeof this.minValue == 'string') this.minValue = this.parseDate(this.minValue);
    if(typeof this.maxValue == 'string') this.maxValue = this.parseDate(this.maxValue);
    this.ddMatch = /ddnone/;
    if(this.disabledDates){
        var dd = this.disabledDates;
        var re = "(?:";
        for(var i = 0; i < dd.length; i++){
            re += dd[i];
            if(i != dd.length-1) re += "|";
        }
        this.ddMatch = new RegExp(re + ")");
    }
};

YAHOO.ext.grid.DateEditor.prototype = {
    init : function(grid, bodyElement, callback){
        if(this.initialized) return;
        
        this.initialized = true;
        this.callback = callback;
        this.grid = grid;
        bodyElement.appendChild(this.div.dom);
        this.initEvents();
    },
    
    initEvents : function(){
         var stopOnEnter = function(e){
            if(e.browserEvent.keyCode == e.RETURN){
                this.stopEditing(true);
            }else if(e.browserEvent.keyCode == e.ESC){
                this.setValue(this.originalValue);
                this.stopEditing(true);
            }
        }
        this.element.mon('keydown', stopOnEnter, this, true);
        var vtask = new YAHOO.ext.util.DelayedTask(this.validate, this);
        this.element.mon('keyup', vtask.delay.createDelegate(vtask, [this.validationDelay]));
        this.pick.on('click', this.showCalendar, this, true);
    },
    
    startEditing : function(value, row, cell){
        this.originalValue = value;
        this.rowIndex = row.rowIndex;
        this.colIndex = cell.columnIndex;
        this.cell = cell;
        this.setValue(value);
        this.validate();
        var cellbox = getEl(cell, true).getBox();
        this.div.setBox(cellbox, true);
        this.element.setWidth(cellbox.width-this.pick.getWidth());
        this.editing = true;
        YAHOO.util.Event.on(document, "mousedown", this.mouseDownHandler);
        this.show();
    },
     
     stopEditing : function(focusCell){
         if(this.editing){
             YAHOO.util.Event.removeListener(document, "mousedown", this.mouseDownHandler);
             this.editing = false;
             var newValue = this.getValue();
             this.hide();
             //if(focusCell){try{this.cell.focus();}catch(e){}}// try to give the cell focus so keyboard nav still works
             if(this.originalValue != newValue){
                this.callback(newValue, this.rowIndex, this.colIndex);
             }
         }
     },
    
    setValue : function(value){
        this.element.dom.value = this.formatDate(value);
        this.validate();
    },
    
    getValue : function(){
        if(!this.validate()){
           return this.originalValue;
       }else{
           var value = this.element.dom.value;
           if(value.length < 1){
               return value;
           } else{
               return this.parseDate(value);
           }
       }   
    },
    
    show : function() {
        this.div.show();
        this.element.focus();
        this.validate();
    },
    
    hide : function(){
        try{
            this.element.dom.blur();
        }catch(e){}
        this.div.hide();
    },
    
    validate : function(){
        var dom = this.element.dom;
        var value = dom.value;
        if(value.length < 1){ // if it's blank
             if(this.allowBlank){
                 dom.title = '';
                 this.element.removeClass('ygrid-editor-invalid');
                 return true;
             }else{
                 dom.title = this.blankText;
                 this.element.addClass('ygrid-editor-invalid');
                 return false;
             }
        }
        value = this.parseDate(value);
        if(!value){
            dom.title = this.invalidText.replace('%0', dom.value).replace('%1', this.format);
            this.element.addClass('ygrid-editor-invalid');
            return false;
        }
        var time = value.getTime();
        if(this.minValue && time < this.minValue.getTime()){
            dom.title = this.minText.replace('%0', this.formatDate(this.minValue));
            this.element.addClass('ygrid-editor-invalid');
            return false;
        }
        if(this.maxValue && time > this.maxValue.getTime()){
            dom.title = this.maxText.replace('%0', this.formatDate(this.maxValue));
            this.element.addClass('ygrid-editor-invalid');
            return false;
        }
        if(this.disabledDays){
            var day = value.getDay();
            for(var i = 0; i < this.disabledDays.length; i++) {
            	if(day === this.disabledDays[i]){
            	    dom.title = this.disabledDaysText;
                    this.element.addClass('ygrid-editor-invalid');
                    return false;
            	}
            }
        }
        var fvalue = this.formatDate(value);
        if(this.ddMatch.test(fvalue)){
            dom.title = this.disabledDatesText.replace('%0', fvalue);
            this.element.addClass('ygrid-editor-invalid');
            return false;
        }
        var msg = this.validator(value);
        if(msg !== true){
            dom.title = msg;
            this.element.addClass('ygrid-editor-invalid');
            return false;
        }
        dom.title = '';
        this.element.removeClass('ygrid-editor-invalid');
        return true;
    },
    
    handleMouseDown : function(e){
        var t = e.getTarget();
        var dom = this.div.dom;
        if(t != dom && !YAHOO.util.Dom.isAncestor(dom, t)){
            this.stopEditing();
        }
    },
    
    showCalendar : function(value){
        if(this.cal == null){
            this.cal = new YAHOO.ext.DatePicker(this.div.dom.parentNode.parentNode);
        }
        this.cal.minDate = this.minValue;
        this.cal.maxDate = this.maxValue;
        this.cal.disabledDatesRE = this.ddMatch;
        this.cal.disabledDatesText = this.disabledDatesText;
        this.cal.disabledDays = this.disabledDays;
        this.cal.disabledDaysText = this.disabledDaysText;
        this.cal.format = this.format;
        if(this.minValue){
            this.cal.minText = this.minText.replace('%0', this.formatDate(this.minValue));
        }
        if(this.maxValue){
            this.cal.maxText = this.maxText.replace('%0', this.formatDate(this.maxValue));
        }
        var r = this.div.getRegion();
        this.cal.show(r.left, r.bottom, this.getValue(), this.setValue.createDelegate(this));
    },
    
    parseDate : function(value){
        if(!value || value instanceof Date) return value;
        return Date.parseDate(value, this.format);
    },
    
    formatDate : function(date){
        if(!date || !(date instanceof Date)) return date;
        return date.format(this.format);
    }
};

YAHOO.ext.grid.DateEditor.prototype.format = 'm/d/y';
YAHOO.ext.grid.DateEditor.prototype.disabledDays = null;
YAHOO.ext.grid.DateEditor.prototype.disabledDaysText = '';
YAHOO.ext.grid.DateEditor.prototype.disabledDates = null;
YAHOO.ext.grid.DateEditor.prototype.disabledDatesText = '';
YAHOO.ext.grid.DateEditor.prototype.allowBlank = true;
YAHOO.ext.grid.DateEditor.prototype.minValue = null;
YAHOO.ext.grid.DateEditor.prototype.maxValue = null;
YAHOO.ext.grid.DateEditor.prototype.minText = 'The date in this field must be after %0';
YAHOO.ext.grid.DateEditor.prototype.maxText = 'The date in this field must be before %0';
YAHOO.ext.grid.DateEditor.prototype.blankText = 'This field cannot be blank';
YAHOO.ext.grid.DateEditor.prototype.invalidText = '%0 is not a valid date - it must be in the format %1';
YAHOO.ext.grid.DateEditor.prototype.validationDelay = 200;
YAHOO.ext.grid.DateEditor.prototype.validator = function(){return true;};

/*
------------------------------------------------------------------
// File: grid\editor\NumberEditor.js
------------------------------------------------------------------
*/
YAHOO.ext.grid.NumberEditor = function(config){
    var element = document.createElement('input');
    element.type = 'text';
    element.className = 'ygrid-editor ygrid-num-editor';
    element.setAttribute('autocomplete', 'off');
    document.body.appendChild(element);
    YAHOO.ext.grid.NumberEditor.superclass.constructor.call(this, element);
    YAHOO.ext.util.Config.apply(this, config);
};
YAHOO.extendX(YAHOO.ext.grid.NumberEditor, YAHOO.ext.grid.CellEditor);

YAHOO.ext.grid.NumberEditor.prototype.initEvents = function(){
    var stopOnEnter = function(e){
        if(e.browserEvent.keyCode == e.RETURN){
            this.stopEditing(true);
        }else if(e.browserEvent.keyCode == e.ESC){
            this.setValue(this.originalValue);
            this.stopEditing(true);
        }
    };
    
    var allowed = "0123456789";
    if(this.allowDecimals){
        allowed += this.decimalSeparator;
    }
    if(this.allowNegative){
        allowed += '-';
    }
    var keyPress = function(e){
        var c = e.getCharCode();
        if(c != e.BACKSPACE && allowed.indexOf(String.fromCharCode(c)) === -1){
            e.stopEvent();
        }
    };
    this.element.mon('keydown', stopOnEnter, this, true);
    var vtask = new YAHOO.ext.util.DelayedTask(this.validate, this);
    this.element.mon('keyup', vtask.delay.createDelegate(vtask, [this.validationDelay]));
    this.element.mon('keypress', keyPress, this, true);
    this.element.on('blur', this.stopEditing, this, true);
};

YAHOO.ext.grid.NumberEditor.prototype.validate = function(){
    var dom = this.element.dom;
    var value = dom.value;
    if(value.length < 1){ // if it's blank
         if(this.allowBlank){
             dom.title = '';
             this.element.removeClass('ygrid-editor-invalid');
             return true;
         }else{
             dom.title = this.blankText;
             this.element.addClass('ygrid-editor-invalid');
             return false;
         }
    }
    if(value.search(/\d+/) === -1){
        dom.title = this.nanText.replace('%0', value);
        this.element.addClass('ygrid-editor-invalid');
        return false;
    }
    var num = this.parseValue(value);
    if(num < this.minValue){
        dom.title = this.minText.replace('%0', this.minValue);
        this.element.addClass('ygrid-editor-invalid');
        return false;
    }
    if(num > this.maxValue){
        dom.title = this.maxText.replace('%0', this.maxValue);
        this.element.addClass('ygrid-editor-invalid');
        return false;
    }
    var msg = this.validator(value);
    if(msg !== true){
        dom.title = msg;
        this.element.addClass('ygrid-editor-invalid');
        return false;
    }
    dom.title = '';
    this.element.removeClass('ygrid-editor-invalid');
    return true;
};

YAHOO.ext.grid.NumberEditor.prototype.show = function(){
    this.element.dom.title = '';
    YAHOO.ext.grid.NumberEditor.superclass.show.call(this);
    if(this.selectOnFocus){
        try{
            this.element.dom.select();
        }catch(e){}
    }
    this.validate(this.element.dom.value);
};

YAHOO.ext.grid.NumberEditor.prototype.getValue = function(){
   if(!this.validate()){
       return this.originalValue;
   }else{
       var value = this.element.dom.value;
       if(value.length < 1){
           return value;
       } else{
           return this.fixPrecision(this.parseValue(value));
       }
   }   
};
YAHOO.ext.grid.NumberEditor.prototype.parseValue = function(value){
    return parseFloat(new String(value).replace(this.decimalSeparator, '.'));
};

YAHOO.ext.grid.NumberEditor.prototype.fixPrecision = function(value){
   if(!this.allowDecimals || this.decimalPrecision == -1 || isNaN(value) || value == 0 || !value){
       return value;
   }
   // this should work but doesn't due to precision error in JS
   // var scale = Math.pow(10, this.decimalPrecision);
   // var fixed = this.decimalPrecisionFcn(value * scale);
   // return fixed / scale;
   //
   // so here's our workaround:
   var scale = Math.pow(10, this.decimalPrecision+1);
   var fixed = this.decimalPrecisionFcn(value * scale);
   fixed = this.decimalPrecisionFcn(fixed/10);
   return fixed / (scale/10);
};

YAHOO.ext.grid.NumberEditor.prototype.allowBlank = true;
YAHOO.ext.grid.NumberEditor.prototype.allowDecimals = true;
YAHOO.ext.grid.NumberEditor.prototype.decimalSeparator = '.';
YAHOO.ext.grid.NumberEditor.prototype.decimalPrecision = 2;
YAHOO.ext.grid.NumberEditor.prototype.decimalPrecisionFcn = Math.floor;
YAHOO.ext.grid.NumberEditor.prototype.allowNegative = true;
YAHOO.ext.grid.NumberEditor.prototype.selectOnFocus = true;
YAHOO.ext.grid.NumberEditor.prototype.minValue = Number.NEGATIVE_INFINITY;
YAHOO.ext.grid.NumberEditor.prototype.maxValue = Number.MAX_VALUE;
YAHOO.ext.grid.NumberEditor.prototype.minText = 'The minimum value for this field is %0';
YAHOO.ext.grid.NumberEditor.prototype.maxText = 'The maximum value for this field is %0';
YAHOO.ext.grid.NumberEditor.prototype.blankText = 'This field cannot be blank';
YAHOO.ext.grid.NumberEditor.prototype.nanText = '%0 is not a valid number';
YAHOO.ext.grid.NumberEditor.prototype.validationDelay = 100;
YAHOO.ext.grid.NumberEditor.prototype.validator = function(){return true;};

/*
------------------------------------------------------------------
// File: widgets\DatePicker.js
------------------------------------------------------------------
*/
YAHOO.ext.DatePicker = function(id, parentElement){
    this.id = id;
    this.selectedDate = new Date();
    this.visibleDate = new Date();
    this.element = null;
    this.shadow = null;
    this.callback = null;
    this.buildControl(parentElement || document.body);
    this.mouseDownHandler = YAHOO.ext.EventManager.wrap(this.handleMouseDown, this, true);
    this.keyDownHandler = YAHOO.ext.EventManager.wrap(this.handleKeyDown, this, true);
    this.wheelHandler = YAHOO.ext.EventManager.wrap(this.handleMouseWheel, this, true);
};

YAHOO.ext.DatePicker.prototype = {
    show : function(x, y, value, callback){
        this.hide();
        this.selectedDate = value;
        this.visibleDate = value;
        this.callback = callback;
        this.refresh();
        this.element.show();
        this.element.setXY(this.constrainToViewport ? this.constrainXY(x, y) : [x, y]);
        this.shadow.show();
        this.shadow.setRegion(this.element.getRegion());
        this.element.dom.tabIndex = 1;
        this.element.focus();
        YAHOO.util.Event.on(document, "mousedown", this.mouseDownHandler);
        YAHOO.util.Event.on(document, "keydown", this.keyDownHandler);
        YAHOO.util.Event.on(document, "mousewheel", this.wheelHandler);
        YAHOO.util.Event.on(document, "DOMMouseScroll", this.wheelHandler);
    },
    
    constrainXY : function(x, y){
        var w = YAHOO.util.Dom.getViewportWidth();
        var h = YAHOO.util.Dom.getViewportHeight();
        var size = this.element.getSize();
        return [
            Math.min(w-size.width, x),
            Math.min(h-size.height, y)
        ];
    },
    
    hide : function(){
        this.shadow.hide();
        this.element.hide();
        YAHOO.util.Event.removeListener(document, "mousedown", this.mouseDownHandler);
        YAHOO.util.Event.removeListener(document, "keydown", this.keyDownHandler);
        YAHOO.util.Event.removeListener(document, "mousewheel", this.wheelHandler);
        YAHOO.util.Event.removeListener(document, "DOMMouseScroll", this.wheelHandler);
    },
    
    setSelectedDate : function(date){
        this.selectedDate = date;
    },
    
    getSelectedDate : function(){
        return this.selectedDate;
    },
    
    showPrevMonth : function(){
        this.visibleDate = this.getPrevMonth(this.visibleDate);
        this.refresh();
    },
    
    showNextMonth : function(){
        this.visibleDate = this.getNextMonth(this.visibleDate);
        this.refresh();
    },
    
    showPrevYear : function(){
        var d = this.visibleDate;
        this.visibleDate = new Date(d.getFullYear()-1, d.getMonth(), d.getDate());
        this.refresh();
    },
    
    showNextYear : function(){
        var d = this.visibleDate;
        this.visibleDate = new Date(d.getFullYear()+1, d.getMonth(), d.getDate());
        this.refresh();
    },
    
    handleMouseDown : function(e){
        var target = e.getTarget();
        if(target != this.element.dom && !YAHOO.util.Dom.isAncestor(this.element.dom, target)){
            this.hide();
        }
    },
    
    handleKeyDown : function(e){
        switch(e.browserEvent.keyCode){
            case e.LEFT:
                this.showPrevMonth();
                e.stopEvent();
            break;
            case e.RIGHT:
                this.showNextMonth();
                e.stopEvent();
            break;
            case e.DOWN:
                this.showPrevYear();
                e.stopEvent();
            break;
            case e.UP:
                this.showNextYear();
                e.stopEvent();
            break;
        }
    },
    
    handleMouseWheel : function(e){
        var delta = e.getWheelDelta();
        if(delta > 0){
            this.showPrevMonth();
            e.stopEvent();
        } else if(delta < 0){
            this.showNextMonth();
            e.stopEvent();
        }
    },
    
    handleClick : function(e){
        var d = this.visibleDate;
        var t = e.getTarget();
        if(t && t.className){
            switch(t.className){
                case 'active':
                    this.handleSelection(new Date(d.getFullYear(), d.getMonth(), parseInt(t.innerHTML)));
                break;
                case 'prevday':
                    var p = this.getPrevMonth(d);
                    this.handleSelection(new Date(p.getFullYear(), p.getMonth(), parseInt(t.innerHTML)));
                break;
                case 'nextday':
                    var n = this.getNextMonth(d);
                    this.handleSelection(new Date(n.getFullYear(), n.getMonth(), parseInt(t.innerHTML)));
                break;
                case 'ypopcal-today':
                    this.handleSelection(new Date());
                break;
                case 'next-month':
                    this.showNextMonth();
                break;
                case 'prev-month':
                    this.showPrevMonth();
                break;
            }   
        }
        e.stopEvent();
    },
    
    selectToday : function(){
        this.handleSelection(new Date());
    },
    
    handleSelection: function(date){
        this.selectedDate = date;
        this.callback(date);
        this.hide();    
    },
    
    getPrevMonth : function(d){
        var m = d.getMonth();var y = d.getFullYear();
        return (m == 0 ? new Date(--y, 11, 1) : new Date(y, --m, 1));
    },
    
    getNextMonth : function(d){
        var m = d.getMonth();var y = d.getFullYear();
        return (m == 11 ? new Date(++y, 0, 1) : new Date(y, ++m, 1));
    },
    
    getDaysInMonth : function(m, y){
        return (m == 1 || m == 3 || m == 5 || m == 7 || m == 8 || m == 10 || m == 12) ? 31 : (m == 4 || m == 6 || m == 9 || m == 11) ? 30 : this.isLeapYear(y) ? 29 : 28;
    },
    
    isLeapYear : function(y){
        return (((y % 4) == 0) && ((y % 100) != 0) || ((y % 400) == 0));
    },
    
    clearTime : function(date){
        if(date){
            date.setHours(0);
            date.setMinutes(0);
            date.setSeconds(0);
            date.setMilliseconds(0);
        }
        return date;
    },
    
    refresh : function(){
        var d = this.visibleDate;
        this.buildInnerCal(d);
        this.calHead.update(this.monthNames[d.getMonth()] + ' ' + d.getFullYear());
        if(this.element.isVisible()){
            this.shadow.setRegion(this.element.getRegion());
        }
    }
};

/**
 * This code is not pretty, but it is fast!
 * @ignore
 */ 
YAHOO.ext.DatePicker.prototype.buildControl = function(parentElement){
    var c = document.createElement('div');
    c.style.position = 'absolute';
    c.style.visibility = 'hidden';
    document.body.appendChild(c);
    var html = '<iframe id="'+this.id+'_shdw" frameborder="0" style="position:absolute; z-index:2000; display:none; top:0px; left:0px;" class="ypopcal-shadow"></iframe>' +
    '<div hidefocus="true" class="ypopcal" id="'+this.id+'" style="-moz-outline:none; position:absolute; z-index:2001; display:none; top:0px; left:0px;">' +
    '<table class="ypopcal-head" border=0 cellpadding=0 cellspacing=0><tbody><tr><td class="ypopcal-arrow"><div class="prev-month">&nbsp;</div></td><td class="ypopcal-month">&nbsp;</td><td class="ypopcal-arrow"><div class="next-month">&nbsp;</div></td></tr></tbody></table>' +
    '<center><div class="ypopcal-inner">';
    html += "<table border=0 cellpadding=2 cellspacing=0 class=\"ypopcal-table\"><thead><tr class=\"ypopcal-daynames\">";
    var names = this.dayNames;
    for(var i = 0; i < names.length; i++){
        html += '<td>' + names[i].substr(0, 1) + '</td>';
    }
    html+= "</tr></thead><tbody><tr>";
    for(var i = 0; i < 42; i++) {
        if(i % 7 == 0 && i != 0){
            html += '</tr><tr>';
        }
        html += "<td>&nbsp;</td>";
    }
    html += "</tr></tbody></table>";
    html += '</div><button class="ypopcal-today" style="margin-top:2px;">'+this.todayText+'</button></center></div>';
    c.innerHTML = html;
    this.shadow = getEl(c.childNodes[0], true);
    this.shadow.enableDisplayMode();
    this.element = getEl(c.childNodes[1], true);
    this.element.enableDisplayMode();
    document.body.appendChild(this.shadow.dom);
    document.body.appendChild(this.element.dom);
    document.body.removeChild(c);
    this.element.on("selectstart", function(){return false;});
    var tbody = this.element.dom.getElementsByTagName('tbody')[1];
    this.cells = tbody.getElementsByTagName('td');
    this.calHead = this.element.getChildrenByClassName('ypopcal-month', 'td')[0];
    this.element.mon('mousedown', this.handleClick, this, true);
};

YAHOO.ext.DatePicker.prototype.buildInnerCal = function(dateVal){
    var days = this.getDaysInMonth(dateVal.getMonth() + 1, dateVal.getFullYear());
    var firstOfMonth = new Date(dateVal.getFullYear(), dateVal.getMonth(), 1);
    var startingPos = firstOfMonth.getDay();
    if(startingPos == 0) startingPos = 7;
    var pm = this.getPrevMonth(dateVal);
    var prevStart = this.getDaysInMonth(pm.getMonth()+1, pm.getFullYear())-startingPos;
    var cells = this.cells;
    days += startingPos;
    
    // convert everything to numbers so it's fast
    var day = 86400000;
    var date = this.clearTime(new Date(pm.getFullYear(), pm.getMonth(), prevStart));
    var today = this.clearTime(new Date()).getTime();
    var sel = this.selectedDate ? this.clearTime(this.selectedDate).getTime() : today + 1; //today +1 will never match anything
    var min = this.minDate ? this.clearTime(this.minDate).getTime() : Number.NEGATIVE_INFINITY;
    var max = this.maxDate ? this.clearTime(this.maxDate).getTime() : Number.POSITIVE_INFINITY;
    var ddMatch = this.disabledDatesRE;
    var ddText = this.disabledDatesText;
    var ddays = this.disabledDays;
    var ddaysText = this.disabledDaysText;
    var format = this.format;
    
    var setCellClass = function(cal, cell, d){
        cell.title = '';
        var t = d.getTime();
        if(t == today){
            cell.className += ' today';
            cell.title = cal.todayText;
        }
        if(t == sel){
            cell.className += ' selected';
        }
        // disabling
        if(t < min) {
            cell.className = ' ypopcal-disabled';
            cell.title = cal.minText;
            return;
        }
        if(t > max) {
            cell.className = ' ypopcal-disabled';
            cell.title = cal.maxText;
            return;
        }
        if(ddays){
            var day = d.getDay();
            for(var i = 0; i < ddays.length; i++) {
            	if(day === ddays[i]){
            	    cell.title = ddaysText;
                    cell.className = ' ypopcal-disabled';
                    return;
                }
            }
        }
        if(ddMatch && format){
            var fvalue = d.format(format);
            if(ddMatch.test(fvalue)){
                cell.title = ddText.replace('%0', fvalue);
                cell.className = ' ypopcal-disabled';
                return;
            }
        }
    };
    
    var i = 0;
    for(; i < startingPos; i++) {
        cells[i].innerHTML = (++prevStart);
        date.setDate(date.getDate()+1);
        cells[i].className = 'prevday';
        setCellClass(this, cells[i], date);
    }
    for(; i < days; i++){
        intDay = i - startingPos + 1;
        cells[i].innerHTML = (intDay);
        date.setDate(date.getDate()+1);
        cells[i].className = 'active';
        setCellClass(this, cells[i], date);
    }
    var extraDays = 0;
    for(; i < 42; i++) {
         cells[i].innerHTML = (++extraDays);
         date.setDate(date.getDate()+1);
         cells[i].className = 'nextday';
         setCellClass(this, cells[i], date);
    }
};

YAHOO.ext.DatePicker.prototype.todayText = "Today";
YAHOO.ext.DatePicker.prototype.minDate = null;
YAHOO.ext.DatePicker.prototype.maxDate = null;
YAHOO.ext.DatePicker.prototype.minText = "This date is before the minimum date";
YAHOO.ext.DatePicker.prototype.maxText = "This date is after the maximum date";
YAHOO.ext.DatePicker.prototype.format = 'm/d/y';
YAHOO.ext.DatePicker.prototype.disabledDays = null;
YAHOO.ext.DatePicker.prototype.disabledDaysText = '';
YAHOO.ext.DatePicker.prototype.disabledDatesRE = null;
YAHOO.ext.DatePicker.prototype.disabledDatesText = '';
YAHOO.ext.DatePicker.prototype.constrainToViewport = true;


YAHOO.ext.DatePicker.prototype.monthNames = Date.monthNames;

YAHOO.ext.DatePicker.prototype.dayNames = Date.dayNames;

/*
------------------------------------------------------------------
// File: grid\editor\SelectEditor.js
------------------------------------------------------------------
*/
YAHOO.ext.grid.SelectEditor = function(element){
    element.hideFocus = true;
    YAHOO.ext.grid.SelectEditor.superclass.constructor.call(this, element);
    this.element.swallowEvent('click');
};
YAHOO.extendX(YAHOO.ext.grid.SelectEditor, YAHOO.ext.grid.CellEditor);

YAHOO.ext.grid.SelectEditor.prototype.fitToCell = function(box){
    if(YAHOO.ext.util.Browser.isGecko){
        box.height -= 3;
    }
    this.element.setBox(box, true);
};

/*
------------------------------------------------------------------
// File: grid\editor\TextEditor.js
------------------------------------------------------------------
*/
YAHOO.ext.grid.TextEditor = function(config){
    var element = document.createElement('input');
    element.type = 'text';
    element.className = 'ygrid-editor ygrid-text-editor';
    element.setAttribute('autocomplete', 'off');
    document.body.appendChild(element);
    YAHOO.ext.grid.TextEditor.superclass.constructor.call(this, element);
    YAHOO.ext.util.Config.apply(this, config);
};
YAHOO.extendX(YAHOO.ext.grid.TextEditor, YAHOO.ext.grid.CellEditor);

YAHOO.ext.grid.TextEditor.prototype.validate = function(){
    var dom = this.element.dom;
    var value = dom.value;
    if(value.length < 1){ // if it's blank
         if(this.allowBlank){
             dom.title = '';
             this.element.removeClass('ygrid-editor-invalid');
             return true;
         }else{
             dom.title = this.blankText;
             this.element.addClass('ygrid-editor-invalid');
             return false;
         }
    }
    if(value.length < this.minLength){
        dom.title = this.minText.replace('%0', this.minLength);
        this.element.addClass('ygrid-editor-invalid');
        return false;
    }
    if(value.length > this.maxLength){
        dom.title = this.maxText.replace('%0', this.maxLength);
        this.element.addClass('ygrid-editor-invalid');
        return false;
    }
    var msg = this.validator(value);
    if(msg !== true){
        dom.title = msg;
        this.element.addClass('ygrid-editor-invalid');
        return false;
    }
    if(this.regex && !this.regex.test(value)){
        dom.title = this.regexText;
        this.element.addClass('ygrid-editor-invalid');
        return false;
    }
    dom.title = '';
    this.element.removeClass('ygrid-editor-invalid');
    return true;
};

YAHOO.ext.grid.TextEditor.prototype.initEvents = function(){
    YAHOO.ext.grid.TextEditor.superclass.initEvents.call(this);
    var vtask = new YAHOO.ext.util.DelayedTask(this.validate, this);
    this.element.mon('keyup', vtask.delay.createDelegate(vtask, [this.validationDelay]));
};

YAHOO.ext.grid.TextEditor.prototype.show = function(){
    this.element.dom.title = '';
    YAHOO.ext.grid.TextEditor.superclass.show.call(this);
    this.element.focus();
    if(this.selectOnFocus){
        try{
            this.element.dom.select();
        }catch(e){}
    }
    this.validate(this.element.dom.value);
};

YAHOO.ext.grid.TextEditor.prototype.getValue = function(){
   if(!this.validate()){
       return this.originalValue;
   }else{
       return this.element.dom.value;
   }   
};

YAHOO.ext.grid.TextEditor.prototype.allowBlank = true;
YAHOO.ext.grid.TextEditor.prototype.minLength = 0;
YAHOO.ext.grid.TextEditor.prototype.maxLength = Number.MAX_VALUE;
YAHOO.ext.grid.TextEditor.prototype.minText = 'The minimum length for this field is %0';
YAHOO.ext.grid.TextEditor.prototype.maxText = 'The maximum length for this field is %0';
YAHOO.ext.grid.TextEditor.prototype.selectOnFocus = true;
YAHOO.ext.grid.TextEditor.prototype.blankText = 'This field cannot be blank';
YAHOO.ext.grid.TextEditor.prototype.validator = function(){return true;};
YAHOO.ext.grid.TextEditor.prototype.validationDelay = 200;
YAHOO.ext.grid.TextEditor.prototype.regex = null;
YAHOO.ext.grid.TextEditor.prototype.regexText = '';

/*
------------------------------------------------------------------
// File: layout\LayoutManager.js
------------------------------------------------------------------
*/
/**
 * Very basic base class for LayoutManagers
 */
YAHOO.ext.LayoutManager = function(container){
    YAHOO.ext.LayoutManager.superclass.constructor.call(this);
    this.el = getEl(container, true);
    this.id = this.el.id;
    this.el.addClass('ylayout-container');
    this.monitorWindowResize = true;
    this.regions = {};
    this.events = {
        'layout' : new YAHOO.util.CustomEvent(),
        'regionresized' : new YAHOO.util.CustomEvent(),
        'regioncollapsed' : new YAHOO.util.CustomEvent(),
        'regionexpanded' : new YAHOO.util.CustomEvent()
    };
    this.updating = false;
    YAHOO.ext.EventManager.onWindowResize(this.onWindowResize, this, true);
};

YAHOO.extendX(YAHOO.ext.LayoutManager, YAHOO.ext.util.Observable, {
    isUpdating : function(){
        return this.updating; 
    },
    
    beginUpdate : function(){
        this.updating = true;    
    },
    
    endUpdate : function(noLayout){
        this.updating = false;
        if(!noLayout){
            this.layout();
        }    
    },
    
    layout: function(){
        
    },
    
    onRegionResized : function(region, newSize){
        this.fireEvent('regionresized', region, newSize);
        this.layout();
    },
    
    onRegionCollapsed : function(region){
        this.fireEvent('regioncollapsed', region);
    },
    
    onRegionExpanded : function(region){
        this.fireEvent('regionexpanded', region);
    },
        
    getViewSize : function(){
        var size;
        if(this.el.dom != document.body){
            this.el.beginMeasure();
            size = this.el.getSize();
            this.el.endMeasure();
        }else{
            size = {width: YAHOO.util.Dom.getViewportWidth(), height: YAHOO.util.Dom.getViewportHeight()};
        }
        size.width -= this.el.getBorderWidth('lr')-this.el.getPadding('lr');
        size.height -= this.el.getBorderWidth('tb')-this.el.getPadding('tb');
        return size;
    },
    
    getEl : function(){
        return this.el;
    },
    
    getRegion : function(target){
        return this.regions[target];
    },
    
    onWindowResize : function(){
        if(this.monitorWindowResize){
            this.layout();
        }
    }
});

/*
------------------------------------------------------------------
// File: layout\LayoutRegion.js
------------------------------------------------------------------
*/

YAHOO.ext.LayoutRegion = function(mgr, config, pos){
    this.mgr = mgr;
    this.position  = pos;
    var dh = YAHOO.ext.DomHelper;
    this.el = dh.append(mgr.el.dom, {tag: 'div', cls: 'ylayout-panel ylayout-panel-' + this.position}, true);
    this.titleEl = dh.append(this.el.dom, {tag: 'div', cls: 'ylayout-panel-hd ylayout-title-'+this.position, children:[
        {tag: 'span', cls: 'ylayout-panel-hd-text', html: '&nbsp;'},
        {tag: 'div', cls: 'ylayout-panel-hd-tools'}
    ]}, true);
    this.titleEl.enableDisplayMode();
    this.titleTextEl = this.titleEl.dom.firstChild;
    this.tools = getEl(this.titleEl.dom.childNodes[1], true);
    this.closeBtn = this.createTool(this.tools.dom, 'ylayout-close');
    this.closeBtn.enableDisplayMode();
    this.closeBtn.on('click', this.closeClicked, this, true);
    this.closeBtn.hide();
    
    this.bodyEl = dh.append(this.el.dom, {tag: 'div', cls: 'ylayout-panel-body'}, true);
    this.events = {
        'invalidated' : new YAHOO.util.CustomEvent('invalidated'),
        'visibilitychange' : new YAHOO.util.CustomEvent('visibilitychange'),
        'paneladded' : new YAHOO.util.CustomEvent('paneladded'),
        'panelremoved' : new YAHOO.util.CustomEvent('panelremoved'),
        'collapsed' : new YAHOO.util.CustomEvent('collapsed'),
        'expanded' : new YAHOO.util.CustomEvent('expanded'),
        'panelactivated' : new YAHOO.util.CustomEvent('panelactivated'),
        'resized' : new YAHOO.util.CustomEvent('resized')
    }
    this.panels = new YAHOO.ext.util.MixedCollection();
    this.panels.getKey = this.getPanelId.createDelegate(this);
    this.box = null;
    this.visible = false;
    this.collapsed = false;
    this.hide();
    this.on('paneladded', this.validateVisibility, this, true);
    this.on('panelremoved', this.validateVisibility, this, true);
    this.activePanel = null;
    
    this.applyConfig(config);
    
    /* Not yet but soon!
    this.draggable = config.draggable || false;
    if(YAHOO.ext.LayoutRegionDD && mgr.enablePanelDD){
        this.dd = new YAHOO.ext.LayoutRegionDD(this);
    }*/
};

YAHOO.extendX(YAHOO.ext.LayoutRegion, YAHOO.ext.util.Observable, {
    getPanelId : function(p){
        return p.getId();
    },
    
    applyConfig : function(config){
        if(config.collapsible && this.position != 'center' && !this.collapsedEl){
            var dh = YAHOO.ext.DomHelper;
            this.collapseBtn = this.createTool(this.tools.dom, 'ylayout-collapse-'+this.position);
            this.collapseBtn.mon('click', this.collapse, this, true);
            this.collapsedEl = dh.append(this.mgr.el.dom, {tag: 'div', cls: 'ylayout-collapsed ylayout-collapsed-'+this.position, children:[
                {tag: 'div', cls: 'ylayout-collapsed-tools'}
            ]}, true);
            if(config.floatable !== false){
               this.collapsedEl.addClassOnOver('ylayout-collapsed-over');
               this.collapsedEl.mon('click', this.collapseClick, this, true);
            }
            this.expandBtn = this.createTool(this.collapsedEl.dom.firstChild, 'ylayout-expand-'+this.position);
            this.expandBtn.mon('click', this.expand, this, true);
        }
        if(this.collapseBtn){
            this.collapseBtn.setVisible(config.collapsible == true);
        }
        this.cmargins = config.cmargins || this.cmargins || 
                         (this.position == 'west' || this.position == 'east' ? 
                             {top: 0, left: 2, right:2, bottom: 0} : 
                             {top: 2, left: 0, right:0, bottom: 2});
        this.margins = config.margins || this.margins || {top: 0, left: 0, right:0, bottom: 0};
        this.bottomTabs = config.tabPosition != 'top';
        this.autoScroll = config.autoScroll || false;
        if(this.autoScroll){
            this.bodyEl.setStyle('overflow', 'auto');
        }else{
            this.bodyEl.setStyle('overflow', 'hidden');
        }
        if((!config.titlebar && !config.title) || config.titlebar === false){
            this.titleEl.hide();
        }else{
            this.titleEl.show();
            if(config.title){
                this.titleTextEl.innerHTML = config.title;
            }
        }
        this.duration = config.duration || .30;
        this.slideDuration = config.slideDuration || .45;
        this.config = config;
        if(config.collapsed){
            this.collapse(true);
        }
    },
    
    /**
     * Resizes the region to the specified size. For vertical regions (west, east) this adjusts the width, for horizontal (north, south) the height.
     * @param {Number} newSize The new width or height
     */
    resizeTo : function(newSize){
        switch(this.position){
            case 'east':
            case 'west':
                this.el.setWidth(newSize);
                this.fireEvent('resized', this, newSize);
            break;
            case 'north':
            case 'south':
                this.el.setHeight(newSize);
                this.fireEvent('resized', this, newSize);
            break;                
        }
    },
    
    getBox : function(){
        var b;
        if(!this.collapsed){
            b = this.el.getBox(false, true);
        }else{
            b = this.collapsedEl.getBox(false, true);
        }
        return b;
    },
    
    getMargins : function(){
        return this.collapsed ? this.cmargins : this.margins;
    },
    
    highlight : function(){
        this.el.addClass('ylayout-panel-dragover'); 
    },
    
    unhighlight : function(){
        this.el.removeClass('ylayout-panel-dragover'); 
    },
    
    updateBox : function(box){
        this.box = box;
        if(!this.collapsed){
            this.el.dom.style.left = box.x + 'px';
            this.el.dom.style.top = box.y + 'px';
            this.el.setSize(box.width, box.height);
            var bodyHeight = this.config.titlebar ? box.height - (this.titleEl.getHeight()||0) : box.height;
            bodyHeight -= this.el.getBorderWidth('tb');
            bodyWidth = box.width - this.el.getBorderWidth('rl');
            this.bodyEl.setHeight(bodyHeight);
            this.bodyEl.setWidth(bodyWidth);
            var tabHeight = bodyHeight;
            if(this.tabs){
                tabHeight = this.tabs.syncHeight(bodyHeight);
                if(YAHOO.ext.util.Browser.isIE) this.tabs.el.repaint();
            }
            this.panelSize = {width: bodyWidth, height: tabHeight};
            if(this.activePanel){
                this.activePanel.setSize(bodyWidth, tabHeight);
            }
        }else{
            this.collapsedEl.dom.style.left = box.x + 'px';
            this.collapsedEl.dom.style.top = box.y + 'px';
            this.collapsedEl.setSize(box.width, box.height);
        }
        if(this.tabs){
            this.tabs.autoSizeTabs();
        }
    },
    
    getEl : function(){
        return this.el;
    },
    
    hide : function(){
        if(!this.collapsed){
            this.el.dom.style.left = '-2000px';
            this.el.hide();
        }else{
            this.collapsedEl.dom.style.left = '-2000px';
            this.collapsedEl.hide();
        }
        this.visible = false;
        this.fireEvent('visibilitychange', this, false);
    },
    
    show : function(){
        if(!this.collapsed){
            this.el.show();
        }else{
            this.collapsedEl.show();
        }
        this.visible = true;
        this.fireEvent('visibilitychange', this, true);
    },
    
    isVisible : function(){
        return this.visible;
    },
    
    closeClicked : function(){
        if(this.activePanel){
            this.remove(this.activePanel);
        }
    },
    
    collapseClick : function(e){
       if(this.isSlid){
           e.stopPropagation();
           this.slideIn();
       }else{
           e.stopPropagation();
           this.slideOut();
       }   
    },
    
    collapse : function(skipAnim){
        if(this.collapsed) return;
        this.collapsed = true;
        if(this.split){
            this.split.el.hide();
        }
        if(this.config.animate && skipAnim !== true){
            this.animateCollapse();
        }else{
            this.el.setLocation(-20000,-20000);
            this.el.hide();
            this.collapsedEl.show();
            this.fireEvent('collapsed', this); 
        }
        this.fireEvent('invalidated', this);    
    },
    
    animateCollapse : function(){
        // overridden
    },
    
    expand : function(e, skipAnim){
        if(e) e.stopPropagation();
        if(!this.collapsed) return;
        if(this.isSlid){
            this.slideIn(this.expand.createDelegate(this));
            return;
        }
        this.collapsed = false;
        this.el.show();
        if(this.config.animate && skipAnim !== true){
            this.animateExpand();
        }else{
            if(this.split){
                this.split.el.show();
            }
            this.collapsedEl.setLocation(-2000,-2000);
            this.collapsedEl.hide();
            this.fireEvent('invalidated', this);
            this.fireEvent('expanded', this);
        }
    },
    
    animateExpand : function(){
        // overridden
    },
    
    initTabs : function(){
        this.bodyEl.setStyle('overflow', 'hidden');
        var ts = new YAHOO.ext.TabPanel(this.bodyEl.dom, this.bottomTabs);
        this.tabs = ts;
        ts.resizeTabs = this.config.resizeTabs === true;
        ts.minTabWidth = this.config.minTabWidth || 40;
        ts.maxTabWidth = this.config.maxTabWidth || 250;
        ts.preferredTabWidth = this.config.preferredTabWidth || 150;
        ts.monitorResize = false;
        ts.bodyEl.setStyle('overflow', this.config.autoScroll ? 'auto' : 'hidden');
        this.panels.each(this.initPanelAsTab, this);
    },
    
    initPanelAsTab : function(panel){
        var ti = this.tabs.addTab(panel.getEl().id, panel.getTitle(), null, 
                    this.config.closeOnTab && panel.isClosable());
        ti.on('activate', function(){ 
              this.setActivePanel(panel); 
        }, this, true);
        if(this.config.closeOnTab){
            ti.on('close', function(){
                     this.remove(panel);
                }, this, true);
            }
        return ti;
    },
    
    updatePanelTitle : function(panel, title){
        if(this.activePanel == panel){
            this.updateTitle(title);
        }
        if(this.tabs){
            this.tabs.getTab(panel.getEl().id).setText(title);
        }
    },
    
    updateTitle : function(title){
        if(this.titleTextEl && !this.config.title){
            this.titleTextEl.innerHTML = (typeof title != 'undefined' && title.length > 0 ? title : "&nbsp;");
        }
    },
    
    setActivePanel : function(panel){
        panel = this.getPanel(panel);
        if(this.activePanel && this.activePanel != panel){
            this.activePanel.setActiveState(false);
        }
        this.activePanel = panel;
        panel.setActiveState(true);
        if(this.panelSize){
            panel.setSize(this.panelSize.width, this.panelSize.height);
        }
        this.closeBtn.setVisible(!this.config.closeOnTab && !this.isSlid && panel.isClosable());
        this.updateTitle(panel.getTitle());
        this.fireEvent('panelactivated', this, panel);
        /*
        if(this.dd && !panel.enableDD){
            this.dd.lock();
        }*/
    },
    
    showPanel : function(panel){
        if(panel = this.getPanel(panel)){
            if(this.tabs){
                this.tabs.activate(panel.getEl().id);
            }else{
                this.setActivePanel(panel);
            }
        }
    },
    
    getActivePanel : function(){
        return this.activePanel;
    },
    
    validateVisibility : function(){
        if(this.panels.getCount() < 1){
            this.updateTitle('&nbsp;');
            this.closeBtn.hide();
            this.hide();
        }else{
            if(!this.isVisible()){
                this.show();
            }
        }
    },
    
    add : function(panel){
        panel.setRegion(this);
        this.panels.add(panel);
        if(this.panels.getCount() == 1 && !this.config.alwaysShowTabs){
            this.bodyEl.dom.appendChild(panel.getEl().dom);
            this.setActivePanel(panel);
            this.fireEvent('paneladded', this, panel);
            return panel;
        }
        if(!this.tabs){
            this.initTabs();
        }else{
            this.initPanelAsTab(panel);
        }
        this.tabs.activate(panel.getEl().id);
        this.fireEvent('paneladded', this, panel);
        return panel;
    },
    
    hasPanel : function(panel){
        return this.getPanel(panel) ? true : false;
    },
    
    hidePanel : function(panel){
        if(this.tabs && (panel = this.getPanel(panel))){
            this.tabs.hideTab(panel.getEl().id);
        }
    },
    
    unhidePanel : function(panel){
        if(this.tabs && (panel = this.getPanel(panel))){
            this.tabs.unhideTab(panel.getEl().id);
        }
    },
    
    clearPanels : function(){
        this.panels.each(this.remove, this);
    },
    
    remove : function(panel){
        panel = this.getPanel(panel);
        if(!panel){
            return null;
        }
        var panelId = panel.getId();
        this.panels.removeKey(panelId);
        if(this.tabs){
            this.tabs.removeTab(panel.getEl().id);
        }else{
            this.bodyEl.dom.removeChild(panel.getEl().dom);
        }
        if(this.panels.getCount() == 1 && this.tabs && !this.config.alwaysShowTabs){
            var p = this.panels.first();
            var tempEl = document.createElement('span'); // temp holder to keep IE from deleting the node
            tempEl.appendChild(p.getEl().dom);
            this.bodyEl.update('');
            this.bodyEl.dom.appendChild(p.getEl().dom);
            tempEl = null;
            this.updateTitle(p.getTitle());
            this.tabs = null;
            this.bodyEl.setStyle('overflow', this.config.autoScroll ? 'auto' : 'hidden');
            this.setActivePanel(p);
        }
        panel.setRegion(null);
        if(this.activePanel == panel){
            this.activePanel = null;
        }
        this.fireEvent('panelremoved', this, panel);
        return panel;
    },
    
    getTabs : function(){
        return this.tabs;    
    },
    
    getPanel : function(id){
        if(typeof id == 'object'){ // must be panel obj
            return id;
        }
        return this.panels.get(id);
    },
    
    getPosition: function(){
        return this.position;    
    },
    
    createTool : function(parentEl, className){
        var btn = YAHOO.ext.DomHelper.append(parentEl, {tag: 'div', cls: 'ylayout-tools-button', 
            children: [{tag: 'div', cls: 'ylayout-tools-button-inner ' + className, html: '&nbsp;'}]}, true);
        btn.on('mouseover', btn.addClass.createDelegate(btn, ['ylayout-tools-button-over']));
        btn.on('mouseout', btn.removeClass.createDelegate(btn, ['ylayout-tools-button-over']));
        return btn;
    }
});

/*
------------------------------------------------------------------
// File: layout\SplitLayoutRegion.js
------------------------------------------------------------------
*/

YAHOO.ext.SplitLayoutRegion = function(mgr, config, pos, cursor){
    this.cursor = cursor;
    YAHOO.ext.SplitLayoutRegion.superclass.constructor.call(this, mgr, config, pos);
    if(config.split){
        this.hide();
    }
};

YAHOO.extendX(YAHOO.ext.SplitLayoutRegion, YAHOO.ext.LayoutRegion, {
    applyConfig : function(config){
        YAHOO.ext.SplitLayoutRegion.superclass.applyConfig.call(this, config);
        if(config.split){
            if(!this.split){
                var splitEl = YAHOO.ext.DomHelper.append(this.mgr.el.dom, 
                        {tag: 'div', id: this.el.id + '-split', cls: 'ylayout-split ylayout-split-'+this.position, html: '&nbsp;'});
                this.split = new YAHOO.ext.SplitBar(splitEl, this.el);
                this.split.onMoved.subscribe(this.onSplitMove, this, true);
                this.split.useShim = config.useShim === true;
                YAHOO.util.Dom.setStyle([this.split.el.dom, this.split.proxy], 'cursor', this.cursor);
                this.split.getMaximumSize = this.getMaxSize.createDelegate(this);
            }
            if(typeof config.minSize != 'undefined'){
                this.split.minSize = config.minSize;
            }
            if(typeof config.maxSize != 'undefined'){
                this.split.maxSize = config.maxSize;
            }
        }
    },
    
    getMaxSize : function(){
         var cmax = this.config.maxSize || 10000;
         var center = this.mgr.getRegion('center');
         return Math.min(cmax, (this.el.getWidth()+center.getEl().getWidth())-center.getMinWidth());
    },
    
    onSplitMove : function(split, newSize){
        this.fireEvent('resized', this, newSize);
    },
    
    getSplitBar : function(){
        return this.split;
    },
    
    hide : function(){
        if(this.split){
            this.split.el.setLocation(-2000,-2000);
            this.split.el.hide();
        }
        YAHOO.ext.SplitLayoutRegion.superclass.hide.call(this);
    },
    
    show : function(){
        if(this.split){
            this.split.el.show();
        }
        YAHOO.ext.SplitLayoutRegion.superclass.show.call(this);
    },
    
    beforeSlide: function(){
        if(YAHOO.ext.util.Browser.isGecko){// firefox overflow auto bug workaround
            if(this.tabs) this.tabs.bodyEl.clip();
            if(this.activePanel){
                this.activePanel.getEl().clip();
                if(this.activePanel.beforeSlide){
                    this.activePanel.beforeSlide();
                }
            }
        }
    },
    
    afterSlide : function(){
        if(YAHOO.ext.util.Browser.isGecko){// firefox overflow auto bug workaround
            if(this.tabs) this.tabs.bodyEl.unclip();
            if(this.activePanel){
                this.activePanel.getEl().unclip();
                if(this.activePanel.afterSlide){
                    this.activePanel.afterSlide();
                }
            }
        }
    },
    
    slideOut : function(){
        if(!this.slideEl){
            this.slideEl = new YAHOO.ext.Actor(
                YAHOO.ext.DomHelper.append(this.mgr.el.dom, {tag: 'div', cls:'ylayout-slider'}));
            if(this.config.autoHide !== false){
                var slideInTask = new YAHOO.ext.util.DelayedTask(this.slideIn, this);
                this.slideEl.mon('mouseout', function(e){
                    var to = e.getRelatedTarget();
                    if(to && to != this.slideEl.dom && !YAHOO.util.Dom.isAncestor(this.slideEl.dom, to)){
                        slideInTask.delay(500);
                    }
                }, this, true);
                this.slideEl.mon('mouseover', function(e){
                    slideInTask.cancel();
                }, this, true);
            }
        }
        var sl = this.slideEl, c = this.collapsedEl, cm = this.cmargins;
        this.isSlid = true;
        this.snapshot = {
            'left': this.el.getLeft(true),
            'top': this.el.getTop(true),
            'colbtn': this.collapseBtn.isVisible(),
            'closebtn': this.closeBtn.isVisible(),
            'elclip': this.el.getStyle('overflow')
        };
        this.collapseBtn.hide();
        this.closeBtn.hide();
        this.beforeSlide();
        this.el.show();
        this.el.setLeftTop(0,0);
        //sl.setStyle('background', 'red')
        sl.startCapture(true);
        var size;
        switch(this.position){
            case 'west':
                sl.setLeft(c.getRight(true));
                sl.setTop(c.getTop(true));
                size = this.el.getWidth();
            break;
            case 'east':
                sl.setRight(this.mgr.getViewSize().width-c.getLeft(true));
                sl.setTop(c.getTop(true));
                size = this.el.getWidth();
            break;
            case 'north':
                sl.setLeft(c.getLeft(true));
                sl.setTop(c.getBottom(true));
                size = this.el.getHeight();
            break;
            case 'south':
                sl.setLeft(c.getLeft(true));
                sl.setBottom(this.mgr.getViewSize().height-c.getTop(true));
                size = this.el.getHeight();
            break;
        }
        sl.dom.appendChild(this.el.dom);
        YAHOO.util.Event.on(document.body, 'click', this.slideInIf, this, true);
        sl.setSize(this.el.getWidth(), this.el.getHeight());
        sl.slideShow(this.getAnchor(), size, this.slideDuration, null, false);
        sl.play();
    },
    
    slideInIf : function(e){
        var t = YAHOO.util.Event.getTarget(e);
        if(!YAHOO.util.Dom.isAncestor(this.el.dom, t)){
            this.slideIn();
        }
    },
    
    slideIn : function(callback){
        if(this.isSlid && !this.slideEl.playlist.isPlaying()){
            YAHOO.util.Event.removeListener(document.body, 'click', this.slideInIf, this, true);
            this.slideEl.startCapture(true);
            this.slideEl.slideHide(this.getAnchor(), this.slideDuration, null);
            this.slideEl.play(function(){
                this.isSlid = false;
                this.el.setPositioning(this.snapshot);
                this.collapseBtn.setVisible(this.snapshot.colbtn);
                this.closeBtn.setVisible(this.snapshot.closebtn);
                this.afterSlide();
                this.mgr.el.dom.appendChild(this.el.dom);
                if(typeof callback == 'function'){
                    callback();
                }
            }.createDelegate(this));
        }
    },
    
    animateExpand : function(){
        var em = this.margins, cm = this.cmargins;
        var c = this.collapsedEl, el = this.el;
        var direction, distance;
        switch(this.position){
            case 'west':
                direction = 'right';
                el.setLeft(-(el.getWidth() + (em.right+em.left)));
                el.setTop(c.getTop(true)-cm.top+em.top);
                distance = el.getWidth() + (em.right+em.left);
            break;
            case 'east':
                direction = 'left';
                el.setLeft(this.mgr.getViewSize().width + em.left);
                el.setTop(c.getTop(true)-cm.top+em.top);
                distance = el.getWidth() + (em.right+em.left);
            break;
            case 'north':
                direction = 'down';
                el.setLeft(em.left);
                el.setTop(-(el.getHeight() + (em.top+em.bottom)));
                distance = el.getHeight() + (em.top+em.bottom);
            break;
            case 'south':
                direction = 'up';
                el.setLeft(em.left);
                el.setTop(this.mgr.getViewSize().height + em.top);
                distance = el.getHeight() + (em.top+em.bottom);
            break;
        }
        this.beforeSlide();
        el.setStyle('z-index', '100');
        el.show();
        c.setLocation(-2000,-2000);
        c.hide();
        el.move(direction, distance, true, this.duration, function(){
            this.afterSlide();
            el.setStyle('z-index', '');
            if(this.split){
                this.split.el.show();
            }
            this.fireEvent('invalidated', this);
            this.fireEvent('expanded', this);
        }.createDelegate(this), this.config.easing || YAHOO.util.Easing.easeOut);
    },
    
    animateCollapse : function(){
        var em = this.margins, cm = this.cmargins;
        var c = this.collapsedEl, el = this.el;
        var direction, distance;
        switch(this.position){
            case 'west':
                direction = 'left';
                distance = el.getWidth() + (em.right+em.left);
            break;
            case 'east':
                direction = 'right';
                distance = el.getWidth() + (em.right+em.left);
            break;
            case 'north':
                direction = 'up';
                distance = el.getHeight() + (em.top+em.bottom);
            break;
            case 'south':
                direction = 'down';
                distance = el.getHeight() + (em.top+em.bottom);
            break;
        }
        this.el.setStyle('z-index', '100');
        this.beforeSlide();
        this.el.move(direction, distance, true, this.duration, function(){
            this.afterSlide();
            this.el.setStyle('z-index', '');
            this.el.setLocation(-20000,-20000);
            this.el.hide();
            this.collapsedEl.show();
            this.fireEvent('collapsed', this); 
        }.createDelegate(this), YAHOO.util.Easing.easeIn);
    },
    
    getAnchor : function(){
        switch(this.position){
            case 'west':
                return 'left';
            case 'east':
                return 'right';
            case 'north':
                return 'top';
            case 'south':
                return 'bottom';
        }
    }
});

/*
------------------------------------------------------------------
// File: layout\BorderLayout.js
------------------------------------------------------------------
*/

YAHOO.ext.BorderLayout = function(container, config){
    YAHOO.ext.BorderLayout.superclass.constructor.call(this, container);
    this.factory = config.factory || YAHOO.ext.BorderLayout.RegionFactory;
    this.hideOnLayout = config.hideOnLayout || false;
    for(var i = 0, len = this.factory.validRegions.length; i < len; i++) {
    	var target = this.factory.validRegions[i];
    	if(config[target]){
    	    this.addRegion(target, config[target]);
    	}
    }
    //this.dragOverDelegate = YAHOO.ext.EventManager.wrap(this.onDragOver, this, true);
};

YAHOO.extendX(YAHOO.ext.BorderLayout, YAHOO.ext.LayoutManager, {
    addRegion : function(target, config){
        if(!this.regions[target]){
            var r = this.factory.create(target, this, config);
    	    this.regions[target] = r;
    	    r.on('visibilitychange', this.layout, this, true);
            r.on('paneladded', this.layout, this, true);
            r.on('panelremoved', this.layout, this, true);
            r.on('invalidated', this.layout, this, true);
            r.on('resized', this.onRegionResized, this, true);
            r.on('collapsed', this.onRegionCollapsed, this, true);
            r.on('expanded', this.onRegionExpanded, this, true);
        }
        return this.regions[target];
    },
    
    layout : function(){
        if(this.updating) return;
        //var bench = new YAHOO.ext.util.Bench();
	    //bench.start('Layout...');
        var size = this.getViewSize();
        var w = size.width, h = size.height;
        var centerW = w, centerH = h, centerY = 0, centerX = 0;
        var x = 0, y = 0;
        
        var rs = this.regions;
        var n = rs['north'], s = rs['south'], west = rs['west'], e = rs['east'], c = rs['center'];
        if(this.hideOnLayout){
            c.el.setStyle('display', 'none');
        }
        if(n && n.isVisible()){
            var b = n.getBox();
            var m = n.getMargins();
            b.width = w - (m.left+m.right);
            b.x = m.left;
            b.y = m.top;
            centerY = b.height + b.y + m.bottom;
            centerH -= centerY;
            n.updateBox(this.safeBox(b));
        }
        if(s && s.isVisible()){
            var b = s.getBox();
            var m = s.getMargins();
            b.width = w - (m.left+m.right);
            b.x = m.left;
            var totalHeight = (b.height + m.top + m.bottom);
            b.y = h - totalHeight + m.top;
            centerH -= totalHeight;
            s.updateBox(this.safeBox(b));
        }
        if(west && west.isVisible()){
            var b = west.getBox();
            var m = west.getMargins();
            b.height = centerH - (m.top+m.bottom);
            b.x = m.left;
            b.y = centerY + m.top;
            var totalWidth = (b.width + m.left + m.right);
            centerX += totalWidth;
            centerW -= totalWidth;
            west.updateBox(this.safeBox(b));
        }
        if(e && e.isVisible()){
            var b = e.getBox();
            var m = e.getMargins();
            b.height = centerH - (m.top+m.bottom);
            var totalWidth = (b.width + m.left + m.right);
            b.x = w - totalWidth + m.left;
            b.y = centerY + m.top;
            centerW -= totalWidth;
            e.updateBox(this.safeBox(b));
        }
        if(c){
            var m = c.getMargins();
            var centerBox = {
                x: centerX + m.left,
                y: centerY + m.top,
                width: centerW - (m.left+m.right),
                height: centerH - (m.top+m.bottom)
            };
            if(this.hideOnLayout){
                c.el.setStyle('display', 'block');
            }
            c.updateBox(this.safeBox(centerBox));
        }
        //this.el.repaint();
        this.fireEvent('layout', this);
        //bench.stop();
	    //alert(bench.toString());
    },
    
    safeBox : function(box){
        box.width = Math.max(0, box.width);
        box.height = Math.max(0, box.height);
        return box;
    },
    
    add : function(target, panel){
        target = target.toLowerCase();
        return this.regions[target].add(panel);
    },
    
    remove : function(target, panel){
        target = target.toLowerCase();
        return this.regions[target].remove(panel);
    },
    
    /**
     * Searches all regions for a panel with the specified id
     */
    findPanel : function(panelId){
        var rs = this.regions;
        for(var target in rs){
            if(typeof rs[target] != 'function'){
                var p = rs[target].getPanel(panelId);
                if(p){
                    return p;
                }
            }
        }
        return null;
    },
    
    showPanel : function(panelId) {
      var rs = this.regions;
      for(var target in rs){
         if(typeof rs[target] != 'function'){
            if(rs[target].getPanel(panelId)){
               rs[target].showPanel(panelId);
            }
         }
      }
   },
   
   restoreState : function(provider){
        if(!provider){
            provider = YAHOO.ext.state.Manager;
        }
        var sm = new YAHOO.ext.LayoutStateManager();
        sm.init(this, provider);
    }
});

YAHOO.ext.BorderLayout.RegionFactory = {};
YAHOO.ext.BorderLayout.RegionFactory.validRegions = ['north','south','east','west','center'];
YAHOO.ext.BorderLayout.RegionFactory.create = function(target, mgr, config){
    target = target.toLowerCase();
    switch(target){
        case 'north':
            return new YAHOO.ext.NorthLayoutRegion(mgr, config);
        case 'south':
            return new YAHOO.ext.SouthLayoutRegion(mgr, config);
        case 'east':
            return new YAHOO.ext.EastLayoutRegion(mgr, config);
        case 'west':
            return new YAHOO.ext.WestLayoutRegion(mgr, config);
        case 'center':
            return new YAHOO.ext.CenterLayoutRegion(mgr, config);
    }
    throw 'Layout region "'+target+'" not supported.';
};

/*
------------------------------------------------------------------
// File: layout\BorderLayoutRegions.js
------------------------------------------------------------------
*/

YAHOO.ext.CenterLayoutRegion = function(mgr, config){
    YAHOO.ext.CenterLayoutRegion.superclass.constructor.call(this, mgr, config, 'center');
    this.visible = true;
    this.minWidth = config.minWidth || 20;
    this.minHeight = config.minHeight || 20;
};

YAHOO.extendX(YAHOO.ext.CenterLayoutRegion, YAHOO.ext.LayoutRegion, {
    hide : function(){
        // center panel can't be hidden
    },
    
    show : function(){
        // center panel can't be hidden
    },
    
    getMinWidth: function(){
        return this.minWidth;
    },
    
    getMinHeight: function(){
        return this.minHeight;
    }
});


YAHOO.ext.NorthLayoutRegion = function(mgr, config){
    YAHOO.ext.NorthLayoutRegion.superclass.constructor.call(this, mgr, config, 'north', 'n-resize');
    if(this.split){
        this.split.placement = YAHOO.ext.SplitBar.TOP;
        this.split.orientation = YAHOO.ext.SplitBar.VERTICAL;
        this.split.el.addClass('ylayout-split-v');
    }
    if(typeof config.initialSize != 'undefined'){
        this.el.setHeight(config.initialSize);
    }
};
YAHOO.extendX(YAHOO.ext.NorthLayoutRegion, YAHOO.ext.SplitLayoutRegion, {
    getBox : function(){
        if(this.collapsed){
            return this.collapsedEl.getBox();
        }
        var box = this.el.getBox();
        if(this.split){
            box.height += this.split.el.getHeight();
        }
        return box;
    },
    
    updateBox : function(box){
        if(this.split && !this.collapsed){
            box.height -= this.split.el.getHeight();
            this.split.el.setLeft(box.x);
            this.split.el.setTop(box.y+box.height);
            this.split.el.setWidth(box.width);
        }
        if(this.collapsed){
            this.el.setWidth(box.width);
            var bodyWidth = box.width - this.el.getBorderWidth('rl');
            this.bodyEl.setWidth(bodyWidth);
            if(this.activePanel && this.panelSize){
                this.activePanel.setSize(bodyWidth, this.panelSize.height);
            }
        }
        YAHOO.ext.NorthLayoutRegion.superclass.updateBox.call(this, box);
    }
});

YAHOO.ext.SouthLayoutRegion = function(mgr, config){
    YAHOO.ext.SouthLayoutRegion.superclass.constructor.call(this, mgr, config, 'south', 's-resize');
    if(this.split){
        this.split.placement = YAHOO.ext.SplitBar.BOTTOM;
        this.split.orientation = YAHOO.ext.SplitBar.VERTICAL;
        this.split.el.addClass('ylayout-split-v');
    }
    if(typeof config.initialSize != 'undefined'){
        this.el.setHeight(config.initialSize);
    }
};
YAHOO.extendX(YAHOO.ext.SouthLayoutRegion, YAHOO.ext.SplitLayoutRegion, {
    getBox : function(){
        if(this.collapsed){
            return this.collapsedEl.getBox();
        }
        var box = this.el.getBox();
        if(this.split){
            var sh = this.split.el.getHeight();
            box.height += sh;
            box.y -= sh;
        }
        return box;
    },
    
    updateBox : function(box){
        if(this.split && !this.collapsed){
            var sh = this.split.el.getHeight();
            box.height -= sh;
            box.y += sh;
            this.split.el.setLeft(box.x);
            this.split.el.setTop(box.y-sh);
            this.split.el.setWidth(box.width);
        }
        if(this.collapsed){
            this.el.setWidth(box.width);
            var bodyWidth = box.width - this.el.getBorderWidth('rl');
            this.bodyEl.setWidth(bodyWidth);
            if(this.activePanel && this.panelSize){
                this.activePanel.setSize(bodyWidth, this.panelSize.height);
            }
        }
        YAHOO.ext.SouthLayoutRegion.superclass.updateBox.call(this, box);
    }
});

YAHOO.ext.EastLayoutRegion = function(mgr, config){
    YAHOO.ext.EastLayoutRegion.superclass.constructor.call(this, mgr, config, 'east', 'e-resize');
    if(this.split){
        this.split.placement = YAHOO.ext.SplitBar.RIGHT;
        this.split.orientation = YAHOO.ext.SplitBar.HORIZONTAL;
        this.split.el.addClass('ylayout-split-h');
    }
    if(typeof config.initialSize != 'undefined'){
        this.el.setWidth(config.initialSize);
    }
};
YAHOO.extendX(YAHOO.ext.EastLayoutRegion, YAHOO.ext.SplitLayoutRegion, {
    getBox : function(){
        if(this.collapsed){
            return this.collapsedEl.getBox();
        }
        var box = this.el.getBox();
        if(this.split){
            var sw = this.split.el.getWidth();
            box.width += sw;
            box.x -= sw;
        }
        return box;
    },
    
    updateBox : function(box){
        if(this.split && !this.collapsed){
            var sw = this.split.el.getWidth();
            box.width -= sw;
            this.split.el.setLeft(box.x);
            this.split.el.setTop(box.y);
            this.split.el.setHeight(box.height);
            box.x += sw;
        }
        if(this.collapsed){
            this.el.setHeight(box.height);
            var bodyHeight = box.height - this.el.getBorderWidth('tb');
            this.bodyEl.setHeight(bodyHeight);
            if(this.activePanel && this.panelSize){
                this.activePanel.setSize(this.panelSize.width, bodyHeight);
            }
        }
        YAHOO.ext.EastLayoutRegion.superclass.updateBox.call(this, box);
    }
});

YAHOO.ext.WestLayoutRegion = function(mgr, config){
    YAHOO.ext.WestLayoutRegion.superclass.constructor.call(this, mgr, config, 'west', 'w-resize');
    if(this.split){
        this.split.placement = YAHOO.ext.SplitBar.LEFT;
        this.split.orientation = YAHOO.ext.SplitBar.HORIZONTAL;
        this.split.el.addClass('ylayout-split-h');
    }
    if(typeof config.initialSize != 'undefined'){
        this.el.setWidth(config.initialSize);
    }
};
YAHOO.extendX(YAHOO.ext.WestLayoutRegion, YAHOO.ext.SplitLayoutRegion, {
    getBox : function(){
        if(this.collapsed){
            return this.collapsedEl.getBox();
        }
        var box = this.el.getBox();
        if(this.split){
            box.width += this.split.el.getWidth();
        }
        return box;
    },
    
    updateBox : function(box){
        if(this.split && !this.collapsed){
            var sw = this.split.el.getWidth();
            box.width -= sw;
            this.split.el.setLeft(box.x+box.width);
            this.split.el.setTop(box.y);
            this.split.el.setHeight(box.height);
        }
        if(this.collapsed){
            this.el.setHeight(box.height);
            var bodyHeight = box.height - this.el.getBorderWidth('tb');
            this.bodyEl.setHeight(bodyHeight);
            if(this.activePanel && this.panelSize){
                this.activePanel.setSize(this.panelSize.width, bodyHeight);
            }
        }
        YAHOO.ext.WestLayoutRegion.superclass.updateBox.call(this, box);
    }
});


/*
------------------------------------------------------------------
// File: layout\ContentPanels.js
------------------------------------------------------------------
*/
YAHOO.ext.ContentPanel = function(el, config, content){
    YAHOO.ext.ContentPanel.superclass.constructor.call(this);
    this.el = getEl(el, true);
    if(!this.el && config && config.autoCreate){
        if(typeof config.autoCreate == 'object'){
            if(!config.autoCreate.id){
                config.autoCreate.id = el;
            }
            this.el = YAHOO.ext.DomHelper.append(document.body,
                        config.autoCreate, true);
        }else{
            this.el = YAHOO.ext.DomHelper.append(document.body,
                        {tag: 'div', cls: 'ylayout-inactive-content', id: el}, true);
        }
    }
    this.closable = false;
    this.loaded = false;
    this.active = false;
    if(typeof config == 'string'){
        this.title = config;
    }else{
        YAHOO.ext.util.Config.apply(this, config);
    }
    if(this.resizeEl){
        this.resizeEl = getEl(this.resizeEl, true);
    }else{
        this.resizeEl = this.el;
    }
    this.events = {
        'activate' : new YAHOO.util.CustomEvent('activate'),
        'deactivate' : new YAHOO.util.CustomEvent('deactivate') 
    };
    if(content){
        this.setContent(content);
    }
};

YAHOO.extendX(YAHOO.ext.ContentPanel, YAHOO.ext.util.Observable, {
    setRegion : function(region){
        this.region = region;
        if(region){
           this.el.replaceClass('ylayout-inactive-content', 'ylayout-active-content'); 
        }else{
           this.el.replaceClass('ylayout-active-content', 'ylayout-inactive-content'); 
        } 
    },
    
    getToolbar : function(){
        return this.toolbar;
    },
    
    setActiveState : function(active){
        this.active = active;
        if(!active){
            this.fireEvent('deactivate', this);
        }else{
            this.fireEvent('activate', this);
        }
    },
    
    /**
     * Updates this panel's element
     */
    setContent : function(content, loadScripts){
        this.el.update(content, loadScripts);
    },
    
    /**
     * Get the {@link YAHOO.ext.UpdateManager} for this panel. Enables you to perform Ajax updates.
     * @return {YAHOO.ext.UpdateManager} The UpdateManager
     */
    getUpdateManager : function(){
        return this.el.getUpdateManager();
    },
    
    /**
     * Set a URL to be used to load the content for this panel.
     * @param {String/Function} url The url to load the content from or a function to call to get the url
     * @param {<i>String/Object</i>} params (optional) The string params for the update call or an object of the params. See {@link YAHOO.ext.UpdateManager#update} for more details. (Defaults to null)
     * @param {<i>Boolean</i>} loadOnce (optional) Whether to only load the content once. If this is false it makes the Ajax call every time this panel is activated. (Defaults to false)
     * @return {YAHOO.ext.UpdateManager} The UpdateManager
     */
    setUrl : function(url, params, loadOnce){
        if(this.refreshDelegate){
            this.removeListener('activate', this.refreshDelegate);
        }
        this.refreshDelegate = this._handleRefresh.createDelegate(this, [url, params, loadOnce]);
        this.on('activate', this._handleRefresh.createDelegate(this, [url, params, loadOnce]));
    },
    
    /** @private */
    _handleRefresh : function(url, params, loadOnce){
        if(!loadOnce || !this.loaded){
            var updater = this.el.getUpdateManager();
            updater.update(url, params, this._setLoaded.createDelegate(this));
        }
    },
    
    /** @private */
    _setLoaded : function(){
        this.loaded = true;
    }, 
    
    getId : function(){
        return this.el.id;
    },
    
    getEl : function(){
        return this.el;
    },
    
    adjustForComponents : function(width, height){
        if(this.toolbar){
            height -= this.toolbar.getEl().getHeight();
        }
        return {'width': width, 'height': height};
    },
    
    setSize : function(width, height){
        if(this.fitToFrame){
            var size = this.adjustForComponents(width, height);
            this.resizeEl.setSize(size.width, size.height);
        }
    },
    
    getTitle : function(){
        return this.title;
    },
    
    setTitle : function(title){
        this.title = title;
        if(this.region){
            this.region.updatePanelTitle(this, title);
        }
    },
    
    isClosable : function(){
        return this.closable;
    },
    
    beforeSlide : function(){
        this.el.clip();
        this.resizeEl.clip();
    },
    
    afterSlide : function(){
        this.el.unclip();
        this.resizeEl.unclip();
    }
});


YAHOO.ext.GridPanel = function(grid, config){
    this.wrapper = YAHOO.ext.DomHelper.append(document.body, // wrapper for IE7 strict & safari scroll issue
        {tag: 'div', cls: 'ylayout-grid-wrapper ylayout-inactive-content'}, true);
    this.wrapper.dom.appendChild(grid.container.dom);
    YAHOO.ext.GridPanel.superclass.constructor.call(this, this.wrapper, config);
    if(this.toolbar){
        this.toolbar.el.insertBefore(this.wrapper.dom.firstChild);
    }
    grid.monitorWindowResize = false; // turn off autosizing
    this.grid = grid;
    this.grid.container.replaceClass('ylayout-inactive-content', 'ylayout-component-panel');
};

YAHOO.extendX(YAHOO.ext.GridPanel, YAHOO.ext.ContentPanel, {
    getId : function(){
        return this.grid.id;
    },
    
    getGrid : function(){
        return this.grid;    
    },
    
    setSize : function(width, height){
        var grid = this.grid;
        var size = this.adjustForComponents(width, height);
        grid.container.setSize(size.width, size.height);
        grid.autoSize();
    },
    
    beforeSlide : function(){
        this.grid.getView().wrapEl.clip();
    },
    
    afterSlide : function(){
        this.grid.getView().wrapEl.unclip();
    }
});


YAHOO.ext.NestedLayoutPanel = function(layout, config){
    YAHOO.ext.NestedLayoutPanel.superclass.constructor.call(this, layout.getEl(), config);
    layout.monitorWindowResize = false; // turn off autosizing
    this.layout = layout;
    this.layout.getEl().addClass('ylayout-nested-layout');
};

YAHOO.extendX(YAHOO.ext.NestedLayoutPanel, YAHOO.ext.ContentPanel, {
    setSize : function(width, height){
        var size = this.adjustForComponents(width, height);
        this.layout.getEl().setSize(size.width, size.height);
        this.layout.layout();
    },
    
    getLayout : function(){
        return this.layout;
    }
});

/*
------------------------------------------------------------------
// File: layout\LayoutStateManager.js
------------------------------------------------------------------
*/
YAHOO.ext.LayoutStateManager = function(layout){
     // default empty state
     this.state = {
        north: {},
        south: {},
        east: {},
        west: {}       
    };
};

YAHOO.ext.LayoutStateManager.prototype = {
    init : function(layout, provider){
        this.provider = provider;
        var state = provider.get(layout.id+'-layout-state');
        if(state){
            var wasUpdating = layout.isUpdating();
            if(!wasUpdating){
                layout.beginUpdate();
            }
            for(var key in state){
                if(typeof state[key] != 'function'){
                    var rstate = state[key];
                    var r = layout.getRegion(key);
                    if(r && rstate){
                        if(rstate.size){
                            r.resizeTo(rstate.size);
                        }
                        if(rstate.collapsed == true){
                            r.collapse(true);
                        }else{
                            r.expand(null, true);
                        }
                    }
                }
            }
            if(!wasUpdating){
                layout.endUpdate();
            }
            this.state = state; 
        }
        this.layout = layout;
        layout.on('regionresized', this.onRegionResized, this, true);
        layout.on('regioncollapsed', this.onRegionCollapsed, this, true);
        layout.on('regionexpanded', this.onRegionExpanded, this, true);
    },
    
    storeState : function(){
        this.provider.set(this.layout.id+'-layout-state', this.state);
    },
    
    onRegionResized : function(region, newSize){
        this.state[region.getPosition()].size = newSize;
        this.storeState();
    },
    
    onRegionCollapsed : function(region){
        this.state[region.getPosition()].collapsed = true;
        this.storeState();
    },
    
    onRegionExpanded : function(region){
        this.state[region.getPosition()].collapsed = false;
        this.storeState();
    }
};

/*
------------------------------------------------------------------
// File: widgets\BasicDialog.js
------------------------------------------------------------------
*/
/*
 * YUI Extensions 0.33 beta
 * Copyright(c) 2006, Jack Slocum.
 * 
 * This code is licensed under BSD license. 
 * http://www.opensource.org/licenses/bsd-license.php
 */

/**
 * @class YAHOO.ext.BasicDialog
 * Lightweight Dialog Class
 * Supports the following configuration options (listed with default value):
 * width: auto
 * height: auto
 * x: (center screen x)
 * y: (center screen y)
 * animateTarget: null (no animation) This is the id or element to animate from
 * resizable: true
 * minHeight: 80
 * minWidth: 200
 * modal: false
 * autoScroll: true
 * closable: true
 * constraintoviewport: true
 * draggable: true
 * autoTabs: false (if true searches child nodes for elements with class ydlg-tab and converts them to tabs)
 * proxyDrag: false (drag a proxy element rather than the dialog itself)
 * fixedcenter: false
 * shadow: false
 * minButtonWidth: 75 
 * 
 * It has the following events (with handlers arguments in parens):
 * keydown (this, e)
 * resize (this, width, height)
 * move (this, x, y)
 * beforehide (this) (this can be cancelled by returning false in a subscriber method)
 * hide (this)
 * beforeshow (this) (this can be cancelled by returning false in a subscriber method)
 * show (this)
 * 
 * @constructor
 * @param {String/HTMLElement/YAHOO.ext.Element} el The id of or container element
 * @param {Object} config configuration options
 */
YAHOO.ext.BasicDialog = function(el, config){
    this.el = getEl(el, true);
    this.id = this.el.id;
    this.el.addClass('ydlg');
    this.shadowOffset = 3;
    this.minHeight = 80;
    this.minWidth = 200;
    this.minButtonWidth = 75;
    
    YAHOO.ext.util.Config.apply(this, config);
    
    this.proxy = this.el.createProxy('ydlg-proxy');
    this.proxy.enableDisplayMode('block');
    this.proxy.setOpacity(.5);
    
    if(config.width){
        this.el.setWidth(config.width);
    }
    if(config.height){
        this.el.setHeight(config.height);
    }
    this.size = this.el.getSize();
    if(typeof config.x != 'undefined' && typeof config.y != 'undefined'){
        this.xy = [config.x,config.y];
    }else{
        this.xy = this.el.getCenterXY(true);
    }
    // find the header, body and footer
    var cn = this.el.dom.childNodes;
    for(var i = 0, len = cn.length; i < len; i++) {
    	var node = cn[i];
    	if(node && node.nodeType == 1){
    	    if(YAHOO.util.Dom.hasClass(node, 'ydlg-hd')){
    	        this.header = getEl(node, true);
    	    }else if(YAHOO.util.Dom.hasClass(node, 'ydlg-bd')){
    	        this.body = getEl(node, true);
    	    }else if(YAHOO.util.Dom.hasClass(node, 'ydlg-ft')){
    	        this.footer = getEl(node, true);
    	    }
    	}
    }
    
    var dh = YAHOO.ext.DomHelper;
    if(!this.header){
        this.header = dh.append(this.el.dom, {tag: 'div', cls:'ydlg-hd'}, true);
    }
    if(!this.body){
        this.body = dh.append(this.el.dom, {tag: 'div', cls:'ydlg-bd'}, true);
    }
    // wrap the header for special rendering
    var hl = dh.insertBefore(this.header.dom, {tag: 'div', cls:'ydlg-hd-left'});
    var hr = dh.append(hl, {tag: 'div', cls:'ydlg-hd-right'});
    hr.appendChild(this.header.dom);
    
    // wrap the body and footer for special rendering
    this.bwrap = dh.insertBefore(this.body.dom, {tag: 'div', cls:'ydlg-dlg-body'}, true);
    this.bwrap.dom.appendChild(this.body.dom);
    if(this.footer) this.bwrap.dom.appendChild(this.footer.dom);
    
    if(this.autoScroll !== false && !this.autoTabs){
        this.body.setStyle('overflow', 'auto');
    }
    if(this.closable !== false){
        this.el.addClass('ydlg-closable');
        this.close = dh.append(this.el.dom, {tag: 'div', cls:'ydlg-close'}, true);
        this.close.mon('click', function(){
            this.hide();
        }, this, true);
    }
    if(this.resizable !== false){
        this.el.addClass('yresizable-pinned');
        this.el.addClass('ydlg-resizable');
        this.resizer = new YAHOO.ext.Resizable(this.el, {
            minWidth: this.minWidth || 80, 
            minHeight:this.minHeight || 80, 
            disableTrackOver:true, 
            multiDirectional: true
        });
        this.resizer.proxy.setStyle('z-index', parseInt(this.el.getStyle('z-index'),10)+1);
        this.resizer.on('beforeresize', this.beforeResize, this, true);
        this.resizer.delayedListener('resize', this.onResize, this, true);
    }
    if(this.draggable !== false){
        this.el.addClass('ydlg-draggable');
        if (!this.proxyDrag) {
            var dd = new YAHOO.util.DD(this.el.dom, 'WindowDrag');
        }
        else {
            var dd = new YAHOO.util.DDProxy(this.el.dom, 'WindowDrag', {dragElId: this.proxy.id});
        }
        dd.setHandleElId(this.header.id);
        dd.endDrag = this.endMove.createDelegate(this);
        dd.startDrag = this.startMove.createDelegate(this);
        dd.onDrag = this.adjustShadow.createDelegate(this);
        this.dd = dd;
    }
    if(this.modal){
        this.mask = dh.append(document.body, {tag: 'div', cls:'ydlg-mask'}, true);
        this.mask.originalDisplay = 'block';
        this.mask.enableDisplayMode();
    }
    if(this.shadow){
        this.shadow = dh.append(document.body, {tag: 'div', cls:'ydlg-shadow'}, true);
        this.shadow.setOpacity(.3);
        this.shadow.setAbsolutePositioned(10000);
        this.shadow.enableDisplayMode('block');
        this.shadow.hide();
    }
    if(this.autoTabs){
        var tabEls = YAHOO.util.Dom.getElementsByClassName('ydlg-tab', 'div', this.el.dom);
        if(tabEls.length > 0){
            this.body.addClass(this.tabPosition == 'bottom' ? 'ytabs-bottom' : 'ytabs-top');
            this.tabs = new YAHOO.ext.TabPanel(this.body.dom, this.tabPosition == 'bottom');
            for(var i = 0, len = tabEls.length; i < len; i++) {
            	var tabEl = tabEls[i];
            	this.tabs.addTab(YAHOO.util.Dom.generateId(tabEl), tabEl.title);
            	tabEl.title = '';
            }
            this.tabs.activate(tabEls[0].id);
        }
    }
    this.syncBodyHeight();
    this.events = {
        'keydown' : new YAHOO.util.CustomEvent('keydown'),
        'move' : new YAHOO.util.CustomEvent('move'),
        'resize' : new YAHOO.util.CustomEvent('resize'),
        'beforehide' : new YAHOO.util.CustomEvent('beforehide'),
        'hide' : new YAHOO.util.CustomEvent('hide'),
        'beforeshow' : new YAHOO.util.CustomEvent('beforeshow'),
        'show' : new YAHOO.util.CustomEvent('show')
    };
    
    this.keyDownDelegate = YAHOO.ext.EventManager.wrap(this.onKeyDown, this, true);
    YAHOO.ext.EventManager.onWindowResize(this.adjustViewport, this, true);
    this.preloaded = false;
    this.el.setDisplayed(false);
    this.defaultButton = null;
};

YAHOO.extendX(YAHOO.ext.BasicDialog, YAHOO.ext.util.Observable, {
    beforeResize : function(){
        this.resizer.minHeight = Math.max(this.minHeight, this.getHeaderFooterHeight(true)+40);
    },
    
    onResize : function(){
        this.refreshSize();
        this.syncBodyHeight();
        this.adjustShadow();
        this.fireEvent('resize', this, this.size.width, this.size.height);
    },
    
    onKeyDown : function(e){
        this.fireEvent('keydown', this, e);
    },
    
    /**
     * @method addKeyListener
     * Adds a key listener for when this dialog is displayed
     * @param {Number/Array/Object} key Either the numeric key code, array of key codes or an object with the following options: 
     *                                  {key: (number or array), shift: (true/false), ctrl: (true/false), alt: (true/false)}
     * @param {Function} fn The function to call
     * @param {Object} scope (optional) The scope of the function
     */
    addKeyListener : function(key, fn, scope){
        var keyCode, shift, ctrl, alt;
        if(typeof key == 'object'){
            keyCode = key['key'];
            shift = key['shift'];
            ctrl = key['ctrl'];
            alt = key['alt'];
        }else{
            keyCode = key;
        }
        var handler = function(dlg, e){
            if((!shift || e.shiftKey) && (!ctrl || e.ctrlKey) &&  (!alt || e.altKey)){
                var k = e.getKey();
                if(keyCode instanceof Array){
                    for(var i = 0, len = keyCode.length; i < len; i++){
                        if(keyCode[i] == k){
                          fn.call(scope || window, dlg, k, e);
                          return;
                        }
                    }
                }else{
                    if(k == keyCode){
                        fn.call(scope || window, dlg, k, e);
                    }
                }
            }
        };
        this.on('keydown', handler);
    },
    
    /**
     * @method getTabs
     * Returns the TabPanel component (if autoTabs)
     */
    getTabs : function(){
        if(!this.tabs){
            this.body.addClass(this.tabPosition == 'bottom' ? 'ytabs-bottom' : 'ytabs-top');
            this.tabs = new YAHOO.ext.TabPanel(this.body.dom, this.tabPosition == 'bottom');
        }
        return this.tabs;    
    },
    
    /**
     * @method addButton
     * Adds a button.
     * @param {String/Object} config A string becomes the button text, an object is expected to be a valid YAHOO.ext.DomHelper element config
     * @param {Function} handler The function called when the button is clicked
     * @param {Object} scope The scope of the handler function
     */
    addButton : function(config, handler, scope){
        var dh = YAHOO.ext.DomHelper;
        if(!this.footer){
            this.footer = dh.append(this.bwrap.dom, {tag: 'div', cls:'ydlg-ft'}, true);
        }
        var btn;
        if(typeof config == 'string'){
            if(!this.buttonTemplate){
                // hideous table template
                this.buttonTemplate = new YAHOO.ext.DomHelper.Template('<a href="#" class="ydlg-button-focus"><table border="0" cellpadding="0" cellspacing="0" class="ydlg-button-wrap"><tr><td class="ydlg-button-left">&nbsp;</td><td class="ydlg-button-center" unselectable="on">{0}</td><td class="ydlg-button-right">&nbsp;</td></tr></table></a>');
            }
            var btn = this.buttonTemplate.append(this.footer.dom, [config], true);
            var tbl = getEl(btn.dom.firstChild, true);
            if(this.minButtonWidth){
                 tbl.beginMeasure();
                 if(tbl.getWidth() < this.minButtonWidth){
                       tbl.setWidth(this.minButtonWidth);
                 }
                 tbl.endMeasure();
            }
        }else{
            btn = dh.append(this.footer.dom, config, true);
        }
        var bo = new YAHOO.ext.BasicDialog.Button(btn, handler, scope);
        this.syncBodyHeight();
        if(!this.preloaded){ // preload images
            btn.addClass('ydlg-button-over');
            btn.removeClass('ydlg-button-over');
            btn.addClass('ydlg-button-click');
            btn.removeClass('ydlg-button-click');
            this.preloaded = true;
        }
        if(!this.buttons){
            this.buttons = [];
        }
        this.buttons.push(bo);
        return bo;
    },
    
    setDefaultButton : function(btn){
        this.defaultButton = btn;  
    },
    
    getHeaderFooterHeight : function(safe){
        if(safe)this.el.beginMeasure();
        var height = 0;
        if(this.header){
           height += this.header.getHeight();
        }
        if(this.footer){
            var fm = this.footer.getMargins();
            height += (this.footer.getHeight()+fm.top+fm.bottom);
        }
        if(safe)this.el.endMeasure();
        height += this.bwrap.getPadding('tb')+this.bwrap.getBorderWidth('tb');
        return height;
    },
    
    syncBodyHeight : function(){
        this.el.beginMeasure();
        var height = this.size.height - this.getHeaderFooterHeight(false);
        var bm = this.body.getMargins();
        this.body.setHeight(height-(bm.top+bm.bottom));
        if(this.tabs){
            this.tabs.syncHeight();
        }
        this.bwrap.setHeight(this.size.height-this.header.getHeight());
        // 11/07/06 jvs update to set fixed width for IE7
        this.body.setWidth(this.el.getWidth(true)-this.bwrap.getBorderWidth('lr')-this.bwrap.getPadding('lr'));
        this.el.endMeasure();
    },
    
    /**
     * @method restoreState
     * Restores the previous state of the dialog if YAHOO.ext.state is configured
     */
    restoreState : function(){
        var box = YAHOO.ext.state.Manager.get(this.el.id + '-state');
        if(box && box.width){
            this.xy = [box.x, box.y];
            this.size = box;
            this.el.setLocation(box.x, box.y);
            this.resizer.resizeTo(box.width, box.height);
            this.adjustViewport();
        }else{
            this.resizer.resizeTo(this.size.width, this.size.height);
            this.adjustViewport();
        }
    },
    
    beforeShow : function(){
        YAHOO.util.Event.on(document, 'keydown', this.keyDownDelegate);
        
        if (this.fixedcenter) {
            this.el.beginMeasure();
            this.xy = this.el.getCenterXY();
            this.el.endMeasure();
        }
        if(this.modal){
            YAHOO.util.Dom.addClass(document.body, 'masked');
            this.mask.setSize(YAHOO.util.Dom.getViewportWidth(), YAHOO.util.Dom.getViewportHeight());
            this.mask.show();
        }
                
    },
    
    /**
     * @method show
     * Shows the dialog.
     * @param {String/HTMLElement/YAHOO.ext.Element} animateTarget Reset the animation target
     */
    show : function(animateTarget){
        if (this.fireEvent('beforeshow', this) === false)
            return;
        
        this.animateTarget = animateTarget || this.animateTarget;
        if(!this.el.isVisible()){
            this.beforeShow();
            if(this.animateTarget){
                var b = getEl(this.animateTarget, true).getBox();
                this.proxy.setLocation(b.x, b.y);
                this.proxy.setSize(b.width, b.height);
                this.proxy.show();
                this.proxy.setBounds(this.xy[0], this.xy[1], this.size.width, this.size.height, true, .35, this.showEl.createDelegate(this));
            }else{
                this.el.setDisplayed(true);
                this.el.setXY(this.xy);
                this.el.show();
                var box = this.el.getBox();
                if(this.shadow){
                    this.shadow.show();
                    this.shadow.setSize(box.width, box.height);
                    this.shadow.setLocation(box.x + this.shadowOffset, box.y + this.shadowOffset);
                }
                if(this.defaultButton) this.defaultButton.focus();
                this.fireEvent('show', this);
            }
        }
    },
    
    showEl : function(){
        var box = this.proxy.getBox();
        this.el.setDisplayed(true);
        this.el.setBox(box);
        this.el.show();
        this.proxy.hide();
        if(this.shadow){
            this.shadow.show();
            this.shadow.setSize(box.width, box.height);
            this.shadow.setLocation(box.x + this.shadowOffset, box.y + this.shadowOffset);
        }
        if(this.defaultButton) this.defaultButton.focus();
        this.fireEvent('show', this);
    },
    
    adjustViewport : function(width, height){
        this.viewSize = [width, height];
        if(this.modal && this.mask.isVisible()){
            this.mask.setSize(width, height);
        }
        var moved = false;
        if(this.xy[0] + this.size.width > this.viewSize[0]){
            this.xy[0] = Math.max(0, this.viewSize[0] - this.size.width);
            moved = true;
        }
        if(this.xy[1] + this.size.height > this.viewSize[1]){
            this.xy[1] = Math.max(0, this.viewSize[1] - this.size.height);
            moved = true;
        }
        if(moved){
            this.el.setXY(this.xy);
            this.adjustShadow();
        }
    },
    
    adjustShadow : function(){
        if(this.shadow){
            var box = this.el.getBox();
            box.x += this.shadowOffset;
            box.y += this.shadowOffset;
            this.shadow.setBox(box);
        }
    },
    
    startMove : function(){
        if(this.constraintoviewport != false){
            this.dd.resetConstraints();
            this.viewSize = [YAHOO.util.Dom.getViewportWidth(),YAHOO.util.Dom.getViewportHeight()];
            this.dd.setXConstraint(this.xy[0], this.viewSize[0]-this.xy[0]-this.el.getWidth()-this.shadowOffset);
            this.dd.setYConstraint(this.xy[1], this.viewSize[1]-this.xy[1]-this.el.getHeight()-this.shadowOffset);
        }
    },
    
    endMove : function(){
        YAHOO.util.DDProxy.prototype.endDrag.apply(this.dd, arguments);
        this.refreshSize();
        this.adjustShadow();
        this.fireEvent('move', this, this.xy[0], this.xy[1])
    },
   
    
    /**
     * @method isVisible
     * Returns true if the dialog is visible
     */
    isVisible : function(){
        return this.el.isVisible();    
    },
    
    beforeHide : function(){
        YAHOO.util.Event.removeListener(document, 'keydown', this.keyDownDelegate);
        if(this.modal){
            this.mask.hide();
            YAHOO.util.Dom.removeClass(document.body, 'masked');
        }
    },
    
    /**
     * @method hide
     * Hides the dialog.
     * @param {Function} callback Function to call when the dialog is hidden
     */
    hide : function(callback){
        if (this.fireEvent('beforehide', this) === false)
            return;
        
        this.beforeHide();
        if(this.shadow){
            this.shadow.hide();
        }
        if(this.animateTarget){
            this.proxy.setBounds(this.xy[0], this.xy[1], this.size.width, this.size.height);
            var b = getEl(this.animateTarget, true).getBox();
            this.proxy.show();
            this.el.setDisplayed(false);
            this.el.hide();
            this.proxy.setBounds(b.x, b.y, b.width, b.height, true, .35, this.hideEl.createDelegate(this, [callback]));
        }else{
            this.proxy.hide();
            this.el.setDisplayed(false);
            this.el.hide();
            this.fireEvent('hide', this);
        }
    },
    
    hideEl : function(callback){
        this.proxy.hide();
        this.fireEvent('hide', this);
        if(typeof callback == 'function'){
            callback();
        }
    },
    
    refreshSize : function(){
        this.size = this.el.getSize();
        this.xy = this.el.getXY();
        YAHOO.ext.state.Manager.set(this.el.id + '-state', this.el.getBox());
    }
});

YAHOO.ext.LayoutDialog = function(el, config){
    config.autoTabs = false;
    YAHOO.ext.LayoutDialog.superclass.constructor.call(this, el, config);
    this.body.setStyle({overflow:'hidden', position:'relative'});
    this.el.setDisplayed(true);
    this.layout = new YAHOO.ext.BorderLayout(this.body.dom, config);
    this.layout.monitorWindowResize = false;
};
YAHOO.extendX(YAHOO.ext.LayoutDialog, YAHOO.ext.BasicDialog, {
    endUpdate : function(){
        this.layout.endUpdate();
        this.el.setDisplayed(false);
    },
    beginUpdate : function(){
        this.layout.beginUpdate();
        this.el.setDisplayed(true);
    },
    getLayout : function(){
        return this.layout;
    },
    syncBodyHeight : function(){
        YAHOO.ext.LayoutDialog.superclass.syncBodyHeight.call(this);
        if(this.layout)this.layout.layout();
    }
});


YAHOO.ext.BasicDialog.Button = function(el, handler, scope){
    this.el = el;
    this.el.addClass('ydlg-button');
    this.el.mon('click', this.onClick, this, true);
    this.el.on('mouseover', this.onMouseOver, this, true);
    this.el.on('mouseout', this.onMouseOut, this, true);
    this.el.on('mousedown', this.onMouseDown, this, true);
    this.el.on('mouseup', this.onMouseUp, this, true);
    this.handler = handler;
    this.scope = scope;
    this.disabled = false;
};

YAHOO.ext.BasicDialog.Button.prototype = {
    getEl : function(){
        return this.el;  
    },
    
    focus : function(){
        this.el.focus();    
    },
    
    disable : function(){
        this.el.addClass('ydlg-button-disabled');
        this.disabled = true;
    },
    
    enable : function(){
        this.el.removeClass('ydlg-button-disabled');
        this.disabled = false;
    },
    
    onClick : function(e){
        e.preventDefault();
        if(!this.disabled){
            this.handler.call(this.scope || window);
        }
    },
    onMouseOver : function(){
        if(!this.disabled){
            this.el.addClass('ydlg-button-over');
        }
    },
    onMouseOut : function(){
        this.el.removeClass('ydlg-button-over');
    },
    onMouseDown : function(){
        if(!this.disabled){
            this.el.addClass('ydlg-button-click');
        }
    },
    onMouseUp : function(){
        this.el.removeClass('ydlg-button-click');
    }    
};

/*
------------------------------------------------------------------
// File: widgets\TemplateView.js
------------------------------------------------------------------
*/
YAHOO.ext.View = function(container, tpl, dataModel, config){
    this.el = getEl(container, true);
    this.nodes = this.el.dom.childNodes;
    if(typeof tpl == 'string'){
        tpl = new YAHOO.ext.Template(tpl);
    }
    tpl.compile();
    this.tpl = tpl;
    this.setDataModel(dataModel);
    
    var CE = YAHOO.util.CustomEvent;
	/** @private */
	this.events = {
	    'click' : new CE('click'),
	    'dblclick' : new CE('dblclick'),
	    'contextmenu' : new CE('contextmenu'),
	    'selectionchange' : new CE('selectionchange')
	};
	this.el.mon("click", this.onClick, this, true);
    this.el.mon("dblclick", this.onDblClick, this, true);
    this.el.mon("contextmenu", this.onContextMenu, this, true);
    this.el.swallowEvent("selectstart", true);
    this.selectedClass = 'ydataview-selected';
    
    this.selections = [];
    this.lastSelection = null;
    YAHOO.ext.util.Config.apply(this, config);
    if(this.renderUpdates || this.jsonRoot){
        var um = this.el.getUpdateManager();
        um.setRenderer(this);
    }
};

YAHOO.extendX(YAHOO.ext.View, YAHOO.ext.util.Observable, {
    getEl : function(){
        return this.el;  
    },
    render : function(el, response){
        this.clearSelections();
        this.el.update('');
        var o;
        try{
            o = YAHOO.ext.util.JSON.decode(response.responseText);
            if(this.jsonRoot){
                o = eval('o.' + this.jsonRoot);
            }
        }catch(e){}
        if(o && o.length){
            this.html = [];
            for(var i = 0, len = o.length; i < len; i++) {
            	this.renderEach(o[i]);
            }
            this.el.update(this.html.join(''));
            this.html = null;
            this.nodes = this.el.dom.childNodes;
            this.updateIndexes(0);
        }
    },
    
    refresh : function(){
        this.clearSelections();
        this.el.update('');
        this.html = [];
        this.dataModel.each(this.renderEach, this);
        this.el.update(this.html.join(''));
        this.html = null;
        this.nodes = this.el.dom.childNodes;
        this.updateIndexes(0);
    },
    
    prepareData : function(data, index){
        return data;  
    },
    
    renderEach : function(data){
        this.html[this.html.length] = this.tpl.applyTemplate(this.prepareData(data));
    },
    
    refreshNode : function(index){
        this.refreshNodes(index, index);
    },
    
    refreshNodes : function(dm, startIndex, endIndex){
        this.clearSelections();
        var dm = this.dataModel;
        var ns = this.nodes;
        for(var i = startIndex; i <= endIndex; i++){
            var d = this.prepareData(dm.getRow(i), i);
            if(i < ns.length-1){
                var old = ns[i];
                this.tpl.insertBefore(old, d);
                this.el.dom.removeChild(old);
            }else{
                this.tpl.append(this.el.dom, d);
            }
        }
        this.updateIndexes(startIndex, endIndex);
    },
    
    deleteNodes : function(dm, startIndex, endIndex){
        this.clearSelections();
        if(startIndex == 0 && endIndex >= this.nodes.length-1){
            this.el.update('');
        }else{
            var el = this.el.dom;
            for(var i = startIndex; i <= endIndex; i++){
                el.removeChild(this.nodes[startIndex]);
            }
            this.updateIndexes(startIndex);
        }
    },
    
    insertNodes : function(dm, startIndex, endIndex){
        if(this.nodes.length == 0){
            this.refresh();
        }else{
            this.clearSelections();
            var t = this.tpl;
            var before = this.nodes[startIndex];
            var dm = this.dataModel;
            if(before){
                for(var i = startIndex; i <= endIndex; i++){
                    t.insertBefore(before, this.prepareData(dm.getRow(i), i));
                }
            }else{
                var el = this.el.dom;
                for(var i = startIndex; i <= endIndex; i++){
                    t.append(el, this.prepareData(dm.getRow(i), i));
                }
            }
            this.updateIndexes(startIndex);
        }
    },
    
    updateIndexes : function(dm, startIndex, endIndex){
        var ns = this.nodes;
        startIndex = startIndex || 0;
        endIndex = endIndex || ns.length-1;
        for(var i = startIndex; i <= endIndex; i++){
            ns[i].nodeIndex = i;
        }
    },
    
    setDataModel : function(dm){
        this.unplugDataModel(this.dataModel);
        this.dataModel = dm;
        dm.on('cellupdated', this.refreshNode, this, true);
        dm.on('datachanged', this.refresh, this, true);
        dm.on('rowsdeleted', this.deleteNodes, this, true);
        dm.on('rowsinserted', this.insertNodes, this, true);
        dm.on('rowsupdated', this.refreshNodes, this, true);
        dm.on('rowssorted', this.refresh, this, true);
        this.refresh();
    },
    
    unplugDataModel : function(dm){
        if(!dm) return;
        dm.removeListener('cellupdated', this.refreshNode, this);
        dm.removeListener('datachanged', this.refresh, this);
        dm.removeListener('rowsdeleted', this.deleteNodes, this);
        dm.removeListener('rowsinserted', this.insertNodes, this);
        dm.removeListener('rowsupdated', this.refreshNodes, this);
        dm.removeListener('rowssorted', this.refresh, this);
        this.dataModel = null;
    },
    
    findItemFromChild : function(node){
        var el = this.el.dom;
        if(!node || node.parentNode == el){
		    return node;
	    }
	    var p = node.parentNode;
	    while(p && p != el){
            if(p.parentNode == el){
            	return p;
            }
            p = p.parentNode;
        }
	    return null;
    },
    
    /** @ignore */
    onClick : function(e){
        var item = this.findItemFromChild(e.getTarget());
        if(item){
            var index = this.indexOf(item);
            this.onItemClick(item, index, e);
            this.fireEvent('click', this, index, item, e);
        }else{
            this.clearSelections();
        }
    },

    /** @ignore */
    onContextMenu : function(e){
        var item = this.findItemFromChild(e.getTarget());
        if(item){
            this.fireEvent('contextmenu', this, this.indexOf(item), item, e);
        }
    },

    /** @ignore */
    onDblClick : function(e){
        var item = this.findItemFromChild(e.getTarget());
        if(item){
            this.fireEvent('dblclick', this, this.indexOf(item), item, e);
        }
    },
    
    onItemClick : function(item, index, e){
        if(this.multiSelect || this.singleSelect){
            if(this.multiSelect && e.shiftKey && this.lastSelection){
                this.select(this.getNodes(this.indexOf(this.lastSelection), index), false);
            }else{
                this.select(item, this.multiSelect && e.ctrlKey);
                this.lastSelection = item;
            }
        }
    },
    
    getSelectionCount : function(){
        return this.selections.length;
    },
    
    getSelectedNodes : function(){
        return this.selections;
    },
    
    getSelectedIndexes : function(){
        var indexes = [];
        for(var i = 0, len = this.selections.length; i < len; i++) {
        	indexes.push(this.selections[i].nodeIndex);
        }
        return indexes;
    },
    
    clearSelections : function(suppressEvent){
        if(this.multiSelect || this.singleSelect){
            YAHOO.util.Dom.removeClass(this.selections, this.selectedClass);
            this.selections = [];
            if(!suppressEvent){
                this.fireEvent('selectionchange', this, this.selections);
            }
        }
    },
    
    select : function(nodeInfo, keepExisting, suppressEvent){
        if(!keepExisting){
            this.clearSelections(true);
        }
        if(nodeInfo instanceof Array){
            for(var i = 0, len = nodeInfo.length; i < len; i++) {
            	this.select(nodeInfo[i], true, true);
            }
        }else{
            var node = this.getNode(nodeInfo);
            if(node){
                YAHOO.util.Dom.addClass(node, this.selectedClass);
                this.selections.push(node);
            }
        }
        if(!suppressEvent){
            this.fireEvent('selectionchange', this, this.selections);
        }
    },
    
    getNode : function(nodeInfo){
        if(typeof nodeInfo == 'object'){
            return nodeInfo;
        }else if(typeof nodeInfo == 'strong'){
            return document.getElementById(nodeInfo);
        }else if(typeof nodeInfo == 'number'){
            return this.nodes[nodeInfo];
        }
        return null;
    },
    
    getNodes : function(start, end){
        var ns = this.nodes;
        startIndex = startIndex || 0;
        endIndex = typeof endIndex == 'undefined' ? ns.length-1 : endIndex;
        var nodes = [];
        for(var i = start; i <= end; i++) {
        	nodes.push(ns[i]);
        }
        return nodes;
    },
    
    indexOf : function(node){
        node = this.getNode(node);
        if(typeof node.nodeIndex == 'number'){
            return node.nodeIndex;
        }
        var ns = this.nodes;
        for(var i = 0, len = ns.length; i < len; i++) {
        	if(ns[i] == node){
        	    return i;
        	}
        }
        return -1;
    }
});

/**
 * Shortcut to class create a JSON UpdateManager View
 */ 
YAHOO.ext.JsonView = function(container, tpl, config){
    var cfg = config || {};
    cfg.renderUpdates = true;
    return new YAHOO.ext.View(container, tpl, null, config); 
};
