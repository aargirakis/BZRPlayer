// some utility code
startsWith= function(a, str) {
	return (a.match("^"+str)==str)
};
startsWith2= function(a, arr) {
	var i;
	for(i= 0; i<arr.length; i++) {
		var k= arr[i];
		if (a.match("^"+k)==k)
			return k;
	}
	return null;
};
