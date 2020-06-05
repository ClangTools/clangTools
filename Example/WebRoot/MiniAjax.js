/**
 *
 * @param options {{dataType: string,type: string,timeout:int,contentType:String, url: string, async: boolean,data:Object,string, beforeSend: function(XMLHttpRequest), success: function(string,int), error: function(XMLHttpRequest, int, Object), complete: function(XMLHttpRequest)}}
 * @constructor
 */
const MiniAjax = function (options) {
    if (!(this instanceof MiniAjax)) throw new Error("非法调用");
    this.options = options || {};
    this.options.async = (typeof this.options.async === "undefined"?true:this.options.async);
    this.xmlHttp = this.GetXmlHttpObject();
    if (this.xmlHttp === null) {
        alert('您的浏览器不支持AJAX！');
        return;
    }
    this.xmlHttp.open(this.options.type || "GET", this.options.url, (this.options.async));
    this.xmlHttp.setRequestHeader("Content-Type", this.options.contentType || "application/x-www-form-urlencoded");
    if(this.options.async){
        this.xmlHttp.responseType = this.options.dataType || "text";
        this.xmlHttp.timeout = this.options.timeout || 0;
        this.xmlHttp.onreadystatechange = MiniAjax.prototype.Callback.bind(this);//发送事件后，收到信息了调用函数
    }
    if (typeof this.options.beforeSend === "function") {
        try {
            this.options.beforeSend(this.xmlHttp);
        } catch (e) {
            console.error(e);
            if (typeof this.options.complete === "function") {
                this.options.complete(this.xmlHttp);
            }
        }
    }
};
MiniAjax.prototype.Send = function () {
    if (!(this instanceof MiniAjax)) throw new Error("非法调用");
    try {
        if ((this.options.type || "GET") === "GET") {
            this.xmlHttp.send();
        } else {
            this.xmlHttp.send(this.options.data || "");
        }
    } catch (e) {
        console.error(e);
        if (typeof this.options.complete === "function") {
            this.options.complete(this.xmlHttp);
        }
    }
    if(!this.options.async){
        try {
            if (this.xmlHttp.readyState === 4 && this.xmlHttp.status === 200) {
                if (typeof this.options.success === "function") {
                    this.options.success(this.xmlHttp.responseText, this.xmlHttp.status);
                }
            } else {
                if (typeof this.options.error === "function") {
                    this.options.error(this.xmlHttp, this.xmlHttp.status, null);
                }
            }
        } catch (e) {
            console.error(e);
        } finally {
            if (typeof this.options.complete === "function") {
                this.options.complete(this.xmlHttp);
            }
        }
    }
};
MiniAjax.prototype.GetXmlHttpObject = function () {
    if (!(this instanceof MiniAjax)) throw new Error("非法调用");
    let http = null;
    if (window.XMLHttpRequest) {
        // code for IE7+, Firefox, Chrome, Opera, Safari
        http = new XMLHttpRequest();
    } else {// code for IE6, IE5
        http = new ActiveXObject("Microsoft.XMLHTTP");
    }
    return http;
};
MiniAjax.prototype.Callback = function () {
    if (!(this instanceof MiniAjax)) throw new Error("非法调用");
    if (this.xmlHttp.readyState === 1 || this.xmlHttp.readyState === 2 || this.xmlHttp.readyState === 3) {
        return;
    }
    try {
        if (this.xmlHttp.readyState === 4 && this.xmlHttp.status === 200) {
            if (typeof this.options.success === "function") {
                this.options.success(this.xmlHttp.responseText, this.xmlHttp.status);
            }
        } else {
            if (typeof this.options.error === "function") {
                this.options.error(this.xmlHttp, this.xmlHttp.status, null);
            }
        }
    } catch (e) {
        console.error(e);
    } finally {
        if (typeof this.options.complete === "function") {
            this.options.complete(this.xmlHttp);
        }
    }
};