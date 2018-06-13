#include "PID.h"

using namespace std;

/*
* TODO: Complete the PID class.
*/

PID::PID() {}

PID::~PID() {}

void PID::Init(std::vector<double> p) {
	PID::Kp = p[0];
	PID::Ki = p[1];
	PID::Kd = p[2];
	PID::p_error = 0;
  PID::i_error = 0;
  PID::d_error = 0;
}

void PID::UpdateError(double cte) {
  d_error = cte - p_error;
  p_error = cte;
  i_error += cte;
/*	if (fabs(cte) < 1e-2) {
		i_error = 0;
	} else {
		i_error += cte;
	} */
	
	
}


double PID::NewSteer() {

	double steer_value = -Kp*p_error - Ki*i_error - Kd*d_error;
	if (steer_value >=1) {
		return  1;
	} else if (steer_value <= -1) {
  		return -1;
  	} else {
  		return steer_value;
  	}
}

