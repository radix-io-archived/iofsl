/**
 * Compatibility header:
 *    places all spirit/phoenix stuff in the ::ourspirit namespace 
 *    and includes the correct headers.
 */
#ifndef IOFWDUTIL_BOOST_SPIRIT_HH
#define IOFWDUTIL_BOOST_SPIRIT_HH

#include "iofwd_config.h"

#ifdef BOOST_SPIRIT_OLD

# include <boost/spirit/core.hpp>
# include <boost/spirit/symbols.hpp>
# include <boost/spirit/attribute.hpp>
# include <boost/spirit/core.hpp>
# include <boost/spirit/symbols.hpp>
# include <boost/spirit/attribute.hpp>
# include <boost/spirit/actor/push_back_actor.hpp>
# include <boost/spirit/actor/assign_actor.hpp>
# include <boost/spirit/utility/chset.hpp>

# include <boost/spirit/phoenix/primitives.hpp>
# include <boost/spirit/phoenix/operators.hpp>
# include <boost/spirit/phoenix/functions.hpp>
# include <boost/spirit/phoenix/casts.hpp>
# include <boost/spirit/phoenix/binders.hpp>



namespace ourspirit = boost::spirit;
namespace ourphoenix = phoenix;

#else

# include <boost/spirit/include/classic_core.hpp>
# include <boost/spirit/include/classic_symbols.hpp>
# include <boost/spirit/include/classic_attribute.hpp>
# include <boost/spirit/include/classic_core.hpp>
# include <boost/spirit/include/classic_symbols.hpp>
# include <boost/spirit/include/classic_attribute.hpp>
# include <boost/spirit/include/classic_push_back_actor.hpp>
# include <boost/spirit/include/classic_assign_actor.hpp>
# include <boost/spirit/include/classic_chset.hpp>

# include <boost/spirit/include/phoenix1_primitives.hpp>
# include <boost/spirit/include/phoenix1_operators.hpp>
# include <boost/spirit/include/phoenix1_functions.hpp>
# include <boost/spirit/include/phoenix1_casts.hpp>
# include <boost/spirit/include/phoenix1_binders.hpp>

namespace ourspirit = boost::spirit::classic;
namespace ourphoenix = phoenix;

#endif

#endif
