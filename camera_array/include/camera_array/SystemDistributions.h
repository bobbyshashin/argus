#pragma once

#include "camera_array/DistributionInterfaces.h"
#include "camera_array/SystemStates.h"

#include "argus_utils/MultivariateGaussian.hpp"

namespace camera_array
{

class RobotTargetDistribution
: public Distribution<RobotTargetState>
{
public:

	struct Properties
	{
		bool sampleTargetPoses;
		bool sampleTargetVelocities;
		bool sampleRobotPose;
		bool sampleRobotVelocity;
		
		Properties( bool targetPose = false, bool targetVel = false,
		            bool robotPose = false, bool robotVel = false );
	};
	
	RobotTargetDistribution( Properties props = Properties() );
	
	void SetMean( const RobotTargetState& b );
	
	virtual RobotTargetState Sample();
	
private:
	
	RobotTargetState base;
	Properties properties;
	argus_utils::MultivariateGaussian<6> gaussian; // Default mean zero, identity cov
	
	argus_utils::PoseSE3 SamplePose( const argus_utils::PoseSE3& mean, 
	                                 const argus_utils::PoseSE3::CovarianceMatrix& cov );
	
	argus_utils::PoseSE3::TangentVector 
	SampleVelocity( const argus_utils::PoseSE3::TangentVector& mean, 
	                const argus_utils::PoseSE3::CovarianceMatrix& cov );
	
};

}