#pragma once

#include <isam/Pose3d.h>
#include <isam/Point3d.h>

#include <ros/time.h>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include "camplex/FiducialCommon.h"

#include "graphopt/sclam_fiducial.h"
#include "argus_utils/geometry/PoseSE3.h"

#include <geometry_msgs/Point.h>

namespace argus
{

enum TargetType
{
    TARGET_STATIC, // A non-moving target
    TARGET_DYNAMIC, // A moving target with odometry info
    TARGET_DISCONTINUOUS // A moving target without odometry info
};

TargetType string_to_target( const std::string& str );
std::string target_to_string( TargetType type );

/*! \brief Convert to and from the ISAM pose representation. */
isam::PoseSE3 PoseToIsam( const PoseSE3& pose );
PoseSE3 IsamToPose( const isam::PoseSE3& is );

/*! \brief Convert to and from the ISAM point representation. */
isam::Point3d PointToIsam( const Translation3Type& msg );
Translation3Type IsamToPoint( const isam::Point3d& is );

/*! \brief Convert to and from the ISAM detection representation. */
isam::FiducialDetection DetectionToIsam( const FiducialDetection& detection );
FiducialDetection IsamToDetection( const isam::FiducialDetection& detection );

/*! \brief Convert to and from the ISAM fiducial representation. */
isam::FiducialIntrinsics FiducialToIsam( const Fiducial& fid );
Fiducial IsamToFiducial( const isam::FiducialIntrinsics& fid );

isam::MonocularIntrinsics CalibrationToIsam( const CameraCalibration& calib );
CameraCalibration IsamToCalibration( const isam::MonocularIntrinsics& calib );

}
