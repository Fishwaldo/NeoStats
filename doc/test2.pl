sub pkg_load {
		print "hello world\n";
		print "this is my stupid message\n";
		NeoStats::print("and this is a neostats message\n");
}
NeoStats::register( "Test Script 2", "2.0",                  
                 "Test Script 2 Description" ); 
pkg_load();
