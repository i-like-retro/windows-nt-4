package File::HomeDir;

# See POD at end for documentation

use 5.00503;
use strict;
use Carp        ();
use Config      ();
use File::Spec  ();
use File::Which ();

# Globals
use vars qw{$VERSION @ISA @EXPORT @EXPORT_OK $IMPLEMENTED_BY};
BEGIN {
	$VERSION = '0.97';

	# Inherit manually
	require Exporter;
	@ISA       = qw{ Exporter };
	@EXPORT    = qw{ home     };
	@EXPORT_OK = qw{
		home
		my_home
		my_desktop
		my_documents
		my_music
		my_pictures
		my_videos
		my_data
		my_dist_config
		my_dist_data
		users_home
		users_desktop
		users_documents
		users_music
		users_pictures
		users_videos
		users_data
	};

	# %~ doesn't need (and won't take) exporting, as it's a magic
	# symbol name that's always looked for in package 'main'.
}

# Inlined Params::Util functions
sub _CLASS ($) {
	(defined $_[0] and ! ref $_[0] and $_[0] =~ m/^[^\W\d]\w*(?:::\w+)*\z/s) ? $_[0] : undef;
}
sub _DRIVER ($$) {
	(defined _CLASS($_[0]) and eval "require $_[0];" and ! $@ and $_[0]->isa($_[1]) and $_[0] ne $_[1]) ? $_[0] : undef;
}

# Platform detection
if ( $IMPLEMENTED_BY ) {
	# Allow for custom HomeDir classes
	# Leave it as the existing value
} elsif ( $^O eq 'MSWin32' ) {
	# All versions of Windows
	$IMPLEMENTED_BY = 'File::HomeDir::Windows';
} elsif ( $^O eq 'darwin') {
	# 1st: try Mac::SystemDirectory by chansen
	if ( eval { require Mac::SystemDirectory; 1 } ) {
		$IMPLEMENTED_BY = 'File::HomeDir::Darwin::Cocoa';
	} elsif ( eval { require Mac::Files; 1 } ) {
		# 2nd try Mac::Files: Carbon - unmaintained since 2006 except some 64bit fixes
		$IMPLEMENTED_BY = 'File::HomeDir::Darwin::Carbon';
	} else {
		# 3rd: fallback: pure perl
		$IMPLEMENTED_BY = 'File::HomeDir::Darwin';
	}
} elsif ( $^O eq 'MacOS' ) {
	# Legacy Mac OS
	$IMPLEMENTED_BY = 'File::HomeDir::MacOS9';
} elsif ( File::Which::which('xdg-user-dir') ) {
	# freedesktop unixes
	$IMPLEMENTED_BY = 'File::HomeDir::FreeDesktop';
} else {
	# Default to Unix semantics
	$IMPLEMENTED_BY = 'File::HomeDir::Unix';
}
unless ( _DRIVER($IMPLEMENTED_BY, 'File::HomeDir::Driver') ) {
	Carp::croak("Missing or invalid File::HomeDir driver $IMPLEMENTED_BY");
}





#####################################################################
# Current User Methods

sub my_home {
	$IMPLEMENTED_BY->my_home;
}

sub my_desktop {
	$IMPLEMENTED_BY->can('my_desktop')
		? $IMPLEMENTED_BY->my_desktop
		: Carp::croak("The my_desktop method is not implemented on this platform");
}

sub my_documents {
	$IMPLEMENTED_BY->can('my_documents')
		? $IMPLEMENTED_BY->my_documents
		: Carp::croak("The my_documents method is not implemented on this platform");
}

sub my_music {
	$IMPLEMENTED_BY->can('my_music')
		? $IMPLEMENTED_BY->my_music
		: Carp::croak("The my_music method is not implemented on this platform");
}

sub my_pictures {
	$IMPLEMENTED_BY->can('my_pictures')
		? $IMPLEMENTED_BY->my_pictures
		: Carp::croak("The my_pictures method is not implemented on this platform");
}

sub my_videos {
	$IMPLEMENTED_BY->can('my_videos')
		? $IMPLEMENTED_BY->my_videos
		: Carp::croak("The my_videos method is not implemented on this platform");
}

sub my_data {
	$IMPLEMENTED_BY->can('my_data')
		? $IMPLEMENTED_BY->my_data
		: Carp::croak("The my_data method is not implemented on this platform");
}


sub my_dist_data {
	my $params = ref $_[-1] eq 'HASH' ? pop : {};
	my $dist   = pop or Carp::croak("The my_dist_data method requires an argument");
	my $data   = my_data();

        # If datadir is not defined, there's nothing we can do: bail out
        # and return nothing...	
	return undef unless defined $data;

        # On traditional unixes, hide the top-level directory
	my $var = $data eq home()
		? File::Spec->catdir( $data, '.perl', 'dist', $dist )
		: File::Spec->catdir( $data, 'Perl',  'dist', $dist );

	# directory exists: return it
	return $var if -d $var;

	# directory doesn't exist: check if we need to create it...
	return undef unless $params->{create};

	# user requested directory creation
	require File::Path;
	File::Path::mkpath( $var );
	return $var;
}

sub my_dist_config {
	my $params = ref $_[-1] eq 'HASH' ? pop : {};
	my $dist   = pop or Carp::croak("The my_dist_config method requires an argument");
	
	# not all platforms support a specific my_config() method
	my $config = $IMPLEMENTED_BY->can('my_config')
		? $IMPLEMENTED_BY->my_config
		: $IMPLEMENTED_BY->my_documents;

    # If neither configdir nor my_documents is defined, there's
    # nothing we can do: bail out and return nothing...	
	return undef unless defined $config;

    # On traditional unixes, hide the top-level dir
	my $etc = $config eq home()
		? File::Spec->catdir( $config, '.perl', $dist )
		: File::Spec->catdir( $config, 'Perl',  $dist );

	# directory exists: return it
	return $etc if -d $etc;

	# directory doesn't exist: check if we need to create it...
	return undef unless $params->{create};

	# user requested directory creation
	require File::Path;
	File::Path::mkpath( $etc );
	return $etc;
}




#####################################################################
# General User Methods

sub users_home {
	$IMPLEMENTED_BY->can('users_home')
		? $IMPLEMENTED_BY->users_home( $_[-1] )
		: Carp::croak("The users_home method is not implemented on this platform");
}

sub users_desktop {
	$IMPLEMENTED_BY->can('users_desktop')
		? $IMPLEMENTED_BY->users_desktop( $_[-1] )
		: Carp::croak("The users_desktop method is not implemented on this platform");
}

sub users_documents {
	$IMPLEMENTED_BY->can('users_documents')
		? $IMPLEMENTED_BY->users_documents( $_[-1] )
		: Carp::croak("The users_documents method is not implemented on this platform");
}

sub users_music {
	$IMPLEMENTED_BY->can('users_music')
		? $IMPLEMENTED_BY->users_music( $_[-1] )
		: Carp::croak("The users_music method is not implemented on this platform");
}

sub users_pictures {
	$IMPLEMENTED_BY->can('users_pictures')
		? $IMPLEMENTED_BY->users_pictures( $_[-1] )
		: Carp::croak("The users_pictures method is not implemented on this platform");
}

sub users_videos {
	$IMPLEMENTED_BY->can('users_videos')
		? $IMPLEMENTED_BY->users_videos( $_[-1] )
		: Carp::croak("The users_videos method is not implemented on this platform");
}

sub users_data {
	$IMPLEMENTED_BY->can('users_data')
		? $IMPLEMENTED_BY->users_data( $_[-1] )
		: Carp::croak("The users_data method is not implemented on this platform");
}





#####################################################################
# Legacy Methods

# Find the home directory of an arbitrary user
sub home (;$) {
	# Allow to be called as a method
	if ( $_[0] and $_[0] eq 'File::HomeDir' ) {
		shift();
	}

	# No params means my home
	return my_home() unless @_;

	# Check the param
	my $name = shift;
	if ( ! defined $name ) {
		Carp::croak("Can't use undef as a username");
	}
	if ( ! length $name ) {
		Carp::croak("Can't use empty-string (\"\") as a username");
	}

	# A dot also means my home
	### Is this meant to mean File::Spec->curdir?
	if ( $name eq '.' ) {
		return my_home();
	}

	# Now hand off to the implementor
	$IMPLEMENTED_BY->users_home($name);
}





#####################################################################
# Tie-Based Interface

# Okay, things below this point get scary

CLASS: {
	# Make the class for the %~ tied hash:
	package File::HomeDir::TIE;

	# Make the singleton object.
	# (We don't use the hash for anything, though)
	### THEN WHY MAKE IT???
	my $SINGLETON = bless {};

	sub TIEHASH { $SINGLETON }

	sub FETCH {
		# Catch a bad username
		unless ( defined $_[1] ) {
			Carp::croak("Can't use undef as a username");
		}

		# Get our homedir
		unless ( length $_[1] ) {
			return File::HomeDir::my_home();
		}

		# Get a named user's homedir
		return File::HomeDir::home($_[1]);
	}

	sub STORE    { _bad('STORE')    }
	sub EXISTS   { _bad('EXISTS')   }
	sub DELETE   { _bad('DELETE')   }
	sub CLEAR    { _bad('CLEAR')    }
	sub FIRSTKEY { _bad('FIRSTKEY') }
	sub NEXTKEY  { _bad('NEXTKEY')  }

	sub _bad ($) {
		Carp::croak("You can't $_[0] with the %~ hash")
	}
}

# Do the actual tie of the global %~ variable
tie %~, 'File::HomeDir::TIE';
eval {
	require Portable;
	Portable->import('HomeDir');
};


1;

__END__

=pod

=head1 NAME

File::HomeDir - Find your home and other directories on any platform

=head1 SYNOPSIS

  use File::HomeDir;
  
  # Modern Interface (Current User)
  $home     = File::HomeDir->my_home;
  $desktop  = File::HomeDir->my_desktop;
  $docs     = File::HomeDir->my_documents;
  $music    = File::HomeDir->my_music;
  $pics     = File::HomeDir->my_pictures;
  $videos   = File::HomeDir->my_videos;
  $data     = File::HomeDir->my_data;
  $dist     = File::HomeDir->my_dist_data('File-HomeDir');
  $dist     = File::HomeDir->my_dist_config('File-HomeDir');
  
  # Modern Interface (Other Users)
  $home    = File::HomeDir->users_home('foo');
  $desktop = File::HomeDir->users_desktop('foo');
  $docs    = File::HomeDir->users_documents('foo');
  $music   = File::HomeDir->users_music('foo');
  $pics    = File::HomeDir->users_pictures('foo');
  $video   = File::HomeDir->users_videos('foo');
  $data    = File::HomeDir->users_data('foo');
  
  # Legacy Interfaces
  print "My dir is ", home(), " and root's is ", home('root'), "\n";
  print "My dir is $~{''} and root's is $~{root}\n";
  # These both print the same thing, something like:
  # "My dir is /home/user/mojo and root's is /"

=head1 DESCRIPTION

B<File::HomeDir> is a module for locating the directories that are "owned"
by a user (typicaly your user) and to solve the various issues that arise
trying to find them consistently across a wide variety of platforms.

The end result is a single API that can find your resources on any platform,
making it relatively trivial to create Perl software that works elegantly
and correctly no matter where you run it.

This module provides two main interfaces.

The first is a modern L<File::Spec>-style interface with a consistent
OO API and different implementation modules to support various
platforms. You are B<strongly> recommended to use this interface.

The second interface is for legacy support of the original 0.07 interface
that exported a C<home()> function by default and tied the C<%~> variable.

It is generally not recommended that you use this interface, but due to
back-compatibility reasons they will remain supported until at least 2010.

The C<%~> interface has been deprecated. Documentation was removed in 2009,
Unit test were removed in 2011, usage will issue warnings from 2013, and the
interface will be removed entirely in 2015  (in line with the general Perl
toolchain convention of a 10 year support period for legacy APIs that
are potentially or actually in common use).

=head2 Platform Neutrality

In the Unix world, many different types of data can be mixed together
in your home directory (although on some Unix platforms this is no longer
the case, particularly for "desktop"-oriented platforms).

On some non-Unix platforms, separate directories are allocated for
different types of data and have been for a long time.

When writing applications on top of B<File::HomeDir>, you should thus
always try to use the most specific method you can. User documents should
be saved in C<my_documents>, data that supports an application but isn't
normally editing by the user directory should go into C<my_data>.

On platforms that do not make any distinction, all these different
methods will harmlessly degrade to the main home directory, but on
platforms that care B<File::HomeDir> will always try to Do The Right
Thing(tm).

=head1 METHODS

Two types of methods are provided. The C<my_method> series of methods for
finding resources for the current user, and the C<users_method> (read as
"user's method") series for finding resources for arbitrary users.

This split is necessary, as on most platforms it is B<much> easier to find
information about the current user compared to other users, and indeed
on a number you cannot find out information such as C<users_desktop> at
all, due to security restrictions.

All methods will double check (using a C<-d> test) that a directory
actually exists before returning it, so you may trust in the values
that are returned (subject to the usual caveats of race conditions of
directories being deleted at the moment between a directory being returned
and you using it).

However, because in some cases platforms may not support the concept of home
directories at all, any method may return C<undef> (both in scalar and list
context) to indicate that there is no matching directory on the system.

For example, most untrusted 'nobody'-type users do not have a home
directory. So any modules that are used in a CGI application that
at some level of recursion use your code, will result in calls to
File::HomeDir returning undef, even for a basic home() call.

=head2 my_home

The C<my_home> method takes no arguments and returns the main home/profile
directory for the current user.

If the distinction is important to you, the term "current" refers to the
real user, and not the effective user.

This is also the case for all of the other "my" methods.

Returns the directory path as a string, C<undef> if the current user
does not have a home directory, or dies on error.

=head2 my_desktop

The C<my_desktop> method takes no arguments and returns the "desktop"
directory for the current user.

Due to the diversity and complexity of implementions required to deal with
implementing the required functionality fully and completely, the
C<my_desktop> method may or may not be implemented on each platform.

That said, I am extremely interested in code to implement C<my_desktop> on
Unix, as long as it is capable of dealing (as the Windows implementation
does) with internationalisation. It should also avoid false positive
results by making sure it only returns the appropriate directories for the
appropriate platforms.

Returns the directory path as a string, C<undef> if the current user
does not have a desktop directory, or dies on error.

=head2 my_documents

The C<my_documents> method takes no arguments and returns the directory (for
the current user) where the user's documents are stored.

Returns the directory path as a string, C<undef> if the current user
does not have a documents directory, or dies on error.

=head2 my_music

The C<my_music> method takes no arguments and returns the directory
where the current user's music is stored.

No bias is made to any particular music type or music program, rather the
concept of a directory to hold the user's music is made at the level of the
underlying operating system or (at least) desktop environment.

Returns the directory path as a string, C<undef> if the current user
does not have a suitable directory, or dies on error.

=head2 my_pictures

The C<my_pictures> method takes no arguments and returns the directory
where the current user's pictures are stored.

No bias is made to any particular picture type or picture program, rather the
concept of a directory to hold the user's pictures is made at the level of the
underlying operating system or (at least) desktop environment.

Returns the directory path as a string, C<undef> if the current user
does not have a suitable directory, or dies on error.

=head2 my_videos

The C<my_videos> method takes no arguments and returns the directory
where the current user's videos are stored.

No bias is made to any particular video type or video program, rather the
concept of a directory to hold the user's videos is made at the level of the
underlying operating system or (at least) desktop environment.

Returns the directory path as a string, C<undef> if the current user
does not have a suitable directory, or dies on error.

=head2 my_data

The C<my_data> method takes no arguments and returns the directory where
local applications should stored their internal data for the current
user.

Generally an application would create a subdirectory such as C<.foo>,
beneath this directory, and store its data there. By creating your
directory this way, you get an accurate result on the maximum number of
platforms. But see the documentation about C<my_dist_config()> or
C<my_dist_data()> below.

For example, on Unix you get C<~/.foo> and on Win32 you get
C<~/Local Settings/Application Data/.foo>

Returns the directory path as a string, C<undef> if the current user
does not have a data directory, or dies on error.


=head2 my_dist_config

  File::HomeDir->my_dist_config( $dist [, \%params] );
  
  # For example...
  
  File::HomeDir->my_dist_config( 'File-HomeDir' );
  File::HomeDir->my_dist_config( 'File-HomeDir', { create => 1 } );

The C<my_dist_config> method takes a distribution name as argument and
returns an application-specific directory where they should store their
internal configuration.

The base directory will be either C<my_config> if the platform supports
it, or C<my_documents> otherwise. The subdirectory itself will be 
C<BASE/Perl/Dist-Name>. If the base directory is the user's homedir,
C<my_dist_config> will be in C<~/.perl/Dist-Name> (and thus be hidden on
all Unixes).

The optional last argument is a hash reference to tweak the method
behaviour. The following hash keys are recognized:

=over 4

=item * create

Passing a true value to this key will force the creation of the
directory if it doesn't exist (remember that C<File::HomeDir>'s policy
is to return C<undef> if the directory doesn't exist).

Defaults to false, meaning no automatic creation of directory.

=back


=head2 my_dist_data

  File::HomeDir->my_dist_data( $dist [, \%params] );
  
  # For example...
  
  File::HomeDir->my_dist_data( 'File-HomeDir' );
  File::HomeDir->my_dist_data( 'File-HomeDir', { create => 1 } );

The C<my_dist_data> method takes a distribution name as argument and
returns an application-specific directory where they should store their
internal data.

This directory will be of course a subdirectory of C<my_data>. Platforms
supporting data-specific directories will use
C<DATA_DIR/perl/dist/Dist-Name> following the common
"DATA/vendor/application" pattern. If the C<my_data> directory is the
user's homedir, C<my_dist_data> will be in C<~/.perl/dist/Dist-Name>
(and thus be hidden on all Unixes).

The optional last argument is a hash reference to tweak the method
behaviour. The following hash keys are recognized:

=over 4

=item * create

Passing a true value to this key will force the creation of the
directory if it doesn't exist (remember that C<File::HomeDir>'s policy
is to return C<undef> if the directory doesn't exist).

Defaults to false, meaning no automatic creation of directory.

=back

=head2 users_home

  $home = File::HomeDir->users_home('foo');

The C<users_home> method takes a single param and is used to locate the
parent home/profile directory for an identified user on the system.

While most of the time this identifier would be some form of user name,
it is permitted to vary per-platform to support user ids or UUIDs as
applicable for that platform.

Returns the directory path as a string, C<undef> if that user
does not have a home directory, or dies on error.

=head2 users_documents

  $docs = File::HomeDir->users_documents('foo');

Returns the directory path as a string, C<undef> if that user
does not have a documents directory, or dies on error.

=head2 users_data

  $data = File::HomeDir->users_data('foo');

Returns the directory path as a string, C<undef> if that user
does not have a data directory, or dies on error.

=head1 FUNCTIONS

=head2 home

  use File::HomeDir;
  $home = home();
  $home = home('foo');
  $home = File::HomeDir::home();
  $home = File::HomeDir::home('foo');

The C<home> function is exported by default and is provided for
compatibility with legacy applications. In new applications, you should
use the newer method-based interface above.

Returns the directory path to a named user's home/profile directory.

If provided no param, returns the directory path to the current user's
home/profile directory.

=head1 TO DO

=over 4

=item * Become generally clearer on situations in which a user might not
have a particular resource.

=item * Add more granularity to Unix, and add support to VMS and other
esoteric platforms, so we can consider going core.

=item * Add consistent support for users_* methods 

=back

=head1 SUPPORT

This module is stored in an Open Repository at the following address.

L<http://svn.ali.as/cpan/trunk/File-HomeDir>

Write access to the repository is made available automatically to any
published CPAN author, and to most other volunteers on request.

If you are able to submit your bug report in the form of new (failing)
unit tests, or can apply your fix directly instead of submitting a patch,
you are B<strongly> encouraged to do so as the author currently maintains
over 100 modules and it can take some time to deal with non-Critical bug
reports or patches.

This will guarantee that your issue will be addressed in the next
release of the module.

If you cannot provide a direct test or fix, or don't have time to do so,
then regular bug reports are still accepted and appreciated via the CPAN
bug tracker.

L<http://rt.cpan.org/NoAuth/ReportBug.html?Queue=File-HomeDir>

For other issues, for commercial enhancement or support, or to have your
write access enabled for the repository, contact the author at the email
address above.

=head1 ACKNOWLEDGEMENTS

The biggest acknowledgement must go to Chris Nandor, who wielded his
legendary Mac-fu and turned my initial fairly ordinary Darwin
implementation into something that actually worked properly everywhere,
and then donated a Mac OS X license to allow it to be maintained properly.

=head1 AUTHORS

Adam Kennedy E<lt>adamk@cpan.orgE<gt>

Sean M. Burke E<lt>sburke@cpan.orgE<gt>

Chris Nandor E<lt>cnandor@cpan.orgE<gt>

Stephen Steneker E<lt>stennie@cpan.orgE<gt>

=head1 SEE ALSO

L<File::ShareDir>, L<File::HomeDir::Win32> (legacy)

=head1 COPYRIGHT

Copyright 2005 - 2011 Adam Kennedy.

Some parts copyright 2000 Sean M. Burke.

Some parts copyright 2006 Chris Nandor.

Some parts copyright 2006 Stephen Steneker.

Some parts copyright 2009-2011 Jérôme Quelin.

This program is free software; you can redistribute
it and/or modify it under the same terms as Perl itself.

The full text of the license can be found in the
LICENSE file included with this module.

=cut
