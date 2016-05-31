#pragma once

#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <image_geometry/pinhole_camera_model.h>

#include "odoflow/InterestPointDetector.h"
#include "odoflow/InterestPointTracker.h"
#include "odoflow/MotionEstimator.h"

#include "argus_utils/PoseSE3.h"

#include "lookup/LookupInterface.h"
#include "extrinsics_array/ExtrinsicsInfoManager.h"

namespace odoflow
{

// TODO Output displacement_raw directly from tracking estimates
// TODO Abstract keyframe, redetection policies?
// TODO Covariance estimator interface
	
/*! \brief A complete VO pipeline that consumes images and outputs velocity estimates.
 * Subscribes to "/image_raw" for the image source. Uses the lookup system to
 * get extrinsics for cameras.
 *
 * detector:
 *   type: [string] {corner, fixed, FAST} (corner)
 * tracker:
 *   type: [string] {lucas_kanade} (lucas_kanade)
 * estimator:
 *   type: [string] {rigid} (rigid) */
class VisualOdometryPipeline
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	
	typedef std::shared_ptr<VisualOdometryPipeline> Ptr;
	
	VisualOdometryPipeline( ros::NodeHandle& nh, ros::NodeHandle& ph );
	~VisualOdometryPipeline();
	
	void SetDetector( InterestPointDetector::Ptr det );
	void SetTracker( InterestPointTracker::Ptr tr );
	void SetEstimator( MotionEstimator::Ptr es );
	
	void SetRedetectionThreshold( unsigned int t );
	void SetVisualization( bool enable );
	
private:
	
	ros::NodeHandle nodeHandle;
	ros::NodeHandle privHandle;
	
	ros::Publisher dispPub;
	image_transport::ImageTransport imagePort;
	image_transport::CameraSubscriber imageSub;
	image_transport::Publisher debugPub;
	
	lookup::LookupInterface lookupInterface;
	extrinsics_array::ExtrinsicsInfoManager extrinsicsManager;
	
	InterestPointDetector::Ptr detector;
	InterestPointTracker::Ptr tracker;
	MotionEstimator::Ptr estimator;
	
	bool showOutput;
	
	struct CameraRegistration
	{
		std::string name;
		ros::Time keyframeTimestamp;
		cv::Mat keyframe;
		InterestPoints keyframePointsImage;
		ros::Time lastPointsTimestamp;
		InterestPoints lastPointsImage;
		argus::PoseSE3 lastPointsPose;
	};
	std::unordered_map<std::string, CameraRegistration> cameraRegistry;
	
	unsigned int redetectionThreshold;
	unsigned int minNumInliers;
	argus::PoseSE3::CovarianceMatrix obsCovariance;
	
	void VisualizeFrame( const CameraRegistration& registration,
	                     const cv::Mat& frame,
	                     const ros::Time& timestamp );
	
	void SetKeyframe( CameraRegistration& registration,
	                  const cv::Mat& frame, 
	                  const image_geometry::PinholeCameraModel& model,
	                  const ros::Time& timestamp );
	
	bool RegisterCamera( const std::string& name );
	void ImageCallback( const sensor_msgs::ImageConstPtr& msg,	
						const sensor_msgs::CameraInfoConstPtr& info_msg );
	
};

} // end namespace odoflow
