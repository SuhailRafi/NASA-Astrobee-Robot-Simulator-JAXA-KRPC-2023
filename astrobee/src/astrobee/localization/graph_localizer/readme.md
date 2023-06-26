\page graphlocalizer Graph Localizer

# Package Overview
The GraphLocalizer uses the GraphOptimizer (please first read the documentation in the GraphOptimizer package as this package uses many objects from there) to track the pose, velocity and IMU biases (the set of these three state parameters is referred to as a CombinedNavState) for a robot.  It uses a set of measurements (IMU, optical flow, map-based image features, ar tag detections, 3D handrail and wall plane points) to generate factors that constrain its history of CombinedNavStates.  
## Code Structure
The GraphLocalizer package contains the GraphLocalizer, GraphLocalizerWrapper, and GraphLocalizerNodelet objects.  The GraphLocalizer handles the creation of factors and contains the GraphOptimizer, while the GraphLocalizerWrapper provides an interface that can be used both online with the GraphLocalizerNodelet and offline in a ROS free environement as is done in the GraphBag tool (see GraphBag package).  The GraphLocalizerWrapper converts ROS messages to localization_measurement objects and subsequently passes these to the GraphLocalizer so that the GraphLocalizer does not contain any ROS code.  Finally, the GraphLocalizerNodelet is a ROS nodelet that subscribes to the required messages and passes these to the GraphLocalizerWrapper.  It also publishes required messages and TFs for online usage.

# Background
For more information on the theory behind the GraphLocalizer and the factors used, please see our paper (TODO(rsoussan): link paper when publicly available).  
# Background (until paper is linked)
 The GraphLocaluzer uses the `CombinedImuFactor` (_Forster, Christian, et al. "IMU preintegration on manifold for efficient visual-inertial maximum-a-posteriori estimation." Georgia Institute of Technology, 2015._) from gtsam which intelligently integrates imu measurements and provides an error function for relative states (consisting of pose, velocity, and imu biases) that depends on each state.  Additionally, the graph localizer uses the `SmartProjectionFactor` (_Carlone, Luca, et al. "Eliminating conditionally independent sets in factor graphs: A unifying perspective based on smart factors." 2014 IEEE International Conference on Robotics and Automation (ICRA). IEEE, 2014._) which is a structureless visual odometry factor that combines feature tracks from different camera poses and uses these measurements to estimate first the 3D feature, then project this feature into each measurement to create the factor.  The factor is structureless since it does not optimize for the 3D feature, rather it marginalizes this out using a similar null-space projection as presented in (_Mourikis, Anastasios I., and Stergios I. Roumeliotis. "A multi-state constraint Kalman filter for vision-aided inertial navigation." Proceedings 2007 IEEE International Conference on Robotics and Automation. IEEE, 2007._).  Additionally, image projection measurements from map to image space are incorporated into the graph.   


# Important Classes

## Graph Localizer
The GraphLocalizer is a GraphOptimizer that contains measurement specific FactorAdders and state parameter specific NodeUpdaters.  These are detailed further in the following sections.

## FactorAdders
### LocFactorAdder
The LocFactorAdder takes map-based image feature measurements and generates either LocProjectionFactors or LocPoseFactors, depeneding on if the LocProjectionFactors suffered cheirality errors or not.

### ProjectionFactorAdder
The ProjectionFactorAdder creates bundle-adjustment ProjectionFactors for tracked image features. (Not currently used).

### RotationFactorAdder
The RotationFactorAdder generates a relative rotation using tracked image features in two successive images.  (Not currently used).

### SmartProjectionCumulativeFactorAdder
The SmartProjectionCumulativeFactorAdder generates visual odometry smart factors using image feature tracks.  It contains options for the minimum disparity allowed for feature tracks, minimum separation in image space for added feature tracks, whether to use a rotation-only factor if created smart factors suffer from cheirality errors, and more. It can generate factors using maximally spaced measurements in time to include a longer history of measurements while including only a maximum number of total measurements or only include measurements from a given set with a set spacing (toggle these with the use\_allowed\_timestamps option).  

### StandstillFactorAdder
The StandstillFactorAdder creates a zero velocity prior and zero tranform between factor for successive CombinedNavState nodes when standstill is detected.  Standstill detection checks for a minimum average disparity for image feature tracks over time.

## Factors

### LocPoseFactor
The LocPoseFactor is simply a gtsam::PriorFactor\<gtsam::Pose3\> that enables the differention of a pose prior from a localization map-based image feature factor.

### LocProjectionFactor
The LocProjectionFactor is almost a direct copy of the gtsam::ProjectionFactor except it does not optimize for the 3D feature point location.

### PoseRotationFactor
The PoseRotationFactor constrains two gtsam::Pose3 nodes using their relative rotation.

### PointToHandrailEndpointFactor.h
The PointToHandrailEndpointFactor constrains a gtsam::Pose3 using a handrail endpoint detection in the sensor frame compared with the closest handrail endpoint from a know handrail. 

### PointToLineFactor.h
The PointToLineFactor constrains a gtsam::Pose3 using a point detection in the sensor frame compared with a line in the world frame.

### PointToLineSegmentFactor.h
The PointToLineSegmentFactor constrains a gtsam::Pose3 using a point detection in the sensor frame compared with a line segment in the world frame.  This factor contains a discontinuity in the Jacobian as there is zero error along the line segment axis if the point is between line segment endpoints and non-zero error otherwise.  An option to use a SILU (Sigmoid Linear Unit) approximation is provided for this case.

###PointToPlaneFactor.h
The PointToPlaneFactor constrains a gtsam::Pose3 using a point detection in the sensor frame compared with a plane in the world frame.

### RobustSmartProjectionFactor
The RobustSmartProjectionFactor adds to the gtsam::SmartProjectionFactor by providing a robust huber kernel.  Additionally, it fixes some issues in the SmartProjectionFactor allowing for a rotation-only fallback when using the JacobianSVD option and allows for proper serialization of the factor.

## NodeUpdaters
### CombinedNavStateNodeUpdater
The CombinedNavStateNodeUpdater is responsible for inserting and removing CombinedNavState nodes for the graph.  It links nodes using IMU preintegration factors (gtsam::CombinedIMUFactor).  If a node to be created's timestamp is between two existing nodes, the IMU factor linking the two existing nodes is split and the new node is inserted between them.  The CombinedNavStateNodeUpdater uses the LatestImuIntegrator object to store and utilize IMU measurements.  When sliding the window, old nodes are removed from the graph and prior factors are created for the new oldest node using the node's covariances from the most recent round of optimization if the add_priors option is enabled. 

### FeaturePointNodeUpdater
The FeaturePointNodeUpdater maintains feature point nodes for image features.  It removes old point nodes that are no longer being tracked by the graph.  Creation of point nodes occurs in this case in the ProjectionGraphActionCompleter which uses each measurement of a feature track to triangulate a new point node. (TODO(rsoussan): Update this when this is changed)

## Other
### FeatureTracker
The FeatureTracker maintains feature tracks for image based measurements.  Feature tracks can be queried using feature ids or by returning the N longest feature tracks.  The feature tracker also slides the window for feature tracks to remove old measurements if necessary.

### Sanity Checker
The SanityChecker validates the current localization pose using a reference pose or by checking that the covariances are within defined bounds.
### GraphLocalizerInitializer
The GraphLocalizerInitializer is responsible for loading GraphLocalizer parameters.  It also stores the starting pose for the localizer and estimates initial IMU biases using a set of N consecutive measurements at standstill.

### GraphLocalizerWrapper
The GraphLocalizerWrapper contains the GraphLocalizer, GraphLocalizerInitializer, and SanityChecker.  It passes ROS messages to each of these and also provides interfaces for accessing some GraphLocalizer data, such as messages built using GraphLocalizer values, estimates of the dock and handrail poses wrt the world frame, and more.

### GraphLocalizerNodelet
The GraphLocalizerNodlet subscribes to ROS messages for online use and publishes GraphLocalizer messages and TFs.

# Inputs
* `/loc/of/features`
* `/loc/ml/features`
* `/loc/ar/features`
* `/loc/handrail/features`
* `/hw/imu`

# Outputs
* `graph_loc/graph`
* `graph_loc/state`
