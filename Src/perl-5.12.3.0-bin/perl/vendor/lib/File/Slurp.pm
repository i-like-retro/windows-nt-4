package File::Slurp;

my $printed ;

use strict;

use Carp ;
use Exporter ;
use Fcntl qw( :DEFAULT ) ;
use POSIX qw( :fcntl_h ) ;
use Symbol ;
use UNIVERSAL ;

use vars qw( @ISA %EXPORT_TAGS @EXPORT_OK $VERSION @EXPORT ) ;
@ISA = qw( Exporter ) ;

%EXPORT_TAGS = ( 'all' => [
	qw( read_file write_file overwrite_file append_file read_dir ) ] ) ;

@EXPORT = ( @{ $EXPORT_TAGS{'all'} } );
@EXPORT_OK = qw( slurp prepend_file ) ;

$VERSION = '9999.16';

my $max_fast_slurp_size = 1024 * 100 ;

my $is_win32 = $^O =~ /win32/i ;

# Install subs for various constants that aren't set in older perls
# (< 5.005).  Fcntl on old perls uses Exporter to define subs without a
# () prototype These can't be overridden with the constant pragma or
# we get a prototype mismatch.  Hence this less than aesthetically
# appealing BEGIN block:

BEGIN {
	unless( defined &SEEK_SET ) {
		*SEEK_SET = sub { 0 };
		*SEEK_CUR = sub { 1 };
		*SEEK_END = sub { 2 };
	}

	unless( defined &O_BINARY ) {
		*O_BINARY = sub { 0 };
		*O_RDONLY = sub { 0 };
		*O_WRONLY = sub { 1 };
	}

	unless ( defined &O_APPEND ) {

		if ( $^O =~ /olaris/ ) {
			*O_APPEND = sub { 8 };
			*O_CREAT = sub { 256 };
			*O_EXCL = sub { 1024 };
		}
		elsif ( $^O =~ /inux/ ) {
			*O_APPEND = sub { 1024 };
			*O_CREAT = sub { 64 };
			*O_EXCL = sub { 128 };
		}
		elsif ( $^O =~ /BSD/i ) {
			*O_APPEND = sub { 8 };
			*O_CREAT = sub { 512 };
			*O_EXCL = sub { 2048 };
		}
	}
}

# print "OS [$^O]\n" ;

# print "O_BINARY = ", O_BINARY(), "\n" ;
# print "O_RDONLY = ", O_RDONLY(), "\n" ;
# print "O_WRONLY = ", O_WRONLY(), "\n" ;
# print "O_APPEND = ", O_APPEND(), "\n" ;
# print "O_CREAT   ", O_CREAT(), "\n" ;
# print "O_EXCL   ", O_EXCL(), "\n" ;


*slurp = \&read_file ;

sub read_file {

	my $file_name = shift ;
	my $opts = ( ref $_[0] eq 'HASH' ) ? shift : { @_ } ;

	if ( !ref $file_name && 0 &&
	     -e $file_name && -s _ < $max_fast_slurp_size && ! %{$opts} && !wantarray ) {

		local( *FH ) ;

		unless( open( FH, $file_name ) ) {

			@_ = ( $opts, "read_file '$file_name' - sysopen: $!");
			goto &_error ;
		}

		my $read_cnt = sysread( FH, my $buf, -s _ ) ;

		unless ( defined $read_cnt ) {

# handle the read error

			@_ = ( $opts,
				"read_file '$file_name' - small sysread: $!");
			goto &_error ;
		}

		return $buf ;
	}

# set the buffer to either the passed in one or ours and init it to the null
# string

	my $buf ;
	my $buf_ref = $opts->{'buf_ref'} || \$buf ;
	${$buf_ref} = '' ;

	my( $read_fh, $size_left, $blk_size ) ;

# deal with ref for a file name
# it could be an open handle or an overloaded object

	if ( ref $file_name ) {

		my $ref_result = _check_ref( $file_name ) ;

		if ( ref $ref_result ) {

# we got an error, deal with it

			@_ = ( $opts, $ref_result ) ;
			goto &_error ;
		}

		if ( $ref_result ) {

# we got an overloaded object and the result is the stringified value
# use it as the file name

			$file_name = $ref_result ;
		}
		else {

# here we have just an open handle. set $read_fh so we don't do a sysopen

			$read_fh = $file_name ;
			$blk_size = $opts->{'blk_size'} || 1024 * 1024 ;
			$size_left = $blk_size ;
		}
	}

# see if we have a path we need to open

	unless ( $read_fh ) {

# a regular file. set the sysopen mode

		my $mode = O_RDONLY ;

#printf "RD: BINARY %x MODE %x\n", O_BINARY, $mode ;

# open the file and handle any error

		$read_fh = gensym ;
		unless ( sysopen( $read_fh, $file_name, $mode ) ) {
			@_ = ( $opts, "read_file '$file_name' - sysopen: $!");
			goto &_error ;
		}

		if ( my $binmode = $opts->{'binmode'} ) {
			binmode( $read_fh, $binmode ) ;
		}

# get the size of the file for use in the read loop

		$size_left = -s $read_fh ;

#print "SIZE $size_left\n" ;


# we need a blk_size if the size is 0 so we can handle pseudofiles like in
# /proc. these show as 0 size but have data to be slurped.

		unless( $size_left ) {

			$blk_size = $opts->{'blk_size'} || 1024 * 1024 ;
			$size_left = $blk_size ;
		}
	}


# 	if ( $size_left < 10000 && keys %{$opts} == 0 && !wantarray ) {

# #print "OPT\n" and $printed++ unless $printed ;

# 		my $read_cnt = sysread( $read_fh, my $buf, $size_left ) ;

# 		unless ( defined $read_cnt ) {

# # handle the read error

# 			@_ = ( $opts, "read_file '$file_name' - small2 sysread: $!");
# 			goto &_error ;
# 		}

# 		return $buf ;
# 	}

# infinite read loop. we exit when we are done slurping

	while( 1 ) {

# do the read and see how much we got

		my $read_cnt = sysread( $read_fh, ${$buf_ref},
				$size_left, length ${$buf_ref} ) ;

		unless ( defined $read_cnt ) {

# handle the read error

			@_ = ( $opts, "read_file '$file_name' - loop sysread: $!");
			goto &_error ;
		}

# good read. see if we hit EOF (nothing left to read)

		last if $read_cnt == 0 ;

# loop if we are slurping a handle. we don't track $size_left then.

		next if $blk_size ;

# count down how much we read and loop if we have more to read.

		$size_left -= $read_cnt ;
		last if $size_left <= 0 ;
	}

# fix up cr/lf to be a newline if this is a windows text file

	${$buf_ref} =~ s/\015\012/\n/g if $is_win32 && !$opts->{'binmode'} ;

# this is the 5 returns in a row. each handles one possible
# combination of caller context and requested return type

	my $sep = $/ ;
	$sep = '\n\n+' if defined $sep && $sep eq '' ;

# see if caller wants lines

	if( wantarray || $opts->{'array_ref'} ) {

		my @parts = split m/($sep)/, ${$buf_ref}, -1;

		my @lines ;

		while( @parts > 2 ) {

			my( $line, $sep ) = splice( @parts, 0, 2 ) ;
			push @lines, "$line$sep" ;
		}

		push @lines, shift @parts if @parts && length $parts[0] ;

		return \@lines if $opts->{'array_ref'} ;
		return @lines ;
	}

# caller wants a scalar ref to the slurped text

	return $buf_ref if $opts->{'scalar_ref'} ;

# caller wants a scalar with the slurped text (normal scalar context)

	return ${$buf_ref} if defined wantarray ;

# caller passed in an i/o buffer by reference (normal void context)

	return ;


# # caller wants to get an array ref of lines

# # this split doesn't work since it tries to use variable length lookbehind
# # the m// line works.
# #	return [ split( m|(?<=$sep)|, ${$buf_ref} ) ] if $opts->{'array_ref'}  ;
# 	return [ length(${$buf_ref}) ? ${$buf_ref} =~ /(.*?$sep|.+)/sg : () ]
# 		if $opts->{'array_ref'}  ;

# # caller wants a list of lines (normal list context)

# # same problem with this split as before.
# #	return split( m|(?<=$sep)|, ${$buf_ref} ) if wantarray ;
# 	return length(${$buf_ref}) ? ${$buf_ref} =~ /(.*?$sep|.+)/sg : ()
# 		if wantarray ;

# # caller wants a scalar ref to the slurped text

# 	return $buf_ref if $opts->{'scalar_ref'} ;

# # caller wants a scalar with the slurped text (normal scalar context)

# 	return ${$buf_ref} if defined wantarray ;

# # caller passed in an i/o buffer by reference (normal void context)

# 	return ;
}


# errors in this sub are returned as scalar refs
# a normal IO/GLOB handle is an empty return
# an overloaded object returns its stringified as a scalarfilename

sub _check_ref {

	my( $handle ) = @_ ;

# check if we are reading from a handle (GLOB or IO object)

	if ( eval { $handle->isa( 'GLOB' ) || $handle->isa( 'IO' ) } ) {

# we have a handle. deal with seeking to it if it is DATA

		my $err = _seek_data_handle( $handle ) ;

# return the error string if any

		return \$err if $err ;

# we have good handle
		return ;
	}

	eval { require overload } ;

# return an error if we can't load the overload pragma
# or if the object isn't overloaded

	return \"Bad handle '$handle' is not a GLOB or IO object or overloaded"
		 if $@ || !overload::Overloaded( $handle ) ;

# must be overloaded so return its stringified value

	return "$handle" ;
}

sub _seek_data_handle {

	my( $handle ) = @_ ;

# DEEP DARK MAGIC. this checks the UNTAINT IO flag of a
# glob/handle. only the DATA handle is untainted (since it is from
# trusted data in the source file). this allows us to test if this is
# the DATA handle and then to do a sysseek to make sure it gets
# slurped correctly. on some systems, the buffered i/o pointer is not
# left at the same place as the fd pointer. this sysseek makes them
# the same so slurping with sysread will work.

	eval{ require B } ;

	if ( $@ ) {

		return <<ERR ;
Can't find B.pm with this Perl: $!.
That module is needed to properly slurp the DATA handle.
ERR
	}

	if ( B::svref_2object( $handle )->IO->IoFLAGS & 16 ) {

# set the seek position to the current tell.

		unless( sysseek( $handle, tell( $handle ), SEEK_SET ) ) {
			return "read_file '$handle' - sysseek: $!" ;
		}
	}

# seek was successful, return no error string

	return ;
}


sub write_file {

	my $file_name = shift ;

# get the optional argument hash ref from @_ or an empty hash ref.

	my $opts = ( ref $_[0] eq 'HASH' ) ? shift : {} ;

	my( $buf_ref, $write_fh, $no_truncate, $orig_file_name, $data_is_ref ) ;

# get the buffer ref - it depends on how the data is passed into write_file
# after this if/else $buf_ref will have a scalar ref to the data.

	if ( ref $opts->{'buf_ref'} eq 'SCALAR' ) {

# a scalar ref passed in %opts has the data
# note that the data was passed by ref

		$buf_ref = $opts->{'buf_ref'} ;
		$data_is_ref = 1 ;
	}
	elsif ( ref $_[0] eq 'SCALAR' ) {

# the first value in @_ is the scalar ref to the data
# note that the data was passed by ref

		$buf_ref = shift ;
		$data_is_ref = 1 ;
	}
	elsif ( ref $_[0] eq 'ARRAY' ) {

# the first value in @_ is the array ref to the data so join it.

		${$buf_ref} = join '', @{$_[0]} ;
	}
	else {

# good old @_ has all the data so join it.

		${$buf_ref} = join '', @_ ;
	}

# deal with ref for a file name

	if ( ref $file_name ) {

		my $ref_result = _check_ref( $file_name ) ;

		if ( ref $ref_result ) {

# we got an error, deal with it

			@_ = ( $opts, $ref_result ) ;
			goto &_error ;
		}

		if ( $ref_result ) {

# we got an overloaded object and the result is the stringified value
# use it as the file name

			$file_name = $ref_result ;
		}
		else {

# we now have a proper handle ref.
# make sure we don't call truncate on it.

			$write_fh = $file_name ;
			$no_truncate = 1 ;
		}
	}

# see if we have a path we need to open

	unless( $write_fh ) {

# spew to regular file.

		if ( $opts->{'atomic'} ) {

# in atomic mode, we spew to a temp file so make one and save the original
# file name.
			$orig_file_name = $file_name ;
			$file_name .= ".$$" ;
		}

# set the mode for the sysopen

		my $mode = O_WRONLY | O_CREAT ;
		$mode |= O_APPEND if $opts->{'append'} ;
		$mode |= O_EXCL if $opts->{'no_clobber'} ;

		my $perms = $opts->{perms} ;
		$perms = 0666 unless defined $perms ;

#printf "WR: BINARY %x MODE %x\n", O_BINARY, $mode ;

# open the file and handle any error.

		$write_fh = gensym ;
		unless ( sysopen( $write_fh, $file_name, $mode, $perms ) ) {

			@_ = ( $opts, "write_file '$file_name' - sysopen: $!");
			goto &_error ;
		}
	}

	if ( my $binmode = $opts->{'binmode'} ) {
		binmode( $write_fh, $binmode ) ;
	}

	sysseek( $write_fh, 0, SEEK_END ) if $opts->{'append'} ;


#print 'WR before data ', unpack( 'H*', ${$buf_ref}), "\n" ;

# fix up newline to write cr/lf if this is a windows text file

	if ( $is_win32 && !$opts->{'binmode'} ) {

# copy the write data if it was passed by ref so we don't clobber the
# caller's data
		$buf_ref = \do{ my $copy = ${$buf_ref}; } if $data_is_ref ;
		${$buf_ref} =~ s/\n/\015\012/g ;
	}

#print 'after data ', unpack( 'H*', ${$buf_ref}), "\n" ;

# get the size of how much we are writing and init the offset into that buffer

	my $size_left = length( ${$buf_ref} ) ;
	my $offset = 0 ;

# loop until we have no more data left to write

	do {

# do the write and track how much we just wrote

		my $write_cnt = syswrite( $write_fh, ${$buf_ref},
				$size_left, $offset ) ;

		unless ( defined $write_cnt ) {

# the write failed
			@_ = ( $opts, "write_file '$file_name' - syswrite: $!");
			goto &_error ;
		}

# track much left to write and where to write from in the buffer

		$size_left -= $write_cnt ;
		$offset += $write_cnt ;

	} while( $size_left > 0 ) ;

# we truncate regular files in case we overwrite a long file with a shorter file
# so seek to the current position to get it (same as tell()).

	truncate( $write_fh,
		  sysseek( $write_fh, 0, SEEK_CUR ) ) unless $no_truncate ;

	close( $write_fh ) ;

# handle the atomic mode - move the temp file to the original filename.

	if ( $opts->{'atomic'} && !rename( $file_name, $orig_file_name ) ) {

		@_ = ( $opts, "write_file '$file_name' - rename: $!" ) ;
		goto &_error ;
	}

	return 1 ;
}

# this is for backwards compatibility with the previous File::Slurp module. 
# write_file always overwrites an existing file

*overwrite_file = \&write_file ;

# the current write_file has an append mode so we use that. this
# supports the same API with an optional second argument which is a
# hash ref of options.

sub append_file {

# get the optional opts hash ref
	my $opts = $_[1] ;
	if ( ref $opts eq 'HASH' ) {

# we were passed an opts ref so just mark the append mode

		$opts->{append} = 1 ;
	}
	else {

# no opts hash so insert one with the append mode

		splice( @_, 1, 0, { append => 1 } ) ;
	}

# magic goto the main write_file sub. this overlays the sub without touching
# the stack or @_

	goto &write_file
}

# basic wrapper around opendir/readdir

# prepend data to the beginning of a file

sub prepend_file {

	my $file_name = shift ;

#print "FILE $file_name\n" ;

	my $opts = ( ref $_[0] eq 'HASH' ) ? shift : {} ;

# delete unsupported options

	my @bad_opts =
		grep $_ ne 'err_mode' && $_ ne 'binmode', keys %{$opts} ;

	delete @{$opts}{@bad_opts} ;

	my $prepend_data = shift ;
	$prepend_data = '' unless defined $prepend_data ;
	$prepend_data = ${$prepend_data} if ref $prepend_data eq 'SCALAR' ;

#print "PRE [$prepend_data]\n" ;


###### set croak as error_mode
###### wrap in eval

	my $err_mode = delete $opts->{err_mode} ;
	$opts->{ err_mode } = 'croak' ;
	$opts->{ scalar_ref } = 1 ;

	my $existing_data ;
	eval { $existing_data = read_file( $file_name, $opts ) } ;

	if ( $@ ) {

		@_ = ( { err_mode => $err_mode },
			"prepend_file '$file_name' - read_file: $!" ) ;
		goto &_error ;
	}

#print "EXIST [$$existing_data]\n" ;

	$opts->{ atomic } = 1 ;

	my $write_result = eval { 
		write_file( $file_name, $opts,
			$prepend_data, $$existing_data ) ;
	} ;

	if ( $@ ) {

		@_ = ( { err_mode => $err_mode },
			"prepend_file '$file_name' - write_file: $!" ) ;
		goto &_error ;
	}

	return $write_result ;
}

sub read_dir {

	my $dir = shift ;
	my $opts = ( ref $_[0] eq 'HASH' ) ? shift : { @_ } ;

# this handle will be destroyed upon return

	local(*DIRH);

# open the dir and handle any errors

	unless ( opendir( DIRH, $dir ) ) {

		@_ = ( $opts, "read_dir '$dir' - opendir: $!" ) ;
		goto &_error ;
	}

	my @dir_entries = readdir(DIRH) ;

	@dir_entries = grep( $_ ne "." && $_ ne "..", @dir_entries )
		unless $opts->{'keep_dot_dot'} ;

	return @dir_entries if wantarray ;
	return \@dir_entries ;
}

# error handling section
#
# all the error handling uses magic goto so the caller will get the
# error message as if from their code and not this module. if we just
# did a call on the error code, the carp/croak would report it from
# this module since the error sub is one level down on the call stack
# from read_file/write_file/read_dir.


my %err_func = (
	'carp'	=> \&carp,
	'croak'	=> \&croak,
) ;

sub _error {

	my( $opts, $err_msg ) = @_ ;

# get the error function to use

 	my $func = $err_func{ $opts->{'err_mode'} || 'croak' } ;

# if we didn't find it in our error function hash, they must have set
# it to quiet and we don't do anything.

	return unless $func ;

# call the carp/croak function

	$func->($err_msg) if $func ;

# return a hard undef (in list context this will be a single value of
# undef which is not a legal in-band value)

	return undef ;
}

1;
__END__

=head1 NAME

File::Slurp - Simple and Efficient Reading/Writing of Complete Files

=head1 SYNOPSIS

  use File::Slurp;

# read in a whole file into a scalar

  my $text = read_file( 'filename' ) ;

# read in a whole file into an array of lines

  my @lines = read_file( 'filename' ) ;

# write out a whole file from a scalar

  write_file( 'filename', $text ) ;

# write out a whole file from an array of lines

  write_file( 'filename', @lines ) ;

# Here is a simple and fast way to load and save a simple config file
# made of key=value lines.

  my %conf = read_file( $file_name ) =~ /^(\w+)=(\.*)$/mg ;
  write_file( $file_name, {atomic => 1}, map "$_=$conf{$_}\n", keys %conf ;

# read in a whole directory of file names (skipping . and ..)

  my @files = read_dir( '/path/to/dir' ) ;

=head1 DESCRIPTION

This module provides subs that allow you to read or write entire files
with one simple call. They are designed to be simple to use, have
flexible ways to pass in or get the file contents and to be very
efficient.  There is also a sub to read in all the files in a
directory other than C<.> and C<..>

These slurp/spew subs work for files, pipes and sockets, stdio,
pseudo-files, and the DATA handle. Read more about why slurping files is
a good thing in the file 'slurp_article.pod' in the extras/ directory.

If you are interested in how fast these calls work, check out the
slurp_bench.pl program in the extras/ directory. It compares many
different forms of slurping. You can select the I/O direction, context
and file sizes. Use the --help option to see how to run it.

=head2 B<read_file>

This sub reads in an entire file and returns its contents to the
caller.  In scalar context it returns the entire file as a single
scalar. In list context it will return a list of lines (using the
current value of $/ as the separator including support for paragraph
mode when it is set to '').

  my $text = read_file( 'filename' ) ;
  my $bin = read_file( 'filename' { binmode => ':raw' } ) ;
  my @lines = read_file( 'filename' ) ;
  my $lines = read_file( 'filename', array_ref => 1 ) ;

The first argument is the file to slurp in. If the next argument is a
hash reference, then it is used as the options. Otherwise the rest of
the argument list are is used as key/value options.

If the file argument is a handle (if it is a ref and is an IO or GLOB
object), then that handle is slurped in. This mode is supported so you
slurp handles such as C<DATA> and C<STDIN>. See the test handle.t for
an example that does C<open( '-|' )> and the child process spews data
to the parant which slurps it in.  All of the options that control how
the data is returned to the caller still work in this case.

If the first argument is an overloaded object then its stringified value
is used for the filename and that file is opened.  This is a new feature
in 9999.14. See the stringify.t test for an example.

By default C<read_file> returns an undef in scalar contex or a single
undef in list context if it encounters an error. Those are both
impossible to get with a clean read_file call which means you can check
the return value and always know if you had an error. You can change how
errors are handled with the C<err_mode> option.

NOTE: as of version 9999.06, read_file works correctly on the C<DATA>
handle. It used to need a sysseek workaround but that is now handled
when needed by the module itself.

You can optionally request that C<slurp()> is exported to your code. This
is an alias for read_file and is meant to be forward compatible with
Perl 6 (which will have slurp() built-in).

The options for C<read_file> are:

=head3 binmode

If you set the binmode option, then its value is passed to a call to
binmode on the opened handle. You can use this to set the file to be
read in binary mode, utf8, etc. See perldoc -f binmode for more.

	my $bin_data = read_file( $bin_file, binmode => ':raw' ) ;
	my $utf_text = read_file( $bin_file, binmode => ':utf8' ) ;

=head3 array_ref

If this boolean option is set, the return value (only in scalar
context) will be an array reference which contains the lines of the
slurped file. The following two calls are equivalent:

	my $lines_ref = read_file( $bin_file, array_ref => 1 ) ;
	my $lines_ref = [ read_file( $bin_file ) ] ;

=head3 scalar_ref

If this boolean option is set, the return value (only in scalar context)
will be an scalar reference to a string which is the contents of the
slurped file. This will usually be faster than returning the plain
scalar. It will also save memory as it will not make a copy of the file
to return.

	my $text_ref = read_file( $bin_file, scalar_ref => 1 ) ;

=head3 buf_ref

You can use this option to pass in a scalar reference and the slurped
file contents will be stored in the scalar. This can be used in
conjunction with any of the other options. This saves an extra copy of
the slurped file and can lower ram usage vs returning the file.

	read_file( $bin_file, buf_ref => \$buffer ) ;

=head3 blk_size

You can use this option to set the block size used when slurping from
an already open handle (like \*STDIN). It defaults to 1MB.

	my $text_ref = read_file( $bin_file, blk_size => 10_000_000,
					     array_ref => 1 ) ;

=head3 err_mode

You can use this option to control how read_file behaves when an error
occurs. This option defaults to 'croak'. You can set it to 'carp' or to
'quiet to have no special error handling. This code wants to carp and
then read another file if it fails.

	my $text_ref = read_file( $file, err_mode => 'carp' ) ;
	unless ( $text_ref ) {

		# read a different file but croak if not found
		$text_ref = read_file( $another_file ) ;
	}
	
	# process ${$text_ref}

=head2 B<write_file>

This sub writes out an entire file in one call.

  write_file( 'filename', @data ) ;

The first argument to C<write_file> is the filename. The next argument
is an optional hash reference and it contains key/values that can
modify the behavior of C<write_file>. The rest of the argument list is
the data to be written to the file.

  write_file( 'filename', {append => 1 }, @data ) ;
  write_file( 'filename', {binmode => ':raw'}, $buffer ) ;

As a shortcut if the first data argument is a scalar or array reference,
it is used as the only data to be written to the file. Any following
arguments in @_ are ignored. This is a faster way to pass in the output
to be written to the file and is equivalent to the C<buf_ref> option of
C<read_file>. These following pairs are equivalent but the pass by
reference call will be faster in most cases (especially with larger
files).

  write_file( 'filename', \$buffer ) ;
  write_file( 'filename', $buffer ) ;

  write_file( 'filename', \@lines ) ;
  write_file( 'filename', @lines ) ;

If the first argument is a handle (if it is a ref and is an IO or GLOB
object), then that handle is written to. This mode is supported so you
spew to handles such as \*STDOUT. See the test handle.t for an example
that does C<open( '-|' )> and child process spews data to the parent
which slurps it in.  All of the options that control how the data are
passed into C<write_file> still work in this case.

If the first argument is an overloaded object then its stringified value
is used for the filename and that file is opened.  This is new feature
in 9999.14. See the stringify.t test for an example.

By default C<write_file> returns 1 upon successfully writing the file or
undef if it encountered an error. You can change how errors are handled
with the C<err_mode> option.

The options are:

=head3 binmode

If you set the binmode option, then its value is passed to a call to
binmode on the opened handle. You can use this to set the file to be
read in binary mode, utf8, etc. See perldoc -f binmode for more.

	write_file( $bin_file, {binmode => ':raw'}, @data ) ;
	write_file( $bin_file, {binmode => ':utf8'}, $utf_text ) ;

=head3 perms

The perms option sets the permissions of newly-created files. This value
is modified by your process's umask and defaults to 0666 (same as
sysopen).

NOTE: this option is new as of File::Slurp version 9999.14;

=head3 buf_ref

You can use this option to pass in a scalar reference which has the
data to be written. If this is set then any data arguments (including
the scalar reference shortcut) in @_ will be ignored. These are
equivalent:

	write_file( $bin_file, { buf_ref => \$buffer } ) ;
	write_file( $bin_file, \$buffer ) ;
	write_file( $bin_file, $buffer ) ;

=head3 atomic

If you set this boolean option, the file will be written to in an
atomic fashion. A temporary file name is created by appending the pid
($$) to the file name argument and that file is spewed to. After the
file is closed it is renamed to the original file name (and rename is
an atomic operation on most OS's). If the program using this were to
crash in the middle of this, then the file with the pid suffix could
be left behind.

=head3 append

If you set this boolean option, the data will be written at the end of
the current file. Internally this sets the sysopen mode flag O_APPEND.

	write_file( $file, {append => 1}, @data ) ;

 You
can import append_file and it does the same thing.

=head3 no_clobber

If you set this boolean option, an existing file will not be overwritten.

	write_file( $file, {no_clobber => 1}, @data ) ;

=head3 err_mode

You can use this option to control how C<write_file> behaves when an
error occurs. This option defaults to 'croak'. You can set it to
'carp' or to 'quiet' to have no error handling other than the return
value. If the first call to C<write_file> fails it will carp and then
write to another file. If the second call to C<write_file> fails, it
will croak.

	unless ( write_file( $file, { err_mode => 'carp', \$data ) ;

		# write a different file but croak if not found
		write_file( $other_file, \$data ) ;
	}

=head2 overwrite_file

This sub is just a typeglob alias to write_file since write_file
always overwrites an existing file. This sub is supported for
backwards compatibility with the original version of this module. See
write_file for its API and behavior.

=head2 append_file

This sub will write its data to the end of the file. It is a wrapper
around write_file and it has the same API so see that for the full
documentation. These calls are equivalent:

	append_file( $file, @data ) ;
	write_file( $file, {append => 1}, @data ) ;


=head2 prepend_file

This sub writes data to the beginning of a file. The previously existing
data is written after that so the effect is prepending data in front of
a file. It is a counterpart to the append_file sub in this module. It
works by first using C<read_file> to slurp in the file and then calling
C<write_file> with the new data and the existing file data.

The first argument to C<prepend_file> is the filename. The next argument
is an optional hash reference and it contains key/values that can modify
the behavior of C<prepend_file>. The rest of the argument list is the
data to be written to the file and that is passed to C<write_file> as is
(see that for allowed data).

Only the C<binmode> and C<err_mode> options are supported. The
C<write_file> call has the C<atomic> option set so you will always have
a consistant file. See above for more about those options.

C<prepend_file> is not exported by default, you need to import it
explicitly.

	use File::Slurp qw( prepend_file ) ;
	prepend_file( $file, $header ) ;
	prepend_file( $file, \@lines ) ;
	prepend_file( $file, { binmode => 'raw:'}, $bin_data ) ;

=head2 read_dir

This sub reads all the file names from directory and returns them to
the caller but C<.> and C<..> are removed by default.

	my @files = read_dir( '/path/to/dir' ) ;

The first argument is the path to the directory to read.  If the next
argument is a hash reference, then it is used as the options.
Otherwise the rest of the argument list are is used as key/value
options.

In list context C<read_dir> returns a list of the entries in the
directory. In a scalar context it returns an array reference which has
the entries.

=head3 err_mode

If the C<err_mode> option is set, it selects how errors are handled (see
C<err_mode> in C<read_file> or C<write_file>).

=head3 keep_dot_dot

If this boolean option is set, C<.> and C<..> are not removed from the
list of files.

	my @all_files = read_dir( '/path/to/dir', keep_dot_dot => 1 ) ;

=head2 EXPORT

  read_file write_file overwrite_file append_file read_dir

=head2 LICENSE

  Same as Perl.

=head2 SEE ALSO

An article on file slurping in extras/slurp_article.pod. There is
also a benchmarking script in extras/slurp_bench.pl.

=head2 BUGS

If run under Perl 5.004, slurping from the DATA handle will fail as
that requires B.pm which didn't get into core until 5.005.

=head1 AUTHOR

Uri Guttman, E<lt>uri AT stemsystems DOT comE<gt>

=cut
