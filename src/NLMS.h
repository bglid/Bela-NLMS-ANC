// NLMS filter for noise cancellation

#pragma once

class NLMS {

public:
	// Constructor
	NLMS();
	
	// Constructor
	NLMS (int M, float mu, float eps);
	
	// Set the sample rate, used for all calculations
	void setup(int M, float mu, float eps);
	
	// step through filtering process
	float step(float x_n, float d_n);
	
	// Reset
	void reset();
	
	// Destructor
	~NLMS();

private:
	// State variables, not accessible to the outside world
	int   M_;
	float mu_;
	float eps_;
	std::vector<float> W_;
	std::vector<float> buffer_;
	int writeAmount_;
};