sub pkg_load {
		print "hello world\n";
		print "this is my stupid message\n";
		NeoStats::print("and this is a neostats message\n");
}
pkg_load();
