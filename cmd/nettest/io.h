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

#ifndef IO_H
#define IO_H

#include "swfunc.h"

//
// Macro
//
#define ob(p,d)	outp(p,d)
#define ib(p) inp(p)


void WriteSOC_DD(uint32_t addr, uint32_t data);
uint32_t ReadSOC_DD(uint32_t addr);
#endif
