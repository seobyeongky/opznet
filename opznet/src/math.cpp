#include "mathHelper.h"

#include <cmath>
#include <limits>
#include <cassert>

const float QUARTER_PI	= atan(1.0f);
const float HALF_PI		= QUARTER_PI * 2.0f;
const float PI			= QUARTER_PI * 4.0f;
const float DOUBLE_PI	= PI * 2.0f;

const float FLOAT_EPSILON = std::numeric_limits<float>::epsilon();

unsigned int Pow2(unsigned x)
{
	static const unsigned int table[32] = {1U, 2U, 4U, 8U, 16U, 32U, 64U, 128U, 256U,
	512U, 1024U, 2048U, 4096U, 8192U, 16384U, 32768U, 65536U, 131072U, 262144U,
	524288U, 1048576U, 2097152U, 4194304U, 8388608U, 16777216U, 33554432U,
	67108864U, 134217728U, 268435456U, 536870912U, 1073741824U, 2147483648U};
	if(x > 31U)
	{
//		assert(!L"Pow2의 인자값이 너무 큽니다!");
		return 0U;
	}
	else
		return table[x];
}