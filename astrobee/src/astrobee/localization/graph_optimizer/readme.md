\page graphoptimizer Graph Optimizer

# Package Overview
The GraphOptimizer class provides a sliding window optimizer using the GTSAM library that slides in time.  Several objects are utilitized to track state parameters (nodes) and error functions (factors) as described below: NodeUpdater, FactorAdder, GraphValues, and GraphActionCompleter. The GraphOptimizer contains a set of NodeUpdaters which handle node insertion and deletion for each state parameter type.  Each of these NodeUpdaters contains a GraphValues object for storing a type of state parameters.  

<p align="center">
<img src="./doc/images/graph_optimizer.png" width="330">
</p>

FactorAdders push factors to the GraphOptimzer where they are buffered until they are able to be added to the graph.  

<p align="center">
<img src="./doc/images/measurement_adding.png" width="520">
</p>

If the factors are too old they are discarded.  Otherwise, if the factors are more recent than the state parameter nodes in the graph they depend on, the factors remain buffered until this is no longer the case. This can occur, for example, if a factor is created using a sensor measurement that arrives at a higher frequency than the sensor data  required to link subsequent state parameter nodes.  For more information on linking state parameter nodes, see the NodeUpdater section below.<br/>
The GraphOptimizer uses the latest and oldest timestamps of nodes stored in its NodeUpdaters to maintiain a sliding window of the correct duration.   



# Background
The GraphOptimizer uses the GTSAM library (_Dellaert, Frank. Factor graphs and GTSAM: A hands-on introduction. Georgia Institute of Technology, 2012._) to optimize a nonlinear weighted least-squares problem where the weights are the measurement uncertainties and the least-squares errors are measurement errors (factors).

# Important Classes
## GraphOptimizer
  The GraphOptimizer maintains sets of NodeUpdaters and GraphActionCompleters.  It adds factors provided by FactorAdders and CumulativeFactorAdders and handles optimization of the graph.  When factors are added to the graph, the NodeUpdaters responsible for creating new nodes are called and the factors are updated with Keys for the newly created nodes.  The GraphOptimizer also maintains the required sliding window duration by calling each of its NodeUpdaters with the new desired window size.  Marginal factors can optionally be added for factors removed during a sliding window operation.  Additional functions are provided that allow for functions to be called after the window is slid and after each Update() call which includes the nonlinear optimization of the graph.  The Update() call is illustrated below:

  <p align="center">
  <img src="./doc/images/update.png" width="200">
  </p>

## FactorAdder
FactorAdders are responsible for outputting factors given a certain measurement type.  The output factors from a FactorAdder should be added to the GraphOptimizer using its BufferFactors() member function.

## CumulativeFactorAdder
CumulativeFactorAdders provide an alternative interface for adding factors to a GraphOptimizer than FactorAdders.  Whereas FactorAdders add factors given a single measurement, CumulativeFactorAdders add factors for a set of measurements.  CumulativeFactorAdder creation is triggered at the beginning of each GraphOptimizer Update() call so that factors are created once the maximum number of measurements have been accumulated. An example of a use for CumulativeFactorAdders is for SmartFactors (_Carlone, Luca, et al. "Eliminating conditionally independent sets in factor graphs: A unifying perspective based on smart factors." 2014 IEEE International Conference on Robotics and Automation (ICRA). IEEE, 2014._).  These create a set of factors using a set of measurements depending on a state parameter and subsequently marginalize out the state parameter from this factor set, creating a single resulting factor containing the information of the factor set in the process.  Since this can only occur once all the measurements are provided, the normal FactorAdder interface that creates factors given a single measurement would not suffice for this type of factor.  

## GraphValues
GraphValues offer an interface on top of the gtsam::Values class that adds several helper functions for accessing values.  Additionally, OldestTimestamp() and LatestTimestamp() functions are provided so the values can be used for sliding window optimization.

## NodeUpdater
NodeUpdaters are responsible for adding and removing state parameters when new factors are added or the graph optimizer window slides. For time varying state parameters, new nodes should be created when a new factor is added whose timestamp is not contained in the current set of state parameter nodes that it depends on.  Typically this occurs when a factor is more recent than previously added factors or when an out of order measurement arrives and a state parameter needs to be added between previously existing state parameters.  The NodeUpdater maintains its own copy of GraphValues for the state parameter being tracked and updates this with newly added state parameters.  Additionally, the NodeUpdater is responsible for adding required relative factors for new state parameter nodes.

<p align="center">
<img src="./doc/images/factor_updating.png" width="500">
</p>

 For a new state parameter node that is more recent than any previous nodes, this simply requires connecting the new node with the previous latest node using some type of factor that provides a relative constraint.  Typically these are in the form of a motion model or using sensor measurements such as an IMU when dealing with a robot body pose as the state parameter.  Additionally, NodeUpdaters are responsible for providing a SlideWindow() member function that handles removing old state parameter nodes.  Each unique state parameter set tracked in a GraphOptimizer should have its own NodeUpdater to handle insertion and deletion of its nodes.  NodeUpdaters must be added to the GraphOptimizer object using its AddNodeUpdater() function.

## GraphActionCompleter
The GraphActionCompleter provides the option of completing a graph action operation after a set of factors from a FactorAdder that were buffered have been added to the GraphOptimizer.  A graph action is any operation modifying the newly added factors or other factors in the graph.  Since factors from a FactorAdder are not immediately added to the graph optimizer (see Overivew section for more details), this enables checks on factors once required state parameter nodes are created. For example, checks can see if a factor measurement is an outlier and remove the factor. GraphActionCompleters should be pushed to the GraphOptimizer object using the AddGraphActionCompleter() function.

# Delayed Insertion of Factor and Key Objects
As factors are first buffered in the GraphOptimizer before being added (see Overview section for more details), not all required information for factors are available when they are created in FactorAdders.  For example, GTSAM requires Keys for the state parameter nodes that factors depend on.  Since these may not have been created yet, the FactorToAdd and KeyInfo classes are used to allow for the delayed insertion of factors.

## FactorToAdd
The FactorToAdd object contains a set of factors and KeyInfo objects that contain the required information for subsequent key and node creation in the GraphOptimizer.
## KeyInfo
KeyInfo objects contain the required information to create a gtsam::Key.  They take a KeyCreatorFunction (typically gtsam::symbol_shorthand::X, where X should be unique for each state parameter type), the NodeUpdaterType so the graph optimizer knows which NodeUpdater to use for node creation, and an optional timestamp.  KeyInfos can be static or dynamic, depending on if they vary in time or not.  
