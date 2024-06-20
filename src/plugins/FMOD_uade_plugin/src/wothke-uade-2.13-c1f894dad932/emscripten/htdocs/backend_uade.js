/*
 backend_uade.js: UADE plugin for use in ScriptNodePlayer.

   version 1.2
   copyright (C) 2018-2023 Juergen Wothke


 Known limitation: seeking is not supported by UADE

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


// create separate namespace for all the Emscripten stuff.. otherwise naming clashes may occur especially when 
// optimizing using closure compiler..
window.spp_backend_state_UADE = {
	locateFile: function(path, scriptDirectory) { return (typeof window.WASM_SEARCH_PATH == 'undefined') ? path : window.WASM_SEARCH_PATH + path; },
	print: function(t) {
		// suppress annoying "source file" info
		setTimeout(console.log.bind(console, t));	// "lose" the original context
	},
	notReady: true,
	adapterCallback: function(){}	// overwritten later	
};
window.spp_backend_state_UADE["onRuntimeInitialized"] = function() {	// emscripten callback needed in case async init is used (e.g. for WASM)
	this.notReady= false;
	this.adapterCallback();
}.bind(window.spp_backend_state_UADE);

var backend_UADE = (function(Module) {var d;d||(d=typeof Module !== 'undefined' ? Module : {});function aa(a,b){return a.match("^"+b)==b}var ba={},k;for(k in d)d.hasOwnProperty(k)&&(ba[k]=d[k]);d.arguments=[];d.thisProgram="./this.program";d.quit=function(a,b){throw b;};d.preRun=[];d.postRun=[];var ca=!1,l=!1,m=!1,da=!1;ca="object"===typeof window;l="function"===typeof importScripts;m="object"===typeof process&&"function"===typeof require&&!ca&&!l;da=!ca&&!m&&!l;
if(d.ENVIRONMENT)throw Error("Module.ENVIRONMENT has been deprecated. To force the environment, use the ENVIRONMENT compile-time option (for example, -s ENVIRONMENT=web or -s ENVIRONMENT=node)");assert("undefined"===typeof d.memoryInitializerPrefixURL,"Module.memoryInitializerPrefixURL option was removed, use Module.locateFile instead");assert("undefined"===typeof d.pthreadMainPrefixURL,"Module.pthreadMainPrefixURL option was removed, use Module.locateFile instead");
assert("undefined"===typeof d.cdInitializerPrefixURL,"Module.cdInitializerPrefixURL option was removed, use Module.locateFile instead");assert("undefined"===typeof d.filePackagePrefixURL,"Module.filePackagePrefixURL option was removed, use Module.locateFile instead");var p="";function ea(a){return d.locateFile?d.locateFile(a,p):p+a}
if(m){p=__dirname+"/";var fa,ha;d.read=function(a,b){fa||(fa=require("fs"));ha||(ha=require("path"));a=ha.normalize(a);a=fa.readFileSync(a);return b?a:a.toString()};d.readBinary=function(a){a=d.read(a,!0);a.buffer||(a=new Uint8Array(a));assert(a.buffer);return a};1<process.argv.length&&(d.thisProgram=process.argv[1].replace(/\\/g,"/"));d.arguments=process.argv.slice(2);"undefined"!==typeof module&&(module.exports=d);process.on("uncaughtException",function(a){if(!(a instanceof ia))throw a;});process.on("unhandledRejection",
q);d.quit=function(a){process.exit(a)};d.inspect=function(){return"[Emscripten Module object]"}}else if(da)"undefined"!=typeof read&&(d.read=function(a){return read(a)}),d.readBinary=function(a){if("function"===typeof readbuffer)return new Uint8Array(readbuffer(a));a=read(a,"binary");assert("object"===typeof a);return a},"undefined"!=typeof scriptArgs?d.arguments=scriptArgs:"undefined"!=typeof arguments&&(d.arguments=arguments),"function"===typeof quit&&(d.quit=function(a){quit(a)});else if(ca||l)l?
p=self.location.href:document.currentScript&&(p=document.currentScript.src),p=0!==p.indexOf("blob:")?p.substr(0,p.lastIndexOf("/")+1):"",d.read=function(a){var b=new XMLHttpRequest;b.open("GET",a,!1);b.send(null);return b.responseText},l&&(d.readBinary=function(a){var b=new XMLHttpRequest;b.open("GET",a,!1);b.responseType="arraybuffer";b.send(null);return new Uint8Array(b.response)}),d.readAsync=function(a,b,c){var e=new XMLHttpRequest;e.open("GET",a,!0);e.responseType="arraybuffer";e.onload=function(){200==
e.status||0==e.status&&e.response?b(e.response):c()};e.onerror=c;e.send(null)},d.setWindowTitle=function(a){document.title=a};else throw Error("environment detection error");var ja=d.print||("undefined"!==typeof console?console.log.bind(console):"undefined"!==typeof print?print:null),r=d.printErr||("undefined"!==typeof printErr?printErr:"undefined"!==typeof console&&console.warn.bind(console)||ja);for(k in ba)ba.hasOwnProperty(k)&&(d[k]=ba[k]);ba=void 0;ka=la=ma=function(){q("cannot use the stack before compiled code is ready to run, and has provided stack access")};
function na(a){assert(!oa);var b=t;t=t+a+15&-16;assert(t<v,"not enough memory for static allocation - increase TOTAL_MEMORY");return b}function pa(a){assert(w);var b=y[w>>2];a=b+a+15&-16;y[w>>2]=a;if(a=a>=v)qa(),a=!0;return a?(y[w>>2]=b,0):b}function ra(a){var b;b||(b=16);return Math.ceil(a/b)*b}function sa(a){ta||(ta={});ta[a]||(ta[a]=1,r(a))}var ta,ua={"f64-rem":function(a,b){return a%b},"debugger":function(){debugger}},wa=!1;function assert(a,b){a||q("Assertion failed: "+b)}
var ya={stackSave:function(){ka()},stackRestore:function(){la()},arrayToC:function(a){var b=ma(a.length);assert(0<=a.length,"writeArrayToMemory array must have a length (should be an array or typed array)");A.set(a,b);return b},stringToC:function(a){var b=0;if(null!==a&&void 0!==a&&0!==a){var c=(a.length<<2)+1,e=b=ma(c);assert("number"==typeof c,"stringToUTF8(str, outPtr, maxBytesToWrite) is missing the third parameter that specifies the length of the output buffer!");xa(a,B,e,c)}return b}},za={string:ya.stringToC,
array:ya.arrayToC};function Aa(a,b){if("number"===typeof a){var c=!0;var e=a}else c=!1,e=a.length;b=4==b?f:["function"===typeof Ba?Ba:na,ma,na,pa][void 0===b?2:b](Math.max(e,1));if(c){var f=b;assert(0==(b&3));for(a=b+(e&-4);f<a;f+=4)y[f>>2]=0;for(a=b+e;f<a;)A[f++>>0]=0;return b}a.subarray||a.slice?B.set(a,b):B.set(new Uint8Array(a),b);return b}function Ca(a){return oa?C?Ba(a):pa(a):na(a)}
function D(a,b){if(0===b||!a)return"";for(var c=0,e,f=0;;){assert(a+f<v);e=B[a+f>>0];c|=e;if(0==e&&!b)break;f++;if(b&&f==b)break}b||(b=f);e="";if(128>c){for(;0<b;)c=String.fromCharCode.apply(String,B.subarray(a,a+Math.min(b,1024))),e=e?e+c:c,a+=1024,b-=1024;return e}return Da(a)}var Ea="undefined"!==typeof TextDecoder?new TextDecoder("utf8"):void 0;
function Fa(a,b){for(var c=b;a[c];)++c;if(16<c-b&&a.subarray&&Ea)return Ea.decode(a.subarray(b,c));for(c="";;){var e=a[b++];if(!e)return c;if(e&128){var f=a[b++]&63;if(192==(e&224))c+=String.fromCharCode((e&31)<<6|f);else{var g=a[b++]&63;if(224==(e&240))e=(e&15)<<12|f<<6|g;else{var h=a[b++]&63;if(240==(e&248))e=(e&7)<<18|f<<12|g<<6|h;else{var n=a[b++]&63;if(248==(e&252))e=(e&3)<<24|f<<18|g<<12|h<<6|n;else{var x=a[b++]&63;e=(e&1)<<30|f<<24|g<<18|h<<12|n<<6|x}}}65536>e?c+=String.fromCharCode(e):(e-=
65536,c+=String.fromCharCode(55296|e>>10,56320|e&1023))}}else c+=String.fromCharCode(e)}}function Da(a){return Fa(B,a)}
function xa(a,b,c,e){if(!(0<e))return 0;var f=c;e=c+e-1;for(var g=0;g<a.length;++g){var h=a.charCodeAt(g);if(55296<=h&&57343>=h){var n=a.charCodeAt(++g);h=65536+((h&1023)<<10)|n&1023}if(127>=h){if(c>=e)break;b[c++]=h}else{if(2047>=h){if(c+1>=e)break;b[c++]=192|h>>6}else{if(65535>=h){if(c+2>=e)break;b[c++]=224|h>>12}else{if(2097151>=h){if(c+3>=e)break;b[c++]=240|h>>18}else{if(67108863>=h){if(c+4>=e)break;b[c++]=248|h>>24}else{if(c+5>=e)break;b[c++]=252|h>>30;b[c++]=128|h>>24&63}b[c++]=128|h>>18&63}b[c++]=
128|h>>12&63}b[c++]=128|h>>6&63}b[c++]=128|h&63}}b[c]=0;return c-f}"undefined"!==typeof TextDecoder&&new TextDecoder("utf-16le");function Ga(a){return a.replace(/__Z[\w\d_]+/g,function(a){sa("warning: build with  -s DEMANGLE_SUPPORT=1  to link in libcxxabi demangling");return a===a?a:a+" ["+a+"]"})}
function Ha(){a:{var a=Error();if(!a.stack){try{throw Error(0);}catch(b){a=b}if(!a.stack){a="(no stack trace available)";break a}}a=a.stack.toString()}d.extraStackTrace&&(a+="\n"+d.extraStackTrace());return Ga(a)}var buffer,A,B,Ia,y,Ja,Ka,La;
function Ma(){d.HEAP8=A=new Int8Array(buffer);d.HEAP16=Ia=new Int16Array(buffer);d.HEAP32=y=new Int32Array(buffer);d.HEAPU8=B=new Uint8Array(buffer);d.HEAPU16=new Uint16Array(buffer);d.HEAPU32=Ja=new Uint32Array(buffer);d.HEAPF32=Ka=new Float32Array(buffer);d.HEAPF64=La=new Float64Array(buffer)}var Na,t,oa,Oa,Pa,F,Qa,w;Na=t=Oa=Pa=F=Qa=w=0;oa=!1;
function Ra(){34821223==Ja[(F>>2)-1]&&2310721022==Ja[(F>>2)-2]||q("Stack overflow! Stack cookie has been overwritten, expected hex dwords 0x89BACDFE and 0x02135467, but received 0x"+Ja[(F>>2)-2].toString(16)+" "+Ja[(F>>2)-1].toString(16));if(1668509029!==y[0])throw"Runtime error: The application has corrupted its heap memory area (address zero)!";}
function qa(){q("Cannot enlarge memory arrays. Either (1) compile with  -s TOTAL_MEMORY=X  with X higher than the current value "+v+", (2) compile with  -s ALLOW_MEMORY_GROWTH=1  which allows increasing the size at runtime, or (3) if you want malloc to return NULL (0) instead of this abort, compile with  -s ABORTING_MALLOC=0 ")}var Sa=d.TOTAL_STACK||5242880,v=d.TOTAL_MEMORY||33554432;v<Sa&&r("TOTAL_MEMORY should be larger than TOTAL_STACK, was "+v+"! (TOTAL_STACK="+Sa+")");
assert("undefined"!==typeof Int32Array&&"undefined"!==typeof Float64Array&&void 0!==Int32Array.prototype.subarray&&void 0!==Int32Array.prototype.set,"JS engine does not provide full typed array support");
d.buffer?(buffer=d.buffer,assert(buffer.byteLength===v,"provided buffer should be "+v+" bytes, but it is "+buffer.byteLength)):("object"===typeof WebAssembly&&"function"===typeof WebAssembly.Memory?(assert(0===v%65536),d.wasmMemory=new WebAssembly.Memory({initial:v/65536,maximum:v/65536}),buffer=d.wasmMemory.buffer):buffer=new ArrayBuffer(v),assert(buffer.byteLength===v),d.buffer=buffer);Ma();y[0]=1668509029;Ia[1]=25459;
if(115!==B[2]||99!==B[3])throw"Runtime error: expected the system to be little-endian!";function Ta(a){for(;0<a.length;){var b=a.shift();if("function"==typeof b)b();else{var c=b.oa;"number"===typeof c?void 0===b.X?d.dynCall_v(c):d.dynCall_vi(c,b.X):c(void 0===b.X?null:b.X)}}}var Ua=[],Va=[],Wa=[],Xa=[],Ya=[],C=!1,G=!1;function Za(){var a=d.preRun.shift();Ua.unshift(a)}assert(Math.imul&&Math.fround&&Math.clz32&&Math.trunc,"this is a legacy browser, build with LEGACY_VM_SUPPORT");
var $a=0,ab=null,bb=null,cb={};function db(a){for(var b=a;cb[a];)a=b+Math.random();return a}function eb(a){$a++;d.monitorRunDependencies&&d.monitorRunDependencies($a);a?(assert(!cb[a]),cb[a]=1,null===ab&&"undefined"!==typeof setInterval&&(ab=setInterval(function(){if(wa)clearInterval(ab),ab=null;else{var a=!1,c;for(c in cb)a||(a=!0,r("still waiting on run dependencies:")),r("dependency: "+c);a&&r("(end of list)")}},1E4))):r("warning: run dependency added without ID")}
function fb(a){$a--;d.monitorRunDependencies&&d.monitorRunDependencies($a);a?(assert(cb[a]),delete cb[a]):r("warning: run dependency removed without ID");0==$a&&(null!==ab&&(clearInterval(ab),ab=null),bb&&(a=bb,bb=null,a()))}d.preloadedImages={};d.preloadedAudios={};function gb(a){return String.prototype.startsWith?a.startsWith("data:application/octet-stream;base64,"):0===a.indexOf("data:application/octet-stream;base64,")}
(function(){function a(){try{if(d.wasmBinary)return new Uint8Array(d.wasmBinary);if(d.readBinary)return d.readBinary(f);throw"both async and sync fetching of the wasm failed";}catch(u){q(u)}}function b(){return d.wasmBinary||!ca&&!l||"function"!==typeof fetch?new Promise(function(b){b(a())}):fetch(f,{credentials:"same-origin"}).then(function(a){if(!a.ok)throw"failed to load wasm binary file at '"+f+"'";return a.arrayBuffer()}).catch(function(){return a()})}function c(a){function c(a){n=a.exports;
if(n.memory){a=n.memory;var b=d.buffer;a.byteLength<b.byteLength&&r("the new buffer in mergeMemory is smaller than the previous one. in native wasm, we should grow memory here");b=new Int8Array(b);(new Int8Array(a)).set(b);d.buffer=buffer=a;Ma()}d.asm=n;d.usingWasm=!0;fb("wasm-instantiate")}function e(a){assert(d===u,"the Module object should not be replaced during async compilation - perhaps the order of HTML elements is wrong?");u=null;c(a.instance)}function g(a){b().then(function(a){return WebAssembly.instantiate(a,
h)}).then(a,function(a){r("failed to asynchronously prepare wasm: "+a);q(a)})}if("object"!==typeof WebAssembly)return q("No WebAssembly support found. Build with -s WASM=0 to target JavaScript instead."),r("no native wasm support detected"),!1;if(!(d.wasmMemory instanceof WebAssembly.Memory))return r("no native wasm Memory in use"),!1;a.memory=d.wasmMemory;h.global={NaN:NaN,Infinity:Infinity};h["global.Math"]=Math;h.env=a;eb("wasm-instantiate");if(d.instantiateWasm)try{return d.instantiateWasm(h,
c)}catch(va){return r("Module.instantiateWasm callback failed with error: "+va),!1}var u=d;d.wasmBinary||"function"!==typeof WebAssembly.instantiateStreaming||gb(f)||"function"!==typeof fetch?g(e):WebAssembly.instantiateStreaming(fetch(f,{credentials:"same-origin"}),h).then(e,function(a){r("wasm streaming compile failed: "+a);r("falling back to ArrayBuffer instantiation");g(e)});return{}}var e="uade.wast",f="uade.wasm",g="uade.temp.asm.js";gb(e)||(e=ea(e));gb(f)||(f=ea(f));gb(g)||(g=ea(g));var h=
{global:null,env:null,asm2wasm:ua,parent:d},n=null;d.asmPreload=d.asm;var x=d.reallocBuffer;d.reallocBuffer=function(a){if("asmjs"===z)var b=x(a);else a:{var c=d.usingWasm?65536:16777216;0<a%c&&(a+=c-a%c);c=d.buffer.byteLength;if(d.usingWasm)try{b=-1!==d.wasmMemory.grow((a-c)/65536)?d.buffer=d.wasmMemory.buffer:null;break a}catch(E){console.error("Module.reallocBuffer: Attempted to grow from "+c+" bytes to "+a+" bytes, but got error: "+E);b=null;break a}b=void 0}return b};var z="";d.asm=function(a,
b){if(!b.table){a=d.wasmTableSize;void 0===a&&(a=1024);var e=d.wasmMaxTableSize;b.table="object"===typeof WebAssembly&&"function"===typeof WebAssembly.Table?void 0!==e?new WebAssembly.Table({initial:a,maximum:e,element:"anyfunc"}):new WebAssembly.Table({initial:a,element:"anyfunc"}):Array(a);d.wasmTable=b.table}b.memoryBase||(b.memoryBase=d.STATIC_BASE);b.tableBase||(b.tableBase=0);b=c(b);assert(b,"no binaryen method succeeded. consider enabling more options, like interpreting, if you want that: http://kripken.github.io/emscripten-site/docs/compiling/WebAssembly.html#binaryen-methods");
return b}})();Na=1024;t=Na+991952;Va.push({oa:function(){hb()}},{oa:function(){ib()}});d.STATIC_BASE=Na;d.STATIC_BUMP=991952;var jb=t;t+=16;assert(0==jb%8);var H={};
function kb(a){if(kb.A){var b=y[a>>2];var c=y[b>>2]}else kb.A=!0,H.USER=H.LOGNAME="web_user",H.PATH="/",H.PWD="/",H.HOME="/home/web_user",H.LANG="C.UTF-8",H._=d.thisProgram,c=Ca(1024),b=Ca(256),y[b>>2]=c,y[a>>2]=b;a=[];var e=0,f;for(f in H)if("string"===typeof H[f]){var g=f+"="+H[f];a.push(g);e+=g.length}if(1024<e)throw Error("Environment size exceeded TOTAL_ENV_SIZE!");for(f=0;f<a.length;f++){e=g=a[f];for(var h=c,n=0;n<e.length;++n)assert(e.charCodeAt(n)===e.charCodeAt(n)&255),A[h++>>0]=e.charCodeAt(n);
A[h>>0]=0;y[b+4*f>>2]=c;c+=g.length+1}y[b+4*a.length>>2]=0}
var I={F:1,u:2,Dc:3,zb:4,C:5,ga:6,Sa:7,Xb:8,v:9,gb:10,da:11,Nc:11,ya:12,S:13,sb:14,jc:15,T:16,ea:17,Oc:18,V:19,W:20,K:21,h:22,Sb:23,wa:24,D:25,Kc:26,tb:27,ec:28,O:29,Ac:30,Lb:31,tc:32,pb:33,xc:34,ac:42,wb:43,hb:44,Cb:45,Db:46,Eb:47,Kb:48,Lc:49,Vb:50,Bb:51,mb:35,Yb:37,Ya:52,ab:53,Pc:54,Tb:55,bb:56,cb:57,nb:35,eb:59,hc:60,Wb:61,Hc:62,fc:63,bc:64,cc:65,zc:66,Zb:67,Va:68,Ec:69,ib:70,uc:71,Nb:72,qb:73,$a:74,oc:76,Za:77,yc:78,Fb:79,Gb:80,Jb:81,Ib:82,Hb:83,ic:38,fa:39,Ob:36,U:40,pc:95,sc:96,lb:104,Ub:105,
Wa:97,wc:91,mc:88,dc:92,Bc:108,kb:111,Ta:98,jb:103,Rb:101,Pb:100,Ic:110,ub:112,vb:113,yb:115,Xa:114,ob:89,Mb:90,vc:93,Cc:94,Ua:99,Qb:102,Ab:106,kc:107,Jc:109,Mc:87,rb:122,Fc:116,nc:95,$b:123,xb:84,qc:75,fb:125,lc:131,rc:130,Gc:86},lb={0:"Success",1:"Not super-user",2:"No such file or directory",3:"No such process",4:"Interrupted system call",5:"I/O error",6:"No such device or address",7:"Arg list too long",8:"Exec format error",9:"Bad file number",10:"No children",11:"No more processes",12:"Not enough core",
13:"Permission denied",14:"Bad address",15:"Block device required",16:"Mount device busy",17:"File exists",18:"Cross-device link",19:"No such device",20:"Not a directory",21:"Is a directory",22:"Invalid argument",23:"Too many open files in system",24:"Too many open files",25:"Not a typewriter",26:"Text file busy",27:"File too large",28:"No space left on device",29:"Illegal seek",30:"Read only file system",31:"Too many links",32:"Broken pipe",33:"Math arg out of domain of func",34:"Math result not representable",
35:"File locking deadlock error",36:"File or path name too long",37:"No record locks available",38:"Function not implemented",39:"Directory not empty",40:"Too many symbolic links",42:"No message of desired type",43:"Identifier removed",44:"Channel number out of range",45:"Level 2 not synchronized",46:"Level 3 halted",47:"Level 3 reset",48:"Link number out of range",49:"Protocol driver not attached",50:"No CSI structure available",51:"Level 2 halted",52:"Invalid exchange",53:"Invalid request descriptor",
54:"Exchange full",55:"No anode",56:"Invalid request code",57:"Invalid slot",59:"Bad font file fmt",60:"Device not a stream",61:"No data (for no delay io)",62:"Timer expired",63:"Out of streams resources",64:"Machine is not on the network",65:"Package not installed",66:"The object is remote",67:"The link has been severed",68:"Advertise error",69:"Srmount error",70:"Communication error on send",71:"Protocol error",72:"Multihop attempted",73:"Cross mount point (not really error)",74:"Trying to read unreadable message",
75:"Value too large for defined data type",76:"Given log. name not unique",77:"f.d. invalid for this operation",78:"Remote address changed",79:"Can   access a needed shared lib",80:"Accessing a corrupted shared lib",81:".lib section in a.out corrupted",82:"Attempting to link in too many libs",83:"Attempting to exec a shared library",84:"Illegal byte sequence",86:"Streams pipe error",87:"Too many users",88:"Socket operation on non-socket",89:"Destination address required",90:"Message too long",91:"Protocol wrong type for socket",
92:"Protocol not available",93:"Unknown protocol",94:"Socket type not supported",95:"Not supported",96:"Protocol family not supported",97:"Address family not supported by protocol family",98:"Address already in use",99:"Address not available",100:"Network interface is not configured",101:"Network is unreachable",102:"Connection reset by network",103:"Connection aborted",104:"Connection reset by peer",105:"No buffer space available",106:"Socket is already connected",107:"Socket is not connected",108:"Can't send after socket shutdown",
109:"Too many references",110:"Connection timed out",111:"Connection refused",112:"Host is down",113:"Host is unreachable",114:"Socket already connected",115:"Connection already in progress",116:"Stale file handle",122:"Quota exceeded",123:"No medium (in tape drive)",125:"Operation canceled",130:"Previous owner died",131:"State not recoverable"};function mb(a){d.___errno_location?y[d.___errno_location()>>2]=a:r("failed to set errno from JS");return a}
function nb(a,b){for(var c=0,e=a.length-1;0<=e;e--){var f=a[e];"."===f?a.splice(e,1):".."===f?(a.splice(e,1),c++):c&&(a.splice(e,1),c--)}if(b)for(;c;c--)a.unshift("..");return a}function ob(a){var b="/"===a.charAt(0),c="/"===a.substr(-1);(a=nb(a.split("/").filter(function(a){return!!a}),!b).join("/"))||b||(a=".");a&&c&&(a+="/");return(b?"/":"")+a}
function pb(a){var b=/^(\/?|)([\s\S]*?)((?:\.{1,2}|[^\/]+?|)(\.[^.\/]*|))(?:[\/]*)$/.exec(a).slice(1);a=b[0];b=b[1];if(!a&&!b)return".";b&&(b=b.substr(0,b.length-1));return a+b}function qb(a){if("/"===a)return"/";var b=a.lastIndexOf("/");return-1===b?a:a.substr(b+1)}function rb(){var a=Array.prototype.slice.call(arguments,0);return ob(a.join("/"))}function J(a,b){return ob(a+"/"+b)}
function sb(){for(var a="",b=!1,c=arguments.length-1;-1<=c&&!b;c--){b=0<=c?arguments[c]:"/";if("string"!==typeof b)throw new TypeError("Arguments to path.resolve must be strings");if(!b)return"";a=b+"/"+a;b="/"===b.charAt(0)}a=nb(a.split("/").filter(function(a){return!!a}),!b).join("/");return(b?"/":"")+a||"."}var tb=[];function ub(a,b){tb[a]={input:[],output:[],J:b};vb(a,wb)}
var wb={open:function(a){var b=tb[a.node.rdev];if(!b)throw new K(I.V);a.tty=b;a.seekable=!1},close:function(a){a.tty.J.flush(a.tty)},flush:function(a){a.tty.J.flush(a.tty)},read:function(a,b,c,e){if(!a.tty||!a.tty.J.qa)throw new K(I.ga);for(var f=0,g=0;g<e;g++){try{var h=a.tty.J.qa(a.tty)}catch(n){throw new K(I.C);}if(void 0===h&&0===f)throw new K(I.da);if(null===h||void 0===h)break;f++;b[c+g]=h}f&&(a.node.timestamp=Date.now());return f},write:function(a,b,c,e){if(!a.tty||!a.tty.J.aa)throw new K(I.ga);
for(var f=0;f<e;f++)try{a.tty.J.aa(a.tty,b[c+f])}catch(g){throw new K(I.C);}e&&(a.node.timestamp=Date.now());return f}},yb={qa:function(a){if(!a.input.length){var b=null;if(m){var c=new Buffer(256),e=0,f=process.stdin.fd;if("win32"!=process.platform){var g=!1;try{f=fs.openSync("/dev/stdin","r"),g=!0}catch(h){}}try{e=fs.readSync(f,c,0,256,null)}catch(h){if(-1!=h.toString().indexOf("EOF"))e=0;else throw h;}g&&fs.closeSync(f);0<e?b=c.slice(0,e).toString("utf-8"):b=null}else"undefined"!=typeof window&&
"function"==typeof window.prompt?(b=window.prompt("Input: "),null!==b&&(b+="\n")):"function"==typeof readline&&(b=readline(),null!==b&&(b+="\n"));if(!b)return null;a.input=xb(b,!0)}return a.input.shift()},aa:function(a,b){null===b||10===b?(ja(Fa(a.output,0)),a.output=[]):0!=b&&a.output.push(b)},flush:function(a){a.output&&0<a.output.length&&(ja(Fa(a.output,0)),a.output=[])}},zb={aa:function(a,b){null===b||10===b?(r(Fa(a.output,0)),a.output=[]):0!=b&&a.output.push(b)},flush:function(a){a.output&&0<
a.output.length&&(r(Fa(a.output,0)),a.output=[])}},L={s:null,l:function(){return L.createNode(null,"/",16895,0)},createNode:function(a,b,c,e){if(24576===(c&61440)||4096===(c&61440))throw new K(I.F);L.s||(L.s={dir:{node:{o:L.c.o,i:L.c.i,lookup:L.c.lookup,L:L.c.L,rename:L.c.rename,unlink:L.c.unlink,rmdir:L.c.rmdir,readdir:L.c.readdir,symlink:L.c.symlink},stream:{B:L.f.B}},file:{node:{o:L.c.o,i:L.c.i},stream:{B:L.f.B,read:L.f.read,write:L.f.write,ha:L.f.ha,ta:L.f.ta,va:L.f.va}},link:{node:{o:L.c.o,i:L.c.i,
readlink:L.c.readlink},stream:{}},ka:{node:{o:L.c.o,i:L.c.i},stream:Ab}});c=Bb(a,b,c,e);M(c.mode)?(c.c=L.s.dir.node,c.f=L.s.dir.stream,c.b={}):32768===(c.mode&61440)?(c.c=L.s.file.node,c.f=L.s.file.stream,c.g=0,c.b=null):40960===(c.mode&61440)?(c.c=L.s.link.node,c.f=L.s.link.stream):8192===(c.mode&61440)&&(c.c=L.s.ka.node,c.f=L.s.ka.stream);c.timestamp=Date.now();a&&(a.b[b]=c);return c},Ea:function(a){if(a.b&&a.b.subarray){for(var b=[],c=0;c<a.g;++c)b.push(a.b[c]);return b}return a.b},Sc:function(a){return a.b?
a.b.subarray?a.b.subarray(0,a.g):new Uint8Array(a.b):new Uint8Array},la:function(a,b){a.b&&a.b.subarray&&b>a.b.length&&(a.b=L.Ea(a),a.g=a.b.length);if(!a.b||a.b.subarray){var c=a.b?a.b.length:0;c>=b||(b=Math.max(b,c*(1048576>c?2:1.125)|0),0!=c&&(b=Math.max(b,256)),c=a.b,a.b=new Uint8Array(b),0<a.g&&a.b.set(c.subarray(0,a.g),0))}else for(!a.b&&0<b&&(a.b=[]);a.b.length<b;)a.b.push(0)},Na:function(a,b){if(a.g!=b)if(0==b)a.b=null,a.g=0;else{if(!a.b||a.b.subarray){var c=a.b;a.b=new Uint8Array(new ArrayBuffer(b));
c&&a.b.set(c.subarray(0,Math.min(b,a.g)))}else if(a.b||(a.b=[]),a.b.length>b)a.b.length=b;else for(;a.b.length<b;)a.b.push(0);a.g=b}},c:{o:function(a){var b={};b.dev=8192===(a.mode&61440)?a.id:1;b.ino=a.id;b.mode=a.mode;b.nlink=1;b.uid=0;b.gid=0;b.rdev=a.rdev;M(a.mode)?b.size=4096:32768===(a.mode&61440)?b.size=a.g:40960===(a.mode&61440)?b.size=a.link.length:b.size=0;b.atime=new Date(a.timestamp);b.mtime=new Date(a.timestamp);b.ctime=new Date(a.timestamp);b.G=4096;b.blocks=Math.ceil(b.size/b.G);return b},
i:function(a,b){void 0!==b.mode&&(a.mode=b.mode);void 0!==b.timestamp&&(a.timestamp=b.timestamp);void 0!==b.size&&L.Na(a,b.size)},lookup:function(){throw Cb[I.u];},L:function(a,b,c,e){return L.createNode(a,b,c,e)},rename:function(a,b,c){if(M(a.mode)){try{var e=Db(b,c)}catch(g){}if(e)for(var f in e.b)throw new K(I.fa);}delete a.parent.b[a.name];a.name=c;b.b[c]=a;a.parent=b},unlink:function(a,b){delete a.b[b]},rmdir:function(a,b){var c=Db(a,b),e;for(e in c.b)throw new K(I.fa);delete a.b[b]},readdir:function(a){var b=
[".",".."],c;for(c in a.b)a.b.hasOwnProperty(c)&&b.push(c);return b},symlink:function(a,b,c){a=L.createNode(a,b,41471,0);a.link=c;return a},readlink:function(a){if(40960!==(a.mode&61440))throw new K(I.h);return a.link}},f:{read:function(a,b,c,e,f){var g=a.node.b;if(f>=a.node.g)return 0;a=Math.min(a.node.g-f,e);assert(0<=a);if(8<a&&g.subarray)b.set(g.subarray(f,f+a),c);else for(e=0;e<a;e++)b[c+e]=g[f+e];return a},write:function(a,b,c,e,f,g){if(!e)return 0;a=a.node;a.timestamp=Date.now();if(b.subarray&&
(!a.b||a.b.subarray)){if(g)return assert(0===f,"canOwn must imply no weird position inside the file"),a.b=b.subarray(c,c+e),a.g=e;if(0===a.g&&0===f)return a.b=new Uint8Array(b.subarray(c,c+e)),a.g=e;if(f+e<=a.g)return a.b.set(b.subarray(c,c+e),f),e}L.la(a,f+e);if(a.b.subarray&&b.subarray)a.b.set(b.subarray(c,c+e),f);else for(g=0;g<e;g++)a.b[f+g]=b[c+g];a.g=Math.max(a.g,f+e);return e},B:function(a,b,c){1===c?b+=a.position:2===c&&32768===(a.node.mode&61440)&&(b+=a.node.g);if(0>b)throw new K(I.h);return b},
ha:function(a,b,c){L.la(a.node,b+c);a.node.g=Math.max(a.node.g,b+c)},ta:function(a,b,c,e,f,g,h){if(32768!==(a.node.mode&61440))throw new K(I.V);c=a.node.b;if(h&2||c.buffer!==b&&c.buffer!==b.buffer){if(0<f||f+e<a.node.g)c.subarray?c=c.subarray(f,f+e):c=Array.prototype.slice.call(c,f,f+e);a=!0;e=Ba(e);if(!e)throw new K(I.ya);b.set(c,e)}else a=!1,e=c.byteOffset;return{Uc:e,Qc:a}},va:function(a,b,c,e,f){if(32768!==(a.node.mode&61440))throw new K(I.V);if(f&2)return 0;L.f.write(a,b,0,e,c,!1);return 0}}},
O={R:!1,Qa:function(){O.R=!!process.platform.match(/^win/);var a=process.binding("constants");a.fs&&(a=a.fs);O.ma={1024:a.O_APPEND,64:a.O_CREAT,128:a.O_EXCL,0:a.O_RDONLY,2:a.O_RDWR,4096:a.O_SYNC,512:a.O_TRUNC,1:a.O_WRONLY}},ia:function(a){return Buffer.A?Buffer.from(a):new Buffer(a)},l:function(a){assert(m);return O.createNode(null,"/",O.pa(a.$.root),0)},createNode:function(a,b,c){if(!M(c)&&32768!==(c&61440)&&40960!==(c&61440))throw new K(I.h);a=Bb(a,b,c);a.c=O.c;a.f=O.f;return a},pa:function(a){try{var b=
fs.lstatSync(a);O.R&&(b.mode=b.mode|(b.mode&292)>>2)}catch(c){if(!c.code)throw c;throw new K(I[c.code]);}return b.mode},m:function(a){for(var b=[];a.parent!==a;)b.push(a.name),a=a.parent;b.push(a.l.$.root);b.reverse();return rb.apply(null,b)},Da:function(a){a&=-2656257;var b=0,c;for(c in O.ma)a&c&&(b|=O.ma[c],a^=c);if(a)throw new K(I.h);return b},c:{o:function(a){a=O.m(a);try{var b=fs.lstatSync(a)}catch(c){if(!c.code)throw c;throw new K(I[c.code]);}O.R&&!b.G&&(b.G=4096);O.R&&!b.blocks&&(b.blocks=
(b.size+b.G-1)/b.G|0);return{dev:b.dev,ino:b.ino,mode:b.mode,nlink:b.nlink,uid:b.uid,gid:b.gid,rdev:b.rdev,size:b.size,atime:b.atime,mtime:b.mtime,ctime:b.ctime,G:b.G,blocks:b.blocks}},i:function(a,b){var c=O.m(a);try{void 0!==b.mode&&(fs.chmodSync(c,b.mode),a.mode=b.mode),void 0!==b.size&&fs.truncateSync(c,b.size)}catch(e){if(!e.code)throw e;throw new K(I[e.code]);}},lookup:function(a,b){var c=J(O.m(a),b);c=O.pa(c);return O.createNode(a,b,c)},L:function(a,b,c,e){a=O.createNode(a,b,c,e);b=O.m(a);
try{M(a.mode)?fs.mkdirSync(b,a.mode):fs.writeFileSync(b,"",{mode:a.mode})}catch(f){if(!f.code)throw f;throw new K(I[f.code]);}return a},rename:function(a,b,c){a=O.m(a);b=J(O.m(b),c);try{fs.renameSync(a,b)}catch(e){if(!e.code)throw e;throw new K(I[e.code]);}},unlink:function(a,b){a=J(O.m(a),b);try{fs.unlinkSync(a)}catch(c){if(!c.code)throw c;throw new K(I[c.code]);}},rmdir:function(a,b){a=J(O.m(a),b);try{fs.rmdirSync(a)}catch(c){if(!c.code)throw c;throw new K(I[c.code]);}},readdir:function(a){a=O.m(a);
try{return fs.readdirSync(a)}catch(b){if(!b.code)throw b;throw new K(I[b.code]);}},symlink:function(a,b,c){a=J(O.m(a),b);try{fs.symlinkSync(c,a)}catch(e){if(!e.code)throw e;throw new K(I[e.code]);}},readlink:function(a){var b=O.m(a);try{return b=fs.readlinkSync(b),b=Eb.relative(Eb.resolve(a.l.$.root),b)}catch(c){if(!c.code)throw c;throw new K(I[c.code]);}}},f:{open:function(a){var b=O.m(a.node);try{32768===(a.node.mode&61440)&&(a.N=fs.openSync(b,O.Da(a.flags)))}catch(c){if(!c.code)throw c;throw new K(I[c.code]);
}},close:function(a){try{32768===(a.node.mode&61440)&&a.N&&fs.closeSync(a.N)}catch(b){if(!b.code)throw b;throw new K(I[b.code]);}},read:function(a,b,c,e,f){if(0===e)return 0;try{return fs.readSync(a.N,O.ia(b.buffer),c,e,f)}catch(g){throw new K(I[g.code]);}},write:function(a,b,c,e,f){try{return fs.writeSync(a.N,O.ia(b.buffer),c,e,f)}catch(g){throw new K(I[g.code]);}},B:function(a,b,c){if(1===c)b+=a.position;else if(2===c&&32768===(a.node.mode&61440))try{b+=fs.fstatSync(a.N).size}catch(e){throw new K(I[e.code]);
}if(0>b)throw new K(I.h);return b}}};t+=16;t+=16;t+=16;var Fb=null,Gb={},Ib=[],Jb=1,P=null,Kb=!0,Q={},K=null,Cb={};
function R(a,b){a=sb("/",a);b=b||{};if(!a)return{path:"",node:null};var c={na:!0,ba:0},e;for(e in c)void 0===b[e]&&(b[e]=c[e]);if(8<b.ba)throw new K(I.U);a=nb(a.split("/").filter(function(a){return!!a}),!1);var f=Fb;c="/";for(e=0;e<a.length;e++){var g=e===a.length-1;if(g&&b.parent)break;f=Db(f,a[e]);c=J(c,a[e]);f.M&&(!g||g&&b.na)&&(f=f.M.root);if(!g||b.H)for(g=0;40960===(f.mode&61440);)if(f=Lb(c),c=sb(pb(c),f),f=R(c,{ba:b.ba}).node,40<g++)throw new K(I.U);}return{path:c,node:f}}
function S(a){for(var b;;){if(a===a.parent)return a=a.l.ua,b?"/"!==a[a.length-1]?a+"/"+b:a+b:a;b=b?a.name+"/"+b:a.name;a=a.parent}}function Mb(a,b){for(var c=0,e=0;e<b.length;e++)c=(c<<5)-c+b.charCodeAt(e)|0;return(a+c>>>0)%P.length}function Nb(a){var b=Mb(a.parent.id,a.name);a.I=P[b];P[b]=a}function Db(a,b){var c;if(c=(c=Ob(a,"x"))?c:a.c.lookup?0:I.S)throw new K(c,a);for(c=P[Mb(a.id,b)];c;c=c.I){var e=c.name;if(c.parent.id===a.id&&e===b)return c}return a.c.lookup(a,b)}
function Bb(a,b,c,e){Pb||(Pb=function(a,b,c,e){a||(a=this);this.parent=a;this.l=a.l;this.M=null;this.id=Jb++;this.name=b;this.mode=c;this.c={};this.f={};this.rdev=e},Pb.prototype={},Object.defineProperties(Pb.prototype,{read:{get:function(){return 365===(this.mode&365)},set:function(a){a?this.mode|=365:this.mode&=-366}},write:{get:function(){return 146===(this.mode&146)},set:function(a){a?this.mode|=146:this.mode&=-147}},Ha:{get:function(){return M(this.mode)}},Ga:{get:function(){return 8192===(this.mode&
61440)}}}));a=new Pb(a,b,c,e);Nb(a);return a}function M(a){return 16384===(a&61440)}var Qb={r:0,rs:1052672,"r+":2,w:577,wx:705,xw:705,"w+":578,"wx+":706,"xw+":706,a:1089,ax:1217,xa:1217,"a+":1090,"ax+":1218,"xa+":1218};function Rb(a){var b=["r","w","rw"][a&3];a&512&&(b+="w");return b}function Ob(a,b){if(Kb)return 0;if(-1===b.indexOf("r")||a.mode&292){if(-1!==b.indexOf("w")&&!(a.mode&146)||-1!==b.indexOf("x")&&!(a.mode&73))return I.S}else return I.S;return 0}
function Sb(a,b){try{return Db(a,b),I.ea}catch(c){}return Ob(a,"wx")}function Tb(a){var b=4096;for(a=a||0;a<=b;a++)if(!Ib[a])return a;throw new K(I.wa);}function Ub(a,b){Vb||(Vb=function(){},Vb.prototype={},Object.defineProperties(Vb.prototype,{object:{get:function(){return this.node},set:function(a){this.node=a}}}));var c=new Vb,e;for(e in a)c[e]=a[e];a=c;b=Tb(b);a.fd=b;return Ib[b]=a}var Ab={open:function(a){a.f=Gb[a.node.rdev].f;a.f.open&&a.f.open(a)},B:function(){throw new K(I.O);}};
function vb(a,b){Gb[a]={f:b}}function Wb(a,b){var c="/"===b,e=!b;if(c&&Fb)throw new K(I.T);if(!c&&!e){var f=R(b,{na:!1});b=f.path;f=f.node;if(f.M)throw new K(I.T);if(!M(f.mode))throw new K(I.W);}b={type:a,$:{},ua:b,Ja:[]};a=a.l(b);a.l=b;b.root=a;c?Fb=a:f&&(f.M=b,f.l&&f.l.Ja.push(b))}function Xb(a,b,c){var e=R(a,{parent:!0}).node;a=qb(a);if(!a||"."===a||".."===a)throw new K(I.h);var f=Sb(e,a);if(f)throw new K(f);if(!e.c.L)throw new K(I.F);return e.c.L(e,a,b,c)}
function T(a,b){return Xb(a,(void 0!==b?b:511)&1023|16384,0)}function Yb(a,b,c){"undefined"===typeof c&&(c=b,b=438);return Xb(a,b|8192,c)}function Zb(a,b){if(!sb(a))throw new K(I.u);var c=R(b,{parent:!0}).node;if(!c)throw new K(I.u);b=qb(b);var e=Sb(c,b);if(e)throw new K(e);if(!c.c.symlink)throw new K(I.F);return c.c.symlink(c,b,a)}
function $b(a){var b=R(a,{parent:!0}).node,c=qb(a),e=Db(b,c);a:{try{var f=Db(b,c)}catch(h){f=h.j;break a}var g=Ob(b,"wx");f=g?g:M(f.mode)?I.K:0}if(f)throw new K(f);if(!b.c.unlink)throw new K(I.F);if(e.M)throw new K(I.T);try{Q.willDeletePath&&Q.willDeletePath(a)}catch(h){console.log("FS.trackingDelegate['willDeletePath']('"+a+"') threw an exception: "+h.message)}b.c.unlink(b,c);b=Mb(e.parent.id,e.name);if(P[b]===e)P[b]=e.I;else for(b=P[b];b;){if(b.I===e){b.I=e.I;break}b=b.I}try{if(Q.onDeletePath)Q.onDeletePath(a)}catch(h){console.log("FS.trackingDelegate['onDeletePath']('"+
a+"') threw an exception: "+h.message)}}function Lb(a){a=R(a).node;if(!a)throw new K(I.u);if(!a.c.readlink)throw new K(I.h);return sb(S(a.parent),a.c.readlink(a))}function ac(a,b){var c;"string"===typeof a?c=R(a,{H:!0}).node:c=a;if(!c.c.i)throw new K(I.F);c.c.i(c,{mode:b&4095|c.mode&-4096,timestamp:Date.now()})}
function bc(a,b,c,e){if(""===a)throw new K(I.u);if("string"===typeof b){var f=Qb[b];if("undefined"===typeof f)throw Error("Unknown file open mode: "+b);b=f}c=b&64?("undefined"===typeof c?438:c)&4095|32768:0;if("object"===typeof a)var g=a;else{a=ob(a);try{g=R(a,{H:!(b&131072)}).node}catch(x){}}f=!1;if(b&64)if(g){if(b&128)throw new K(I.ea);}else g=Xb(a,c,0),f=!0;if(!g)throw new K(I.u);8192===(g.mode&61440)&&(b&=-513);if(b&65536&&!M(g.mode))throw new K(I.W);if(!f){var h=g?40960===(g.mode&61440)?I.U:
M(g.mode)&&("r"!==Rb(b)||b&512)?I.K:Ob(g,Rb(b)):I.u;if(h)throw new K(h);}if(b&512){c=g;var n;"string"===typeof c?n=R(c,{H:!0}).node:n=c;if(!n.c.i)throw new K(I.F);if(M(n.mode))throw new K(I.K);if(32768!==(n.mode&61440))throw new K(I.h);if(c=Ob(n,"w"))throw new K(c);n.c.i(n,{size:0,timestamp:Date.now()})}b&=-641;e=Ub({node:g,path:S(g),flags:b,seekable:!0,position:0,f:g.f,Ra:[],error:!1},e);e.f.open&&e.f.open(e);!d.logReadFiles||b&1||(cc||(cc={}),a in cc||(cc[a]=1,h("read file: "+a)));try{Q.onOpenFile&&
(h=0,1!==(b&2097155)&&(h|=1),0!==(b&2097155)&&(h|=2),Q.onOpenFile(a,h))}catch(x){console.log("FS.trackingDelegate['onOpenFile']('"+a+"', flags) threw an exception: "+x.message)}return e}function dc(a){if(null===a.fd)throw new K(I.v);a.Y&&(a.Y=null);try{a.f.close&&a.f.close(a)}catch(b){throw b;}finally{Ib[a.fd]=null}a.fd=null}function ec(a,b,c){if(null===a.fd)throw new K(I.v);if(!a.seekable||!a.f.B)throw new K(I.O);a.position=a.f.B(a,b,c);a.Ra=[]}
function fc(a,b,c,e,f,g){if(0>e||0>f)throw new K(I.h);if(null===a.fd)throw new K(I.v);if(0===(a.flags&2097155))throw new K(I.v);if(M(a.node.mode))throw new K(I.K);if(!a.f.write)throw new K(I.h);a.flags&1024&&ec(a,0,2);var h="undefined"!==typeof f;if(!h)f=a.position;else if(!a.seekable)throw new K(I.O);b=a.f.write(a,b,c,e,f,g);h||(a.position+=b);try{if(a.path&&Q.onWriteToFile)Q.onWriteToFile(a.path)}catch(n){console.log("FS.trackingDelegate['onWriteToFile']('"+path+"') threw an exception: "+n.message)}return b}
function hc(){K||(K=function(a,b){this.node=b;this.Pa=function(a){this.j=a;for(var b in I)if(I[b]===a){this.code=b;break}};this.Pa(a);this.message=lb[a];this.stack&&Object.defineProperty(this,"stack",{value:Error().stack,writable:!0});this.stack&&(this.stack=Ga(this.stack))},K.prototype=Error(),K.prototype.constructor=K,[I.u].forEach(function(a){Cb[a]=new K(a);Cb[a].stack="<generic error, no stack>"}))}var ic;function jc(a,b){var c=0;a&&(c|=365);b&&(c|=146);return c}
function kc(a,b,c,e){a=J("string"===typeof a?a:S(a),b);return T(a,jc(c,e))}function lc(a,b){a="string"===typeof a?a:S(a);for(b=b.split("/").reverse();b.length;){var c=b.pop();if(c){var e=J(a,c);try{T(e)}catch(f){}a=e}}return e}function mc(a,b,c,e){a=J("string"===typeof a?a:S(a),b);c=jc(c,e);return Xb(a,(void 0!==c?c:438)&4095|32768,0)}
function nc(a,b,c,e,f,g){a=b?J("string"===typeof a?a:S(a),b):a;e=jc(e,f);f=Xb(a,(void 0!==e?e:438)&4095|32768,0);if(c){if("string"===typeof c){a=Array(c.length);b=0;for(var h=c.length;b<h;++b)a[b]=c.charCodeAt(b);c=a}ac(f,e|146);a=bc(f,"w");fc(a,c,0,c.length,0,g);dc(a);ac(f,e)}return f}
function U(a,b,c,e){a=J("string"===typeof a?a:S(a),b);b=jc(!!c,!!e);U.sa||(U.sa=64);var f=U.sa++<<8|0;vb(f,{open:function(a){a.seekable=!1},close:function(){e&&e.buffer&&e.buffer.length&&e(10)},read:function(a,b,e,f){for(var g=0,h=0;h<f;h++){try{var n=c()}catch(N){throw new K(I.C);}if(void 0===n&&0===g)throw new K(I.da);if(null===n||void 0===n)break;g++;b[e+h]=n}g&&(a.node.timestamp=Date.now());return g},write:function(a,b,c,f){for(var g=0;g<f;g++)try{e(b[c+g])}catch(u){throw new K(I.C);}f&&(a.node.timestamp=
Date.now());return g}});return Yb(a,b,f)}function oc(a,b,c){a=J("string"===typeof a?a:S(a),b);return Zb(c,a)}
function pc(a){if(a.Ga||a.Ha||a.link||a.b)return!0;var b=!0;if("undefined"!==typeof XMLHttpRequest)throw Error("Lazy loading should have been performed (contents set) in createLazyFile, but it was not. Lazy loading only works in web workers. Use --embed-file or --preload-file in emcc on the main thread.");if(d.read)try{a.b=xb(d.read(a.url),!0),a.g=a.b.length}catch(c){b=!1}else throw Error("Cannot load without read() or XMLHttpRequest.");b||mb(I.C);return b}
function qc(a,b,c,e,f){function g(){this.Z=!1;this.P=[]}g.prototype.get=function(a){if(!(a>this.length-1||0>a)){var b=a%this.chunkSize;return this.ra(a/this.chunkSize|0)[b]}};g.prototype.Oa=function(a){this.ra=a};g.prototype.ja=function(){var a=new XMLHttpRequest;a.open("HEAD",c,!1);a.send(null);if(!(200<=a.status&&300>a.status||304===a.status))throw Error("Couldn't load "+c+". Status: "+a.status);var b=Number(a.getResponseHeader("Content-length")),e,f=(e=a.getResponseHeader("Accept-Ranges"))&&"bytes"===
e;a=(e=a.getResponseHeader("Content-Encoding"))&&"gzip"===e;var g=1048576;f||(g=b);var h=this;h.Oa(function(a){var e=a*g,f=(a+1)*g-1;f=Math.min(f,b-1);if("undefined"===typeof h.P[a]){var n=h.P;if(e>f)throw Error("invalid range ("+e+", "+f+") or no bytes requested!");if(f>b-1)throw Error("only "+b+" bytes available! programmer error!");var u=new XMLHttpRequest;u.open("GET",c,!1);b!==g&&u.setRequestHeader("Range","bytes="+e+"-"+f);"undefined"!=typeof Uint8Array&&(u.responseType="arraybuffer");u.overrideMimeType&&
u.overrideMimeType("text/plain; charset=x-user-defined");u.send(null);if(!(200<=u.status&&300>u.status||304===u.status))throw Error("Couldn't load "+c+". Status: "+u.status);e=void 0!==u.response?new Uint8Array(u.response||[]):xb(u.responseText||"",!0);n[a]=e}if("undefined"===typeof h.P[a])throw Error("doXHR failed!");return h.P[a]});if(a||!b)g=b=1,g=b=this.ra(0).length,console.log("LazyFiles on gzip forces download of the whole file when length is accessed");this.Aa=b;this.za=g;this.Z=!0};if("undefined"!==
typeof XMLHttpRequest){if(!l)throw"Cannot do synchronous binary XHRs outside webworkers in modern browsers. Use --embed-file or --preload-file in emcc";var h=new g;Object.defineProperties(h,{length:{get:function(){this.Z||this.ja();return this.Aa}},chunkSize:{get:function(){this.Z||this.ja();return this.za}}});var n=void 0}else n=c,h=void 0;var x=mc(a,b,e,f);h?x.b=h:n&&(x.b=null,x.url=n);Object.defineProperties(x,{g:{get:function(){return this.b.length}}});var z={};Object.keys(x.f).forEach(function(a){var b=
x.f[a];z[a]=function(){if(!pc(x))throw new K(I.C);return b.apply(null,arguments)}});z.read=function(a,b,c,e,f){if(!pc(x))throw new K(I.C);a=a.node.b;if(f>=a.length)return 0;e=Math.min(a.length-f,e);assert(0<=e);if(a.slice)for(var g=0;g<e;g++)b[c+g]=a[f+g];else for(g=0;g<e;g++)b[c+g]=a.get(f+g);return e};x.f=z;return x}
function rc(a,b,c,e,f,g,h,n,x,z){function u(c){function u(c){z&&z();n||nc(a,b,c,e,f,x);g&&g();fb(N)}var E=!1;d.preloadPlugins.forEach(function(a){!E&&a.canHandle(W)&&(a.handle(c,W,u,function(){h&&h();fb(N)}),E=!0)});E||u(c)}Browser.Tc();var W=b?sb(J(a,b)):a,N=db("cp "+W);eb(N);"string"==typeof c?Browser.Rc(c,function(a){u(a)},h):u(c)}var FS={},Pb,Vb,cc;
function sc(a,b){try{var c=R(a,{H:!0}).node;if(!c)throw new K(I.u);if(!c.c.o)throw new K(I.F);var e=c.c.o(c)}catch(f){if(f&&f.node&&ob(a)!==ob(S(f.node)))return-I.W;throw f;}y[b>>2]=e.dev;y[b+4>>2]=0;y[b+8>>2]=e.ino;y[b+12>>2]=e.mode;y[b+16>>2]=e.nlink;y[b+20>>2]=e.uid;y[b+24>>2]=e.gid;y[b+28>>2]=e.rdev;y[b+32>>2]=0;y[b+36>>2]=e.size;y[b+40>>2]=4096;y[b+44>>2]=e.blocks;y[b+48>>2]=e.atime.getTime()/1E3|0;y[b+52>>2]=0;y[b+56>>2]=e.mtime.getTime()/1E3|0;y[b+60>>2]=0;y[b+64>>2]=e.ctime.getTime()/1E3|
0;y[b+68>>2]=0;y[b+72>>2]=e.ino;return 0}var V=0;function X(){V+=4;return y[V-4>>2]}function tc(){var a=Ib[X()];if(!a)throw new K(I.v);return a}function uc(a){return Math.pow(2,a)}function vc(a){return Math.log(a)/Math.LN10}function wc(){wc.A||(wc.A=[]);wc.A.push(ka());return wc.A.length-1}var Y=t;t+=48;Aa(xb("GMT"),2);
function xc(){function a(a){return(a=a.toTimeString().match(/\(([A-Za-z ]+)\)$/))?a[1]:"GMT"}if(!yc){yc=!0;y[zc()>>2]=60*(new Date).getTimezoneOffset();var b=new Date(2E3,0,1),c=new Date(2E3,6,1);y[Ac()>>2]=Number(b.getTimezoneOffset()!=c.getTimezoneOffset());var e=a(b),f=a(c);e=Aa(xb(e),0);f=Aa(xb(f),0);c.getTimezoneOffset()<b.getTimezoneOffset()?(y[Bc()>>2]=e,y[Bc()+4>>2]=f):(y[Bc()>>2]=f,y[Bc()+4>>2]=e)}}var yc;hc();P=Array(4096);Wb(L,"/");T("/tmp");T("/home");T("/home/web_user");
(function(){T("/dev");vb(259,{read:function(){return 0},write:function(a,b,f,g){return g}});Yb("/dev/null",259);ub(1280,yb);ub(1536,zb);Yb("/dev/tty",1280);Yb("/dev/tty1",1536);if("undefined"!==typeof crypto){var a=new Uint8Array(1);var b=function(){crypto.getRandomValues(a);return a[0]}}else m?b=function(){return require("crypto").randomBytes(1)[0]}:b=function(){q("random_device")};U("/dev","random",b);U("/dev","urandom",b);T("/dev/shm");T("/dev/shm/tmp")})();T("/proc");T("/proc/self");T("/proc/self/fd");
Wb({l:function(){var a=Bb("/proc/self","fd",16895,73);a.c={lookup:function(a,c){var b=Ib[+c];if(!b)throw new K(I.v);a={parent:null,l:{ua:"fake"},c:{readlink:function(){return b.path}}};return a.parent=a}};return a}},"/proc/self/fd");
Va.unshift(function(){if(!d.noFSInit&&!ic){assert(!ic,"FS.init was previously called. If you want to initialize later with custom parameters, remove any earlier calls (note that one is automatically added to the generated code)");ic=!0;hc();d.stdin=d.stdin;d.stdout=d.stdout;d.stderr=d.stderr;d.stdin?U("/dev","stdin",d.stdin):Zb("/dev/tty","/dev/stdin");d.stdout?U("/dev","stdout",null,d.stdout):Zb("/dev/tty","/dev/stdout");d.stderr?U("/dev","stderr",null,d.stderr):Zb("/dev/tty1","/dev/stderr");var a=
bc("/dev/stdin","r");assert(0===a.fd,"invalid handle for stdin ("+a.fd+")");a=bc("/dev/stdout","w");assert(1===a.fd,"invalid handle for stdout ("+a.fd+")");a=bc("/dev/stderr","w");assert(2===a.fd,"invalid handle for stderr ("+a.fd+")")}});Wa.push(function(){Kb=!1});Xa.push(function(){ic=!1;var a=d._fflush;a&&a(0);for(a=0;a<Ib.length;a++){var b=Ib[a];b&&dc(b)}});d.FS_createFolder=kc;d.FS_createPath=lc;d.FS_createDataFile=nc;d.FS_createPreloadedFile=rc;d.FS_createLazyFile=qc;d.FS_createLink=oc;
d.FS_createDevice=U;d.FS_unlink=$b;Va.unshift(function(){});Xa.push(function(){});if(m){var fs=require("fs"),Eb=require("path");O.Qa()}w=na(4);Oa=Pa=ra(t);F=Oa+Sa;Qa=ra(F);y[w>>2]=Qa;oa=!0;assert(Qa<v,"TOTAL_MEMORY not big enough for stack");
function xb(a,b){for(var c=0,e=0;e<a.length;++e){var f=a.charCodeAt(e);55296<=f&&57343>=f&&(f=65536+((f&1023)<<10)|a.charCodeAt(++e)&1023);127>=f?++c:c=2047>=f?c+2:65535>=f?c+3:2097151>=f?c+4:67108863>=f?c+5:c+6}c=Array(c+1);a=xa(a,c,0,c.length);b&&(c.length=a);return c}d.wasmTableSize=4184;d.wasmMaxTableSize=4184;d.Ba={};
d.Ca={enlargeMemory:function(){qa()},getTotalMemory:function(){return v},abortOnCannotGrowMemory:qa,abortStackOverflow:function(a){q("Stack overflow! Attempted to allocate "+a+" bytes on the stack, but stack has only "+(F-ka()+a)+" bytes available!")},nullFunc_ii:function(a){r("Invalid function pointer called with signature 'ii'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");
r("Build with ASSERTIONS=2 for more info.");q(a)},nullFunc_iii:function(a){r("Invalid function pointer called with signature 'iii'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");r("Build with ASSERTIONS=2 for more info.");q(a)},nullFunc_iiii:function(a){r("Invalid function pointer called with signature 'iiii'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");
r("Build with ASSERTIONS=2 for more info.");q(a)},nullFunc_iiiii:function(a){r("Invalid function pointer called with signature 'iiiii'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");r("Build with ASSERTIONS=2 for more info.");q(a)},nullFunc_v:function(a){r("Invalid function pointer called with signature 'v'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");
r("Build with ASSERTIONS=2 for more info.");q(a)},nullFunc_vi:function(a){r("Invalid function pointer called with signature 'vi'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");r("Build with ASSERTIONS=2 for more info.");q(a)},nullFunc_vii:function(a){r("Invalid function pointer called with signature 'vii'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");
r("Build with ASSERTIONS=2 for more info.");q(a)},nullFunc_viiii:function(a){r("Invalid function pointer called with signature 'viiii'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");r("Build with ASSERTIONS=2 for more info.");q(a)},nullFunc_viiiii:function(a){r("Invalid function pointer called with signature 'viiiii'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");
r("Build with ASSERTIONS=2 for more info.");q(a)},nullFunc_viiiiii:function(a){r("Invalid function pointer called with signature 'viiiiii'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)");r("Build with ASSERTIONS=2 for more info.");q(a)},___assert_fail:function(a,
b,c,e){q("Assertion failed: "+D(a)+", at: "+[b?D(b):"unknown filename",c,e?D(e):"unknown function"])},___buildEnvironment:kb,___lock:function(){},___setErrNo:mb,___syscall114:function(a,b){V=b;try{q("cannot wait on child processes")}catch(c){return"undefined"!==typeof FS&&c instanceof K||q(c),-c.j}},___syscall140:function(a,b){V=b;try{var c=tc();X();var e=X(),f=X(),g=X();ec(c,e,g);y[f>>2]=c.position;c.Y&&0===e&&0===g&&(c.Y=null);return 0}catch(h){return"undefined"!==typeof FS&&h instanceof K||q(h),
-h.j}},___syscall145:function(a,b){V=b;try{var c=tc(),e=X();a:{var f=X();for(b=a=0;b<f;b++){var g=y[e+(8*b+4)>>2],h=c,n=y[e+8*b>>2],x=g,z=void 0,u=A;if(0>x||0>z)throw new K(I.h);if(null===h.fd)throw new K(I.v);if(1===(h.flags&2097155))throw new K(I.v);if(M(h.node.mode))throw new K(I.K);if(!h.f.read)throw new K(I.h);var W="undefined"!==typeof z;if(!W)z=h.position;else if(!h.seekable)throw new K(I.O);var N=h.f.read(h,u,n,x,z);W||(h.position+=N);var E=N;if(0>E){var Hb=-1;break a}a+=E;if(E<g)break}Hb=
a}return Hb}catch(va){return"undefined"!==typeof FS&&va instanceof K||q(va),-va.j}},___syscall146:function(a,b){V=b;try{var c=tc(),e=X();a:{var f=X();for(b=a=0;b<f;b++){var g=fc(c,A,y[e+8*b>>2],y[e+(8*b+4)>>2],void 0);if(0>g){var h=-1;break a}a+=g}h=a}return h}catch(n){return"undefined"!==typeof FS&&n instanceof K||q(n),-n.j}},___syscall195:function(a,b){V=b;try{var c=D(X()),e=X();return sc(c,e)}catch(f){return"undefined"!==typeof FS&&f instanceof K||q(f),-f.j}},___syscall197:function(a,b){V=b;try{var c=
tc(),e=X();return sc(c.path,e)}catch(f){return"undefined"!==typeof FS&&f instanceof K||q(f),-f.j}},___syscall221:function(a,b){V=b;try{var c=tc();switch(X()){case 0:var e=X();return 0>e?-I.h:bc(c.path,c.flags,0,e).fd;case 1:case 2:return 0;case 3:return c.flags;case 4:return e=X(),c.flags|=e,0;case 12:case 12:return e=X(),Ia[e+0>>1]=2,0;case 13:case 14:case 13:case 14:return 0;case 16:case 8:return-I.h;case 9:return mb(I.h),-1;default:return-I.h}}catch(f){return"undefined"!==typeof FS&&f instanceof
K||q(f),-f.j}},___syscall5:function(a,b){V=b;try{var c=D(X()),e=X(),f=X();return bc(c,e,f).fd}catch(g){return"undefined"!==typeof FS&&g instanceof K||q(g),-g.j}},___syscall54:function(a,b){V=b;try{var c=tc(),e=X();switch(e){case 21509:case 21505:return c.tty?0:-I.D;case 21510:case 21511:case 21512:case 21506:case 21507:case 21508:return c.tty?0:-I.D;case 21519:if(!c.tty)return-I.D;var f=X();return y[f>>2]=0;case 21520:return c.tty?-I.h:-I.D;case 21531:a=f=X();if(!c.f.Fa)throw new K(I.D);return c.f.Fa(c,
e,a);case 21523:return c.tty?0:-I.D;case 21524:return c.tty?0:-I.D;default:q("bad ioctl syscall "+e)}}catch(g){return"undefined"!==typeof FS&&g instanceof K||q(g),-g.j}},___syscall6:function(a,b){V=b;try{var c=tc();dc(c);return 0}catch(e){return"undefined"!==typeof FS&&e instanceof K||q(e),-e.j}},___unlock:function(){},_abort:function(){d.abort()},_ems_cache_file:function(a,b,c){b=B.subarray(b,b+c);window.ScriptNodePlayer.getInstance().setFileData(D(a),b)},_ems_notify_format_update:function(a){var b=
{};b.format=D(a).replace("type: ","");return window.ScriptNodePlayer.getInstance()._songUpdateCallback(b)},_ems_notify_player_update:function(a){var b={};b.player=D(a);return window.ScriptNodePlayer.getInstance()._songUpdateCallback(b)},_ems_notify_song_update:function(a,b,c,e){b=D(b);c=D(c);e=D(e);a:for(var f=d,g=[];;){var h=f.getValue(a++,"i8",!0);if(!h)break a;g.push(h&255)}f=g;a=[];g="";for(h=0;h<f.length;h++){var n=f[h];13==n||10==n?g.length&&(a.push(g),g=""):g+=window.String.fromCharCode(n)}f=
"";for(h=0;h<a.length;h++)f+=a[h]+"<br>";g={};var x=n=null;for(h=0;h<a.length;h++){var z=a[h];if(aa(z,"Music:")){var u=g.authorname?g.authorname:[];u.unshift(z.substring(6).trim());g.authorname=u}else if(aa(z,"DeliCustom:"))u=g.specialinfo?g.specialinfo:[],u.unshift(z.substring(11).trim()),g.specialinfo=u;else if(aa(z,"File name:"))u=g.title?g.title:[],u.unshift(z.substring(10).trim()),g.title=u;else if(aa(z,"Song title:"))u=g.title?g.title:[],u.unshift(z.substring(11).trim()),g.title=u;else if(aa(z,
"File prefix:"))g.prefix=z.substring(12).trim();else{a:{var W=z,N="MODULENAME: AUTHORNAME: SPECIALINFO: VERSION: CREDITS: Remarks:".split(" ");for(u=0;u<N.length;u++){var E=N[u];if(W.match("^"+E)==E){u=E;break a}}u=null}u?(n&&x&&(g[n]=x,x=null),n=u.substring(0,u.length-1).toLowerCase()):(z=z.trim(),z.length&&n&&(x||(x=[]),x.push(z)))}}n&&x&&(g[n]=x);a=[];!g.title||"modulename"in g||a.push(g.title.shift()+" ("+g.prefix+")");for(;3>=a.length;)if("modulename"in g)a.push(g.modulename.shift()),delete g.modulename;
else if("authorname"in g)a.push(g.authorname.shift()),delete g.authorname;else if("specialinfo"in g)a.push(g.specialinfo.shift()),delete g.specialinfo;else if("version"in g)a.push(g.version.shift()),0==g.version.length&&delete g.version;else if("credits"in g)a.push(g.credits.shift()),0==g.credits.length&&delete g.credits;else if("remarks"in g)a.push(g.remarks.shift()),0==g.remarks.length&&delete g.remarks;else break;g=1<a.length?a[1]:"";h=2<a.length?a[2]:"";n={};n.info1=0<a.length?a[0]:"";n.info2=
g;n.info3=h;n.mins=b;n.maxs=c;n.curs=e;n.infoText=f;return window.ScriptNodePlayer.getInstance()._songUpdateCallback(n)},_ems_request_file:function(a){var b=window.ScriptNodePlayer.getInstance();if(b.isReady())return b._fileRequestCallback(a);window.console.log("error: ems_request_file not ready")},_ems_request_file_size:function(a){return window.ScriptNodePlayer.getInstance()._fileSizeRequestCallback(a)},_emscripten_memcpy_big:function(a,b,c){B.set(B.subarray(b,b+c),a);return a},_emscripten_run_script:function(a){eval(D(a))},
_exit:function(a){Cc();if(d.noExitRuntime)r("exit("+a+") called, but EXIT_RUNTIME is not set, so halting execution but not exiting the runtime or preventing further async execution (build with EXIT_RUNTIME=1, if you want a true shutdown)");else if(wa=!0,Pa=Dc,Ra(),Ta(Xa),G=!0,d.onExit)d.onExit(a);d.quit(a,new ia(a))},_llvm_exp2_f64:function(){return uc.apply(null,arguments)},_llvm_log10_f64:function(){return vc.apply(null,arguments)},_llvm_stackrestore:function(a){var b=wc.A[a];wc.A.splice(a,1);la(b)},
_llvm_stacksave:wc,_localtime:function(a){xc();a=new Date(1E3*y[a>>2]);y[Y>>2]=a.getSeconds();y[Y+4>>2]=a.getMinutes();y[Y+8>>2]=a.getHours();y[Y+12>>2]=a.getDate();y[Y+16>>2]=a.getMonth();y[Y+20>>2]=a.getFullYear()-1900;y[Y+24>>2]=a.getDay();var b=new Date(a.getFullYear(),0,1);y[Y+28>>2]=(a.getTime()-b.getTime())/864E5|0;y[Y+36>>2]=-(60*a.getTimezoneOffset());var c=(new Date(2E3,6,1)).getTimezoneOffset();b=b.getTimezoneOffset();a=(c!=b&&a.getTimezoneOffset()==Math.min(b,c))|0;y[Y+32>>2]=a;a=y[Bc()+
(a?4:0)>>2];y[Y+40>>2]=a;return Y},DYNAMICTOP_PTR:w,STACKTOP:Pa,STACK_MAX:F};var Z=d.asm(d.Ba,d.Ca,buffer),Ec=Z.__GLOBAL__sub_I_Adapter_cpp;Z.__GLOBAL__sub_I_Adapter_cpp=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Ec.apply(null,arguments)};var Fc=Z.___cxa_can_catch;
Z.___cxa_can_catch=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Fc.apply(null,arguments)};var Gc=Z.___cxa_is_pointer_type;
Z.___cxa_is_pointer_type=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Gc.apply(null,arguments)};var Hc=Z.___emscripten_environ_constructor;
Z.___emscripten_environ_constructor=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Hc.apply(null,arguments)};var Ic=Z.___errno_location;
Z.___errno_location=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Ic.apply(null,arguments)};var Jc=Z.__get_daylight;
Z.__get_daylight=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Jc.apply(null,arguments)};var Kc=Z.__get_timezone;
Z.__get_timezone=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Kc.apply(null,arguments)};var Lc=Z.__get_tzname;
Z.__get_tzname=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Lc.apply(null,arguments)};var Mc=Z._emu_compute_audio_samples;
Z._emu_compute_audio_samples=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Mc.apply(null,arguments)};var Nc=Z._emu_get_audio_buffer;
Z._emu_get_audio_buffer=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Nc.apply(null,arguments)};var Oc=Z._emu_get_audio_buffer_length;
Z._emu_get_audio_buffer_length=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Oc.apply(null,arguments)};var Pc=Z._emu_get_trace_streams;
Z._emu_get_trace_streams=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Pc.apply(null,arguments)};var Qc=Z._emu_load_file;
Z._emu_load_file=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Qc.apply(null,arguments)};var Rc=Z._emu_number_trace_streams;
Z._emu_number_trace_streams=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Rc.apply(null,arguments)};var Sc=Z._emu_prepare;
Z._emu_prepare=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Sc.apply(null,arguments)};var Tc=Z._emu_set_panning;
Z._emu_set_panning=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Tc.apply(null,arguments)};var Uc=Z._emu_set_subsong;
Z._emu_set_subsong=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Uc.apply(null,arguments)};var Vc=Z._emu_teardown;
Z._emu_teardown=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Vc.apply(null,arguments)};var Wc=Z._fflush;Z._fflush=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Wc.apply(null,arguments)};
var Xc=Z._free;Z._free=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Xc.apply(null,arguments)};var Yc=Z._llvm_bswap_i16;
Z._llvm_bswap_i16=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Yc.apply(null,arguments)};var Zc=Z._llvm_bswap_i32;
Z._llvm_bswap_i32=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return Zc.apply(null,arguments)};var $c=Z._malloc;Z._malloc=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return $c.apply(null,arguments)};
var ad=Z._putchar;Z._putchar=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return ad.apply(null,arguments)};var bd=Z._sbrk;
Z._sbrk=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return bd.apply(null,arguments)};var cd=Z.establishStackSpace;
Z.establishStackSpace=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return cd.apply(null,arguments)};var dd=Z.getTempRet0;
Z.getTempRet0=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return dd.apply(null,arguments)};var ed=Z.setTempRet0;Z.setTempRet0=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return ed.apply(null,arguments)};
var fd=Z.setThrew;Z.setThrew=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return fd.apply(null,arguments)};var gd=Z.stackAlloc;
Z.stackAlloc=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return gd.apply(null,arguments)};var hd=Z.stackRestore;Z.stackRestore=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return hd.apply(null,arguments)};
var id=Z.stackSave;Z.stackSave=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return id.apply(null,arguments)};d.asm=Z;
var hb=d.__GLOBAL__sub_I_Adapter_cpp=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.__GLOBAL__sub_I_Adapter_cpp.apply(null,arguments)};
d.___cxa_can_catch=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.___cxa_can_catch.apply(null,arguments)};
d.___cxa_is_pointer_type=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.___cxa_is_pointer_type.apply(null,arguments)};
var ib=d.___emscripten_environ_constructor=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.___emscripten_environ_constructor.apply(null,arguments)};
d.___errno_location=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.___errno_location.apply(null,arguments)};
var Ac=d.__get_daylight=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.__get_daylight.apply(null,arguments)},zc=d.__get_timezone=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.__get_timezone.apply(null,
arguments)},Bc=d.__get_tzname=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.__get_tzname.apply(null,arguments)};
d._emu_compute_audio_samples=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._emu_compute_audio_samples.apply(null,arguments)};
d._emu_get_audio_buffer=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._emu_get_audio_buffer.apply(null,arguments)};
d._emu_get_audio_buffer_length=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._emu_get_audio_buffer_length.apply(null,arguments)};
d._emu_get_trace_streams=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._emu_get_trace_streams.apply(null,arguments)};
d._emu_load_file=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._emu_load_file.apply(null,arguments)};
d._emu_number_trace_streams=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._emu_number_trace_streams.apply(null,arguments)};
d._emu_prepare=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._emu_prepare.apply(null,arguments)};
d._emu_set_panning=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._emu_set_panning.apply(null,arguments)};
d._emu_set_subsong=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._emu_set_subsong.apply(null,arguments)};
d._emu_teardown=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._emu_teardown.apply(null,arguments)};
d._fflush=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._fflush.apply(null,arguments)};d._free=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._free.apply(null,arguments)};
d._llvm_bswap_i16=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._llvm_bswap_i16.apply(null,arguments)};
d._llvm_bswap_i32=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._llvm_bswap_i32.apply(null,arguments)};
var Ba=d._malloc=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._malloc.apply(null,arguments)};
d._putchar=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._putchar.apply(null,arguments)};d._sbrk=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm._sbrk.apply(null,arguments)};
d.establishStackSpace=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.establishStackSpace.apply(null,arguments)};
d.getTempRet0=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.getTempRet0.apply(null,arguments)};
d.setTempRet0=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.setTempRet0.apply(null,arguments)};
d.setThrew=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.setThrew.apply(null,arguments)};
var ma=d.stackAlloc=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.stackAlloc.apply(null,arguments)},la=d.stackRestore=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.stackRestore.apply(null,
arguments)},ka=d.stackSave=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.stackSave.apply(null,arguments)};
d.dynCall_v=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.dynCall_v.apply(null,arguments)};
d.dynCall_vi=function(){assert(C,"you need to wait for the runtime to be ready (e.g. wait for main() to be called)");assert(!G,"the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");return d.asm.dynCall_vi.apply(null,arguments)};d.asm=Z;d.intArrayFromString||(d.intArrayFromString=function(){q("'intArrayFromString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.intArrayToString||(d.intArrayToString=function(){q("'intArrayToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});
d.ccall=function(a,b,c,e){var f=d["_"+a];assert(f,"Cannot call unknown function "+a+", make sure it is exported");var g=[];a=0;assert("array"!==b,'Return type should not be "array".');if(e)for(var h=0;h<e.length;h++){var n=za[c[h]];n?(0===a&&(a=ka()),g[h]=n(e[h])):g[h]=e[h]}c=f.apply(null,g);c="string"===b?D(c):"boolean"===b?!!c:c;0!==a&&la(a);return c};d.cwrap||(d.cwrap=function(){q("'cwrap' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.setValue||(d.setValue=function(){q("'setValue' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});
d.getValue=function(a,b){b=b||"i8";"*"===b.charAt(b.length-1)&&(b="i32");switch(b){case "i1":return A[a>>0];case "i8":return A[a>>0];case "i16":return Ia[a>>1];case "i32":return y[a>>2];case "i64":return y[a>>2];case "float":return Ka[a>>2];case "double":return La[a>>3];default:q("invalid type for getValue: "+b)}return null};d.allocate||(d.allocate=function(){q("'allocate' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.getMemory=Ca;d.Pointer_stringify=D;
d.AsciiToString||(d.AsciiToString=function(){q("'AsciiToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.stringToAscii||(d.stringToAscii=function(){q("'stringToAscii' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.UTF8ArrayToString||(d.UTF8ArrayToString=function(){q("'UTF8ArrayToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.UTF8ToString=Da;d.stringToUTF8Array||(d.stringToUTF8Array=function(){q("'stringToUTF8Array' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});
d.stringToUTF8||(d.stringToUTF8=function(){q("'stringToUTF8' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.lengthBytesUTF8||(d.lengthBytesUTF8=function(){q("'lengthBytesUTF8' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.UTF16ToString||(d.UTF16ToString=function(){q("'UTF16ToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.stringToUTF16||(d.stringToUTF16=function(){q("'stringToUTF16' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});
d.lengthBytesUTF16||(d.lengthBytesUTF16=function(){q("'lengthBytesUTF16' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.UTF32ToString||(d.UTF32ToString=function(){q("'UTF32ToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.stringToUTF32||(d.stringToUTF32=function(){q("'stringToUTF32' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.lengthBytesUTF32||(d.lengthBytesUTF32=function(){q("'lengthBytesUTF32' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});
d.allocateUTF8||(d.allocateUTF8=function(){q("'allocateUTF8' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.stackTrace||(d.stackTrace=function(){q("'stackTrace' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.addOnPreRun||(d.addOnPreRun=function(){q("'addOnPreRun' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.addOnInit||(d.addOnInit=function(){q("'addOnInit' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});
d.addOnPreMain||(d.addOnPreMain=function(){q("'addOnPreMain' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.addOnExit||(d.addOnExit=function(){q("'addOnExit' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.addOnPostRun||(d.addOnPostRun=function(){q("'addOnPostRun' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.writeStringToMemory||(d.writeStringToMemory=function(){q("'writeStringToMemory' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});
d.writeArrayToMemory||(d.writeArrayToMemory=function(){q("'writeArrayToMemory' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.writeAsciiToMemory||(d.writeAsciiToMemory=function(){q("'writeAsciiToMemory' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.addRunDependency=eb;d.removeRunDependency=fb;d.ENV||(d.ENV=function(){q("'ENV' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.FS||(d.FS=function(){q("'FS' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});
d.FS_createFolder=kc;d.FS_createPath=lc;d.FS_createDataFile=nc;d.FS_createPreloadedFile=rc;d.FS_createLazyFile=qc;d.FS_createLink=oc;d.FS_createDevice=U;d.FS_unlink=$b;d.GL||(d.GL=function(){q("'GL' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.staticAlloc||(d.staticAlloc=function(){q("'staticAlloc' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.dynamicAlloc||(d.dynamicAlloc=function(){q("'dynamicAlloc' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});
d.warnOnce||(d.warnOnce=function(){q("'warnOnce' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.loadDynamicLibrary||(d.loadDynamicLibrary=function(){q("'loadDynamicLibrary' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.loadWebAssemblyModule||(d.loadWebAssemblyModule=function(){q("'loadWebAssemblyModule' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.getLEB||(d.getLEB=function(){q("'getLEB' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});
d.getFunctionTables||(d.getFunctionTables=function(){q("'getFunctionTables' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.alignFunctionTables||(d.alignFunctionTables=function(){q("'alignFunctionTables' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.registerFunctions||(d.registerFunctions=function(){q("'registerFunctions' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.addFunction||(d.addFunction=function(){q("'addFunction' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});
d.removeFunction||(d.removeFunction=function(){q("'removeFunction' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.getFuncWrapper||(d.getFuncWrapper=function(){q("'getFuncWrapper' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.prettyPrint||(d.prettyPrint=function(){q("'prettyPrint' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.makeBigInt||(d.makeBigInt=function(){q("'makeBigInt' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});
d.dynCall||(d.dynCall=function(){q("'dynCall' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.getCompilerSetting||(d.getCompilerSetting=function(){q("'getCompilerSetting' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.stackSave||(d.stackSave=function(){q("'stackSave' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.stackRestore||(d.stackRestore=function(){q("'stackRestore' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});
d.stackAlloc||(d.stackAlloc=function(){q("'stackAlloc' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.establishStackSpace||(d.establishStackSpace=function(){q("'establishStackSpace' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.print||(d.print=function(){q("'print' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});d.printErr||(d.printErr=function(){q("'printErr' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")});
d.ALLOC_NORMAL||Object.defineProperty(d,"ALLOC_NORMAL",{get:function(){q("'ALLOC_NORMAL' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")}});d.ALLOC_STACK||Object.defineProperty(d,"ALLOC_STACK",{get:function(){q("'ALLOC_STACK' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")}});d.ALLOC_STATIC||Object.defineProperty(d,"ALLOC_STATIC",{get:function(){q("'ALLOC_STATIC' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")}});
d.ALLOC_DYNAMIC||Object.defineProperty(d,"ALLOC_DYNAMIC",{get:function(){q("'ALLOC_DYNAMIC' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")}});d.ALLOC_NONE||Object.defineProperty(d,"ALLOC_NONE",{get:function(){q("'ALLOC_NONE' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")}});function ia(a){this.name="ExitStatus";this.message="Program terminated with exit("+a+")";this.status=a}ia.prototype=Error();ia.prototype.constructor=ia;var Dc;
bb=function jd(){d.calledRun||kd();d.calledRun||(bb=jd)};
function kd(){function a(){if(!d.calledRun&&(d.calledRun=!0,!wa)){Ra();C||(C=!0,Ta(Va));Ra();Ta(Wa);if(d.onRuntimeInitialized)d.onRuntimeInitialized();assert(!d._main,'compiled without a main, but one is present. if you added it from JS, use Module["onRuntimeInitialized"]');Ra();if(d.postRun)for("function"==typeof d.postRun&&(d.postRun=[d.postRun]);d.postRun.length;){var a=d.postRun.shift();Ya.unshift(a)}Ta(Ya)}}if(!(0<$a)){assert(0==(F&3));Ja[(F>>2)-1]=34821223;Ja[(F>>2)-2]=2310721022;if(d.preRun)for("function"==
typeof d.preRun&&(d.preRun=[d.preRun]);d.preRun.length;)Za();Ta(Ua);0<$a||d.calledRun||(d.setStatus?(d.setStatus("Running..."),setTimeout(function(){setTimeout(function(){d.setStatus("")},1);a()},1)):a(),Ra())}}d.run=kd;
function Cc(){var a=ja,b=r,c=!1;ja=r=function(){c=!0};try{var e=d._fflush;e&&e(0);["stdout","stderr"].forEach(function(a){a="/dev/"+a;try{var b=R(a,{H:!0});a=b.path}catch(n){}var e={Ia:!1,exists:!1,error:0,name:null,path:null,object:null,Ka:!1,Ma:null,La:null};try{b=R(a,{parent:!0}),e.Ka=!0,e.Ma=b.path,e.La=b.node,e.name=qb(a),b=R(a,{H:!0}),e.exists=!0,e.path=b.path,e.object=b.node,e.name=b.node.name,e.Ia="/"===b.path}catch(n){e.error=n.j}e&&(b=tb[e.object.rdev])&&b.output&&b.output.length&&(c=!0)})}catch(f){}ja=
a;r=b;c&&sa("stdio streams had content in them that was not flushed. you should set EXIT_RUNTIME to 1 (see the FAQ), or make sure to emit a newline when you printf etc.")}var ld=[];function q(a){if(d.onAbort)d.onAbort(a);void 0!==a?(ja(a),r(a),a=JSON.stringify(a)):a="";wa=!0;var b="abort("+a+") at "+Ha()+"";ld&&ld.forEach(function(c){b=c(b,a)});throw b;}d.abort=q;if(d.preInit)for("function"==typeof d.preInit&&(d.preInit=[d.preInit]);0<d.preInit.length;)d.preInit.pop()();d.noExitRuntime=!0;kd();
  return {
	Module: Module,  // expose original Module
  };
})(window.spp_backend_state_UADE);
/*
 uade_adapter.js: Adapts UADE backend to generic WebAudio/ScriptProcessor player.

   version 1.2
   copyright (C) 2018-2023 Juergen Wothke


 Known limitation: seeking is not supported by UADE

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

function _splitPath(url)
{
	let arr = url.split("/");

	let path = "";
	for (let i = 0; i < arr.length-1; i++) {
		if (i > 0)
		{
			path += "/";
		}
		path += arr[i];
	}
	let filename = arr[arr.length-1];
	return [path, filename];
}


// treat WantedTeam.bin as local resource
class WantedTeamPatcher {
	constructor(fileMapper)
	{
		this._fileMapper = fileMapper;
	}

	mapToVirtualFilename(filename) { return [false, filename]; }

	backend2web(fullFilename)
	{
		let patched = fullFilename.indexOf("WantedTeam.bin") > -1;
		if (patched)
		{
			this._wantedOrig = fullFilename;											// e.g. /songs/JO/WantedTeam.bin
			fullFilename =  this._fileMapper.basePathUADE + "/players/WantedTeam.bin"; // always use local file since the file often seems to be missing
		}
		return [patched, fullFilename];
	}

	web2backend(pathFilenameArray)
	{
		let patched = pathFilenameArray[1].indexOf("WantedTeam.bin") > -1;
		if (patched)
		{
			if (typeof this._wantedOrig != 'undefined')
			{
				pathFilenameArray = _splitPath(this._wantedOrig);	// make sure it "saves" under the original name that the backend expects
			}
		}
		return [patched, pathFilenameArray];
	}
}


// load amiga libs always from the "system" folder
class AmigaLibsPatcher {
	constructor(fileMapper)
	{
		this._fileMapper = fileMapper;
		this._libMap = {};
	}

	mapToVirtualFilename(filename) { return [false, filename]; }

	backend2web(fullFilename)
	{
		let patched = fullFilename.endsWith(".library");
		if (patched)
		{
			let p = _splitPath(fullFilename);

			this._libMap[p[1]] = fullFilename;
			fullFilename = this._fileMapper.basePathUADE + "/system/" + p[1];
		}
		return [patched, fullFilename];
	}

	web2backend(pathFilenameArray)
	{
		let patched = pathFilenameArray[1].endsWith(".library");
		if (patched)
		{
			if (typeof this._libMap != 'undefined')
			{
				// fixme: seems the path info isn't always properly spilt when it gets here
				let tmp = pathFilenameArray[0] + "/" + pathFilenameArray[1];
				let tp = _splitPath(tmp);

				let op = this._libMap[tp[1]];
				if (typeof op != 'undefined')
				{
					pathFilenameArray = _splitPath(op);
				}
			}
		}
		return [patched, pathFilenameArray];
	}
}


// From the WantedTeam's Quartet_ST player perspective there are 2 used
// prefixes: "QTS." and "SMP.". It makes no sense to use this Amiga specific naming
// for what are originally Atari ST files (Even modland has meanwhile replaced the
// Amiga-naming versions with properly named files.). webUADE therefore uses the
// original names with extensions ".4v" and ".set" and maps the files such that
// the eagleplayer sees the dumb Amiga names while the physical files are
// named properly. It also handles the ".4q" archives (which contain both the
// .4v and the .set data) by extracting those files on the fly (see
// converted_quartet_4q() in eagleplayer.c). (webUADE doesn't care whether or not
// the Amiga-naming might still work when used directly as input files. If you
// still have respective files, do rename them!)
// note: the default name for sample sets is: SMP.set (on Atari and on Amiga!)

class QartettST_Patcher {
	constructor(fileMapper)
	{
		this._fileMapper = fileMapper;
	}

	mapToVirtualFilename(filename) { return [false, filename]; }

	backend2web(fullFilename)
	{
		let patched = (fullFilename.indexOf("/SMP.") > -1 );
		if (patched)
		{
			if ((fullFilename.indexOf("/SMP.set") < 0))
			{
				fullFilename = fullFilename.replace("/SMP.", "/") + ".set";
			}
		}
		return [patched, fullFilename];
	}

	web2backend(pathFilenameArray)
	{
		let input = pathFilenameArray[1];

		let patched = input.endsWith(".set");
		if (patched)
		{
			if (!input.endsWith("smp.set") && !input.endsWith("SMP.set"))
			{
				pathFilenameArray[1] = "SMP." + input.replace(".set", "");
			}
		}
		return [patched, pathFilenameArray];
	}
}


// On Modland the extension .sng is used for both RichardJoseph and Zoundmonitor while eagleplayer expects .zm
// for Zoundmonitor - also the Zoundmonitor "Samples" are stored  in a separate top level folder in Modland
// and the lib files all use incorrect lowercase names

class ZoundPatcher {
	constructor(fileMapper)
	{
		this._fileMapper = fileMapper;
		this._nameMap = {};
	}

	mapToVirtualFilename(filename)
	{
		let patched = false;
		if (filename.endsWith(".sng"))
		{
			filename = filename.replace(".sng", ".zm");
			patched = true;
		}
		return [patched, filename];
	}

	backend2web(fullFilename)
	{
		let patched = false;
		if (fullFilename.indexOf("/Samples/") > -1) 	// libs on modland are all lowercase
		{
			let n = fullFilename.substr(0, fullFilename.lastIndexOf("/")) + fullFilename.substr(fullFilename.lastIndexOf("/")).toLowerCase();

			// modland idiots changed the layout and now put the sample files
			// into a separate "top" level folder.."
			n = n.replace("/Samples/", "/../Samples/");

			this._nameMap[n] = fullFilename;
			fullFilename = n;
			patched = true;
		}
		return [patched, fullFilename];
	}

	web2backend(pathFilenameArray)
	{
		let n = pathFilenameArray[0] + "/" + pathFilenameArray[1];
		let orig = this._nameMap[n];
		let patched = typeof orig != 'undefined';
		if (patched)
		{
			// reverse mapping of "Samples" path
			pathFilenameArray = _splitPath(orig);
		}
		return [patched, pathFilenameArray];
	}
}


class AbstractPatcher {
	constructor(fileMapper)
	{
		this._fileMapper = fileMapper;
		this._nameMap = {};

		// set these in subclasses
		this._mapToVirtualCfg;
		this._backend2webCfg;
	}

	mapToVirtualFilename(filename)
	{
		return this._patch(this._mapToVirtualCfg, filename, false)
	}

	backend2web(fullFilename)
	{
		return this._patch(this._backend2webCfg, fullFilename, true)
	}

	web2backend(pathFilenameArray)
	{
		let n = pathFilenameArray[0] + "/" + pathFilenameArray[1];
		let orig = this._nameMap[n];
		let patched = typeof orig != 'undefined';

		if (patched)
		{
			pathFilenameArray = _splitPath(orig);
		}

		return [false, pathFilenameArray];
	}

	_patch(cfg, filename, setMap)
	{
		let patched = false;

		cfg.every((c) => {
			let match = c[0];
			let replacement = c[1];

			if (filename.indexOf(match) > -1)
			{
				let n = filename.replace(match, replacement);
				if (setMap)
				{
					this._nameMap[n] = filename;
				}
				filename = n;
				patched = true;
			}
			return !patched;
		});

		return [patched, filename];
	}
}

// hack: there only seem to be these 2 songs; MusicMaker songs need a prefix that allows
// UADE to match the correct player
class OldMM8_Patcher extends AbstractPatcher {
	constructor(fileMapper)
	{
		super(fileMapper);

		this._mapToVirtualCfg = [	["/best of guitars.", "/mm8.best of guitars."],
									["/moveback.", "/mm4.moveback."]	];
		this._backend2webCfg = [	["mm8.", ""],
									["mm4.", ""]	];
	}
}

// .mcmd detection conflicts with certain Hippel songs.. fake postfix to force correct detection
class MCMD_Patcher extends AbstractPatcher {
	constructor(fileMapper) {
		super(fileMapper);

		this._mapToVirtualCfg = [	[".mcmd", ".mcmd_org"]	];
		this._backend2webCfg = [	[".mcmd_org", ".mcmd"]	];
	}
}

// mdst. prefix needed for proper ST player selection (modland's naming of the
// "smpl." file also is total garbage.

// note: the player actually uses /players/ENV/EaglePlayer/EP-TFMX_Pro.cfg
class TFMX_ST_Patcher extends AbstractPatcher {
	constructor(fileMapper) {
		super(fileMapper);

		this._mapToVirtualCfg = [	["/mdat.", "/mdst."]	];
		this._backend2webCfg = [	["/mdst.", "/mdat."],
									["smpl.mdst.", "smpl."]	];
	}
}

// patch various names used on Modland: either the modland names are
// garbage or UADE's naming conventions are..
class ModlandPatcher {
	constructor(fileMapper, filename)
	{
		this._fileMapper = fileMapper;

		this._originalFile = decodeURI(filename);
		this._modlandMap = {};
	}

	mapToVirtualFilename(filename) { return [false, filename]; }

	backend2web(fullFilename)
	{
		fullFilename = decodeURI(fullFilename);	// replace escape sequences...

		// modland uses wrong (lower) case for practically all sample file names.. damn jerks
		let output = fullFilename.replace(".adsc.AS", ".adsc.as");	// AudioSculpture

		output = output.replace("/SMP.", "/smp.");	// Dirk Bialluch, Dynamic Synthesizer, Jason Page, Magnetic Fields Packer,
													// Quartet ST(not used here since handled previously), Synth Dream, Thomas Hermann
		output = output.replace(".SSD", ".ssd");	// Paul Robotham
		output = output.replace(".INS", ".ins");	// Richard Joseph
		output = output.replace("/mcS.", "/mcs.");	// Mark Cooksey Old


		let o = this._originalFile.substr(this._originalFile.lastIndexOf("/") + 1);

		let ot = output.substr(output.lastIndexOf("/")+1);

		if (this._originalFile.endsWith(".spm") && (output.indexOf("/SPS.") > 0)) 	// Stonetracker
		{
			output = output.replace("SPS.", "").replace(".spm", ".sps");
		}
		else if (this._originalFile.endsWith(".soc") && output.endsWith(".so")) 	// Hippel ST COSO
		{
			output = output.substr(0, output.lastIndexOf("/")) + "/smp.set";
		}
		else if (this._originalFile.endsWith(".pap") && output.endsWith(".pa"))  // Pierre Adane Packer
		{
			output = output.substr(0, output.lastIndexOf("/")) + "/smp.set";
		}
		else if (this._originalFile.endsWith(".osp") && output.endsWith(".os"))  // Synth Pack
		{
			output = output.substr(0, output.lastIndexOf("/")) + "/smp.set";
		}
		else if (o.startsWith("sdr.") && ot.startsWith("smp."))  // Synth Dream  (always use the "set".. other songs don't seem to play properly anyway - even in uade123)
		{
			output = output.substr(0, output.lastIndexOf("/")) + "/smp.set";
		}
		else if (this._originalFile.endsWith(".ymst") && output.endsWith("replay")) // YMST
		{
			let idx = output.lastIndexOf("/");
			let fn = output.substr(idx).toLowerCase().replace(" ", "_");	// see inconsistent zout-game.ymst
			output = output.substr(0, idx) + fn;
		}

		// map:  actual file name -> "wrong" name used on emu side
		// e.g.  "smp.set"  ->      "dyter07 ongame01.os"
		if (fullFilename != output)	// remember the filename mapping (path is the same)
			this._modlandMap[output.replace(/^.*[\\\/]/, '')]= fullFilename.replace(/^.*[\\\/]/, '');	// needed to create FS expected by "amiga"

		return [true, output];
	}

	web2backend(pathFilenameArray)
	{
		pathFilenameArray = this._fileMapper.normalizePathFilename(pathFilenameArray);

		let patched = typeof this._modlandMap[pathFilenameArray[1]] != 'undefined';
		if (patched)
		{
			pathFilenameArray[1] = this._modlandMap[pathFilenameArray[1]];	// reverse map
		}
		return [patched, pathFilenameArray];
	}
}


/**
* This mapper has three main purposes: 1) handle the various shared resource files
* that need to be located independently of the location of the music files.
* 2) handle the various "sample lib" files that are named totally inconsistently
* on modland - so that the player can still play respective songs directly from
* that collection. 3) make sure the song files use the "virtual" file extensions that
* UADE's eagleplayer detection expects.
*/
class UadeFileMapper extends SimpleFileMapper {
	constructor(module, basePathUADE, modlandMode)
	{
		super(module);

		this.basePathUADE = basePathUADE;

		this._modlandMode = (typeof modlandMode != 'undefined') ? modlandMode : 0;
		this._patchers = [];
	}

	init(filename)
	{
		// the folder names matched below to identify song formats are Modland specific,
		// but by using those names UADE can be given an extra hint whenever necessary..

		this._patchers = [];

		this._patchers.push(new WantedTeamPatcher(this));
		this._patchers.push(new AmigaLibsPatcher(this));

		if (filename.endsWith(".4v")) this._patchers.push(new QartettST_Patcher(this));
		if (filename.indexOf("/Zoundmonitor/") > -1) this._patchers.push(new ZoundPatcher(this));

		if ((filename.indexOf("/best of guitars.") > -1) || (filename.indexOf("/moveback.") > -1)) this._patchers.push(new OldMM8_Patcher(this));
		if (filename.indexOf("/MCMD/") > -1) this._patchers.push(new MCMD_Patcher(this));
		if (filename.indexOf("/TFMX ST/") > -1) this._patchers.push(new TFMX_ST_Patcher(this));

		if (this._modlandMode) this._patchers.push(new ModlandPatcher(this, filename));
	}

	mapToVirtualFilename(filename)
	{
		filename = super.mapToVirtualFilename(filename);
		filename = decodeURI(filename);	// e.g. see TFMX songs Huelsbeck with spaces XXX.. move into baseclass?

		let done = false;
		this._patchers.every((p) => {
			[done, filename] = p.mapToVirtualFilename(filename);
			return !done;
		});
		return filename;
	}

	registerFileData(pathFilenameArray, data)
	{
		// NOTE: UADE uses the "C fopen" based impl to access all files, i.e. the files
		// MUST BE properly provided within the FS (but the local cache within the player
		// is also used when pre-play song conversion is used)

		let done = false;

		this._patchers.every((p) => {
			[done, pathFilenameArray] = p.web2backend(pathFilenameArray);
			return !done;
		});

		return this._registerEmscriptenFileData(this.normalizePathFilename(pathFilenameArray), data);
	}

	mapBackendFilename(name)
	{
		// the input "name" is what UADE later will use for fopen (i.e. the above registerFileData()
		// must ensure that the FS contains the file under that name

		let done = false;

		this._patchers.every((p) => {
			[done, name] = p.backend2web(name);
			return !done;
		});

		return name;
	}
};

class UADEBackendAdapter extends EmsHEAP16BackendAdapter {
	constructor(basePath, modlandMode, disableModConverter, onUpdateSongInfo)
	{
		super(backend_UADE.Module, 2, new UadeFileMapper(backend_UADE.Module, basePath, modlandMode),
						new HEAP16ScopeProvider(backend_UADE.Module, 0x8000));

		if (typeof onUpdateSongInfo == 'undefined') onUpdateSongInfo = function() {};
		this._onUpdateSongInfo = onUpdateSongInfo;

		this.disableModConverter = (typeof disableModConverter != 'undefined') ? disableModConverter : 0;

		this._basePathUADE = basePath;
		this._sendUpdateNotifications = true;

		this.ensureReadyNotification();
	}

	disableModConverter(disable)
	{
		// disabling "automatic mod file comversion" reverts to UADE's original mod file handling
		this.disableModConverter = (typeof disable != 'undefined') ? disable : 0;
	}

	loadMusicData(sampleRate, path, filename, data, options)
	{
		this._sendUpdateNotifications = false;

		this.Module.ccall('emu_prepare', 'number', ['string'], [this._basePathUADE]);	// init virtual FS

		let ret = this.Module.ccall('emu_load_file', 'number',
							['number', 'string', 'string','number'],
							[sampleRate, path, filename, this.disableModConverter]);

		if (ret == 0)
		{
			// UADE's maximum sample rate is SOUNDTICKS_NTSC 3579545 which
			// should never be a relevant limitation here..
			let inputSampleRate = sampleRate;
			this.resetSampleRate(sampleRate, inputSampleRate);
		}
		return ret;
	}

	setPanning(value)
	{
		if (!this.isAdapterReady()) return;

		let pan = Math.min(Math.max(-1.0, value), 1.0);
		this.Module.ccall('emu_set_panning', 'number', ['number'], [pan]);
	}

	evalTrackOptions(options)
	{
		super.evalTrackOptions(options);

		if ((typeof options.pan != 'undefined'))
		{
			this.setPanning(options.pan);
		}

		let track = (typeof options.track === 'undefined') ? -1 : options.track;
		let ret = this.Module.ccall('emu_set_subsong', 'number', ['number'], [track]);

		this._sendUpdateNotifications = true; // any updates that occur later require a notification

		return ret;
	}

	_setDefaultSongAttributes(target)
	{

		let keys = ["info1", "info2", "info3", "mins", "maxs", "curs", "infoText", "format", "player"];
		keys.forEach((key) => {
			target[key] = "";
		});
	}

	getSongInfoMeta()
	{
		return {
			info1: String,
			info2: String,
			info3: String,
			mins: String,
			maxs: String,
			curs: String,
			infoText: String,
			format: String,
			player: String
		};
	}

	// by default called after load of song - normally backends then have the
	// meta info (but not UADE - where some meta data may be asynchronously filled in later)

	updateSongInfo(filename)
	{
		if (Object.keys(this._songInfo).length == 0)
			this._setDefaultSongAttributes(this._songInfo);
	}


	// disable unsupported default impls
	getMaxPlaybackPosition()
	{
		return 0;
	}
	getPlaybackPosition()
	{
		return 0;
	}
	seekPlaybackPosition(ms)
	{
	}

// --------------------------- async file loading stuff -------------------------

	handleBackendSongAttributes(backendAttr)	// called via _songUpdateCallback from callback.js
	{
		// UADE may trigger this from "song load" before updateSongInfo is called (the respective
		// "player" and "format" info is triggered before the remaining song info)

		if (Object.keys(this._songInfo).length == 0)
			this._setDefaultSongAttributes(this._songInfo);

		Object.assign(this._songInfo, backendAttr);


		// unfortunately UADE also triggers some of these notifications late during "computeAudioSamples"
		// (testcase: Confuzion (Hubbard cover).4v; which sets "maxs, mins, curs" from there)

		if (this._sendUpdateNotifications) this._onUpdateSongInfo();	// notify UI of changes
	}
};