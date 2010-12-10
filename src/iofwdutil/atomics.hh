#ifndef ATOMICS_ATOMICS_HH
#define ATOMICS_ATOMICS_HH

#include <boost/thread.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_integral.hpp>

#include "always_assert.hh"
#include "iofwd_config.h"

/**
 * Library for portable atomic operations in C++.
 *
 * Doesn't implement the operations itself, but can use one of multiple
 * other libraries to perform the operations.
 *
 * Currently only emulation and OPA/OpenPA.
 *
 * NOTE: Only POD types supported!
 */

namespace iofwdutil
{
//===========================================================================

/**
 * Default class; emulates atomic access using locks.
 */
template <typename BASE>
class atomic_base
{
public:

   enum { USING_LOCKS = 1 };

   void store (BASE newv)
   {
      boost::mutex::scoped_lock l(lock_);
      value_ = newv;
   }

   BASE load () const
   {
      boost::mutex::scoped_lock l(lock_);
      return value_;
   }

   /**
    * store_unprotected updates the value with the need for atomicity.
    * Used mainly in situations where you know this is safe
    * (typically constructor)
    *
    * Might need memory barrier though.
    */
   void store_unprotected (BASE newv)
   {
      value_ = newv;
   }

   /**
    * Compare and swap.
    * If current value equal to compare, swap with newvalue.
    * Returns true if values were equal and swap occured.
    */
   BASE cas (BASE compare, BASE newv)
   {
      boost::mutex::scoped_lock l(lock_);
      BASE old = value_;
      if (value_ == compare)
      {
         value_ = newv;
         return true;
      }
      else
         return false;
   }

   /**
    * Decrement and test.
    *
    * Returns true if the value was decremented to 0.
    */
   bool decr_and_test ()
   {
      boost::mutex::scoped_lock l(lock_);
      return ! (--value_);
   }

   /**
    * Subtract val from value, return value before subtracting.
    */
   BASE fetch_and_decr ()
   {
      boost::mutex::scoped_lock l(lock_);
      BASE copy = value_;
      --value_;
      return copy;
   }

   /**
    * Increment value, return old value
    */
   BASE fetch_and_incr ()
   {
      boost::mutex::scoped_lock l(lock_);
      BASE copy = value_;
      ++value_;
      return copy;
   }

   /**
    * Add val to value, return value before adding
    */
   BASE fetch_and_add (BASE val)
   {
      boost::mutex::scoped_lock l(lock_);
      BASE copy = value_;
      value_ += val;
      return copy;
   }

   /**
    * Set new value, return old value
    */
   BASE swap (BASE other)
   {
      boost::mutex::scoped_lock l(lock_);
      BASE copy = value_;
      value_ = other;
      return copy;
   }

   /** Increment */
   void incr ()
   {
      boost::mutex::scoped_lock l(lock_);
      ++value_;
   }

   /** decrement */
   void decr ()
   {
      boost::mutex::scoped_lock l(lock_);
      --value_;
   }

   /** Add */
   void add (BASE val)
   {
      boost::mutex::scoped_lock l(lock_);
      value_ += val;
   }


private:
   BASE         value_;

   mutable boost::mutex  lock_;
};

//====================================================================
}

#ifdef HAVE_OPENPA

#include <opa_primitives.h>

namespace iofwdutil
{
//====================================================================

/**
 * OPA specialization for integers
 * Prefer OpenPA
 */
template <>
class atomic_base<int>
{
   typedef int BASE;

   public:

      enum { USING_LOCKS = 0 };

   void store (BASE newv)
   {
      OPA_store_int (&value_, newv);
   }

   BASE load () const
   {
      // OPA load is not const correct...
      return OPA_load_int (const_cast<OPA_int_t*>(&value_));
   }

   void store_unprotected (BASE newv)
   {
      OPA_store_int (&value_, newv);
   }

   BASE cas (BASE compare, BASE newv)
   {
      return (OPA_cas_int (&value_, compare, newv) == compare);
   }

   bool decr_and_test ()
   {
      return OPA_decr_and_test_int (&value_);
   }

   BASE fetch_and_decr ()
   {
      return OPA_fetch_and_decr_int (&value_);
   }

   BASE fetch_and_incr ()
   {
      return OPA_fetch_and_incr_int (&value_);
   }

   BASE fetch_and_add (BASE val)
   {
      return OPA_fetch_and_add_int (&value_, val);
   }

   BASE swap (BASE other)
   {
      return OPA_swap_int (&value_, other);
   }

   void incr ()
   {
      return OPA_incr_int (&value_);
   }

   void decr ()
   {
      return OPA_decr_int (&value_);
   }

   void add (BASE val)
   {
      OPA_add_int (&value_, val);
   }

   private:
      OPA_int_t value_;
};

}

#elif HAVE_GLIB

#include <glib.h>

namespace iofwdutil
{

/**
 * Glib specialization for integers
 */
template <>
class atomic_base<int>
{
   typedef int BASE;

   public:

      enum { USING_LOCKS = 0 };

   void store (BASE newv)
   {
      g_atomic_int_set (&value_, newv);
   }

   BASE load () const
   {
      return g_atomic_int_get (&value_);
   }

   void store_unprotected (BASE newv)
   {
      value_ = newv;
   }

   bool cas (BASE compare, BASE newval)
   {
      return g_atomic_int_compare_and_exchange (&value_, compare, newval);
   }

   bool decr_and_test ()
   {
      return g_atomic_int_dec_and_test (&value_);
   }

   BASE fetch_and_decr ()
   {
      return g_atomic_int_exchange_and_add (&value_, -1);
   }

   BASE fetch_and_incr ()
   {
      return g_atomic_int_exchange_and_add (&value_, 1);
   }

   BASE fetch_and_add (BASE val)
   {
      return g_atomic_int_exchange_and_add (&value_, val);
   }

   BASE swap (BASE other)
   {
      // Emulate swap using load + CAS
      int oldval;
      do
      {
         oldval = load ();
      }
      while (!g_atomic_int_compare_and_exchange (&value_, oldval, other));
      return oldval;
   }

   void incr ()
   {
      g_atomic_int_inc (&value_);
   }

   void decr ()
   {
      g_atomic_int_add (&value_, -1);
   }

   void add (BASE val)
   {
      g_atomic_int_add (&value_, val);
   }

   private:
      gint value_;
};

}

#endif

namespace iofwdutil
{

/**
 * Adds shared functionality to the atomic_base class.
 * Overloads, operators, ...
 */
template <typename BASE>
class atomic : public atomic_base<BASE>
{
private:
   BOOST_STATIC_ASSERT(boost::is_integral<BASE>::value);

public:
   atomic ()
   { }

   atomic (BASE b)
   {
      this->store_unprotected (b);
   }

   atomic (const atomic<BASE> & other)
   {
      this->store_unprotected (other.load ());
   }


public:
   /**
    * General C++ functions, defined in terms of the atomic_base
    * object.
    */

     /**
    * Assignment operator
    */
   atomic_base<BASE> & operator = (const BASE & val)
   {
      this->store (val);
      return *this;
   }

   /**
    * Increment; postincrement operator
    * NOTE: return BASE type, not atomic<BASE>
    */
   BASE operator ++ (int )
   {
      return this->fetch_and_incr ();
   }

   /**
    * Increment; preincrement
    * NOTE: return BASE type, not atomic<BASE>
    */
   BASE operator ++ ()
   {
      BASE copy = this->fetch_and_incr ();
      return ++copy;
   }

   /**
    * Decrement; postdecrement operator
    * NOTE: return BASE type, not atomic<BASE>
    */
   BASE operator -- (int)
   {
      return this->fetch_and_decr ();
   }

   /**
    * Decrement; predecrement
    * NOTE: return BASE type, not atomic<BASE>
    */
   BASE operator -- ()
   {
      BASE copy = this->fetch_and_decr ();
      return --copy;
   }


   /**
    * Don't support operator + to avoid things like:
    * a = b + c
    * which cannot be atomic.
    */
   void operator + (BASE val)
   {
      this->add (val);
   }

   void operator - (BASE val)
   {
      this->add (-val);
   }

   void operator += (BASE val)
   {
      this->add (val);
   }

   void operator -= (BASE val)
   {
      this->decr (val);
   }

   /**
    * Implicit conversion operator; const version
    */
   operator BASE () const
   {
      return this->load ();
   }
};


/**
 * SPecialization for bool.
 * Makes it an int instead.
 * Doesn't support ++, --, +, -
 */
template <>
class atomic<bool> : public atomic_base<int>
{
protected:
   typedef bool BASE;

public:
   atomic ()
   { }

   atomic (bool b)
   {
      this->store_unprotected (b);
   }

   atomic (const atomic<bool> & other)
   {
      this->store_unprotected (other.load ());
   }


public:
   /**
    * General C++ functions, defined in terms of the atomic_base
    * object.
    */

     /**
    * Assignment operator
    */
   atomic<bool> & operator = (const bool & val)
   {
      this->store (val);
      return *this;
   }

   /**
    * Implicit conversion operator; const version
    */
   operator bool () const
   {
      return this->load ();
   }
};

/**
 * Fast atomic class: includes static assert which fails if no hardware
 * support is available.
 */
template <typename T>
class fast_atomic : public atomic<T>
{
public:
   BOOST_STATIC_ASSERT(fast_atomic<T>::USING_LOCKS == 0);

   fast_atomic ()
   {
   }

   fast_atomic (const T & init)
      : atomic<T>(init)
   {
   }

};

//===========================================================================
}


#endif
