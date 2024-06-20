// utilites useful when dealing with simple WEBGL

window.onerror = function(msg, url, lineno) {
	alert(url + '(' + lineno + '): ' + msg);
}

function setContextWEBGL(canvas)
{
	window.gl = canvas.getContext('webgl2', { premultipliedAlpha: false });

	if ((window.gl == null) || (typeof window.gl == 'undefined'))
	{
		alert("Unfortunately your browser does not support WEBGL2 which is required to run this page.");
		return false;
	}
	return true;
}

function useWEBGL()
{
	if (typeof window._enableWEBGL == 'undefined')
	{
		const urlParams = new URLSearchParams(window.location.search);
		const disableWEBGL = urlParams.get('disableWEBGL');
		window._enableWEBGL = (typeof disableWEBGL == 'undefined') || (disableWEBGL == null);

		if (window._enableWEBGL)
		{
			window._enableWEBGL = setContextWEBGL(document.getElementById('c'));
		}
	}
	return window._enableWEBGL;
}

function checkGLError()
{
    let error = gl.getError();
    if (error != gl.NO_ERROR && error != gl.CONTEXT_LOST_WEBGL)
	{
        let str = "GL Error: " + error;
        document.body.appendChild(document.createTextNode(str));
        throw str;
    }
}

function createShader(str, type)
{
	let shader = gl.createShader(type);
	gl.shaderSource(shader, str);
	gl.compileShader(shader);

	if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS))
	{
		throw gl.getShaderInfoLog(shader);
	}
	return shader;
}

function setupProgram(vstr, fstr)
{
	let program = gl.createProgram();
	let vshader = createShader(vstr, gl.VERTEX_SHADER);
	let fshader = createShader(fstr, gl.FRAGMENT_SHADER);
	gl.attachShader(program, vshader);
	gl.attachShader(program, fshader);
	gl.linkProgram(program);

	if (!gl.getProgramParameter(program, gl.LINK_STATUS))
	{
		throw gl.getProgramInfoLog(program);
	}
	return program;
}

function screenQuad()
{
	let vertexPosBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, vertexPosBuffer);
	let vertices = [-1, -1, 1, -1, -1, 1, 1, 1];
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW);
	vertexPosBuffer.itemSize = 2;
	vertexPosBuffer.numItems = 4;

	/*
	 2___3
	 |\  |
	 | \ |
	 |__\|
	 0   1
	*/
	return vertexPosBuffer;
}

function linkProgram(program)
{
	let vshader = createShader(program.vshaderSource, gl.VERTEX_SHADER);
	let fshader = createShader(program.fshaderSource, gl.FRAGMENT_SHADER);
	gl.attachShader(program, vshader);
	gl.attachShader(program, fshader);
	gl.linkProgram(program);

	if (!gl.getProgramParameter(program, gl.LINK_STATUS))
	{
		throw gl.getProgramInfoLog(program);
	}
}

function loadFile(url, callback, noCache, isJson)
{
	let request = new XMLHttpRequest();
	request.onreadystatechange = function() {
		if (request.readyState == 1)
		{
			if (isJson)
			{
				request.overrideMimeType('application/json');
			}
			request.send();
		}
		else if (request.readyState == 4)
		{
			if (request.status == 200)
			{
				callback(request.responseText);
			}
			else if (request.status == 404)
			{
				throw 'File "' + url + '" does not exist.';
			}
			else
			{
				throw 'XHR error ' + request.status + '.';
			}
		}
	};

	if (noCache)
	{
		url += '?' + (new Date()).getTime();
	}
	request.open('GET', url, true);
}

function loadProgram(vs, fs, callback)
{
	let program = gl.createProgram();
	function vshaderLoaded(str) {
		program.vshaderSource = str;
		if (program.fshaderSource)
		{
			linkProgram(program);
			callback(program);
		}
	}
	function fshaderLoaded(str) {
		program.fshaderSource = str;
		if (program.vshaderSource)
		{
			linkProgram(program);
			callback(program);
		}
	}
	loadFile(vs, vshaderLoaded, true);
	loadFile(fs, fshaderLoaded, true);
	return program;
}

function activateTexture2d(unit, texture)
{
	// the WEBGL call sequence is just so braindead!
	gl.activeTexture(gl.TEXTURE0 + unit);
	gl.bindTexture(gl.TEXTURE_2D, texture);
}

function activateTextureCubeMap(unit, texture)
{
	gl.activeTexture(gl.TEXTURE0 + unit);
	gl.bindTexture(gl.TEXTURE_CUBE_MAP, texture);
}

function setTextureConfig(texture, width, height)
{
	gl.bindTexture(gl.TEXTURE_2D, texture);
	{
		// define size and format of level 0
		const level = 0;
		const internalFormat = gl.RGBA;
		const border = 0;
		const format = gl.RGBA;
		const type = gl.UNSIGNED_BYTE;
		const data = null;
		gl.texImage2D(gl.TEXTURE_2D, level, internalFormat,
					width, height, border, format, type, data);

		// set the filtering so we don't need mips
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
	}
}
function createTexture(unit, width, height)
{
	const texture = gl.createTexture();
	setTextureConfig(texture, width, height);
	return texture;
}

function printStatus(s)
{
	switch (s) {
		case 0:
			return "gl.ERROR";
		case gl.FRAMEBUFFER_COMPLETE:
			return "gl.FRAMEBUFFER_COMPLETE";
		case gl.FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			return "gl.FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
		case gl.FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			return "gl.FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
		case gl.FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
			return "gl.FRAMEBUFFER_INCOMPLETE_DIMENSIONS";
		case gl.FRAMEBUFFER_UNSUPPORTED:
			return "gl.FRAMEBUFFER_UNSUPPORTED";
		case gl.FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			return "gl.FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
	}
	return "unknown gl.OK";
}

function setFrameBuffer(attachmentPoint, targetTexture, width, height)
{
	if (targetTexture == null)
	{
		gl.bindFramebuffer(gl.FRAMEBUFFER, null);
	}
	else
	{
		// fucking garbage does NOT preserve these setttings..
		setTextureConfig(targetTexture, width, height)

		// Create and bind the framebuffer
		const fb = gl.createFramebuffer();
		gl.bindFramebuffer(gl.FRAMEBUFFER, fb);

		// attach the texture as the color attachment
		const level = 0;
		gl.framebufferTexture2D(gl.FRAMEBUFFER, attachmentPoint, gl.TEXTURE_2D, targetTexture, level);

		return fb;
	}
}

function readImage(filename)
{
	return new Promise((resolve, reject) => {
		let image = document.createElement("img");

		image.onload = e => resolve(e.returnValue ? image : null);
		image.onerror = reject;

		image.src = filename;
	});
}

function loadCubemap(fn1, fn2, fn3, fn4, fn5, fn6) {
	return new Promise((resolve, reject) => {
		let p1 = readImage(fn1);
		let p2 = readImage(fn2);
		let p3 = readImage(fn3);
		let p4 = readImage(fn4);
		let p5 = readImage(fn5);
		let p6 = readImage(fn6);

		Promise.all([p1, p2, p3, p4, p5, p6]).then((images) => {

			let texture = gl.createTexture();
			gl.bindTexture(gl.TEXTURE_CUBE_MAP, texture);

			gl.texImage2D(gl.TEXTURE_CUBE_MAP_POSITIVE_X, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, images[0]);
			gl.texImage2D(gl.TEXTURE_CUBE_MAP_NEGATIVE_X, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, images[1]);
			gl.texImage2D(gl.TEXTURE_CUBE_MAP_POSITIVE_Y, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, images[2]);
			gl.texImage2D(gl.TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, images[3]);
			gl.texImage2D(gl.TEXTURE_CUBE_MAP_POSITIVE_Z, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, images[4]);
			gl.texImage2D(gl.TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, images[5]);


			gl.generateMipmap(gl.TEXTURE_CUBE_MAP);
			gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR);

			resolve(texture);
		});
	});
}


function loadTexture(filename, mode)
{
	if (typeof mode == 'undefined') mode = gl.REPEAT;

	return new Promise((resolve, reject) => {
		let p = readImage(filename);

		p.then((image) => {

			let t = gl.createTexture();
			gl.bindTexture(gl.TEXTURE_2D, t);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, mode);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, mode);

			gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, image);
			checkGLError();

			resolve(t);
		});
	});
}



(function() {
    let lastTime = 0;
    let vendors = ['ms', 'moz', 'webkit', 'o'];
    for(let x = 0; x < vendors.length && !window.requestAnimationFrame; ++x) {
        window.requestAnimationFrame = window[vendors[x] + 'RequestAnimationFrame'];
        window.cancelRequestAnimationFrame = window[vendors[x] + 'CancelRequestAnimationFrame'];
    }

    if (!window.requestAnimationFrame)
	{
        window.requestAnimationFrame = function(callback, element) {
            let currTime = new Date().getTime();
            let timeToCall = Math.max(0, 16 - (currTime - lastTime));
            let id = window.setTimeout(function() { callback(currTime + timeToCall); },
              timeToCall);
            lastTime = currTime + timeToCall;
            return id;
        };
	}
    if (!window.cancelAnimationFrame)
	{
        window.cancelAnimationFrame = function(id) {
            clearTimeout(id);
        };
	}
}())