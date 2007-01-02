package NeoStats::NV;

use 5.006;
use strict;
use warnings;
use Carp;

require Exporter;
require DynaLoader;

our @ISA = qw(Exporter DynaLoader);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use NeoStats::NV ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(
	
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
);
our $VERSION = '0.01';

bootstrap NeoStats::NV $VERSION;

# Preloaded methods go here.

sub AUTOLOAD {
   my $self = shift;
   my $prop;
   my $val;

   ($prop = $NeoStats::NV::AUTOLOAD) =~ s/.*:://g;
   if (ref $self eq "NeoStats::NV" ) {
      if (exists($self->{$prop})) {
         return $self->{$prop};
      } else {
         $self->rAUTOLOAD($prop, @_);
      }
   }
}

1;
__END__
