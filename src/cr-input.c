/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 8-*- */

/*
 *This file is part of the Croco Library
 *
 *The Croco Library is free software; 
 *you can redistribute it and/or modify it under the terms of 
 *the GNU General Public License as 
 *published by the Free Software Foundation; either version 2, 
 *or (at your option) any later version.
 *
 *The Croco Library is distributed in the hope 
 *that it will be useful, but WITHOUT ANY WARRANTY; 
 *without even the implied warranty of 
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *See the GNU General Public License for more details.
 *
 *You should have received a copy of the 
 *GNU General Public License along with The Croco Library; 
 *see the file COPYING. If not, write to 
 *the Free Software Foundation, Inc., 
 *59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *Copyright 2002-2003 Dodji Seketeli <dodji@seketeli.org>
 */

/*
 *$Id$
 */


#include "stdio.h"
#include "string.h"
#include "cr-input.h"
#include "cr-enc-handler.h"


/**
 *@file
 *The definition of the #CRInput class.
 */

/*******************
 *Private type defs
 *******************/


/**
 *The privated attributes of
 *the #CRInputPriv class.
 */
struct _CRInputPriv 
{
        /*
         *The input buffer
         */
        guchar *in_buf ;
        glong in_buf_size ;

        glong nb_bytes ;

        /*
         *The index of the next byte
         *to be read.
         */
        glong next_byte_index ;

        /*
         *The current line number
         */
        gulong line ;

        /*
         *The current col number.
         */
        gulong col ;

        gboolean end_of_line ;
        gboolean end_of_input ;

        /*
         *the reference count of this
         *instance.
         */
        guint ref_count ;
} ;


#define PRIVATE(object) (object)->priv


/***************************
 *private constants
 **************************/
#define CR_INPUT_MEM_CHUNK_SIZE 1024


/****************
 *Public methods
 ***************/


/**
 *Creates a new input stream from
 *a file.
 *@param a_file_uri the file to create
 *the input stream from.
 *@param a_enc the encoding of the file
 *to create the input from
 *@return the newly created input stream if
 *this method couldn read the file and create it,
 *NULL otherwise.
 */
CRInput *
cr_input_new_from_uri (gchar *a_file_uri, enum CREncoding a_enc)
{
        CRInput * result = NULL ;
        enum CRStatus status = CR_OK ;
        FILE * file_ptr = NULL ;
        guchar tmp_buf[CR_INPUT_MEM_CHUNK_SIZE] = {0} ;
        gint nb_read = 0 ;
        gboolean loop = TRUE ;
        CREncHandler * enc_handler = NULL ;

        g_return_val_if_fail (a_file_uri, NULL) ;

        file_ptr = fopen (a_file_uri, "r") ;

        if (file_ptr == NULL) 
        {

#ifdef CR_DEBUG
                cr_utils_trace_debug ("could not open file") ;
#endif
                g_warning ("Could not open file %s\n", a_file_uri) ;
                
                return NULL ;
        }

        result = g_malloc0 (sizeof (CRInput)) ;
        PRIVATE (result) = g_malloc0 (sizeof (CRInputPriv)) ;
        
        while (loop) 
        {

                nb_read =
                        fread (tmp_buf, 1/*read bytes*/,
                               CR_INPUT_MEM_CHUNK_SIZE/*nb of bytes*/,
                               file_ptr) ;

                if (nb_read != CR_INPUT_MEM_CHUNK_SIZE) 
                {                        
                        /*we read less chars than we wanted*/

                        if (feof (file_ptr)) 
                        {
                                /*we reached eof*/
                                loop = FALSE ;
                        } 
                        else 
                        {  
                                /*a pb occured !!*/
#ifdef CR_DEBUG
                                cr_utils_trace_debug 
                                        ("an io error occured") ;
#endif                                                    
                                status = CR_ERROR ;
                        }
                }

                if (status == CR_OK) 
                {
                        /*read went well*/

                        PRIVATE (result)->in_buf =
                                g_realloc (PRIVATE (result)->in_buf,
                                           PRIVATE (result)->in_buf_size
                                           + CR_INPUT_MEM_CHUNK_SIZE) ;

                        memcpy (PRIVATE (result)->in_buf
                                + PRIVATE (result)->in_buf_size ,
                                tmp_buf, nb_read) ;
                
                        PRIVATE (result)->in_buf_size += 
                                nb_read ;
                }
        }

        PRIVATE (result)->nb_bytes = PRIVATE (result)->in_buf_size ;

        if (file_ptr) 
        {
                fclose (file_ptr) ;
                file_ptr = NULL ;
        }

        if (status == CR_OK) 
        {
                /*
                 *Make sure the input's internal buffer
                 *is encoded in utf-8.
                 */
                if (a_enc != CR_UTF_8)
                {
                        enc_handler = cr_enc_handler_get_instance (a_enc);
                        if (enc_handler == NULL && result)
                        {
                                goto error ;
                        }
                        else
                        {
                                /*encode the buffer in utf8.*/
                                guchar *utf8_buf = NULL ;
                                gulong len = 0 ;

                                status = cr_enc_handler_convert_input 
                                        (enc_handler,
                                         PRIVATE (result)->in_buf,
                                         &PRIVATE (result)->in_buf_size,
                                         &utf8_buf, &len) ;

                                if (status != CR_OK)
                                {
                                        goto error ;
                                }
                                
                                g_free (PRIVATE (result)->in_buf) ;
                                PRIVATE (result)->in_buf = utf8_buf ;
                                PRIVATE (result)->in_buf_size = len ;
                                PRIVATE (result)->line = 1 ;
                        }
                }
        }        
        else
        {
                cr_utils_trace_debug ("Input loading failed") ;
                goto error ;
        }

        return result ;

 error:
        if (result) 
            {

                /*free result*/
                cr_input_destroy (result) ;
                result = NULL ;
            }

        return NULL ;
}


/**
 *The destructor of the #CRInput class.
 *@param a_this the current instance of #CRInput.
 */
void
cr_input_destroy (CRInput *a_this)
{
        if (a_this == NULL)
                return ;

        if (PRIVATE (a_this)) 
        {
                if (PRIVATE (a_this)->in_buf) 
                {
                        g_free (PRIVATE (a_this)->in_buf) ;
                        PRIVATE (a_this)->in_buf = NULL ;

                }

                g_free (PRIVATE (a_this)) ;
                PRIVATE (a_this) = NULL ;
        }

        g_free (a_this) ;
}


/**
 *Increments the reference count of the current
 *instance of #CRInput.
 *@param a_this the current instance of #CRInput.
 */
void 
cr_input_ref (CRInput *a_this)
{
        g_return_if_fail (a_this && PRIVATE (a_this)) ;
        
        PRIVATE (a_this)->ref_count ++ ;
}


/**
 *Decrements the reference count of this instance
 *of #CRInput. If the reference count goes down to
 *zero, this instance is destroyed.
 *@param a_this the current instance of #CRInput.
 *
 */        
void
cr_input_unref (CRInput *a_this)
{
        g_return_if_fail (a_this && PRIVATE (a_this)) ;
        
        if (PRIVATE (a_this)->ref_count)  
        {
                PRIVATE (a_this)->ref_count -- ;
        }

        if (PRIVATE (a_this)->ref_count == 0) 
        {
                cr_input_destroy (a_this) ;
        }
}


/**
 *Tests wether the current instance of
 *#CRInput has reached it's input buffer.
 *@param a_this the current instance of #CRInput.
 *@param a_end_of_input out parameter. Is set to TRUE if
 *the current instance has reached the end of its input buffer,
 *FALSE otherwise.
 *@param CR_OK upon successfull completion, an error code otherwise.
 *Note that all the out parameters of this method are valid if
 *and only if this method returns CR_OK.
 */
enum CRStatus
cr_input_end_of_input (CRInput *a_this, gboolean *a_end_of_input)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_end_of_input, CR_BAD_PARAM_ERROR) ;

        *a_end_of_input = (PRIVATE (a_this)->next_byte_index 
                           >= PRIVATE (a_this)->in_buf_size)? TRUE : FALSE ;

        return CR_OK ;
}


/**
 *Retunrs the number of bytes left in the input stream
 *before the end.
 *@param a_this the current instance of #CRInput.
 *@return the number of characters left or -1 in case of error.
 */
glong
cr_input_get_nb_bytes_left (CRInput *a_this)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this), -1) ;
        g_return_val_if_fail (PRIVATE (a_this)->nb_bytes 
                              >= PRIVATE (a_this)->in_buf_size, -1) ;
        g_return_val_if_fail (PRIVATE (a_this)->next_byte_index 
                              <= PRIVATE (a_this)->nb_bytes, -1) ;

        if (PRIVATE (a_this)->end_of_input)
                return 0 ;
        
        return PRIVATE (a_this)->nb_bytes 
                - PRIVATE (a_this)->next_byte_index ;
}


/**
 *Returns the next byte of the input.
 *Update the state of the input so that
 *the next invocation of this method  returns
 *the next coming byte.
 *
 *@param a_this the current instance of #CRInput.
 *@param a_byte out parameter the returned byte.
 *@return CR_OK upon sucessfull completion, an error code
 *otherwise. All the out param of this method are valid if
 *and only if this method returns CR_OK.
 */
enum CRStatus
cr_input_read_byte (CRInput *a_this, guchar *a_byte)
{
        g_return_val_if_fail (a_this 
                              && PRIVATE (a_this) 
                              && a_byte,
                              CR_BAD_PARAM_ERROR) ;

        g_return_val_if_fail (PRIVATE (a_this)->next_byte_index >= 0
                              && PRIVATE (a_this)->nb_bytes >= 0,
                              CR_BAD_PARAM_ERROR) ;

        g_return_val_if_fail (PRIVATE (a_this)->next_byte_index <= 
                              PRIVATE (a_this)->nb_bytes,
                              CR_BAD_PARAM_ERROR) ;

        if (PRIVATE (a_this)->end_of_input == TRUE)
                return CR_END_OF_INPUT_ERROR ; 

        *a_byte =
                PRIVATE 
                (a_this)->in_buf[PRIVATE (a_this)->next_byte_index] ;

        if (PRIVATE (a_this)->nb_bytes -
            PRIVATE (a_this)->next_byte_index < 2)
        {
                PRIVATE (a_this)->end_of_input = TRUE ;
        }
        else
        {
                PRIVATE (a_this)->next_byte_index ++ ;
        }

        return CR_OK ;
}


/**
 *Reads an unicode character from the current instance of
 *#CRInput.
 *@param a_this the current instance of CRInput.
 *@param a_char out parameter. The read character.
 *@return CR_OK upon successfull completion, an error code
 *otherwise.
 */
enum CRStatus
cr_input_read_char (CRInput *a_this, guint32 *a_char)
{
        enum CRStatus status = CR_OK ;
        gulong consumed = 0, nb_bytes_left = 0 ;
        
        g_return_val_if_fail (a_this && PRIVATE (a_this) && a_char,
                              CR_BAD_PARAM_ERROR) ;

        if (PRIVATE (a_this)->end_of_input == TRUE)
                return CR_END_OF_INPUT_ERROR ; 

        nb_bytes_left = cr_input_get_nb_bytes_left (a_this) ;

        if (nb_bytes_left < 1)
        {
                return  CR_END_OF_INPUT_ERROR ;
        }

        status =
                cr_utils_read_char_from_utf8_buf 
                (PRIVATE (a_this)->in_buf 
                 + 
                 PRIVATE (a_this)->next_byte_index,
                 nb_bytes_left, a_char, &consumed) ;

        if (status == CR_OK)
        {
                /*update next byte index*/
                PRIVATE (a_this)->next_byte_index += consumed ;

                /*upate line and column number*/
                if (PRIVATE (a_this)->end_of_line == TRUE)
                {
                        PRIVATE (a_this)->col = 1 ;
                        PRIVATE (a_this)->line ++ ;
                        PRIVATE (a_this)->end_of_line = FALSE ;
                }
                else if (*a_char != '\n')
                {
                        PRIVATE (a_this)->col ++ ;
                }
                
                if (*a_char == '\n')
                {
                        PRIVATE (a_this)->end_of_line = TRUE ;
                }

        }

        return status ;
}


/**
 *Setter of the current line number.
 *@param a_this the "this pointer" of the current instance of
 *#CRInput.
 *@param a_line_num the new line number.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_input_set_line_num (CRInput *a_this, glong a_line_num)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this),
                              CR_BAD_PARAM_ERROR) ;

        PRIVATE (a_this)->line = a_line_num ;

        return CR_OK ;
}


/**
 *Getter of the curren line number.
 *@param a_this the "this pointer" of the current instance of
 *#CRInput.
 *@param a_line_num the returned line number.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_input_get_line_num (CRInput *a_this, glong *a_line_num)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_line_num,
                              CR_BAD_PARAM_ERROR) ;
        
        *a_line_num = PRIVATE (a_this)->line ;

        return CR_OK ;
}


/**
 *Setter of the current column number.
 *@param a_this the "this pointer" of the current instance of
 *#CRInput.
 *@param a_col the new column column number.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_input_set_column_num (CRInput *a_this, glong a_col)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this),
                          CR_BAD_PARAM_ERROR) ;

        PRIVATE (a_this)->col = a_col ;

        return CR_OK ;
}


/**
 *Getter of the current column number.
 *@param a_this the "this pointer" of the current instance of
 *#CRInput.
 *@param a_col out parameter
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_input_get_column_num (CRInput *a_this, glong *a_col)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this) && a_col,
                          CR_BAD_PARAM_ERROR) ;
        
        *a_col = PRIVATE (a_this)->col ;

        return CR_OK ;
}


/**
 *Increments the current line number.
 *@param a_this the "this pointer" of the current instance of
 *#CRInput.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_input_increment_line_num (CRInput *a_this, glong a_increment)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this),
                              CR_BAD_PARAM_ERROR) ;
        
        PRIVATE (a_this)->line += a_increment ;

        return CR_OK ;
}


/**
 *Increments the current column number.
 *@param a_this the "this pointer" of the current instance of
 *#CRInput.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_input_increment_col_num (CRInput *a_this, glong a_increment)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this),
                              CR_BAD_PARAM_ERROR) ;

        PRIVATE (a_this)->col += a_increment ;

        return CR_OK ;
}



/**
 *Consumes the next character of the input stream if
 *and only if that character equals a_char.
 *
 *@param a_this the this pointer.
 *@param a_char the character to consume. If set to zero,
 *consumes any character.
 *@return CR_OK upon successfull completion, CR_PARSING_ERROR if
 *next char is different from a_char, an other error code otherwise
 */
enum CRStatus
cr_input_consume_char (CRInput *a_this, guint32 a_char)
{
        guint32 c ;
        enum CRStatus status ;

        g_return_val_if_fail (a_this && PRIVATE (a_this),
                              CR_BAD_PARAM_ERROR) ;

        if ((status = cr_input_peek_char (a_this, &c)) != CR_OK)
        {
                return status ;
        }

        if (c == a_char || a_char == 0)
        {
                status = cr_input_read_char (a_this, &c) ;
        }
        else
        {
                return CR_PARSING_ERROR ;
        }

        return status ;
}


/**
 *Consumes up to a_nb_char occurences of the next contiguous characters 
 *which equal a_char. Note that the next character of the input stream
 **MUST* equal a_char to trigger the consumption, or else, the error
 *code CR_PARSING_ERROR is returned.
 *If the number of contiguous characters that equals a_char is less than
 *a_nb_char, then this function consumes all the characters it can consumed.
 *
 *@param a_this the this pointer of the current instance of #CRInput.
 *@param a_char the character to consume.
 *@param a_nb_char in/out parameter. The number of characters to consume.
 *If set to a negative value, the function will consume all the occurences
 *of a_char found.
 *After return, if the return value equals CR_OK, this variable contains 
 *the number of characters actually consumed.
 *@return CR_OK if at least one character has been consumed, an error code
 *otherwise.
 */
enum CRStatus
cr_input_consume_chars (CRInput *a_this, guint32 a_char, glong *a_nb_char)
{
        enum CRStatus status = CR_OK ;
        gulong nb_consumed = 0 ;

        g_return_val_if_fail (a_this && PRIVATE (a_this) && a_nb_char,
                              CR_BAD_PARAM_ERROR) ;

        g_return_val_if_fail (a_char != 0 || a_nb_char >=0,
                              CR_BAD_PARAM_ERROR) ;

        for (nb_consumed = 0 ;
             (status == CR_OK)
             && ((*a_nb_char > 0 && nb_consumed < *a_nb_char)
                 || (*a_nb_char < 0)) ;
             nb_consumed ++)
        {
                status = cr_input_consume_char (a_this, a_char) ;
        }

        *a_nb_char = nb_consumed ;

        if ((nb_consumed > 0) 
            && ((status == CR_PARSING_ERROR) 
                ||  (status == CR_END_OF_INPUT_ERROR)))
        {                
                status = CR_OK ;
        }

        return status ;
}

/**
 *Same as cr_input_consume_chars() but this one consumes white
 *spaces.
 *
 *@param a_this the "this pointer" of the current instance of #CRInput.
 *@param a_nb_chars in/out parameter. The number of white spaces to
 *consume. After return, holds the number of white spaces actually consumed.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_input_consume_white_spaces (CRInput *a_this, glong *a_nb_chars)
{
        enum CRStatus status = CR_OK ;
        guint32 cur_char = 0, nb_consumed = 0 ;

        g_return_val_if_fail (a_this && PRIVATE (a_this) && a_nb_chars,
                              CR_BAD_PARAM_ERROR) ;

        for (nb_consumed = 0;
             ((*a_nb_chars > 0) && (nb_consumed < *a_nb_chars))
                     || (*a_nb_chars < 0) ;
             nb_consumed ++)
        {
                status = cr_input_peek_char (a_this, &cur_char) ;
                if (status != CR_OK) break ;

                /*if the next char is a white space, consume it !*/
                if (cr_utils_is_white_space (cur_char) == TRUE)
                {
                        status = cr_input_read_char (a_this, &cur_char) ;
                        if (status != CR_OK) break ;
                        continue ;
                }
                
                break ;
        
        }

        if (nb_consumed && status == CR_END_OF_INPUT_ERROR) 
        {
                status = CR_OK ;
        }
        
        return status ;
}


/**
 *Same as cr_input_read_char() but does not update the
 *internal state of the input stream. The next call
 *to cr_input_peek_char() or cr_input_read_char() will thus
 *return the same character as the current one.
 *@param a_this the current instance of #CRInput.
 *@param a_char out parameter. The returned character.
 *@return CR_OK upon successfull completion, an error code
 *otherwise.
 */
enum CRStatus
cr_input_peek_char (CRInput *a_this, guint32 *a_char)
{
        enum CRStatus status = CR_OK ;
        glong consumed = 0, nb_bytes_left = 0 ;

        g_return_val_if_fail (a_this && PRIVATE (a_this) 
                              && a_char, CR_BAD_PARAM_ERROR) ;

        if (PRIVATE (a_this)->next_byte_index >= 
            PRIVATE (a_this)->in_buf_size)
        {
                return CR_END_OF_INPUT_ERROR ;
        }

        nb_bytes_left = cr_input_get_nb_bytes_left (a_this) ;

        if (nb_bytes_left < 1)
        {
                return CR_END_OF_INPUT_ERROR ;
        }

        status =
                cr_utils_read_char_from_utf8_buf 
                (PRIVATE (a_this)->in_buf + PRIVATE (a_this)->next_byte_index,
                 nb_bytes_left, a_char, &consumed) ;

        return status ;
}


/**
 *Gets a byte from the input stream.
 *starting from the current position in the input stream.
 *Unlike cr_input_peek_next_byte() this method
 *does not update the state of the current input stream.
 *Subsequent calls to cr_input_peek_byte with the same arguments
 *will return the same byte.
 *
 *@param a_this the current instance of #CRInput.
 *@param a_origin the origin to consider in the calculation
 *of the position of the byte to peek.
 *@param a_offset the offset of the byte to peek, starting from
 *the origin specified by a_origin.
 *@param a_byte out parameter the peeked byte.
 *@return CR_OK upon successfull completion or,
 *
 *<ul>
 *<li>CR_BAD_PARAM_ERROR if at least of of the parameters is invalid</li>
 *<li>CR_OUT_OF_BOUNDS_ERROR if the indexed byte is out of bounds</li>
 *</ul>
 */
enum CRStatus
cr_input_peek_byte (CRInput *a_this, enum CRSeekPos a_origin,
                    gulong a_offset, guchar *a_byte)
{
        gulong abs_offset = 0 ;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_byte, 
                              CR_BAD_PARAM_ERROR) ;
        

        switch (a_origin) {

        case CR_SEEK_CUR:
                abs_offset = 
                        PRIVATE (a_this)->next_byte_index - 1 + a_offset ;
                break ;

        case CR_SEEK_BEGIN:
                abs_offset = a_offset ;
                break ;

        case CR_SEEK_END:
                abs_offset = 
                        PRIVATE (a_this)->in_buf_size - 1 - a_offset ;
                break;

        default:
                return CR_BAD_PARAM_ERROR ;
        }

        if (abs_offset < PRIVATE (a_this)->in_buf_size) {

                *a_byte =
                        PRIVATE (a_this)->in_buf[abs_offset] ;

                return CR_OK ;

        } else {
                return CR_END_OF_INPUT_ERROR ;
        }
}

/**
 *Returns the memory address of the byte located at a given offset
 *in the input stream.
 *@param a_this the current instance of #CRInput.
 *@param a_offset the offset of the byte in the input stream starting
 *from the begining of the stream.
 *@return the address, otherwise NULL if an error occured.
 */
guchar *
cr_input_get_byte_addr (CRInput *a_this, 
                        gulong a_offset)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this), NULL) ;
        
        if (a_offset >= PRIVATE (a_this)->nb_bytes)
        {
                return NULL ;
        }

        return &PRIVATE (a_this)->in_buf[a_offset] ;
}

/**
 *Sets the "current byte index" of the current instance
 *of #CRInput. Next call to cr_input_get_byte() will return
 *the byte next after the new "current byte index".
 *
 *@param a_this the current instance of #CRInput.
 *
 *@param a_origin the origin to consider during the calculation
 *of the absolute position of the new "current byte index".
 *
 *@param a_pos the relative offset of the new "current byte index."
 *This offset is relative to the origin a_origin.
 *
 *@return CR_OK upon successfull completion otherwise returns
 *<ul>
 *<li>CR_BAD_PARAM_ERROR if at least one of the parameters is not valid</li>
 *<li>CR_OUT_BOUNDS_ERROR</li>
 *</ul>
 */
enum CRStatus
cr_input_seek_index (CRInput *a_this, enum CRSeekPos a_origin, gint a_pos)
{

        glong abs_offset = 0 ;

        g_return_val_if_fail (a_this && PRIVATE (a_this), CR_BAD_PARAM_ERROR) ;

  
        switch (a_origin) {

        case CR_SEEK_CUR:
                abs_offset =
                        PRIVATE (a_this)->next_byte_index - 1 + a_pos ;
                break ;

        case CR_SEEK_BEGIN:
                abs_offset = a_pos ;
                break ;

        case CR_SEEK_END:
                abs_offset =
                        PRIVATE (a_this)->in_buf_size - 1 - a_pos ;
                break ;

        default:
                return CR_BAD_PARAM_ERROR ;
        }

        if (abs_offset < PRIVATE (a_this)->nb_bytes) {

                /*update the input stream's internal state*/
                PRIVATE (a_this)->next_byte_index = abs_offset + 1 ;

                return CR_OK ;
        }

        return CR_OUT_OF_BOUNDS_ERROR ;
}


/**
 *Gets the position of the "current byte index" which
 *is basically the position of the last returned byte in the
 *input stream.
 *
 *@param a_this the current instance of #CRInput.
 *
 *@param a_pos out parameter. The returned position.
 *
 *@return CR_OK upon sucessfull completion. Otherwise,
 *<ul>
 *<li>CR_BAD_PARAMETER_ERROR if at least of the arguments is invalid.</li>
 *<li>CR_START_OF_INPUT if no call to neither cr_input_read_byte()
 *not cr_input_seek_index() have been issued before calling 
 *cr_input_get_cur_pos()</li>
 *</ul>
 *Note that the out parameters of this function are valid if and only if this
 *function returns CR_OK.
 */
enum CRStatus
cr_input_get_cur_pos (CRInput *a_this, CRInputPos * a_pos)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this) && a_pos,
                              CR_BAD_PARAM_ERROR) ;

        if (PRIVATE (a_this)->next_byte_index < 0)
                return CR_START_OF_INPUT_ERROR ;

        a_pos->next_byte_index = PRIVATE (a_this)->next_byte_index ;
        a_pos->line = PRIVATE (a_this)->line ;
        a_pos->col = PRIVATE (a_this)->col ;
        a_pos->end_of_line = PRIVATE (a_this)->end_of_line ;
        a_pos->end_of_file = PRIVATE (a_this)->end_of_input ;
        
        return CR_OK ;
}


/**
 *Getter of the next byte index. 
 *It actually returns the index of the
 *next byte to be read.
 *@param a_this the "this pointer" of the current instance of
 *#CRInput
 *@param a_index out parameter. The returned index.
 *@return CR_OK upon sucessfull completion, an error code
 *otherwise.
 */
enum CRStatus
cr_input_get_cur_index (CRInput *a_this, glong *a_index)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this) 
                              && a_index, 
                              CR_BAD_PARAM_ERROR) ;

        *a_index = PRIVATE (a_this)->next_byte_index ;

        return CR_OK ;
}


/**
 *Setter of the next byte index.
 *It sets the index of the next byte to be read.
 *@param a_this the "this pointer" of the current instance
 *of #CRInput .
 *@param a_index the new index to set.
 *@return CR_OK upon sucessfull completion, an error code otherwise.
 */
enum CRStatus
cr_input_set_cur_index (CRInput *a_this, glong a_index)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this),
                              CR_BAD_PARAM_ERROR) ;
        
        PRIVATE (a_this)->next_byte_index = a_index ;

        return CR_OK ;
}


/**
 *Sets the end of file flag.
 *@param a_this the current instance of #CRInput.
 *@param a_eof the new end of file flag.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_input_set_end_of_file (CRInput *a_this, gboolean a_eof)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this),
                              CR_BAD_PARAM_ERROR) ;

        PRIVATE (a_this)->end_of_input = a_eof ;

        return CR_OK ;
}

/**
 *Gets the end of file flag.
 *@param a_this the current instance of #CRInput.
 *@param a_eof out parameter the place where to put the end of
 *file flag.
 *@return CR_OK upon sucessfull completion, an error code otherwise.
 */
enum CRStatus
cr_input_get_end_of_file (CRInput *a_this, gboolean *a_eof)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_eof,
                              CR_BAD_PARAM_ERROR) ;

        *a_eof = PRIVATE (a_this)->end_of_input ;

        return CR_OK ;
}


/**
 *Sets the end of line flag.
 *@param a_this the current instance of #CRInput.
 *@param a_eol the new end of line flag.
 *@return CR_OK upon successfull completion, an error code
 *otherwise.
 */
enum CRStatus
cr_input_set_end_of_line (CRInput *a_this, gboolean a_eol)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this),
                              CR_BAD_PARAM_ERROR) ;

        PRIVATE (a_this)->end_of_line = a_eol ;

        return CR_OK ;
}

/**
 *Gets the end of line flag of the current input.
 *@param a_this the current instance of #CRInput
 *@param a_eol out parameter. The place where to put
 *the returned flag
 *@return CR_OK upon successfull completion, an error code
 *otherwise.
 */
enum CRStatus
cr_input_get_end_of_line (CRInput *a_this, gboolean *a_eol)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_eol,
                              CR_BAD_PARAM_ERROR) ;

        *a_eol = PRIVATE (a_this)->end_of_line ;

        return CR_OK ;
}


/**
 *Sets the current position in the input stream.
 *
 *@param a_this the "this pointer" of the current instance of
 *#CRInput.
 *@param a_pos the new position.
 */
enum CRStatus
cr_input_set_cur_pos (CRInput *a_this, CRInputPos *a_pos)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this) && a_pos,
                              CR_BAD_PARAM_ERROR) ;
        
        cr_input_set_column_num (a_this, a_pos->col) ;
        cr_input_set_line_num (a_this, a_pos->line) ;
        cr_input_set_cur_index (a_this, a_pos->next_byte_index) ;
        cr_input_set_end_of_line (a_this, a_pos->end_of_line) ;
        cr_input_set_end_of_file (a_this, a_pos->end_of_file) ;

        return CR_OK ;
}
