/*
 * Copyright (c) International Business Machines Corp., 2007,2012
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Author: Kirill Gorelov
 */

/*
 * Adds support for eth0=... and eth1=... parameters in U-Boot to make
 * it possible to specify PHY type and its address.
 *
 * The parameters for the interface config values comply to the
 * following grammar:
 *
 *   CONFIG ::= IFACE = CFG
 *   IFACE ::= 'eth0' | 'eth1'
 *   CFG ::= CFG_FIX | CFG_BCM5221 | CFG_BCM5222 | CFG_BCM5241
 *   CFG_FIX ::= 'nophy', SPEED, DUPLEX
 *   SPEED ::= '10' | '100' | '1000'
 *   DUPLEX ::= 'full' | 'half'
 *   CFG_BCM5221 ::= 'bcm5221'
 *   CFG_BCM5241 ::= 'bcm5241'
 *   CFG_BCM5222 ::= 'bcm5222', OWNER, PHY_ADDRESS
 *   CFG_BCM5461 ::= 'bcm5461'
 *   OWNER ::= IFACE
 *   PHY_ADDRESS ::= '0' | '1'
 *
 * Explanation:
 *   o The BCM5221 and the newer BCM5241 are single phys with phy id of 0.
 *     They are normally operated with the associated EMAC e.g. EMAC0 drives
 *     phy for eth0 and EMAC1 drives the phy for eth1. The syntax currently
 *     does not support an owner or a different phy id than 0.
 *   o The BCM5222 is a dual phy (one chip containing two phys).
 *     It is normally operated by using EMAC0, but there are configurations
 *     where eth0 uses phy id 1 and eth1 uses phy id 0. The normal usage is
 *     eth0 has phy id 0 and eth1 has phy id 1. The syntax above allows both.
 */

#include <common.h>

#include <commproc.h>
#include <miiphy.h>		/* official u-boot interface to the outside */
#include <asm/io.h>
#include <asm/processor.h>
#include <fsp_enet.h>

#define DEBUG
#if defined(DEBUG)
#  define dprintf(fmt, args...)	printf (fmt ,##args)
#else
#  define dprintf(fmt, args...)
#endif

/*
 * Since FSP-1 has not sophisticted ZMII or RMII setup we need this
 * ugly stuff to let the miiphy_getemac_offset function know current
 * EMAC offset. The official U-Boot miiphy interface is lacking
 * parameter for the emac to be used to access the phy!
 */
unsigned long curr_emac_offset = 0;

void miiphy_set_curr_emac_offset(unsigned long emac_offset)
{
	curr_emac_offset = emac_offset;
}

unsigned int miiphy_getemac_offset (void)
{
	return curr_emac_offset;
}

#define MAX_ETHNAME_CHARS	5 /* Maximum number of chars in eth name*/

enum state {
	STATE_P0 = 0,
	STATE_P1 = 1,
	STATE_P2 = 2,
	STATE_EXIT = -1,
};

/*
 * Unfortunately it happened that we had wrong parameters configured
 * for some cards. The code here should help to reduce the likelyhood
 * of those systems to fail by setting up reasonable parameters for
 * the FSP-1 cards. A warning is printed and the problem should be
 * fixed by altering the configuration data in the card's flashes.
 */
static int phyparam_verify_and_correct(int devnum, struct phy_props *props)
{
	/* More fixups to come ... ? */
	/* FIXME Verify setting by probing the phy */
	return 0;
}

/*
 * EMAC4
 */
int emac4_miiphy_read(const char *devname, int addr, int reg, u16 *value)
{
	unsigned long sta_reg = 0; /* STA scratch area */
	unsigned long i;
	unsigned long emac_reg;

	emac_reg = miiphy_getemac_offset();

	/* see if it is ready for 1000 nsec */
	i = 0;

	/* see if it is ready for  sec */
	while ((in32 (EMAC_STACR + emac_reg) & EMAC_STACR_OC) ==
	       EMAC_STACR_OC) {
		udelay (7);
		if (i > 5) {
			printf("PHY STACR[%08lx]=%08lx not ready, continue.\n",
			       EMAC_STACR + emac_reg,
			       in32(EMAC_STACR + emac_reg));
			break;
			/* return -1; */ /* old behaviour was to stop */
		}
		i++;
	}
	sta_reg = reg | EMAC_STACR_READ | EMAC_STACR_OC |
		(addr << STACR_PORTADDR_SHIFT);	/* Phy address */

	out32 (EMAC_STACR + emac_reg, sta_reg);
	/* printf ("%s: EMAC_STACR=0x%08x\n", __func__, sta_reg); */

	sta_reg = in32 (EMAC_STACR + emac_reg);
	i = 0;
	while ((sta_reg & EMAC_STACR_OC) == EMAC_STACR_OC) {
		udelay (7);
		if (i > 5) {
			return -1;
		}
		i++;
		sta_reg = in32 (EMAC_STACR + emac_reg);
	}

	if ((sta_reg & EMAC_STACR_OC) == EMAC_STACR_OC) {
		printf ("PHY read timeout #2!\n");
		return -1;
	}
	if ((sta_reg & EMAC_STACR_PHYE) != 0) {
		printf ("EMAC MDIO PHY error!\n");
		return -1;
	}

	*value = (unsigned short)(sta_reg >> 16);
	return 0;
}

int emac4_mdio_read(struct mii_dev *bus, int port_addr, int dev_addr,
			int regnum) {
	u16 val = 0;
	int rc = 0;
	rc = emac4_miiphy_read(NULL, port_addr, regnum, &val);
	if (rc == 0)
		return val;
	return rc;
}
/*
 * EMAC4
 *
 * write a phy reg and return the value with a rc
 */

int emac4_miiphy_write (const char *devname, int addr, int reg, u16 value)
{
	unsigned long sta_reg = 0; /* STA scratch area */
	unsigned long i;
	unsigned long emac_reg;

	emac_reg = miiphy_getemac_offset ();
	/* see if it is ready for 1000 nsec */
	i = 0;

	while ((in32 (EMAC_STACR + emac_reg) & EMAC_STACR_OC) ==
	       EMAC_STACR_OC) {
		if (i > 5) {
			printf("PHY STACR[%08lx]=%08lx not ready, continue.\n",
			       EMAC_STACR + emac_reg,
			       in32(EMAC_STACR + emac_reg));
			break;
			/* return -1; */ /* old behaviour was to stop */
		}
		udelay (7);
		i++;
	}

	sta_reg = reg | EMAC_STACR_WRITE | EMAC_STACR_OC |
		((unsigned long) addr << STACR_PORTADDR_SHIFT);	/* Phy addr */

	memcpy (&sta_reg, &value, 2); /* put in data */

	/* printf ("%s: EMAC_STACR=0x%08x\n", __func__, sta_reg); */
	out32 (EMAC_STACR + emac_reg, sta_reg);

	/* wait for completion */
	i = 0;
	sta_reg = in32 (EMAC_STACR + emac_reg);
	while ((sta_reg & EMAC_STACR_OC) == EMAC_STACR_OC) {
		udelay (7);
		if (i > 5)
			return -1;
		i++;
		sta_reg = in32 (EMAC_STACR + emac_reg);
	}

	if ((sta_reg & EMAC_STACR_OC) == EMAC_STACR_OC)
		printf("PHY write timeout #2!\n");

	if ((sta_reg & EMAC_STACR_PHYE) != 0) {
		printf ("EMAC MDIO PHY error!\n");
		return -1;
	}
	return 0;
}

int emac4_mdio_write(struct mii_dev *bus, int port_addr, int dev_addr,
			int regnum, u16 value) {
	return emac4_miiphy_write(NULL, port_addr, regnum, value);
}

static int ethparam_recognize(int devnum, char *p, struct phy_props *props)
{
	char *pos = p, *next;
	int len;
	enum state state = STATE_P0;

	while ((pos) && (*pos) && (state != STATE_EXIT)) {
		next = strchr(pos, ',');

		/* If comma not found position to the end of the string */
		if (next == NULL)
			next = p + strlen(p);

		len = (next - pos);

		switch(state) {
		case STATE_P0:
			state++;
			if (!strncmp(pos, "nophy", len))
				props->type = PHY_TYPE_NOPHY;
			else if (!strncmp(pos, "bcm5241", len)) {
				props->type = PHY_TYPE_BCM5241;
				props->addr = 0x00;
			} else if (!strncmp(pos, "bcm5221", len)) {
				props->type = PHY_TYPE_BCM5221;
				props->addr = 0x00;
			} else if (!strncmp(pos, "bcm5461", len)) {
				props->type = PHY_TYPE_BCM5461;
			} else if (!strncmp(pos, "bcm5222", len))
				props->type = PHY_TYPE_BCM5222;
			else
				state = STATE_EXIT;
			break;
		case STATE_P1:
			state++;
			switch (props->type) {
			case PHY_TYPE_NOPHY:
				/* nophy:: get speed */
				if (!strncmp(pos, "10", len))
					props->speed = 10;
				else
				if (!strncmp(pos, "100", len))
					props->speed = 100;
				else
					state = STATE_EXIT;
				break;
			case PHY_TYPE_BCM5222:
				/* bcm5222:: get owner */
				if (!strncmp(pos, "eth0", len))
					props->owner = 0;
				else
				if (!strncmp(pos, "eth1", len))
					props->owner = 1;
				else
					state = STATE_EXIT;
				break;
			default:
				state = STATE_EXIT;
			} /* phy type switch */
			break;
		case STATE_P2:
			state++;
			switch (props->type) {
			case PHY_TYPE_NOPHY:
				/* nophy:: get duplex mode */
				if (!strncmp(pos, "full", len))
					props->duplex = FULL;
				else
				if (!strncmp(pos, "half", len))
					props->duplex = HALF;
				else
					state = STATE_EXIT;
				break;
			case PHY_TYPE_BCM5222:
				/* bcm 5222:: get phy address */
				if (!strncmp(pos, "0", len))
					props->addr = 0;
				else
				if (!strncmp(pos, "1", len))
					props->addr = 1;
				else
					state = STATE_EXIT;

				break;
			default:
				state = STATE_EXIT;
			} /* phy type switch */
			break;

		case STATE_EXIT:
		default:
			break;
		} /* state switch */

		/* Skip the comma we've found */
		if (*next == ',')
			pos = next + 1;
		else
			pos = next;
	}
	return 0;
}

void miiphy_init_phy_params(struct phy_props *props, int devnum)
{
	int rc;
	char *s_eth = NULL;
	char ethx[MAX_ETHNAME_CHARS+1];

	/* Prepare the name of a variable */
	sprintf(ethx,"eth%d", devnum);

	/* Initialize parameters - use reasonable defaults */
	props->type = PHY_TYPE_UNKNOWN;
	props->owner = devnum;

	switch (devnum) {
	case 1:	props->addr = CONFIG_PHY1_ADDR;
		break;
	default:
	case 0:	props->addr = CONFIG_PHY_ADDR;
		break;
	}
	props->speed = 0;	/* probe */
	props->duplex = 0;	/* probe */

	if ((s_eth = env_get(ethx)) != NULL) {
		/* Parse the parameter */
		rc = ethparam_recognize(devnum, s_eth, props);
		if (rc != 0)
			printf("WARNING! eth%d=%s param was not recognized "
			       "correctly\n", devnum, s_eth);
	} else
		printf("WARNING! No eth%d parameter defined. Using defaults\n",
		       devnum);

	rc = phyparam_verify_and_correct(devnum, props);
	if (rc != 0)
		printf("WARNING! Invalid eth%d parameters. Using defaults\n",
		       devnum);

	printf("PHY eth%d type=%d EMAC%d id=%d speed=%d duplex=%d\n",
		devnum, props->type, props->owner, props->addr,
		props->speed, props->duplex);
}

/*
 * Dump out to the screen PHY regs
 */
void miiphy_dump (char *devname, unsigned char addr)
{
	unsigned long i;
	unsigned short data;

	for (i = 0; i < 0x1A; i++) {
		if (emac4_miiphy_read (devname, addr, i, &data)) {
			printf ("read error for reg %lx\n", i);
			return;
		}
		printf ("Phy reg %lx ==> %4x\n", i, data);

		/* jump to the next set of regs */
		if (i == 0x07)
			i = 0x0f;
	}
}

/*
 * (Re)start autonegotiation
 */
int phy_setup_aneg (char *devname, unsigned char addr)
{
	unsigned short ctl, adv;

	/* Setup standard advertise */
	emac4_miiphy_read(devname, addr, MII_ADVERTISE, &adv);
	adv |= (ADVERTISE_LPACK	| ADVERTISE_RFAULT | ADVERTISE_100BASE4 |
		ADVERTISE_100FULL | ADVERTISE_100HALF | ADVERTISE_10FULL |
		ADVERTISE_10HALF);
	emac4_miiphy_write(devname, addr, MII_ADVERTISE, adv);

	/* Start/Restart aneg */
	emac4_miiphy_read(devname, addr, MII_BMCR, &ctl);
	ctl |= (BMCR_ANENABLE | BMCR_ANRESTART);
	emac4_miiphy_write(devname, addr, MII_BMCR, ctl);

	return 0;
}


