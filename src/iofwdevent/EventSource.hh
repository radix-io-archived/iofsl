#ifndef IOFWDEVENT_EVENTSOURCE_HH
#define IOFWDEVENT_EVENTSOURCE_HH

namespace iofwdevent
{

/**
 * Represents pollable event source
 */
class EventSource
{
public:

   virtual void poll (bool wait) = 0; 

   virtual ~EventSource (); 

}; 


}

#endif
