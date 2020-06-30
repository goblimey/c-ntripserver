#include <concurrency/TimerManager.h>
#include <concurrency/PlatformThreadFactory.h>

#include "Client.h"
#include "RtnetDataHandler.h"

#include <cstdio>
#include <cstring>
extern "C" {
#include <strings.h>
#include <sys/signal.h>
#include <unistd.h>
}

using namespace apache::thrift::concurrency;
using namespace com::gpssolutions::rtnet;

volatile int STOP=0;
void catch_ctrlc(int /*sig_num*/)
{
  static int once=0;
  printf("Caught SIGINT signal, terminating rtnet_sdo_example\n");
  if (once) exit(-1);
  once=1;
  STOP=1;
}

void usage()
{
  printf("rtnet_sdo_example  -H (RTNet host default=localhost)\n");
  printf("                   -p (RTNet SDO port)\n");
  printf("                  [-h (show this usage information)]\n");
}

int main(int argc, char** argv)
{
  signal(SIGINT, catch_ctrlc);
  int ii;
  bool prevArgSet=false;
  bool rtnetPortSet=false;
  enum Argument
  {
    RtnetPort,
    RtnetHost
  };
  Argument prevArg=RtnetPort;
  std::string host;
  long port=0;
  long tmpval;
  char *endptr;
  for (ii=1; ii < argc; ++ii)
  {
    if (prevArgSet)
    {
      switch (prevArg)
      {
      case RtnetPort:
        tmpval = strtol(argv[ii],&endptr,10);
        if (endptr == argv[ii] || tmpval < 1) {
          fprintf(stderr,"Invalid port '%s' given for rtnet port.\n",argv[ii]);
          usage();
          return 1;
        }
        port = tmpval;
        rtnetPortSet=true;
        break;
      case RtnetHost:
        host = argv[ii];
        break;
      }
      prevArgSet=false;
    } else {
      if (!strcmp(argv[ii],"-p")) {
        prevArgSet=true;
        prevArg=RtnetPort;
      } else if (!strcmp(argv[ii],"-H")) {
        prevArgSet=true;
        prevArg=RtnetHost;
      } else if (!strcmp(argv[ii],"-h")) {
        usage();
        return 0;
      } else {
        fprintf(stderr,"Unknown argument %s\n",argv[ii]);
        usage();
        return 1;
      }
    }
  }
  if (!rtnetPortSet) {
    fprintf(stderr,"RTNet port must be set.\n");
    usage();
    return 1;
  }
  boost::shared_ptr<TimerManager> timeMgr(new TimerManager);
  boost::shared_ptr<ThreadFactory> thFactory(new PlatformThreadFactory);
  timeMgr->threadFactory(thFactory);
  timeMgr->start();
  boost::shared_ptr<RtnetDataHandler> dataHandler(new RtnetDataHandler());
  boost::shared_ptr<Client> clnt(new Client(host,port,timeMgr,dataHandler));

  while (!STOP)
    usleep(100);

  // application cleanup
  timeMgr->stop();
  clnt->stop();
  printf("Clean stop\n");
  return 0;
}

