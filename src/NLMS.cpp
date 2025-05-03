
#include <vector>
#include <cmath>
#include "NLMS.h"


// Constructor
NLMS::NLMS()
{
	setup(16, 0.001, 0.00000001);
}

// Constructor specifying a sample rate
NLMS::NLMS (int M, float mu, float eps)
{
	setup(M, mu, eps);
}

// Set the sample rate, used for all calculations
void NLMS::setup(int M, float mu, float eps)
{
	M_ = M;
	mu_ = mu;
	eps_ = eps;	
	W_.assign(M, 0.0f);
	buffer_.assign(M, 0.0f);
	writeAmount_ = 0;
}

// step through filtering process
float NLMS::step(float x_n, float d_n)
{
	// insert into buffer
	buffer_[writeAmount_] = x_n;
	
	// buildg vector and computing y
	float y = 0.0f;
	float normalizer = 0.0f;
	// starting pointer at top and counting down
	unsigned int writePointer = writeAmount_;
	// iterating through vectors to do calculations
	for (unsigned int i = 0; i < M_; i++){
		// input vec
		float xI = buffer_[i];
		// weight vec
		float wI = W_[i];
		// calc y
		y += wI * xI;
		// normalize
		normalizer += xI * xI;
		
		// reset writePointer if it's 0
		if (writePointer == 0){
			writePointer = M_ - 1;
		}
		else{
			writePointer--;
		}
	}
		
		// error
		float error = d_n - y;
		
		// normalize 
		float alpha = mu_ / (eps_ + normalizer);
		
		
	// update the weights
	writePointer = writeAmount_;
	// iterating through vectors to do calculations
	for (unsigned int i = 0; i < M_; i++){
		W_[i] += alpha * error * buffer_[i];
		
		// reset writePointer if it's 0
		if (writePointer == 0){
			writePointer = M_ - 1;
		}
		else{
			writePointer--;
		}
		
		// Advance write and read pointer
		if(++writePointer >= M_) {writePointer = 0;}
	}
	// return y;
	return y;
}

void NLMS::reset(){
	std::fill(W_.begin(), W_.end(), 0.0f);
}

// Destructor
NLMS::~NLMS()
{
}