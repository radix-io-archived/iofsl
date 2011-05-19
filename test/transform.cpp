#include <iterator>
#include <set>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sys/time.h>

#include <boost/program_options.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>

#include "iofwdutil/IofwdutilLinkHelper.hh"
#include "iofwdutil/transform/GenericTransform.hh"

namespace po = boost::program_options;
using namespace std;

class TransformTest
{
  public:
    TransformTest (std::string input, std::string output) :
      input_(input),
      output_(output),
      inputPos(0),
      written(0)
    {
      struct stat st; 
      if (stat(input.c_str(), &st) == 0)
          inputSize = st.st_size;
      else
          cout << "Input file is not valid" << endl;
      inFile.open(input.c_str());
      outFile.open(output.c_str());
    }
  
    boost::shared_ptr<void> getInputData (size_t * size)
    {
      if ( inputPos + *size > inputSize)
        *size = inputSize - inputPos;
      inputPos += *size;
      boost::shared_ptr<void> ret((void*)new char[*size]);
      inFile.read( (char *)ret.get(),*size);
      return ret; 
    } 
  
    void setOutputData(boost::shared_ptr<void> mem, size_t size)
    {
       outFile.write((const char *)mem.get(), size);
       written += size;
    }
      
    ~TransformTest()
    {
       inFile.close();
       outFile.close();
    }      
    std::string input_;
    std::string output_;
    ifstream inFile;
    ofstream outFile;  
    size_t inputSize; 
    size_t inputPos;
    size_t written;
};
static const int SUPPLY_INBUF = iofwdutil::transform::SUPPLY_INBUF;
static const int TRANSFORM_DONE = iofwdutil::transform::TRANSFORM_DONE;
static const int CONSUME_OUTBUF = iofwdutil::transform::CONSUME_OUTBUF;

std::set<std::string> getTransforms(void)
{
  /* Determine what encoders/decoders are availible */
  std::set<std::string> encode;
  std::set<std::string> decode;
  std::set<std::string> cantest;

  /* Get transforms that are availible */
  registerIofwdutilFactoryClients ();

  iofwdutil::transform::GenericTransformEncodeFactory::instance().keys 
      (std::inserter(encode, encode.begin()));
   
  iofwdutil::transform::GenericTransformDecodeFactory::instance().keys 
      (std::inserter(decode, decode.begin()));

  std::set_intersection (encode.begin(), encode.end(),
       decode.begin(), decode.end(), std::inserter(cantest, cantest.begin()));
  
  return cantest;  
}


void doList()
{
  std::set<std::string> transforms = getTransforms();
  std::cout << "Transforms" << std::endl;
  BOOST_FOREACH (const std::string & s, transforms)
  {
    cout << "\t" << s << ":ENC" << endl;
    cout << "\t" << s << ":DEC" << endl;
  }
}

boost::shared_ptr<iofwdutil::transform::GenericTransform> getTransform(std::string transform)
{
  /* Get transforms that are availible */
  registerIofwdutilFactoryClients ();
  boost::shared_ptr<iofwdutil::transform::GenericTransform> t;
  char * tmp =  new char[20];
  if (strncmp(":ENC", &(((char *)transform.c_str())[transform.length() - 4]), 4) == 0)
  {   
     strncpy(tmp, transform.c_str(), transform.length() - 4);
     tmp[transform.length() - 4] = '\000';
     cout << "Using Encoder: " << tmp << endl;    
     t.reset(iofwdutil::transform::GenericTransformEncodeFactory::instance().construct(tmp)()); 
  }
  else
  {
     strncpy(tmp, transform.c_str(), transform.length() - 4);     
     tmp[transform.length() - 4] = '\000';
     cout << "Using Decoder: " << tmp << endl;
     t.reset(iofwdutil::transform::GenericTransformDecodeFactory::instance().construct(tmp)());
  }
  delete[] tmp;
  return t; 
}


void testTransform ( std::string transform, std::string input, std::string output)
{
   timespec t1, t2;
   double runtime = 0; 

   const size_t readsize =  250000000;
   int status = -99;
   bool flush = false;
  
   boost::shared_ptr<void> inputMem; 
   size_t inputSize = readsize;

   boost::shared_ptr<void> outputMem((void *)new char[readsize]);
   size_t outputSize = readsize;
   size_t outputPos = 0;

   boost::shared_ptr<iofwdutil::transform::GenericTransform> t(getTransform (transform));
   boost::shared_ptr<TransformTest> f(new TransformTest(input,output));
   status = SUPPLY_INBUF;
   while (status != TRANSFORM_DONE && inputSize != 0)
   {
      if (status == CONSUME_OUTBUF)
      {
        f.get()->setOutputData(outputMem, outputPos);
        outputPos = 0;
        outputSize = readsize;
      }
      if (status == SUPPLY_INBUF && flush != true)
      {
        inputSize = readsize;
        inputMem = f.get()->getInputData(&inputSize);
        if (inputSize < readsize)
          flush = true;
      }
      void * loc = (void *)&(((char*)outputMem.get())[outputPos]);
      size_t retSize;
      clock_gettime( CLOCK_REALTIME, &t1 );     
      t.get()->transform( (const void *const) inputMem.get(), inputSize,
                          loc, outputSize - outputPos, &retSize, &status, 
                          flush);
      clock_gettime( CLOCK_REALTIME, &t2 );
      outputPos += retSize;
      runtime = runtime + ((double) (t2.tv_sec - t1.tv_sec) +
                1.0e-9 * (double) (t2.tv_nsec - t1.tv_nsec) );
      if (flush == true && outputSize - outputPos != 0)
        break; 
   }
   f.get()->setOutputData(outputMem, outputPos);
   cout << input << "," << runtime 
        << "," << transform  << "," << f.get()->written << endl;
}

int main (int argc, char *argv[])
{
   // Declare the supported options.
   po::options_description desc("Allowed options");
   desc.add_options()
      ("help", "produce help message")
      ("list", "List registered transforms")
      ("transform", po::value<std::string>(), "Name of the transform (ZLIB,BZLIB,COPY,LZF)")
      ("input", po::value<std::string>(), "Input filename")
      ("output", po::value<std::string>(), "Output Filename")
      ("verbose", "Verbose operation")
      ;

   po::variables_map vm;
   po::store(po::parse_command_line(argc, argv, desc), vm);
   po::notify(vm);
    
   if (vm.count("help"))
   {
      std::cout << desc << std::endl;
      return 0;
   }

   if (vm.count("list"))
   {
      doList ();
      return 0;
   }

   if (vm.count("transform") == 0)
   {
      std::cout << "Missing transform flag, please see --help or --list for availible transforms"
                << std::endl;
      return -1;
   }

   if (vm.count("input") == 0)
   {
      std::cout << "No input file given" << std::endl;
      return -1;
   } 
   
   if (vm.count("output") == 0)
   {
      std::cout << "No output file given" << std::endl;
      return -1;
   }
    
   std::string transform = vm["transform"].as<std::string>();
   std::string input = vm["input"].as<std::string>();
   std::string output = vm["output"].as<std::string>();
   testTransform (transform, input, output);
}



