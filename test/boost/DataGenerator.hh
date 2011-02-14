#ifndef TEST_BOOST_DATAGENERATOR_HH
#define TEST_BOOST_DATAGENERATOR_HH

struct DataGenerator
{
   public:
      virtual void generate (void * ptr, size_t bytes) = 0;

      virtual ~DataGenerator () {}
};

#endif
