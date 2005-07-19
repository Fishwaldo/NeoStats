# NeoStats Perl Module to test Perl Interface
# 
# Copyright 2004 Justin Hammond
#
# You can use this module as a template to code your own. I'll try to describe a lot of it 
# for you.

# First thing that you must ensure happens (ie, first command) is to register this script
# Using the following command:

NeoStats::register( "Test", "1.0", "Test Script 1 description", "setupbot", "shutdownbot");

# Events make up the core communications of NeoStats, here you register the events your 
# Interested in, and the function to call. A optional third arguement ($options) allows you
# to specify optional items such as:
#    Event Flags: 
#	These Determine under what circumstances the event will be called
#	Specify as $option->{flags} = <EVENT_FLAG_???>
#    User Data:
#	Not Currently implemented

NeoStats::hook_event(NeoStats::EVENT_MODULELOAD, "event_moduleload");
NeoStats::hook_event(NeoStats::EVENT_MODULEUNLOAD, "event_moduleunload");
NeoStats::hook_event(NeoStats::EVENT_SERVER, "event_server");
NeoStats::hook_event(NeoStats::EVENT_SQUIT, "event_squit");
NeoStats::hook_event(NeoStats::EVENT_PING, "event_ping");
NeoStats::hook_event(NeoStats::EVENT_PONG, "event_pong");
NeoStats::hook_event(NeoStats::EVENT_SIGNON, "event_signon");
NeoStats::hook_event(NeoStats::EVENT_QUIT, "event_quit");
NeoStats::hook_event(NeoStats::EVENT_NICKIP, "event_nickip");
NeoStats::hook_event(NeoStats::EVENT_KILL, "event_kill");
NeoStats::hook_event(NeoStats::EVENT_GLOBALKILL, "event_globalkill");
NeoStats::hook_event(NeoStats::EVENT_LOCALKILL, "event_localkill");
NeoStats::hook_event(NeoStats::EVENT_SERVERKILL, "event_serverkill");
NeoStats::hook_event(NeoStats::EVENT_BOTKILL, "event_botkill");
NeoStats::hook_event(NeoStats::EVENT_NICK, "event_nick");
NeoStats::hook_event(NeoStats::EVENT_AWAY, "event_away");
NeoStats::hook_event(NeoStats::EVENT_UMODE, "event_umode");
NeoStats::hook_event(NeoStats::EVENT_SMODE, "event_smode");
NeoStats::hook_event(NeoStats::EVENT_NEWCHAN, "event_newchan");
NeoStats::hook_event(NeoStats::EVENT_DELCHAN, "event_delchan");
NeoStats::hook_event(NeoStats::EVENT_JOIN, "event_join");
NeoStats::hook_event(NeoStats::EVENT_PART, "event_part");
NeoStats::hook_event(NeoStats::EVENT_PARTBOT, "event_partbot");
NeoStats::hook_event(NeoStats::EVENT_EMPTYCHAN, "event_emptychan");
NeoStats::hook_event(NeoStats::EVENT_KICK, "event_kick");
NeoStats::hook_event(NeoStats::EVENT_KICKBOT, "event_kickbot");
NeoStats::hook_event(NeoStats::EVENT_TOPIC, "event_topic");
NeoStats::hook_event(NeoStats::EVENT_CMODE, "event_cmode");
NeoStats::hook_event(NeoStats::EVENT_PRIVATE, "event_private");
NeoStats::hook_event(NeoStats::EVENT_NOTICE, "event_notice");
NeoStats::hook_event(NeoStats::EVENT_CPRIVATE, "event_cprivate");
NeoStats::hook_event(NeoStats::EVENT_CNOTICE, "event_cnotice");
NeoStats::hook_event(NeoStats::EVENT_GLOBOPS, "event_globops");
NeoStats::hook_event(NeoStats::EVENT_CHATOPS, "event_chatops");
NeoStats::hook_event(NeoStats::EVENT_WALLOPS, "event_wallops");
NeoStats::hook_event(NeoStats::EVENT_CTCPVERSIONRPL, "event_ctcpversionrpl");
NeoStats::hook_event(NeoStats::EVENT_CTCPVERSIONREQ, "event_ctcpversionreq");
NeoStats::hook_event(NeoStats::EVENT_CTCPFINGERRPL, "event_ctcpfingerrpl");
NeoStats::hook_event(NeoStats::EVENT_CTCPFINGERREQ, "event_ctcpfingerreq");
NeoStats::hook_event(NeoStats::EVENT_CTCPACTIONREQ, "event_ctcpactionreq");
NeoStats::hook_event(NeoStats::EVENT_CTCPTIMERPL, "event_ctcptimerpl");
NeoStats::hook_event(NeoStats::EVENT_CTCPTIMEREQ, "event_ctcptimereq");
NeoStats::hook_event(NeoStats::EVENT_CTCPPINGRPL, "event_ctcppingrpl");
NeoStats::hook_event(NeoStats::EVENT_CTCPPINGREQ, "event_ctcppingreq");
NeoStats::hook_event(NeoStats::EVENT_DCCSEND, "event_dccsend");
NeoStats::hook_event(NeoStats::EVENT_DCCCHAT, "event_dccchat");
NeoStats::hook_event(NeoStats::EVENT_DCCCHATMSG, "event_dccchatmsg");
NeoStats::hook_event(NeoStats::EVENT_ADDBAN, "event_addban");
NeoStats::hook_event(NeoStats::EVENT_DELBAN, "event_delban");

sub event_moduleload {
	my ($param1) = @_;
	NeoStats::print("New Module Loaded: $param1");
}

sub event_moduleunload {
	my ($param1) = @_;
	NeoStats::print("Module Unloaded: $param1");
}

sub event_server {
	my ($test) = @_;
	NeoStats::print ("New Server $test");
	my $server = NeoStats::FindServer($test);
	NeoStats::print ("Server Uplink $test->{uplink}");
}

sub event_squit {
	my ($server, $msg) = @_;
	NeoStats::print ("Server $server Squit: $msg");
}

sub event_ping {
	my ($source) = @_;
	NeoStats::print ("Ping $source");
}

sub event_pong {
	my ($source) = @_;
	NeoStats::print ("Pong $source");
}

sub event_signon {
	my ($source) = @_;
	NeoStats::print ("Signon $source");
	my $user = NeoStats::FindUser($source);
	NeoStats::print ("Host: $user->{hostname}");
	NeoStats::print ("Server $user->{server}");
}

sub event_quit {
	my ($source, $msg) = @_;
	NeoStats::print ("Quit $source: $msg");
}

sub event_nickip {
	my ($source) = @_;
	NeoStats::print ("NickIP $source");
}

sub event_kill {
	my ($source, $target, $msg) = @_;
	NeoStats::print ("KILL $target by $source: $msg");
}

sub event_globalkill {
	my ($source, $target, $msg) = @_;
	NeoStats::print ("GLOBALKILL $target by $source: $msg");
}

sub event_localkill {
	my ($source, $target, $msg) = @_;
	NeoStats::print ("LOCALKILL $target by $source: $msg");
}

sub event_serverkill {
	my ($source, $target, $msg) = @_;
	NeoStats::print ("SEVERKILL $target by $source: $msg");
}

sub event_botkill {
	my ($target, $msg) = @_;
	NeoStats::print ("BOTKILL $target: $msg");
}

sub event_nick {
	my ($source, $target) = @_;
	NeoStats::print ("NICKChange $source: $target");
}

sub event_away {
	my ($source) = @_;
	NeoStats::print ("AwayChange $source");
}

sub event_umode {
	my ($source, $mode) = @_;
	NeoStats::print ("UMODE $source, $mode");
}

sub event_smode {
	my ($source, $mode) = @_;
	NeoStats::print ("SMODE $source, $mode");
}

sub event_newchan {
	my ($channel) = @_;
	NeoStats::print ("NewChan $channel");
}

sub event_delchan {
	my ($channel) = @_;
	NeoStats::print ("DelChan $channel");
}

sub event_join {
	my ($channel, $source) = @_;
	NeoStats::print ("Join $channel: $source");
	my $chan = NeoStats::FindChan($channel);
	NeoStats::print ("Channel users $chan->{users}");
}

sub event_part {
	my ($channel, $source, $msg) = @_;
	NeoStats::print ("Part $channel: $source: $msg");
}

sub event_partbot {
	my ($channel, $source, $msg) = @_;
	NeoStats::print ("Partbot $channel: $source: $msg");
}

sub event_emptychan {
	my ($channel, $source, $bot, $msg) = @_;
	NeoStats::print ("Empty $channel $source $bot: $msg");
}

sub event_kick {
	my ($channel, $source, $target, $msg) = @_;
	NeoStats::print ("Kick $channel $source $target: $msg");
}

sub event_kickbot {
	my ($channel, $source, $target, $msg) = @_;
	NeoStats::print ("Kickbot $channel $source $target: $msg");
}

sub event_topic {
	my ($channel, $source) = @_;
	NeoStats::print ("Topic $channel $source");
}

sub event_cmode {
	NeoStats::print ("ToDO");
}

sub event_private {
	my ($source, $target, $msg) = @_;
	NeoStats::print ("Privmsg $source $target: $msg");
}

sub event_notice {
	my ($source, $target, $msg) = @_;
	NeoStats::print ("NOTICE $source $target: $msg");
}

sub event_cprivate {
	my ($source, $target, $msg) = @_;
	NeoStats::print ("CPRIVATE $source $target: $msg");
}

sub event_cnotice {
	my ($source, $target, $msg) = @_;
	NeoStats::print ("CNOTICE $source $target: $msg");
}

sub event_globops {
	my ($source, $msg) = @_;
	NeoStats::print ("GLOBOPS $source $msg");
}

sub event_chatops {
	my ($source, $msg) = @_;
	NeoStats::print ("CHATOPS $source: $msg");
}

sub event_wallops {
	my ($source, $msg) = @_;
	NeoStats::print ("WALLOPS $source: $msg");
}

sub event_ctcpversionrpl {
	my ($source, $msg) = @_;
	NeoStats::print ("CTCPVERSIONRPL $source: $msg");
}

sub event_ctcpversionreq {
	my ($source) = @_;
	NeoStats::print ("CTCPVERSIONREQ $source");
}

sub event_ctcpfingerrpl {
	my ($source, $msg) = @_;
	NeoStats::print ("CTCPFINGERRPL $source: $msg");
}

sub event_ctcpfingerreq {
	my ($source) = @_;
	NeoStats::print ("CTCPFINGERREQ $source");
}

sub event_ctcpactionreq {
	my ($source) = @_;
	NeoStats::print ("CTCPACTIONREQ $source");
}

sub event_ctcptimerpl {
	my ($source, $msg) = @_;
	NeoStats::print ("CTCPTIMERPL $source: $msg");
}

sub event_ctcptimereq {
	my ($source) = @_;
	NeoStats::print ("CTCPTIMEREQ $source");
}

sub event_ctcppingrpl {
	my ($source, $msg) = @_;
	NeoStats::print ("CTCPPINGRPL $source: $msg");
}

sub event_ctcppingreq {
	my ($source) = @_;
	NeoStats::print ("CTCPPINGREQ $source");
}

sub event_dccsend {
	my ($source, $msg) = @_;
	NeoStats::print ("DCCSEND $source: $msg");
}

sub event_dccchat {
	my ($source, $msg) = @_;
	NeoStats::print ("DCCCHAT $source: $msg");
}

sub event_dccmsg {
	my ($source, $msg) = @_;
	NeoStats::print ("DCCMSG $source: $msg");
}

sub event_addban {
	NeoStats::print ("AddBan");
}

sub event_delban {
	NeoStats::print ("DelBan");
}



sub setupbot {
	my $botinfo;
	NeoStats::print("Setup");
	$botinfo->{nick} = "Fishy";
	$botinfo->{altnick} = "Fishy2";
	$botinfo->{ident} = "fish";
	$botinfo->{host} = "Host.com";
	$botinfo->{gecos} = "My Gecos";
	NeoStats::AddBot($botinfo, NeoStats::BOT_FLAG_SERVICEBOT);
	$botinfo->{nick} = "fishy2";
	NeoStats::AddBot($botinfo, NeoStats::BOT_FLAG_SERVICEBOT);
	NeoStats::print("Added Second Bot $botinfo->{nick}");
	NeoStats::DelBot($botinfo->{nick});
	
}

sub shutdownbot {
	NeoStats::Print("Shutdown");
}

