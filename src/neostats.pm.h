"BEGIN {\n"
"$INC{'NeoStats.pm'} = 'DUMMY';\n"
"}\n"
"\n"
"use File::Spec();\n"
"use File::Basename();\n"
"use Symbol();\n"
"\n"
"{\n"
"package NeoStats;\n"
"use base qw(Exporter);\n"
"\n"
"use warnings;\n"
"our %EXPORT_TAGS = ( all => [\n"
"qw(register hook_server hook_command),\n"
"qw(hook_print hook_timer unhook print command),\n"
"qw(find_context get_context set_context),\n"
"qw(get_info get_prefs emit_print nickcmp),\n"
"qw(get_list context_info strip_code),\n"
"qw(EVENT_MODULELOAD EVENT_MODULEUNLOAD),\n"
"qw(PRI_LOWEST EAT_NONE EAT_NeoStats NS_FAILURE),\n"
"qw(NS_SUCCESS KEEP REMOVE),\n"
"],\n"
"constants => [\n"
"qw(EVENT_MODULELOAD EVENT_MODULEUNLOAD PRI_NORM PRI_LOW),\n"
"qw(PRI_LOWEST EAT_NONE EAT_NeoStats),\n"
"qw(NS_FAILURE NS_SUCCESS FD_READ FD_WRITE),\n"
"qw(FD_EXCEPTION FD_NOTSOCKET KEEP REMOVE),\n"
"],\n"
"hooks => [\n"
"qw(hook_server hook_command),\n"
"qw(hook_print hook_timer unhook),\n"
"],\n"
"util => [\n"
"qw(register print command find_context),\n"
"qw(get_context set_context get_info get_prefs),\n"
"qw(emit_print nickcmp get_list context_info),\n"
"qw(strip_code),\n"
"],\n"
");\n"
"\n"
"our @EXPORT = @{$EXPORT_TAGS{constants}};\n"
"our @EXPORT_OK = @{$EXPORT_TAGS{all}};\n"
"\n"
"sub register {\n"
"if (@_ != 5) {\n"
"NeoStats::debug(\"Invalid Number of arguments to register\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my ($package) = caller;\n"
"my $pkg_info = NeoStats::Embed::pkg_info( $package );\n"
"my $filename = $pkg_info->{filename};\n"
"\n"
"if ($pkg_info->{type} != 0) {\n"
"NeoStats::debug(\"Extension tried to register as a module\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"\n"
"my ($name, $version, $description, $startupcb, $shutdowncb) = @_;\n"
"$description = \"\" unless defined $description;\n"
"\n"
"\n"
"$pkg_info->{name} = $name;\n"
"$pkg_info->{version} = $version;\n"
"$pkg_info->{description} = $description;\n"
"$pkg_info->{gui_entry} =\n"
"NeoStats::Internal::register( $pkg_info->{name}, $pkg_info->{version}, $pkg_info->{description});\n"
"$startupcb = NeoStats::Embed::fix_callback( $package, $startupcb );\n"
"$shutdowncb = NeoStats::Embed::fix_callback( $package, $shutdowncb );\n"
"$pkg_info->{shutdown} = $shutdowncb;\n"
"$pkg_info->{startup} = $startupcb;\n"
"\n"
"\n"
"return NeoStats::NS_SUCCESS;\n"
"}\n"
"\n"
"sub registerextension {\n"
"if (@_ != 4) {\n"
"NeoStats::debug(\"Invalid Number of arguments to registerextension\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my ($package) = caller;\n"
"my $pkg_info = NeoStats::Embed::pkg_info( $package );\n"
"my $filename = $pkg_info->{filename};\n"
"\n"
"if ($pkg_info->{type} != 1) {\n"
"NeoStats::debug(\"Perl Module tried to register as a extension\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"\n"
"my ($name, $version, $startupcb, $shutdowncb) = @_;\n"
"$pkg_info->{name} = $name;\n"
"$pkg_info->{version} = $version;\n"
"$pkg_info->{gui_entry} =\n"
"NeoStats::Internal::registerextension( $pkg_info->{name}, $pkg_info->{version});\n"
"$startupcb = NeoStats::Embed::fix_callback( $package, $startupcb );\n"
"$shutdowncb = NeoStats::Embed::fix_callback( $package, $shutdowncb );\n"
"$pkg_info->{shutdown} = $shutdowncb;\n"
"$pkg_info->{startup} = $startupcb;\n"
"\n"
"\n"
"return NeoStats::NS_SUCCESS;\n"
"}\n"
"\n"
"\n"
"sub hook_event {\n"
"if (@_ < 2) {\n"
"NeoStats::debug(\"Invalid Number of arguments to hook_event\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"\n"
"my $event = shift;\n"
"my $callback = shift;\n"
"my $options = shift;\n"
"my ($package) = caller;\n"
"my $flags = 0;\n"
"my $data = \"\";\n"
"$callback = NeoStats::Embed::fix_callback( $package, $callback );\n"
"\n"
"if ( ref( $options ) eq 'HASH' ) {\n"
"if ( exists( $options->{flags} ) && defined( $options->{flags} ) ) {\n"
"$flags = $options->{flags};\n"
"}\n"
"if ( exists( $options->{data} ) && defined( $options->{data} ) ) {\n"
"$data = $options->{data};\n"
"}\n"
"}\n"
"\n"
"my $pkg_info = NeoStats::Embed::pkg_info( $package );\n"
"my $hook =  NeoStats::Internal::hook_event( $event, $flags, $callback, $data);\n"
"if ( defined ( $hook )) {\n"
"push @{$pkg_info->{hooks}}, $event;\n"
"return NeoStats::NS_SUCCESS;\n"
"} else {\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"}\n"
"\n"
"sub unhook_event {\n"
"NeoStats::debug(\"todo\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"\n"
"\n"
"\n"
"sub AddBot {\n"
"if (@_ < 2) {\n"
"NeoStats::debug(\"Invalid Number of arguments to AddBot\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"\n"
"my $botinfo = shift;\n"
"my $botflag = shift;\n"
"my $data = shift;\n"
"my ($package) = caller;\n"
"\n"
"\n"
"if (!ref( $botinfo ) eq 'HASH' ) {\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"if ((!exists( $botinfo->{nick} )) || (!defined( $botinfo->{nick} ))) {\n"
"NeoStats::debug(\"Botinfo->{nick} not defined\");\n"
"return NeoStats::NS_FAILURE;\n"
"}    \n"
"if ((!exists( $botinfo->{altnick} )) || (!defined( $botinfo->{altnick} ))) {\n"
"NeoStats::debug(\"Botinfo->{altnick} not defined\");\n"
"return NeoStats::NS_FAILURE;\n"
"}    \n"
"if ((!exists( $botinfo->{ident} )) || (!defined( $botinfo->{ident} ))) {\n"
"NeoStats::debug(\"Botinfo->{ident} not defined\");\n"
"return NeoStats::NS_FAILURE;\n"
"}    \n"
"if ((!exists( $botinfo->{host} )) || (!defined( $botinfo->{host} ))) {\n"
"NeoStats::debug(\"Botinfo->{host} not defined\");\n"
"return NeoStats::NS_FAILURE;\n"
"}    \n"
"if ((!exists( $botinfo->{gecos} )) || (!defined( $botinfo->{gecos} ))) {\n"
"NeoStats::debug(\"Botinfo->{gecos} not defined\");\n"
"return NeoStats::NS_FAILURE;\n"
"}    \n"
"\n"
"my $bot =  NeoStats::Internal::AddBot( $botinfo, $botflag, $data);\n"
"if ( defined ( $bot )) {\n"
"return $bot;\n"
"} else {\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"}\n"
"sub DelBot {\n"
"if (@_ < 1) {\n"
"NeoStats::debug(\"Invalid Number of arguments to DelBot\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $botname = shift;\n"
"my $reason = shift;\n"
"if (!defined($reason)) {\n"
"$reason = \"Unknown\";\n"
"}\n"
"return NeoStats::Internal::DelBot($botname, $reason);\n"
"}    \n"
"\n"
"sub FindUser {\n"
"if (@_ < 1) {\n"
"NeoStats::debug(\"Invalid Number of arguments to FindUser\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $nick = shift;\n"
"return NeoStats::Internal::FindUser($nick);\n"
"}\n"
"\n"
"sub FindServer {\n"
"if (@_ < 1) {\n"
"NeoStats::debug(\"Invalid Number of arguments to FindServer\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $name = shift;\n"
"return NeoStats::Internal::FindServer($name);\n"
"}\n"
"\n"
"sub FindChan {\n"
"if (@_ < 1) {\n"
"NeoStats::debug(\"Invalid Number of arguments to FindChannel\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $name = shift;\n"
"return NeoStats::Internal::FindChannel($name);\n"
"}\n"
"\n"
"sub AddCmd {\n"
"if (@_ < 3) {\n"
"NeoStats::debug(\"Invalid Number of arguments to AddCmd\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"\n"
"my $bot = shift;\n"
"my $botcmd = shift;\n"
"my $callback = shift;\n"
"my $data = shift;\n"
"my ($package) = caller;\n"
"$callback = NeoStats::Embed::fix_callback( $package, $callback );\n"
"\n"
"if (!ref( $botcmd ) eq 'HASH' ) {\n"
"NeoStats::debug(\"Botcmd is not a hash\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"if ((!exists( $botcmd->{cmd} )) || (!defined( $botcmd->{cmd} ))) {\n"
"NeoStats::debug(\"Botinfo->{cmd} not defined\");\n"
"return NeoStats::NS_FAILURE;\n"
"}    \n"
"if ((!exists( $botcmd->{minparams} )) || (!defined( $botcmd->{minparams} ))) {\n"
"NeoStats::debug(\"Botinfo->{minparams} not defined\");\n"
"return NeoStats::NS_FAILURE;\n"
"}    \n"
"if ((!exists( $botcmd->{ulevel} )) || (!defined( $botcmd->{ulevel} ))) {\n"
"NeoStats::debug(\"Botinfo->{ulevel} not defined\");\n"
"return NeoStats::NS_FAILURE;\n"
"}    \n"
"if ((!exists( $botcmd->{helptext} )) || (!defined( $botcmd->{helptext} ))) {\n"
"NeoStats::debug(\"Botinfo->{helptext} not defined\");\n"
"return NeoStats::NS_FAILURE;\n"
"}    \n"
"\n"
"if ((!exists( $botcmd->{flags} )) || (!defined( $botcmd->{flags} ))) {\n"
"NeoStats::debug(\"Botinfo->{flags} not defined\");\n"
"return NeoStats::NS_FAILURE;\n"
"}    \n"
"my $ret =  NeoStats::Internal::AddCommand( $bot, $botcmd, $callback);\n"
"return $ret;\n"
"}\n"
"\n"
"sub DelCmd {\n"
"if (@_ < 2) {\n"
"NeoStats::debug(\"Invalid Number of arguments to DelCmd\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $botcmd = shift;\n"
"return NeoStats::Internal::DelCommand($bot, $botcmd);\n"
"}\n"
"\n"
"sub PrefMsg {\n"
"if (@_ < 3) {\n"
"NeoStats::debug(\"Invalid Number of arguments to PreMsg\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $target = shift;\n"
"my $message = shift;\n"
"return NeoStats::Internal::Prefmsg($bot, $target, $message);\n"
"}\n"
"\n"
"sub ChanAlert {\n"
"if (@_ < 2) {\n"
"NeoStats::debug(\"Invalid Number of arguments to ChanAlert\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $message = shift;\n"
"return NeoStats::Internal::ChanAlert($bot, $message);\n"
"}\n"
"\n"
"sub PrivMsg {\n"
"if (@_ < 3) {\n"
"NeoStats::debug(\"Invalid Number of arguments to PrivMsg\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $to = shift;\n"
"my $message = shift;\n"
"return NeoStats::Internal::PrivMsg($bot, $to, $message);\n"
"}\n"
"\n"
"sub Notice {\n"
"if (@_ < 3) {\n"
"NeoStats::debug(\"Invalid Number of arguments to Notice\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $to = shift;\n"
"my $message = shift;\n"
"return NeoStats::Internal::Notice($bot, $to, $message);\n"
"}\n"
"\n"
"sub ChanPrivMsg {\n"
"if (@_ < 3) {\n"
"NeoStats::debug(\"Invalid Number of arguments to ChanPrivMsg\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $to = shift;\n"
"my $message = shift;\n"
"return NeoStats::Internal::ChanPrivMsg($bot, $to, $message);\n"
"}\n"
"\n"
"sub ChanNotice {\n"
"if (@_ < 3) {\n"
"NeoStats::debug(\"Invalid Number of arguments to ChanNotice\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $to = shift;\n"
"my $message = shift;\n"
"return NeoStats::Internal::ChanNotice($bot, $to, $message);\n"
"}\n"
"\n"
"sub Globops {\n"
"if (@_ < 2) {\n"
"NeoStats::debug(\"Invalid Number of arguments to Globops\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $message = shift;\n"
"return NeoStats::Internal::Globops($bot, $message);\n"
"}\n"
"\n"
"sub Wallops {\n"
"if (@_ < 2) {\n"
"NeoStats::debug(\"Invalid Number of arguments to Wallops\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $message = shift;\n"
"return NeoStats::Internal::Globops($bot, $message);\n"
"}\n"
"\n"
"sub Numeric {\n"
"if (@_ < 3) {\n"
"NeoStats::debug(\"Invalid Number of arguments to Numeric\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $numeric = shift;\n"
"my $target = shift;\n"
"my $message = shift;\n"
"return NeoStats::Internal::Numeric($numeric, $target, $message);\n"
"}\n"
"\n"
"sub Umode {\n"
"if (@_ < 3) {\n"
"NeoStats::debug(\"Invalid Number of arguments to Umode\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $target = shift;\n"
"my $umode = shift;\n"
"return NeoStats::Internal::Umode($bot, $target, $umode);\n"
"}\n"
"\n"
"sub Join {\n"
"if (@_ < 3) {\n"
"NeoStats::debug(\"Invalid Number of arguments to Cmode\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $channel = shift;\n"
"my $cmode = shift;\n"
"return NeoStats::Internal::Join($bot, $channel, $cmode);\n"
"}\n"
"\n"
"sub Part {\n"
"if (@_ < 2) {\n"
"NeoStats::debug(\"Invalid Number of arguments to Part\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $channel = shift;\n"
"my $message = shift;\n"
"return NeoStats::Internal::Part($bot, $channel, $message);\n"
"}\n"
"\n"
"sub NickChange {\n"
"if (@_ < 2) {\n"
"NeoStats::debug(\"Invalid Number of arguments to NickChange\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $newnick = shift;\n"
"return NeoStats::Internal::NickChange($bot, $newnick);\n"
"}\n"
"\n"
"sub CMode {\n"
"if (@_ < 3) {\n"
"NeoStats::debug(\"Invalid Number of arguments to CMode\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $chan = shift;\n"
"my $modes = shift;\n"
"my $args = shift;\n"
"return NeoStats::Internal::CMode($bot, $chan, $modes, $args);\n"
"}\n"
"\n"
"sub ChanUserMode {\n"
"if (@_ < 4) {\n"
"NeoStats::debug(\"Invalid Number of arguments to ChanUserMode\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $chan = shift;\n"
"my $modes = shift;\n"
"my $target = shift;\n"
"return NeoStats::Internal::ChanUserMode($bot, $chan, $modes, $target);\n"
"}\n"
"\n"
"sub Kill {\n"
"if (@_ < 3) {\n"
"NeoStats::debug(\"Invalid Number of arguments to Kill\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $target = shift;\n"
"my $reason = shift;\n"
"return NeoStats::Internal::Kill($bot, $target, $reason);\n"
"}\n"
"\n"
"sub Kick {\n"
"if (@_ < 3) {\n"
"NeoStats::debug(\"Invalid Number of arguments to Kick\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $chan = shift;\n"
"my $target = shift;\n"
"my $reason = shift;\n"
"return NeoStats::Internal::Kick($bot, $chan, $target, $reason);\n"
"}\n"
"\n"
"sub Invite {\n"
"if (@_ < 3) {\n"
"NeoStats::debug(\"Invalid Number of arguments to Invite\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $chan = shift;\n"
"my $target = shift;\n"
"return NeoStats::Internal::Invite($bot, $chan, $target);\n"
"}\n"
"\n"
"sub Topic {\n"
"if (@_ < 3) {\n"
"NeoStats::debug(\"Invalid Number of arguments to Topic\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $chan = shift;\n"
"my $topic = shift;\n"
"return NeoStats::Internal::Topic($bot, $chan, $topic);\n"
"}\n"
"\n"
"sub SvsKill {\n"
"if (@_ < 3) {\n"
"NeoStats::debug(\"Invalid Number of arguments to SvsKill\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $target = shift;\n"
"my $reason = shift;\n"
"return NeoStats::Internal::SvsKill($bot, $target, $reason);\n"
"}\n"
"\n"
"sub SvsMode {\n"
"if (@_ < 3) {\n"
"NeoStats::debug(\"Invalid Number of arguments to SvsMode\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $target = shift;\n"
"my $mode = shift;\n"
"return NeoStats::Internal::SvsKill($bot, $target, $mode);\n"
"}\n"
"\n"
"sub SvsHost {\n"
"if (@_ < 3) {\n"
"NeoStats::debug(\"Invalid Number of arguments to SvsHost\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $target = shift;\n"
"my $host = shift;\n"
"return NeoStats::Internal::SvsHost($bot, $target, $host);\n"
"}\n"
"\n"
"sub SvsJoin {\n"
"if (@_ < 3) {\n"
"NeoStats::debug(\"Invalid Number of arguments to SvsJoin\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $target = shift;\n"
"my $chan = shift;\n"
"return NeoStats::Internal::SvsJoin($bot, $target, $chan);\n"
"}\n"
"\n"
"sub SvsPart {\n"
"if (@_ < 3) {\n"
"NeoStats::debug(\"Invalid Number of arguments to SvsPart\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $target = shift;\n"
"my $chan = shift;\n"
"return NeoStats::Internal::SvsPart($bot, $target, $chan);\n"
"}\n"
"\n"
"sub Swhois {\n"
"if (@_ < 2) {\n"
"NeoStats::debug(\"Invalid Number of arguments to Swhois\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $target = shift;\n"
"my $swhois = shift;\n"
"return NeoStats::Internal::Swhois($target, $swhois);\n"
"}\n"
"\n"
"sub SvsNick {\n"
"if (@_ < 3) {\n"
"NeoStats::debug(\"Invalid Number of arguments to SvsNick\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $target = shift;\n"
"my $newnick = shift;\n"
"return NeoStats::Internal::SvsNick($bot, $target, $newnick);\n"
"}\n"
"\n"
"sub SMO {\n"
"if (@_ < 3) {\n"
"NeoStats::debug(\"Invalid Number of arguments to SMO\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $umodetarget = shift;\n"
"my $message = shift;\n"
"return NeoStats::Internal::SMO($bot, $umodetarget, $message);\n"
"}\n"
"\n"
"sub Akill {\n"
"if (@_ < 5) {\n"
"NeoStats::debug(\"Invalid Number of arguments to Akill\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $host = shift;\n"
"my $ident = shift;\n"
"my $length = shift;\n"
"my $message = shift;\n"
"return NeoStats::Internal::Akill($bot, $host, $ident, $length, $message);\n"
"}\n"
"\n"
"sub Rakill {\n"
"if (@_ < 3) {\n"
"NeoStats::debug(\"Invalid Number of arguments to Rakill\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $bot = shift;\n"
"my $host = shift;\n"
"my $ident = shift;\n"
"return NeoStats::Internal::Rakill($bot, $host, $ident);\n"
"}\n"
"\n"
"sub AddTimer {\n"
"if (@_ < 4) {\n"
"NeoStats::debug(\"Invalid Number of arguments to AddTimer\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $type = shift;\n"
"my $name = shift;\n"
"my $interval = shift;\n"
"my $callback = shift;\n"
"my ($package) = caller;\n"
"$callback = NeoStats::Embed::fix_callback( $package, $callback );\n"
"NeoStats::debug(\"Callback is $callback\");\n"
"return NeoStats::Internal::AddTimer($type, $name, $interval, $callback);\n"
"}\n"
"\n"
"sub DelTimer {\n"
"if (@_ < 1) {\n"
"NeoStats::debug(\"Invalid Number of arguments to DelTimer\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $name = shift;\n"
"return NeoStats::Internal::DelTimer($name);\n"
"}\n"
"\n"
"sub debug {\n"
"\n"
"my $text = shift @_;\n"
"return 1 unless $text;\n"
"if ( ref( $text ) eq 'ARRAY' ) {\n"
"if ( $, ) {\n"
"$text = join $, , @$text;\n"
"} else {\n"
"$text = join \"\", @$text;\n"
"}\n"
"}\n"
"NeoStats::Internal::debug( $text );\n"
"return 1;\n"
"}\n"
"\n"
"sub printf {\n"
"my $format = shift;\n"
"NeoStats::debug( sprintf( $format, @_ ) );\n"
"}\n"
"\n"
"sub strip_code {\n"
"my $pattern =\n"
"qr/\\cB| #Bold\n"
"\\cC\\d{0,2}(?:,\\d{0,2})?| #Color\n"
"\\cG| #Beep\n"
"\\cO| #Reset\n"
"\\cV| #Reverse\n"
"\\c_  #Underline\n"
"/x;\n"
"\n"
"if ( defined wantarray ) {\n"
"my $msg = shift;\n"
"$msg =~ s/$pattern//g;\n"
"return $msg;\n"
"} else {\n"
"$_[0] =~ s/$pattern//g;\n"
"}\n"
"}\n"
"\n"
"}\n"
"\n"
"$SIG{__WARN__} = sub {\n"
"my $message = shift @_;\n"
"my ($package) = caller;\n"
"my $pkg_info = NeoStats::Embed::pkg_info( $package );\n"
"\n"
"if( $pkg_info ) {\n"
"$message =~ s/\\(eval \\d+\\)/$pkg_info->{filename}/;\n"
"}\n"
"NeoStats::debug( $message );\n"
"};\n"
"\n"
"{\n"
"package NeoStats::Embed;\n"
"use strict;\n"
"use warnings;\n"
"\n"
"\n"
"our %scripts;\n"
"sub expand_homedir {\n"
"my $file = shift @_;\n"
"\n"
"if( $^O eq \"MSWin32\" ) {\n"
"$file =~ s/^~/$ENV{USERPROFILE}/;\n"
"} else {\n"
"$file =~\n"
"s{^~}{\n"
"(getpwuid($>))[7] ||  $ENV{HOME} || $ENV{LOGDIR}\n"
"}ex;\n"
"}\n"
"return $file;\n"
"}\n"
"sub file2pkg {\n"
"\n"
"my $string = File::Basename::basename( shift @_ );\n"
"$string =~ s/\\.pl$//i;\n"
"$string =~ s|([^A-Za-z0-9/])|'_'.unpack(\"H*\",$1)|eg;\n"
"\n"
"return \"NeoStats::Module::\" . $string;\n"
"}\n"
"\n"
"sub pkg_info {\n"
"my $package = shift @_;\n"
"return $scripts{$package};\n"
"}\n"
"\n"
"sub fix_callback {\n"
"my ($package, $callback) = @_;\n"
"\n"
"unless( ref $callback ) {\n"
"\n"
"$callback =~ s/^.*:://;\n"
"$callback = qq[${package}::$callback];\n"
"}\n"
"return $callback;\n"
"}\n"
"\n"
"sub load {\n"
"my $file = expand_homedir( shift @_ );\n"
"\n"
"my $package = file2pkg( $file );\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"if ( open FH, $file ) {\n"
"my $source = do {local $/; <FH>};\n"
"close FH;\n"
"\n"
"if ( my $replacements = $source =~ s/^\\s*package ([\\w:]+).*?;//mg ) {\n"
"my $original_package = $1;\n"
"\n"
"if ( $replacements > 1 ) {\n"
"NeoStats::debug( \"Too many package defintions, only 1 is allowed\\n\" );\n"
"return 1;\n"
"}\n"
"\n"
"\n"
"$source =~ s/${original_package}:://g;\n"
"\n"
"}\n"
"\n"
"\n"
"\n"
"$scripts{$package}{filename} = $file;\n"
"$scripts{$package}{type} = 0;\n"
"\n"
"{\n"
"\n"
"eval \"package $package; $source;\";\n"
"}\n"
"\n"
"if ( $@ ) {\n"
"\n"
"NeoStats::debug( \"Error loading '$file':\\n$@\\n\" );\n"
"\n"
"\n"
"unload( $scripts{$package}{filename} );\n"
"return 1;\n"
"}\n"
"\n"
"} else {\n"
"NeoStats::debug( \"Error opening '$file': $!\\n\" );\n"
"return 2;\n"
"}\n"
"\n"
"return 0;\n"
"}\n"
"\n"
"\n"
"sub loadextension {\n"
"my $file = expand_homedir( shift @_ );\n"
"\n"
"my $package = file2pkg( $file );\n"
"\n"
"if ( open FH, $file ) {\n"
"my $source = do {local $/; <FH>};\n"
"close FH;\n"
"\n"
"if ( my $replacements = $source =~ s/^\\s*package ([\\w:]+).*?;//mg ) {\n"
"my $original_package = $1;\n"
"\n"
"if ( $replacements > 1 ) {\n"
"NeoStats::debug( \"Too many package defintions, only 1 is allowed\\n\" );\n"
"return 1;\n"
"}\n"
"\n"
"\n"
"$source =~ s/${original_package}:://g;\n"
"\n"
"}\n"
"\n"
"\n"
"\n"
"$scripts{$package}{filename} = $file;\n"
"$scripts{$package}{type} = 1;\n"
"\n"
"{\n"
"\n"
"eval \"package $package; $source;\";\n"
"}\n"
"\n"
"if ( $@ ) {\n"
"\n"
"NeoStats::debug( \"Error loading extension '$file':\\n$@\\n\" );\n"
"\n"
"\n"
"unload( $scripts{$package}{filename} );\n"
"return 1;\n"
"}\n"
"\n"
"} else {\n"
"NeoStats::debug( \"Error opening '$file': $!\\n\" );\n"
"return 2;\n"
"}\n"
"\n"
"return 0;\n"
"}\n"
"\n"
"sub unload {\n"
"my $file = shift @_;\n"
"my $package = file2pkg( $file );\n"
"my $pkg_info = pkg_info( $package );\n"
"\n"
"if( $pkg_info ) {\n"
"\n"
"if( exists $pkg_info->{hooks} ) {\n"
"for my $hook ( @{$pkg_info->{hooks}} ) {\n"
"NeoStats::Internal::unhook_event( $hook );\n"
"}\n"
"}\n"
"\n"
"\n"
"if( exists $pkg_info->{shutdown} ) {\n"
"if( ref $pkg_info->{shutdown} eq 'CODE' ) {\n"
"$pkg_info->{shutdown}->();\n"
"} elsif( $pkg_info->{shutdown} ) {\n"
"eval {\n"
"no strict 'refs';\n"
"&{$pkg_info->{shutdown}};\n"
"};\n"
"}\n"
"}\n"
"\n"
"Symbol::delete_package( $package );\n"
"delete $scripts{$package};\n"
"return NeoStats::NS_SUCCESS;\n"
"} else {\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"}\n"
"\n"
"sub reload {\n"
"my $file = shift @_;\n"
"my $package = file2pkg( $file );\n"
"my $pkg_info = pkg_info( $package );\n"
"my $fullpath = $file;\n"
"\n"
"if( $pkg_info ) {\n"
"$fullpath = $pkg_info->{filename};\n"
"unload( $file );\n"
"}\n"
"load( $fullpath );\n"
"return NeoStats::NS_SUCCESS;\n"
"}\n"
"\n"
"sub unload_all {\n"
"for my $package ( keys %scripts ) {\n"
"unload( $scripts{$package}->{filename} );\n"
"}\n"
"return NeoStats::NS_SUCCESS;\n"
"}\n"
"\n"
"sub sync {\n"
"my $file = shift @_;\n"
"my $package = file2pkg( $file );\n"
"my $pkg_info = pkg_info( $package );\n"
"if( exists $pkg_info->{startup} ) {\n"
"if( ref $pkg_info->{startup} eq 'CODE' ) {\n"
"$pkg_info->{startup}->();\n"
"} elsif( $pkg_info->{startup} ) {\n"
"eval {\n"
"no strict 'refs';\n"
"&{$pkg_info->{startup}};\n"
"};\n"
"}\n"
"}\n"
"}    \n"
"\n"
"\n"
"}\n"
