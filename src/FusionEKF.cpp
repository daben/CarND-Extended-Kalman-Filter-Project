#include "FusionEKF.h"
#include "tools.h"
#include "Eigen/Dense"
#include <iostream>

using namespace std;
using Eigen::MatrixXd;
using Eigen::VectorXd;
using std::vector;

/*
 * Constructor.
 */
FusionEKF::FusionEKF() {
  is_initialized_ = false;

  previous_timestamp_ = 0;

  // initializing matrices
  R_laser_ = MatrixXd(2, 2);
  R_radar_ = MatrixXd(3, 3);
  H_laser_ = MatrixXd(2, 4);
  Hj_ = MatrixXd(3, 4);

  // measurement covariance matrix - laser
  R_laser_ << 0.0225,      0,
                   0, 0.0225;

  // measurement covariance matrix - radar
  R_radar_ << 0.09,     0,    0,
                0, 0.0009,    0,
                0,      0, 0.09;
  
  // laser measurement matrix
  H_laser_ << 1, 0, 0, 0,
              0, 1, 0, 0;
}

/**
* Destructor.
*/
FusionEKF::~FusionEKF() {}

void FusionEKF::ProcessMeasurement(const MeasurementPackage &measurement_pack) {
  /*****************************************************************************
   *  Initialization
   ****************************************************************************/
  if (!is_initialized_) {
    // first measurement
    cout << "EKF: " << endl;
    VectorXd x(4);

    if (measurement_pack.sensor_type_ == MeasurementPackage::RADAR) {
      /**
      Convert radar from polar to cartesian coordinates and initialize state.
      */
      const double rho = measurement_pack.raw_measurements_[0];
      const double phi = measurement_pack.raw_measurements_[1];
      
      const double px = rho * cos(phi);
      const double py = rho * sin(phi);
      
      x << px, py, 0, 0;
    }
    else if (measurement_pack.sensor_type_ == MeasurementPackage::LASER) {
      /**
      Initialize state.
      */
      const double px = measurement_pack.raw_measurements_[0];
      const double py = measurement_pack.raw_measurements_[1];
      
      x << px, py, 0, 0;
    }
    else {
      throw runtime_error("Unknown sensor");
    }

    // Initial state covariance
    MatrixXd P(4, 4);
    P << 1, 0, 0, 0,
         0, 1, 0, 0,
         0, 0, 1000, 0,
         0, 0, 0, 1000;
    
    // Transition matrix
    MatrixXd F(4, 4);
    // Process covariance
    MatrixXd Q(4, 4);
    
    // Initialize the EKF struct. H and R depend on the measurement type,
    // they will be set in the update state
    MatrixXd nullMatrix;
    ekf_.Init(x, P, F, nullMatrix, nullMatrix, Q);
    
    // Set the initial timestamp
    previous_timestamp_ = measurement_pack.timestamp_;
    
    // done initializing, no need to predict or update
    is_initialized_ = true;
    return;
  }
  
  /*****************************************************************************
   *  Prediction
   ****************************************************************************/

  // compute the time elapsed between the current and previous measurements
  const double dt = (measurement_pack.timestamp_ - previous_timestamp_) / 1e6;
  previous_timestamp_ = measurement_pack.timestamp_;
  
  // Update the transition matrix
  ekf_.F_ << 1,  0, dt,  0,
             0,  1,  0, dt,
             0,  0,  1,  0,
             0,  0,  0,  1;
  
  // Update the process covariance matrix
  const double dt2 = dt * dt;
  const double dt3 = dt * dt2 * 0.5;
  const double dt4 = dt * dt3 * 0.5;
  const double noise_ax = 9.0;
  const double noise_ay = 9.0;
  
  ekf_.Q_ << dt4 * noise_ax, 0, dt3 * noise_ax, 0,
             0, dt4 * noise_ay, 0, dt3 * noise_ay,
             dt3 * noise_ax, 0, dt2 * noise_ax, 0,
             0, dt3 * noise_ay, 0, dt2 * noise_ay;
  
  ekf_.Predict();

  /*****************************************************************************
   *  Update
   ****************************************************************************/

  if (measurement_pack.sensor_type_ == MeasurementPackage::RADAR) {
    // Radar updates
    // Jacobian for current state
    ekf_.H_ = Hj_ = tools.CalculateJacobian(ekf_.x_);
    ekf_.R_ = R_radar_;
    ekf_.UpdateEKF(measurement_pack.raw_measurements_);
  } else {
    // Laser updates
    ekf_.H_ = H_laser_;
    ekf_.R_ = R_laser_;
    ekf_.Update(measurement_pack.raw_measurements_);
  }
  
  // print the output
  cout << "x_ = " << ekf_.x_ << endl;
  cout << "P_ = " << ekf_.P_ << endl;
}
