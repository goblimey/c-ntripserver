#!/usr/bin/perl -w

use strict;
use IO::Socket;

# List of Parameters
# ------------------
my($port) = @ARGV;

if (!defined($port)) {
  die "Usage: test_tcpip_client.pl portNumber\n";
}

# Local Variables
# ---------------
my($serverHostName) = "localhost";
my $server;

my $retries = 10;
while ($retries--) {
  $server = IO::Socket::INET->new( Proto    => "tcp",
                                   PeerAddr => $serverHostName,
                                   PeerPort => $port);
  last if ($server);
}
die "Cannot connect to $serverHostName on $port: $!" unless ($server);

my $buffer;
while (defined ($buffer = <$server>)) {
  print $buffer;
}


