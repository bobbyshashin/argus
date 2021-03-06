#pragma once

#include "camera_array/CameraArrayManager.h"
#include "camera_array/SystemModels.h"
#include "camera_array/FiducialDetectionModel.h"
#include "camera_array/FiducialRewardFunction.h"
#include "camera_array/SystemDistributions.h"

#include "lookup/LookupInterface.h"

#include "nav_msgs/Odometry.h"
#include "argus_msgs/OdometryArray.h"

namespace argus
{

/*! \brief Takes care of updating state and parsing the policy outputs. */
class PolicyManager
{
public:

	typedef std::shared_ptr<PolicyManager> Ptr;
	
	PolicyManager( const ros::NodeHandle& nh, const ros::NodeHandle& ph,
	               const CameraArrayManager::Ptr& cam );
	
protected:
	
	ros::NodeHandle nodeHandle;
	ros::NodeHandle privHandle;
	
	CameraArrayManager::Ptr manager;
	std::shared_ptr<ros::Timer> updateTimer;
	ros::Subscriber targetSub;
	ros::Subscriber odomSub;
	
	nav_msgs::Odometry::ConstPtr lastOdometry;
	argus_msgs::OdometryArray::ConstPtr lastTargets;
	
	LookupInterface lookupInterface;
	FiducialDetectionModel::Ptr fiducialModel;
	
	ArrayTransitionFunction::Ptr arrayTransitionFunction;
	TargetTransitionFunction::Ptr targetTransitionFunction;
	RobotArrayTransitionFunction::Ptr systemTransitionFunction;
	ArrayActionGenerator::Ptr arrayActionGenerator;
	
	RobotArrayReward::Ptr rewardFunction;
	
	RobotArrayPolicy::Ptr policy;
	
	void TargetCallback( const argus_msgs::OdometryArray::ConstPtr& msg );
	void OdometryCallback( const nav_msgs::Odometry::ConstPtr& msg );
	void TimerCallback( const ros::TimerEvent& event );
};

}

