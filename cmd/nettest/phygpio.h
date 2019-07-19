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

#ifndef PHYGPIO_H
#define PHYGPIO_H

//#define PHY_GPIO_ReadValueEveryTime
#define PHY_GPIO_MDC_HalfPeriod                           1
#define PHY_GPIO_DELAY( x )                           udelay( x ) // For Uboot, the unit of udelay() is us.
typedef struct {
	uint32_t                IOAdr_Dat                               ;//__attribute__ ((aligned (4)));
	uint32_t                IOAdr_OE                                ;//__attribute__ ((aligned (4)));
	uint32_t                MDIO_shiftbit                           ;//__attribute__ ((aligned (4)));
	uint32_t                Mask_MDC                                ;//__attribute__ ((aligned (4)));
	uint32_t                Mask_MDIO                               ;//__attribute__ ((aligned (4)));
	uint32_t                Mask_all                                ;//__attribute__ ((aligned (4)));
	uint32_t                Value_Dat                               ;//__attribute__ ((aligned (4)));
	uint32_t                Value_OE                                ;//__attribute__ ((aligned (4)));

	char                 Dat_RdDelay                             ;//__attribute__ ((aligned (4)));
} PHY_GPIOstr;


#endif // PHYGPIO_H
