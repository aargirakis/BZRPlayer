// create separate namespace for all the Emscripten stuff.. otherwise naming clashes may occur especially when 
// optimizing using closure compiler..
window.spp_backend_state_UADE= {
	notReady: true,
	adapterCallback: function(){}	// overwritten later	
};
window.spp_backend_state_UADE["onRuntimeInitialized"] = function() {	// emscripten callback needed in case async init is used (e.g. for WASM)
	this.notReady= false;
	this.adapterCallback();
}.bind(window.spp_backend_state_UADE);

// HACK
window.getStringifiedLines= function(bytes) {
	var ret= [];
	
	var line = "";
	for (var i= 0; i<bytes.length; i++) {
		var c= bytes[i];
		if ((c == 13) || (c == 10)) { // CR or LF
			if (line.length) {	
				ret.push(line);	
				line= "";
			} else {
				// just ignore empty lines
			}
		} else {
			// this seems to be all that is needed.. see /Delitracker Custom/Chris Huelsbeck/ (copyright sign, etc)
			line = line + String.fromCharCode(c);
		}
	}
	return ret;
}

window.rawBytes= function(module, ptr) {
	var ret= [];	
	while (1) {
		var ch = module.getValue(ptr++, 'i8', true);
		if (!ch) return ret;

		ret.push(ch & 0xff);
	}
}

var backend_UADE = (function(Module) {var d;d||(d=typeof Module !== 'undefined' ? Module : {});function aa(a,b){return a.match("^"+b)==b}var ba={},k;for(k in d)d.hasOwnProperty(k)&&(ba[k]=d[k]);d.arguments=[];d.thisProgram="./this.program";d.quit=function(a,b){throw b;};d.preRun=[];d.postRun=[];var ca=!1,l=!1,m=!1,da=!1;ca="object"===typeof window;l="function"===typeof importScripts;m="object"===typeof process&&"function"===typeof require&&!ca&&!l;da=!ca&&!m&&!l;
if(d.ENVIRONMENT)throw Error("Module.ENVIRONMENT has been deprecated. To force the environment, use the ENVIRONMENT compile-time option (for example, -s ENVIRONMENT=web or -s ENVIRONMENT=node)");assert("undefined"===typeof d.memoryInitializerPrefixURL,"Module.memoryInitializerPrefixURL option was removed, use Module.locateFile instead");assert("undefined"===typeof d.pthreadMainPrefixURL,"Module.pthreadMainPrefixURL option was removed, use Module.locateFile instead");
assert("undefined"===typeof d.cdInitializerPrefixURL,"Module.cdInitializerPrefixURL option was removed, use Module.locateFile instead");assert("undefined"===typeof d.filePackagePrefixURL,"Module.filePackagePrefixURL option was removed, use Module.locateFile instead");var p="";function ea(a){return d.locateFile?d.locateFile(a,p):p+a}
if(m){p=__dirname+"/";var fa,ha;d.read=function(a,b){fa||(fa=require("fs"));ha||(ha=require("path"));a=ha.normalize(a);a=fa.readFileSync(a);return b?a:a.toString()};d.readBinary=function(a){a=d.read(a,!0);a.buffer||(a=new Uint8Array(a));assert(a.buffer);return a};1<process.argv.length&&(d.thisProgram=process.argv[1].replace(/\\/g,"/"));d.arguments=process.argv.slice(2);"undefined"!==typeof module&&(module.exports=d);process.on("uncaughtException",function(a){if(!(a instanceof ia))throw a;});process.on("unhandledRejection",
q);d.quit=function(a){process.exit(a)};d.inspect=function(){return"[Emscripten Module object]"}}else if(da)"undefined"!=typeof read&&(d.read=function(a){return read(a)}),d.readBinary=function(a){if("function"===typeof readbuffer)return new Uint8Array(readbuffer(a));a=read(a,"binary");assert("object"===typeof a);return a},"undefined"!=typeof scriptArgs?d.arguments=scriptArgs:"undefined"!=typeof arguments&&(d.arguments=arguments),"function"===typeof quit&&(d.quit=function(a){quit(a)});else if(ca||l)l?
p=self.location.href:document.currentScript&&(p=document.currentScript.src),p=0!==p.indexOf("blob:")?p.substr(0,p.lastIndexOf("/")+1):"",d.read=function(a){var b=new XMLHttpRequest;b.open("GET",a,!1);b.send(null);return b.responseText},l&&(d.readBinary=function(a){var b=new XMLHttpRequest;b.open("GET",a,!1);b.responseType="arraybuffer";b.send(null);return new Uint8Array(b.response)}),d.readAsync=function(a,b,c){var e=new XMLHttpRequest;e.open("GET",a,!0);e.responseType="arraybuffer";e.onload=function(){200==
e.status||0==e.status&&e.response?b(e.response):c()};e.onerror=c;e.send(null)},d.setWindowTitle=function(a){document.title=a};else throw Error("environment detection error");var ja=d.print||("undefined"!==typeof console?console.log.bind(console):"undefined"!==typeof print?print:null),r=d.printErr||("undefined"!==typeof printErr?printErr:"undefined"!==typeof console&&console.warn.bind(console)||ja);for(k in ba)ba.hasOwnProperty(k)&&(d[k]=ba[k]);ba=void 0;ka=la=ma=function(){q("cannot use the stack before compiled code is ready to run, and has provided stack access")};
function na(a){assert(!oa);var b=u;u=u+a+15&-16;assert(u<v,"not enough memory for static allocation - increase TOTAL_MEMORY");return b}function pa(a){assert(w);var b=y[w>>2];a=b+a+15&-16;y[w>>2]=a;if(a=a>=v)qa(),a=!0;return a?(y[w>>2]=b,0):b}function ra(a){var b;b||(b=16);return Math.ceil(a/b)*b}function sa(a){ua||(ua={});ua[a]||(ua[a]=1,r(a))}var ua,va={"f64-rem":function(a,b){return a%b},"debugger":function(){debugger}},wa=!1;function assert(a,b){a||q("Assertion failed: "+b)}
var ya={stackSave:function(){ka()},stackRestore:function(){la()},arrayToC:function(a){var b=ma(a.length);assert(0<=a.length,"writeArrayToMemory array must have a length (should be an array or typed array)");A.set(a,b);return b},stringToC:function(a){var b=0;if(null!==a&&void 0!==a&&0!==a){var c=(a.length<<2)+1,e=b=ma(c);assert("number"==typeof c,"stringToUTF8(str, outPtr, maxBytesToWrite) is missing the third parameter that specifies the length of the output buffer!");xa(a,B,e,c)}return b}},za={string:ya.stringToC,
array:ya.arrayToC};function Aa(a,b){if("number"===typeof a){var c=!0;var e=a}else c=!1,e=a.length;b=4==b?f:["function"===typeof Ba?Ba:na,ma,na,pa][void 0===b?2:b](Math.max(e,1));if(c){var f=b;assert(0==(b&3));for(a=b+(e&-4);f<a;f+=4)y[f>>2]=0;for(a=b+e;f<a;)A[f++>>0]=0;return b}a.subarray||a.slice?B.set(a,b):B.set(new Uint8Array(a),b);return b}function Ca(a){return oa?C?Ba(a):pa(a):na(a)}
function E(a,b){if(0===b||!a)return"";for(var c=0,e,f=0;;){assert(a+f<v);e=B[a+f>>0];c|=e;if(0==e&&!b)break;f++;if(b&&f==b)break}b||(b=f);e="";if(128>c){for(;0<b;)c=String.fromCharCode.apply(String,B.subarray(a,a+Math.min(b,1024))),e=e?e+c:c,a+=1024,b-=1024;return e}return Da(B,a)}var Ea="undefined"!==typeof TextDecoder?new TextDecoder("utf8"):void 0;
function Da(a,b){for(var c=b;a[c];)++c;if(16<c-b&&a.subarray&&Ea)return Ea.decode(a.subarray(b,c));for(c="";;){var e=a[b++];if(!e)return c;if(e&128){var f=a[b++]&63;if(192==(e&224))c+=String.fromCharCode((e&31)<<6|f);else{var h=a[b++]&63;if(224==(e&240))e=(e&15)<<12|f<<6|h;else{var g=a[b++]&63;if(240==(e&248))e=(e&7)<<18|f<<12|h<<6|g;else{var n=a[b++]&63;if(248==(e&252))e=(e&3)<<24|f<<18|h<<12|g<<6|n;else{var x=a[b++]&63;e=(e&1)<<30|f<<24|h<<18|g<<12|n<<6|x}}}65536>e?c+=String.fromCharCode(e):(e-=
65536,c+=String.fromCharCode(55296|e>>10,56320|e&1023))}}else c+=String.fromCharCode(e)}}
function xa(a,b,c,e){if(!(0<e))return 0;var f=c;e=c+e-1;for(var h=0;h<a.length;++h){var g=a.charCodeAt(h);if(55296<=g&&57343>=g){var n=a.charCodeAt(++h);g=65536+((g&1023)<<10)|n&1023}if(127>=g){if(c>=e)break;b[c++]=g}else{if(2047>=g){if(c+1>=e)break;b[c++]=192|g>>6}else{if(65535>=g){if(c+2>=e)break;b[c++]=224|g>>12}else{if(2097151>=g){if(c+3>=e)break;b[c++]=240|g>>18}else{if(67108863>=g){if(c+4>=e)break;b[c++]=248|g>>24}else{if(c+5>=e)break;b[c++]=252|g>>30;b[c++]=128|g>>24&63}b[c++]=128|g>>18&63}b[c++]=
128|g>>12&63}b[c++]=128|g>>6&63}b[c++]=128|g&63}}b[c]=0;return c-f}"undefined"!==typeof TextDecoder&&new TextDecoder("utf-16le");function Fa(a){return a.replace(/__Z[\w\d_]+/g,function(a){sa("warning: build with  -s DEMANGLE_SUPPORT=1  to link in libcxxabi demangling");return a===a?a:a+" ["+a+"]"})}
function Ga(){a:{var a=Error();if(!a.stack){try{throw Error(0);}catch(b){a=b}if(!a.stack){a="(no stack trace available)";break a}}a=a.stack.toString()}d.extraStackTrace&&(a+="\n"+d.extraStackTrace());return Fa(a)}var buffer,A,B,Ha,y,Ia,Ja,Ka;
function La(){d.HEAP8=A=new Int8Array(buffer);d.HEAP16=Ha=new Int16Array(buffer);d.HEAP32=y=new Int32Array(buffer);d.HEAPU8=B=new Uint8Array(buffer);d.HEAPU16=new Uint16Array(buffer);d.HEAPU32=Ia=new Uint32Array(buffer);d.HEAPF32=Ja=new Float32Array(buffer);d.HEAPF64=Ka=new Float64Array(buffer)}var Ma,u,oa,Na,Oa,F,Pa,w;Ma=u=Na=Oa=F=Pa=w=0;oa=!1;
function Qa(){34821223==Ia[(F>>2)-1]&&2310721022==Ia[(F>>2)-2]||q("Stack overflow! Stack cookie has been overwritten, expected hex dwords 0x89BACDFE and 0x02135467, but received 0x"+Ia[(F>>2)-2].toString(16)+" "+Ia[(F>>2)-1].toString(16));if(1668509029!==y[0])throw"Runtime error: The application has corrupted its heap memory area (address zero)!";}
function qa(){q("Cannot enlarge memory arrays. Either (1) compile with  -s TOTAL_MEMORY=X  with X higher than the current value "+v+", (2) compile with  -s ALLOW_MEMORY_GROWTH=1  which allows increasing the size at runtime, or (3) if you want malloc to return NULL (0) instead of this abort, compile with  -s ABORTING_MALLOC=0 ")}var Ra=d.TOTAL_STACK||5242880,v=d.TOTAL_MEMORY||33554432;v<Ra&&r("TOTAL_MEMORY should be larger than TOTAL_STACK, was "+v+"! (TOTAL_STACK="+Ra+")");
assert("undefined"!==typeof Int32Array&&"undefined"!==typeof Float64Array&&void 0!==Int32Array.prototype.subarray&&void 0!==Int32Array.prototype.set,"JS engine does not provide full typed array support");
d.buffer?(buffer=d.buffer,assert(buffer.byteLength===v,"provided buffer should be "+v+" bytes, but it is "+buffer.byteLength)):("object"===typeof WebAssembly&&"function"===typeof WebAssembly.Memory?(assert(0===v%65536),d.wasmMemory=new WebAssembly.Memory({initial:v/65536,maximum:v/65536}),buffer=d.wasmMemory.buffer):buffer=new ArrayBuffer(v),assert(buffer.byteLength===v),d.buffer=buffer);La();y[0]=1668509029;Ha[1]=25459;
if(115!==B[2]||99!==B[3])throw"Runtime error: expected the system to be little-endian!";function Sa(a){for(;0<a.length;){var b=a.shift();if("function"==typeof b)b();else{var c=b.Da;"number"===typeof c?void 0===b.V?d.dynCall_v(c):d.dynCall_vi(c,b.V):c(void 0===b.V?null:b.V)}}}var Ta=[],Ua=[],Va=[],Wa=[],Xa=[],C=!1,G=!1;function Ya(){var a=d.preRun.shift();Ta.unshift(a)}assert(Math.imul&&Math.fround&&Math.clz32&&Math.trunc,"this is a legacy browser, build with LEGACY_VM_SUPPORT");
var Za=0,$a=null,ab=null,bb={};function cb(a){for(var b=a;bb[a];)a=b+Math.random();return a}function db(a){Za++;d.monitorRunDependencies&&d.monitorRunDependencies(Za);a?(assert(!bb[a]),bb[a]=1,null===$a&&"undefined"!==typeof setInterval&&($a=setInterval(function(){if(wa)clearInterval($a),$a=null;else{var a=!1,c;for(c in bb)a||(a=!0,r("still waiting on run dependencies:")),r("dependency: "+c);a&&r("(end of list)")}},1E4))):r("warning: run dependency added without ID")}
function eb(a){Za--;d.monitorRunDependencies&&d.monitorRunDependencies(Za);a?(assert(bb[a]),delete bb[a]):r("warning: run dependency removed without ID");0==Za&&(null!==$a&&(clearInterval($a),$a=null),ab&&(a=ab,ab=null,a()))}d.preloadedImages={};d.preloadedAudios={};function fb(a){return String.prototype.startsWith?a.startsWith("data:application/octet-stream;base64,"):0===a.indexOf("data:application/octet-stream;base64,")}
(function(){function a(){try{if(d.wasmBinary)return new Uint8Array(d.wasmBinary);if(d.readBinary)return d.readBinary(f);throw"both async and sync fetching of the wasm failed";}catch(t){q(t)}}function b(){return d.wasmBinary||!ca&&!l||"function"!==typeof fetch?new Promise(function(b){b(a())}):fetch(f,{credentials:"same-origin"}).then(function(a){if(!a.ok)throw"failed to load wasm binary file at '"+f+"'";return a.arrayBuffer()}).catch(function(){return a()})}function c(a){function c(a){n=a.exports;
if(n.memory){a=n.memory;var b=d.buffer;a.byteLength<b.byteLength&&r("the new buffer in mergeMemory is smaller than the previous one. in native wasm, we should grow memory here");b=new Int8Array(b);(new Int8Array(a)).set(b);d.buffer=buffer=a;La()}d.asm=n;d.usingWasm=!0;eb("wasm-instantiate")}function e(a){assert(d===t,"the Module object should not be replaced during async compilation - perhaps the order of HTML elements is wrong?");t=null;c(a.instance)}function h(a){b().then(function(a){return WebAssembly.instantiate(a,
g)}).then(a,function(a){r("failed to asynchronously prepare wasm: "+a);q(a)})}if("object"!==typeof WebAssembly)return q("No WebAssembly support found. Build with -s WASM=0 to target JavaScript instead."),r("no native wasm support detected"),!1;if(!(d.wasmMemory instanceof WebAssembly.Memory))return r("no native wasm Memory in use"),!1;a.memory=d.wasmMemory;g.global={NaN:NaN,Infinity:Infinity};g["global.Math"]=Math;g.env=a;db("wasm-instantiate");if(d.instantiateWasm)try{return d.instantiateWasm(g,
c)}catch(ta){return r("Module.instantiateWasm callback failed with error: "+ta),!1}var t=d;d.wasmBinary||"function"!==typeof WebAssembly.instantiateStreaming||fb(f)||"function"!==typeof fetch?h(e):WebAssembly.instantiateStreaming(fetch(f,{credentials:"same-origin"}),g).then(e,function(a){r("wasm streaming compile failed: "+a);r("falling back to ArrayBuffer instantiation");h(e)});return{}}var e="uade.wast",f="uade.wasm",h="uade.temp.asm.js";fb(e)||(e=ea(e));fb(f)||(f=ea(f));fb(h)||(h=ea(h));var g=
{global:null,env:null,asm2wasm:va,parent:d},n=null;d.asmPreload=d.asm;var x=d.reallocBuffer;d.reallocBuffer=function(a){if("asmjs"===z)var b=x(a);else a:{var c=d.usingWasm?65536:16777216;0<a%c&&(a+=c-a%c);c=d.buffer.byteLength;if(d.usingWasm)try{b=-1!==d.wasmMemory.grow((a-c)/65536)?d.buffer=d.wasmMemory.buffer:null;break a}catch(D){console.error("Module.reallocBuffer: Attempted to grow from "+c+" bytes to "+a+" bytes, but got error: "+D);b=null;break a}b=void 0}return b};var z="";d.asm=function(a,
b){if(!b.table){a=d.wasmTableSize;void 0===a&&(a=1024);var e=d.wasmMaxTableSize;b.table="object"===typeof WebAssembly&&"function"===typeof WebAssembly.Table?void 0!==e?new WebAssembly.Table({initial:a,maximum:e,element:"anyfunc"}):new WebAssembly.Table({initial:a,element:"anyfunc"}):Array(a);d.wasmTable=b.table}b.memoryBase||(b.memoryBase=d.STATIC_BASE);b.tableBase||(b.tableBase=0);b=c(b);assert(b,"no binaryen method succeeded. consider enabling more options, like interpreting, if you want that: http://kripken.github.io/emscripten-site/docs/compiling/WebAssembly.html#binaryen-methods");
return b}})();Ma=1024;u=Ma+960512;Ua.push({Da:function(){gb()}});d.STATIC_BASE=Ma;d.STATIC_BUMP=960512;var hb=u;u+=16;assert(0==hb%8);var H={};
function ib(a){if(ib.ha){var b=y[a>>2];var c=y[b>>2]}else ib.ha=!0,H.USER=H.LOGNAME="web_user",H.PATH="/",H.PWD="/",H.HOME="/home/web_user",H.LANG="C.UTF-8",H._=d.thisProgram,c=Ca(1024),b=Ca(256),y[b>>2]=c,y[a>>2]=b;a=[];var e=0,f;for(f in H)if("string"===typeof H[f]){var h=f+"="+H[f];a.push(h);e+=h.length}if(1024<e)throw Error("Environment size exceeded TOTAL_ENV_SIZE!");for(f=0;f<a.length;f++){e=h=a[f];for(var g=c,n=0;n<e.length;++n)assert(e.charCodeAt(n)===e.charCodeAt(n)&255),A[g++>>0]=e.charCodeAt(n);
A[g>>0]=0;y[b+4*f>>2]=c;c+=h.length+1}y[b+4*a.length>>2]=0}
var I={F:1,v:2,Dc:3,zb:4,B:5,fa:6,Sa:7,Xb:8,u:9,gb:10,aa:11,Nc:11,wa:12,R:13,sb:14,jc:15,S:16,ba:17,Oc:18,U:19,da:20,I:21,h:22,Sb:23,va:24,C:25,Kc:26,tb:27,ec:28,N:29,Ac:30,Lb:31,tc:32,pb:33,xc:34,ac:42,wb:43,hb:44,Cb:45,Db:46,Eb:47,Kb:48,Lc:49,Vb:50,Bb:51,mb:35,Yb:37,Ya:52,ab:53,Pc:54,Tb:55,bb:56,cb:57,nb:35,eb:59,hc:60,Wb:61,Hc:62,fc:63,bc:64,cc:65,zc:66,Zb:67,Va:68,Ec:69,ib:70,uc:71,Nb:72,qb:73,$a:74,oc:76,Za:77,yc:78,Fb:79,Gb:80,Jb:81,Ib:82,Hb:83,ic:38,ea:39,Ob:36,T:40,pc:95,sc:96,lb:104,Ub:105,
Wa:97,wc:91,mc:88,dc:92,Bc:108,kb:111,Ta:98,jb:103,Rb:101,Pb:100,Ic:110,ub:112,vb:113,yb:115,Xa:114,ob:89,Mb:90,vc:93,Cc:94,Ua:99,Qb:102,Ab:106,kc:107,Jc:109,Mc:87,rb:122,Fc:116,nc:95,$b:123,xb:84,qc:75,fb:125,lc:131,rc:130,Gc:86},jb={0:"Success",1:"Not super-user",2:"No such file or directory",3:"No such process",4:"Interrupted system call",5:"I/O error",6:"No such device or address",7:"Arg list too long",8:"Exec format error",9:"Bad file number",10:"No children",11:"No more processes",12:"Not enough core",
13:"Permission denied",14:"Bad address",15:"Block device required",16:"Mount device busy",17:"File exists",18:"Cross-device link",19:"No such device",20:"Not a directory",21:"Is a directory",22:"Invalid argument",23:"Too many open files in system",24:"Too many open files",25:"Not a typewriter",26:"Text file busy",27:"File too large",28:"No space left on device",29:"Illegal seek",30:"Read only file system",31:"Too many links",32:"Broken pipe",33:"Math arg out of domain of func",34:"Math result not representable",
35:"File locking deadlock error",36:"File or path name too long",37:"No record locks available",38:"Function not implemented",39:"Directory not empty",40:"Too many symbolic links",42:"No message of desired type",43:"Identifier removed",44:"Channel number out of range",45:"Level 2 not synchronized",46:"Level 3 halted",47:"Level 3 reset",48:"Link number out of range",49:"Protocol driver not attached",50:"No CSI structure available",51:"Level 2 halted",52:"Invalid exchange",53:"Invalid request descriptor",
54:"Exchange full",55:"No anode",56:"Invalid request code",57:"Invalid slot",59:"Bad font file fmt",60:"Device not a stream",61:"No data (for no delay io)",62:"Timer expired",63:"Out of streams resources",64:"Machine is not on the network",65:"Package not installed",66:"The object is remote",67:"The link has been severed",68:"Advertise error",69:"Srmount error",70:"Communication error on send",71:"Protocol error",72:"Multihop attempted",73:"Cross mount point (not really error)",74:"Trying to read unreadable message",
75:"Value too large for defined data type",76:"Given log. name not unique",77:"f.d. invalid for this operation",78:"Remote address changed",79:"Can   access a needed shared lib",80:"Accessing a corrupted shared lib",81:".lib section in a.out corrupted",82:"Attempting to link in too many libs",83:"Attempting to exec a shared library",84:"Illegal byte sequence",86:"Streams pipe error",87:"Too many users",88:"Socket operation on non-socket",89:"Destination address required",90:"Message too long",91:"Protocol wrong type for socket",
92:"Protocol not available",93:"Unknown protocol",94:"Socket type not supported",95:"Not supported",96:"Protocol family not supported",97:"Address family not supported by protocol family",98:"Address already in use",99:"Address not available",100:"Network interface is not configured",101:"Network is unreachable",102:"Connection reset by network",103:"Connection aborted",104:"Connection reset by peer",105:"No buffer space available",106:"Socket is already connected",107:"Socket is not connected",108:"Can't send after socket shutdown",
109:"Too many references",110:"Connection timed out",111:"Connection refused",112:"Host is down",113:"Host is unreachable",114:"Socket already connected",115:"Connection already in progress",116:"Stale file handle",122:"Quota exceeded",123:"No medium (in tape drive)",125:"Operation canceled",130:"Previous owner died",131:"State not recoverable"};function kb(a){d.___errno_location?y[d.___errno_location()>>2]=a:r("failed to set errno from JS");return a}
function lb(a,b){for(var c=0,e=a.length-1;0<=e;e--){var f=a[e];"."===f?a.splice(e,1):".."===f?(a.splice(e,1),c++):c&&(a.splice(e,1),c--)}if(b)for(;c;c--)a.unshift("..");return a}function mb(a){var b="/"===a.charAt(0),c="/"===a.substr(-1);(a=lb(a.split("/").filter(function(a){return!!a}),!b).join("/"))||b||(a=".");a&&c&&(a+="/");return(b?"/":"")+a}
function nb(a){var b=/^(\/?|)([\s\S]*?)((?:\.{1,2}|[^\/]+?|)(\.[^.\/]*|))(?:[\/]*)$/.exec(a).slice(1);a=b[0];b=b[1];if(!a&&!b)return".";b&&(b=b.substr(0,b.length-1));return a+b}function ob(a){if("/"===a)return"/";var b=a.lastIndexOf("/");return-1===b?a:a.substr(b+1)}function pb(){var a=Array.prototype.slice.call(arguments,0);return mb(a.join("/"))}function J(a,b){return mb(a+"/"+b)}
function qb(){for(var a="",b=!1,c=arguments.length-1;-1<=c&&!b;c--){b=0<=c?arguments[c]:"/";if("string"!==typeof b)throw new TypeError("Arguments to path.resolve must be strings");if(!b)return"";a=b+"/"+a;b="/"===b.charAt(0)}a=lb(a.split("/").filter(function(a){return!!a}),!b).join("/");return(b?"/":"")+a||"."}var rb=[];function sb(a,b){rb[a]={input:[],output:[],H:b};tb(a,ub)}
var ub={open:function(a){var b=rb[a.node.rdev];if(!b)throw new K(I.U);a.tty=b;a.seekable=!1},close:function(a){a.tty.H.flush(a.tty)},flush:function(a){a.tty.H.flush(a.tty)},read:function(a,b,c,e){if(!a.tty||!a.tty.H.pa)throw new K(I.fa);for(var f=0,h=0;h<e;h++){try{var g=a.tty.H.pa(a.tty)}catch(n){throw new K(I.B);}if(void 0===g&&0===f)throw new K(I.aa);if(null===g||void 0===g)break;f++;b[c+h]=g}f&&(a.node.timestamp=Date.now());return f},write:function(a,b,c,e){if(!a.tty||!a.tty.H.Z)throw new K(I.fa);
for(var f=0;f<e;f++)try{a.tty.H.Z(a.tty,b[c+f])}catch(h){throw new K(I.B);}e&&(a.node.timestamp=Date.now());return f}},wb={pa:function(a){if(!a.input.length){var b=null;if(m){var c=new Buffer(256),e=0,f=process.stdin.fd;if("win32"!=process.platform){var h=!1;try{f=fs.openSync("/dev/stdin","r"),h=!0}catch(g){}}try{e=fs.readSync(f,c,0,256,null)}catch(g){if(-1!=g.toString().indexOf("EOF"))e=0;else throw g;}h&&fs.closeSync(f);0<e?b=c.slice(0,e).toString("utf-8"):b=null}else"undefined"!=typeof window&&
"function"==typeof window.prompt?(b=window.prompt("Input: "),null!==b&&(b+="\n")):"function"==typeof readline&&(b=readline(),null!==b&&(b+="\n"));if(!b)return null;a.input=vb(b,!0)}return a.input.shift()},Z:function(a,b){null===b||10===b?(ja(Da(a.output,0)),a.output=[]):0!=b&&a.output.push(b)},flush:function(a){a.output&&0<a.output.length&&(ja(Da(a.output,0)),a.output=[])}},xb={Z:function(a,b){null===b||10===b?(r(Da(a.output,0)),a.output=[]):0!=b&&a.output.push(b)},flush:function(a){a.output&&0<a.output.length&&
(r(Da(a.output,0)),a.output=[])}},L={o:null,j:function(){return L.createNode(null,"/",16895,0)},createNode:function(a,b,c,e){if(24576===(c&61440)||4096===(c&61440))throw new K(I.F);L.o||(L.o={dir:{node:{s:L.c.s,i:L.c.i,lookup:L.c.lookup,K:L.c.K,rename:L.c.rename,unlink:L.c.unlink,rmdir:L.c.rmdir,readdir:L.c.readdir,symlink:L.c.symlink},stream:{A:L.f.A}},file:{node:{s:L.c.s,i:L.c.i},stream:{A:L.f.A,read:L.f.read,write:L.f.write,ga:L.f.ga,sa:L.f.sa,ua:L.f.ua}},link:{node:{s:L.c.s,i:L.c.i,readlink:L.c.readlink},
stream:{}},ka:{node:{s:L.c.s,i:L.c.i},stream:yb}});c=zb(a,b,c,e);N(c.mode)?(c.c=L.o.dir.node,c.f=L.o.dir.stream,c.b={}):32768===(c.mode&61440)?(c.c=L.o.file.node,c.f=L.o.file.stream,c.g=0,c.b=null):40960===(c.mode&61440)?(c.c=L.o.link.node,c.f=L.o.link.stream):8192===(c.mode&61440)&&(c.c=L.o.ka.node,c.f=L.o.ka.stream);c.timestamp=Date.now();a&&(a.b[b]=c);return c},Ea:function(a){if(a.b&&a.b.subarray){for(var b=[],c=0;c<a.g;++c)b.push(a.b[c]);return b}return a.b},Sc:function(a){return a.b?a.b.subarray?
a.b.subarray(0,a.g):new Uint8Array(a.b):new Uint8Array},la:function(a,b){a.b&&a.b.subarray&&b>a.b.length&&(a.b=L.Ea(a),a.g=a.b.length);if(!a.b||a.b.subarray){var c=a.b?a.b.length:0;c>=b||(b=Math.max(b,c*(1048576>c?2:1.125)|0),0!=c&&(b=Math.max(b,256)),c=a.b,a.b=new Uint8Array(b),0<a.g&&a.b.set(c.subarray(0,a.g),0))}else for(!a.b&&0<b&&(a.b=[]);a.b.length<b;)a.b.push(0)},Na:function(a,b){if(a.g!=b)if(0==b)a.b=null,a.g=0;else{if(!a.b||a.b.subarray){var c=a.b;a.b=new Uint8Array(new ArrayBuffer(b));c&&
a.b.set(c.subarray(0,Math.min(b,a.g)))}else if(a.b||(a.b=[]),a.b.length>b)a.b.length=b;else for(;a.b.length<b;)a.b.push(0);a.g=b}},c:{s:function(a){var b={};b.dev=8192===(a.mode&61440)?a.id:1;b.ino=a.id;b.mode=a.mode;b.nlink=1;b.uid=0;b.gid=0;b.rdev=a.rdev;N(a.mode)?b.size=4096:32768===(a.mode&61440)?b.size=a.g:40960===(a.mode&61440)?b.size=a.link.length:b.size=0;b.atime=new Date(a.timestamp);b.mtime=new Date(a.timestamp);b.ctime=new Date(a.timestamp);b.D=4096;b.blocks=Math.ceil(b.size/b.D);return b},
i:function(a,b){void 0!==b.mode&&(a.mode=b.mode);void 0!==b.timestamp&&(a.timestamp=b.timestamp);void 0!==b.size&&L.Na(a,b.size)},lookup:function(){throw Ab[I.v];},K:function(a,b,c,e){return L.createNode(a,b,c,e)},rename:function(a,b,c){if(N(a.mode)){try{var e=Bb(b,c)}catch(h){}if(e)for(var f in e.b)throw new K(I.ea);}delete a.parent.b[a.name];a.name=c;b.b[c]=a;a.parent=b},unlink:function(a,b){delete a.b[b]},rmdir:function(a,b){var c=Bb(a,b),e;for(e in c.b)throw new K(I.ea);delete a.b[b]},readdir:function(a){var b=
[".",".."],c;for(c in a.b)a.b.hasOwnProperty(c)&&b.push(c);return b},symlink:function(a,b,c){a=L.createNode(a,b,41471,0);a.link=c;return a},readlink:function(a){if(40960!==(a.mode&61440))throw new K(I.h);return a.link}},f:{read:function(a,b,c,e,f){var h=a.node.b;if(f>=a.node.g)return 0;a=Math.min(a.node.g-f,e);assert(0<=a);if(8<a&&h.subarray)b.set(h.subarray(f,f+a),c);else for(e=0;e<a;e++)b[c+e]=h[f+e];return a},write:function(a,b,c,e,f,h){if(!e)return 0;a=a.node;a.timestamp=Date.now();if(b.subarray&&
(!a.b||a.b.subarray)){if(h)return assert(0===f,"canOwn must imply no weird position inside the file"),a.b=b.subarray(c,c+e),a.g=e;if(0===a.g&&0===f)return a.b=new Uint8Array(b.subarray(c,c+e)),a.g=e;if(f+e<=a.g)return a.b.set(b.subarray(c,c+e),f),e}L.la(a,f+e);if(a.b.subarray&&b.subarray)a.b.set(b.subarray(c,c+e),f);else for(h=0;h<e;h++)a.b[f+h]=b[c+h];a.g=Math.max(a.g,f+e);return e},A:function(a,b,c){1===c?b+=a.position:2===c&&32768===(a.node.mode&61440)&&(b+=a.node.g);if(0>b)throw new K(I.h);return b},
ga:function(a,b,c){L.la(a.node,b+c);a.node.g=Math.max(a.node.g,b+c)},sa:function(a,b,c,e,f,h,g){if(32768!==(a.node.mode&61440))throw new K(I.U);c=a.node.b;if(g&2||c.buffer!==b&&c.buffer!==b.buffer){if(0<f||f+e<a.node.g)c.subarray?c=c.subarray(f,f+e):c=Array.prototype.slice.call(c,f,f+e);a=!0;e=Ba(e);if(!e)throw new K(I.wa);b.set(c,e)}else a=!1,e=c.byteOffset;return{Uc:e,Qc:a}},ua:function(a,b,c,e,f){if(32768!==(a.node.mode&61440))throw new K(I.U);if(f&2)return 0;L.f.write(a,b,0,e,c,!1);return 0}}},
O={P:!1,Qa:function(){O.P=!!process.platform.match(/^win/);var a=process.binding("constants");a.fs&&(a=a.fs);O.ma={1024:a.O_APPEND,64:a.O_CREAT,128:a.O_EXCL,0:a.O_RDONLY,2:a.O_RDWR,4096:a.O_SYNC,512:a.O_TRUNC,1:a.O_WRONLY}},ia:function(a){return Buffer.ha?Buffer.from(a):new Buffer(a)},j:function(a){assert(m);return O.createNode(null,"/",O.oa(a.Y.root),0)},createNode:function(a,b,c){if(!N(c)&&32768!==(c&61440)&&40960!==(c&61440))throw new K(I.h);a=zb(a,b,c);a.c=O.c;a.f=O.f;return a},oa:function(a){try{var b=
fs.lstatSync(a);O.P&&(b.mode=b.mode|(b.mode&292)>>2)}catch(c){if(!c.code)throw c;throw new K(I[c.code]);}return b.mode},l:function(a){for(var b=[];a.parent!==a;)b.push(a.name),a=a.parent;b.push(a.j.Y.root);b.reverse();return pb.apply(null,b)},Ca:function(a){a&=-2656257;var b=0,c;for(c in O.ma)a&c&&(b|=O.ma[c],a^=c);if(a)throw new K(I.h);return b},c:{s:function(a){a=O.l(a);try{var b=fs.lstatSync(a)}catch(c){if(!c.code)throw c;throw new K(I[c.code]);}O.P&&!b.D&&(b.D=4096);O.P&&!b.blocks&&(b.blocks=
(b.size+b.D-1)/b.D|0);return{dev:b.dev,ino:b.ino,mode:b.mode,nlink:b.nlink,uid:b.uid,gid:b.gid,rdev:b.rdev,size:b.size,atime:b.atime,mtime:b.mtime,ctime:b.ctime,D:b.D,blocks:b.blocks}},i:function(a,b){var c=O.l(a);try{void 0!==b.mode&&(fs.chmodSync(c,b.mode),a.mode=b.mode),void 0!==b.size&&fs.truncateSync(c,b.size)}catch(e){if(!e.code)throw e;throw new K(I[e.code]);}},lookup:function(a,b){var c=J(O.l(a),b);c=O.oa(c);return O.createNode(a,b,c)},K:function(a,b,c,e){a=O.createNode(a,b,c,e);b=O.l(a);
try{N(a.mode)?fs.mkdirSync(b,a.mode):fs.writeFileSync(b,"",{mode:a.mode})}catch(f){if(!f.code)throw f;throw new K(I[f.code]);}return a},rename:function(a,b,c){a=O.l(a);b=J(O.l(b),c);try{fs.renameSync(a,b)}catch(e){if(!e.code)throw e;throw new K(I[e.code]);}},unlink:function(a,b){a=J(O.l(a),b);try{fs.unlinkSync(a)}catch(c){if(!c.code)throw c;throw new K(I[c.code]);}},rmdir:function(a,b){a=J(O.l(a),b);try{fs.rmdirSync(a)}catch(c){if(!c.code)throw c;throw new K(I[c.code]);}},readdir:function(a){a=O.l(a);
try{return fs.readdirSync(a)}catch(b){if(!b.code)throw b;throw new K(I[b.code]);}},symlink:function(a,b,c){a=J(O.l(a),b);try{fs.symlinkSync(c,a)}catch(e){if(!e.code)throw e;throw new K(I[e.code]);}},readlink:function(a){var b=O.l(a);try{return b=fs.readlinkSync(b),b=Cb.relative(Cb.resolve(a.j.Y.root),b)}catch(c){if(!c.code)throw c;throw new K(I[c.code]);}}},f:{open:function(a){var b=O.l(a.node);try{32768===(a.node.mode&61440)&&(a.M=fs.openSync(b,O.Ca(a.flags)))}catch(c){if(!c.code)throw c;throw new K(I[c.code]);
}},close:function(a){try{32768===(a.node.mode&61440)&&a.M&&fs.closeSync(a.M)}catch(b){if(!b.code)throw b;throw new K(I[b.code]);}},read:function(a,b,c,e,f){if(0===e)return 0;try{return fs.readSync(a.M,O.ia(b.buffer),c,e,f)}catch(h){throw new K(I[h.code]);}},write:function(a,b,c,e,f){try{return fs.writeSync(a.M,O.ia(b.buffer),c,e,f)}catch(h){throw new K(I[h.code]);}},A:function(a,b,c){if(1===c)b+=a.position;else if(2===c&&32768===(a.node.mode&61440))try{b+=fs.fstatSync(a.M).size}catch(e){throw new K(I[e.code]);
}if(0>b)throw new K(I.h);return b}}};u+=16;u+=16;u+=16;var Db=null,Eb={},Gb=[],Hb=1,P=null,Ib=!0,Q={},K=null,Ab={};
function R(a,b){a=qb("/",a);b=b||{};if(!a)return{path:"",node:null};var c={na:!0,$:0},e;for(e in c)void 0===b[e]&&(b[e]=c[e]);if(8<b.$)throw new K(I.T);a=lb(a.split("/").filter(function(a){return!!a}),!1);var f=Db;c="/";for(e=0;e<a.length;e++){var h=e===a.length-1;if(h&&b.parent)break;f=Bb(f,a[e]);c=J(c,a[e]);f.L&&(!h||h&&b.na)&&(f=f.L.root);if(!h||b.J)for(h=0;40960===(f.mode&61440);)if(f=Jb(c),c=qb(nb(c),f),f=R(c,{$:b.$}).node,40<h++)throw new K(I.T);}return{path:c,node:f}}
function S(a){for(var b;;){if(a===a.parent)return a=a.j.ta,b?"/"!==a[a.length-1]?a+"/"+b:a+b:a;b=b?a.name+"/"+b:a.name;a=a.parent}}function Kb(a,b){for(var c=0,e=0;e<b.length;e++)c=(c<<5)-c+b.charCodeAt(e)|0;return(a+c>>>0)%P.length}function Lb(a){var b=Kb(a.parent.id,a.name);a.G=P[b];P[b]=a}function Bb(a,b){var c;if(c=(c=Mb(a,"x"))?c:a.c.lookup?0:I.R)throw new K(c,a);for(c=P[Kb(a.id,b)];c;c=c.G){var e=c.name;if(c.parent.id===a.id&&e===b)return c}return a.c.lookup(a,b)}
function zb(a,b,c,e){Nb||(Nb=function(a,b,c,e){a||(a=this);this.parent=a;this.j=a.j;this.L=null;this.id=Hb++;this.name=b;this.mode=c;this.c={};this.f={};this.rdev=e},Nb.prototype={},Object.defineProperties(Nb.prototype,{read:{get:function(){return 365===(this.mode&365)},set:function(a){a?this.mode|=365:this.mode&=-366}},write:{get:function(){return 146===(this.mode&146)},set:function(a){a?this.mode|=146:this.mode&=-147}},Ha:{get:function(){return N(this.mode)}},Ga:{get:function(){return 8192===(this.mode&
61440)}}}));a=new Nb(a,b,c,e);Lb(a);return a}function N(a){return 16384===(a&61440)}var Ob={r:0,rs:1052672,"r+":2,w:577,wx:705,xw:705,"w+":578,"wx+":706,"xw+":706,a:1089,ax:1217,xa:1217,"a+":1090,"ax+":1218,"xa+":1218};function Pb(a){var b=["r","w","rw"][a&3];a&512&&(b+="w");return b}function Mb(a,b){if(Ib)return 0;if(-1===b.indexOf("r")||a.mode&292){if(-1!==b.indexOf("w")&&!(a.mode&146)||-1!==b.indexOf("x")&&!(a.mode&73))return I.R}else return I.R;return 0}
function Qb(a,b){try{return Bb(a,b),I.ba}catch(c){}return Mb(a,"wx")}function Rb(a){var b=4096;for(a=a||0;a<=b;a++)if(!Gb[a])return a;throw new K(I.va);}function Sb(a,b){Tb||(Tb=function(){},Tb.prototype={},Object.defineProperties(Tb.prototype,{object:{get:function(){return this.node},set:function(a){this.node=a}}}));var c=new Tb,e;for(e in a)c[e]=a[e];a=c;b=Rb(b);a.fd=b;return Gb[b]=a}var yb={open:function(a){a.f=Eb[a.node.rdev].f;a.f.open&&a.f.open(a)},A:function(){throw new K(I.N);}};
function tb(a,b){Eb[a]={f:b}}function Ub(a,b){var c="/"===b,e=!b;if(c&&Db)throw new K(I.S);if(!c&&!e){var f=R(b,{na:!1});b=f.path;f=f.node;if(f.L)throw new K(I.S);if(!N(f.mode))throw new K(I.da);}b={type:a,Y:{},ta:b,Ja:[]};a=a.j(b);a.j=b;b.root=a;c?Db=a:f&&(f.L=b,f.j&&f.j.Ja.push(b))}function Vb(a,b,c){var e=R(a,{parent:!0}).node;a=ob(a);if(!a||"."===a||".."===a)throw new K(I.h);var f=Qb(e,a);if(f)throw new K(f);if(!e.c.K)throw new K(I.F);return e.c.K(e,a,b,c)}
function T(a,b){return Vb(a,(void 0!==b?b:511)&1023|16384,0)}function Wb(a,b,c){"undefined"===typeof c&&(c=b,b=438);return Vb(a,b|8192,c)}function Xb(a,b){if(!qb(a))throw new K(I.v);var c=R(b,{parent:!0}).node;if(!c)throw new K(I.v);b=ob(b);var e=Qb(c,b);if(e)throw new K(e);if(!c.c.symlink)throw new K(I.F);return c.c.symlink(c,b,a)}
function Yb(a){var b=R(a,{parent:!0}).node,c=ob(a),e=Bb(b,c);a:{try{var f=Bb(b,c)}catch(g){f=g.m;break a}var h=Mb(b,"wx");f=h?h:N(f.mode)?I.I:0}if(f)throw new K(f);if(!b.c.unlink)throw new K(I.F);if(e.L)throw new K(I.S);try{Q.willDeletePath&&Q.willDeletePath(a)}catch(g){console.log("FS.trackingDelegate['willDeletePath']('"+a+"') threw an exception: "+g.message)}b.c.unlink(b,c);b=Kb(e.parent.id,e.name);if(P[b]===e)P[b]=e.G;else for(b=P[b];b;){if(b.G===e){b.G=e.G;break}b=b.G}try{if(Q.onDeletePath)Q.onDeletePath(a)}catch(g){console.log("FS.trackingDelegate['onDeletePath']('"+
a+"') threw an exception: "+g.message)}}function Jb(a){a=R(a).node;if(!a)throw new K(I.v);if(!a.c.readlink)throw new K(I.h);return qb(S(a.parent),a.c.readlink(a))}function Zb(a,b){var c;"string"===typeof a?c=R(a,{J:!0}).node:c=a;if(!c.c.i)throw new K(I.F);c.c.i(c,{mode:b&4095|c.mode&-4096,timestamp:Date.now()})}
function $b(a,b,c,e){if(""===a)throw new K(I.v);if("string"===typeof b){var f=Ob[b];if("undefined"===typeof f)throw Error("Unknown file open mode: "+b);b=f}c=b&64?("undefined"===typeof c?438:c)&4095|32768:0;if("object"===typeof a)var h=a;else{a=mb(a);try{h=R(a,{J:!(b&131072)}).node}catch(x){}}f=!1;if(b&64)if(h){if(b&128)throw new K(I.ba);}else h=Vb(a,c,0),f=!0;if(!h)throw new K(I.v);8192===(h.mode&61440)&&(b&=-513);if(b&65536&&!N(h.mode))throw new K(I.da);if(!f){var g=h?40960===(h.mode&61440)?I.T:
N(h.mode)&&("r"!==Pb(b)||b&512)?I.I:Mb(h,Pb(b)):I.v;if(g)throw new K(g);}if(b&512){c=h;var n;"string"===typeof c?n=R(c,{J:!0}).node:n=c;if(!n.c.i)throw new K(I.F);if(N(n.mode))throw new K(I.I);if(32768!==(n.mode&61440))throw new K(I.h);if(c=Mb(n,"w"))throw new K(c);n.c.i(n,{size:0,timestamp:Date.now()})}b&=-641;e=Sb({node:h,path:S(h),flags:b,seekable:!0,position:0,f:h.f,Ra:[],error:!1},e);e.f.open&&e.f.open(e);!d.logReadFiles||b&1||(ac||(ac={}),a in ac||(ac[a]=1,g("read file: "+a)));try{Q.onOpenFile&&
(g=0,1!==(b&2097155)&&(g|=1),0!==(b&2097155)&&(g|=2),Q.onOpenFile(a,g))}catch(x){console.log("FS.trackingDelegate['onOpenFile']('"+a+"', flags) threw an exception: "+x.message)}return e}function bc(a){if(null===a.fd)throw new K(I.u);a.W&&(a.W=null);try{a.f.close&&a.f.close(a)}catch(b){throw b;}finally{Gb[a.fd]=null}a.fd=null}function cc(a,b,c){if(null===a.fd)throw new K(I.u);if(!a.seekable||!a.f.A)throw new K(I.N);a.position=a.f.A(a,b,c);a.Ra=[]}
function dc(a,b,c,e,f,h){if(0>e||0>f)throw new K(I.h);if(null===a.fd)throw new K(I.u);if(0===(a.flags&2097155))throw new K(I.u);if(N(a.node.mode))throw new K(I.I);if(!a.f.write)throw new K(I.h);a.flags&1024&&cc(a,0,2);var g="undefined"!==typeof f;if(!g)f=a.position;else if(!a.seekable)throw new K(I.N);b=a.f.write(a,b,c,e,f,h);g||(a.position+=b);try{if(a.path&&Q.onWriteToFile)Q.onWriteToFile(a.path)}catch(n){console.log("FS.trackingDelegate['onWriteToFile']('"+path+"') threw an exception: "+n.message)}return b}
function ec(){K||(K=function(a,b){this.node=b;this.Pa=function(a){this.m=a;for(var b in I)if(I[b]===a){this.code=b;break}};this.Pa(a);this.message=jb[a];this.stack&&Object.defineProperty(this,"stack",{value:Error().stack,writable:!0});this.stack&&(this.stack=Fa(this.stack))},K.prototype=Error(),K.prototype.constructor=K,[I.v].forEach(function(a){Ab[a]=new K(a);Ab[a].stack="<generic error, no stack>"}))}var fc;function hc(a,b){var c=0;a&&(c|=365);b&&(c|=146);return c}
function ic(a,b,c,e){a=J("string"===typeof a?a:S(a),b);return T(a,hc(c,e))}function jc(a,b){a="string"===typeof a?a:S(a);for(b=b.split("/").reverse();b.length;){var c=b.pop();if(c){var e=J(a,c);try{T(e)}catch(f){}a=e}}return e}function kc(a,b,c,e){a=J("string"===typeof a?a:S(a),b);c=hc(c,e);return Vb(a,(void 0!==c?c:438)&4095|32768,0)}
function lc(a,b,c,e,f,h){a=b?J("string"===typeof a?a:S(a),b):a;e=hc(e,f);f=Vb(a,(void 0!==e?e:438)&4095|32768,0);if(c){if("string"===typeof c){a=Array(c.length);b=0;for(var g=c.length;b<g;++b)a[b]=c.charCodeAt(b);c=a}Zb(f,e|146);a=$b(f,"w");dc(a,c,0,c.length,0,h);bc(a);Zb(f,e)}return f}
function U(a,b,c,e){a=J("string"===typeof a?a:S(a),b);b=hc(!!c,!!e);U.ra||(U.ra=64);var f=U.ra++<<8|0;tb(f,{open:function(a){a.seekable=!1},close:function(){e&&e.buffer&&e.buffer.length&&e(10)},read:function(a,b,e,f){for(var g=0,h=0;h<f;h++){try{var n=c()}catch(M){throw new K(I.B);}if(void 0===n&&0===g)throw new K(I.aa);if(null===n||void 0===n)break;g++;b[e+h]=n}g&&(a.node.timestamp=Date.now());return g},write:function(a,b,c,f){for(var g=0;g<f;g++)try{e(b[c+g])}catch(t){throw new K(I.B);}f&&(a.node.timestamp=
Date.now());return g}});return Wb(a,b,f)}function mc(a,b,c){a=J("string"===typeof a?a:S(a),b);return Xb(c,a)}
function nc(a){if(a.Ga||a.Ha||a.link||a.b)return!0;var b=!0;if("undefined"!==typeof XMLHttpRequest)throw Error("Lazy loading should have been performed (contents set) in createLazyFile, but it was not. Lazy loading only works in web workers. Use --embed-file or --preload-file in emcc on the main thread.");if(d.read)try{a.b=vb(d.read(a.url),!0),a.g=a.b.length}catch(c){b=!1}else throw Error("Cannot load without read() or XMLHttpRequest.");b||kb(I.B);return b}
function oc(a,b,c,e,f){function h(){this.X=!1;this.O=[]}h.prototype.get=function(a){if(!(a>this.length-1||0>a)){var b=a%this.chunkSize;return this.qa(a/this.chunkSize|0)[b]}};h.prototype.Oa=function(a){this.qa=a};h.prototype.ja=function(){var a=new XMLHttpRequest;a.open("HEAD",c,!1);a.send(null);if(!(200<=a.status&&300>a.status||304===a.status))throw Error("Couldn't load "+c+". Status: "+a.status);var b=Number(a.getResponseHeader("Content-length")),e,f=(e=a.getResponseHeader("Accept-Ranges"))&&"bytes"===
e;a=(e=a.getResponseHeader("Content-Encoding"))&&"gzip"===e;var g=1048576;f||(g=b);var h=this;h.Oa(function(a){var e=a*g,f=(a+1)*g-1;f=Math.min(f,b-1);if("undefined"===typeof h.O[a]){var n=h.O;if(e>f)throw Error("invalid range ("+e+", "+f+") or no bytes requested!");if(f>b-1)throw Error("only "+b+" bytes available! programmer error!");var t=new XMLHttpRequest;t.open("GET",c,!1);b!==g&&t.setRequestHeader("Range","bytes="+e+"-"+f);"undefined"!=typeof Uint8Array&&(t.responseType="arraybuffer");t.overrideMimeType&&
t.overrideMimeType("text/plain; charset=x-user-defined");t.send(null);if(!(200<=t.status&&300>t.status||304===t.status))throw Error("Couldn't load "+c+". Status: "+t.status);e=void 0!==t.response?new Uint8Array(t.response||[]):vb(t.responseText||"",!0);n[a]=e}if("undefined"===typeof h.O[a])throw Error("doXHR failed!");return h.O[a]});if(a||!b)g=b=1,g=b=this.qa(0).length,console.log("LazyFiles on gzip forces download of the whole file when length is accessed");this.za=b;this.ya=g;this.X=!0};if("undefined"!==
typeof XMLHttpRequest){if(!l)throw"Cannot do synchronous binary XHRs outside webworkers in modern browsers. Use --embed-file or --preload-file in emcc";var g=new h;Object.defineProperties(g,{length:{get:function(){this.X||this.ja();return this.za}},chunkSize:{get:function(){this.X||this.ja();return this.ya}}});var n=void 0}else n=c,g=void 0;var x=kc(a,b,e,f);g?x.b=g:n&&(x.b=null,x.url=n);Object.defineProperties(x,{g:{get:function(){return this.b.length}}});var z={};Object.keys(x.f).forEach(function(a){var b=
x.f[a];z[a]=function(){if(!nc(x))throw new K(I.B);return b.apply(null,arguments)}});z.read=function(a,b,c,e,f){if(!nc(x))throw new K(I.B);a=a.node.b;if(f>=a.length)return 0;e=Math.min(a.length-f,e);assert(0<=e);if(a.slice)for(var g=0;g<e;g++)b[c+g]=a[f+g];else for(g=0;g<e;g++)b[c+g]=a.get(f+g);return e};x.f=z;return x}
function pc(a,b,c,e,f,h,g,n,x,z){function t(c){function t(c){z&&z();n||lc(a,b,c,e,f,x);h&&h();eb(M)}var D=!1;d.preloadPlugins.forEach(function(a){!D&&a.canHandle(V)&&(a.handle(c,V,t,function(){g&&g();eb(M)}),D=!0)});D||t(c)}Browser.Tc();var V=b?qb(J(a,b)):a,M=cb("cp "+V);db(M);"string"==typeof c?Browser.Rc(c,function(a){t(a)},g):t(c)}var FS={},Nb,Tb,ac,W=0;function X(){W+=4;return y[W-4>>2]}function qc(){var a=Gb[X()];if(!a)throw new K(I.u);return a}function rc(a){return Math.pow(2,a)}
function sc(a){return Math.log(a)/Math.LN10}var Y=u;u+=48;Aa(vb("GMT"),2);function tc(){function a(a){return(a=a.toTimeString().match(/\(([A-Za-z ]+)\)$/))?a[1]:"GMT"}if(!uc){uc=!0;y[vc()>>2]=60*(new Date).getTimezoneOffset();var b=new Date(2E3,0,1),c=new Date(2E3,6,1);y[wc()>>2]=Number(b.getTimezoneOffset()!=c.getTimezoneOffset());var e=a(b),f=a(c);e=Aa(vb(e),0);f=Aa(vb(f),0);c.getTimezoneOffset()<b.getTimezoneOffset()?(y[xc()>>2]=e,y[xc()+4>>2]=f):(y[xc()>>2]=f,y[xc()+4>>2]=e)}}var uc;ec();P=Array(4096);
Ub(L,"/");T("/tmp");T("/home");T("/home/web_user");(function(){T("/dev");tb(259,{read:function(){return 0},write:function(a,b,f,h){return h}});Wb("/dev/null",259);sb(1280,wb);sb(1536,xb);Wb("/dev/tty",1280);Wb("/dev/tty1",1536);if("undefined"!==typeof crypto){var a=new Uint8Array(1);var b=function(){crypto.getRandomValues(a);return a[0]}}else m?b=function(){return require("crypto").randomBytes(1)[0]}:b=function(){q("random_device")};U("/dev","random",b);U("/dev","urandom",b);T("/dev/shm");T("/dev/shm/tmp")})();
T("/proc");T("/proc/self");T("/proc/self/fd");Ub({j:function(){var a=zb("/proc/self","fd",16895,73);a.c={lookup:function(a,c){var b=Gb[+c];if(!b)throw new K(I.u);a={parent:null,j:{ta:"fake"},c:{readlink:function(){return b.path}}};return a.parent=a}};return a}},"/proc/self/fd");
Ua.unshift(function(){if(!d.noFSInit&&!fc){assert(!fc,"FS.init was previously called. If you want to initialize later with custom parameters, remove any earlier calls (note that one is automatically added to the generated code)");fc=!0;ec();d.stdin=d.stdin;d.stdout=d.stdout;d.stderr=d.stderr;d.stdin?U("/dev","stdin",d.stdin):Xb("/dev/tty","/dev/stdin");d.stdout?U("/dev","stdout",null,d.stdout):Xb("/dev/tty","/dev/stdout");d.stderr?U("/dev","stderr",null,d.stderr):Xb("/dev/tty1","/dev/stderr");var a=
$b("/dev/stdin","r");assert(0===a.fd,"invalid handle for stdin ("+a.fd+")");a=$b("/dev/stdout","w");assert(1===a.fd,"invalid handle for stdout ("+a.fd+")");a=$b("/dev/stderr","w");assert(2===a.fd,"invalid handle for stderr ("+a.fd+")")}});Va.push(function(){Ib=!1});Wa.push(function(){fc=!1;var a=d._fflush;a&&a(0);for(a=0;a<Gb.length;a++){var b=Gb[a];b&&bc(b)}});d.FS_createFolder=ic;d.FS_createPath=jc;d.FS_createDataFile=lc;d.FS_createPreloadedFile=pc;d.FS_createLazyFile=oc;d.FS_createLink=mc;
d.FS_createDevice=U;d.FS_unlink=Yb;Ua.unshift(function(){});Wa.push(function(){});if(m){var fs=require("fs"),Cb=require("path");O.Qa()}w=na(4);Na=Oa=ra(u);F=Na+Ra;Pa=ra(F);y[w>>2]=Pa;oa=!0;assert(Pa<v,"TOTAL_MEMORY not big enough for stack");
function vb(a,b){for(var c=0,e=0;e<a.length;++e){var f=a.charCodeAt(e);55296<=f&&57343>=f&&(f=65536+((f&1023)<<10)|a.charCodeAt(++e)&1023);127>=f?++c:c=2047>=f?c+2:65535>=f?c+3:2097151>=f?c+4:67108863>=f?c+5:c+6}c=Array(c+1);a=xa(a,c,0,c.length);b&&(c.length=a);return c}d.wasmTableSize=4152;d.wasmMaxTableSize=4152;d.Aa={};
d.Ba={enlargeMemory:function(){qa()},getTotalMemory:function(){return v},abortOnCannotGrowMemory:qa,abortStackOverflow:function(a){q("Stack overflow! Attempted to allocate "+a+" bytes on the stack, but stack has only "+(F-ka()+a)+" bytes available!")},nullFunc_ii:function(a){r("Invalid function pointer called with signature 'ii'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");
r("Build with ASSERTIONS=2 for more info.");q(a)},nullFunc_iii:function(a){r("Invalid function pointer called with signature 'iii'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");r("Build with ASSERTIONS=2 for more info.");q(a)},nullFunc_iiii:function(a){r("Invalid function pointer called with signature 'iiii'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");
r("Build with ASSERTIONS=2 for more info.");q(a)},nullFunc_v:function(a){r("Invalid function pointer called with signature 'v'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");r("Build with ASSERTIONS=2 for more info.");q(a)},nullFunc_vi:function(a){r("Invalid function pointer called with signature 'vi'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");
r("Build with ASSERTIONS=2 for more info.");q(a)},nullFunc_vii:function(a){r("Invalid function pointer called with signature 'vii'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");r("Build with ASSERTIONS=2 for more info.");q(a)},___assert_fail:function(a,
b,c,e){q("Assertion failed: "+E(a)+", at: "+[b?E(b):"unknown filename",c,e?E(e):"unknown function"])},___buildEnvironment:ib,___lock:function(){},___setErrNo:kb,___syscall114:function(a,b){W=b;try{q("cannot wait on child processes")}catch(c){return"undefined"!==typeof FS&&c instanceof K||q(c),-c.m}},___syscall140:function(a,b){W=b;try{var c=qc();X();var e=X(),f=X(),h=X();cc(c,e,h);y[f>>2]=c.position;c.W&&0===e&&0===h&&(c.W=null);return 0}catch(g){return"undefined"!==typeof FS&&g instanceof K||q(g),
-g.m}},___syscall145:function(a,b){W=b;try{var c=qc(),e=X();a:{var f=X();for(b=a=0;b<f;b++){var h=y[e+(8*b+4)>>2],g=c,n=y[e+8*b>>2],x=h,z=void 0,t=A;if(0>x||0>z)throw new K(I.h);if(null===g.fd)throw new K(I.u);if(1===(g.flags&2097155))throw new K(I.u);if(N(g.node.mode))throw new K(I.I);if(!g.f.read)throw new K(I.h);var V="undefined"!==typeof z;if(!V)z=g.position;else if(!g.seekable)throw new K(I.N);var M=g.f.read(g,t,n,x,z);V||(g.position+=M);var D=M;if(0>D){var Fb=-1;break a}a+=D;if(D<h)break}Fb=
a}return Fb}catch(ta){return"undefined"!==typeof FS&&ta instanceof K||q(ta),-ta.m}},___syscall146:function(a,b){W=b;try{var c=qc(),e=X();a:{var f=X();for(b=a=0;b<f;b++){var h=dc(c,A,y[e+8*b>>2],y[e+(8*b+4)>>2],void 0);if(0>h){var g=-1;break a}a+=h}g=a}return g}catch(n){return"undefined"!==typeof FS&&n instanceof K||q(n),-n.m}},___syscall221:function(a,b){W=b;try{var c=qc();switch(X()){case 0:var e=X();return 0>e?-I.h:$b(c.path,c.flags,0,e).fd;case 1:case 2:return 0;case 3:return c.flags;case 4:return e=
X(),c.flags|=e,0;case 12:case 12:return e=X(),Ha[e+0>>1]=2,0;case 13:case 14:case 13:case 14:return 0;case 16:case 8:return-I.h;case 9:return kb(I.h),-1;default:return-I.h}}catch(f){return"undefined"!==typeof FS&&f instanceof K||q(f),-f.m}},___syscall5:function(a,b){W=b;try{var c=E(X()),e=X(),f=X();return $b(c,e,f).fd}catch(h){return"undefined"!==typeof FS&&h instanceof K||q(h),-h.m}},___syscall54:function(a,b){W=b;try{var c=qc(),e=X();switch(e){case 21509:case 21505:return c.tty?0:-I.C;case 21510:case 21511:case 21512:case 21506:case 21507:case 21508:return c.tty?
0:-I.C;case 21519:if(!c.tty)return-I.C;var f=X();return y[f>>2]=0;case 21520:return c.tty?-I.h:-I.C;case 21531:a=f=X();if(!c.f.Fa)throw new K(I.C);return c.f.Fa(c,e,a);case 21523:return c.tty?0:-I.C;case 21524:return c.tty?0:-I.C;default:q("bad ioctl syscall "+e)}}catch(h){return"undefined"!==typeof FS&&h instanceof K||q(h),-h.m}},___syscall6:function(a,b){W=b;try{var c=qc();bc(c);return 0}catch(e){return"undefined"!==typeof FS&&e instanceof K||q(e),-e.m}},___unlock:function(){},_abort:function(){d.abort()},
_emscripten_memcpy_big:function(a,b,c){B.set(B.subarray(b,b+c),a);return a},_emscripten_run_script:function(a){eval(E(a))},_exit:function(a){yc();if(d.noExitRuntime)r("exit("+a+") called, but EXIT_RUNTIME is not set, so halting execution but not exiting the runtime or preventing further async execution (build with EXIT_RUNTIME=1, if you want a true shutdown)");else if(wa=!0,Oa=zc,Qa(),Sa(Wa),G=!0,d.onExit)d.onExit(a);d.quit(a,new ia(a))},_gettimeofday:function(a){var b=Date.now();y[a>>2]=b/1E3|0;
y[a+4>>2]=b%1E3*1E3|0;return 0},_llvm_exp2_f64:function(){return rc.apply(null,arguments)},_llvm_log10_f64:function(){return sc.apply(null,arguments)},_localtime:function(a){tc();a=new Date(1E3*y[a>>2]);y[Y>>2]=a.getSeconds();y[Y+4>>2]=a.getMinutes();y[Y+8>>2]=a.getHours();y[Y+12>>2]=a.getDate();y[Y+16>>2]=a.getMonth();y[Y+20>>2]=a.getFullYear()-1900;y[Y+24>>2]=a.getDay();var b=new Date(a.getFullYear(),0,1);y[Y+28>>2]=(a.getTime()-b.getTime())/864E5|0;y[Y+36>>2]=-(60*a.getTimezoneOffset());var c=
(new Date(2E3,6,1)).getTimezoneOffset();b=b.getTimezoneOffset();a=(c!=b&&a.getTimezoneOffset()==Math.min(b,c))|0;y[Y+32>>2]=a;a=y[xc()+(a?4:0)>>2];y[Y+40>>2]=a;return Y},_time:function(a){var b=Date.now()/1E3|0;a&&(y[a>>2]=b);return b},_uade_notify_song_update:function(a,b,c,e){b=E(b);c=E(c);e=E(e);var f=window.rawBytes(d,a),h=window.getStringifiedLines(f);f="";for(a=0;a<h.length;a++)f+=h[a]+"<br>";var g={},n=null,x=null;for(a=0;a<h.length;a++){var z=h[a];if(aa(z,"Music:")){var t=g.authorname?g.authorname:
[];t.unshift(z.substring(6).trim());g.authorname=t}else if(aa(z,"DeliCustom:"))t=g.specialinfo?g.specialinfo:[],t.unshift(z.substring(11).trim()),g.specialinfo=t;else if(aa(z,"File name:"))t=g.title?g.title:[],t.unshift(z.substring(10).trim()),g.title=t;else if(aa(z,"Song title:"))t=g.title?g.title:[],t.unshift(z.substring(11).trim()),g.title=t;else if(aa(z,"File prefix:"))g.prefix=z.substring(12).trim();else{a:{var V=z,M="MODULENAME: AUTHORNAME: SPECIALINFO: VERSION: CREDITS: Remarks:".split(" ");
for(t=0;t<M.length;t++){var D=M[t];if(V.match("^"+D)==D){t=D;break a}}t=null}t?(n&&x&&(g[n]=x,x=null),n=t.substring(0,t.length-1).toLowerCase()):(z=z.trim(),z.length&&n&&(x||(x=[]),x.push(z)))}}n&&x&&(g[n]=x);a=[];!g.title||"modulename"in g||a.push(g.title.shift()+" ("+g.prefix+")");for(;3>=a.length;)if("modulename"in g)a.push(g.modulename.shift()),delete g.modulename;else if("authorname"in g)a.push(g.authorname.shift()),delete g.authorname;else if("specialinfo"in g)a.push(g.specialinfo.shift()),
delete g.specialinfo;else if("version"in g)a.push(g.version.shift()),0==g.version.length&&delete g.version;else if("credits"in g)a.push(g.credits.shift()),0==g.credits.length&&delete g.credits;else if("remarks"in g)a.push(g.remarks.shift()),0==g.remarks.length&&delete g.remarks;else break;g=1<a.length?a[1]:"";h=2<a.length?a[2]:"";n=[];n.info1=0<a.length?a[0]:"";n.info2=g;n.info3=h;n.minText=b;n.maxText=c;n.currText=e;n.infoText=f;return window.songUpdateCallback(n)},_uade_request_file:function(a){return window.fileRequestCallback(a)},
_uade_request_file_size:function(a){return window.fileSizeRequestCallback(a)},DYNAMICTOP_PTR:w,STACKTOP:Oa,STACK_MAX:F};var Z=d.asm(d.Aa,d.Ba,buffer),Ac=Z.___emscripten_environ_constructor;Z.___emscripten_environ_constructor=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Ac.apply(null,arguments)};var Bc=Z.___errno_location;
Z.___errno_location=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Bc.apply(null,arguments)};var Cc=Z.__get_daylight;
Z.__get_daylight=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Cc.apply(null,arguments)};var Dc=Z.__get_timezone;
Z.__get_timezone=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Dc.apply(null,arguments)};var Ec=Z.__get_tzname;
Z.__get_tzname=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Ec.apply(null,arguments)};var Fc=Z._emu_compute_audio_samples;
Z._emu_compute_audio_samples=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Fc.apply(null,arguments)};var Gc=Z._emu_get_audio_buffer;
Z._emu_get_audio_buffer=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Gc.apply(null,arguments)};var Hc=Z._emu_get_audio_buffer_length;
Z._emu_get_audio_buffer_length=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Hc.apply(null,arguments)};var Ic=Z._emu_get_number_trace_streams;
Z._emu_get_number_trace_streams=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Ic.apply(null,arguments)};var Jc=Z._emu_get_trace_streams;
Z._emu_get_trace_streams=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Jc.apply(null,arguments)};var Kc=Z._emu_init;
Z._emu_init=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Kc.apply(null,arguments)};var Lc=Z._emu_is_exit;Z._emu_is_exit=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Lc.apply(null,arguments)};
var Mc=Z._emu_prepare;Z._emu_prepare=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Mc.apply(null,arguments)};var Nc=Z._emu_set_panning;
Z._emu_set_panning=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Nc.apply(null,arguments)};var Oc=Z._emu_set_subsong;
Z._emu_set_subsong=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Oc.apply(null,arguments)};var Pc=Z._emu_teardown;
Z._emu_teardown=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Pc.apply(null,arguments)};var Qc=Z._fflush;Z._fflush=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Qc.apply(null,arguments)};
var Rc=Z._free;Z._free=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Rc.apply(null,arguments)};var Sc=Z._llvm_bswap_i16;
Z._llvm_bswap_i16=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Sc.apply(null,arguments)};var Tc=Z._llvm_bswap_i32;
Z._llvm_bswap_i32=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Tc.apply(null,arguments)};var Uc=Z._malloc;Z._malloc=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Uc.apply(null,arguments)};
var Vc=Z._sbrk;Z._sbrk=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Vc.apply(null,arguments)};var Wc=Z.establishStackSpace;
Z.establishStackSpace=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Wc.apply(null,arguments)};var Xc=Z.getTempRet0;
Z.getTempRet0=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Xc.apply(null,arguments)};var Yc=Z.setTempRet0;Z.setTempRet0=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Yc.apply(null,arguments)};
var Zc=Z.setThrew;Z.setThrew=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Zc.apply(null,arguments)};var $c=Z.stackAlloc;
Z.stackAlloc=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return $c.apply(null,arguments)};var ad=Z.stackRestore;Z.stackRestore=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return ad.apply(null,arguments)};
var bd=Z.stackSave;Z.stackSave=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return bd.apply(null,arguments)};d.asm=Z;
var gb=d.___emscripten_environ_constructor=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.___emscripten_environ_constructor.apply(null,arguments)};
d.___errno_location=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.___errno_location.apply(null,arguments)};
var wc=d.__get_daylight=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.__get_daylight.apply(null,arguments)},vc=d.__get_timezone=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.__get_timezone.apply(null,
arguments)},xc=d.__get_tzname=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.__get_tzname.apply(null,arguments)};
d._emu_compute_audio_samples=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._emu_compute_audio_samples.apply(null,arguments)};
d._emu_get_audio_buffer=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._emu_get_audio_buffer.apply(null,arguments)};
d._emu_get_audio_buffer_length=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._emu_get_audio_buffer_length.apply(null,arguments)};
d._emu_get_number_trace_streams=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._emu_get_number_trace_streams.apply(null,arguments)};
d._emu_get_trace_streams=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._emu_get_trace_streams.apply(null,arguments)};
d._emu_init=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._emu_init.apply(null,arguments)};
d._emu_is_exit=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._emu_is_exit.apply(null,arguments)};
d._emu_prepare=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._emu_prepare.apply(null,arguments)};
d._emu_set_panning=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._emu_set_panning.apply(null,arguments)};
d._emu_set_subsong=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._emu_set_subsong.apply(null,arguments)};
d._emu_teardown=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._emu_teardown.apply(null,arguments)};
d._fflush=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._fflush.apply(null,arguments)};d._free=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._free.apply(null,arguments)};
d._llvm_bswap_i16=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._llvm_bswap_i16.apply(null,arguments)};
d._llvm_bswap_i32=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._llvm_bswap_i32.apply(null,arguments)};
var Ba=d._malloc=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._malloc.apply(null,arguments)};d._sbrk=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._sbrk.apply(null,arguments)};
d.establishStackSpace=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.establishStackSpace.apply(null,arguments)};
d.getTempRet0=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.getTempRet0.apply(null,arguments)};
d.setTempRet0=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.setTempRet0.apply(null,arguments)};
d.setThrew=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.setThrew.apply(null,arguments)};
var ma=d.stackAlloc=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.stackAlloc.apply(null,arguments)},la=d.stackRestore=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.stackRestore.apply(null,
arguments)},ka=d.stackSave=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.stackSave.apply(null,arguments)};
d.dynCall_v=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.dynCall_v.apply(null,arguments)};
d.dynCall_vi=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.dynCall_vi.apply(null,arguments)};d.asm=Z;d.intArrayFromString||(d.intArrayFromString=function(){q("'intArrayFromString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.intArrayToString||(d.intArrayToString=function(){q("'intArrayToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});
d.ccall=function(a,b,c,e){var f=d["_"+a];assert(f,"Cannot call unknown function "+a+", make sure it is exported");var h=[];a=0;assert("array"!==b,'Return type should not be "array".');if(e)for(var g=0;g<e.length;g++){var n=za[c[g]];n?(0===a&&(a=ka()),h[g]=n(e[g])):h[g]=e[g]}c=f.apply(null,h);c="string"===b?E(c):"boolean"===b?!!c:c;0!==a&&la(a);return c};d.cwrap||(d.cwrap=function(){q("'cwrap' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.setValue||(d.setValue=function(){q("'setValue' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});
d.getValue=function(a,b){b=b||"i8";"*"===b.charAt(b.length-1)&&(b="i32");switch(b){case "i1":return A[a>>0];case "i8":return A[a>>0];case "i16":return Ha[a>>1];case "i32":return y[a>>2];case "i64":return y[a>>2];case "float":return Ja[a>>2];case "double":return Ka[a>>3];default:q("invalid type for getValue: "+b)}return null};d.allocate||(d.allocate=function(){q("'allocate' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.getMemory=Ca;d.Pointer_stringify=E;
d.AsciiToString||(d.AsciiToString=function(){q("'AsciiToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.stringToAscii||(d.stringToAscii=function(){q("'stringToAscii' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.UTF8ArrayToString||(d.UTF8ArrayToString=function(){q("'UTF8ArrayToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.UTF8ToString||(d.UTF8ToString=function(){q("'UTF8ToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});
d.stringToUTF8Array||(d.stringToUTF8Array=function(){q("'stringToUTF8Array' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.stringToUTF8||(d.stringToUTF8=function(){q("'stringToUTF8' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.lengthBytesUTF8||(d.lengthBytesUTF8=function(){q("'lengthBytesUTF8' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.UTF16ToString||(d.UTF16ToString=function(){q("'UTF16ToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});
d.stringToUTF16||(d.stringToUTF16=function(){q("'stringToUTF16' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.lengthBytesUTF16||(d.lengthBytesUTF16=function(){q("'lengthBytesUTF16' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.UTF32ToString||(d.UTF32ToString=function(){q("'UTF32ToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.stringToUTF32||(d.stringToUTF32=function(){q("'stringToUTF32' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});
d.lengthBytesUTF32||(d.lengthBytesUTF32=function(){q("'lengthBytesUTF32' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.allocateUTF8||(d.allocateUTF8=function(){q("'allocateUTF8' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.stackTrace||(d.stackTrace=function(){q("'stackTrace' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.addOnPreRun||(d.addOnPreRun=function(){q("'addOnPreRun' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});
d.addOnInit||(d.addOnInit=function(){q("'addOnInit' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.addOnPreMain||(d.addOnPreMain=function(){q("'addOnPreMain' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.addOnExit||(d.addOnExit=function(){q("'addOnExit' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.addOnPostRun||(d.addOnPostRun=function(){q("'addOnPostRun' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});
d.writeStringToMemory||(d.writeStringToMemory=function(){q("'writeStringToMemory' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.writeArrayToMemory||(d.writeArrayToMemory=function(){q("'writeArrayToMemory' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.writeAsciiToMemory||(d.writeAsciiToMemory=function(){q("'writeAsciiToMemory' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.addRunDependency=db;
d.removeRunDependency=eb;d.ENV||(d.ENV=function(){q("'ENV' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.FS||(d.FS=function(){q("'FS' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.FS_createFolder=ic;d.FS_createPath=jc;d.FS_createDataFile=lc;d.FS_createPreloadedFile=pc;d.FS_createLazyFile=oc;d.FS_createLink=mc;d.FS_createDevice=U;d.FS_unlink=Yb;d.GL||(d.GL=function(){q("'GL' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});
d.staticAlloc||(d.staticAlloc=function(){q("'staticAlloc' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.dynamicAlloc||(d.dynamicAlloc=function(){q("'dynamicAlloc' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.warnOnce||(d.warnOnce=function(){q("'warnOnce' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.loadDynamicLibrary||(d.loadDynamicLibrary=function(){q("'loadDynamicLibrary' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});
d.loadWebAssemblyModule||(d.loadWebAssemblyModule=function(){q("'loadWebAssemblyModule' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.getLEB||(d.getLEB=function(){q("'getLEB' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.getFunctionTables||(d.getFunctionTables=function(){q("'getFunctionTables' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.alignFunctionTables||(d.alignFunctionTables=function(){q("'alignFunctionTables' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});
d.registerFunctions||(d.registerFunctions=function(){q("'registerFunctions' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.addFunction||(d.addFunction=function(){q("'addFunction' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.removeFunction||(d.removeFunction=function(){q("'removeFunction' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.getFuncWrapper||(d.getFuncWrapper=function(){q("'getFuncWrapper' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});
d.prettyPrint||(d.prettyPrint=function(){q("'prettyPrint' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.makeBigInt||(d.makeBigInt=function(){q("'makeBigInt' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.dynCall||(d.dynCall=function(){q("'dynCall' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.getCompilerSetting||(d.getCompilerSetting=function(){q("'getCompilerSetting' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});
d.stackSave||(d.stackSave=function(){q("'stackSave' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.stackRestore||(d.stackRestore=function(){q("'stackRestore' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.stackAlloc||(d.stackAlloc=function(){q("'stackAlloc' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.establishStackSpace||(d.establishStackSpace=function(){q("'establishStackSpace' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});
d.print||(d.print=function(){q("'print' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.printErr||(d.printErr=function(){q("'printErr' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.ALLOC_NORMAL||Object.defineProperty(d,"ALLOC_NORMAL",{get:function(){q("'ALLOC_NORMAL' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")}});d.ALLOC_STACK||Object.defineProperty(d,"ALLOC_STACK",{get:function(){q("'ALLOC_STACK' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")}});
d.ALLOC_STATIC||Object.defineProperty(d,"ALLOC_STATIC",{get:function(){q("'ALLOC_STATIC' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")}});d.ALLOC_DYNAMIC||Object.defineProperty(d,"ALLOC_DYNAMIC",{get:function(){q("'ALLOC_DYNAMIC' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")}});d.ALLOC_NONE||Object.defineProperty(d,"ALLOC_NONE",{get:function(){q("'ALLOC_NONE' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")}});
function ia(a){this.name="ExitStatus";this.message="Program terminated with exit("+a+")";this.status=a}ia.prototype=Error();ia.prototype.constructor=ia;var zc;ab=function cd(){d.calledRun||dd();d.calledRun||(ab=cd)};
function dd(){function a(){if(!d.calledRun&&(d.calledRun=!0,!wa)){Qa();C||(C=!0,Sa(Ua));Qa();Sa(Va);if(d.onRuntimeInitialized)d.onRuntimeInitialized();assert(!d._main,'compiled without a main, but one is present. if you added it from JS, use Module["onRuntimeInitialized"]');Qa();if(d.postRun)for("function"==typeof d.postRun&&(d.postRun=[d.postRun]);d.postRun.length;){var a=d.postRun.shift();Xa.unshift(a)}Sa(Xa)}}if(!(0<Za)){assert(0==(F&3));Ia[(F>>2)-1]=34821223;Ia[(F>>2)-2]=2310721022;if(d.preRun)for("function"==
typeof d.preRun&&(d.preRun=[d.preRun]);d.preRun.length;)Ya();Sa(Ta);0<Za||d.calledRun||(d.setStatus?(d.setStatus("Running..."),setTimeout(function(){setTimeout(function(){d.setStatus("")},1);a()},1)):a(),Qa())}}d.run=dd;
function yc(){var a=ja,b=r,c=!1;ja=r=function(){c=!0};try{var e=d._fflush;e&&e(0);["stdout","stderr"].forEach(function(a){a="/dev/"+a;try{var b=R(a,{J:!0});a=b.path}catch(n){}var e={Ia:!1,exists:!1,error:0,name:null,path:null,object:null,Ka:!1,Ma:null,La:null};try{b=R(a,{parent:!0}),e.Ka=!0,e.Ma=b.path,e.La=b.node,e.name=ob(a),b=R(a,{J:!0}),e.exists=!0,e.path=b.path,e.object=b.node,e.name=b.node.name,e.Ia="/"===b.path}catch(n){e.error=n.m}e&&(b=rb[e.object.rdev])&&b.output&&b.output.length&&(c=!0)})}catch(f){}ja=
a;r=b;c&&sa("stdio streams had content in them that was not flushed. you should set EXIT_RUNTIME to 1 (see the FAQ), or make sure to emit a newline when you printf etc.")}var ed=[];function q(a){if(d.onAbort)d.onAbort(a);void 0!==a?(ja(a),r(a),a=JSON.stringify(a)):a="";wa=!0;var b="abort("+a+") at "+Ga()+"";ed&&ed.forEach(function(c){b=c(b,a)});throw b;}d.abort=q;if(d.preInit)for("function"==typeof d.preInit&&(d.preInit=[d.preInit]);0<d.preInit.length;)d.preInit.pop()();d.noExitRuntime=!0;dd();
  return {
	Module: Module,  // expose original Module
  };
})(window.spp_backend_state_UADE);
/*
 uade_adapter.js: Adapts UADE backend to generic WebAudio/ScriptProcessor player.
 
 Known limitation: seeking is not supported by UADE
 
 version 1.01 with added support for "outside files"
 
 	Copyright (C) 2018 Juergen Wothke

 LICENSE
 
 This library is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or (at
 your option) any later version. This library is distributed in the hope
 that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
*/
UADEBackendAdapter = (function(){ var $this = function (basePath, modlandMode) { 
	$this.base.call(this, backend_UADE.Module, 2);
	
		// aka dumshit ftp.modland.com mode:
		this.modlandMode= (typeof modlandMode != 'undefined') ? modlandMode : 0;
		this.originalFile= "";
		this.modlandMap= {};	// mapping of weird shit filenames used on modland 

		this.basePath= basePath;
		this.isReady= false;
		
		if (!backend_UADE.Module.notReady) {
			// in sync scenario the "onRuntimeInitialized" has already fired before execution gets here,
			// i.e. it has to be called explicitly here (in async scenario "onRuntimeInitialized" will trigger
			// the call directly)
			this.doOnAdapterReady();
		}		
	}; 
	// UADE's sample buffer contains 2-byte integer sample data (i.e. must be rescaled) 
	// of 2 interleaved channels
	extend(EmsHEAP16BackendAdapter, $this, {
		doOnAdapterReady: function() {
			// called when runtime is ready (e.g. asynchronously when WASM is loaded)
			this.Module.ccall('emu_prepare', 'number', ['string'], [this.basePath]);	// init virtual FS
			this.isReady = true;
		},
		isAdapterReady: function() { 
			return this.isReady;
		},		
		
		getAudioBuffer: function() {
			var ptr=  this.Module.ccall('emu_get_audio_buffer', 'number');			
			// make it a this.Module.HEAP16 pointer
			return ptr >> 1;	// 2 x 16 bit samples			
		},
		getAudioBufferLength: function() {
			var len= this.Module.ccall('emu_get_audio_buffer_length', 'number') >>2;
			return len;
		},
		computeAudioSamples: function() {
			var status= this.Module.ccall('emu_compute_audio_samples', 'number');
			
			if (status == -1)  {
				return status;	// waiting for some file
			} else if (status > 0) {
				// song is done or error (file can not be loaded with no hope of recovery)

				var isExit= this.Module.ccall('emu_is_exit', 'number');
				if ( isExit) {
					return 2;		// error
				} else {
					return 1;		// end
				}
			}
			return 0;	
		},
		mapUrl: function(filename) {			
			// used transform the "internal filename" to a valid URL
			var uri= this.mapFs2Uri(filename);
			var p= uri.indexOf("@");	// cut off "basePath" for "outside" files
			if (p >= 0) {
				uri= uri.substring(p+1);
			}
			
			if (this.modlandMode) {
				if (uri.indexOf("/TFMX ST/")>0) {
					// restore original/incorect name for retrieval
					uri= uri.replace("/mdst.", "/mdat.");
				}
				if (uri.indexOf("/Zoundmonitor/")>0) {
					// restore original/incorect name for retrieval
					uri= uri.replace(".zm", ".sng");
					
					// modland idiots changed the layout and now put the sample files
					// into a separate "top" level folder.."
					uri= uri.replace("/Samples/", "/../Samples/");
				}
			}
			return uri;
		},
		/*
		* Creates the URL used to retrieve the song file.
		*/
		mapInternalFilename: function(overridePath, basePath, filename) {
			// the problem is that in UADE there is only one "basePath" and this specifies 
			// where to look for *any* files, i.e. uade prefixes this path to whatever
			// files it is tying to load (config/music - doesn't matter), i.e. a correct 
			// outside URL CANNOT be passed through UADE without being messed up in the process
			
			// solution: use a special marker for "outside" URLs and later just substitute 
			// whatever garbage path information UADE is adding (see mapUrl() above)
			
			// map URL to some valid FS path (must not contain "//", ":" or "?")
			// input e.g. "@mod_proxy.php?mod=Fasttracker/4-Mat/bonus.mod" or
			// "@ftp://foo.com/foo/bar/file.mod" (should avoid name clashes)
			
			filename= this.mapUri2Fs("@" + filename);	// treat all songs as "from outside"

			var f= ((overridePath)?overridePath:basePath) + filename;	// this._basePath ever needed?

			if (this.modlandMode) {
				f= decodeURI(f);	// e.g. see TFMX songs Huelsbeck with spaces				
				this.originalFile= f;
				
				// these files need the correct prefix for UADE (or any other player) to handle them
				if (f.indexOf("/TFMX ST/")>0) {
					f= f.replace("/mdat.", "/mdst.");
				}
				if (f.indexOf("/Zoundmonitor/")>0) {
					f= f.replace(".sng", ".zm");
					
					// modland idiots changed the layout and now put the sample files
					// into a separate "top" level folder..".. 
					f= f.replace("/../Samples/", "/Samples/");
				}			
			}
			return f;
		},
		getPathAndFilename: function(fullFilename) {
			// input is path+filename combined: base for "registerFileData" & "loadMusicData"
			
			if (fullFilename.substring(0, this.basePath.length) == this.basePath) {
				fullFilename= fullFilename.substring(this.basePath.length);
			}
			// since uade needs the basePath to *ALWAYS* point to the folder where the various config 
			// files can be found, the filename returned here is actually still a path 
			return [this.basePath, fullFilename];
		},
		mapCacheFileName: function (name) {
			// problem: in some folders, modland uses one "shared lib" file (e.g. "smp.set") that is 
			// accessed via different "alias names". when the next alias name is used the file 
			// will already be found in the cache => but for the song to work the respective file data
			// must be registered in the FS under the additional alias (was solved by adding "alias" 
			// support in the scriptnode player)
			
			return name;
		},
		mapModlandShit: function (input) {
			var input= decodeURI(input);	// replace escape sequences... 
			
			// modland uses wrong (lower) case for practically all sample file names.. damn jerks
			var output= input.replace(".adsc.AS", ".adsc.as");	// AudioSculpture
			output= output.replace("/SMP.", "/smp.");	// Dirk Bialluch, Dynamic Synthesizer, Jason Page, Magnetic Fields Packer, Quartet ST, Synth Dream, Thomas Hermann 
			output= output.replace(".SSD", ".ssd");		// Paul Robotham 
			output= output.replace(".INS", ".ins");	// Richard Joseph  
			output= output.replace("/mcS.", "/mcs.");	// Mark Cooksey Old  
			
			var o= this.originalFile.substr(this.originalFile.lastIndexOf("/")+1);
			var ot= output.substr(output.lastIndexOf("/")+1);

			if (this.originalFile.endsWith(".soc") && output.endsWith(".so")) {	// Hippel ST COSO 
				output= output.substr(0, output.lastIndexOf("/")) + "/smp.set";
			} else if (this.originalFile.endsWith(".pap") && output.endsWith(".pa")) { // Pierre Adane Packer 
				output= output.substr(0, output.lastIndexOf("/")) + "/smp.set";
			} else if (this.originalFile.endsWith(".osp") && output.endsWith(".os")) { // Synth Pack  
				output= output.substr(0, output.lastIndexOf("/")) + "/smp.set";
			} else if (o.startsWith("sdr.") && ot.startsWith("smp.")) { // Synth Dream  (always use the "set".. other songs don't seem to play properly anyway - even in uade123)
				output= output.substr(0, output.lastIndexOf("/")) + "/smp.set";
			} else if (this.originalFile.endsWith(".ymst") && output.endsWith("replay")) { // YMST 
				var idx= output.lastIndexOf("/");
				var fn= output.substr(idx).toLowerCase().replace(" ", "_");	// see inconsistent zout-game.ymst 
				output= output.substr(0, idx) + fn;
			} else if (ot.startsWith("smpl.mdst.")) { // TFMX_ST 
				output= output.replace("smpl.mdst.", "smpl.");
			} else if (output.startsWith("mdat.") && (output.indexOf("/TFMX ST/")>0)) {
				// these ST files need the correct prefix for UADE (or any other player) to handle them
				output= output.replace("/mdat.", "/mdst.");				
			} else if (output.endsWith(".sng") && (output.indexOf("/Zoundmonitor/")>0)) {
				// needed ext for UADE to handle them
				output= output.replace(".sng", ".zm");
			}  else if (this.originalFile.endsWith(".sng") && (output.indexOf("/Samples/")>0)) {
				// libs on modland are all lowercase				
				output= output.substr(0, output.lastIndexOf("/")) + output.substr(output.lastIndexOf("/")).toLowerCase();
			}
			// map:  actual file name -> "wrong" name used on emu side
			// e.g.  "smp.set"  ->      "dyter07 ongame01.os"
			if (input != output)	// remember the filename mapping (path is the same)
				this.modlandMap[output.replace(/^.*[\\\/]/, '')]= input.replace(/^.*[\\\/]/, '');	// needed to create FS expected by "amiga"
			
			return output;
		},		
		mapBackendFilename: function (name) {
			// triggered by UADE whenever it tries to load a file, the input "name" is
			// composed of UADE's basePath combined with the file that uade is looking for:
			// the "name" is what UADE later WILL USE for fopen (which is NOT affected by any mappings 
			// that might be done here.. i.e. in order to function the file must be installed in the FS
			// under exactly that path/name!)
			var input= this.Module.Pointer_stringify(name);
			
			if (input.indexOf("WantedTeam.bin") >= 0) {
				this.wantedOrig = input;							// e.g. uade/@songs/JO/WantedTeam.bin
				input= this.basePath + "players/WantedTeam.bin";	// always use local file since the file often seems to be missing
				
			} else if (input.indexOf("stonepacker.library") >= 0) {
				this.stoned = input;							// e.g. uade/@songs/JO/WantedTeam.bin
				input= this.basePath + "players/stonepacker.library";	// always use local file since the file often seems to be missing
				
				
			} else if (this.modlandMode) input= this.mapModlandShit(input);

			return input;
		},
		splitPath: function(url) {
			var arr= url.split("/");

			var path= "";
			for(var i= 0; i<arr.length-1; i++) {
				if (i>0)
					path += "/";
				path += arr[i];
			}
			var filename= arr[arr.length-1];
			return [path, filename];
		},
		registerFileData: function(pathFilenameArray, data) {
			// NOTE: UADE uses the "C fopen" based impl to access all files, i.e. the files 
			// MUST BE properly provided within the FS (the local cache within the player is nothing more than a 
			// convenience shortcut to detect if data is  already available - but irrelevant to UADE)
			
			if (pathFilenameArray[1].indexOf("WantedTeam.bin") >= 0) {	// make sure it "saves" under the original name that the backend expects
				if (typeof this.wantedOrig != 'undefined') pathFilenameArray = this.splitPath(this.wantedOrig);			
			}			
			if (pathFilenameArray[1].indexOf("stonepacker.library") >= 0) {	// make sure it "saves" under the original name that the backend expects
				if (typeof this.stoned != 'undefined') pathFilenameArray = this.splitPath(this.stoned);		// XXX		
			}			
			// input: the path is fixed to the basePath & the filename is actually still a path+filename
			var path= pathFilenameArray[0];
			var filename= pathFilenameArray[1];
			
			// MANDATORTY to move any path info still present in the "filename" to "path"
			var tmpPathFilenameArray = new Array(2);	// do not touch original IO param			
			var p= filename.lastIndexOf("/");
			if (p > 0) {
				tmpPathFilenameArray[0]= path + filename.substring(0, p);
				tmpPathFilenameArray[1]= filename.substring(p+1);
			} else  {
				tmpPathFilenameArray[0]= path;
				tmpPathFilenameArray[1]= filename;
			}

			if (this.modlandMode) {
				if (typeof this.modlandMap[tmpPathFilenameArray[1]] != 'undefined') {
					tmpPathFilenameArray[1]= this.modlandMap[tmpPathFilenameArray[1]];	// reverse map
					
					// e.g. saves the file under the name "dyter07 ongame01.os" (but
					// with the "URL-PATH". e.g. proxy.phpÿmod=Synth Pack/Karsten Obarski/Dyter-07)
				}
			}
			// setup data in our virtual FS (the next access should then be OK)
			return this.registerEmscriptenFileData(tmpPathFilenameArray, data);
		},
		loadMusicData: function(sampleRate, path, filename, data, options) {
			if (path.substr(-1) === "/") path= path.substring(0, path.length-1);
			
			var ret = this.Module.ccall('emu_init', 'number', 
								['number', 'string', 'string'], 
								[sampleRate, path, filename]);
				
			if (ret == 0) {
				// UADE's maximum sample rate is SOUNDTICKS_NTSC 3579545 which 
				// should never be a relevant limitation here..
				var inputSampleRate = sampleRate;
				this.resetSampleRate(sampleRate, inputSampleRate); 
			}
			return ret;
		},
		setPanning: function(value) {
			var pan= Math.min(Math.max(-1.0, value), 1.0);
			this.Module.ccall('emu_set_panning', 'number', ['number'], [pan]);
		},
		evalTrackOptions: function(options) {
			if ((typeof options.timeout != 'undefined') && (options.timeout >0)) {
				ScriptNodePlayer.getInstance().setPlaybackTimeout(options.timeout*1000);
			}			
			if ((typeof options.pan != 'undefined')) {
				this.setPanning(options.pan);
			}
			if ((typeof options.track === 'undefined')) {
				options.track= -1;	// use lowest or default (if available in format)
			}
			return this.Module.ccall('emu_set_subsong', 'number', ['number'], [options.track]);
		},
		teardown: function() {
//			this.Module.ccall('emu_teardown', 'number'); for some reason wasn't used in the old version
		},		
		
		initSongAttributes: function() {
			this.songInfo= new Object();
			this.songInfo.info1= "";
			this.songInfo.info2= "";
			this.songInfo.info3= "";;
			this.songInfo.mins= "";
			this.songInfo.maxs= "";
			this.songInfo.curs= "";			
			this.songInfo.infoText= "";			
		},
		getSongInfoMeta: function() {
			return {info1: String,
					info2: String,
					info3: String,
					mins: String,
					maxs: String,
					curs: String,
					infoText: String 
					};
		},
		updateSongInfo: function(filename, result) {
			if (typeof this.songInfo == 'undefined') this.initSongAttributes()
				
			result.info1= this.songInfo.info1;
			result.info2= this.songInfo.info2;
			result.info3= this.songInfo.info3;;			
			result.mins= this.songInfo.mins;
			result.maxs= this.songInfo.maxs;
			result.curs= this.songInfo.curs;
			result.infoText= this.songInfo.infoText;
		},
		getNumberTraceStreams: function() {
			return this.Module.ccall('emu_get_number_trace_streams', 'number');			
		},
		getTraceStreams: function() {
			var result= [];
			var n= this.getNumberTraceStreams();

			var ret = this.Module.ccall('emu_get_trace_streams', 'number');			
			var array = this.Module.HEAP32.subarray(ret>>2, (ret>>2)+n);
			
			for (var i= 0; i<n; i++) {
				result.push(array[i] >> 1);	// pointer to int16 array
			}
			return result;
		},
		enableScope: function(on) {
			// hack: always on
		},
		readFloatTrace: function(buffer, idx) {
			return (this.Module.HEAP16[buffer+idx])/0x8000;
		},
		
		// --------------------------- async file loading stuff -------------------------
		handleBackendSongAttributes: function(backendAttr, target) {
			this.initSongAttributes();
			// UADE "asynchronously" pushes a respective update..
			this.songInfo.info1= backendAttr.info1;
			this.songInfo.info2= backendAttr.info2;
			this.songInfo.info3= backendAttr.info3;;		
			this.songInfo.mins= backendAttr.minText;
			this.songInfo.maxs= backendAttr.maxText;
			this.songInfo.curs= backendAttr.currText;
			this.songInfo.infoText= backendAttr.infoText;
			
			this.updateSongInfo("", target);		
		},
		
	});	return $this; })();
