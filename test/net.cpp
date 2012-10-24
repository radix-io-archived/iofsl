#include <boost/program_options.hpp>
#include <boost/scoped_array.hpp>
#include <boost/format.hpp>
#include <boost/exception/diagnostic_information.hpp>

#include <iostream>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/density.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <boost/accumulators/statistics/moment.hpp>
#include <boost/accumulators/statistics/variance.hpp>

#include "iofwdutil/Timer.hh"
#include "iofwdutil/tools.hh"
#include "iofwdevent/SingleCompletion.hh"

#include "net/loopback/LoopbackConnector.hh"
#include "net/bmi/BMIConnector.hh"

#include "net/Net.hh"

#include "iofwdutil/bmi/BMI.hh"
#include "iofwdevent/BMIResource.hh"
#include "iofwdutil/ZException.hh"

#include "sm/SimpleSM.hh"
#include "sm/SimpleSlots.hh"

#include "TrackLive.hh"

#include "test/boost/TestData.hh"

#include <boost/cstdint.hpp>

using namespace std;
using namespace net;
using namespace boost::program_options;
using namespace boost::accumulators;
using namespace boost;

using namespace iofwdutil;
using namespace iofwdutil::bmi;

using namespace iofwdevent;

using namespace sm;

enum { MB = 1024 * 1024 };

static size_t opt_loops;
static std::string opt_remote;
static std::string opt_listen;
static bool opt_validate = false;
static bool opt_corrupt = false;
static bool opt_local = false;
static size_t opt_suggested;
static size_t opt_concurrent = 1;

iofwdutil::IOFWDLogSource & mylog ( iofwdutil::IOFWDLog::getSource () );

// ==========================================================================

//===========================================================================

class BounceData : public sm::SimpleSM<BounceData>,
                   public TrackLive
{
   public:
      BounceData (sm::SMManager & smm,
            ZeroCopyInputStream * in, ZeroCopyOutputStream * out)
         : sm::SimpleSM<BounceData> (smm),
           in_(in), out_(out),
           read_requested_ (0),
           write_requested_ (0),
           todo_ (0),
           done_ (0),
           read_ptr_ (0), read_size_(0), read_used_(0),
           write_ptr_ (0), write_size_ (0), write_used_ (0),
           slot_ (*this),
           first_ (true)
      {
      }

      void init (iofwdevent::CBException e)
      {
         if (!checkError (e))
            return;

         ZLOG_DEBUG(mylog, "BounceData::init");
         e.check ();
         setNextMethod (&BounceData::getBuffers, e);
      }

      void requestRead (iofwdevent::CBException e)
      {
         if (!checkError (e))
            return;

         ZLOG_DEBUG(mylog, "BounceData::requestread");
         e.check ();
         in_->read (&read_ptr_, &read_size_, slot_[0], 0);
         slot_.wait (0, &BounceData::readAvailable);
      }

      void readAvailable (iofwdevent::CBException e)
      {
         if (!checkError (e))
            return;

         ZLOG_DEBUG(mylog, format("BounceData::readAvailable: %u") %
               read_size_);
         e.check ();
         read_requested_ += read_size_;
         read_used_ = 0;
         setNextMethod (&BounceData::getBuffers);
      }

      void requestWrite (iofwdevent::CBException e)
      {
         if (!checkError (e))
            return;

         ZLOG_DEBUG(mylog, "BounceData::requestWrite");
         out_->write (&write_ptr_, &write_size_, slot_[0], opt_suggested);
         slot_.wait (0, &BounceData::writeAvailable);
      }

      void writeAvailable (iofwdevent::CBException e)
      {
         if (!checkError (e))
            return;

         ZLOG_DEBUG(mylog, format("BounceData::writeAvailable: %s")
               % write_size_);

         write_requested_ += write_size_;
         write_used_ = 0;
         setNextMethod (&BounceData::getBuffers);
      }

      void getBuffers (iofwdevent::CBException e)
      {
         if (!checkError (e))
            return;

         ZLOG_DEBUG(mylog, "BounceData::getBuffers");

         const size_t readavail = read_size_ - read_used_;
         if (!readavail && (first_ || (done_ != todo_)))
         {
            setNextMethod (&BounceData::requestRead, e);
            return;
         }

         const size_t writeavail = write_size_ - write_used_;
         if (!writeavail && (first_ || (done_ != todo_)))
         {
            setNextMethod (&BounceData::requestWrite, e);
            return;
         }

         if (first_)
         {
            ALWAYS_ASSERT(read_size_ >= 4);
            todo_ = *(boost::uint32_t*) read_ptr_;
            ZLOG_DEBUG (mylog, format ("BounceData: stream size = %u") % todo_);
            first_ = false;
         }

         const size_t thiscopy = std::min (readavail, writeavail);
         ALWAYS_ASSERT(thiscopy);
         memcpy (
               static_cast<char *>(write_ptr_) + write_used_,
               static_cast<const char*>(read_ptr_) + read_used_,
               thiscopy);
         done_ += thiscopy;
         write_used_ += thiscopy;
         read_used_ += thiscopy;

         if (done_ == todo_)
         {
            out_->rewindOutput (write_size_ - write_used_, slot_[0]);
            slot_.wait (0, &BounceData::doFlush);
            return;
         }
         else
         {
            setNextMethod (&BounceData::getBuffers, e);
            return;
         }
      }

      void doFlush (iofwdevent::CBException e)
      {
         if (!checkError (e))
            return;

         ZLOG_DEBUG (mylog, "BounceData::doFlush");
         out_->flush (slot_[0]);
         slot_.wait (0, &BounceData::done);
      }

      void done (iofwdevent::CBException e)
      {
         if (!checkError (e))
            return;

         ZLOG_DEBUG(mylog, "BounceData::done");
      }

      void error (iofwdevent::CBException )
      {
      }

   protected:
      bool checkError (const iofwdevent::CBException & e)
      {
         if (e.hasException ())
         {
            ZLOG_ERROR (mylog, "Stream exception!");
            setNextMethod (&BounceData::error, e);
            return false;
         }
         return true;
      }

   protected:
      scoped_ptr<ZeroCopyInputStream> in_;
      scoped_ptr<ZeroCopyOutputStream> out_;

      size_t read_requested_;
      size_t write_requested_;

      size_t todo_;

      size_t done_;

      const void * read_ptr_;
      size_t       read_size_;
      size_t       read_used_;

      void *       write_ptr_;
      size_t       write_size_;
      size_t       write_used_;

      sm::SimpleSlots<1, BounceData> slot_;

      bool first_;
};

// ==========================================================================

class WriteSM : public SimpleSM<WriteSM>,
                public TrackLive
{
   public:
      WriteSM (SMManager & smm, size_t todo, ZeroCopyOutputStream * out,
            DataGenerator * gen)
         : SimpleSM<WriteSM> (smm),
           slot_ (*this),
           out_ (out),
           todo_(todo),
           first_ (true),
           gen_ (gen)
      {
      }

      void init (CBException e)
      {
         ZLOG_DEBUG(mylog, "WriteSM::init");
         e.check ();
         setNextMethod (&WriteSM::requestWrite, e);
      }

      void requestWrite (CBException e)
      {
         ZLOG_DEBUG(mylog, "WriteSM::requestWrite");
         e.check ();
         out_->write (&write_ptr_, &write_size_, slot_[0], opt_suggested);
         slot_.wait (0, &WriteSM::writeAvailable);
      }

      void writeAvailable (CBException e)
      {
         ZLOG_DEBUG(mylog, format("WriteSM::writeAvailable: %u") % write_size_);
         e.check ();
         const size_t thiswrite = std::min (todo_, write_size_);

         if (gen_)
         {
            gen_->generate (
                  first_ ? static_cast<char*>(write_ptr_) + 4 : write_ptr_,
                  first_ ? thiswrite - 4 : thiswrite);

            if (opt_corrupt)
            {
               if (thiswrite > 4)
               {
                  const size_t ofs = 4 + random () % (thiswrite - 4);
                  *(static_cast<char*>(write_ptr_)+ofs) = random ();
               }
            }
         }

         if (first_)
         {
            ALWAYS_ASSERT(write_size_ >= 4);
            * (uint32_t*) write_ptr_ = todo_;
            first_ = false;
         }

         todo_ -= thiswrite;

         if (!todo_)
         {
            // see if we need to rewind
            if (thiswrite < write_size_)
            {
               out_->rewindOutput (write_size_ - thiswrite, slot_[0]);
               slot_.wait (0, &WriteSM::doFlush);
               return;
            }
            else
            {
               setNextMethod (&WriteSM::doFlush, e);
               return;
            }
         }

         // More work todo...
         setNextMethod (&WriteSM::requestWrite, e);
      }

      void doFlush (CBException e)
      {
         ZLOG_DEBUG(mylog, "WriteSM::doFlush");
         e.check ();
         out_->flush (slot_[0]);
         slot_.wait (0, &WriteSM::flushDone);
      }

      void flushDone (CBException e)
      {
         ZLOG_DEBUG(mylog, "WriteSM::done");
         e.check ();
         // all done
      }

   protected:
      SimpleSlots<1, WriteSM> slot_;

      scoped_ptr<ZeroCopyOutputStream> out_;

      size_t todo_;

      void * write_ptr_;
      size_t write_size_;
      bool first_;

      scoped_ptr<DataGenerator> gen_;
};

// ==========================================================================

class ReadSM : public SimpleSM<ReadSM>,
               public TrackLive
{
   public:
      ReadSM (SMManager & smm, size_t todo, ZeroCopyInputStream * in,
            DataValidator * val)
         : SimpleSM<ReadSM> (smm),
         slot_(*this),
         todo_(todo),
         in_(in),
         val_(val),
         first_ (true)
      {
      }

      void init (CBException e)
      {
         ZLOG_DEBUG(mylog, "ReadSM::init");
         setNextMethod (&ReadSM::requestRead, e);
      }

      void requestRead (CBException e)
      {
         ZLOG_DEBUG(mylog, "ReadSM::requestRead");
         e.check ();
         in_->read (&read_ptr_, &read_size_, slot_[0], 0);
         slot_.wait (0, &ReadSM::readAvail);
      }

      void readAvail (CBException e)
      {
         ZLOG_DEBUG(mylog, format("ReadSM::readAvail: %u") % read_size_);
         // the sender should never send more than expected...
         ALWAYS_ASSERT(read_size_ <= todo_);

         if (val_)
         {
            val_->validate (
                  first_ ? static_cast<const char*>(read_ptr_) + 4 : read_ptr_,
                  first_ ? read_size_ - 4 : read_size_);
         }

         first_ = false;

         todo_ -= read_size_;

         if (todo_)
            setNextMethod (&ReadSM::requestRead, e);
         else
            setNextMethod (&ReadSM::done, e);
      }

      void done (CBException e)
      {
         ZLOG_DEBUG(mylog, "ReadSM::done");
         e.check ();
      }

   protected:
      SimpleSlots<1, ReadSM> slot_;
      size_t todo_;
      scoped_ptr<ZeroCopyInputStream> in_;

      const void * read_ptr_;
      size_t read_size_;

      scoped_ptr<DataValidator> val_;
      bool first_;
};

// ==========================================================================

static void incoming (SMManager & smm, const Net::AcceptInfo & info)
{
   cerr << "Incoming connection...\n";
   smm.schedule (new BounceData (smm, info.in, info.out));
}

static void startServer (SMManager & smm, Net * net)
{
   cerr << "Press CTRL-C to stop server..." << endl;
   net->setAcceptHandler (bind (&incoming, boost::ref(smm), _1));
}

/*static void stopServer (Net * net)
{
   net->clearAcceptHandler ();
} */

static DataGenerator * getGenerator ()
{
   return (opt_validate ? new TestData () : 0);
}

static DataValidator * getValidator ()
{
   return (opt_validate ? new TestData () : 0);
}

void runClientTest (SMManager & smm, Net * net, const std::string &
      destination)
{
   cerr << "Looking up " << destination << "...";
   SingleCompletion comp;
   AddressPtr dest;
   net->lookup (destination.c_str(), &dest, comp);
   comp.wait ();
   cerr << " done\n";

   for (size_t i = 0; i<opt_loops; ++i)
   {
      cerr << "Iteration " << i+1 << "/" << opt_loops << endl;
      size_t totalsize = 0;
      system_time t1 = get_system_time ();
      for (size_t con = 0; con < opt_concurrent; ++con)
      {
         size_t size = (random () % (16*1024*1024));
         if (!size)
            size += 4;
         cerr << "Starting stream for size " << size << "... " << endl;
         Connection conn (net->connect (dest));
         smm.schedule (new ReadSM (smm, size, conn.in, getValidator ()));
         smm.schedule (new WriteSM (smm, size, conn.out, getGenerator ()));
         totalsize += size;
      }
      cerr << "Average bandwidth: ";
      ReadSM::waitAll ();
      system_time t2 = get_system_time ();
      // x 2 since we did a send and receive
      cerr << (double(2*totalsize)/(1024*1024))
           /  (double((t2-t1).total_microseconds ()) / 1000000)
         << " MB/s" << endl;
   }
}

void doLocalTest (SMManager & man)
{
   net::loopback::LoopbackConnector net;
   startServer (man, &net);
   runClientTest (man, &net, "localhost");
}

void bmiSetup ()
{
   if (!opt_listen.empty())
   {
      BMI::setInitServer (opt_listen.c_str());
   }
   else
   {
      BMI::setInitClient ();
   }
}

void runBMITest (SMManager & man)
{
   bmiSetup ();
   BMI::get ();

   BMIResource bmi;
   bmi.start ();

   scoped_ptr<net::bmi::BMIConnector> con (
         new net::bmi::BMIConnector (bmi, mylog));

   if (!opt_listen.empty())
   {
    // start server listener
      cerr << "Server listening on " << opt_listen << "..." << endl;
      con->setAcceptHandler (bind (&incoming, boost::ref(man), _1));
   }

   if (opt_remote.empty ())
   {
      cerr << "Press enter to stop server...\n";
      std::string s;
      std::getline (cin, s);
      cerr << "Waiting for connected streams to completed...\n";
      BounceData::waitAll ();
   }
   else
   {
      runClientTest (man, con.get(), opt_remote);
   }

   bmi.stop ();
}

//===========================================================================

int main (int argc, char ** args)
try
{
   const size_t seed = (getenv ("SEED") ? atoi (getenv ("SEED")) : getpid ());
   ZLOG_INFO (mylog, format("seed=%i") % getpid());
   srandom (seed);

   options_description desc ("Options");
   desc.add_options ()
      ("help", "Show program usage")
      ("loops", value<size_t>(&opt_loops)->default_value(8),
          "Number of iterations for each message size")
      ("remote", value<std::string>(&opt_remote), "Remote server to connect to")
      ("local", "Run loopback test")
      ("listen", value<std::string>(&opt_listen), "Which address to listen on (for servers)")
      ("validate", "Validate received data")
      ("corrupt", "Corrupt data to test error detection")
      ("suggested", value<size_t>(&opt_suggested)->default_value(0),
         "Suggested stream block size")
      ("concurrent", value<size_t>(&opt_concurrent)->default_value(1),
         "Number of concurrent streams")
      ;

   variables_map vm;
   store(command_line_parser(argc, args).options(desc).run(),
         vm);
   notify(vm);

   opt_validate = vm.count ("validate");
   opt_corrupt = vm.count ("corrupt");
   opt_local = vm.count ("local");


   if (vm.count ("help"))
   {
         std::cout << desc << "\n";
         return 0;
   }


   //bool opt_server = vm.count ("server");

   sm::SMManager manager (4);

   iofwdutil::ThreadPool::instance().start ();

   if (opt_local)
   {
      doLocalTest (manager);
   }
   else
   {
      runBMITest (manager);
   }

   return 0;
}
catch (std::exception & e)
{
   std::string usermsg = iofwdutil::getUserErrorMessage (e);
   if (!usermsg.empty ())
         cerr << "User error: " << usermsg << endl;

   cerr << boost::diagnostic_information (e);
}


