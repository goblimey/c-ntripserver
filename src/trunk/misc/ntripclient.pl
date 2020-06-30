#! /usr/bin/perl
#
#       Name:         ntripclient.pl
#       Authors:      Dirk Stöcker
#       Description:  perl script to access casters using HTTP and HTTPS

use strict;
#use warnings;
#no warnings "uninitialized";
use MIME::Base64;
use Net::HTTP;
use Net::HTTPS;
use Net::SSLeay;
use IO::Handle;
STDOUT->autoflush(1);

my $user = "...";
my $pwd = "...";

#getstream("ntrip.dgpsonline.eu", 80, "http", $user, $pwd, "WILD_RTK");
#getstream("ntrip.dgpsonline.eu", 2101, "http", $user, $pwd, "WILD_RTK");
getstream("ntrip.dgpsonline.eu", 443, "https", $user, $pwd, "WILD_RTK");

sub printcert
{
  my $cert = $_[0];
  return $cert;
};

sub getstream
{
  my ($server, $port, $protocol, $user, $pwd, $mp) = @_;
  eval
  {
    local $SIG{ALRM} = sub { die "alarm"; };
    alarm(20);
    my $s;
    if($protocol eq "https")
    {
      $s = Net::HTTPS->new(Host => "$server:$port", Timeout => 10);
      if($s)
      {
        print "Cipher:  ".$s->get_cipher()."\n";
        my $cert = $s->get_peer_certificate();
        print "Time:    ".$cert->not_before() . " to " . $cert->not_after(). "\n";
        print "Subject: ".printcert($cert->subject_name())."\n";
        print "Issuer:  ".printcert($cert->issuer_name())."\n";
        print "\n";
      }
    }
    else
    {
      $s = Net::HTTP->new(Host => "$server:$port", Timeout => 10);
    }
    if($s)
    {
      $s->write_request(GET => "/$mp", 'Host' => "$server:$port", Timeout => 10,
      'User-Agent' => "NTRIP ssltestclient.pl/1.0",
      "Ntrip-Version" => 'Ntrip/2.0',
      ($user ? ("Authorization" => "Basic " . encode_base64("$user:$pwd")) : ())
      );
      my ($code, $message, %headers) = $s->read_response_headers;
      print "$code $message\n";
      foreach my $s (keys %headers)
      {
        print "$s: $headers{$s}\n";
      }
      print("\n");
      if($code == 200)
      {
        while(1)
        {
          alarm(10);
          my $n;
          my $buf;
          {
            $n = $s->read_entity_body($buf, 1024);
          }
          last unless $n;
          print $buf;
        }
      }
      else
      {
        while(1)
        {
          my $buf;
          my $n = $s->read_entity_body($buf, 1024);
          last unless $n;
          print $buf;
        }
      }
    }
  };
}
