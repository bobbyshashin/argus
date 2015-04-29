#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>

#include <opencv2/highgui/highgui.hpp>

void ImageCallback( const sensor_msgs::ImageConstPtr& msg, 
					const std::string& windowName )
{
	
	cv_bridge::CvImageConstPtr frame;
	try
	{
		frame = cv_bridge::toCvShare( msg, "mono8" );
	}
	catch( cv_bridge::Exception& e )
	{
		ROS_ERROR( "VisualOdometryNode cv_bridge exception: %s", e.what() );
		return;
	}
	
	cv::imshow( windowName, frame->image );
	cv::waitKey(1);
}

int main( int argc, char** argv )
{
	ros::init( argc, argv, "image_viewer" );
	
	ros::NodeHandle nodeHandle;
	ros::NodeHandle privHandle("~");
	
	std::string topicName = nodeHandle.resolveName( "image" );
	
	image_transport::ImageTransport rImagePort( nodeHandle );
	image_transport::Subscriber rImageSub = 
		rImagePort.subscribe( topicName, 5, boost::bind( &ImageCallback, _1, topicName ) );
	
	cv::namedWindow( topicName );
	
	ros::spin();
	
	cv::destroyWindow( topicName );
	
	return 0;
}
