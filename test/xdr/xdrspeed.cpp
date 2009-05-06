#include <iostream>
#include <vector>
#include <typeinfo>
#include <string.h>
#include <vector>
#include "iofwdutil/xdr/XDRReader.hh"
#include "iofwdutil/xdr/XDRWriter.hh"
#include "iofwdutil/Timer.hh"
#include "iofwdutil/xdr/XDRSizeProcessor.hh"
#include "iofwdutil/zoidfs-xdr.hh"

#include "common/zoidfs-wrapped.hh"

#define MEMSIZE 32*1024*1024


// How long we measure speeds 
#define MEASURETIME 2.0

#define MB (1024*1024)

using namespace iofwdutil::xdr; 
using namespace std;


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

template <typename T>
void type_init (T & t)
{
   char * ptr = (char*) &t; 
   for (unsigned int i=0; i<sizeof(T); ++i)
      *ptr++ = random();
}

template <typename T>
bool type_compare (const T & t1, const T & t2)
{
   return memcmp (&t1, &t2, sizeof(T))==0; 
}

template <typename T> 
void testSpeed (char * mem, size_t size)
{
   XDRReader reader (mem, size); 
   XDRWriter writer (mem, size); 
   iofwdutil::Timer t;
   T test; 
   double elapsed; 
   int count; 

   // Round size of basic type up to multiple of 4
   // (see XDR specs)
   const unsigned int xdrsize = getXDRSize (test).actual; 
   const unsigned int loopcount = size / xdrsize; 

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
         writer << test; 
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
         reader >> test; 
      t.stop (); 
      elapsed += t.elapsed().toDouble(); 
      ++count; 
   } while (elapsed < MEASURETIME); 
   cout << double(loopcount * count * sizeof(T))/(MB*elapsed) << "MB/s" << endl;
}

void testCreate ()
{
   cout << "Testing xdrmem_create / xdr_destroy speed... ";
   cout.flush (); 

   char mem[4]; 
   size_t memsize = sizeof (mem); 

   const unsigned long  LOOP_COUNT = 3000000; 

   iofwdutil::Timer t;
   double elapsed = 0; 
   long count = 0; 
   do
   {
      t.start(); 
      for (unsigned long  i=0; i<LOOP_COUNT; ++i)
      {
         XDRReader r (&mem[0], memsize); 
      }
      t.stop (); 
      ++count;
      elapsed += t.elapsed().toDouble (); 
   } while (elapsed < MEASURETIME); 

   cout << ((elapsed*iofwdutil::TimeVal::US_PER_SECOND) / (count*LOOP_COUNT))  << "us overhead" << endl; 
}

template <typename T>
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
   const size_t xdrsize = getXDRSize (source).actual; 
   std::vector<char> buf(xdrsize); 
   XDRReader reader (&buf[0], buf.size()); 
   XDRWriter writer (&buf[0], buf.size()); 
   
   bool fail = false; 

   for (unsigned int i=0; i<100; ++i)
   {
      type_init (source); 
      type_init (dest); 

      writer.rewind (); 
      reader.rewind (); 
      writer << source;
      reader >> dest; 

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

void validateString ()
{
   enum {BUFSIZE = 2048} ; 
   char buf1[BUFSIZE]; 
   char buf2[BUFSIZE]; 

   char mem[sizeof(buf1)+4]; 

   XDRReader reader (mem, sizeof (mem)); 
   XDRWriter writer (mem, sizeof (mem)); 

   bool failed = false; 

   cout << "Validating string encoding/decoding... "; 
   cout.flush (); 
   for (unsigned int i=0; i<100; ++i)
   {
      memset (buf1, 0, sizeof(buf1)); 
      memset (buf2, 0, sizeof(buf2)); 

      const unsigned int len = random () % (sizeof (buf1)-1); 
      char * ptr = buf1; 
      for (unsigned int j=0; j<len; ++j)
         *ptr++ =(random() % 254) + 1;
      *ptr++=0; 

      reader.rewind (); 
      writer.rewind (); 

      process (writer, XDRString(buf1, sizeof(buf1))); 
      process (reader, XDRString(buf2, sizeof(buf2))); 

      if (strcmp (buf1, buf2)!=0)
      {
         failed = true; 
         ret = 1; 
         break; 
      }
   }
   cout << ( failed ? "FAILED" : "OK" ) << endl; 
}

template <typename T>
void validateSizeProcessor ()
{
   T dummy; 
   static char buf[4096]; 
   XDRReader reader (buf, sizeof(buf)); 
   XDRWriter writer (buf, sizeof(buf)); 
   
   cout << "Checking if sizes are valid for " 
      << getName<T>() << "... ";
   cout.flush (); 
   writer << dummy; 
   reader >> dummy; 
   const size_t writersize = writer.getPos (); 
   const size_t readersize = reader.getPos (); 
   if (readersize == writersize)
   {
      //cout << "OK (" << readersize << ")" << endl; 
   }
   else
   {
      cout << "Failed! (reader: " << readersize << ", writer: "
         << writersize << ")" << endl; 
      ret = 1; 
   }
   cout.flush (); 
   const XDRSizeProcessor::XDRSize predicted = getXDRSize (dummy); 
   const size_t pmin = predicted.actual; 
   const size_t pmax = predicted.max; 
   if (readersize <= pmin && readersize <= pmax)
   {
      cout << readersize << " in [" << predicted.actual << "," 
         << predicted.max << "], OK" << endl; 
   }
   else
   {
      cout << "FAILED: " << readersize << " not in [" << predicted.actual << "," 
         << predicted.max << "]" << endl; 
      ret = 1; 
   }
}

template <typename T>
void validateXDRSpecSize (size_t expected)
{
   T dummy; 
   const size_t actual = getXDRSize (dummy).actual; 
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

int main (int UNUSED(argc), const char * UNUSED(args[]))
{
   std::vector<char> mem (MEMSIZE); 

   iofwdutil::Timer::dumpInfo (cout); 
   
   validateXDRSpecSize<uint8_t> (4); 
   validateXDRSpecSize<uint16_t> (4); 
   validateXDRSpecSize<uint32_t> (4); 
   validateXDRSpecSize<uint64_t> (8); 

   validateXDRSpecSize<int8_t> (4); 
   validateXDRSpecSize<int16_t> (4); 
   validateXDRSpecSize<int32_t> (4); 
   validateXDRSpecSize<int64_t> (8);  

   validateSizeProcessor<uint8_t> (); 
   validateSizeProcessor<uint16_t> (); 
   validateSizeProcessor<uint32_t> (); 
   validateSizeProcessor<uint64_t> (); 
   
   validateSizeProcessor<int8_t> (); 
   validateSizeProcessor<int16_t> (); 
   validateSizeProcessor<int32_t> (); 
   validateSizeProcessor<int64_t> (); 

   validateSizeProcessor<zoidfs_handle_t> (); 
   validateSizeProcessor<zoidfs_attr_type_t> (); 
   validateSizeProcessor<zoidfs_time_t> (); 
   validateSizeProcessor<zoidfs_attr_t> (); 
   validateSizeProcessor<zoidfs_cache_hint_t> (); 
   validateSizeProcessor<zoidfs_sattr_t> (); 
   validateSizeProcessor<zoidfs_dirent_cookie_t> (); 
   validateSizeProcessor<zoidfs_dirent_t> (); 

   cout << endl; 

   validate<uint8_t>  (); 
   validate<uint16_t>  (); 
   validate<uint32_t>  (); 
   validate<uint64_t>  (); 
   validate<int8_t>  (); 
   validate<int16_t>  (); 
   validate<int32_t>  (); 
   validate<int64_t>  (); 

  
   validateString (); 

   validate<zoidfs_handle_t> (); 
   validate<zoidfs_attr_type_t> (); 
   
   cout << endl; 

   testCreate (); 

   testSpeed<uint8_t> (&mem[0], mem.size());
   testSpeed<uint16_t> (&mem[0], mem.size());
   testSpeed<uint32_t> (&mem[0], mem.size());
   testSpeed<uint64_t> (&mem[0], mem.size());
   testSpeed<int8_t> (&mem[0], mem.size());
   testSpeed<int16_t> (&mem[0], mem.size());
   testSpeed<int32_t> (&mem[0], mem.size());
   testSpeed<int64_t> (&mem[0], mem.size());
   
   testSpeed<zoidfs_dirent_t> (&mem[0], mem.size ()); 
   testSpeed<zoidfs_handle_t> (&mem[0], mem.size ()); 

   return ret; 
}
