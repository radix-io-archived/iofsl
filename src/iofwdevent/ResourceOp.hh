#ifndef IOFWDEVENT_RESOURCEOP_HH
#define IOFWDEVENT_RESOURCEOP_HH

namespace iofwdevent
{
//===========================================================================

/**
 * Base callback class for blocking operations.
 *
 * @TODO: Remove this; Deprecated. Use CBType instead.
 */
class ResourceOp
{
public:

   /// Called when the operation completed succesfully
   virtual void success () =0;

   /// Called when the operation was cancelled.
   virtual void cancel () =0;

   /// Called to transport the exception to the invoking thread
   ///virtual void exception () =0;

   virtual ~ResourceOp ();

#ifdef WENEEDEXTRADATA
protected:
   void * resourcedata_;

   friend void * getResourceOpData (const ResourceOp * op);
   friend void setResourceOpData (ResourceOp * op);
#endif
};

//===========================================================================

#ifdef WENEEDEXTRADATA
void setResourceOpData (ResourceOp * op, void * data)
{
   op->resourcedata_ = data;
}

void * getResourceOpData (const ResourceOp * op)
{
   return op->resourcedata_;
}
#endif

//===========================================================================
}

#endif
