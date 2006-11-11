// stops IE6 repeat loading of CSS background images
// http://ajaxian.com/archives/no-more-ie6-background-flicker
if(YAHOO.ext.util.Browser.isIE && !YAHOO.ext.util.Browser.isIE7){
    try{
        document.execCommand("BackgroundImageCache", false, true);
    }catch(e){}
}