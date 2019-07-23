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

#define IO_C

#include "swfunc.h"

#include "comminf.h"
#include <command.h>
#include <common.h>
#include <malloc.h>
#include <net.h>
#include <post.h>

//#include "io.h"
#include "typedef.h"

//------------------------------------------------------------
// General Access API
//------------------------------------------------------------
BYTE Check_BEorLN(uint32_t chkaddr)
{
	BYTE ret = BIG_ENDIAN_ADDRESS;
        BYTE i = 0;

        do {
                if (LittleEndianArea[i].StartAddr ==
                    LittleEndianArea[i].EndAddr)
                        break;

                if ((LittleEndianArea[i].StartAddr <= chkaddr) &&
                    (LittleEndianArea[i].EndAddr >= chkaddr)) {
                        ret = LITTLE_ENDIAN_ADDRESS;
                        break;
                }
                i++;
        } while (1);

        return ret;
}

void WriteSOC_DD(uint32_t addr, uint32_t data)
{
	if (Check_BEorLN(addr) == BIG_ENDIAN_ADDRESS)
		*(volatile uint32_t *)(addr) = cpu_to_le32(data);
	else
		*(volatile uint32_t *)(addr) = data;
}

//------------------------------------------------------------
uint32_t ReadSOC_DD(uint32_t addr)
{
	if (Check_BEorLN(addr) == BIG_ENDIAN_ADDRESS)
		return le32_to_cpu(*(volatile uint32_t *)(addr));
	else
		return (*(volatile uint32_t *)(addr));

	return 0;
}
