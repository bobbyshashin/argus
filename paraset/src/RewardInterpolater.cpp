#include "paraset/RewardInterpolater.h"
#include "argus_utils/utils/ParamUtils.h"
#include "argus_utils/utils/MapUtils.hpp"

#include <sstream>

using namespace paraset;

namespace argus
{

InterpolationMode StringToInterpMode( const std::string& str )
{
	if( str == "zero_order_hold" )
	{
		return INTERP_ZERO_ORDER_HOLD;
	}
	else if( str == "piecewise_linear" )
	{
		return INTERP_PIECEWISE_LINEAR;
	}
	else
	{
		throw std::runtime_error( "StringToInterpMode: Unknown string: " + str );
	}
}

RewardInterpolater::RewardInterpolater() {}

void RewardInterpolater::Initialize( ros::NodeHandle& nh,
                                     ros::NodeHandle& ph )
{
	GetParamRequired( ph, "cache_time", _cacheTime );

	std::string modeStr;
	GetParamRequired( ph, "interpolation_mode", modeStr );
	_interpMode = StringToInterpMode( modeStr );

	_rewardSub = nh.subscribe( "reward", 
	                           0, 
	                           &RewardInterpolater::RewardCallback, 
	                           this );
}

double RewardInterpolater::Evaluate( const ros::Time& t ) const
{
	switch( _interpMode )
	{
		case INTERP_ZERO_ORDER_HOLD:
			return EvaluateZOH( t );
		case INTERP_PIECEWISE_LINEAR:
			return EvaluatePWL( t );
		default:
			throw std::invalid_argument( "RewardInterpolater: Invalid interpolation mode." );
	}
}

void RewardInterpolater::RewardCallback( const RewardStamped::ConstPtr& msg )
{
	_rewards[ msg->header.stamp ] = msg->reward;

	while( ComputeSpan() > _cacheTime )
	{
		remove_lowest( _rewards );
	}
}

double RewardInterpolater::ComputeSpan() const
{
	return ( get_highest_key( _rewards ) - get_lowest_key( _rewards ) ).toSec();
}

double RewardInterpolater::EvaluatePWL( const ros::Time& t ) const
{
	RewardSeries::const_iterator prev, after;
	if( !get_closest_lesser_eq( _rewards, t, prev ) ||
	    !get_closest_greater_eq( _rewards, t, after ) )
	{
		std::stringstream ss;
		ss << "RewardInterpolater: Query " << t << " out of range.";
		throw std::out_of_range( ss.str() );
	}

	double dtPrev = ( t - prev->first ).toSec();
	double dtAft = ( after->first - t ).toSec();
	double dtSum = dtPrev + dtAft;
	double interp = ( dtPrev * after->second + dtAft * prev->second ) / dtSum;
	return interp;
}

double RewardInterpolater::EvaluateZOH( const ros::Time& t ) const
{
	RewardSeries::const_iterator prev;
	if( !get_closest_lesser_eq( _rewards, t, prev ) )
	{
		throw std::out_of_range( "RewardInterpolater: Query out of range." );
	}

	return prev->second;
}

}