#include <limits>
#include <algorithm>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Resource.hh"
#include "iofwdutil/assert.hh"
#include "ContextBase.hh"
#include "iofwdutil/tools.hh"

using namespace boost::posix_time;

namespace iofwdutil
{
   namespace completion
   {
//===========================================================================

      unsigned char ContextBase::registerResource (Resource * res)
      {
         boost::mutex::scoped_lock l (lock_);

         std::find(resources_.begin(), resources_.end(),  res) == resources_.end();
         unsigned char ret = static_cast<unsigned char> (resources_.size()); 
         resources_.push_back (res); 
         return ret; 
      }

      unsigned int ContextBase::getActiveCount () const
      {
         unsigned int c = 0; 
         for (unsigned int i=0; i<resources_.size(); ++i)
         {
           /* if (resources_[i]->isActive ())
            {
               ++c; 
            } */
         }
         return c; 
      }

      bool ContextBase::isActive () const
      {
         boost::mutex::scoped_lock l (lock_); 
         return (getActiveCount ()); 
         
      } 

      ContextBase::~ContextBase ()
      {
         for (unsigned int i=0; i<resources_.size(); ++i)
         {
            //ALWAYS_ASSERT(!resources_[i]->isActive ()); 
         }
      }

      void ContextBase::wait (const CompletionID & UNUSED(id), void ** UNUSED(user))
      {
         //resources_[id.getResourceID()]->wait (id, user); 
      }

      bool ContextBase::test (const CompletionID & UNUSED(id), void ** UNUSED(user), 
            unsigned int UNUSED(mstimeout))
      {
         //return resources_[id.getResourceID()]->test (id, user, mstimeout); 
         // @TODO
         return false; 
      }

      void ContextBase::waitAny (std::vector<CompletionID *> & ret)
      {
         testAny (ret, std::numeric_limits<unsigned int>::max()); 
      }

      void ContextBase::testAny (std::vector<CompletionID *> & ret,  unsigned int maxms)
      {
         const size_t initial = ret.size(); 
         const ptime timeout = microsec_clock::universal_time() + millisec (maxms); 
         do
         {
            for (unsigned int i=0; i<resources_.size(); ++i)
            {
               //resources_[i]->testAny (ret, maxms); 
            }
         } while (ret.size() == initial && microsec_clock::universal_time() < timeout); 
      }


//===========================================================================
   }
}
