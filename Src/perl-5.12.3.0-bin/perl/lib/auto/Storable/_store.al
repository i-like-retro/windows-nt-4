# NOTE: Derived from blib\lib\Storable.pm.
# Changes made here will be lost when autosplit is run again.
# See AutoSplit.pm.
package Storable;

#line 238 "blib\lib\Storable.pm (autosplit into blib\lib\auto\Storable\_store.al)"
# Internal store to file routine
sub _store {
	my $xsptr = shift;
	my $self = shift;
	my ($file, $use_locking) = @_;
	logcroak "not a reference" unless ref($self);
	logcroak "wrong argument number" unless @_ == 2;	# No @foo in arglist
	local *FILE;
	if ($use_locking) {
		open(FILE, ">>$file") || logcroak "can't write into $file: $!";
		unless (&CAN_FLOCK) {
			logcarp "Storable::lock_store: fcntl/flock emulation broken on $^O";
			return undef;
		}
		flock(FILE, LOCK_EX) ||
			logcroak "can't get exclusive lock on $file: $!";
		truncate FILE, 0;
		# Unlocking will happen when FILE is closed
	} else {
		open(FILE, ">$file") || logcroak "can't create $file: $!";
	}
	binmode FILE;				# Archaic systems...
	my $da = $@;				# Don't mess if called from exception handler
	my $ret;
	# Call C routine nstore or pstore, depending on network order
	eval { $ret = &$xsptr(*FILE, $self) };
	# close will return true on success, so the or short-circuits, the ()
	# expression is true, and for that case the block will only be entered
	# if $@ is true (ie eval failed)
	# if close fails, it returns false, $ret is altered, *that* is (also)
	# false, so the () expression is false, !() is true, and the block is
	# entered.
	if (!(close(FILE) or undef $ret) || $@) {
		unlink($file) or warn "Can't unlink $file: $!\n";
	}
	logcroak $@ if $@ =~ s/\.?\n$/,/;
	$@ = $da;
	return $ret;
}

# end of Storable::_store
1;
