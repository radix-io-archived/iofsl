#ifndef ENCODER_UTIL_HH
#define ENCODER_UTIL_HH

#include <boost/type_traits/is_base_of.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/is_same.hpp>


namespace encoder
{

   // -- Forwards --
   class Encoder;
   class Decoder;
   class Size;

   /**
    * These can be use with boost::enable_if to disable or enable process
    * overloads.
    *
    * For example:
    *
    *   template <typename T>
    *   void process (T & proc, int i, 
    *      typename boost::enable_if<is_size_processor<T> >::type * dummy = 0)
    *   {
    *   }
    *
    * Or using the shorter versions:
    *
    *   template <typename T>
    *   void process (T & proc, int i, 
    *      typename only_size_processor<T>::type * = 0)
    *      
    */

   // Inherits from true_type if T is a size_processor
   template <typename T>
   struct is_size_processor : public boost::is_base_of<Size,T>
   {
   };

   template <typename T>
   struct is_encoder_processor : public boost::is_base_of<Encoder,T>
   {
   };

   template <typename T>
   struct is_decoder_processor : public boost::is_base_of<Decoder,T>
   {
   };


   template <typename T>
   struct only_size_processor : public boost::enable_if<is_size_processor<T> >
   {
   };

   template <typename T>
   struct only_encoder_processor : public boost::enable_if<is_encoder_processor<T> >
   {
   };

   template <typename T>
   struct only_decoder_processor : public boost::enable_if<is_decoder_processor<T> >
   {
   };


// Trick to avoid writing process functions twice: 
//   make a template function, but only have it apply to exactly two 
//   types: const P and P.
//
//  Usage:
//     process (enc, P & obj, 
//       typename process_filter<P, zoidfs_handle_t>::type * = 0)
//       {
//       }
//
//    The process_filter template makes sure that the function can only 
//    match const zoidfs_handle_t & and zoidfs_handle_t.
//
//    Furthermore, if the processfunction recursively calls other process
//    functions. (i.e. process (p, zoidfs_handle_t.data)) it will
//    automatically inherit the correct const status.
//
template <typename T, typename DEST>
struct process_filter
  : public boost::enable_if<
      typename boost::is_same<
         typename boost::remove_const<T>::type,
         typename boost::remove_const<DEST>::type >::type>
{
};

//===========================================================================


}

#endif
