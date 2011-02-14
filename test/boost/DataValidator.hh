#ifndef TEST_BOOST_DATAVALIDATOR_HH
#define TEST_BOOST_DATAVALIDATOR_HH

struct DataValidator
{
   public:
      virtual void validate (const void * ptr, size_t bytes) = 0;

      virtual ~DataValidator () {}
};

#endif
