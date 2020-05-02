/**
*****************************************************************************
**
**  File        : syscalls.c
**
**  Abstract    : System Workbench Minimal System calls file
**
** 		          For more information about which c-functions
**                need which of these lowlevel functions
**                please consult the Newlib libc-manual
**
**  Environment : System Workbench for MCU
**
**  Distribution: The file is distributed �as is,� without any warranty
**                of any kind.
**
*****************************************************************************
**
** <h2><center>&copy; COPYRIGHT(c) 2014 Ac6</center></h2>
**
** Redistribution and use in source and binary forms, with or without modification,
** are permitted provided that the following conditions are met:
**   1. Redistributions of source code must retain the above copyright notice,
**      this list of conditions and the following disclaimer.
**   2. Redistributions in binary form must reproduce the above copyright notice,
**      this list of conditions and the following disclaimer in the documentation
**      and/or other materials provided with the distribution.
**   3. Neither the name of Ac6 nor the names of its contributors
**      may be used to endorse or promote products derived from this software
**      without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
** SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
** CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
** OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
*****************************************************************************
*/

/* Includes */
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/unistd.h>

#include "fatfs.h"

/* Variables */
//#undef errno
extern int errno;
extern int __io_putchar(int ch) __attribute__((weak));
extern int __io_getchar(void) __attribute__((weak));

char *__env[1] = {0};
char **environ = __env;
static char *heap_end = NULL;

#define MAX_FILES 1
static FIL files[MAX_FILES];
static FIL *openfiles[MAX_FILES];
extern UART_HandleTypeDef huart1;

const char *ff_errors [] = {
    "FR_OK",
    "FR_DISK_ERR",			
    "FR_INT_ERR",				
    "FR_NOT_READY",			
    "FR_NO_FILE",				
    "FR_NO_PATH",				
    "FR_INVALID_NAME",		
    "FR_DENIED",				
    "FR_EXIST",				
    "FR_INVALID_OBJECT",		
    "FR_WRITE_PROTECTED",		
    "FR_INVALID_DRIVE",		
    "FR_NOT_ENABLED",			
    "FR_NO_FILESYSTEM",		
    "FR_MKFS_ABORTED",		
    "FR_TIMEOUT",			
    "FR_LOCKED",				
    "FR_NOT_ENOUGH_CORE",		
    "FR_TOO_MANY_OPEN_FILES",	
    "FR_INVALID_PARAMETER"
};

void ff_error(const char *fname, FRESULT err){
    if(err){
        printf("%s: (%d) %s\n", fname, err, ff_errors[err]);
    }
}


/* Functions */
uint32_t memavail(void){
	return (SDRAM_DEVICE_ADDR + SDRAM_DEVICE_SIZE) - (uint32_t)heap_end;
}

caddr_t _sbrk(int incr)
{	
	char *prev_heap_end;

	if (heap_end == 0)
	{
		heap_end = (char *)SDRAM_DEVICE_ADDR + (SDRAM_DEVICE_SIZE >> 1);
	}

	prev_heap_end = heap_end;
	if ((uint32_t)(heap_end + incr) > (SDRAM_DEVICE_ADDR + SDRAM_DEVICE_SIZE))
	{
		//		write(1, "Heap and stack collision\n", 25);
		//		abort();
		errno = ENOMEM;
		return (caddr_t)-1;
	}

	heap_end += incr;

	return (caddr_t)prev_heap_end;
}

int __getpid(void)
{
	return 1;
}

int _kill(int pid, int sig)
{
	errno = EINVAL;
	return -1;
}

void _exit(int status)
{
	_kill(status, -1);
	while (1)
	{
	} /* Make sure we hang here */
}

int _read(int file, char *ptr, int len)
{
	FRESULT fr;
	UINT br;	

	if (file == STDOUT_FILENO){
		errno = EBADF;
		return -1;
	}

	// Skip system files
	file >>= 4;

	if(file > MAX_FILES){
		errno = EMFILE;
		return -1;
	}

	fr = f_read(&files[file - 1], ptr, len, &br);

	if (fr != FR_OK)
	{
		ff_error(__FUNCTION__, fr);
		return -1;
	}
	return br;
}

int _write(int file, char *data, int len)
{
	if ((file != STDOUT_FILENO) && (file != STDERR_FILENO))
	{
		errno = EBADF;
		return -1;
	}

	// 1:stdout 
	// 2:stderr
	if (file == STDOUT_FILENO || file == STDERR_FILENO)
	{
		HAL_StatusTypeDef status = HAL_UART_Transmit(&huart1, (uint8_t *)data, len, 1000);
		// return # of bytes written - as best we can tell
		return (status == HAL_OK ? len : 0);
	}
	return -1;
}

int _close(int file)
{
	// Skip system files
	file >>= 4;

	if(file > MAX_FILES){		
		return FR_TOO_MANY_OPEN_FILES;
	}

	openfiles[file - 1] = NULL;

	f_close(&files[file - 1]);

	return 0;
}

int __fstat(int file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

int __isatty(int file)
{
	return 1;
}

int _lseek(int file, int ptr, int dir)
{

	if(ptr < 0 || dir == SEEK_CUR)
		return FR_INVALID_PARAMETER;

	// Skip system files
	file >>= 4;

	if(file > MAX_FILES){		
		return FR_TOO_MANY_OPEN_FILES;
	}

	FRESULT fr = f_lseek(&files[file - 1], ptr);

	if(fr != FR_OK)
	{
		return fr;
	}

	return FR_OK;
}

int _open(char *path, int flags, ...)
{
	uint8_t fn;

	for(fn = 1; fn <= MAX_FILES; fn++)
	{
		if(openfiles[fn - 1] == NULL){
			FRESULT fr = f_open(&files[fn - 1], path, FA_READ);
			if (fr == FR_OK){
				openfiles[fn - 1] = &files[fn - 1];
				return (fn << 4); // first 16 files are reserved for system
			}
		}
	}
	return -1;
}

int __wait(int *status)
{
	errno = ECHILD;
	return -1;
}

int __unlink(char *name)
{
	errno = ENOENT;
	return -1;
}

int __times(struct tms *buf)
{
	return -1;
}

int __stat(char *file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

int __link(char *old, char *new)
{
	errno = EMLINK;
	return -1;
}

int __fork(void)
{
	errno = EAGAIN;
	return -1;
}

int __execve(char *name, char **argv, char **env)
{
	errno = ENOMEM;
	return -1;
}
