
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <set>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <queue>
#include <stack>
#include <complex>
#include <limits>

#include "gsl/gsl_rng.h"
#include "gsl/gsl_randist.h"

#undef min
#undef max

namespace  Vixen
{
	namespace Scape
	{
		#include "CVector.h"
		#include "CMatrix.h"
		#include "CTensor.h"
		#include "Definitions.h"
		#include "CTMesh.h"
		#include "NRBM.h"
	}	// end Scape
}	// end Vixen