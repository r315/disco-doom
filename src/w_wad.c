// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//	Handles WAD file header, directory, lump I/O.
//
//-----------------------------------------------------------------------------


static const char
rcsid[] = "$Id: w_wad.c,v 1.5 1997/02/03 16:47:57 b1 Exp $";

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>

#include "common.h"
#include "m_swap.h"
#include "doomtype.h"
#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"

//
// GLOBALS
//

// Location of each lump on disk.
lumpinfo_t*     lumpinfo;
static int      numlumps;

static int      reloadlump;
static char*    reloadname;

static void**   lumpcache;
//static int		profilecount;
//static int		info[2500][10];

#if !defined(WIN32)
void strupr (char* s)
{
    while (*s) { *s = toupper(*s); s++; }
}
#endif

/*
int filelength (FILE *handle) 
{ 
    unsigned long pos, size;
    
    pos = ftell(handle);
printf("Position was %lu\n", pos);
    fseek(handle, 0, SEEK_END);
    size = ftell(handle);
    fseek(handle, pos, SEEK_SET);
printf("Size is %lu\n", size);

    return (int)size;
}


void
ExtractFileBase
( char*		path,
  char*		dest )
{
    char*	src;
    int		length;

    src = path + strlen(path) - 1;
    
    // back up until a \ or the start
    while (src != path
	   && *(src-1) != '\\'
	   && *(src-1) != '/')
    {
	src--;
    }
    
    // copy up to eight characters
    memset (dest,0,8);
    length = 0;
    
    while (*src && *src != '.')
    {
	if (++length == 9)
	    I_Error ("Filename base of %s >8 chars",path);

	*dest++ = toupper((int)*src++);
    }
}*/


//
// LUMP BASED ROUTINES.
//

//
// W_AddFile
// All files are optional, but at least one file must be
//  found (PWAD, if all required lumps are present).
// Files with a .wad extension are wadlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.
//
// If filename starts with a tilde, the file is handled
//  specially to allow map reloads.
// But: the reload feature is a fragile hack...

void W_AddFile (char *filename)
{
    unsigned	i;
    int			length;
    int			storehandle;
    wadinfo_t	wadinfo;
    filelump_t*	fileinfo;
    lumpinfo_t* tmplumpinfo;    
    FILE	    *handle;       
    		
    if ( (handle = fopen (filename,"rb")) == NULL){
    	COM_Print ("W_AddFile: couldn't open %s\n",filename);
    	return;
    }

    COM_Print ("\tAdding %s..\n",filename);
   
	// WAD file header
	fread(&wadinfo, 1, sizeof(wadinfo_t), handle);     

	numlumps = wadinfo.numlumps;
	length   = wadinfo.numlumps * sizeof(filelump_t);	
	fileinfo = malloc (length);
    if(!fileinfo){
        I_Error ("W_AddFile: Couldn't allocate fileinfo");
    }

	fseek (handle, wadinfo.infotableofs, SEEK_SET);
	fread (fileinfo, 1, length, handle);	        
 
    // allocate lumpinfo
    lumpinfo = malloc(wadinfo.numlumps * sizeof(lumpinfo_t));    
    if (!lumpinfo){
	   I_Error ("W_AddFile: Couldn't allocate lumpinfo");
    }
	   
    tmplumpinfo = lumpinfo;
	
    storehandle = reloadname ? -1 : (int)handle;    

    for (i=0; i < wadinfo.numlumps; i++,tmplumpinfo++, fileinfo++){
        tmplumpinfo->handle   = storehandle;
        tmplumpinfo->position = fileinfo->filepos;
        tmplumpinfo->size     = fileinfo->size;
        strncpy (tmplumpinfo->name, fileinfo->name, 8);
    }    
	
    if (reloadname)
	   fclose (handle);
	   
    if (!wadinfo.numlumps)	
        I_Error ("W_AddFile: no files found");
    
    // set up caching
    length = wadinfo.numlumps * sizeof(*lumpcache);
        
    lumpcache = malloc (length);
    
    if (!lumpcache){
        I_Error ("W_AddFile: Couldn't allocate lumpcache");
    }
        
    memset (lumpcache, 0, length);
    
//    for(i=0;i<numlumps;i++)printf("lump name[%u] %s\n",i,lumpinfo[i].name);

}

//
// W_Reload
// Flushes any of the reloadable lumps in memory
// and reloads the directory.
//
void W_Reload (void)
{
    wadinfo_t	header;
    int			lumpcount;
    lumpinfo_t*	lump_p;
    unsigned	i;
    FILE		*handle;
    int			length;
    filelump_t*	fileinfo;
	
    if (!reloadname)
	    return;
		
    if ( (handle = fopen (reloadname,"rb")) == NULL)
	    I_Error ("W_Reload: couldn't open %s",reloadname);

    fread (&header, 1, sizeof(header), handle);
    lumpcount = LONG(header.numlumps);
    header.infotableofs = LONG(header.infotableofs);
    length = lumpcount*sizeof(filelump_t);
	fileinfo = (filelump_t*)malloc(length);    
    if(!fileinfo)
        I_Error ("W_Reload: mem allocation fail\n");

    fseek (handle, header.infotableofs, SEEK_SET);
    fread (fileinfo, 1, length, handle);
    
    // Fill in lumpinfo
    lump_p = &lumpinfo[reloadlump];
	
    for (i=reloadlump ; i<reloadlump+lumpcount ; i++,lump_p++, fileinfo++)
    {
	    if (lumpcache[i])
	        Z_Free (lumpcache[i]);

	    lump_p->position = LONG(fileinfo->filepos);
	    lump_p->size = LONG(fileinfo->size);
    }
	
    fclose (handle);
    free(fileinfo);    
}

//
// W_CheckNumForName
// Returns -1 if name not found.
//

int W_CheckNumForName (char* name)
{
    union 
    {
        char s[9];	
        int	x[2];	
    }name8;
    
    int		v1;
    int		v2;
    lumpinfo_t*	lump_p;

    // make the name into two integers for easy compares
    strncpy (name8.s,name,8);

    // in case the name was a fill 8 chars
    name8.s[8] = 0;

    // case insensitive
    strupr (name8.s);		

    v1 = name8.x[0];
    v2 = name8.x[1];

    // scan backwards so patch lump files take precedence
    lump_p = lumpinfo + numlumps;   

    while (lump_p-- != lumpinfo)
    {
	   if (*(int*)lump_p->name == v1 && *(int*)&lump_p->name[4] == v2)	
            return lump_p - lumpinfo;
	
    }

    // TFB. Not found.
    return -1;
}




//
// W_GetNumForName
// Calls W_CheckNumForName, but bombs out if not found.
//
int W_GetNumForName (char* name)
{
    int	i;

    i = W_CheckNumForName (name);
    
    if (i == -1)
      I_Error ("W_GetNumForName: %s not found!", name);
      
    return i;
}


//
// W_LumpLength
// Returns the buffer size needed to load the given lump.
//
int W_LumpLength (int lump)
{
    if (lump >= numlumps)
	I_Error ("W_LumpLength: %i >= numlumps",lump);

    return lumpinfo[lump].size;
}



//
// W_ReadLump
// Loads the lump into the given buffer,
//  which must be >= W_LumpLength().
//
void W_ReadLump (int lump, void* dest)
{
    int		c;
    lumpinfo_t*	l;
    FILE	*handle;
	
    if (lump >= numlumps)
	I_Error ("W_ReadLump: %i >= numlumps",lump);

    l = lumpinfo+lump;
	
    // ??? I_BeginRead ();
	
    if (l->handle == -1)
    {
	    // reloadable file, so use open / read / close
	    if ( (handle = fopen (reloadname,"rb")) == NULL){
	        I_Error ("W_ReadLump: couldn't open %s",reloadname);
        }
    }
    else
	handle = (FILE *)l->handle;
		
    if((c = fseek (handle, l->position, SEEK_SET))){
        I_Error ("W_ReadLump: fail seek to %d", l->position);       
    }

    c = fread (dest, 1, l->size, handle);

    if (c < l->size){
	    I_Error ("W_ReadLump: only read %i of %i on lump %i", c, l->size, lump);
    }

    if (l->handle == -1)
	fclose (handle);
		
    // ??? I_EndRead ();
}

//
// W_CacheLumpNum
//
void* W_CacheLumpNum(int lump, int tag)
{
    if ((unsigned)lump >= numlumps)
	I_Error ("W_CacheLumpNum: %i >= numlumps",lump);
		
    if (!lumpcache[lump])
    {
	// read the lump in
	
	//printf ("cache miss on lump %i\n",lump);
    Z_Malloc (W_LumpLength (lump), tag, &lumpcache[lump]);
	W_ReadLump (lump, lumpcache[lump]);
    }
    else
    {
	//printf ("cache hit on lump %i\n",lump);
	Z_ChangeTag (lumpcache[lump],tag);
    }
	
    return lumpcache[lump];
}

//
// W_CacheLumpName
//
void* W_CacheLumpName(char*	name,int tag)
{
    return W_CacheLumpNum (W_GetNumForName(name), tag);
}

//
// W_Profile
// 
/*
void W_Profile (void)
{
    int		i;
    memblock_t*	block;
    void*	ptr;
    char	ch;
    FILE*	f;
    int		j;
    char	name[9];
	
	
    for (i=0 ; i<numlumps ; i++)
    {	
	ptr = lumpcache[i];
	if (!ptr)
	{
	    ch = ' ';
	    continue;
	}
	else
	{
	    block = (memblock_t *) ( (byte *)ptr - sizeof(memblock_t));
	    if (block->tag < PU_PURGELEVEL)
		ch = 'S';
	    else
		ch = 'P';
	}
	info[i][profilecount] = ch;
    }
    profilecount++;
	
    f = fopen ("waddump.txt","w");
    name[8] = 0;

    for (i=0 ; i<numlumps ; i++)
    {
	memcpy (name,lumpinfo[i].name,8);

	for (j=0 ; j<8 ; j++)
	    if (!name[j])
		break;

	for ( ; j<8 ; j++)
	    name[j] = ' ';

	fprintf (f,"%s ",name);

	for (j=0 ; j<profilecount ; j++)
	    fprintf (f,"    %c",info[i][j]);

	fprintf (f,"\n");
    }
    fclose (f);
}
*/


