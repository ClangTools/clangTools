/*
 * Copyright (c) 2011, Run With Robots
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the fastjson library nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY RUN WITH ROBOTS ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL MICHAEL ANDERSON BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "fastjson/core.h"
#include "fastjson/dom.h"
#include "fastjson/fastjson.h"

namespace fastjson { namespace dom {

  bool parse_string( const std::string & s, Token * tok, Chunk * chunk, unsigned int mode, fastjson::UserErrorCallback user_error_callback, void * user_data )
  {
    fastjson::JsonElementCount count;
    count.user_error_callback = user_error_callback;
    count.user_data = user_data;
    count.mode = mode;

    if( ! fastjson::count_elements( s, &count ) ) return false;

    // Allocate up enough space.
    fastjson::Document doc;
    doc.user_error_callback = user_error_callback;
    doc.user_data = user_data;
    doc.mode = mode;
    if( count.n_array_elements() > 0 )
    {
      doc.array_store = new fastjson::ArrayEntry[ count.n_array_elements() ];
    }
    else
    {
      doc.array_store = NULL;
    }

    if( count.n_dict_elements() > 0 )
    {
      doc.dict_store = new fastjson::DictEntry[ count.n_dict_elements() ];
    }
    else
    {
      doc.dict_store = NULL;
    }

    if( count.n_string_length() > 0 )
    {
      doc.string_store = new unsigned char[ count.n_string_length() ];
    }
    else
    {
      doc.string_store = NULL;
    }

    //Actually read the stuff into the arrays etc.

    if( ! fastjson::parse_doc( s, &doc ) )
    {
      delete [] doc.array_store;
      delete [] doc.dict_store;
      delete [] doc.string_store;
      return false;
    }

    *tok = doc.root;

    //Add the data to the chunk.

    if( count.n_array_elements()>0)
    {
      chunk->add_array_page( doc.array_store, count.n_array_elements() );
    }
    if( count.n_dict_elements()>0)
    {
      chunk->add_dict_page ( doc.dict_store, count.n_dict_elements() );
    }
    if( count.n_string_length()>0)
    {
      chunk->add_string_page( (char*)doc.string_store, count.n_string_length() );
    }

    return true;
  }

  void clone_token( const Token * in_token, Token * out_token, Chunk * chunk )
  {
    switch( in_token->type )
    {
      case Token::ArrayToken:
        {
          Array a = Array::create_array( out_token, chunk );
          //Add all the children.
          ArrayEntry * child_orig = in_token->array.ptr;
          while( child_orig )
          {
            ArrayEntry * child_copy = a.add_child_raw();
            clone_token( &child_orig->tok, &child_copy->tok, chunk );
            child_orig = child_orig->next;
          }
        }
        break;
      case Token::DictToken:
        {
          Dictionary d = Dictionary::create_dict( out_token, chunk );
          //Add all the children.
          DictEntry * child_orig = in_token->dict.ptr;
          while( child_orig )
          {
            DictEntry * child_copy = d.add_child_raw();
            clone_token( &child_orig->key_tok, &child_copy->key_tok, chunk );
            clone_token( &child_orig->value_tok, &child_copy->value_tok, chunk );
            child_orig = child_orig->next;
          }
        }
        break;
      case Token::ValueToken:
        {
          out_token->type = Token::ValueToken;
          out_token->value.size = in_token->value.size;
          out_token->value.type_hint = in_token->value.type_hint;
          if( in_token->value.ptr == NULL )
          {
            out_token->value.ptr = NULL;
          }
          else
          {
            out_token->value.ptr = chunk->create_raw_buffer( in_token->value.ptr, in_token->value.size );
          }
        }
        break;
      case Token::LiteralTrueToken:
        out_token->type=Token::LiteralTrueToken;
        break;
      case Token::LiteralFalseToken:
        out_token->type=Token::LiteralFalseToken;
        break;
      case Token::LiteralNullToken:
        out_token->type=Token::LiteralNullToken;
        break;
    }
  }


} }

