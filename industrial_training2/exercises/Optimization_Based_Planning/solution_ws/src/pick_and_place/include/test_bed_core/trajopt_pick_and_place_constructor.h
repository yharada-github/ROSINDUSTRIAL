#pragma once
#include <tesseract_planning/trajopt/trajopt_planner.h>
#include <tesseract_ros/kdl/kdl_env.h>
#include <trajopt/problem_description.hpp>


/**
 * @brief The TrajoptPickAndPlaceConstructor class
 */
class TrajoptPickAndPlaceConstructor
{
private:
  /**< @brief Problem Construction Info */
  std::string manipulator_, ee_link_, pick_object_;
  Eigen::Isometry3d tcp_;             /**< @brief Tool center point offset */
  tesseract::BasicEnvConstPtr env_; /**< @brief Environment description */
  tesseract::BasicKinConstPtr kin_; /**< @brief Kinematics description */

public:
  // Needed because an Eigen::Isometry3d is a class member
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  TrajoptPickAndPlaceConstructor(tesseract::BasicEnvConstPtr env,
                                 std::string manipulator,
                                 std::string ee_link,
                                 std::string pick_object,
                                 Eigen::Isometry3d tcp = Eigen::Isometry3d::Identity());

  /**
   * @brief addTotalTimeCost - Adds cost to the overall total time to elapsed in the trajectory
   * @param pci - The trajopt problem construction info to which the cost is added
   * @param coeff - Used to scale this cost relative to other costs
   */
  void addTotalTimeCost(trajopt::ProblemConstructionInfo& pci, double coeff);

  /**
   * @brief Adds a single waypoint at the desired pose
   * @param pci - The trajopt problem construction info to which the cost is added
   * @param pose - The target pose
   * @param time_step - Time step at which the cost applies
   */
  void addSingleWaypoint(trajopt::ProblemConstructionInfo& pci,
                                                       Eigen::Isometry3d pose,
                                                       int time_step);

  /**
   * @brief Adds a linear move to the problem construction info
   * @param pci - The trajopt problem construction info to which the move is added
   * @param start_pose - The starting pose of the linear move
   * @param end_pose - The end pose of the linear move
   * @param num_steps -  Number of steps for the move.
   * @param first_time_step - Time step at which the move is added
   */
  void addLinearMotion(trajopt::ProblemConstructionInfo& pci,
                       Eigen::Isometry3d start_pose,
                       Eigen::Isometry3d end_pose,
                       int num_steps,
                       int first_time_step);

  /**
   * @brief Generates a trajopt problem for a "pick" move
   * Consists of 2 phases - a free space move to approach_pose and a linear move to final_pose
   * @param approach_pose - Pose moved to prior to picking
   * @param final_pose - Pose moved to for the pick operation
   * @param steps_per_phase - Number of steps per phase. Total move is steps_per_phase*2
   * @return
   */
  trajopt::TrajOptProbPtr generatePickProblem(Eigen::Isometry3d& approach_pose,
                                              Eigen::Isometry3d& final_pose,
                                              int steps_per_phase);

  /**
   * @brief Generates a trajopt problem for a "place" move
   * Consists of 3 phases - linearly lifting the object to retreat_pose, a free space move to approach_pose, and then
   * linearly placing the object to final_pose
   * @param retreat_pose - Pose to which the object is moved from it's starting pose
   * @param approach_pose  - Pose to which the object is moved prior to placing
   * @param final_pose - Final "placed" pose
   * @param steps_per_phase - Number of steps per move phase. Total move length is steps_per_phase*3
   * @return
   */
  trajopt::TrajOptProbPtr generatePlaceProblem(Eigen::Isometry3d& retreat_pose,
                                               Eigen::Isometry3d& approach_pose,
                                               Eigen::Isometry3d& final_pose,
                                               int steps_per_phase);

  /**
   * @brief Uses trajopt to calculate inverse kinematics
   * Calculates the IK solution that is closest to the current joint position. This is useful for creating a
   * STRAIGHT_LINE constraint
   * @param end_pose - The pose of the point for which the IK is desired.
   * @return
   */
  Eigen::VectorXd numericalIK(Eigen::Isometry3d& end_pose);
};
