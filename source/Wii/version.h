#ifndef _VERSION_H
#define _VERSION_H

#include "revision.inc"

#if REVISION_NUMBER==169
#define VERSION_AS_STRING "V1.0.3"
#elif REVISION_NUMBER==148
#define VERSION_AS_STRING "V1.0.2"
#elif REVISION_NUMBER==138
#define VERSION_AS_STRING "V1.0.1"
#elif REVISION_NUMBER==109
#define VERSION_AS_STRING "V1.0RC"
#elif REVISION_NUMBER==122
#define VERSION_AS_STRING "V1.0"
#else
#define VERSION_AS_STRING "Build "REVISION_NUMBER_STRING
#endif

#endif
