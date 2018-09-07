#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
//#include <sys/stat.h>
#include <net/if.h>
#include <errno.h>
#include <linux/sockios.h>

#ifndef SIOCETHTOOL
#define SIOCETHTOOL 0x8946
#endif

/* The forced speed, 10Mb, 100Mb, gigabit, 2.5Gb, 10GbE. */

#define SPEED_10 10
#define SPEED_100 100
#define SPEED_1000 1000
#define SPEED_2500 2500
#define SPEED_10000 10000

/* Duplex, half or full. */

#define DUPLEX_HALF 0x00
#define DUPLEX_FULL 0x01

/* CMDs currently supported */
#define ETHTOOL_GSET 0x00000001 /* Get settings. */
#define ETHTOOL_SSET 0x00000002 /* Set settings. */

/* hack, so we may include kernel's ethtool.h */
//typedef unsigned long long __u64;

typedef __uint32_t __u32; /* ditto */
typedef __uint16_t __u16; /* ditto */
typedef __uint8_t __u8; /* ditto */

/* This should work for both 32 and 64 bit userland. */

struct ethtool_cmd
{
__u32 cmd;
__u32 supported; /* Features this interface supports */
__u32 advertising; /* Features this interface advertises */
__u16 speed; /* The forced speed, 10Mb, 100Mb, gigabit */
__u8 duplex; /* Duplex, half or full */
__u8 port; /* Which connector port */
__u8 phy_address;
__u8 transceiver; /* Which transceiver to use */
__u8 autoneg; /* Enable or disable autonegotiation */
__u32 maxtxpkt; /* Tx pkts before generating tx int */
__u32 maxrxpkt; /* Rx pkts before generating rx int */
__u32 reserved[4];
};

static int dump_ecmd (struct ethtool_cmd *ep);
static int do_gset (int fd, struct ifreq *ifr);

static int
dump_ecmd (struct ethtool_cmd *ep)
{
fprintf (stdout, " Speed: ");
switch (ep->speed)
    {
    case SPEED_10:
      fprintf (stdout, "10Mb/s\n");
      break;
    case SPEED_100:
      fprintf (stdout, "100Mb/s\n");
      break;
    case SPEED_1000:
      fprintf (stdout, "1000Mb/s\n");
      break;
    case SPEED_2500:
      fprintf (stdout, "2500Mb/s\n");
      break;
    case SPEED_10000:
      fprintf (stdout, "10000Mb/s\n");
      break;
    default:
      fprintf (stdout, "Unknown! (%i) Mbps\n", ep->speed);
      break;
    };

fprintf (stdout, " Duplex: ");
switch (ep->duplex)
 {
    case DUPLEX_HALF:
      fprintf (stdout, "Half\n");
      break;
    case DUPLEX_FULL:
      fprintf (stdout, "Full\n");
      break;
    default:
      fprintf (stdout, "Unknown! (%i)\n", ep->duplex);
      break;
    };

fprintf (stdout, " autoneg : %s\n", (ep->autoneg==1)?"on":"off");
return 0;
}

static int
do_gset (int fd, struct ifreq *ifr)
{
int err;
struct ethtool_cmd ecmd;
int allfail = 1;
fprintf (stdout, "Settings for %s:\n", ifr->ifr_name);
ecmd.cmd = ETHTOOL_GSET;
ifr->ifr_ifru.ifru_data = (caddr_t) & ecmd;
err = ioctl (fd, SIOCETHTOOL, ifr);
if (err == 0)
    {

     err = dump_ecmd (&ecmd);
      if (err)
        return err;
      allfail = 0;
  }

else if (errno != EOPNOTSUPP)
    {
      perror ("Cannot get device settings");
    }

if (allfail)
    {
      fprintf (stdout, "No data available\n");
return 75;
    }
close(fd);
return 0;
}

int
main (int argc, char *argv[])
{

char *device;
int fd;
struct ifreq ifr;
if (argc > 2)
    return 1;
if (argc < 2)
    {
        printf ("input interface name \n");
        return 1;
    }
if (argc == 0)
    return 1;
device = argv[1];
strcpy (ifr.ifr_name, device);
/* Open control socket. */
fd = socket (AF_INET, SOCK_DGRAM, 0);
if (fd < 0)
   {
      perror ("Cannot get control socket");
      return 70;
    }
do_gset (fd, &ifr);
    return 0;
}








