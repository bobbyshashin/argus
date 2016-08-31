#include "odoscan/ApproximateVoxelGridFilter.h"
#include "argus_utils/utils/ParamUtils.h"

namespace argus
{

ApproximateVoxelGridFilter::ApproximateVoxelGridFilter() {}

void ApproximateVoxelGridFilter::InitializeDerived( ros::NodeHandle& ph )
{
	FullNumericRange leafSize;
	GetParamRequired( ph, "voxel_size", leafSize );
	_leafSize.Initialize( ph, leafSize.init, "voxel_size", 
	                      "Voxel grid size" );
	_leafSize.AddCheck<GreaterThanOrEqual>( leafSize.min );
	_leafSize.AddCheck<LessThanOrEqual>( leafSize.max );

	// FullNumericRange minPointsPerVox;
	// GetParamRequired( ph, "min_voxel_points", minPointsPerVox );
	// _minPointsPerVox.Initialize( ph, minPointsPerVox.init, "min_voxel_points",
	//                              "Minimum number of points per voxel." );
	// _minPointsPerVox.AddCheck<GreaterThanOrEqual>( minPointsPerVox.min );
	// _minPointsPerVox.AddCheck<LessThanOrEqual>( minPointsPerVox.max );
	// _minPointsPerVox.AddCheck<IntegerValued>( ROUND_CLOSEST );
}

ScanFilter::FilterType::Ptr ApproximateVoxelGridFilter::CreateFilter()
{
	boost::shared_ptr<VoxelFilterType> filter = boost::make_shared<VoxelFilterType>();
	
	double ls = _leafSize;
	filter->setLeafSize( ls, ls, ls );
	// filter->setMinimumPointsNumberPerVoxel( _minPointsPerVox ); Needs > PCL 1.7
	return filter;
}

}