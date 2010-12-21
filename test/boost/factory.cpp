#include "iofwdutil/Factory.hh"
#include "iofwdutil/FactoryException.hh"
#include "iofwdutil/FactoryAutoRegister.hh"

#include <boost/ref.hpp>
#include <boost/utility.hpp>
#include <boost/test/unit_test.hpp>
#include <string>
#include <memory>


using namespace std;

class Unit
{
   public:
      FACTORY_CONSTRUCTOR_PARAMS(size_t);
   virtual string getName () const = 0;
};

class CM : public Unit
{
   public:
   CM (size_t )
   {
   }

   string getName () const
   { return "CM"; }
};

class MM : public Unit
{
   public:
   MM (size_t)
   {
   }

   string getName () const
   { return "MM"; }
};

FACTORYAUTOREGISTER(string,Unit,CM,"CM");
FACTORYAUTOREGISTER(string,Unit,MM,"MM");

// == Test for passing references

struct nocopy : public boost::noncopyable
{
};

struct RefTestBase
{
   FACTORY_CONSTRUCTOR_PARAMS(nocopy &, long);

};

struct RefTestImpl : public RefTestBase
{
   RefTestImpl (nocopy &, long )
   {
   }
};

FACTORYAUTOREGISTER(int,RefTestBase,RefTestImpl,1);

BOOST_AUTO_TEST_SUITE (Factory)

struct F
{
};

BOOST_FIXTURE_TEST_CASE ( basic, F )
{
   typedef iofwdutil::Factory<string,Unit> MyF;
   BOOST_TEST_MESSAGE ("Checking basic functionality");
   
   // Needs to be fixed
   

   MyF::instance();
   BOOST_CHECK_EQUAL (MyF::instance().size(), 2);
   
   auto_ptr<Unit> cm (MyF::construct ("CM")(0));
   BOOST_CHECK_EQUAL (cm->getName(), "CM");

   auto_ptr<Unit> mm (MyF::construct ("MM")(1));
   BOOST_CHECK_EQUAL (mm->getName(), "MM");
   
   int i = 1;
   auto_ptr<Unit> mmm (MyF::construct ("MM")(boost::ref(i)));
   BOOST_CHECK_EQUAL (mmm->getName(), "MM");
  
  
   BOOST_CHECK_THROW (MyF::construct
         ("nonexistent")(1),iofwdutil::FactoryException);
}

BOOST_FIXTURE_TEST_CASE ( references, F )
{
   typedef iofwdutil::Factory<int,RefTestBase> MyF;
   BOOST_TEST_MESSAGE ("Checking passing reference functionality");
   
   // Needs to be fixed
   

   nocopy c;
   long l = 0;
   auto_ptr<RefTestBase> dummy (MyF::construct (1)(c,l));
}


BOOST_AUTO_TEST_SUITE_END()

