//==========================================================================
//
//      fatfs1.c
//
//      Test fileio system
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2004 eCosCentric Limited
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):           nickg
// Contributors:        nickg
// Date:                2000-05-25
// Purpose:             Test fileio system
// Description:         This test uses the testfs to check out the initialization
//                      and basic operation of the fileio system
//                      
//                      
//                      
//                      
//                      
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/io_fileio.h>
#include <pkgconf/fs_fat.h>

#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>

#include <cyg/fileio/fileio.h>

#include <cyg/infra/testcase.h>
#include <cyg/infra/diag.h>            // HAL polled output
#include <cyg/fs/fatfs.h>



//==========================================================================

#define SHOW_RESULT( _fn, _res ) \
diag_printf("<FAIL>: " #_fn "() returned %d %s\n", _res, _res<0?strerror(errno):"");

//==========================================================================

#define IOSIZE  100

#define LONGNAME1       "long_file_name_that_should_take_up_more_than_one_directory_entry_1"
#define LONGNAME2       "long_file_name_that_should_take_up_more_than_one_directory_entry_2"


//==========================================================================

#ifndef CYGPKG_LIBC_STRING

char *strcat( char *s1, const char *s2 )
{
    char *s = s1;
    while( *s1 ) s1++;
    while( (*s1++ = *s2++) != 0);
    return s;
}

#endif

//==========================================================================

static void listdir( char *name, int statp, int numexpected, int *numgot )
{
    int err;
    DIR *dirp;
    int num=0;
    
    diag_printf("<INFO>: reading directory %s\n",name);
    
    dirp = opendir( name );
    if( dirp == NULL ) SHOW_RESULT( opendir, -1 );

    for(;;)
    {
        struct dirent *entry = readdir( dirp );
        
        if( entry == NULL )
            break;
        num++;
        diag_printf("<INFO>: entry %14s",entry->d_name);
#ifdef CYGPKG_FS_FAT_RET_DIRENT_DTYPE
        diag_printf(" d_type %2x", entry->d_type);
#endif
        if( statp )
        {
            char fullname[PATH_MAX];
            struct stat sbuf;

            if( name[0] )
            {
                strcpy(fullname, name );
                if( !(name[0] == '/' && name[1] == 0 ) )
                    strcat(fullname, "/" );
            }
            else fullname[0] = 0;
            
            strcat(fullname, entry->d_name );
            
            err = stat( fullname, &sbuf );
            if( err < 0 )
            {
                if( errno == ENOSYS )
                    diag_printf(" <no status available>");
                else SHOW_RESULT( stat, err );
            }
            else
            {
                diag_printf(" [mode %08x ino %08x nlink %d size %ld]",
                            sbuf.st_mode,sbuf.st_ino,sbuf.st_nlink,(long)sbuf.st_size);
            }
#ifdef CYGPKG_FS_FAT_RET_DIRENT_DTYPE
            if ((entry->d_type & S_IFMT) != (sbuf.st_mode & S_IFMT))
              CYG_TEST_FAIL("File mode's don't match between dirent and stat");
#endif
        }

        diag_printf("\n");
    }

    err = closedir( dirp );
    if( err < 0 ) SHOW_RESULT( stat, err );
    if (numexpected >= 0 && num != numexpected)
        CYG_TEST_FAIL("Wrong number of dir entries\n");
    if ( numgot != NULL )
        *numgot = num;
}

//==========================================================================

static void createfile( char *name, size_t size )
{
    char buf[IOSIZE];
    int fd;
    ssize_t wrote;
    int i;
    int err;

    diag_printf("<INFO>: create file %s size %zd \n",name,size);

    err = access( name, F_OK );
    if( err < 0 && errno != EACCES ) SHOW_RESULT( access, err );
    
    for( i = 0; i < IOSIZE; i++ ) buf[i] = i%256;
 
    fd = open( name, O_WRONLY|O_CREAT );
    if( fd < 0 ) SHOW_RESULT( open, fd );

    while( size > 0 )
    {
        ssize_t len = size;
        if ( len > IOSIZE ) len = IOSIZE;
        
        wrote = write( fd, buf, len );
        if( wrote != len ) SHOW_RESULT( write, (int)wrote );        

        size -= wrote;
    }

    err = close( fd );
    if( err < 0 ) SHOW_RESULT( close, err );
}

//==========================================================================

static void maxfile( char *name )
{
    char buf[IOSIZE];
    int fd;
    ssize_t wrote;
    int i;
    int err;
    size_t size = 0;
    size_t prevsize = 0;
    
    diag_printf("<INFO>: create maximal file %s\n",name);
    diag_printf("<INFO>: This may take a few minutes\n");

    err = access( name, F_OK );
    if( err < 0 && errno != EACCES ) SHOW_RESULT( access, err );
    
    for( i = 0; i < IOSIZE; i++ ) buf[i] = i%256;
 
    fd = open( name, O_WRONLY|O_CREAT );
    if( fd < 0 ) SHOW_RESULT( open, fd );

    do
    {
        wrote = write( fd, buf, IOSIZE );
        //if( wrote < 0 ) SHOW_RESULT( write, wrote );        

        if( wrote >= 0 )
            size += wrote;

        if( (size-prevsize) > 100000 )
        {
            diag_printf("<INFO>: size = %zd \n", size);
            prevsize = size;
        }
        
    } while( wrote == IOSIZE );

    diag_printf("<INFO>: file size == %zd\n",size);

    err = close( fd );
    if( err < 0 ) SHOW_RESULT( close, err );
}

//==========================================================================

static void checkfile( char *name )
{
    char buf[IOSIZE];
    int fd;
    ssize_t done;
    int i;
    int err;
    off_t pos = 0;

    diag_printf("<INFO>: check file %s\n",name);
    
    err = access( name, F_OK );
    if( err != 0 ) SHOW_RESULT( access, err );

    fd = open( name, O_RDONLY );
    if( fd < 0 ) SHOW_RESULT( open, fd );

    for(;;)
    {
        done = read( fd, buf, IOSIZE );
        if( done < 0 ) SHOW_RESULT( read, (int)done );

        if( done == 0 ) break;

        for( i = 0; i < done; i++ )
            if( buf[i] != i%256 )
            {
                diag_printf("buf[%ld+%d](%02x) != %02x\n",pos,i,buf[i],i%256);
                CYG_TEST_FAIL("Data read not equal to data written\n");
            }
        
        pos += done;
    }

    err = close( fd );
    if( err < 0 ) SHOW_RESULT( close, err );
}

#ifdef CYGCFG_FS_FAT_USE_ATTRIBUTES
//==========================================================================

static void checkattrib(const char *name, 
                        const cyg_fs_attrib_t test_attrib )
{
    int err;
    cyg_fs_attrib_t file_attrib;

    diag_printf("<INFO>: check attrib %s\n",name);

    err = cyg_fs_get_attrib(name, &file_attrib);
    if( err != 0 ) SHOW_RESULT( stat, err );

    if ( (file_attrib & S_FATFS_ATTRIB) != test_attrib )
        diag_printf("<FAIL>: attrib %s incorrect\n\tExpected %x Was %x\n",
                    name,test_attrib,(file_attrib & S_FATFS_ATTRIB));
}
#endif // CYGCFG_FS_FAT_USE_ATTRIBUTES

//==========================================================================

static void copyfile( char *name2, char *name1 )
{

    int err;
    char buf[IOSIZE];
    int fd1, fd2;
    ssize_t done, wrote;

    diag_printf("<INFO>: copy file %s -> %s\n",name2,name1);

    err = access( name1, F_OK );
    if( err < 0 && errno != EACCES ) SHOW_RESULT( access, err );

    err = access( name2, F_OK );
    if( err != 0 ) SHOW_RESULT( access, err );
    
    fd1 = open( name1, O_WRONLY|O_CREAT );
    if( fd1 < 0 ) SHOW_RESULT( open, fd1 );

    fd2 = open( name2, O_RDONLY );
    if( fd2 < 0 ) SHOW_RESULT( open, fd2 );
    
    for(;;)
    {
        done = read( fd2, buf, IOSIZE );
        if( done < 0 ) SHOW_RESULT( read, (int)done );

        if( done == 0 ) break;

        wrote = write( fd1, buf, done );
        if( wrote != done ) SHOW_RESULT( write, (int) wrote );

        if( wrote != done ) break;
    }

    err = close( fd1 );
    if( err < 0 ) SHOW_RESULT( close, err );

    err = close( fd2 );
    if( err < 0 ) SHOW_RESULT( close, err );
    
}

//==========================================================================

static void comparefiles( char *name2, char *name1 )
{
    int err;
    char buf1[IOSIZE];
    char buf2[IOSIZE];
    int fd1, fd2;
    ssize_t done1, done2;
    int i;

    diag_printf("<INFO>: compare files %s == %s\n",name2,name1);

    err = access( name1, F_OK );
    if( err != 0 ) SHOW_RESULT( access, err );

    err = access( name1, F_OK );
    if( err != 0 ) SHOW_RESULT( access, err );
    
    fd1 = open( name1, O_RDONLY );
    if( fd1 < 0 ) SHOW_RESULT( open, fd1 );

    fd2 = open( name2, O_RDONLY );
    if( fd2 < 0 ) SHOW_RESULT( open, fd2 );
    
    for(;;)
    {
        done1 = read( fd1, buf1, IOSIZE );
        if( done1 < 0 ) SHOW_RESULT( read, (int)done1 );

        done2 = read( fd2, buf2, IOSIZE );
        if( done2 < 0 ) SHOW_RESULT( read, (int)done2 );

        if( done1 != done2 )
            diag_printf("Files different sizes\n");
        
        if( done1 == 0 ) break;

        for( i = 0; i < done1; i++ )
            if( buf1[i] != buf2[i] )
            {
                diag_printf("buf1[%d](%02x) != buf1[%d](%02x)\n",i,buf1[i],i,buf2[i]);
                CYG_TEST_FAIL("Data in files not equal\n");
            }
    }

    err = close( fd1 );
    if( err < 0 ) SHOW_RESULT( close, err );

    err = close( fd2 );
    if( err < 0 ) SHOW_RESULT( close, err );
    
}

//==========================================================================

void checkcwd( const char *cwd )
{
    static char cwdbuf[PATH_MAX];
    char *ret;

    ret = getcwd( cwdbuf, sizeof(cwdbuf));
    if( ret == NULL ) SHOW_RESULT( getcwd, (int)ret );    

    if( strcmp( cwdbuf, cwd ) != 0 )
    {
        diag_printf( "cwdbuf %s cwd %s\n",cwdbuf, cwd );
        CYG_TEST_FAIL( "Current directory mismatch");
    }
}

//==========================================================================
// main

int main( int argc, char **argv )
{
    int err;
    int existingdirents=-1;
#if defined(CYGSEM_FILEIO_BLOCK_USAGE)
    struct cyg_fs_block_usage usage;
#endif

    CYG_TEST_INIT();

    // --------------------------------------------------------------

    err = mount( "/dev/disk0/1", "/", "fatfs" );    
    if( err < 0 ) SHOW_RESULT( mount, err );    

    err = chdir( "/" );
    if( err < 0 ) SHOW_RESULT( chdir, err );

    checkcwd( "/" );
    
    listdir( "/", true, -1, &existingdirents );

    // --------------------------------------------------------------
#if defined(CYGSEM_FILEIO_BLOCK_USAGE)
    err = cyg_fs_getinfo("/", FS_INFO_BLOCK_USAGE, &usage, sizeof(usage));
    if( err < 0 ) SHOW_RESULT( cyg_fs_getinfo, err );
    diag_printf("<INFO>: total size: %6lld blocks, %10lld bytes\n",
		usage.total_blocks, usage.total_blocks * usage.block_size); 
    diag_printf("<INFO>: free size:  %6lld blocks, %10lld bytes\n",
		usage.free_blocks, usage.free_blocks * usage.block_size); 
    diag_printf("<INFO>: block size: %6u bytes\n", usage.block_size);
#endif
    // --------------------------------------------------------------

    createfile( "/foo", 20257 );
    checkfile( "foo" );
    copyfile( "foo", "fee");
    checkfile( "fee" );
    comparefiles( "foo", "/fee" );
    diag_printf("<INFO>: mkdir bar\n");
    err = mkdir( "/bar", 0 );
    if( err < 0 ) SHOW_RESULT( mkdir, err );

    listdir( "/" , true, existingdirents+3, NULL );

    copyfile( "fee", "/bar/fum" );
    checkfile( "bar/fum" );
    comparefiles( "/fee", "bar/fum" );

    diag_printf("<INFO>: cd bar\n");
    err = chdir( "bar" );
    if( err < 0 ) SHOW_RESULT( chdir, err );

    checkcwd( "/bar" );
    
    diag_printf("<INFO>: rename /foo bundy\n");    
    err = rename( "/foo", "bundy" );
    if( err < 0 ) SHOW_RESULT( rename, err );
    
    listdir( "/", true, existingdirents+2, NULL );
    listdir( "" , true, 4, NULL );

    checkfile( "/bar/bundy" );
    comparefiles("/fee", "bundy" );

#if defined(CYGSEM_FILEIO_BLOCK_USAGE)
    err = cyg_fs_getinfo("/", FS_INFO_BLOCK_USAGE, &usage, sizeof(usage));
    if( err < 0 ) SHOW_RESULT( cyg_fs_getinfo, err );
    diag_printf("<INFO>: total size: %6lld blocks, %10lld bytes\n",
		usage.total_blocks, usage.total_blocks * usage.block_size); 
    diag_printf("<INFO>: free size:  %6lld blocks, %10lld bytes\n",
		usage.free_blocks, usage.free_blocks * usage.block_size); 
    diag_printf("<INFO>: block size: %6u bytes\n", usage.block_size);
#endif
    // --------------------------------------------------------------

    diag_printf("<INFO>: unlink fee\n");    
    err = unlink( "/fee" );
    if( err < 0 ) SHOW_RESULT( unlink, err );

    diag_printf("<INFO>: unlink fum\n");        
    err = unlink( "fum" );
    if( err < 0 ) SHOW_RESULT( unlink, err );

    diag_printf("<INFO>: unlink /bar/bundy\n");        
    err = unlink( "/bar/bundy" );
    if( err < 0 ) SHOW_RESULT( unlink, err );

    diag_printf("<INFO>: cd /\n");        
    err = chdir( "/" );
    if( err < 0 ) SHOW_RESULT( chdir, err );

    checkcwd( "/" );
    
    diag_printf("<INFO>: rmdir /bar\n");        
    err = rmdir( "/bar" );
    if( err < 0 ) SHOW_RESULT( rmdir, err );
    
    listdir( "/", false, existingdirents, NULL );

    // --------------------------------------------------------------

#if 0
    diag_printf("<INFO>: mkdir disk2\n");
    err = mkdir( "/disk2", 0 );
    if( err < 0 ) SHOW_RESULT( mkdir, err );
#else
    diag_printf("<INFO>: mount /disk2\n");    
    err = mount( "/dev/disk0/2", "/disk2", "fatfs" );    
    if( err < 0 ) SHOW_RESULT( mount, err );    
#endif
    
    listdir( "/disk2" , true, -1, &existingdirents);
        
    createfile( "/disk2/tinky", 4567 );
    copyfile( "/disk2/tinky", "/disk2/laalaa" );
    checkfile( "/disk2/tinky");
    checkfile( "/disk2/laalaa");
    comparefiles( "/disk2/tinky", "/disk2/laalaa" );

    diag_printf("<INFO>: cd /disk2\n");    
    err = chdir( "/disk2" );
    if( err < 0 ) SHOW_RESULT( chdir, err );

    checkcwd( "/disk2" );
        
    diag_printf("<INFO>: mkdir noonoo\n");    
    err = mkdir( "noonoo", 0 );
    if( err < 0 ) SHOW_RESULT( mkdir, err );

    listdir( "/disk2" , true, existingdirents+3, NULL);

    diag_printf("<INFO>: cd noonoo\n");
    err = chdir( "noonoo" );
    if( err < 0 ) SHOW_RESULT( chdir, err );

    checkcwd( "/disk2/noonoo" );
    
    createfile( "tinky", 6789 );
    checkfile( "tinky" );

    createfile( "dipsy", 34567 );
    checkfile( "dipsy" );
    copyfile( "dipsy", "po" );
    checkfile( "po" );
    comparefiles( "dipsy", "po" );

    listdir( ".", true, 5, NULL );
    listdir( "", true, 5, NULL );
    listdir( "..", true, existingdirents+3, NULL );

    // --------------------------------------------------------------

    diag_printf("<INFO>: unlink tinky\n");    
    err = unlink( "tinky" );
    if( err < 0 ) SHOW_RESULT( unlink, err );

    diag_printf("<INFO>: unlink dipsy\n");    
    err = unlink( "dipsy" );
    if( err < 0 ) SHOW_RESULT( unlink, err );

    diag_printf("<INFO>: unlink po\n");    
    err = unlink( "po" );
    if( err < 0 ) SHOW_RESULT( unlink, err );

    diag_printf("<INFO>: cd ..\n"); 
    err = chdir( ".." );
    if( err < 0 ) SHOW_RESULT( chdir, err );
    checkcwd( "/disk2" );
    
    diag_printf("<INFO>: rmdir noonoo\n"); 
    err = rmdir( "noonoo" );
    if( err < 0 ) SHOW_RESULT( rmdir, err );

    // --------------------------------------------------------------

    err = mkdir( "x", 0 );
    if( err < 0 ) SHOW_RESULT( mkdir, err );
    
    err = mkdir( "x/y", 0 );
    if( err < 0 ) SHOW_RESULT( mkdir, err );
    
    err = mkdir( "x/y/z", 0 );
    if( err < 0 ) SHOW_RESULT( mkdir, err );

    err = mkdir( "x/y/z/w", 0 );
    if( err < 0 ) SHOW_RESULT( mkdir, err );
    
    diag_printf("<INFO>: cd /disk2/x/y/z/w\n");
    err = chdir( "/disk2/x/y/z/w" );
    if( err < 0 ) SHOW_RESULT( chdir, err );
    checkcwd( "/disk2/x/y/z/w" );

    diag_printf("<INFO>: cd ..\n");
    err = chdir( ".." );
    if( err < 0 ) SHOW_RESULT( chdir, err );
    checkcwd( "/disk2/x/y/z" );
    
    diag_printf("<INFO>: cd .\n");
    err = chdir( "." );
    if( err < 0 ) SHOW_RESULT( chdir, err );
    checkcwd( "/disk2/x/y/z" );

    diag_printf("<INFO>: cd ../../y\n");
    err = chdir( "../../y" );
    if( err < 0 ) SHOW_RESULT( chdir, err );
    checkcwd( "/disk2/x/y" );

    diag_printf("<INFO>: cd ../..\n");
    err = chdir( "../.." );
    if( err < 0 ) SHOW_RESULT( chdir, err );
    checkcwd( "/disk2" );

    diag_printf("<INFO>: rmdir x/y/z/w\n"); 
    err = rmdir( "x/y/z/w" );
    if( err < 0 ) SHOW_RESULT( rmdir, err );

    diag_printf("<INFO>: rmdir x/y/z\n"); 
    err = rmdir( "x/y/z" );
    if( err < 0 ) SHOW_RESULT( rmdir, err );

    diag_printf("<INFO>: rmdir x/y\n"); 
    err = rmdir( "x/y" );
    if( err < 0 ) SHOW_RESULT( rmdir, err );

    diag_printf("<INFO>: rmdir x\n"); 
    err = rmdir( "x" );
    if( err < 0 ) SHOW_RESULT( rmdir, err );
    
    // --------------------------------------------------------------

    checkcwd( "/disk2" );
    
    diag_printf("<INFO>: unlink tinky\n");    
    err = unlink( "tinky" );
    if( err < 0 ) SHOW_RESULT( unlink, err );

    diag_printf("<INFO>: unlink laalaa\n");    
    err = unlink( "laalaa" );
    if( err < 0 ) SHOW_RESULT( unlink, err );

    diag_printf("<INFO>: cd /\n");    
    err = chdir( "/" );
    if( err < 0 ) SHOW_RESULT( chdir, err );
    checkcwd( "/" );

    listdir( "/disk2", true, -1, NULL );
    
#if 0
    diag_printf("<INFO>: rmdir dir\n"); 
    err = rmdir( "disk2" );
    if( err < 0 ) SHOW_RESULT( rmdir, err );
#else
    diag_printf("<INFO>: umount /disk2\n");    
    err = umount( "/disk2" );
    if( err < 0 ) SHOW_RESULT( umount, err );    
#endif
    
#ifdef CYGCFG_FS_FAT_USE_ATTRIBUTES
    // Create file
    diag_printf("<INFO>: create /foo\n");
    createfile( "/foo", 20257 );

    // Verify it is created with archive bit set
    checkattrib( "/foo", S_FATFS_ARCHIVE );

    // Make it System
    diag_printf("<INFO>: attrib -A+S /foo\n");
    err = cyg_fs_set_attrib( "/foo", S_FATFS_SYSTEM );
    if( err < 0 ) SHOW_RESULT( chmod system , err );

    // Verify it is now System
    checkattrib( "/foo", S_FATFS_SYSTEM );

    // Make it Hidden
    diag_printf("<INFO>: attrib -S+H /foo\n");
    err = cyg_fs_set_attrib( "/foo", S_FATFS_HIDDEN );
    if( err < 0 ) SHOW_RESULT( chmod system , err );

    // Verify it is now Hidden
    checkattrib( "/foo", S_FATFS_HIDDEN );

    // Make it Read-only
    diag_printf("<INFO>: attrib -H+R /foo\n");
    err = cyg_fs_set_attrib( "/foo", S_FATFS_RDONLY );
    if( err < 0 ) SHOW_RESULT( chmod system , err );

    // Verify it is now Read-only
    checkattrib( "/foo", S_FATFS_RDONLY );

    // Verify we cannot unlink a read-only file
    diag_printf("<INFO>: unlink /foo\n");
    err = unlink( "/foo" );
    if( (err != -1) || (errno != EPERM) ) SHOW_RESULT( unlink, err );

    // Verify we cannot rename a read-only file
    diag_printf("<INFO>: rename /foo bundy\n");
    err = rename( "/foo", "bundy" );
    if( (err != -1) || (errno != EPERM) ) SHOW_RESULT( rename, err );

    // Verify we cannot open read-only file for writing
    int fd;
    diag_printf("<INFO>: create file /foo\n");
    fd = open( "/foo", O_WRONLY );
    if( (err != -1) || (errno != EACCES) ) SHOW_RESULT( open, err );
    if( err > 0 ) close(fd);

    // Make it Normal
    diag_printf("<INFO>: attrib -H /foo\n");
    err = cyg_fs_set_attrib( "/foo", 0 );
    if( err < 0 ) SHOW_RESULT( chmod none , err );

    // Verify it is now nothing
    checkattrib( "/foo", 0 );

    // Now delete our test file
    diag_printf("<INFO>: unlink /foo\n");
    err = unlink( "/foo" );
    if( err < 0 ) SHOW_RESULT( unlink, err );

#endif // CYGCFG_FS_FAT_USE_ATTRIBUTES

    maxfile("file.max");

    listdir( "/", true, -1, NULL );    
        
    diag_printf("<INFO>: unlink file.max\n");    
    err = unlink( "file.max" );
    if( err < 0 ) SHOW_RESULT( unlink, err );    
    diag_printf("<INFO>: umount /\n");    
    err = umount( "/" );
    if( err < 0 ) SHOW_RESULT( umount, err );    
    
    CYG_TEST_PASS_FINISH("fatfs1");
}

// -------------------------------------------------------------------------
// EOF fatfs1.c
