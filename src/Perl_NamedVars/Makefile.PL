use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.

WriteMakefile(
    'NAME'		=> 'NeoStats::NV',
    'VERSION_FROM'	=> 'NV.pm', # finds $VERSION
    'AUTHOR'     	=> 'Justin Hammond <justin@dynama.c',
    'INC'		=> '-I../../include -I../../lib/event -I../../lib/pcre -I../../lib/curl', 
    # 'OBJECT'		=> '$(O_FILES)', # link all the C files too
);
