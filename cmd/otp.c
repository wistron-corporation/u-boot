/*
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <common.h>
#include <console.h>
#include <bootretry.h>
#include <cli.h>
#include <command.h>
#include <console.h>

#include <inttypes.h>
#include <mapmem.h>
#include <asm/io.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

#define OTP_PASSWD	0x349fe38a
#define RETRY		3
#define MODE_CONF	1
#define MODE_STRAP	2
#define MODE_DATA	3
#define MODE_ALL	4

struct otpstrap {
	int value;
	int option_array[7];
	int remain_times;
	int writeable_option;
	int protected;
};

static int otp_read_data(uint32_t offset, uint32_t *data)
{
	writel(offset, 0x1e6f2010); //Read address
	writel(0x23b1e361, 0x1e6f2004); //trigger read
	udelay(2);
	data[0] = readl(0x1e6f2020);
	data[1] = readl(0x1e6f2024);
	return 1;
}

static int otp_read_config(uint32_t offset, uint32_t *data)
{
	int config_offset;

	config_offset = 0x800;
	config_offset |= (offset / 8) * 0x200;
	config_offset |= (offset % 8) * 0x2;

	writel(config_offset, 0x1e6f2010);  //Read address
	writel(0x23b1e361, 0x1e6f2004); //trigger read
	udelay(2);
	data[0] = readl(0x1e6f2020);

	return 1;
}

static int otp_print_config(uint32_t offset, int dw_count)
{
	int i;
	uint32_t ret[1];

	if (offset + dw_count > 32)
		return -1;
	for (i = offset; i < offset + dw_count; i ++) {
		otp_read_config(i, ret);
		printf("OTPCFG%d: %08X\n", i, ret[0]);
	}
	printf("\n");
	return 1;
}

static int otp_print_data(uint32_t offset, int dw_count)
{
	int i;
	uint32_t ret[2];

	if (offset + dw_count > 2048 || offset % 4 != 0)
		return -1;
	for (i = offset; i < offset + dw_count; i += 2) {
		otp_read_data(i, ret);
		if (i % 4 == 0)
			printf("%03X: %08X %08X ", i * 4, ret[0], ret[1]);
		else
			printf("%08X %08X\n", ret[0], ret[1]);

	}
	printf("\n");
	return 1;
}

static int otp_compare(uint32_t otp_addr, uint32_t addr)
{
	uint32_t ret;
	uint32_t *buf;

	buf = map_physmem(addr, 16, MAP_WRBACK);
	printf("%08X\n", buf[0]);
	printf("%08X\n", buf[1]);
	printf("%08X\n", buf[2]);
	printf("%08X\n", buf[3]);
	writel(otp_addr, 0x1e6f2010); //Compare address
	writel(buf[0], 0x1e6f2020); //Compare data 1
	writel(buf[1], 0x1e6f2024); //Compare data 2
	writel(buf[2], 0x1e6f2028); //Compare data 3
	writel(buf[3], 0x1e6f202c); //Compare data 4
	writel(0x23b1e363, 0x1e6f2004); //Compare command
	udelay(10);
	ret = readl(0x1e6f2014); //Compare command
	if (ret & 0x1)
		return 0;
	else
		return -1;
}

static void otp_write(uint32_t otp_addr, uint32_t data)
{
	writel(otp_addr, 0x1e6f2010); //write address
	writel(data, 0x1e6f2020); //write data
	writel(0x23b1e362, 0x1e6f2004); //write command
	udelay(100);
}

static void otp_prog(uint32_t otp_addr, uint32_t prog_bit)
{
	writel(otp_addr, 0x1e6f2010); //write address
	writel(prog_bit, 0x1e6f2020); //write data
	writel(0x23b1e364, 0x1e6f2004); //write command
	udelay(85);
}

static int prog_verify(uint32_t otp_addr, int bit_offset, int value)
{
	int ret;

	writel(otp_addr, 0x1e6f2010); //Read address
	writel(0x23b1e361, 0x1e6f2004); //trigger read
	udelay(2);
	ret = readl(0x1e6f2020);
	// printf("prog_verify = %x\n", ret);
	if (((ret >> bit_offset) & 1) == value)
		return 0;
	else
		return -1;
}

static int otp_conf_parse(uint32_t *OTPCFG)
{
	int tmp, i, pass;

	printf("OTPCFG0-D[0]\n");
	if (OTPCFG[0] & (1))
		printf("  Disable Secure Region programming\n");
	else
		printf("  Enable Secure Region programming\n");
	printf("OTPCFG0-D[1]\n");
	if (OTPCFG[0] & (1 << 1))
		printf("  Enable Secure Boot\n");
	else
		printf("  Disable Secure Boot\n");
	printf("OTPCFG0-D[3]\n");
	if (OTPCFG[0] & (1 << 3))
		printf("  User region ECC enable\n");
	else
		printf("  User region ECC disable\n");
	printf("OTPCFG0-D[4]\n");
	if (OTPCFG[0] & (1 << 4))
		printf("  Secure Region ECC enable\n");
	else
		printf("  Secure Region ECC disable\n");
	printf("OTPCFG0-D[5]\n");
	if (OTPCFG[0] & (1 << 5))
		printf("  Disable low security key\n");
	else
		printf("  Enable low security key\n");
	printf("OTPCFG0-D[6]\n");
	if (OTPCFG[0] & (1 << 6))
		printf("  Ignore Secure Boot hardware strap\n");
	else
		printf("  Do not ignore Secure Boot hardware strap\n");
	printf("OTPCFG0-D[7]\n");
	printf("  Secure Boot Mode: %d\n", (OTPCFG[0] >> 7) & 1);
	printf("OTPCFG0-D[9:8]\n");
	printf("  OTP bit cell mode : ");
	tmp = ((OTPCFG[0] >> 8) & 3);
	if (tmp == 0) {
		printf("Single cell mode (recommended)\n");
	} else if (tmp == 1) {
		printf("Differnetial mode\n");
	} else if (tmp == 2) {
		printf("Differential-redundant mode\n");
	} else {
		printf("Value error\n");
		return -1;
	}
	printf("OTPCFG0-D[11:10]\n");
	printf("  RSA mode : ");
	tmp = ((OTPCFG[0] >> 10) & 3);
	if (tmp == 0) {
		printf("RSA1024\n");
	} else if (tmp == 1) {
		printf("RSA2048\n");
	} else if (tmp == 2) {
		printf("RSA3072\n");
	} else {
		printf("RSA4096\n");
	}
	printf("OTPCFG0-D[13:12]\n");
	printf("  SHA mode : ");
	tmp = ((OTPCFG[0] >> 12) & 3);
	if (tmp == 0) {
		printf("SHA224\n");
	} else if (tmp == 1) {
		printf("SHA256\n");
	} else if (tmp == 2) {
		printf("SHA384\n");
	} else {
		printf("SHA512\n");
	}

	printf("OTPCFG0-D[21:16]\n");
	tmp = ((OTPCFG[0] >> 16) & 0x3F);
	printf("  Secure Region size (DW): %x\n", tmp);

	printf("OTPCFG0-D[22]\n");
	if (OTPCFG[0] & (1 << 22))
		printf("  Secure Region : Write Protect\n");
	else
		printf("  Secure Region : Writable\n");
	printf("OTPCFG0-D[23]\n");
	if (OTPCFG[0] & (1 << 23))
		printf("  User Region : Write Protect\n");
	else
		printf("  User Region : Writable\n");
	printf("OTPCFG0-D[24]\n");
	if (OTPCFG[0] & (1 << 24))
		printf("  Configure Region : Write Protect\n");
	else
		printf("  Configure Region : Writable\n");
	printf("OTPCFG0-D[25]\n");
	if (OTPCFG[0] & (1 << 7))
		printf("  OTP strap Region : Write Protect\n");
	else
		printf("  OTP strap Region : Writable\n");
	printf("OTPCFG0-D[26]\n");
	if (OTPCFG[0] & (1 << 26))
		printf("  Copy Boot Image to Internal SRAM\n");
	else
		printf("  Disable Copy Boot Image to Internal SRAM\n");
	printf("OTPCFG0-D[27]\n");
	if (OTPCFG[0] & (1 << 27))
		printf("  Enable image encryption\n");
	else
		printf("  Disable image encryption\n");
	printf("OTPCFG0-D[29]\n");
	if (OTPCFG[0] & (1 << 29))
		printf("  OTP key retire Region : Write Protect\n");
	else
		printf("  OTP key retire Region : Writable\n");
	printf("OTPCFG0-D[30]\n");
	if (OTPCFG[0] & (1 << 30))
		printf("  SIPROM RED_EN redundancy repair enable\n");
	else
		printf("  SIPROM RED_EN redundancy repair disable\n");
	printf("OTPCFG0-D[31]\n");
	if (OTPCFG[0] & (1 << 31))
		printf("  SIPROM Mlock memory lock enable\n");
	else
		printf("  SIPROM Mlock memory lock disable\n");

	printf("OTPCFG2-D[15:0]\n");
	tmp = (OTPCFG[2] & 0xFFFF);
	printf("  Vender ID : %x\n", tmp);

	printf("OTPCFG2-D[31:16]\n");
	tmp = ((OTPCFG[2] >> 16) & 0xFFFF);
	printf("  Key Revision : %x\n", tmp);

	printf("OTPCFG3-D[15:0]\n");
	tmp = (OTPCFG[3] & 0xFFFF);
	printf("  Secure boot header offset : %x\n", tmp);

	printf("OTPCFG4-D[7:0]\n");
	tmp = (OTPCFG[4] & 0xff);
	pass = -1;
	for (i = 0; i < 7; i++) {
		if (tmp == (1 << i)) {
			if (pass != -1) {
				printf("  Keys valid ID value error : %x\n", tmp);
				return -1;
			}
			pass = i;
		}
	}
	printf("  Keys valid ID : %d\n", pass);

	printf("OTPCFG4-D[23:16]\n");
	tmp = ((OTPCFG[4] >> 16) & 0xff);
	pass = -1;
	for (i = 0; i < 7; i++) {
		if (tmp == (1 << i)) {
			if (pass != -1) {
				printf("  Keys Retire ID value error : %x\n", tmp);
				return -1;
			}
			pass = i;
		}
	}
	printf("  Keys Retire ID : %d\n", pass);

	printf("OTPCFG5-D[31:0]\n");
	printf("  User define data, random number low : %x\n", OTPCFG[5]);

	printf("OTPCFG6-D[31:0]\n");
	printf("  User define data, random number high : %x\n", OTPCFG[6]);

	printf("OTPCFG8-D[31:0]\n");
	printf("  Redundancy Repair : %x\n", OTPCFG[8]);

	printf("OTPCFG10-D[31:0]\n");
	printf("  Manifest ID low : %x\n", OTPCFG[10]);

	printf("OTPCFG11-D[31:0]\n");
	printf("  Manifest ID high : %x\n", OTPCFG[11]);
	return 0;

}

static void buf_print(char *buf, int len)
{
	int i;
	printf("      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
	for (i = 0; i < len; i++) {
		if (i % 16 == 0) {
			printf("%04X: ", i);
		}
		printf("%02X ", buf[i]);
		if ((i + 1) % 16 == 0) {
			printf("\n");
		}
	}
}

static int otp_data_parse(uint32_t *buf, int dw_count)
{
	int key_id, key_offset, last, key_type, key_length, exp_length;
	char *byte_buf;
	int i = 0, len = 0;
	byte_buf = (char *)buf;
	while (1) {
		key_id = buf[i] & 0x7;
		key_offset = buf[i] & 0x1ff8;
		last = (buf[i] >> 13) & 1;
		key_type = (buf[i] >> 14) & 0xf;
		key_length = (buf[i] >> 18) & 0x3;
		exp_length = (buf[i] >> 20) & 0xfff;
		printf("Key[%d]:\n", i);
		printf("Key Type: ");
		switch (key_type) {
		case 0:
			printf("AES-256 as OEM platform key for image encryption/decryption\n");
			break;
		case 1:
			printf("AES-256 as secret vault key\n");
			break;
		case 4:
			printf("HMAC as encrypted OEM HMAC keys in Mode 1\n");
			break;
		case 8:
			printf("RSA-public as OEM DSS public keys in Mode 2\n");
			break;
		case 9:
			printf("RSA-public as SOC public key\n");
			break;
		case 10:
			printf("RSA-public as AES key decryption key\n");
			break;
		case 13:
			printf("RSA-private as SOC private key\n");
			break;
		case 14:
			printf("RSA-private as AES key decryption key\n");
			break;
		default:
			printf("key_type error: %x\n", key_type);
			return -1;
		}
		if (key_type == 4) {
			printf("HMAC SHA Type: ");
			switch (key_length) {
			case 0:
				printf("HMAC(SHA224)\n");
				break;
			case 1:
				printf("HMAC(SHA256)\n");
				break;
			case 2:
				printf("HMAC(SHA384)\n");
				break;
			case 3:
				printf("HMAC(SHA512)\n");
				break;
			}
		} else if (key_type != 0 || key_type != 1) {
			printf("RSA SHA Type: ");
			switch (key_length) {
			case 0:
				printf("RSA1024\n");
				len = 0x100;
				break;
			case 1:
				printf("RSA2048\n");
				len = 0x200;
				break;
			case 2:
				printf("RSA3072\n");
				len = 0x300;
				break;
			case 3:
				printf("RSA4096\n");
				len = 0x400;
				break;
			}
			printf("RSA exponent bit length: %d\n", exp_length);
		}
		if (key_type == 4 || key_type == 8)
			printf("Key Number ID: %d\n", key_id);
		printf("Key Value:\n");
		if (key_type == 4) {
			buf_print(&byte_buf[key_offset], 0x40);
		} else if (key_type == 0 || key_type == 1) {
			printf("AES Key:\n");
			buf_print(&byte_buf[key_offset], 0x20);
			printf("AES IV:\n");
			buf_print(&byte_buf[key_offset + 0x20], 0x10);

		} else {
			printf("RSA mod:\n");
			buf_print(&byte_buf[key_offset], len / 2);
			printf("RSA exp:\n");
			buf_print(&byte_buf[key_offset + (len / 2)], len / 2);
		}
		if (last)
			break;
		i++;
	}
	return 0;
}

static int otp_prog_conf(uint32_t *buf, int otp_addr, int dw_count)
{
	int i, j, k, bit_value;
	int pass, soak;
	uint32_t prog_bit, prog_address;

	otp_write(0x3000, 0x4061); // Write MRA
	otp_write(0x5000, 0x302f); // Write MRB
	otp_write(0x1000, 0x4020); // Write MR
	for (i = 0; i < dw_count; i++) {
		prog_address = 0x800;
		prog_address |= ((i + otp_addr) / 8) * 0x200;
		prog_address |= ((i + otp_addr) % 8) * 0x2;
		for (j = 0; j < 32; j++) {
			bit_value = (buf[i] >> j) & 0x1;
			if (bit_value)
				prog_bit = ~(0x1 << j);
			else
				continue;
			pass = 0;
			soak = 0;
			for (k = 0; k < RETRY; k++) {
				if (!soak) {
					writel(0x04190760, 0x1e602008); //normal program
					otp_prog(prog_address, prog_bit);
					if (prog_verify(prog_address, j, bit_value) == 0) {
						pass = 1;
						break;
					}
					soak = 1;
				}
				writel(0x041930d4, 0x1e602008); //soak program
				otp_prog(prog_address, prog_bit);
				if (prog_verify(prog_address, j, bit_value) == 0) {
					pass = 1;
					break;
				}
			}
			if (!pass)
				return -1;
		}
	}
	return 0;
}

static void otp_strp_status(struct otpstrap *otpstrap)
{
	uint32_t OTPSTRAP_RAW[2];
	int i, j;

	for (j = 0; j < 64; j++) {
		otpstrap[j].value = 0;
		otpstrap[j].remain_times = 7;
		otpstrap[j].writeable_option = -1;
		otpstrap[j].protected = 0;
	}

	for (i = 16; i < 30; i += 2) {
		int option = (i - 16) / 2;
		otp_read_config(i, &OTPSTRAP_RAW[0]);
		otp_read_config(i + 1, &OTPSTRAP_RAW[1]);
		for (j = 0; j < 32; j++) {
			char bit_value = ((OTPSTRAP_RAW[0] >> j) & 0x1);
			if ((bit_value == 0) && (otpstrap[j].writeable_option == -1)) {
				otpstrap[j].writeable_option = option;
			}
			if (bit_value == 1)
				otpstrap[j].remain_times --;
			otpstrap[j].value ^= bit_value;
			otpstrap[j].option_array[option] = bit_value;
		}
		for (j = 32; j < 64; j++) {
			char bit_value = ((OTPSTRAP_RAW[1] >> (j - 32)) & 0x1);
			if ((bit_value == 0) && (otpstrap[j].writeable_option == -1)) {
				otpstrap[j].writeable_option = option;
			}
			if (bit_value == 1)
				otpstrap[j].remain_times --;
			otpstrap[j].value ^= bit_value;
			otpstrap[j].option_array[option] = bit_value;
		}
	}
	otp_read_config(30, &OTPSTRAP_RAW[0]);
	otp_read_config(31, &OTPSTRAP_RAW[1]);
	for (j = 0; j < 32; j++) {
		if (((OTPSTRAP_RAW[0] >> j) & 0x1) == 1)
			otpstrap[j].protected = 1;
	}
	for (j = 32; j < 64; j++) {
		if (((OTPSTRAP_RAW[1] >> (j - 32)) & 0x1) == 1)
			otpstrap[j].protected = 1;
	}
}

static int otp_strap_parse(uint32_t *buf)
{
	int i;
	uint32_t *strap_keep = buf + 2;
	uint32_t *strap_protect = buf + 4;
	int bit, pbit, kbit;
	int fail = 0;
	struct otpstrap otpstrap[64];

	otp_strp_status(otpstrap);
	for (i = 0; i < 64; i++) {
		if (i < 32) {
			bit = (buf[0] >> i) & 0x1;
			kbit = (strap_keep[0] >> i) & 0x1;
			pbit = (strap_protect[0] >> i) & 0x1;
		} else {
			bit = (buf[1] >> (i - 32)) & 0x1;
			kbit = (strap_keep[1] >> (i - 32)) & 0x1;
			pbit = (strap_protect[1] >> (i - 32)) & 0x1;
		}

		if (kbit == 1) {
			continue;
		} else {
			printf("OTPSTRAP[%d]:\n", i);
		}
		if (bit == otpstrap[i].value) {
			printf("    The value is same as before, skip it.\n");
			continue;
		}
		if (otpstrap[i].protected == 1) {
			printf("    This bit is protected and is not writable\n");
			fail = 1;
			continue;
		}
		if (otpstrap[i].remain_times == 0) {
			printf("    This bit is no remaining number of times to write.\n");
			fail = 1;
			continue;
		}
		if (pbit == 1) {
			printf("    This bit will be protected and become non-writable.\n");
		}
		printf("    Write 1 to OTPSTRAP[%d] OPTION[%d], that value becomes frome %d to %d.\n", i, otpstrap[i].writeable_option + 1, otpstrap[i].value, otpstrap[i].value ^ 1);
	}
	if (fail == 1)
		return -1;
	else
		return 0;
}

static void otp_print_strap(void)
{
	int i, j;
	struct otpstrap otpstrap[64];

	otp_strp_status(otpstrap);

	for (i = 0; i < 64; i++) {
		printf("OTPSTRAP[%d]:\n", i);
		printf("  OTP Option value: ");
		for (j = 1; j <= 7; j++)
			printf("[%d]:%d ", j, otpstrap[i].option_array[j - 1]);
		printf("\n");
		printf("  OTP Value: %d\n", otpstrap[i].value);
		printf("  Status:\n");
		if (otpstrap[i].protected == 1) {
			printf("    OTPSTRAP[%d] is protected and is not writable\n", i);
		} else {
			printf("    OTPSTRAP[%d] is not protected ", i);
			if (otpstrap[i].remain_times == 0) {
				printf("and no remaining number of times to write.\n");
			} else {
				printf("and still can write %d number of times\n", otpstrap[i].remain_times);
			}
		}
	}
}

static int otp_prog_strap(uint32_t *buf)
{
	int i, j;
	uint32_t *strap_keep = buf + 2;
	uint32_t *strap_protect = buf + 4;
	uint32_t prog_bit, prog_address;
	int bit, pbit, kbit, offset;
	int fail = 0;
	int pass, soak;
	struct otpstrap otpstrap[64];

	otp_strp_status(otpstrap);

	otp_write(0x3000, 0x4061); // Write MRA
	otp_write(0x5000, 0x302f); // Write MRB
	otp_write(0x1000, 0x4020); // Write MR
	for (i = 0; i < 64; i++) {
		prog_address = 0x800;
		if (i < 32) {
			offset = i;
			bit = (buf[0] >> offset) & 0x1;
			kbit = (strap_keep[0] >> offset) & 0x1;
			pbit = (strap_protect[0] >> offset) & 0x1;
			prog_address |= ((otpstrap[i].writeable_option * 2 + 16) / 8) * 0x200;
			prog_address |= ((otpstrap[i].writeable_option * 2 + 16) % 8) * 0x2;

		} else {
			offset = (i - 32);
			bit = (buf[1] >> offset) & 0x1;
			kbit = (strap_keep[1] >> offset) & 0x1;
			pbit = (strap_protect[1] >> offset) & 0x1;
			prog_address |= ((otpstrap[i].writeable_option * 2 + 17) / 8) * 0x200;
			prog_address |= ((otpstrap[i].writeable_option * 2 + 17) % 8) * 0x2;
		}
		prog_bit = ~(0x1 << offset);

		if (kbit == 1) {
			continue;
		}
		if (bit == otpstrap[i].value) {
			continue;
		}
		if (otpstrap[i].protected == 1) {
			fail = 1;
			continue;
		}
		if (otpstrap[i].remain_times == 0) {
			fail = 1;
			continue;
		}
		pass = 0;
		soak = 0;
		for (j = 0; j < RETRY; j++) {
			if (!soak) {
				writel(0x04190760, 0x1e602008); //normal program
				otp_prog(prog_address, prog_bit);
				if (prog_verify(prog_address, offset, 1) == 0) {
					pass = 1;
					break;
				}
				soak = 1;
			}
			writel(0x041930d4, 0x1e602008); //soak program
			otp_prog(prog_address, prog_bit);
			if (prog_verify(prog_address, offset, 1) == 0) {
				pass = 1;
				break;
			}
		}
		if (!pass)
			return -1;

		if (pbit == 0)
			continue;
		prog_address = 0x800;
		if (i < 32)
			prog_address |= 0x60c;
		else
			prog_address |= 0x60e;

		for (j = 0; j < RETRY; j++) {
			if (!soak) {
				writel(0x04190760, 0x1e602008); //normal program
				otp_prog(prog_address, prog_bit);
				if (prog_verify(prog_address, offset, 1) == 0) {
					pass = 1;
					break;
				}
				soak = 1;
			}
			writel(0x041930d4, 0x1e602008); //soak program
			otp_prog(prog_address, prog_bit);
			if (prog_verify(prog_address, offset, 1) == 0) {
				pass = 1;
				break;
			}
		}
		if (!pass)
			return -1;

	}
	if (fail == 1)
		return -1;
	else
		return 0;

}

static int otp_prog_data(uint32_t *buf, int otp_addr, int dw_count)
{
	int i, j, k, bit_value;
	int pass, soak;
	uint32_t prog_bit, prog_address;

	otp_write(0x3000, 0x4061); // Write MRA
	otp_write(0x5000, 0x302f); // Write MRB
	otp_write(0x1000, 0x4020); // Write MR
	for (i = 0; i < dw_count; i++) {
		prog_address = i + otp_addr;
		for (j = 0; j < 32; j++) {
			bit_value = (buf[i] >> j) & 0x1;
			if (prog_address % 2 == 0) {
				prog_address |= 1 << 15;
				if (bit_value)
					prog_bit = ~(0x1 << j);
				else
					continue;
			} else {
				prog_address |= 1 << 15;
				// printf("bit_value = %x\n", bit_value);
				if (bit_value)
					continue;
				else
					prog_bit = 0x1 << j;
			}
			pass = 0;
			soak = 0;
			for (k = 0; k < RETRY; k++) {
				if (!soak) {
					writel(0x04190760, 0x1e602008); //normal program
					// printf("prog_address = %x\n", prog_address);
					// printf("prog_bit = %x\n", prog_bit);
					otp_prog(prog_address, prog_bit);
					if (prog_verify(prog_address, j, bit_value) == 0) {
						pass = 1;
						break;
					}
					soak = 1;
				}
				writel(0x041930d4, 0x1e602008); //soak program
				otp_prog(prog_address, prog_bit);
				if (prog_verify(prog_address, j, bit_value) == 0) {
					pass = 1;
					break;
				}
			}
			if (!pass)
				return -1;
		}
	}
	return 0;
}

static int do_otp_prog(int mode, int addr, int otp_addr, int dw_count, int nconfirm)
{
	int ret;
	uint32_t *buf;

	buf = map_physmem(addr, dw_count * 4, MAP_WRBACK);
	if (!buf) {
		puts("Failed to map physical memory\n");
		return 1;
	}
	if (!nconfirm) {
		if (mode == MODE_CONF) {
			if (otp_conf_parse(buf) < 0) {
				printf("OTP config error, please check.\n");
				return -1;
			}
		} else if (mode == MODE_DATA) {
			if (otp_data_parse(buf, dw_count) < 0) {
				printf("OTP data error, please check.\n");
				return -1;
			}
		} else if (mode == MODE_STRAP) {
			if (otp_strap_parse(buf) < 0) {
				printf("OTP strap error, please check.\n");
				return -1;
			}
		} else if (mode == MODE_ALL) {
			if (otp_conf_parse(buf) < 0) {
				printf("OTP config error, please check.\n");
				return -1;
			}
			if (otp_strap_parse(&buf[12]) < 0) {
				printf("OTP strap error, please check.\n");
				return -1;
			}
			if (otp_data_parse(&buf[18], dw_count - 18) < 0) {
				printf("OTP data error, please check.\n");
				return -1;
			}
		}
		printf("type \"YES\" (no quotes) to continue:\n");
		if (!confirm_yesno()) {
			printf(" Aborting\n");
			return 1;
		}
	}
	if (mode == MODE_CONF) {
		return otp_prog_conf(buf, otp_addr, dw_count);
	} else if (mode == MODE_STRAP) {
		return otp_prog_strap(buf);
	} else if (mode == MODE_DATA) {
		return otp_prog_data(buf, otp_addr, dw_count);
	} else if (mode == MODE_ALL) {
		printf("programing data region ... ");
		ret = otp_prog_data(&buf[16], 0, dw_count - 18);
		if (ret < 0) {
			printf("Error\n");
			return ret;
		} else {
			printf("Done\n");
		}
		printf("programing strap region ... ");
		ret = otp_prog_strap(&buf[12]);
		if (ret < 0) {
			printf("Error\n");
			return ret;
		} else {
			printf("Done\n");
		}
		printf("programing configuration region ... ");
		ret = otp_prog_conf(buf, 0, 12);
		if (ret < 0) {
			printf("Error\n");
			return ret;
		}
		printf("Done\n");
		return ret;
	}
	return 0;
}
static int do_ast_otp(cmd_tbl_t *cmdtp, int flag, int argc,
		      char *const argv[])
{
	char *cmd;
	int mode = 0;
	int nconfirm = 0;
	uint32_t addr, dw_count, otp_addr;



	if (argc < 2) {
usage:
		return CMD_RET_USAGE;
	}

	cmd = argv[1];
	if (!strcmp(cmd, "read")) {
		if (!strcmp(argv[2], "conf"))
			mode = MODE_CONF;
		else if (!strcmp(argv[2], "data"))
			mode = MODE_DATA;
		else if (!strcmp(argv[2], "strap"))
			mode = MODE_STRAP;
		else
			goto usage;

		writel(OTP_PASSWD, 0x1e6f2000); //password
		otp_addr = simple_strtoul(argv[3], NULL, 16);
		dw_count = simple_strtoul(argv[4], NULL, 16);
		if (mode == MODE_CONF) {
			otp_print_config(otp_addr, dw_count);
		} else if (mode == MODE_DATA) {
			otp_print_data(otp_addr, dw_count);
		} else if (mode == MODE_STRAP) {
			otp_print_strap();
		}
	} else if (!strcmp(cmd, "prog")) {
		if (!strcmp(argv[2], "conf"))
			mode = MODE_CONF;
		else if (!strcmp(argv[2], "strap"))
			mode = MODE_STRAP;
		else if (!strcmp(argv[2], "data"))
			mode = MODE_DATA;
		else if (!strcmp(argv[2], "all"))
			mode = MODE_ALL;
		else
			goto usage;

		if (!strcmp(argv[3], "f"))
			nconfirm = 1;
		writel(OTP_PASSWD, 0x1e6f2000); //password
		addr = simple_strtoul(argv[3 + nconfirm], NULL, 16);
		otp_addr = simple_strtoul(argv[4 + nconfirm], NULL, 16);
		dw_count = simple_strtoul(argv[5 + nconfirm], NULL, 16);
		return do_otp_prog(mode, addr, otp_addr, dw_count, nconfirm);
	} else if (!strcmp(cmd, "comp")) {
		writel(OTP_PASSWD, 0x1e6f2000); //password
		addr = simple_strtoul(argv[2], NULL, 16);
		otp_addr = simple_strtoul(argv[3], NULL, 16);
		if (otp_compare(otp_addr, addr) >= 0) {
			printf("Compare pass\n");
		} else {
			printf("Compare fail\n");
		}
	} else {
		goto usage;
	}


	return 0;
}


U_BOOT_CMD(
	otp, 7, 0,  do_ast_otp,
	"ASPEED One-Time-Programmable sub-system",
	"read conf|strap|data <otp_addr> <dw_count>\n"
	"otp prog conf|strap|data|all [f] <addr> <otp_addr> <dw_count>\n"
	"otp comp <addr> <otp_addr>"
);
