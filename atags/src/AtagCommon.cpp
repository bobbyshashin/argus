#include "atags/AtagCommon.h"
#include "argus_utils/GeometryUtils.h"
#include <boost/foreach.hpp>

using namespace argus_utils;

namespace atags 
{
	
AprilTags::TagDetection MessageToDetection( const argus_msgs::TagDetection& msg )
{
	AprilTags::TagDetection det;
	det.good = true;
	det.id = msg.id;
	det.hammingDistance = msg.hammingDistance;
	double xacc = 0, yacc = 0;
	for( unsigned int i = 0; i < 4; i++ )
	{
		det.p[i].first = msg.corners[i].x;
		det.p[i].second = msg.corners[i].y;
		xacc += msg.corners[i].x;
		yacc += msg.corners[i].y;
	}
	det.cxy.first = xacc/4.0;
	det.cxy.second = yacc/4.0;
	
	// TODO Parameters we can't populate correctly?
	det.obsCode = 0;
	det.code = 0;
	det.observedPerimeter = 0.0;
	det.homography = Eigen::Matrix3d::Identity();
	det.hxy = std::pair<float,float>( 0.0, 0.0 );
	return det;
}

argus_msgs::TagDetection DetectionToMessage( const AprilTags::TagDetection& det,
                                             const std::string& family,
                                             bool undistorted, bool normalized )
{
	argus_msgs::TagDetection msg;
	msg.family = family;
	msg.id = det.id;
	msg.hammingDistance = det.hammingDistance;
	msg.undistorted = undistorted;
	msg.normalized = normalized;
	for( unsigned int i = 0; i < 4; i++ )
	{
		msg.corners[i].x = det.p[i].first;
		msg.corners[i].y = det.p[i].second;
	}
	return msg;
}

argus_msgs::FiducialDetection TagToFiducial( const AprilTags::TagDetection& tag,
                                             const std::string& family )
{
	argus_msgs::FiducialDetection det;
	det.name = "apriltag_" + family + "_id" + std::to_string( tag.id );
	det.undistorted = false;
	det.normalized = false;
	det.points.reserve( 4 );
	for( unsigned int i = 0; i < 4; i++ )
	{
		argus_msgs::Point2D point;
		point.x = tag.p[i].first;
		point.y = tag.p[i].second;
		det.points.push_back( point );
	}
	return det;
}

std::vector<AprilTags::TagDetection> 
MessageToDetections( const argus_msgs::TagDetectionsStamped& msg )
{
	std::vector<AprilTags::TagDetection> detections;
	detections.reserve( msg.detections.size() );
	for( unsigned int i = 0; i < msg.detections.size(); i++ )
	{
		detections.push_back( MessageToDetection( msg.detections[i] ) );
	}
	return detections;
}

argus_msgs::TagDetectionsStamped 
DetectionsToMessage( const std::vector<AprilTags::TagDetection>& detections, 
                     const std::string& family, const sensor_msgs::CameraInfo& info, 
                     bool undistorted, bool normalized)
{
	argus_msgs::TagDetectionsStamped msg;
	msg.detections.reserve( detections.size() );
	for( unsigned int i = 0; i < detections.size(); i++ )
	{
		msg.detections.push_back( DetectionToMessage( detections[i], family,
		                                              undistorted, normalized ) );
	}
	msg.info = info;
	return msg;
}

argus_utils::PoseSE3 ComputeTagPose( const AprilTags::TagDetection& det, double tagSize,
                                   double fx, double fy, double px, double py )
{
	PoseSE3 pose;
	Eigen::Vector3d translation;
	Eigen::Matrix3d rotation;
	
	det.getRelativeTranslationRotation( tagSize, fx, fy, px, py,
                                              translation, rotation );
	static PoseSE3 postrotation( PoseSE3::Translation( 0, 0, 0 ), 
                                 EulerToQuaternion( EulerAngles( -M_PI/2, -M_PI/2, 0 ) ) );
	
	PoseSE3::Translation t( translation );
	PoseSE3::Quaternion q( rotation );
	PoseSE3 H_tag_cam( t, q );
	return H_tag_cam * postrotation;
}

Eigen::Matrix2d ComputeCovariance( const AprilTags::TagDetection& det )
{
	Eigen::Vector2d points[4];
	for( unsigned int i = 0; i < 4; i++ )
	{
		points[i] << det.p[i].first, det.p[i].second;
	}
	
	Eigen::Vector2d mean = Eigen::Vector2d::Zero();
	for( unsigned int i = 0; i < 4; i++ )
	{
		mean += points[i];
	}
	mean *= 0.25;
	
	Eigen::Matrix2d cov = Eigen::Matrix2d::Zero();
	for( unsigned int i = 0; i < 4; i++ )
	{
		points[i] = points[i] - mean;
		cov += points[i] * points[i].transpose();
	}
	return cov * 0.25;
}

std::pair<double, double> ComputeDiagonals( const AprilTags::TagDetection& det )
{
	double dx1 = det.p[0].first - det.p[2].first;
	double dy1 = det.p[0].second - det.p[2].second;
	double dx2 = det.p[1].first - det.p[3].first;
	double dy2 = det.p[1].second - det.p[3].second;
	double dist1 = std::sqrt( dx1*dx1 + dy1*dy1 );
	double dist2 = std::sqrt( dx2*dx2 + dy2*dy2 );
	return std::pair<double,double>( dist1, dist2 );
}
	
} // end namespace atags
