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
#include <stdlib.h>
#include <common.h>
#include <console.h>
#include <bootretry.h>
#include <cli.h>
#include <command.h>
#include <console.h>
#include <malloc.h>
#include <inttypes.h>
#include <mapmem.h>
#include <asm/io.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

#define OTP_PASSWD			0x349fe38a
#define RETRY				3
#define OTP_REGION_STRAP		1
#define OTP_REGION_CONF			2
#define OTP_REGION_DATA			3
#define OTP_REGION_ALL			4

#define OTP_USAGE			-1
#define OTP_FAILURE			-2
#define OTP_SUCCESS			0

#define OTP_PROG_SKIP			1

#define DISABLE_SECREG_PROG		BIT(0)
#define ENABLE_SEC_BOOT			BIT(1)
#define INIT_PROG_DONE			BIT(2)
#define ENABLE_USERREG_ECC		BIT(3)
#define ENABLE_SECREG_ECC		BIT(4)
#define DISABLE_LOW_SEC_KEY		BIT(5)
#define IGNORE_SEC_BOOT_HWSTRAP		BIT(6)
#define SEC_BOOT_MDOES(x)		(x >> 7)
#define   SEC_MODE1			0x0
#define   SEC_MODE2			0x1
#define OTP_BIT_CELL_MODES(x)		((x >> 8) & 0x3)
#define   SINGLE_CELL_MODE		0x0
#define   DIFFERENTIAL_MODE		0x1
#define   DIFFERENTIAL_REDUDANT_MODE	0x2
#define CRYPTO_MODES(x)			((x >> 10) & 0x3)
#define   CRYPTO_RSA1024		0x0
#define   CRYPTO_RSA2048		0x1
#define   CRYPTO_RSA3072		0x2
#define   CRYPTO_RSA4096		0x3
#define HASH_MODES(x)			((x >> 12) & 0x3)
#define   HASH_SAH224			0x0
#define   HASH_SAH256			0x1
#define   HASH_SAH384			0x2
#define   HASH_SAH512			0x3
#define SECREG_SIZE(x)			((x >> 16) & 0x3f)
#define WRITE_PROTECT_SECREG		BIT(22)
#define WRITE_PROTECT_USERREG		BIT(23)
#define WRITE_PROTECT_CONFREG		BIT(24)
#define WRITE_PROTECT_STRAPREG		BIT(25)
#define ENABLE_COPY_TO_SRAM		BIT(26)
#define ENABLE_IMAGE_ENC		BIT(27)
#define WRITE_PROTECT_KEY_RETIRE	BIT(29)
#define ENABLE_SIPROM_RED		BIT(30)
#define ENABLE_SIPROM_MLOCK		BIT(31)

#define VENDER_ID(x) 			(x & 0xFFFF)
#define KEY_REVISION(x)			((x >> 16) & 0xFFFF)

#define SEC_BOOT_HEADER_OFFSET(x)	(x & 0xFFFF)

#define KEYS_VALID_BITS(x)		(x & 0xff)
#define KEYS_RETIRE_BITS(x)		((x >> 16) & 0xff)

#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

void printProgress(int numerator, int denominator, char *format, ...)
{
	int val = numerator * 100 / denominator;
	int lpad = numerator * PBWIDTH / denominator;
	int rpad = PBWIDTH - lpad;
	char buffer[256];
	va_list aptr;

	va_start(aptr, format);
	vsprintf(buffer, format, aptr);
	va_end(aptr);

	printf("\r%3d%% [%.*s%*s] %s", val, lpad, PBSTR, rpad, "", buffer);
	if (numerator == denominator)
		printf("\n");
}

struct otpstrap {
	int value;
	int option_array[7];
	int remain_times;
	int writeable_option;
	int protected;
};

static void otp_read_data(uint32_t offset, uint32_t *data)
{
	writel(offset, 0x1e6f2010); //Read address
	writel(0x23b1e361, 0x1e6f2004); //trigger read
	udelay(2);
	data[0] = readl(0x1e6f2020);
	data[1] = readl(0x1e6f2024);
}

static void otp_read_config(uint32_t offset, uint32_t *data)
{
	int config_offset;

	config_offset = 0x800;
	config_offset |= (offset / 8) * 0x200;
	config_offset |= (offset % 8) * 0x2;

	writel(config_offset, 0x1e6f2010);  //Read address
	writel(0x23b1e361, 0x1e6f2004); //trigger read
	udelay(2);
	data[0] = readl(0x1e6f2020);
}

static int otp_print_config(uint32_t offset, int dw_count)
{
	int i;
	uint32_t ret[1];

	if (offset + dw_count > 32)
		return OTP_USAGE;
	for (i = offset; i < offset + dw_count; i ++) {
		otp_read_config(i, ret);
		printf("OTPCFG%X: %08X\n", i, ret[0]);
	}
	printf("\n");
	return OTP_SUCCESS;
}

static int otp_print_data(uint32_t offset, int dw_count)
{
	int i;
	uint32_t ret[2];

	if (offset + dw_count > 2048 || offset % 4 != 0)
		return OTP_USAGE;
	for (i = offset; i < offset + dw_count; i += 2) {
		otp_read_data(i, ret);
		if (i % 4 == 0)
			printf("%03X: %08X %08X ", i * 4, ret[0], ret[1]);
		else
			printf("%08X %08X\n", ret[0], ret[1]);

	}
	printf("\n");
	return OTP_SUCCESS;
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

static int verify_bit(uint32_t otp_addr, int bit_offset, int value)
{
	int ret;

	writel(otp_addr, 0x1e6f2010); //Read address
	writel(0x23b1e361, 0x1e6f2004); //trigger read
	udelay(2);
	ret = readl(0x1e6f2020);
	// printf("verify_bit = %x\n", ret);
	if (((ret >> bit_offset) & 1) == value)
		return 0;
	else
		return -1;
}

static uint32_t verify_dw(uint32_t otp_addr, uint32_t *value, uint32_t *keep, uint32_t *compare, int size)
{
	uint32_t ret[2];

	otp_addr &= ~(1 << 15);

	if (otp_addr % 2 == 0)
		writel(otp_addr, 0x1e6f2010); //Read address
	else
		writel(otp_addr - 1, 0x1e6f2010); //Read address
	writel(0x23b1e361, 0x1e6f2004); //trigger read
	udelay(2);
	ret[0] = readl(0x1e6f2020);
	ret[1] = readl(0x1e6f2024);
	if (size == 1) {
		if (otp_addr % 2 == 0) {
			// printf("check %x : %x = %x\n", otp_addr, ret[0], value[0]);
			if ((value[0] & ~keep[0]) == (ret[0] & ~keep[0])) {
				compare[0] = 0;
				return 0;
			} else {
				compare[0] = value[0] ^ ret[0];
				return -1;
			}

		} else {
			// printf("check %x : %x = %x\n", otp_addr, ret[1], value[0]);
			if ((value[0] & ~keep[0]) == (ret[1] & ~keep[0])) {
				compare[0] = ~0;
				return 0;
			} else {
				compare[0] = ~(value[0] ^ ret[1]);
				return -1;
			}
		}
	} else if (size == 2) {
		// otp_addr should be even
		if ((value[0] & ~keep[0]) == (ret[0] & ~keep[0]) && (value[1] & ~keep[1]) == (ret[1] & ~keep[1])) {
			// printf("check[0] %x : %x = %x\n", otp_addr, ret[0], value[0]);
			// printf("check[1] %x : %x = %x\n", otp_addr, ret[1], value[1]);
			compare[0] = 0;
			compare[1] = ~0;
			return 0;
		} else {
			// printf("check[0] %x : %x = %x\n", otp_addr, ret[0], value[0]);
			// printf("check[1] %x : %x = %x\n", otp_addr, ret[1], value[1]);
			compare[0] = value[0] ^ ret[0];
			compare[1] = ~(value[1] ^ ret[1]);
			return -1;
		}
	} else {
		return -1;
	}
}

static void otp_soak(int soak)
{
	if (soak) {
		otp_write(0x3000, 0x4021); // Write MRA
		otp_write(0x5000, 0x1027); // Write MRB
		otp_write(0x1000, 0x4820); // Write MR
		writel(0x041930d4, 0x1e602008); //soak program
	} else {
		otp_write(0x3000, 0x4061); // Write MRA
		otp_write(0x5000, 0x302f); // Write MRB
		otp_write(0x1000, 0x4020); // Write MR
		writel(0x04190760, 0x1e602008); //normal program
	}
}

static void otp_prog_dw(uint32_t value, uint32_t keep, uint32_t prog_address)
{
	int j, bit_value, prog_bit;

	for (j = 0; j < 32; j++) {
		if ((keep >> j) & 0x1)
			continue;
		bit_value = (value >> j) & 0x1;
		if (prog_address % 2 == 0) {
			if (bit_value)
				prog_bit = ~(0x1 << j);
			else
				continue;
		} else {
			prog_address |= 1 << 15;
			if (bit_value)
				continue;
			else
				prog_bit = 0x1 << j;
		}
		otp_prog(prog_address, prog_bit);
	}
}

static int otp_conf_parse(uint32_t *OTPCFG)
{
	int tmp, i;
	int pass = 0;
	uint32_t *OTPCFG_KEEP = &OTPCFG[12];

	if (OTPCFG_KEEP[0] & DISABLE_SECREG_PROG) {
		printf("OTPCFG0-D[0]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG0-D[0]\n");
		if (OTPCFG[0] & DISABLE_SECREG_PROG)
			printf("  Disable Secure Region programming\n");
		else
			printf("  Enable Secure Region programming\n");
	}

	if (OTPCFG_KEEP[0] & ENABLE_SEC_BOOT) {
		printf("OTPCFG0-D[1]\n");
		printf("  Skip\n");
	} else {

		printf("OTPCFG0-D[1]\n");
		if (OTPCFG[0] & ENABLE_SEC_BOOT)
			printf("  Enable Secure Boot\n");
		else
			printf("  Disable Secure Boot\n");
	}

	if (OTPCFG_KEEP[0] & ENABLE_USERREG_ECC) {
		printf("OTPCFG0-D[3]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG0-D[3]\n");
		if (OTPCFG[0] & ENABLE_USERREG_ECC)
			printf("  User region ECC enable\n");
		else
			printf("  User region ECC disable\n");
	}

	if (OTPCFG_KEEP[0] & ENABLE_SECREG_ECC) {
		printf("OTPCFG0-D[4]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG0-D[4]\n");
		if (OTPCFG[0] & ENABLE_SECREG_ECC)
			printf("  Secure Region ECC enable\n");
		else
			printf("  Secure Region ECC disable\n");
	}

	if (OTPCFG_KEEP[0] & DISABLE_LOW_SEC_KEY) {
		printf("OTPCFG0-D[5]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG0-D[5]\n");
		if (OTPCFG[0] & DISABLE_LOW_SEC_KEY)
			printf("  Disable low security key\n");
		else
			printf("  Enable low security key\n");
	}

	if (OTPCFG_KEEP[0] & IGNORE_SEC_BOOT_HWSTRAP) {
		printf("OTPCFG0-D[6]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG0-D[6]\n");
		if (OTPCFG[0] & IGNORE_SEC_BOOT_HWSTRAP)
			printf("  Ignore Secure Boot hardware strap\n");
		else
			printf("  Do not ignore Secure Boot hardware strap\n");
	}

	if (SEC_BOOT_MDOES(OTPCFG_KEEP[0]) == 0x1) {
		printf("OTPCFG0-D[7]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG0-D[7]\n");
		if (SEC_BOOT_MDOES(OTPCFG[0]) == SEC_MODE1)
			printf("  Secure Boot Mode: 1\n");
		else
			printf("  Secure Boot Mode: 2\n");
	}

	if (OTP_BIT_CELL_MODES(OTPCFG_KEEP[0]) == 0x3) {
		printf("OTPCFG0-D[9:8]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG0-D[9:8]\n");
		printf("  OTP bit cell mode : ");
		tmp = OTP_BIT_CELL_MODES(OTPCFG[0]);
		if (tmp == SINGLE_CELL_MODE) {
			printf("Single cell mode (recommended)\n");
		} else if (tmp == DIFFERENTIAL_MODE) {
			printf("Differnetial mode\n");
		} else if (tmp == DIFFERENTIAL_REDUDANT_MODE) {
			printf("Differential-redundant mode\n");
		} else {
			printf("Value error\n");
			return -1;
		}
	}
	if (CRYPTO_MODES(OTPCFG_KEEP[0]) == 0x3) {
		printf("OTPCFG0-D[11:10]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG0-D[11:10]\n");
		printf("  RSA mode : ");
		tmp = CRYPTO_MODES(OTPCFG[0]);
		if (tmp == CRYPTO_RSA1024) {
			printf("RSA1024\n");
		} else if (tmp == CRYPTO_RSA2048) {
			printf("RSA2048\n");
		} else if (tmp == CRYPTO_RSA3072) {
			printf("RSA3072\n");
		} else {
			printf("RSA4096\n");
		}
	}
	if (HASH_MODES(OTPCFG_KEEP[0]) == 0x3) {
		printf("OTPCFG0-D[13:12]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG0-D[13:12]\n");
		printf("  SHA mode : ");
		tmp = HASH_MODES(OTPCFG[0]);
		if (tmp == HASH_SAH224) {
			printf("SHA224\n");
		} else if (tmp == HASH_SAH256) {
			printf("SHA256\n");
		} else if (tmp == HASH_SAH384) {
			printf("SHA384\n");
		} else {
			printf("SHA512\n");
		}
	}

	if (SECREG_SIZE(OTPCFG_KEEP[0]) == 0x3f) {
		printf("OTPCFG0-D[21:16]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG0-D[21:16]\n");
		printf("  Secure Region size (DW): %x\n", SECREG_SIZE(OTPCFG[0]));
	}

	if (OTPCFG_KEEP[0] & WRITE_PROTECT_SECREG) {
		printf("OTPCFG0-D[22]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG0-D[22]\n");
		if (OTPCFG[0] & WRITE_PROTECT_SECREG)
			printf("  Secure Region : Write Protect\n");
		else
			printf("  Secure Region : Writable\n");
	}

	if (OTPCFG_KEEP[0] & WRITE_PROTECT_USERREG) {
		printf("OTPCFG0-D[23]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG0-D[23]\n");
		if (OTPCFG[0] & WRITE_PROTECT_USERREG)
			printf("  User Region : Write Protect\n");
		else
			printf("  User Region : Writable\n");
	}

	if (OTPCFG_KEEP[0] & WRITE_PROTECT_CONFREG) {
		printf("OTPCFG0-D[24]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG0-D[24]\n");
		if (OTPCFG[0] & WRITE_PROTECT_CONFREG)
			printf("  Configure Region : Write Protect\n");
		else
			printf("  Configure Region : Writable\n");
	}

	if (OTPCFG_KEEP[0] & WRITE_PROTECT_STRAPREG) {
		printf("OTPCFG0-D[25]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG0-D[25]\n");
		if (OTPCFG[0] & WRITE_PROTECT_STRAPREG)
			printf("  OTP strap Region : Write Protect\n");
		else
			printf("  OTP strap Region : Writable\n");
	}

	if (OTPCFG_KEEP[0] & ENABLE_COPY_TO_SRAM) {
		printf("OTPCFG0-D[25]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG0-D[26]\n");
		if (OTPCFG[0] & ENABLE_COPY_TO_SRAM)
			printf("  Copy Boot Image to Internal SRAM\n");
		else
			printf("  Disable Copy Boot Image to Internal SRAM\n");
	}
	if (OTPCFG_KEEP[0] & ENABLE_IMAGE_ENC) {
		printf("OTPCFG0-D[27]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG0-D[27]\n");
		if (OTPCFG[0] & ENABLE_IMAGE_ENC)
			printf("  Enable image encryption\n");
		else
			printf("  Disable image encryption\n");
	}

	if (OTPCFG_KEEP[0] & WRITE_PROTECT_KEY_RETIRE) {
		printf("OTPCFG0-D[29]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG0-D[29]\n");
		if (OTPCFG[0] & WRITE_PROTECT_KEY_RETIRE)
			printf("  OTP key retire Region : Write Protect\n");
		else
			printf("  OTP key retire Region : Writable\n");
	}

	if (OTPCFG_KEEP[0] & ENABLE_SIPROM_RED) {
		printf("OTPCFG0-D[30]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG0-D[30]\n");
		if (OTPCFG[0] & ENABLE_SIPROM_RED)
			printf("  SIPROM RED_EN redundancy repair enable\n");
		else
			printf("  SIPROM RED_EN redundancy repair disable\n");
	}

	if (OTPCFG_KEEP[0] & ENABLE_SIPROM_MLOCK) {
		printf("OTPCFG0-D[31]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG0-D[31]\n");
		if (OTPCFG[0] & ENABLE_SIPROM_MLOCK)
			printf("  SIPROM Mlock memory lock enable\n");
		else
			printf("  SIPROM Mlock memory lock disable\n");
	}
	if (SECREG_SIZE(OTPCFG_KEEP[2]) == 0xFFFF) {
		printf("OTPCFG2-D[15:0]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG2-D[15:0]\n");
		printf("  Vender ID : %x\n", VENDER_ID(OTPCFG[2]));
	}

	if (SECREG_SIZE(OTPCFG_KEEP[2]) == 0xFFFF) {
		printf("OTPCFG2-D[31:16]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG2-D[31:16]\n");
		printf("  Key Revision : %x\n", KEY_REVISION(OTPCFG[2]));
	}

	if (SEC_BOOT_HEADER_OFFSET(OTPCFG_KEEP[3]) == 0xFFFF) {
		printf("OTPCFG3-D[15:0]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG3-D[15:0]\n");
		printf("  Secure boot header offset : %x\n",
		       SEC_BOOT_HEADER_OFFSET(OTPCFG[3]));
	}

	if (KEYS_VALID_BITS(OTPCFG_KEEP[4]) == 0xFF) {
		printf("OTPCFG4-D[7:0]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG4-D[7:0]\n");
		tmp = KEYS_VALID_BITS(OTPCFG[4]);
		if (tmp != 0) {
			for (i = 0; i < 7; i++) {
				if (tmp == (1 << i)) {
					pass = i + 1;
				}
			}
		} else {
			pass = 0;
		}
		printf("  Keys valid  : %d\n", pass);
	}
	if (KEYS_RETIRE_BITS(OTPCFG_KEEP[4]) == 0xFF) {
		printf("OTPCFG4-D[23:16]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG4-D[23:16]\n");
		tmp = KEYS_RETIRE_BITS(OTPCFG[4]);
		if (tmp != 0) {
			for (i = 0; i < 7; i++) {
				if (tmp == (1 << i)) {
					pass = i + 1;
				}
			}
		} else {
			pass = 0;
		}
		printf("  Keys Retire ID : %d\n", pass);
	}
	if (OTPCFG_KEEP[5] == 0xFFFFFFFF) {
		printf("OTPCFG5-D[31:0]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG5-D[31:0]\n");
		printf("  User define data, random number low : %x\n", OTPCFG[5]);
	}

	if (OTPCFG_KEEP[6] == 0xFFFFFFFF) {
		printf("OTPCFG6-D[31:0]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG6-D[31:0]\n");
		printf("  User define data, random number high : %x\n", OTPCFG[6]);
	}

	if (OTPCFG_KEEP[8] == 0xFFFFFFFF) {
		printf("OTPCFG8-D[31:0]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG8-D[31:0]\n");
		printf("  Redundancy Repair : %x\n", OTPCFG[8]);
	}

	if (OTPCFG_KEEP[10] == 0xFFFFFFFF) {
		printf("OTPCFG10-D[31:0]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG10-D[31:0]\n");
		printf("  Manifest ID low : %x\n", OTPCFG[10]);
	}

	if (OTPCFG_KEEP[11] == 0xFFFFFFFF) {
		printf("OTPCFG11-D[31:0]\n");
		printf("  Skip\n");
	} else {
		printf("OTPCFG11-D[31:0]\n");
		printf("  Manifest ID high : %x\n", OTPCFG[11]);
	}

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

static int otp_data_parse(uint32_t *buf)
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
		} else if (key_type != 0 && key_type != 1) {
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

static int otp_prog_conf(uint32_t *buf)
{
	int i, k;
	int pass = 0;
	int soak = 0;
	uint32_t prog_address;
	uint32_t data[12];
	uint32_t compare[2];
	uint32_t *buf_keep = &buf[12];
	uint32_t data_masked;
	uint32_t buf_masked;

	printf("Read OTP Config Region:\n");

	printProgress(0, 12, "");
	for (i = 0; i < 12 ; i ++) {
		printProgress(i + 1, 12, "");
		prog_address = 0x800;
		prog_address |= (i / 8) * 0x200;
		prog_address |= (i % 8) * 0x2;
		otp_read_data(prog_address, &data[i]);
	}

	printf("Check writable...\n");
	for (i = 0; i < 12; i++) {
		data_masked = data[i]  & ~buf_keep[i];
		buf_masked  = buf[i] & ~buf_keep[i];
		if (data_masked == buf_masked)
			continue;
		if ((data_masked | buf_masked) == buf_masked) {
			continue;
		} else {
			printf("Input image can't program into OTP, please check.\n");
			printf("OTPCFG[%X] = %x\n", i, data[i]);
			printf("Input [%X] = %x\n", i, buf[i]);
			printf("Mask  [%X] = %x\n", i, ~buf_keep[i]);
			return OTP_FAILURE;
		}
	}

	printf("Start Programing...\n");
	printProgress(0, 12, "");
	otp_soak(0);
	for (i = 0; i < 12; i++) {
		data_masked = data[i]  & ~buf_keep[i];
		buf_masked  = buf[i] & ~buf_keep[i];
		prog_address = 0x800;
		prog_address |= (i / 8) * 0x200;
		prog_address |= (i % 8) * 0x2;
		if (data_masked == buf_masked) {
			printProgress(i + 1, 12, "[%03X]=%08X HIT", prog_address, buf[i]);
			continue;
		}
		if (soak) {
			soak = 0;
			otp_soak(0);
		}
		printProgress(i + 1, 12, "[%03X]=%08X    ", prog_address, buf[i]);

		otp_prog_dw(buf[i], buf_keep[i], prog_address);

		pass = 0;
		for (k = 0; k < RETRY; k++) {
			if (verify_dw(prog_address, &buf[i], &buf_keep[i], compare, 1) != 0) {
				if (soak == 0) {
					soak = 1;
					otp_soak(1);
				}
				otp_prog_dw(compare[0], prog_address, 1);
			} else {
				pass = 1;
				break;
			}
		}
	}

	if (!pass)
		return OTP_FAILURE;

	return OTP_SUCCESS;

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
	int skip = -1;
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
			printf("OTPSTRAP[%X]:\n", i);
		}
		if (bit == otpstrap[i].value) {
			printf("    The value is same as before, skip it.\n");
			if (skip == -1)
				skip = 1;
			continue;
		} else {
			skip = 0;
		}
		if (otpstrap[i].protected == 1) {
			printf("    This bit is protected and is not writable\n");
			fail = 1;
			continue;
		}
		if (otpstrap[i].remain_times == 0) {
			printf("    This bit is no remaining times to write.\n");
			fail = 1;
			continue;
		}
		if (pbit == 1) {
			printf("    This bit will be protected and become non-writable.\n");
		}
		printf("    Write 1 to OTPSTRAP[%X] OPTION[%X], that value becomes from %d to %d.\n", i, otpstrap[i].writeable_option + 1, otpstrap[i].value, otpstrap[i].value ^ 1);
	}
	if (fail == 1)
		return OTP_FAILURE;
	else if (skip == 1)
		return OTP_PROG_SKIP;

	return 0;
}

static int otp_print_strap(int start, int count)
{
	int i, j;
	struct otpstrap otpstrap[64];

	if (start < 0 || start > 64)
		return OTP_USAGE;

	if ((start + count) < 0 || (start + count) > 64)
		return OTP_USAGE;

	otp_strp_status(otpstrap);

	for (i = start; i < start + count; i++) {
		printf("OTPSTRAP[%X]:\n", i);
		printf("  OTP Option value: ");
		for (j = 1; j <= 7; j++)
			printf("[%X]:%X ", j, otpstrap[i].option_array[j - 1]);
		printf("\n");
		printf("  OTP Value: %X\n", otpstrap[i].value);
		printf("  Status:\n");
		if (otpstrap[i].protected == 1) {
			printf("    OTPSTRAP[%X] is protected and is not writable\n", i);
		} else {
			printf("    OTPSTRAP[%X] is not protected ", i);
			if (otpstrap[i].remain_times == 0) {
				printf("and no remaining times to write.\n");
			} else {
				printf("and still can write %d times\n", otpstrap[i].remain_times);
			}
		}
	}

	return OTP_SUCCESS;
}

static int otp_prog_strap(uint32_t *buf)
{
	int i, j;
	uint32_t *strap_keep = buf + 2;
	uint32_t *strap_protect = buf + 4;
	uint32_t prog_bit, prog_address;
	int bit, pbit, kbit, offset;
	int fail = 0;
	int pass = 0;
	int soak = 0;
	struct otpstrap otpstrap[64];

	otp_strp_status(otpstrap);

	otp_soak(0);

	for (i = 0; i < 64; i++) {
		printProgress(i + 1, 64, "");
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

		if (soak) {
			soak = 0;
			otp_soak(0);
		}

		otp_prog(prog_address, prog_bit);

		pass = 0;

		for (j = 0; j < RETRY; j++) {
			if (verify_bit(prog_address, offset, 1) == 0) {
				pass = 1;
				break;
			}
			if (soak == 0) {
				soak = 1;
				otp_soak(1);
			}
			otp_prog(prog_address, prog_bit);
		}
		if (!pass)
			return OTP_FAILURE;

		if (pbit == 0)
			continue;
		prog_address = 0x800;
		if (i < 32)
			prog_address |= 0x60c;
		else
			prog_address |= 0x60e;


		if (soak) {
			soak = 0;
			otp_soak(0);
		}

		otp_prog(prog_address, prog_bit);

		pass = 0;

		for (j = 0; j < RETRY; j++) {

			if (verify_bit(prog_address, offset, 1) == 0) {
				pass = 1;
				break;
			}
			if (soak == 0) {
				soak = 1;
				otp_soak(1);
			}
			otp_prog(prog_address, prog_bit);
		}
		if (!pass)
			return OTP_FAILURE;

	}
	if (fail == 1)
		return OTP_FAILURE;
	else
		return OTP_SUCCESS;

}

static void otp_prog_bit(uint32_t value, uint32_t prog_address, uint32_t bit_offset, int soak)
{
	int prog_bit;

	otp_soak(soak);

	if (prog_address % 2 == 0) {
		if (value)
			prog_bit = ~(0x1 << bit_offset);
		else
			return;
	} else {
		prog_address |= 1 << 15;
		if (!value)
			prog_bit = 0x1 << bit_offset;
		else
			return;
	}
	otp_prog(prog_address, prog_bit);
}

static int otp_prog_data(uint32_t *buf)
{
	int i, k;
	int pass;
	int soak = 0;
	uint32_t prog_address;
	uint32_t data[2048];
	uint32_t compare[2];
	uint32_t *buf_keep = &buf[2048];

	uint32_t data0_masked;
	uint32_t data1_masked;
	uint32_t buf0_masked;
	uint32_t buf1_masked;

	printf("Read OTP Data:\n");

	printProgress(0, 2048, "");
	for (i = 0; i < 2048 ; i += 2) {
		printProgress(i + 2, 2048, "");
		otp_read_data(i, &data[i]);
	}


	printf("Check writable...\n");
	for (i = 0; i < 2048; i++) {
		data0_masked = data[i]  & ~buf_keep[i];
		buf0_masked  = buf[i] & ~buf_keep[i];
		if (data0_masked == buf0_masked)
			continue;
		if (i % 2 == 0) {
			if ((data0_masked | buf0_masked) == buf0_masked) {
				continue;
			} else {
				printf("Input image can't program into OTP, please check.\n");
				printf("OTP_ADDR[%x] = %x\n", i, data[i]);
				printf("Input   [%x] = %x\n", i, buf[i]);
				printf("Mask    [%x] = %x\n", i, ~buf_keep[i]);
				return OTP_FAILURE;
			}
		} else {
			if ((data0_masked & buf0_masked) == buf0_masked) {
				continue;
			} else {
				printf("Input image can't program into OTP, please check.\n");
				printf("OTP_ADDR[%x] = %x\n", i, data[i]);
				printf("Input   [%x] = %x\n", i, buf[i]);
				printf("Mask    [%x] = %x\n", i, ~buf_keep[i]);
				return OTP_FAILURE;
			}
		}
	}

	printf("Start Programing...\n");
	printProgress(0, 2048, "");

	for (i = 0; i < 2048; i += 2) {
		prog_address = i;
		data0_masked = data[i]  & ~buf_keep[i];
		buf0_masked  = buf[i] & ~buf_keep[i];
		data1_masked = data[i + 1]  & ~buf_keep[i + 1];
		buf1_masked  = buf[i + 1] & ~buf_keep[i + 1];
		if ((data0_masked == buf0_masked) && (data1_masked == buf1_masked)) {
			printProgress(i + 2, 2048, "[%03X]=%08X HIT;[%03X]=%08X HIT", prog_address, buf[i], prog_address + 1, buf[i + 1]);
			continue;
		}
		if (soak) {
			soak = 0;
			otp_soak(0);
		}
		if (data1_masked == buf1_masked) {
			printProgress(i + 2, 2048, "[%03X]=%08X    ;[%03X]=%08X HIT", prog_address, buf[i], prog_address + 1, buf[i + 1]);
			otp_prog_dw(buf[i], buf_keep[i], prog_address);
		} else if (data0_masked == buf0_masked) {
			printProgress(i + 2, 2048, "[%03X]=%08X HIT;[%03X]=%08X    ", prog_address, buf[i], prog_address + 1, buf[i + 1]);
			otp_prog_dw(buf[i + 1], buf_keep[i + 1], prog_address + 1);
		} else {
			printProgress(i + 2, 2048, "[%03X]=%08X    ;[%03X]=%08X    ", prog_address, buf[i], prog_address + 1, buf[i + 1]);
			otp_prog_dw(buf[i], buf_keep[i], prog_address);
			otp_prog_dw(buf[i + 1], buf_keep[i + 1], prog_address + 1);
		}

		pass = 0;
		for (k = 0; k < RETRY; k++) {
			if (verify_dw(prog_address, &buf[i], &buf_keep[i], compare, 2) != 0) {
				if (soak == 0) {
					soak = 1;
					otp_soak(1);
				}
				if (compare[0] != 0) {
					otp_prog_dw(compare[0], buf_keep[i], prog_address);
				}
				if (compare[1] != ~0) {
					otp_prog_dw(compare[1], buf_keep[i], prog_address + 1);
				}
			} else {
				pass = 1;
				break;
			}
		}

		if (!pass)
			return OTP_FAILURE;
	}
	return OTP_SUCCESS;

}

static int do_otp_prog(int addr, int byte_size, int nconfirm)
{
	int ret;
	int mode;
	uint32_t *buf;
	uint32_t *data_region = NULL;
	uint32_t *conf_region = NULL;
	uint32_t *strap_region = NULL;

	buf = map_physmem(addr, byte_size, MAP_WRBACK);
	if (!buf) {
		puts("Failed to map physical memory\n");
		return OTP_FAILURE;
	}

	if (((buf[0] >> 29) & 0x7) == 0x7) {
		mode = OTP_REGION_ALL;
		conf_region = &buf[1];
		strap_region = &buf[25];
		data_region = &buf[31];
	} else {
		if (buf[0] & BIT(29)) {
			mode = OTP_REGION_DATA;
			data_region = &buf[31];
		}
		if (buf[0] & BIT(30)) {
			mode = OTP_REGION_CONF;
			strap_region = &buf[25];
		}
		if (buf[0] & BIT(31)) {
			mode = OTP_REGION_STRAP;
			conf_region = &buf[1];
		}
	}
	if (!nconfirm) {
		if (mode == OTP_REGION_CONF) {
			if (otp_conf_parse(conf_region) < 0) {
				printf("OTP config error, please check.\n");
				return OTP_FAILURE;
			}
		} else if (mode == OTP_REGION_DATA) {
			if (otp_data_parse(data_region) < 0) {
				printf("OTP data error, please check.\n");
				return OTP_FAILURE;
			}
		} else if (mode == OTP_REGION_STRAP) {
			ret = otp_strap_parse(strap_region);
			if (ret == OTP_FAILURE) {
				printf("OTP strap error, please check.\n");
				return OTP_FAILURE;
			} else if (ret == OTP_PROG_SKIP) {
				printf("OTP strap skip all\n");
				return OTP_SUCCESS;
			}
		} else if (mode == OTP_REGION_ALL) {
			if (otp_conf_parse(conf_region) < 0) {
				printf("OTP config error, please check.\n");
				return OTP_FAILURE;
			}
			if (otp_strap_parse(strap_region) == OTP_FAILURE) {
				printf("OTP strap error, please check.\n");
				return OTP_FAILURE;
			}
			if (otp_data_parse(data_region) < 0) {
				printf("OTP data error, please check.\n");
				return OTP_FAILURE;
			}
		}
		printf("type \"YES\" (no quotes) to continue:\n");
		if (!confirm_yesno()) {
			printf(" Aborting\n");
			return OTP_FAILURE;
		}
	}
	if (mode == OTP_REGION_CONF) {
		return otp_prog_conf(conf_region);
	} else if (mode == OTP_REGION_STRAP) {
		return otp_prog_strap(strap_region);
	} else if (mode == OTP_REGION_DATA) {
		return otp_prog_data(data_region);
	} else if (mode == OTP_REGION_ALL) {
		printf("programing data region ... ");
		ret = otp_prog_data(data_region);
		if (ret != 0) {
			printf("Error\n");
			return ret;
		} else {
			printf("Done\n");
		}
		printf("programing strap region ... ");
		ret = otp_prog_strap(strap_region);
		if (ret != 0) {
			printf("Error\n");
			return ret;
		} else {
			printf("Done\n");
		}
		printf("programing configuration region ... ");
		ret = otp_prog_conf(conf_region);
		if (ret != 0) {
			printf("Error\n");
			return ret;
		}
		printf("Done\n");
		return OTP_SUCCESS;
	}

	return OTP_USAGE;
}

static int do_otp_prog_bit(int mode, int otp_dw_offset, int bit_offset, int value, int nconfirm)
{
	uint32_t read[2];
	uint32_t strap_buf[6];
	uint32_t prog_address = 0;
	struct otpstrap otpstrap[64];
	int otp_bit;
	int i;
	int pass;
	int ret;

	switch (mode) {
	case OTP_REGION_CONF:
		otp_read_config(otp_dw_offset, read);
		prog_address = 0x800;
		prog_address |= (otp_dw_offset / 8) * 0x200;
		prog_address |= (otp_dw_offset % 8) * 0x2;
		otp_bit = (read[0] >> bit_offset) & 0x1;
		if (otp_bit == value) {
			printf("OTPCFG%X[%X] = %d\n", otp_dw_offset, bit_offset, value);
			printf("No need to program\n");
			return OTP_SUCCESS;
		}
		if (otp_bit == 1 && value == 0) {
			printf("OTPCFG%X[%X] = 1\n", otp_dw_offset, bit_offset);
			printf("OTP is programed, which can't be clean\n");
			return OTP_FAILURE;
		}
		printf("Program OTPCFG%X[%X] to 1\n", otp_dw_offset, bit_offset);
		break;
	case OTP_REGION_DATA:
		prog_address = otp_dw_offset;

		if (otp_dw_offset % 2 == 0) {
			otp_read_data(otp_dw_offset, read);
			otp_bit = (read[0] >> bit_offset) & 0x1;
		} else {
			otp_read_data(otp_dw_offset - 1, read);
			otp_bit = (read[1] >> bit_offset) & 0x1;
		}
		if (otp_bit == value) {
			printf("OTPDATA%X[%X] = %d\n", otp_dw_offset, bit_offset, value);
			printf("No need to program\n");
			return OTP_SUCCESS;
		}
		if (otp_bit == 1 && value == 0) {
			printf("OTPDATA%X[%X] = 1\n", otp_dw_offset, bit_offset);
			printf("OTP is programed, which can't be clean\n");
			return OTP_FAILURE;
		}
		printf("Program OTPDATA%X[%X] to 1\n", otp_dw_offset, bit_offset);
		break;
	case OTP_REGION_STRAP:
		otp_strp_status(otpstrap);
		otp_print_strap(bit_offset, 1);
		if (bit_offset < 32) {
			strap_buf[0] = value << bit_offset;
			strap_buf[2] = ~BIT(bit_offset);
			strap_buf[3] = ~0;
			strap_buf[5] = 0;
			// if (protect)
			// 	strap_buf[4] = BIT(bit_offset);
			// else
			// 	strap_buf[4] = 0;
		} else {
			strap_buf[1] = value << (bit_offset - 32);
			strap_buf[2] = ~0;
			strap_buf[3] = ~BIT(bit_offset - 32);
			strap_buf[4] = 0;
			// if (protect)
			// 	strap_buf[5] = BIT(bit_offset - 32);
			// else
			// 	strap_buf[5] = 0;
		}
		ret = otp_strap_parse(strap_buf);
		if (ret == OTP_FAILURE)
			return OTP_FAILURE;
		else if (ret == OTP_PROG_SKIP)
			return OTP_SUCCESS;

		break;
	}

	if (!nconfirm) {
		printf("type \"YES\" (no quotes) to continue:\n");
		if (!confirm_yesno()) {
			printf(" Aborting\n");
			return OTP_FAILURE;
		}
	}

	switch (mode) {
	case OTP_REGION_STRAP:
		return otp_prog_strap(strap_buf);
	case OTP_REGION_CONF:
	case OTP_REGION_DATA:
		otp_prog_bit(value, prog_address, bit_offset, 0);
		pass = -1;
		for (i = 0; i < RETRY; i++) {
			if (verify_bit(prog_address, bit_offset, value) != 0) {
				otp_prog_bit(value, prog_address, bit_offset, 1);
			} else {
				pass = 0;
				break;
			}
		}
		if (pass == 0)
			return OTP_SUCCESS;
	}

	return OTP_USAGE;
}

static int do_otpread(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	uint32_t offset, count;
	int ret;

	if (argc == 4) {
		offset = simple_strtoul(argv[2], NULL, 16);
		count = simple_strtoul(argv[3], NULL, 16);
	} else if (argc == 3) {
		offset = simple_strtoul(argv[2], NULL, 16);
		count = 1;
	} else {
		return CMD_RET_USAGE;
	}


	if (!strcmp(argv[1], "conf")) {
		writel(OTP_PASSWD, 0x1e6f2000); //password
		ret = otp_print_config(offset, count);
	} else if (!strcmp(argv[1], "data")) {
		writel(OTP_PASSWD, 0x1e6f2000); //password
		ret = otp_print_data(offset, count);
	} else if (!strcmp(argv[1], "strap")) {
		writel(OTP_PASSWD, 0x1e6f2000); //password
		ret = otp_print_strap(offset, count);
	} else {
		return CMD_RET_USAGE;
	}

	if (ret == OTP_SUCCESS)
		return CMD_RET_SUCCESS;
	else
		return CMD_RET_USAGE;

}

static int do_otpprog(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	phys_addr_t addr;
	uint32_t byte_size;
	int ret;

	if (argc == 4) {
		if (strcmp(argv[1], "f"))
			return CMD_RET_USAGE;
		addr = simple_strtoul(argv[2], NULL, 16);
		byte_size = simple_strtoul(argv[3], NULL, 16);
		writel(OTP_PASSWD, 0x1e6f2000); //password
		ret = do_otp_prog(addr, byte_size, 1);
	} else if (argc == 3) {
		addr = simple_strtoul(argv[1], NULL, 16);
		byte_size = simple_strtoul(argv[2], NULL, 16);
		writel(OTP_PASSWD, 0x1e6f2000); //password
		ret = do_otp_prog(addr, byte_size, 0);
	} else {
		return CMD_RET_USAGE;
	}

	if (ret == OTP_SUCCESS)
		return CMD_RET_SUCCESS;
	else if (ret == OTP_FAILURE)
		return CMD_RET_FAILURE;
	else
		return CMD_RET_USAGE;
}

static int do_otppb(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int mode = 0;
	int nconfirm = 0;
	int otp_addr = 0;
	int bit_offset;
	int value;
	int ret;

	if (argc != 4 && argc != 5 && argc != 6)
		return CMD_RET_USAGE;

	/* Drop the pb cmd */
	argc--;
	argv++;

	if (!strcmp(argv[0], "conf"))
		mode = OTP_REGION_CONF;
	else if (!strcmp(argv[0], "strap"))
		mode = OTP_REGION_STRAP;
	else if (!strcmp(argv[0], "data"))
		mode = OTP_REGION_DATA;
	else
		return CMD_RET_USAGE;

	/* Drop the region cmd */
	argc--;
	argv++;

	if (!strcmp(argv[0], "f")) {
		nconfirm = 1;
		/* Drop the force option */
		argc--;
		argv++;
	}

	if (mode == OTP_REGION_STRAP) {
		bit_offset = simple_strtoul(argv[0], NULL, 16);
		value = simple_strtoul(argv[1], NULL, 16);
		if (bit_offset >= 64)
			return CMD_RET_USAGE;
	} else {
		otp_addr = simple_strtoul(argv[0], NULL, 16);
		bit_offset = simple_strtoul(argv[1], NULL, 16);
		value = simple_strtoul(argv[2], NULL, 16);
		if (bit_offset >= 32)
			return CMD_RET_USAGE;
	}
	if (value != 0 && value != 1)
		return CMD_RET_USAGE;

	writel(OTP_PASSWD, 0x1e6f2000); //password
	ret = do_otp_prog_bit(mode, otp_addr, bit_offset, value, nconfirm);

	if (ret == OTP_SUCCESS)
		return CMD_RET_SUCCESS;
	else if (ret == OTP_FAILURE)
		return CMD_RET_FAILURE;
	else
		return CMD_RET_USAGE;
}

static int do_otpcmp(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	phys_addr_t addr;
	int otp_addr = 0;

	if (argc != 3)
		return CMD_RET_USAGE;

	writel(OTP_PASSWD, 0x1e6f2000); //password
	addr = simple_strtoul(argv[1], NULL, 16);
	otp_addr = simple_strtoul(argv[2], NULL, 16);
	if (otp_compare(otp_addr, addr) == 0) {
		printf("Compare pass\n");
		return CMD_RET_SUCCESS;
	} else {
		printf("Compare fail\n");
		return CMD_RET_FAILURE;
	}
}

static cmd_tbl_t cmd_otp[] = {
	U_BOOT_CMD_MKENT(read, 4, 0, do_otpread, "", ""),
	U_BOOT_CMD_MKENT(prog, 4, 0, do_otpprog, "", ""),
	U_BOOT_CMD_MKENT(pb, 6, 0, do_otppb, "", ""),
	U_BOOT_CMD_MKENT(cmp, 3, 0, do_otpcmp, "", ""),

};

static int do_ast_otp(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	cmd_tbl_t *cp;

	cp = find_cmd_tbl(argv[1], cmd_otp, ARRAY_SIZE(cmd_otp));

	/* Drop the mmc command */
	argc--;
	argv++;

	if (cp == NULL || argc > cp->maxargs)
		return CMD_RET_USAGE;
	if (flag == CMD_FLAG_REPEAT && !cmd_is_repeatable(cp))
		return CMD_RET_SUCCESS;

	return cp->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(
	otp, 7, 0,  do_ast_otp,
	"ASPEED One-Time-Programmable sub-system",
	"read conf|data <otp_dw_offset> <dw_count>\n"
	"otp read strap <strap_bit_offset> <bit_count>\n"
	"otp info conf|strap|data <otp_dw_offset> <dw_count>\n"
	"otp prog [f] <addr> <byte_size>\n"
	"otp pb conf|data [f] <otp_dw_offset> <bit_offset> <value>\n"
	"otp pb strap [f] <bit_offset> <value> <protect>\n"
	"otp cmp <addr> <otp_dw_offset>\n"
);
