#pragma once

#include "modprop/kalman/kalman.hpp"
#include "modprop/optim/GaussianLikelihoodModule.h"
#include "modprop/optim/MeanModule.h"
#include "argus_utils/filter/FilterInfo.h"

#include <deque>

namespace argus
{
class KalmanChain
{
public:

	KalmanChain();

	void Initialize( const VectorType& x0, const MatrixType& P0 );

	void Foreprop();
	void Invalidate();

	void RemoveEarliest();

	void AddLinearPredict( const MatrixType& A, OutputPort& Qsrc );
	void AddLinearUpdate( const MatrixType& C, const VectorType& y,
	                      OutputPort& Rsrc );
	// TODO Add the nonlinear updates

	OutputPort& GetMeanLikelihood();

private:

	enum ChainType
	{
		CHAIN_PREDICT,
		CHAIN_UPDATE
	};

	KalmanPrior _prior;

	std::deque<ChainType> _types;
	// NOTE Use deques because we need pointers to stay valid
	std::deque<PredictModule> _predicts;
	std::deque<UpdateModule> _updates;
	std::deque<GaussianLikelihoodModule> _likelihoods;
	MeanModule _meanLikelihood;

	KalmanOut& GetLastModule();
	KalmanIn& GetFirstModule();
};
}