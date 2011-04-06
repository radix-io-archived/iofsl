#include "RPCClient.hh"
#include "RPCEncoder.hh"
#include "RPCHeader.hh"
#include "RPCException.hh"
#include "RPCTransform.hh"

#include "iofwdevent/CBException.hh"

#include "iofwdutil/assert.hh"
#include "iofwdutil/IOFWDLog.hh"

#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/range/iterator_range.hpp>

using namespace iofwdevent;
using namespace iofwdutil::transform;
using namespace boost;

namespace rpc
{
   //========================================================================
   void RPCClient::startWrite ()
   {
      con_.out->write (&write_ptr_, &write_size_,
            boost::bind (&RPCClient::writeReady, this, _1), 0);
   }

   void RPCClient::writeReady (const CBException & e)
   {
      e.check ();

      RPCEncoder enc (write_ptr_, write_size_);

      RPCHeader header;
      header.key = key_;
      header.flags_out = flags_out_;
      header.flags_in = flags_in_;

      process (enc, header);
      const size_t used = enc.getPos ();
      con_.out->rewindOutput (write_size_ - used,
            header.flags_out & RPCHeader::FL_NOSTUFF ?
            boost::bind (&RPCClient::flushHeader, this, _1)
            : boost::bind (&RPCClient::rewindReady, this, _1));
   }

   void RPCClient::flushHeader (const CBException & e)
   {
      e.check ();
      con_.out->flush (boost::bind (&RPCClient::rewindReady, this, _1));
   }

   void RPCClient::rewindReady (const CBException & e)
   {
      e.check ();

      // No more need for the output stream, check if we need to put a
      // transform filter in
      if (flags_out_ & RPCHeader::FL_TRANSFORM)
         transout_ = getOutputTransform (con_.out, flags_out_);

      // try to read response
      con_.in->read (&read_ptr_, &read_size_,
            boost::bind (&RPCClient::readReady, this, _1), 0);

      boost::mutex::scoped_lock l (lock_);
      out_ready_ = true;
      if (out_ready_cb_)
      {
         iofwdevent::CBType cb;
         cb.swap (out_ready_cb_);
         l.unlock ();
         cb (iofwdevent::CBException ());
      }
   }

   void RPCClient::readReady (const CBException & e)
   {
      e.check ();

      RPCResponse r;
      RPCDecoder dec (read_ptr_, read_size_);
      process (dec, r);

      if (r.status != RPCResponse::RPC_OK)
      {
         handleError (r.status);
         return;
      }

      // everything OK, rewind input
      con_.in->rewindInput (read_size_ - dec.getPos(),
            boost::bind (&RPCClient::readRewindReady, this, _1));
   }

   void RPCClient::handleError (uint32_t )
   {
      ALWAYS_ASSERT(false && "RPCClient::handleError not implemented");
      /*switch (errcode)
      {
         case RPCResponse::RPC_UNKNOWN:
            cb (UnknownRPCKeyException ());
            break;
         default:
            cb (RPCCommException ());
            break;
      }*/
   }

   void RPCClient::readRewindReady (const CBException & e)
   {
      e.check ();

      // No more need for the input, check if we need a transform filter
      if (flags_in_ & RPCHeader::FL_TRANSFORM)
         transin_ = getInputTransform (con_.in, flags_in_);

      boost::mutex::scoped_lock l(lock_);
      in_ready_ = true;
      if (in_ready_cb_)
      {
         iofwdevent::CBType cb;
         cb.swap (in_ready_cb_);
         l.unlock ();
         cb (e);
      }
   }

   void RPCClient::processOption (const std::string & option)
   {
      size_t pos = option.find (":");
      if (pos == std::string::npos)
      {
         ZLOG_ERROR(ZLOG_DEFAULT, format("Invalid option string: %s")
               % option);
         ALWAYS_ASSERT("invalid RPC option: no ':' found");
      }
      std::string key (option.begin(), option.begin()+pos);
      std::string value (option.begin()+pos+1, option.end());
      trim (key);
      trim (value);
      if (key != std::string ("transform"))
      {
         ZLOG_ERROR(ZLOG_DEFAULT, format("Invalid RPC option: %s")
               % key);
         ALWAYS_ASSERT("invalid RPC option!");
      }
      ZLOG_INFO (ZLOG_DEFAULT, format("RPC transform: %s") % value);
      uint32_t val = 0;
      if (value == std::string ("ZLIB"))
      {
         val = RPCHeader::FL_TRANSFORM_ZLIB;
      } else if (value == std::string ("BZLIB"))
      {
         val = RPCHeader::FL_TRANSFORM_BZLIB;
      } else if (value == std::string ("LZF"))
      {
         val = RPCHeader::FL_TRANSFORM_LZF;
      }
      else
      {
         ZLOG_ERROR (ZLOG_DEFAULT, format("Unknown transform: %s") % value);
      }
      if (val)
      {
         flags_in_ &= ~RPCHeader::FL_TRANSFORM_NONE;
         flags_out_ &= ~RPCHeader::FL_TRANSFORM_NONE;
         flags_in_ |= val;
         flags_out_ |= val;
      }
   }


   RPCClient::RPCClient (const RPCKey & key, const net::Connection & con,
         const char * options)
      : key_(key),
      con_ (con),
      in_ready_ (false),
      flags_out_ (RPCHeader::FL_DEFAULT),
      flags_in_ (RPCHeader::FL_DEFAULT),
      transout_ (0),
      transin_ (0)
   {
      if (options)
      {
         typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
         boost::char_separator<char> sep(";");
         std::string s (options);
         tokenizer tokens(s, sep);
         for (tokenizer::iterator tok_iter = tokens.begin();
               tok_iter != tokens.end(); ++tok_iter)
         {
            processOption (*tok_iter);
         }
      }
      // First obtain write space
      startWrite ();
   }

   void RPCClient::waitOutReady (const iofwdevent::CBType & cb)
   {
      boost::mutex::scoped_lock l (lock_);
      if (out_ready_)
      {
         l.unlock ();
         cb (iofwdevent::CBException ());
         return;
      }
      out_ready_cb_ = cb;
   }

   void RPCClient::waitInReady (const iofwdevent::CBType & cb)
   {
      boost::mutex::scoped_lock l (lock_);
      if (in_ready_)
      {
         l.unlock ();
         cb (iofwdevent::CBException ());
         return;
      }

      in_ready_cb_ = cb;
   }

   RPCClientHandle RPCClient::rpcConnect (const RPCKey & key,
         const net::Connection & con, const char * options)
   {
      if (!options)
      {
         const char * p = getenv ("IOFSL_RPC");
         if (p)
            options = p;
      }
      return RPCClientHandle (new RPCClient (key, con, options));
   }

   //========================================================================
}
