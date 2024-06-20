// copyright (C) 2023 Juergen Wothke
//
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

window.shaders = window.shaders || {};

window.shaders.vertex = `#version 300 es
	in vec2 aVertexPosition;
	void main() {
		gl_Position = vec4(aVertexPosition, 0, 1);
	}
`;

window.shaders.fragment = `#version 300 es
#define PI  3.1415926535897932384626433832795
#define PI2  6.283185307179586476925286766559
#define MC(a,b,t)  mix(a, b, cos(t) * .5 )

#ifdef GL_FRAGMENT_PRECISION_HIGH
	precision highp float;
#else
	precision mediump float;
#endif
	precision mediump int;
	uniform vec2 uCanvasSize;
	uniform float iTime;
	uniform sampler2D tex;

	out vec4 outputColor;



	// lightning effect by srtuss (see https://www.shadertoy.com/view/XlsGRs )
	struct ITSC
	{
		vec3 p;
		float dist;
		vec3 n;
		vec2 uv;
	};

	vec2 rotate(vec2 p, float a)
	{
		return vec2(p.x * cos(a) - p.y * sin(a), p.x * sin(a) + p.y * cos(a));
	}

	float hash1(float p)
	{
		return fract(sin(p * 172.435) * 29572.683) - 0.5;
	}

	float hash2(vec2 p)
	{
		vec2 r = (456.789 * sin(789.123 * p.xy));
		return fract(r.x * r.y * (1.0 + p.x));
	}

	float ns(float p)
	{
		float fr = fract(p);
		float fl = floor(p);
		return mix(hash1(fl), hash1(fl + 1.0), fr);
	}

	float fbm(float p)
	{
		return (ns(p) * 0.4 + ns(p * 2.0 - 10.0) * 0.125 + ns(p * 8.0 + 10.0) * 0.025);
	}

	float fbmd(float p)
	{
		float h = 0.01;
		return atan(fbm(p + h) - fbm(p - h), h);
	}

	float arcsmp(float x, float seed)
	{
		return fbm(x * 3.0 + seed * 1111.111) * (1.0 - exp(-x * 5.0));
	}

	float arc(vec2 p, float seed, float len)
	{
		p *= len;
		//p = rotate(p, iTime);
		float v = abs(p.y - arcsmp(p.x, seed));
		v += exp((2.0 - p.x) * -4.0);
		v = exp(v * -60.0) + exp(v * -10.0) * 0.6;
		//v += exp(p.x * -2.0);
		v *= smoothstep(0.0, 0.05, p.x);
		return v;
	}

	float arcc(vec2 p, float sd)
	{
		float v = 0.0;
		float rnd = fract(sd);
		float sp = 0.0;
		v += arc(p, sd, 1.0);
		for(int i = 0; i < 4; i ++)
		{
			sp = rnd + 0.01;
			vec2 mrk = vec2(sp, arcsmp(sp, sd));
			v += arc(rotate(p - mrk, fbmd(sp)), mrk.x, mrk.x * 0.4 + 1.5);
			rnd = fract(sin(rnd * 195.2837) * 1720.938);
		}
		return v;
	}
	void tPlane(inout ITSC hit, vec3 ro, vec3 rd, vec3 o, vec3 n, vec3 tg, vec2 si)
	{
		vec2 uv;
		ro -= o;
		float t = -dot(ro, n) / dot(rd, n);
		if(t < 0.0)
			return;
		vec3 its = ro + rd * t;
		uv.x = dot(its, tg);
		uv.y = dot(its, cross(tg, n));
		if(abs(uv.x) > si.x || abs(uv.y) > si.y)
			return;

		//if(t < hit.dist)
		{
			hit.dist = t;
			hit.uv = uv;
		}
		return;
	}


	void main() {
		vec2 uv = (gl_FragCoord.xy / uCanvasSize.xy) * 2.0 - vec2(1.0,1.0);
		float l = length(uv);

		float atime= iTime;	// rotating image
		float stime;		// spiral
//			stime = float(int(iTime*1000000.) % 600000000) / 1000000. ;	// reset after 10mins to avoid Moirée
		stime = iTime ;	// reset after 10mins to avoid Moirée XXX
		float ltime= iTime;	// lightning

		float a = atan(uv.x,uv.y)/PI2 + stime*1.4;
		float p = a + l*5.;

		p = a + (pow(2.71828,-l)*MC(0.1, 3.0, stime*.1)) / (l*MC(1.0, 0.2, stime*.2));
		p = smoothstep(l*sin(p*PI*-2.)/PI/fwidth(p), .40, (l, 1.5));

		outputColor = vec4(p, p, p, 1.0);

		// add image
		vec2 rot = vec2(1.2, 1.1 + 5.4*cos(atime*0.1)-2.1*sin(atime*0.2));
		vec2 tlate = vec2(0.0, 0.1+0.1*sin(atime)+0.1*cos(atime*0.6));

		vec2 tv = vec2( uv.x * rot.y + uv.y * rot.x,
						uv.y * rot.y - uv.x * rot.x) + tlate;

		outputColor.rgb = mix(outputColor.rbg, texture(tex, tv).rgb, texture(tex, tv).a);

		// add lighning
		float camtm = ltime * 0.15;
		vec3 ro = vec3(0.0,0.0,0.0);
		vec3 rd = normalize(vec3(uv, 1.2));
		ITSC tunRef;

		vec4 rnd = vec4(0.1, 0.2, 0.3, 0.4);
		float arcv = 0.0, arclight = 0.0;
		for(int i = 0; i < 3; i++)
		{
			float v = 0.0;
			rnd = fract(sin(rnd * 1.111111) * 298729.258972);
			float ts = rnd.z * 4.0 * 1.61803398875 + 1.0;
			float arcfl = floor(ltime / ts + rnd.y) * ts;
			float arcfr = fract(ltime / ts + rnd.y) * ts;

			ITSC arcits;
			arcits.dist = 1e38;
			float arca = rnd.x + arcfl * 2.39996;	// rotation
			float arcz = ro.z + 1.0 + rnd.x * 12.0;
			tPlane(arcits, ro, rd, vec3(0.0, 0.0, arcz), vec3(0.0, 0.0, -1.0), vec3(cos(arca), sin(arca), 0.0), vec2(2.0));

			float arcseed = floor(ltime * 17.0 + rnd.y);
			if(arcits.dist < 20.0)
			{
				arcits.uv *= 0.8;
				v = arcc(vec2(1.0 - abs(arcits.uv.x), arcits.uv.y * sign(arcits.uv.x)) * 1.4, arcseed * 0.033333);
			}
			float arcdur = rnd.x * 0.2 + 0.05;
			float arcint = smoothstep(0.1 + arcdur, arcdur, arcfr);
			v *= arcint;
			arcv += v;
			arclight += exp(abs(arcz - tunRef.p.z) * -0.3) * fract(sin(arcseed) * 198721.6231) * arcint;
		}
		vec3 arccol = vec3(0.9, 0.7, 0.7);
		vec3 col = arclight * arccol * 0.5;
		col = mix(col, arccol, clamp(arcv, 0.0, 1.0));
		col = pow(col, vec3(1.0, 0.8, 0.5) * 1.5) * 1.5;
		col = pow(col, vec3(1.0 / 2.2));

		outputColor.rgb+=col;

		if ((outputColor.r+outputColor.g+outputColor.b)/3. > .9) outputColor.a = 0.;
	}
`;
