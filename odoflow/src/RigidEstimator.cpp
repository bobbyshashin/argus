#include "odoflow/RigidEstimator.h"
#include "odoflow/OpenCVMod.h"

namespace odoflow
{

	RigidEstimator::RigidEstimator()
		: outputScale( 1.0 )
	{}
	
	void RigidEstimator::SetOutputScale( double scale )
	{
		outputScale = scale;
	}
	
	bool RigidEstimator::EstimateMotion( const InterestPoints& firstPoints,
										 const InterestPoints& secondPoints,
										 PoseSE3& transform )
	{
		
		cv::Mat Hest = cv::estimateRigidTransform( firstPoints,
												   secondPoints, false );
		
		if( Hest.empty() )
		{
			return false;
		}
		
		Eigen::Matrix<double,2,3> Ab = MatToEigen<double,2,3>( Hest );
		Eigen::Matrix<double,2,2> A = Ab.block<2,2>(0,0);
		double zScale = A.determinant();
		Ab.block<2,2>(0,0) = A/zScale;
		
		Eigen::Matrix<double,4,4> H = Eigen::Matrix<double,4,4>::Identity();
		H.block<2,2>(0,0) = Ab.block<2,2>(0,0);
		H.block<2,1>(0,3) = Ab.block<2,1>(0,2)*outputScale;
		
		transform = PoseSE3(H);
		return true;
		
	}
	
}
