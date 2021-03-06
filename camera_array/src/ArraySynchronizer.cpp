#include "manycal/ArraySynchronizer.h"
#include "camplex/CaptureFrames.h"

#include <boost/foreach.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace argus
{
	
ArraySynchronizer::ArraySynchronizer( ros::NodeHandle& nh, ros::NodeHandle& ph )
: nodeHandle( nh ), privHandle( ph ), publicPort( nodeHandle ), privatePort( privHandle )
{
	std::vector< std::string > cameraNames;
	if( !privHandle.getParam( "camera_names", cameraNames ) )
	{
		ROS_ERROR_STREAM( "Please specify cameras to capture from." );
		exit( -1 );
	}
	
	std::string lookupNamespace;
	privHandle.param<std::string>( "lookup_namespace", lookupNamespace, "/lookup" );
	lookupInterface.SetLookupNamespace( lookupNamespace );
	
	cameraRegistry.reserve( cameraNames.size() );
	BOOST_FOREACH( const std::string& cameraName, cameraNames )
	{
		if( cameraRegistry.count( cameraName ) > 0 )
		{
			ROS_ERROR_STREAM( "Duplicate camera: " << cameraName );
			exit( -1 );
		}
		
		CameraRegistration registration;
		registration.name = cameraName;
		
		std::string cameraNamespace;
		if( !lookupInterface.ReadNamespace( cameraName, cameraNamespace, ros::Duration( 5.0 ) ) )
		{
			ROS_ERROR_STREAM( "Could not find namespace for " << cameraName );
		}
		std::string captureServiceName = cameraNamespace + "/capture_frames";
		ros::service::waitForService( captureServiceName );
		registration.captureClient = nodeHandle.serviceClient<camplex::CaptureFrames>( captureServiceName );
		
		cameraRegistry[ cameraName ] = ( registration );
	}
	
	imagePub = privatePort.advertiseCamera( "image_output", 2*cameraNames.size() );
	imageSub = publicPort.subscribeCamera( "image_input", 2*cameraNames.size(), &ArraySynchronizer::ImageCallback, this );
		
	captureServer = privHandle.advertiseService( "capture_array", &ArraySynchronizer::CaptureArrayCallback, this );
	
	privHandle.param( "num_simultaneous_captures", numSimultaneous, 1 );
	cameraTokens.Increment( numSimultaneous );
	pool.SetNumWorkers( numSimultaneous );
	pool.StartWorkers();
}

bool ArraySynchronizer::CaptureArrayCallback( manycal::CaptureArray::Request& req, 
                                              manycal::CaptureArray::Response& res )
{
  // TODO Figure out why images are getting dropped
  int diff = numSimultaneous - cameraTokens.Query();
  if( diff > 0 )
    {
      cameraTokens.Increment( diff );
    }
  receivedImages = 0;
  clampTime = ros::Time::now();
	// TODO Make sure buffer is cleared?
	BOOST_FOREACH( CameraRegistry::value_type& item, cameraRegistry )
	{
		argus::WorkerPool::Job job = boost::bind( &ArraySynchronizer::CaptureJob, 
		                                                this, boost::ref( item.second ) );
		pool.EnqueueJob( job );
	}

	// TODO Handle timeouts
	//ROS_INFO( "Dispatched all jobs. Waiting on semaphore." );
	//completedJobs.Decrement( cameraRegistry.size() ); // Wait for all jobs to complete
	return true;
}

void ArraySynchronizer::CaptureJob( CameraRegistration& registration )
{
	cameraTokens.Decrement(); // Manage number of active cameras
	camplex::CaptureFrames srv;
	srv.request.numToCapture = 1;
	if( !registration.captureClient.call( srv ) )
	{
		ROS_WARN_STREAM( "Could not capture from camera " << registration.name );
	}
}

void ArraySynchronizer::ImageCallback( const sensor_msgs::Image::ConstPtr& image, 
                                       const sensor_msgs::CameraInfo::ConstPtr& info )
{
	// Concurrent modification through ref is safe
	if( cameraRegistry.count( image->header.frame_id ) == 0 )
	{
		ROS_WARN_STREAM( "Received image from unregistered camera " << image->header.frame_id );
		return;
		
	}
 	CameraRegistration& registration = cameraRegistry[ image->header.frame_id ];
	ROS_INFO_STREAM( "Image received from " << image->header.frame_id );
	registration.data.image = boost::make_shared<sensor_msgs::Image>( *image );
	registration.data.info = boost::make_shared<sensor_msgs::CameraInfo>( *info );
	
	cameraTokens.Increment();
	//completedJobs.Increment();
	registration.data.image->header.stamp = clampTime;
	registration.data.info->header.stamp = clampTime;
	imagePub.publish( registration.data.image, registration.data.info );
}
	
} // end namespace manycal
