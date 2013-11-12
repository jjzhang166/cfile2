/*
 * cfile.c
 * This file is part of The PaulWay Libraries
 *
 * Copyright (C) 2006 Paul Wayper <paulway@mabula.net>
 * Copyright (C) 2012 Peter Miller
 *
 * The PaulWay Libraries are free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * The PaulWay Libraries are distributed in the hope that they will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <errno.h>
#include <talloc.h>
#include <lzma.h>

#include "cfile_private.h"
#include "cfile_buffer.h"
#include "cfile_xz.h"

/* Predeclare function calls */
off_t   xz_size(cfile *fp);
bool    xz_eof(cfile *fp);
char   *xz_gets(cfile *fp, char *str, size_t len);
ssize_t xz_read(cfile *fp, void *ptr, size_t size, size_t num);
ssize_t xz_write(cfile *fp, const void *ptr, size_t size, size_t num);
int     xz_flush(cfile *fp);
int     xz_close(cfile *fp);

/*! \brief The xz file structure
 *
 * We only need to store the actual (zlib) file pointer.
 */
typedef struct cfile_xz {
    cfile inherited; /*< our inherited function table */
    FILE *xf;        /*< the actual xz file - just a standard handle */
    lzma_stream stream; /*< the LZMA stream information */
    cfile_buffer *buffer; /*< our buffer structure */
} cfile_xz;

static const cfile_vtable xz_cfile_table;

/*! The size of the character buffer for reading lines from xz files.
 *
 *  This isn't really a file cache, just a way of saving us single-byte
 *  calls to bzread.
 */
#define XZ_BUFFER_SIZE 4096

/*! \brief Read callback function to read more data for buffer
 * 
 * This provides uncompressed data to the generic buffer implementation.
 */

size_t xz_read_into_buffer(void *private, const char* buffer, size_t size);
size_t xz_read_into_buffer(void *private, const char* buffer, size_t size) {
    cfile_xz *cfxp = (cfile_xz *)private;

}

/*! \brief Open a xz file for reading or writing
 *
 *  Open the given file using the given mode.  Opens the file and
 *  returns a cfile handle to it.  Mode must start with 'r' or 'w'
 *  to read or write (respectively) - other modes are not expected
 *  to work.
 *
 * \return A successfully created file handle, or NULL on failure.
 */
cfile *xz_open(const char *name, /*!< The name of the file to open */
               const char *mode) /*!< "r" to specify reading, "w" for writing. */
{
}

/*! \brief Returns the _uncompressed_ file size
 *
 * \param fp The file handle to check
 * \return The number of bytes in the uncompressed file.
 */

off_t xz_size(cfile *fp) {
    cfile_xz *cfxp = (cfile_xz *)fp;
}

/*! \brief Returns true if we've reached the end of the file being read.
 *
 *  bzlib doesn't seem to correctly return BZ_STREAM_END
 *  when the stream has actually reached its end, so we have to check
 *  another way - whether the last buffer read was zero bytes long.
 * \param fp The file handle to check.
 * \return True (1) if the file has reached EOF, False (0) if not.
 */

bool xz_eof(cfile *fp) {
    cfile_xz *cfxp = (cfile_xz *)fp;
}

/*! \brief An implementation of fgetc for xz files.
 *
 *  liblzma does not implement any of the 'low level' string functions.
 *  In order to support treating an xz file as a 'real' file, we
 *  we need to provide fgets (for the cfgetline function, if nothing else).
 *  The stdio.c implementation relies on fgetc to get one character at a
 *  time, but trying to consume one byte at a time from an xz file is
 *  difficult.  We use our cfile buffer handler function to
 *  handle the block reads and supply us with a character at a time.
 * \param fp The file to read from.
 * \return the character read, or EOF (-1).
 */
static int xz_fgetc(cfile *fp) {
    cfile_xz *cfxp = (cfile_xz *)fp;
    return buf_fgetc(cfxp->buffer, (void *)cfxp);
}

/*! \brief Get a string from the file, up to a maximum length or newline.
 *
 *  bzlib doesn't provide an equivalent to gets, so we have to copy the
 *  implementation from stdio.c and use it here, referring to our own
 *  bz_fgetc function.
 * \param fp The file handle to read from.
 * \param str An array of characters to read the file contents into.
 * \param len The maximum length, plus one, of the string to read.  In
 *  other words, if this is 10, then fgets will read a maximum of nine
 *  characters from the file.  The character after the last character
 *  read is always set to \\0 to terminate the string.  The newline
 *  character is kept on the line if there was room to read it.
 * \see bz_fgetc
 * \return A pointer to the string thus read.
 */
 
char *xz_gets(cfile *fp, char *str, size_t len) {
    /*cfile_xz *cfxp = (cfile_xz *)fp;*/
    /* Implementation pulled from glibc's stdio.c */
    char *ptr = str;
    int ch;

    if (len <= 0) return NULL;

    while (--len) {
        if ((ch = bz_fgetc(fp)) == EOF) {
            if (ptr == str) return NULL;
            break;
        }

        if ((*ptr++ = ch) == '\n') break;
    }

    *ptr = '\0';
    return str;
}

/*! \brief Print a formatted string to the file, from another function
 *
 *  The standard vfprintf implementation.  For those people that have
 *  to receive a '...' argument in their own function and send it to
 *  a cfile.
 *
 * \param fp The file handle to write to.
 * \param fmt The format string to print.
 * \param ap The compiled va_list of parameters to print.
 * \return The success of the file write operation.
 * \todo Should we be reusing a buffer rather than allocating one each time?
 */

int xz_vprintf(cfile *fp, const char *fmt, va_list ap)
  __attribute ((format (printf, 2, 0)));

int xz_vprintf(cfile *fp, const char *fmt, va_list ap) {
    cfile_xz *cfxp = (cfile_xz *)fp;
    int rtn;
    char *buf = talloc_vasprintf(fp, fmt, ap);
    rtn = BZ2_bzwrite(cfxp->bp, buf, strlen(buf));
    talloc_free(buf);
    return rtn;
}

/*! \brief Read a block of data from the file.
 *
 *  Reads a given number of structures of a specified size from the
 *  file into the memory pointer given.  The destination memory must
 *  be allocated first.  Some read functions only specify one size,
 *  we use two here because that's what fread requires (and it's
 *  better for the programmer anyway IMHO).
 * \param fp The file handle to read from.
 * \param ptr The memory to write into.
 * \param size The size of each structure in bytes.
 * \param num The number of structures to read.
 * \return The success of the file read operation.
 */
 
ssize_t xz_read(cfile *fp, void *ptr, size_t size, size_t num) {
    cfile_xz *cfxp = (cfile_xz *)fp;
    return BZ2_bzread(cfxp->bp, ptr, size * num);
}

/*! \brief Write a block of data from the file.
 *
 *  Writes a given number of structures of a specified size into the
 *  file from the memory pointer given.
 * \param fp The file handle to write into.
 * \param ptr The memory to read from.
 * \param size The size of each structure in bytes.
 * \param num The number of structures to write.
 * \return The success of the file write operation.
 */
 
ssize_t xz_write(cfile *fp, const void *ptr, size_t size, size_t num) {
    cfile_xz *cfxp = (cfile_xz *)fp;
    ssize_t rtn = BZ2_bzwrite(cfxp->bp, (void *)ptr, size * num);
    /* talloc_free(my_ptr); */
    return rtn;
}

/*! \brief Flush the file's output buffer.
 *
 *  This function flushes any data passed to write or printf but not
 *  yet written to disk.  If the file is being read, it has no effect.
 * \param fp The file handle to flush.
 * \return the success of the file flush operation.
 * \note for gzip files, under certain compression methods, flushing
 *  may result in lower compression performance.  We use Z_SYNC_FLUSH
 *  to write to the nearest byte boundary without unduly impacting
 *  compression.
 */
 
int xz_flush(cfile *fp) {
    cfile_xz *cfxp = (cfile_xz *)fp;
    return BZ2_bzflush(cfxp->bp);
}

/*! \brief Close the given file handle.
 *
 *  This function frees the memory allocated for the file handle and
 *  closes the associated file.
 * \param fp The file handle to close.
 * \return the success of the file close operation.
 */
 
int xz_close(cfile *fp) {
    cfile_xz *cfxp = (cfile_xz *)fp;
    /* Use the ReadClose or WriteClose routines to get the error
     * status.  If we were writing, this gives us the uncompressed
     * file size, which can be stored in the extended attribute. */
    /* How do we know whether we were reading or writing?  If the
     * buffer has been allocated, we've been read from (in theory).
     */
    int bzerror;
    if (cfxp->buffer) {
        BZ2_bzReadClose(&bzerror, cfxp->bp);
    } else {
        /* Writing: get the uncompressed byte count and store it. */
        unsigned uncompressed_size;
        /* 0 = don't bother to complete the file if there was an error */
        BZ2_bzWriteClose(&bzerror, cfxp->bp, 0, &uncompressed_size, NULL);
        bzip_attempt_store(fp, uncompressed_size);
    }
    return bzerror;
}

/*! \brief The function dispatch table for xz files */

static const cfile_vtable xz_cfile_table = {
    sizeof(cfile_xz),
    xz_size,
    xz_eof,
    xz_gets,
    xz_vprintf,
    xz_read,
    xz_write,
    xz_flush,
    xz_close,
    "xz file"
};

