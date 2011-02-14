#include "TrackLive.hh"


iofwdutil::fast_atomic<int> TrackLive::counter_ (0);
boost::mutex                TrackLive::lock_;
boost::condition_variable   TrackLive::cond_;


