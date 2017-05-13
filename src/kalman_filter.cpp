#include "kalman_filter.h"

using Eigen::MatrixXd;
using Eigen::VectorXd;

KalmanFilter::KalmanFilter() {}

KalmanFilter::~KalmanFilter() {}

void KalmanFilter::Init(VectorXd &x_in, MatrixXd &P_in, MatrixXd &F_in,
                        MatrixXd &H_in, MatrixXd &R_in, MatrixXd &Q_in) {
  x_ = x_in;
  P_ = P_in;
  F_ = F_in;
  H_ = H_in;
  R_ = R_in;
  Q_ = Q_in;
  I_ = MatrixXd::Identity(x_in.size(), x_in.size());
}

void KalmanFilter::Predict() {
  x_ = F_ * x_;
  P_ = F_ * P_ * F_.transpose() + Q_;
}

void KalmanFilter::Update(const VectorXd &z) {
  const VectorXd y = z - H_ * x_;
  UpdateResidual(y);
}


void KalmanFilter::UpdateEKF(const VectorXd &z) {
  const double px = x_[0];
  const double py = x_[1];
  const double vx = x_[2];
  const double vy = x_[3];
  
  const double rho = sqrt(px * px + py * py);
  const double phi = atan2(py, px); // [-PI, +PI]
  const double rho_dot = (px * vx + py * vy) / rho;
  
  VectorXd z_pred(3);
  z_pred << rho, phi, rho_dot;
 
  VectorXd y = z - z_pred;
  // Caution: Normalize the angle phi to [-PI, PI]
  if (y[1] < -M_PI) y[1] += M_PI*2;
  else if (y[1] > M_PI) y[1] -= M_PI*2;
  
  UpdateResidual(y);
}

void KalmanFilter::UpdateResidual(const VectorXd &y) {
  const MatrixXd S = H_ * P_ * H_.transpose() + R_;
  const MatrixXd K = P_ * H_.transpose() * S.inverse();
  // new prior
  x_ = x_ + (K * y);
  P_ = (I_ - K * H_) * P_;
}

