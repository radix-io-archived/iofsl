#include "iofwdutil/Factory.hh"
#include "iofwdutil/FactoryAutoRegister.hh"

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


int main ()
{
   typedef iofwdutil::Factory<string,Unit> MyF;
   
   // Needs to be fixed
   

   MyF::instance();
   
   auto_ptr<Unit> cm (MyF::construct ("CM")(0));

   auto_ptr<Unit> mm (MyF::construct ("MM")(1));
   
}


