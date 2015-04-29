#ifndef _V4L2D_RESOURCE_USER_H_
#define _V4L2D_RESOURCE_USER_H_

#include <ros/ros.h>
#include "resource_management/ResourceGrant.h"
#include "resource_management/ResourceRequest.h"

#include "resource_management/RequestResources.h"
#include "resource_management/ReleaseResources.h"

namespace resource_management
{

	// NOTE Unsynchronized
	class ResourceUser
	{
	public:
		
		ResourceUser( ros::NodeHandle& nh, ros::NodeHandle& ph );
		
		bool HasResources() const;
		
		void AcquireResources();
		void RelinquishResources();
		
	private:
		
		ros::NodeHandle nodeHandle;
		ros::NodeHandle privHandle;
		
		ros::ServiceClient requestClient;
		ros::ServiceClient releaseClient;
		
		std::vector<ResourceRequest> resourceRequirements;
		bool hasResources;
		unsigned int currentGrantID;
		
	};
	
}

#endif
