\page choreographer Choreographer

The choreographer is the core of the mobility system and provides six critical functions in support of mobility:

1. It provides a single action topic called `/mob/motion`, which supports several types of command -- prep, move, execute, stop and idle.
2. It interacts with the low-level control action to send control goals and monitor the current progress for tolerance errors.
3. It provides an interface for planners to register themselves at runtime, offering a range of different ways to plan a segment.
4. It broadcasts the current flight mode, which is used by peer nodes (fam, ctl, ekf) to reconfigure themselves.
5. It broadcasts the current inertia properties, which are used by peer nodes (fam) to reconfigure themselves.
6. It broadcasts the current set of keep in and keep out zones, which are used by the mapper to check for collisions and build a clutter map.

## Basic operation

The choreographer supports a number of different motion requests -- ```executing``` an existing plan, ```moving``` through a sequence of timestamped poses, ```prepping``` to a given flight mode, ```stopping``` as quickly as possible, and ```idling``` the propulsion system. Each request has different data in the payload, and so it makes sense to support multiple action types:

The choreographer provides the callee with a single action-based entry point for control, called ```ff_msgs::MotionAction```. This "motion action" accepts a command -- PREP, MOVE, EXEC, STOP or IDLE -- and carries it out in the background, sending periodic feedback to the callee. When the task completes a result is returned, along with a response code that captures any errors.

The action supports only one goal, which is fully-preemptible. This means that a new goal always preempts the current goal (the previous goal's callee will be notified of preemption). Consequently, you must be careful not to interact with docking action, or any of its dependencies while active.

## Configurable parameters

The choreographer exposes its configuration through ```ff_common::ConfigServer``` class. Thus, the ```rqt_reconfigure``` client can be used to change settings manually, or the ```ff_common::ConfigClient``` can be used to change the settings programatically.

The following configurable parameters are supported:

| Parameter                        | Description |
|:---------------------------------|:----------- |
| ```planner```                    | Which planner to call when a segment needs to be created for a given sequence of poses. |
| ```enable_immediate```           | Whether a segment should be executed immediately, regardless of the first timestamp value. (move, exec) |
| ```enable_validation```          | Whether a segment should be validated by the mapper prior to execution (move, exec). |
| ```enable_bootstrapping```       | Whether the rsbot should plan a path automatically to move from its current position to the first position of a segment. (exec) |
| ```enable_faceforward```         | Whether the planner should generate a segment where the robot always faces the translation direction. (move) |
| ```enable_collision_checking```  | Whether the choreographer should intervene if the mapper detects an obstacle. (move, exec) |
| ```enable_replanning```          | Whether the choreographer should try and automatically replan around obstacles. (move, exec) |
| ```desired_vel```                | Desired upper bound on linear velocity when planning (move) |
| ```desired_accel```              | Desired upper bound on linear acceleration when planning (move) |
| ```desired_omega```              | Desired upper bound on angular velocity when planning (move) |
| ```desired_alpha```              | Desired upper bound on angular acceleration when planning (move) |
| ```desired_rate```               | Desired sampling rate of segment when planning (move) |

Additional parameters are supported, but are not reconfigurable.

A replanning approach is implemented to support obstacle avoidance using the qp-planner. To utilize this functionality set the following parameters: ```planner``` to "qp", ```enable_faceforward``` and ```enable_collision_checking``` to true.

## Under the hood

Internally, the choreographer is encoded as a finite state machine depicted below and implemented using the ff_util/FSM class. This class essentially captures a map of (state, event) -> lambda function relationships. The state of the system is moved forward by setting the initial state and then pushing a sequence of events to the FSM.

\dotfile choreographer_fsm "Choreographer finite state machine"


## Trajectory checks

To ensure that the robot is following the desired trajectory, the choreographer overviews the controller feedback and cancels the movement if the motion value is above the defined values for a certain amout of time. The parameters are tuned according to the flight mode chosen.  A zero value for any of these parameters disables the check.

| Parameter                        | Description |
|:---------------------------------|:----------- |
| ```tolerance_pos_endpoint```     | End of motion position tolerance. Checked when the trajectory finishes and the robot is stopped |
| ```tolerance_pos```              | Position tolerance checked thoughout the entire trajectory |
| ```tolerance_vel```              | Velocity tolerance checked thoughout the entire trajectory |
| ```tolerance_att```              | Attitude tolerance checked thoughout the entire trajectory |
| ```tolerance_omega```            | Omega tolerance checked thoughout the entire trajectory. |
| ```tolerance_time```             | Time for sync check |

The planned trajectories are also checked against the keep-in and keep-out zones.

The keep-in and keep-out zones describe safe and unsafe areas for flight
respectively. Each zone is a cuboid in 3-space, and is fully-defined by the
coordinates of two diagonally opposite vertices. The union of all keep-in zones
minus the union of all keep-out zones describes the free space in which safe
flight can occur. The default zones are provided as JSON formatted files with
suffix `.json` in the `astrobee/gds_config` directory.

An example of a keep-in and keep-out zone JSON file might look like this:

	{
    "timestamp" : "1475516840",
    "zones":
    [
    {
      "name" : "keepout",
      "safe" : false,
      "sequence" : [ [ -1.0, -0.3, -3, -0.6, 1.0, 3.0 ], [ 0.5, -0.3, -3, 1.0, 1.0, 3.0 ] ]
    },
    {
      "name" : "keepin",
      "safe" : true,
      "sequence" : [ [ -1.5, -1.5, 0, 1.5, 1.5, 3.0 ] ]
    }
    ]
	}

Note that the "sequence" field takes an array of 6-vectors, and not just a
single 6-vector. Each element of this array represents a zone, with each vector
denoting the two coordinates that fully-define the cuboid.

At any point while running the robot, the operator can load and parse a JSON file:

    `rosrun executive zones_pub -compression none <file_name>.json`

Or manually through the `SetZones` service call on the ROS namespace `~/mob/set_zones`.
Please refer to the definition of \ref ff_msgs_SetZones for more information on
how to update zones using a ROS service call.

The resulting data structure is serialized into a binary file. If the option `zone_overwrite`
in `mobility/choreographer.config` is activated (default), it will overwrite the current
`<world>.bin` file containing the default set of zones at start-up. 

The \ref mapper node publishes
[visualization_msgs::MarkerArrays](http://docs.ros.org/kinetic/api/visualization_msgs/html/msg/MarkerArray.html)
on the namespace `~/mob/mapper/zones`. When listened to in rviz, these marker
arrays render keep-in and keep-out zones as semi-transparent green and red
cuboids. The two example zones should be rendered as shown below:

![alt text](../images/mobility/zones.png "How the RVIZ user interface draws zones")

