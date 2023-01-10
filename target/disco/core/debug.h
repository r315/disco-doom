#ifndef _debug_h_
#define _debug_h_

#include <stdint.h>

void DBG_Init(void);
void DGB_FF_Error(const char *fname, FRESULT err);
void DBG_SD_Error(uint32_t error_flags);
void DBG_DumpSector(uint32_t sector);

#endif /* _debug_h_ */