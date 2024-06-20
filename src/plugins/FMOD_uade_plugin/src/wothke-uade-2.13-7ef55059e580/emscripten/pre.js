// some utility code
function startsWith(a, str) {
	return (a.match("^"+str)==str)
}
function startsWith2(a, arr) {
	var i;
	for(i= 0; i<arr.length; i++) {
		var k= arr[i];
		if (a.match("^"+k)==k)
			return k;
	}
	return null;
}
