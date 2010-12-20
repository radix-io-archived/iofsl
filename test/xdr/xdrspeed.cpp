#include <iostream>
#include <vector>
#include <typeinfo>
#include <string.h>
#include <vector>

#include "iofwdutil/Timer.hh"

#include "encoder/xdr/XDRReader.hh"
#include "encoder/xdr/XDRWriter.hh"
#include "encoder/xdr/XDRSizeProcessor.hh"

#include "encoder/none/Reader.hh"
#include "encoder/none/Writer.hh"
#include "encoder/none/Size.hh"

#include "zoidfs/util/zoidfs-xdr.hh"
#include "zoidfs/util/zoidfs-wrapped.hh"

#define MEMSIZE 32*1024*1024


// How long we measure speeds
#define MEASURETIME 0.5
#define LOOP_COUNT 300000

#define MB (1024*1024)

using namespace encoder::xdr;
using namespace encoder;
using namespace std;
using namespace zoidfs;


enum test_enum {
   TEST = 1
} ;


// Return code
int ret = EXIT_SUCCESS;

// Helper for type->name mapping
template <typename T>
inline const char * getName ()
{ return typeid(T).name(); }

#define GETNAMEHELPER(a) \
   template <> inline const char * getName<a>() { return #a; }

GETNAMEHELPER(uint8_t)
GETNAMEHELPER(uint16_t)
GETNAMEHELPER(uint32_t)
GETNAMEHELPER(uint64_t)

GETNAMEHELPER(int8_t)
GETNAMEHELPER(int16_t)
GETNAMEHELPER(int32_t)
GETNAMEHELPER(int64_t)

GETNAMEHELPER(zoidfs_handle_t)
GETNAMEHELPER(zoidfs_attr_type_t)
GETNAMEHELPER(zoidfs_time_t)
GETNAMEHELPER(zoidfs_attr_t)
GETNAMEHELPER(zoidfs_cache_hint_t)
GETNAMEHELPER(zoidfs_sattr_t)
//GETNAMEHELPER(zoidfs_dirent_cookie_t)
GETNAMEHELPER(zoidfs_dirent_t)


GETNAMEHELPER(float)
GETNAMEHELPER(double)

#undef GETNAMEHELPER

// ==================== HELPER FUNCTIONS FOR DEALING WITH TYPES

static void randfill (void * d, size_t max)
{
   char * ptr = static_cast<char*>(d);
   for (unsigned int i=0; i<max; ++i)
      *ptr++ = static_cast<char>(random());
}

static void randstring (char * dst, size_t bufsize)
{
   assert(bufsize);
   const size_t len = (random() & 0x01 ? 0 : (random () % bufsize) + 1) ;
   for (size_t i=0; i<len; ++i)
      dst[i] = 'A' + (random() % 26);
   dst[len]=0;
}

template <typename T>
void type_init (T & t)
{
   randfill (&t, sizeof(T));
}

template <typename T>
bool type_compare (const T & t1, const T & t2)
{
   return memcmp (&t1, &t2, sizeof(T))==0;
}

template <>
void type_init (zoidfs_dirent_t & d)
{
   randfill (&d, sizeof(d));
   randstring (d.name, sizeof(d.name));
}

template <typename SIZE, typename T>
encoder::Size::SizeInfo getSize (const T & t)
{
   SIZE size;
   process (size, t);
   return size.size ();
}

template <typename SIZE, typename WRITER, typename READER, typename T>
void testSpeed (char * mem, size_t size)
{
   READER reader (mem, size);
   WRITER writer (mem, size);
   iofwdutil::Timer t;
   T test;

   type_init (test);

   double elapsed;
   int count;

   // Round size of basic type up to multiple of 4
   // (see XDR specs)
   const size_t xdrsize = getSize<SIZE>(test).getMaxSize();
   const size_t loopcount = size / xdrsize;

   cout << "Testing speed of " << getName<T>() << " encode (type size "
      << xdrsize << ")... ";
   cout.flush ();
   elapsed =0;
   count = 0;
   do
   {
      writer.rewind ();
      t.start ();
      for (unsigned int i=0; i<loopcount; ++i)
         process (writer, test);
      t.stop ();
      elapsed += t.elapsed().toDouble ();
      ++count;
   } while (elapsed < MEASURETIME);
   cout << double(loopcount*count*sizeof(T))/(MB*elapsed) << "MB/s" << endl;

   cout << "Testing speed of " << getName<T>() << " decode (type size "
      << xdrsize << ")... ";
   cout.flush ();
   elapsed = 0;
   count = 0;
   do
   {
      reader.rewind();
      t.start ();
      for (unsigned int i=0; i<loopcount; ++i)
         process (reader, test);
      t.stop ();
      elapsed += t.elapsed().toDouble();
      ++count;
   } while (elapsed < MEASURETIME);
   cout << double(loopcount * count * sizeof(T))/(MB*elapsed) << "MB/s" << endl;
}

template <typename SIZE, typename WRITER, typename READER>
void testCreate ()
{
   cout << "Testing create / destroy speed... ";
   cout.flush ();

   char mem[4];
   size_t memsize = sizeof (mem);


   iofwdutil::Timer t;
   double elapsed = 0;
   long count = 0;
   do
   {
      t.start();
      for (unsigned long  i=0; i<LOOP_COUNT; ++i)
      {
         READER r (&mem[0], memsize);
      }
      t.stop ();
      ++count;
      elapsed += t.elapsed().toDouble ();
   } while (elapsed < MEASURETIME);

   cout << ((elapsed*static_cast<double>(iofwdutil::TimeVal::US_PER_SECOND))
         / static_cast<double>(count*LOOP_COUNT))  << "us overhead" << endl;
}

/**
 * See if type can be encoded and decoded.
 */
template <typename SIZE, typename WRITER, typename READER, typename T>
void validate ()
{
   cout << "Validating " << getName<T>() << " encode/decode... ";
   cout.flush ();

   T source;
   T dest;

   // Note that XDR encoded size != type size:
   // The smallest integer type is 4 bytes in XDR...
   // Also, even opaque data types will be padded so that
   // their size is a multiple of 4...
   const size_t xdrsize = getSize<SIZE> (source).getActualSize();
   std::vector<char> buf(xdrsize);
   READER reader (&buf[0], buf.size());
   WRITER writer (&buf[0], buf.size());

   bool fail = false;

   for (unsigned int i=0; i<100; ++i)
   {
      type_init (source);
      type_init (dest);

      writer.rewind ();
      reader.rewind ();
      process (writer, source);
      process (reader, dest);

      if (!type_compare(source, dest))
      {
         ret = 1;
         fail = true;
         break;
      }
   }
   if (!fail)
      cout << "OK" << endl;
   else
      cerr << getName<T>() << " FAILED!\n";
}

/**
 * Generate random string, try to encode and decode and check if the string is
 * preserved.
 */
template <typename SIZE, typename WRITER, typename READER>
void validateString ()
{
   enum {BUFSIZE = 2048} ;
   char buf1[BUFSIZE];
   char buf2[BUFSIZE];

   char mem[sizeof(buf1)+4];

   READER reader (mem, sizeof (mem));
   WRITER writer (mem, sizeof (mem));

   bool failed = false;

   cout << "Validating string encoding/decoding... ";
   cout.flush ();
   for (unsigned int i=0; i<100; ++i)
   {
      memset (buf1, 0, sizeof(buf1));
      memset (buf2, 0, sizeof(buf2));

      const unsigned int len = static_cast<unsigned int>(random () % (sizeof
               (buf1)-1));
      char * ptr = buf1;
      for (unsigned int j=0; j<len; ++j)
         *ptr++ = static_cast<char>((random() % 254) + 1);
      *ptr++=0;

      reader.rewind ();
      writer.rewind ();

      process (writer, EncString(buf1, sizeof(buf1)));
      process (reader, EncString(buf2, sizeof(buf2)));

      if (strcmp (buf1, buf2)!=0)
      {
         failed = true;
         ret = 1;
         break;
      }
   }
   cout << ( failed ? "FAILED" : "OK" ) << endl;
}

/**
 * Encodes, tries to decode, and checks if the encoded size falls within the
 * range indicated by the SIZE processor.
 */
template <typename SIZE, typename WRITER, typename READER, typename T>
void validateSizeProcessor ()
{
   T dummy;

   type_init (dummy);

   static char buf[4096];
   READER reader (buf, sizeof(buf));
   WRITER writer (buf, sizeof(buf));

   cout << "Checking if sizes are valid for "
      << getName<T>() << "... ";
   cout.flush ();
   process (writer, dummy);
   process (reader, dummy);
   const size_t writersize = writer.getPos ();
   const size_t readersize = reader.getPos ();
   if (readersize == writersize)
   {
      //cout << "OK (" << readersize << ")" << endl;
   }
   else
   {
      cout << "Failed! Reader and write disagree about encoded size "
         "(reader: " << readersize << ", writer: "
         << writersize << ")" << endl;
      ret = 1;
   }
   cout.flush ();
   const Size::SizeInfo predicted = getSize<SIZE> (dummy);
   const size_t pactual = predicted.getActualSize();
   const size_t pmax = predicted.getMaxSize();
   if ((readersize != pactual) || (pactual > pmax))
   {
      cout << "FAILED: encoded size=" << readersize 
         << " size processor predicted actual=" <<
           pactual << ", max=" << pmax << endl;
      ret = 1;
    }
   else
   {
      cout << readersize << " in [" << predicted.getActualSize() << ","
         << predicted.getMaxSize() << "], OK" << endl;
   }
}

//===========================================================================
//===== XDR Specific Tests ==================================================
//===========================================================================

template <typename T>
void validateXDRSpecSize (size_t expected)
{
   T dummy;
   const size_t actual = getXDRSize (dummy).getActualSize();
   cout << "Checking if size of " << getName<T>() << " is " << expected <<
      " as specified by the spec... ";
   if (actual == expected)
   {
      cout << "OK" << endl;
   }
   else
   {
      cout << "Failed: got " << actual << endl;
      ret = 1;
   }
}

void validateXDRSizes ()
{
   validateXDRSpecSize<uint8_t> (4);
   validateXDRSpecSize<uint16_t> (4);
   validateXDRSpecSize<uint32_t> (4);
   validateXDRSpecSize<uint64_t> (8);

   validateXDRSpecSize<int8_t> (4);
   validateXDRSpecSize<int16_t> (4);
   validateXDRSpecSize<int32_t> (4);
   validateXDRSpecSize<int64_t> (8);
}

//===========================================================================
//===========================================================================
//===========================================================================

template <typename SIZE, typename WRITER, typename READER>
void genericTests ()
{
   std::vector<char> mem (MEMSIZE);

   iofwdutil::Timer::dumpInfo (cout);
   validateSizeProcessor<SIZE,WRITER,READER,uint8_t> ();
   validateSizeProcessor<SIZE,WRITER,READER,uint16_t> ();
   validateSizeProcessor<SIZE,WRITER,READER,uint32_t> ();
   validateSizeProcessor<SIZE,WRITER,READER,uint64_t> ();

   validateSizeProcessor<SIZE,WRITER,READER,int8_t> ();
   validateSizeProcessor<SIZE,WRITER,READER,int16_t> ();
   validateSizeProcessor<SIZE,WRITER,READER,int32_t> ();
   validateSizeProcessor<SIZE,WRITER,READER,int64_t> ();

   validateSizeProcessor<SIZE,WRITER,READER,zoidfs_handle_t> ();
   validateSizeProcessor<SIZE,WRITER,READER,zoidfs_attr_type_t> ();
   validateSizeProcessor<SIZE,WRITER,READER,zoidfs_time_t> ();
   validateSizeProcessor<SIZE,WRITER,READER,zoidfs_attr_t> ();
   validateSizeProcessor<SIZE,WRITER,READER,zoidfs_cache_hint_t> ();
   validateSizeProcessor<SIZE,WRITER,READER,zoidfs_sattr_t> ();
   validateSizeProcessor<SIZE,WRITER,READER,zoidfs_dirent_cookie_t> ();
   validateSizeProcessor<SIZE,WRITER,READER,zoidfs_dirent_t> ();

   cout << endl;

   validate<SIZE,WRITER,READER,uint8_t>  ();
   validate<SIZE,WRITER,READER,uint16_t>  ();
   validate<SIZE,WRITER,READER,uint32_t>  ();
   validate<SIZE,WRITER,READER,uint64_t>  ();
   validate<SIZE,WRITER,READER,int8_t>  ();
   validate<SIZE,WRITER,READER,int16_t>  ();
   validate<SIZE,WRITER,READER,int32_t>  ();
   validate<SIZE,WRITER,READER,int64_t>  ();


   validateString<SIZE,WRITER,READER> ();

   validate<SIZE,WRITER,READER,zoidfs_handle_t> ();
   validate<SIZE,WRITER,READER,zoidfs_attr_type_t> ();

   cout << endl;

   testCreate<SIZE,WRITER,READER> ();

   testSpeed<SIZE,WRITER,READER,uint8_t> (&mem[0], mem.size());
   testSpeed<SIZE,WRITER,READER,uint16_t> (&mem[0], mem.size());
   testSpeed<SIZE,WRITER,READER,uint32_t> (&mem[0], mem.size());
   testSpeed<SIZE,WRITER,READER,uint64_t> (&mem[0], mem.size());
   testSpeed<SIZE,WRITER,READER,int8_t> (&mem[0], mem.size());
   testSpeed<SIZE,WRITER,READER,int16_t> (&mem[0], mem.size());
   testSpeed<SIZE,WRITER,READER,int32_t> (&mem[0], mem.size());
   testSpeed<SIZE,WRITER,READER,int64_t> (&mem[0], mem.size());

   testSpeed<SIZE,WRITER,READER,zoidfs_dirent_t> (&mem[0], mem.size ());
   testSpeed<SIZE,WRITER,READER,zoidfs_handle_t> (&mem[0], mem.size ());

}

int main (int UNUSED(argc), const char * UNUSED(args[]))
{
   cout << endl;
   cout << "============================================================\n";
   cout << "Testing XDR encoding...\n";
   cout << "============================================================\n";
   cout << endl;

   validateXDRSizes ();
   genericTests<XDRSizeProcessor, XDRWriter, XDRReader> ();

   cout << endl;
   cout << endl;
   cout << "============================================================\n";
   cout << "Testing NONE encoding...\n";
   cout << "============================================================\n";
   cout << endl;
   genericTests<encoder::none::Size, encoder::none::Writer,
      encoder::none::Reader> ();

   return ret;
}
