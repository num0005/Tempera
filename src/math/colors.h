/*
    You should have received a copy of the GNU General Public License
    along with tempera.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <precompile.h>

//TODO: Investigate why this is the way it is.
struct argb_color;

// 0 - 255, represents a color
struct rgb_color {
	byte red;
	byte green;
	byte blue;
};
STAT_ASSRT(rgb_color, 0x3);

// 0 - 255, represents a color
struct argb_color {
	byte      alpha;
	rgb_color rgb;
};
STAT_ASSRT(argb_color, 0x4);

// represents a color, range: 0 - 1
struct real_rgb_color {
	float red;
	float green;
	float blue;
};
STAT_ASSRT(real_rgb_color, 0xC);

// represents a color, range: 0 - 1
struct real_argb_color {
	float          alpha;
	real_rgb_color rgb;
};
STAT_ASSRT(real_argb_color, 0x10);

//#define vect float
struct vect2 : public D3DXVECTOR2 {
	using D3DXVECTOR2::D3DXVECTOR2;

	void DumpData(bool toConsole, const char *Preceding = nullptr) {
		if (Preceding) {
			Print(toConsole, "%s\tx: %.3f, y: %.3f\n", Preceding, this->x, this->y);
			return;
		}

		Print(toConsole, "\tx: %.3f, y: %.3f\n", this->x, this->y);
	}
};

STAT_ASSRT(vect2, sizeof(D3DXVECTOR2));

// #define vect3 D3DXVECTOR3

struct vect3 : public D3DXVECTOR3 {
	using D3DXVECTOR3::D3DXVECTOR3;
	// vect3() : D3DXVECTOR3() {}
	//
	// vect3(float x, float y, float z) : D3DXVECTOR3(x, y, z) {}

	void DumpData(bool toConsole, const char *Preceding = nullptr) {
		if (Preceding) {
			Print(toConsole, "%s\tx: %.3f, y: %.3f z: %.3f\n", Preceding, this->x, this->y, this->z);
			return;
		}

		Print(toConsole, "\tx: %.3f, y: %.3f z: %.3f\n", this->x, this->y, this->z);
	}

};

STAT_ASSRT(vect3, sizeof(D3DXVECTOR3));

//-------------------------
struct HaloColor {
	float a;
	float r;
	float g;
	float b;

	//HaloColor constexpr(float a, float r, float g, float b);
	HaloColor(float a, float r, float g, float b);
};

const auto hGreen = HaloColor(1,0,1,0);
const auto hBlue  = HaloColor(1,0,0,1);
const auto hRed   = HaloColor(1,1,0,0);

class CMath {
public:

	float Get2dDistance(float x, float y);

	float GetAngleDistance(vect3 local, vect3 toTarget);

	vect2 GetAngleToTarget(vect3 local, vect3 target);
};