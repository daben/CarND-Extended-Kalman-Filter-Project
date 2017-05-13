#include <iostream>
#include "tools.h"

using Eigen::VectorXd;
using Eigen::MatrixXd;
using std::vector;

Tools::Tools() {}

Tools::~Tools() {}

VectorXd Tools::CalculateRMSE(const vector<VectorXd> &estimations,
                              const vector<VectorXd> &ground_truth) {
  if (estimations.size() != ground_truth.size() || estimations.size() == 0) {
    throw std::invalid_argument("Estimations size null or not matching ground_truth");
  }
  
  VectorXd rmse = VectorXd::Zero(4);
  
  for (size_t i = 0; i < estimations.size(); i++) {
    VectorXd residual = estimations[i] - ground_truth[i];
    residual = residual.array() * residual.array();
    rmse += residual;
  }
  
  rmse /= estimations.size();
  rmse = rmse.array().sqrt();
  
  return rmse;
}

MatrixXd Tools::CalculateJacobian(const VectorXd& x_state) {
  // Recover state parameters
  double px = x_state(0);
  double py = x_state(1);
  const double vx = x_state(2);
  const double vy = x_state(3);
  
  // Avoid explosion dividing by small numbers
  if (fabs(px) < 1e-4) px = 1e-4;
  if (fabs(py) < 1e-4) py = 1e-4;
  
  const double p2 = px * px + py * py;
  const double p1 = sqrt(p2);
  const double p3 = p2 * p1;
  
  // Compute the Jacobian matrix
  MatrixXd Hj(3, 4);
  Hj <<  px / p1, py / p1, 0, 0,
        -py / p2, px / p2, 0, 0,
         py * (vx * py - vy * px) / p3, px * (vy * px - vx * py) / p3, px / p1, py / p1;
  
  return Hj;
}
