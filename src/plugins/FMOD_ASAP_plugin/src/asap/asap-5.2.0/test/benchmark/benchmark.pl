use Time::HiRes qw(time);
use strict;

print q{File                          ,MSVC ,MinGW,MGx64,Java ,C#   ,GME   ,SAP
};
my @progs = (
	'win32/msvc/asapconv.exe -o .wav',
	'win32/asapconv.exe -o .wav',
	'win32/x64/asapconv.exe -o .wav',
	'java -jar java/asap2wav.jar',
	'csharp/asap2wav.exe',
	'test/benchmark/gme_benchmark.exe',
	'test/benchmark/sap_benchmark.exe'
);
for my $file (glob 'test/benchmark/*.sap') {
	printf '%30s', $file =~ m{([^/]+)$};
	prog: for my $prog (@progs) {
		my @cmd = (split(/ /, $prog), $file);
		my $time = time;
		my $COUNT = 5;
		print STDERR "@cmd\n";
		for my $i (1 .. $COUNT) {
			unless (system(@cmd) == 0) {
				print ',ERROR';
				next prog;
			}
		}
		$time = (time() - $time) / $COUNT;
		printf ',%5.2f', $time;
	}
	print "\n";
}
