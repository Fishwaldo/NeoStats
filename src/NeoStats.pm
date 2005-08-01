BEGIN {
  $INC{'NeoStats.pm'} = 'DUMMY';
}

use File::Spec();
use File::Basename();
use Symbol();

{
  package NeoStats;
  use base qw(Exporter);
#  use strict;
  use warnings;
  our %EXPORT_TAGS = ( all => [
			       qw(register hook_server hook_command),
			       qw(hook_print hook_timer unhook print command),
			       qw(find_context get_context set_context),
			       qw(get_info get_prefs emit_print nickcmp),
			       qw(get_list context_info strip_code),
			       qw(EVENT_MODULELOAD EVENT_MODULEUNLOAD),
			       qw(PRI_LOWEST EAT_NONE EAT_NeoStats NS_FAILURE),
			       qw(NS_SUCCESS KEEP REMOVE),
			      ],
		       constants => [
				     qw(EVENT_MODULELOAD EVENT_MODULEUNLOAD PRI_NORM PRI_LOW),
				     qw(PRI_LOWEST EAT_NONE EAT_NeoStats),
				     qw(NS_FAILURE NS_SUCCESS FD_READ FD_WRITE),
				     qw(FD_EXCEPTION FD_NOTSOCKET KEEP REMOVE),
				    ],
		       hooks => [
				 qw(hook_server hook_command),
				 qw(hook_print hook_timer unhook),
				],
		       util => [
				qw(register print command find_context),
				qw(get_context set_context get_info get_prefs),
				qw(emit_print nickcmp get_list context_info),
				qw(strip_code),
			       ],
		     );

  our @EXPORT = @{$EXPORT_TAGS{constants}};
  our @EXPORT_OK = @{$EXPORT_TAGS{all}};

  sub register {
    if (@_ != 5) {
      NeoStats::print("Invalid Number of arguments to register");
      return NeoStats::NS_FAILURE;
    }
    my ($package) = caller;
    my $pkg_info = NeoStats::Embed::pkg_info( $package );
    my $filename = $pkg_info->{filename};

    my ($name, $version, $description, $startupcb, $shutdowncb) = @_;
    $description = "" unless defined $description;


    $pkg_info->{name} = $name;
    $pkg_info->{version} = $version;
    $pkg_info->{description} = $description;
    $pkg_info->{gui_entry} =
      NeoStats::Internal::register( $pkg_info->{name}, $pkg_info->{version}, $pkg_info->{description});
    $startupcb = NeoStats::Embed::fix_callback( $package, $startupcb );
    $shutdowncb = NeoStats::Embed::fix_callback( $package, $shutdowncb );
    $pkg_info->{shutdown} = $shutdowncb;
    $pkg_info->{startup} = $startupcb;

    # keep with old behavior
    return NeoStats::NS_SUCCESS;
  }



  sub hook_event {
    if (@_ < 2) {
      NeoStats::print("Invalid Number of arguments to hook_event");
      return NeoStats::NS_FAILURE;
    }

    my $event = shift;
    my $callback = shift;
    my $options = shift;
    my ($package) = caller;
    my $flags = 0;
    my $data = "";
    $callback = NeoStats::Embed::fix_callback( $package, $callback );
  
    if ( ref( $options ) eq 'HASH' ) {
      if ( exists( $options->{flags} ) && defined( $options->{flags} ) ) {
        $flags = $options->{flags};
      }
      if ( exists( $options->{data} ) && defined( $options->{data} ) ) {
        $data = $options->{data};
      }
    }
    
    my $pkg_info = NeoStats::Embed::pkg_info( $package );
    my $hook =  NeoStats::Internal::hook_event( $event, $flags, $callback, $data);
    if ( defined ( $hook )) {
      push @{$pkg_info->{hooks}}, $event;
      return NeoStats::NS_SUCCESS;
    } else {
      return NeoStats::NS_FAILURE;
    }
  }
  
  sub unhook_event {
    NeoStats::print("todo");
    return NeoStats::NS_FAILURE;
  }


# AddBot(botinfo, botflag)
  sub AddBot {
    if (@_ < 2) {
      NeoStats::print("Invalid Number of arguments to AddBot");
      return NeoStats::NS_FAILURE;
    }

    my $botinfo = shift;
    my $botflag = shift;
    my $data = shift;
    my ($package) = caller;
#    $callback = NeoStats::Embed::fix_callback( $package, $callback );
  
    if (!ref( $botinfo ) eq 'HASH' ) {
      return NeoStats::NS_FAILURE;
    }
    if ((!exists( $botinfo->{nick} )) || (!defined( $botinfo->{nick} ))) {
      NeoStats::print("Botinfo->{nick} not defined");
      return NeoStats::NS_FAILURE;
    }    
    if ((!exists( $botinfo->{altnick} )) || (!defined( $botinfo->{altnick} ))) {
      NeoStats::print("Botinfo->{altnick} not defined");
      return NeoStats::NS_FAILURE;
    }    
    if ((!exists( $botinfo->{ident} )) || (!defined( $botinfo->{ident} ))) {
      NeoStats::print("Botinfo->{ident} not defined");
      return NeoStats::NS_FAILURE;
    }    
    if ((!exists( $botinfo->{host} )) || (!defined( $botinfo->{host} ))) {
      NeoStats::print("Botinfo->{host} not defined");
      return NeoStats::NS_FAILURE;
    }    
    if ((!exists( $botinfo->{gecos} )) || (!defined( $botinfo->{gecos} ))) {
      NeoStats::print("Botinfo->{gecos} not defined");
      return NeoStats::NS_FAILURE;
    }    
    
    my $bot =  NeoStats::Internal::AddBot( $botinfo, $botflag, $data);
    if ( defined ( $bot )) {
      return $bot;
    } else {
      return NeoStats::NS_FAILURE;
    }
  }
  sub DelBot {
    if (@_ < 1) {
      NeoStats::print("Invalid Number of arguments to DelBot");
      return NeoStats::NS_FAILURE;
    }
    my $botname = shift;
    my $reason = shift;
    if (!defined($reason)) {
      $reason = "Unknown";
    }
    return NeoStats::Internal::DelBot($botname, $reason);
  }    

  sub FindUser {
    if (@_ < 1) {
      NeoStats::print("Invalid Number of arguments to FindUser");
      return NeoStats::NS_FAILURE;
    }
    my $nick = shift;
    return NeoStats::Internal::FindUser($nick);
  }

  sub FindServer {
    if (@_ < 1) {
      NeoStats::print("Invalid Number of arguments to FindServer");
      return NeoStats::NS_FAILURE;
    }
    my $name = shift;
    return NeoStats::Internal::FindServer($name);
  }

  sub FindChan {
    if (@_ < 1) {
      NeoStats::print("Invalid Number of arguments to FindChannel");
      return NeoStats::NS_FAILURE;
    }
    my $name = shift;
    return NeoStats::Internal::FindChannel($name);
  }
  
  sub AddCmd {
    if (@_ < 3) {
      NeoStats::print("Invalid Number of arguments to AddCmd");
      return NeoStats::NS_FAILURE;
    }

    my $bot = shift;
    my $botcmd = shift;
    my $callback = shift;
    my $data = shift;
    my ($package) = caller;
    $callback = NeoStats::Embed::fix_callback( $package, $callback );
  
    if (!ref( $botcmd ) eq 'HASH' ) {
      NeoStats::print("Botcmd is not a hash");
      return NeoStats::NS_FAILURE;
    }
    if ((!exists( $botcmd->{cmd} )) || (!defined( $botcmd->{cmd} ))) {
      NeoStats::print("Botinfo->{cmd} not defined");
      return NeoStats::NS_FAILURE;
    }    
    if ((!exists( $botcmd->{minparams} )) || (!defined( $botcmd->{minparams} ))) {
      NeoStats::print("Botinfo->{minparams} not defined");
      return NeoStats::NS_FAILURE;
    }    
    if ((!exists( $botcmd->{ulevel} )) || (!defined( $botcmd->{ulevel} ))) {
      NeoStats::print("Bot->{ulevel} not defined");
      return NeoStats::NS_FAILURE;
    }    
#XXX TODO
#    if ((!exists( $botcmd->{helptext} )) || (!defined( $botcmd->{helptext} ))) {
#      NeoStats::print("Botinfo->{host} not defined");
#      return NeoStats::NS_FAILURE;
#    }    
    if ((!exists( $botcmd->{flags} )) || (!defined( $botcmd->{flags} ))) {
      NeoStats::print("Botinfo->{flags} not defined");
      return NeoStats::NS_FAILURE;
    }    
    my $ret =  NeoStats::Internal::AddCommand( $bot, $botcmd, $callback);
    return $ret;
  }

  sub DelCmd {
    if (@_ < 2) {
      NeoStats::print("Invalid Number of arguments to DelCmd");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $botcmd = shift;
NeoStats::print("$bot $botcmd");
    return NeoStats::Internal::DelCommand($bot, $botcmd);
  }

  sub print {

    my $text = shift @_;
    return 1 unless $text;
    if ( ref( $text ) eq 'ARRAY' ) {
      if ( $, ) {
        $text = join $, , @$text;
      } else {
        $text = join "", @$text;
      }
    }
    NeoStats::Internal::debug( $text );
    return 1;
  }

  sub printf {
    my $format = shift;
    NeoStats::print( sprintf( $format, @_ ) );
  }

  sub strip_code {
    my $pattern =
      qr/\cB| #Bold
       \cC\d{0,2}(?:,\d{0,2})?| #Color
       \cG| #Beep
       \cO| #Reset
       \cV| #Reverse
       \c_  #Underline
      /x;

    if ( defined wantarray ) {
      my $msg = shift;
      $msg =~ s/$pattern//g;
      return $msg;
    } else {
      $_[0] =~ s/$pattern//g;
    }
  }

}

$SIG{__WARN__} = sub {
  my $message = shift @_;
  my ($package) = caller;
  my $pkg_info = NeoStats::Embed::pkg_info( $package );
  
  if( $pkg_info ) {
    $message =~ s/\(eval \d+\)/$pkg_info->{filename}/;
  }
  NeoStats::print( $message );
};

{
  package NeoStats::Embed;
  use strict;
  use warnings;

  # list of loaded scripts keyed by their package names
  our %scripts;
  sub expand_homedir {
    my $file = shift @_;
    
    if( $^O eq "MSWin32" ) {
      $file =~ s/^~/$ENV{USERPROFILE}/;
    } else {
      $file =~
        s{^~}{
          (getpwuid($>))[7] ||  $ENV{HOME} || $ENV{LOGDIR}
        }ex;
    }
    return $file;
  }
  sub file2pkg {
   
    my $string = File::Basename::basename( shift @_ );
    $string =~ s/\.pl$//i;
    $string =~ s|([^A-Za-z0-9/])|'_'.unpack("H*",$1)|eg;

    return "NeoStats::Module::" . $string;
  }

  sub pkg_info {
    my $package = shift @_;
    return $scripts{$package};
  }

  sub fix_callback {
    my ($package, $callback) = @_;

    unless( ref $callback ) {
      # change the package to the correct one in case it was hardcoded
      $callback =~ s/^.*:://;
      $callback = qq[${package}::$callback];
    }
    return $callback;
  }

  sub load {
    my $file = expand_homedir( shift @_ );

    my $package = file2pkg( $file );
# no need for this, as we only load one file per interpreter 
#    print $package;
#    if ( exists $scripts{$package} ) {
#      my $pkg_info = pkg_info( $package );
#      my $filename = File::Basename::basename( $pkg_info->{filename} );
#      NeoStats::print( qq{'$filename' already loaded from '$pkg_info->{filename}'.\n} );
#      NeoStats::print( 'If this is a different script then it rename and try loading it again.' );
#      return 2;
#    }

    if ( open FH, $file ) {
      my $source = do {local $/; <FH>};
      close FH;

      if ( my $replacements = $source =~ s/^\s*package ([\w:]+).*?;//mg ) {
        my $original_package = $1;

        if ( $replacements > 1 ) {
          NeoStats::print( "Too many package defintions, only 1 is allowed\n" );
          return 1;
        }

        # fixes things up for code calling subs with fully qualified names
        $source =~ s/${original_package}:://g;

      }

      # this must come before the eval or the filename will not be found in
      # NeoStats::register
      $scripts{$package}{filename} = $file;

      {
#        no strict; no warnings;
        eval "package $package; $source;";
      }

      if ( $@ ) {
        # something went wrong
        NeoStats::print( "Error loading '$file':\n$@\n" );

        # make sure the script list doesn't contain false information
        unload( $scripts{$package}{filename} );
        return 1;
      }

    } else {
      NeoStats::print( "Error opening '$file': $!\n" );
      return 2;
    }
    
    return 0;
  }

  sub unload {
    my $file = shift @_;
    my $package = file2pkg( $file );
    my $pkg_info = pkg_info( $package );

    if( $pkg_info ) {

      if( exists $pkg_info->{hooks} ) {
        for my $hook ( @{$pkg_info->{hooks}} ) {
          NeoStats::Internal::unhook_event( $hook );
        }
      }

    # take care of the shutdown callback
      if( exists $pkg_info->{shutdown} ) {
        if( ref $pkg_info->{shutdown} eq 'CODE' ) {
          $pkg_info->{shutdown}->();
        } elsif( $pkg_info->{shutdown} ) {
          eval {
            no strict 'refs';
            &{$pkg_info->{shutdown}};
          };
        }
      }
      
      Symbol::delete_package( $package );
      delete $scripts{$package};
      return NeoStats::NS_SUCCESS;
    } else {
      return NeoStats::NS_FAILURE;
    }
  }

  sub reload {
    my $file = shift @_;
    my $package = file2pkg( $file );
    my $pkg_info = pkg_info( $package );
    my $fullpath = $file;

    if( $pkg_info ) {
      $fullpath = $pkg_info->{filename};
      unload( $file );
    }
    load( $fullpath );
    return NeoStats::NS_SUCCESS;
  }

  sub unload_all {
    for my $package ( keys %scripts ) {
      unload( $scripts{$package}->{filename} );
    }
    return NeoStats::NS_SUCCESS;
  }

  sub sync {
    my $file = shift @_;
    my $package = file2pkg( $file );
    my $pkg_info = pkg_info( $package );
    if( exists $pkg_info->{startup} ) {
        if( ref $pkg_info->{startup} eq 'CODE' ) {
          $pkg_info->{startup}->();
        } elsif( $pkg_info->{startup} ) {
          eval {
            no strict 'refs';
            &{$pkg_info->{startup}};
          };
        }
      }
  }    


}
