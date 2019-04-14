#!/usr/bin/perl -T
###
###              analog 6.0             http://www.analog.cx/
### This program is copyright (c) Stephen R. E. Turner 1995 - 2004 except as
### stated otherwise.
###
### This program is free software. You can redistribute it and/or modify it
### under the terms of version 2 of the GNU General Public License, which you
### should have received with it.
###
### This program is distributed in the hope that it will be useful, but
### without any warranty, expressed or implied.
### Remember: Even the most carefully-designed CGI programs can accidentally
### have serious security bugs! See docs/form.html for notes on security
### design.
###
### anlgform.pl; the cgi front end for analog

# 1) uncomment (remove everything before $analog) and edit one of the next two
#    lines to give the location (full pathname) of the analog executable.
#       Unix: $analog = '/usr/local/etc/httpd/analog-6.0/analog';
#       Windows: $analog = 'C:\program files\analog 6.0\analog.exe';
# 2) If you're on Unix, edit the first line in this file to give the location
#    of Perl (don't remove the #! though).
# 3) You also need to edit anlgform.html if you want to use the form.
# 4) Add to the forbidden commands below if you want.

@forbidden = qw(LOGFORMAT APACHELOGFORMAT DEFAULTLOGFORMAT
		APACHEDEFAULTLOGFORMAT HEADERFILE FOOTERFILE UNCOMPRESS
		OUTFILE CACHEOUTFILE LOCALCHARTDIR ERRFILE DNS CGI
		SETTINGS PROGRESSFREQ LANGFILE DESCFILE);
# Forbidden commands: sysadmin can add more (must be in upper case!)
# Other commands you might consider adding, because they allow users to
# specify which files to use for the analysis, are LOGFILE and DOMAINSFILE.
# If you add a command, you must also add any aliases it possesses.
# There is a discussion of all this in docs/form.html.

@allowed = qw();
# Allowed commands. If there are _any_ commands listed here, then _only_
# commands which are in @allowed, and not in @forbidden, can be used.

require 5.001;
use CGI;

# 1) INITIALISATION
# delete all dangerous environment variables
$ENV{PATH} = '';  # blank, not deleted, so that UNCOMPRESS doesn't get a path
delete @ENV{qw/IFS CDPATH ENV BASH_ENV/};
$query = new CGI;
$|=1;
$lt = localtime;
$progname = $0 || 'anlgform.pl';
if (($^O =~ /win32/i || $^O =~ /^win/i) && Win32::GetShortPathName($analog)) {
    $analog = Win32::GetShortPathName($analog);
}

# coerce query keys to caps in a new (key, pointer to array) hash called args
# also remember the order the keys arrived in, as far as possible
foreach $p ($query->param) {
    foreach $a ($query->param($p)) {
	checkchars($a);
	push(@order, "\U$p") unless ($args{"\U$p"});
	push(@{$args{"\U$p"}}, $a);
    }
}

# check LOGFILE and CACHEFILE only contain safe chars (see comments below)
checkfilechars("LOGFILE");
checkfilechars("CACHEFILE");

# 2) OPEN THE ANALOG PROCESS
# qv=1 causes args to go straight to stdout, not program
if (${$args{'QV'}}[-1] && !forbidden('QV')) {
    print "Content-Type: text/plain\n\n";
    open(ANALOG, ">-");
}
elsif (!$analog) {
    badreq(500, "Program Incorrectly Configured",
	   "Can't run analog because anlgform.pl not set up properly.\n",
	   "See the server's error log for more details.");
    print STDERR "[$lt] $progname: Can't run analog because the variable \$analog was not set: read the setup instructions!\n";
    die;
}
elsif (!(-x $analog)) {
    badreq(500, "Program Incorrectly Configured", "Can't run analog.",
	   "See the server's error log for more details.");
    print STDERR "[$lt] $progname: Can't run analog because \"$analog\" not found or not executable";
    print STDERR ": $!" if ($!);
    print STDERR ".\n";
    die;
}
else {
    open (ANALOG, "|$analog +g-");  # errors here will get caught on close
}

# 3) PRINT ALL THE COMMANDS
# Special cases: must come first
printargs('CG', 'CONFIGFILE') unless forbidden('CG');
# both 'CG' and 'CONFIGFILE' must be allowed for this to happen.
print ANALOG "CGI ON\nDNS NONE\nWARNINGS FL\n";
printargs('WARNINGS');
printargs('LOGTIMEOFFSET');
foreach $k (@order) {
    printargs($k)
        unless($k eq 'QV' || $k eq 'CG' || $k eq 'CM' || $k =~ /FLOORB$/ ||
	       $k =~ /2$/ || $k =~ '^LOGTIMEOFFSET' || $k =~ '^WARNINGS' ||
               # commands dealt with elsewhere
               $k =~ /[^A-Z12]/ || $k eq '' || $k =~ /^IGNORE/);
               # other stuff not wanted
}
# Special cases: must come last
print ANALOG "DEBUG -C\n";
printargs('CM', 'CONFIGFILE') unless forbidden('CM');
# again, both 'CM' and 'CONFIGFILE' must be allowed for this to happen.
print ANALOG "OUTFILE stdout\n";

# 4) WAIT FOR PROCESS TO FINISH. THAT'S IT.
unless (close(ANALOG)) {
    badreq(500, "Program Failure", 
	   "Analog failed to run or returned an error code.",
	   "Maybe your server's error log will give a clue why.");
    print STDERR "[$lt] $progname: \"$analog\" failed to run or returned an error code";
    print STDERR ": $!" if ($!);
    print STDERR ".\n";
    die;
}


### SUBROUTINES 
# A) IS A GIVEN COMMAND FORBIDDEN?
sub forbidden {
    return (grep($_[0] eq $_, @forbidden) ||
	    (@allowed && !grep($_[0] eq $_, @allowed)));
}

# B) PRINT ONE COMMAND
sub printargs {
    my($is_floora) = 0;
    my($is_12) = 0;
    my($name) = $_[1] || $_[0];
    if ($name =~ /FLOORA$/) {
	chop($name);
	$is_floora = 1;
    }
    elsif ($name =~ /1$/) {
	chop($name);
	$is_12 = 1;
    }
    return if forbidden($name);

    if ($is_floora) {
	$a = ${$args{$name . 'A'}}[-1];  # last "FLOORA=$a" form arg specified
	$b = ${$args{$name . 'B'}}[-1];
	print ANALOG ("$name $a$b\n") if ($b ne '' && $b !~ /\\$/);
	# could bracket $a$b, but no help because any special character in a
	# FLOOR command is junk anyway.
    }
    elsif ($is_12) {
	$a = ${$args{$name . '1'}}[-1];
	$b = ${$args{$name . '2'}}[-1];
	print ANALOG ("$name ", bracket($a), " ", bracket($b), "\n")
	    if ($b ne '');
    }
    else {
	foreach $a (@{$args{$_[0]}}) {  # run through all "NAME=$a" form args
	    if ($a ne '') {
		print ANALOG ("$name ", bracket($a), "\n");
		print ANALOG ("DNS READ\n") if ($name eq 'DNSFILE');
	    }
	}
    }
}

# C) PUT APPROPRIATE DELIMITERS ROUND AN ARGUMENT CONTAINING SPACES
sub bracket {
    local $_ = $_[0];
    return $_ unless (/[\s\#]/ || /^['"\(]/ || /\\$/);
    return "\"$_\"" unless (/"/);
    return "'$_'" unless (/'/);
    return "($_)";
    # analog has no syntax if string contains ) as well as space, ' and "
}

# D) CHECK ONLY SAFE CHARACTERS in LOGFILEs and CACHEFILEs. See docs/form.html.
sub checkfilechars {
    local ($_);
    foreach (@{$args{$_[0]}}, @{$args{$_[0] . '1'}}) {
	if (m([^\w\. /\\:\-\*\?]) || m(\B-|-\B)) {
	    # i.e. contains a non-approved character, or a dash not
	    # between \w's. NB \w includes underscore.
	    badreq(403, "Illegal Request", "Unsafe characters in $_[0].");
	    printf STDERR "[$lt] $progname: Unsafe characters in \"$_[0] $_\" on request from %s\n", $ENV{REMOTE_HOST}?$ENV{REMOTE_HOST}:($ENV{REMOTE_ADDR}?$ENV{REMOTE_ADDR}:"unknown host");
	    die;
	}
    }
}

# E) CHECK NO UNSAFE CHARACTERS IN OTHER COMMANDS. Again, see docs/form.html.
sub checkchars {
    local $_ = $_[0];
    if (/[\x00-\x1F\x7F-\x9F]/) {
	printf STDERR "[$lt] $progname: Unsafe characters in \"\U$p\E $_\" on request from %s\n", $ENV{REMOTE_HOST}?$ENV{REMOTE_HOST}:($ENV{REMOTE_ADDR}?$ENV{REMOTE_ADDR}:"unknown host");
	# Translate dangerous characters to avoid cross-site scripting
	$p =~ s/&/&amp;/;
	$p =~ s/</&lt;/;
	$p =~ s/>/&gt;/;
	$p =~ s/"/&quot;/;
	badreq(403, "Illegal Request", "Unsafe characters in \U$p.");
	die;
    }
}

# F) PRINT OUT ERROR MESSAGE
sub badreq {
    my($i);
    print "Content-Type: text/html\n";
    print "Status: $_[0] $_[1]\n\n";
    print '<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">';
    print "\n<html><head><title>$_[0] $_[1]</title></head>\n";
    print "<body><h1>$_[1]</h1>\n";
    for ($i = 2; defined($_[$i]); $i++) {
	print "<br>" if ($i >= 3);
	print "$_[$i]\n";
    }
    print "</body></html>\n";
}
