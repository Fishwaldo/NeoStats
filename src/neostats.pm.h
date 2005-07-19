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
"NeoStats::print(\"Invalid Number of arguments to register\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my ($package) = caller;\n"
"my $pkg_info = NeoStats::Embed::pkg_info( $package );\n"
"my $filename = $pkg_info->{filename};\n"
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
"\n"
"\n"
"sub hook_event {\n"
"if (@_ < 2) {\n"
"NeoStats::print(\"Invalid Number of arguments to hook_event\");\n"
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
"\n"
"sub AddBot {\n"
"if (@_ < 2) {\n"
"NeoStats::print(\"Invalid Number of arguments to AddBot\");\n"
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
"NeoStats::print(\"Botinfo->{nick} not defined\");\n"
"return NeoStats::NS_FAILURE;\n"
"}    \n"
"if ((!exists( $botinfo->{altnick} )) || (!defined( $botinfo->{altnick} ))) {\n"
"NeoStats::print(\"Botinfo->{altnick} not defined\");\n"
"return NeoStats::NS_FAILURE;\n"
"}    \n"
"if ((!exists( $botinfo->{ident} )) || (!defined( $botinfo->{ident} ))) {\n"
"NeoStats::print(\"Botinfo->{ident} not defined\");\n"
"return NeoStats::NS_FAILURE;\n"
"}    \n"
"if ((!exists( $botinfo->{host} )) || (!defined( $botinfo->{host} ))) {\n"
"NeoStats::print(\"Botinfo->{host} not defined\");\n"
"return NeoStats::NS_FAILURE;\n"
"}    \n"
"if ((!exists( $botinfo->{gecos} )) || (!defined( $botinfo->{gecos} ))) {\n"
"NeoStats::print(\"Botinfo->{gecos} not defined\");\n"
"return NeoStats::NS_FAILURE;\n"
"}    \n"
"\n"
"my $bot =  NeoStats::Internal::AddBot( $botinfo, $botflag, $data);\n"
"if ( defined ( $bot )) {\n"
"return NeoStats::NS_SUCCESS;\n"
"} else {\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"}\n"
"sub DelBot {\n"
"if (@_ < 1) {\n"
"NeoStats::print(\"Invalid Number of arguments to DelBot\");\n"
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
"NeoStats::print(\"Invalid Number of arguments to FindUser\");\n"
"return NeoStats::NS_FAILURE;\n"
"}\n"
"my $nick = shift;\n"
"return NeoStats::Internal::FindUser($nick);\n"
"}\n"
"\n"
"\n"
"\n"
"sub hook_server {\n"
"return undef unless @_ >= 2;\n"
"\n"
"my $message = shift;\n"
"my $callback = shift;\n"
"my $options = shift;\n"
"my ($package) = caller;\n"
"($package) = caller(1) if $package eq 'IRC';\n"
"$callback = NeoStats::Embed::fix_callback( $package, $callback );\n"
"my ($priority, $data) = ( NeoStats::PRI_NORM, undef );\n"
"\n"
"if ( ref( $options ) eq 'HASH' ) {\n"
"if ( exists( $options->{priority} ) && defined( $options->{priority} ) ) {\n"
"$priority = $options->{priority};\n"
"}\n"
"if ( exists( $options->{data} ) && defined( $options->{data} ) ) {\n"
"$data = $options->{data};\n"
"}\n"
"}\n"
"\n"
"my $pkg_info = NeoStats::Embed::pkg_info( $package );\n"
"my $hook =  NeoStats::Internal::hook_server( $message, $priority, $callback, $data);\n"
"push @{$pkg_info->{hooks}}, $hook if defined $hook;\n"
"return $hook;\n"
"\n"
"}\n"
"\n"
"sub hook_command {\n"
"return undef unless @_ >= 2;\n"
"\n"
"my $command = shift;\n"
"my $callback = shift;\n"
"my $options = shift;\n"
"my ($package) = caller;\n"
"($package) = caller(1) if $package eq 'IRC';\n"
"$callback = NeoStats::Embed::fix_callback( $package, $callback );\n"
"my ($priority, $help_text, $data) = ( NeoStats::PRI_NORM, '', undef );\n"
"\n"
"if ( ref( $options ) eq 'HASH' ) {\n"
"if ( exists( $options->{priority} ) && defined( $options->{priority} ) ) {\n"
"$priority = $options->{priority};\n"
"}\n"
"if ( exists( $options->{help_text} ) && defined( $options->{help_text} ) ) {\n"
"$help_text = $options->{help_text};\n"
"}\n"
"if ( exists( $options->{data} ) && defined( $options->{data} ) ) {\n"
"$data = $options->{data};\n"
"}\n"
"}\n"
"\n"
"my $pkg_info = NeoStats::Embed::pkg_info( $package );\n"
"my $hook = NeoStats::Internal::hook_command( $command, $priority, $callback,\n"
"$help_text, $data);\n"
"push @{$pkg_info->{hooks}}, $hook if defined $hook;\n"
"return $hook;\n"
"\n"
"}\n"
"\n"
"sub hook_print {\n"
"return undef unless @_ >= 2;\n"
"\n"
"my $event = shift;\n"
"my $callback = shift;\n"
"my $options = shift;\n"
"my ($package) = caller;\n"
"($package) = caller(1) if $package eq 'IRC';\n"
"$callback = NeoStats::Embed::fix_callback( $package, $callback );\n"
"my ($priority, $data) = ( NeoStats::PRI_NORM, undef );\n"
"\n"
"if ( ref( $options ) eq 'HASH' ) {\n"
"if ( exists( $options->{priority} ) && defined( $options->{priority} ) ) {\n"
"$priority = $options->{priority};\n"
"}\n"
"if ( exists( $options->{data} ) && defined( $options->{data} ) ) {\n"
"$data = $options->{data};\n"
"}\n"
"}\n"
"\n"
"my $pkg_info = NeoStats::Embed::pkg_info( $package );\n"
"my $hook =  NeoStats::Internal::hook_print( $event, $priority, $callback, $data);\n"
"push @{$pkg_info->{hooks}}, $hook if defined $hook;\n"
"return $hook;\n"
"\n"
"}\n"
"\n"
"\n"
"sub hook_timer {\n"
"return undef unless @_ >= 2;\n"
"\n"
"my ($timeout, $callback, $data) = @_;\n"
"my ($package) = caller;\n"
"($package) = caller(1) if $package eq 'IRC';\n"
"$callback = NeoStats::Embed::fix_callback( $package, $callback );\n"
"\n"
"if ( ref( $data ) eq 'HASH' && exists( $data->{data} )\n"
"&& defined( $data->{data} ) ) {\n"
"$data = $data->{data};\n"
"}\n"
"\n"
"my $pkg_info = NeoStats::Embed::pkg_info( $package );\n"
"my $hook = NeoStats::Internal::hook_timer( $timeout, $callback, $data );\n"
"push @{$pkg_info->{hooks}}, $hook if defined $hook;\n"
"return $hook;\n"
"\n"
"}\n"
"\n"
"sub hook_fd {\n"
"return undef unless @_ >= 2;\n"
"my ($fd, $callback, $options) = @_;\n"
"return undef unless defined $fd && defined $callback;\n"
"my $fileno = fileno $fd;\n"
"return undef unless defined $fileno; # no underlying fd for this handle\n"
"\n"
"my ($package) = caller;\n"
"($package) = caller(1) if $package eq 'IRC';\n"
"\n"
"$callback = NeoStats::Embed::fix_callback( $package, $callback );\n"
"\n"
"my ($flags, $data) = (NeoStats::FD_READ, undef);\n"
"\n"
"if( ref( $options ) eq 'HASH' ) {\n"
"if( exists( $options->{flags} ) && defined( $options->{flags} ) ) {\n"
"$flags = $options->{flags};\n"
"}\n"
"if( exists( $options->{data} ) && defined( $options->{data} ) ) {\n"
"$data = $options->{data};\n"
"}\n"
"}\n"
"\n"
"my $cb = sub {\n"
"my $userdata = shift;\n"
"no strict 'refs';\n"
"return &{$userdata->{CB}}($userdata->{FD}, $userdata->{FLAGS},\n"
"$userdata->{DATA},\n"
");\n"
"};\n"
"\n"
"my $pkg_info = NeoStats::Embed::pkg_info( $package );\n"
"my $hook = NeoStats::Internal::hook_fd( $fileno, $cb, $flags,\n"
"{ DATA => $data, FD => $fd, CB => $callback,\n"
"FLAGS => $flags,\n"
"} );\n"
"push @{$pkg_info->{hooks}}, $hook if defined $hook;\n"
"return $hook;\n"
"}\n"
"\n"
"sub unhook {\n"
"my $hook = shift @_;\n"
"my $package = shift @_;\n"
"($package) = caller unless $package;\n"
"my $pkg_info = NeoStats::Embed::pkg_info( $package );\n"
"\n"
"if( $hook =~ /^\\d+$/ && grep { $_ == $hook } @{$pkg_info->{hooks}} ) {\n"
"$pkg_info->{hooks} = [grep { $_ != $hook } @{$pkg_info->{hooks}}];\n"
"return NeoStats::Internal::unhook( $hook );\n"
"}\n"
"\n"
"return ();\n"
"}\n"
"\n"
"sub print {\n"
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
"\n"
"\n"
"if ( @_ >= 1 ) {\n"
"my $channel = shift @_;\n"
"my $server = shift @_;\n"
"my $old_ctx = NeoStats::get_context();\n"
"my $ctx = NeoStats::find_context( $channel, $server );\n"
"\n"
"if ( $ctx ) {\n"
"NeoStats::set_context( $ctx );\n"
"NeoStats::Internal::debug( $text );\n"
"NeoStats::set_context( $old_ctx );\n"
"return 1;\n"
"} else {\n"
"return 0;\n"
"}\n"
"} else {\n"
"NeoStats::Internal::debug( $text );\n"
"return 1;\n"
"}\n"
"\n"
"}\n"
"\n"
"sub printf {\n"
"my $format = shift;\n"
"NeoStats::print( sprintf( $format, @_ ) );\n"
"}\n"
"\n"
"sub command {\n"
"\n"
"my $command = shift;\n"
"my @commands;\n"
"if ( ref( $command ) eq 'ARRAY' ) {\n"
"@commands = @$command;\n"
"} else {\n"
"@commands = ($command);\n"
"}\n"
"if ( @_ >= 1 ) {\n"
"my ($channel, $server) = @_;\n"
"my $old_ctx = NeoStats::get_context();\n"
"my $ctx = NeoStats::find_context( $channel, $server );\n"
"\n"
"if ( $ctx ) {\n"
"NeoStats::set_context( $ctx );\n"
"NeoStats::Internal::command( $_ ) foreach @commands;\n"
"NeoStats::set_context( $old_ctx );\n"
"return 1;\n"
"} else {\n"
"return 0;\n"
"}\n"
"} else {\n"
"NeoStats::Internal::command( $_ ) foreach @commands;\n"
"return 1;\n"
"}\n"
"\n"
"}\n"
"\n"
"sub commandf {\n"
"my $format = shift;\n"
"NeoStats::command( sprintf( $format, @_ ) );\n"
"}\n"
"\n"
"sub set_context {\n"
"my $context;\n"
"\n"
"if ( @_ == 2 ) {\n"
"my ($channel, $server) = @_;\n"
"$context = NeoStats::find_context( $channel, $server );\n"
"} elsif ( @_ == 1 ) {\n"
"if ( $_[0] =~ /^\\d+$/ ) {\n"
"$context = $_[0];\n"
"} else {\n"
"$context = NeoStats::find_context( $_[0] );\n"
"}\n"
"}\n"
"\n"
"return $context ? NeoStats::Internal::set_context( $context ) : 0;\n"
"}\n"
"\n"
"sub get_info {\n"
"my $id = shift;\n"
"my $info;\n"
"\n"
"if ( defined( $id ) ) {\n"
"if ( grep { $id eq $_ } qw(state_cursor) ) {\n"
"$info = NeoStats::get_prefs( $id );\n"
"} else {\n"
"$info = NeoStats::Internal::get_info( $id );\n"
"}\n"
"}\n"
"return $info;\n"
"}\n"
"\n"
"sub user_info {\n"
"my $nick = shift @_ || NeoStats::get_info( \"nick\" );\n"
"my $user;\n"
"\n"
"for (NeoStats::get_list( \"users\" ) ) {\n"
"if ( NeoStats::nickcmp( $_->{nick}, $nick ) == 0 ) {\n"
"$user = $_;\n"
"last;\n"
"}\n"
"}\n"
"return $user;\n"
"}\n"
"\n"
"sub context_info {\n"
"my $ctx = shift @_ || NeoStats::get_context;\n"
"my $old_ctx = NeoStats::get_context;\n"
"my @fields = (qw(away channel host inputbox libdirfs network nick server),\n"
"qw(topic version win_status NeoStatsdir NeoStatsdirfs state_cursor),\n"
");\n"
"\n"
"if (NeoStats::set_context( $ctx )) {\n"
"my %info;\n"
"for my $field ( @fields ) {\n"
"$info{$field} = NeoStats::get_info( $field );\n"
"}\n"
"NeoStats::set_context( $old_ctx );\n"
"\n"
"return %info if wantarray;\n"
"return \\%info;\n"
"} else {\n"
"return undef;\n"
"}\n"
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
"NeoStats::print( $message );\n"
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
"NeoStats::print( \"Too many package defintions, only 1 is allowed\\n\" );\n"
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
"\n"
"{\n"
"\n"
"eval \"package $package; $source;\";\n"
"}\n"
"\n"
"if ( $@ ) {\n"
"\n"
"NeoStats::print( \"Error loading '$file':\\n$@\\n\" );\n"
"\n"
"\n"
"unload( $scripts{$package}{filename} );\n"
"return 1;\n"
"}\n"
"\n"
"} else {\n"
"NeoStats::print( \"Error opening '$file': $!\\n\" );\n"
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
