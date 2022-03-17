#include <test_bed_core/trajopt_utils.h>
#include <trajopt/problem_description.hpp>

#include <ros/ros.h>

#include <trajectory_msgs/JointTrajectory.h>

trajectory_msgs::JointTrajectory trajArrayToJointTrajectoryMsg(std::vector<std::string> joint_names,
                                                               tesseract::TrajArray traj_array,
                                                               bool use_time,
                                                               ros::Duration time_increment)
{
  // Create the joint trajectory
  trajectory_msgs::JointTrajectory traj_msg;
  traj_msg.header.stamp = ros::Time::now();
  traj_msg.header.frame_id = "0";
  traj_msg.joint_names = joint_names;

  tesseract::TrajArray pos_mat;
  tesseract::TrajArray time_mat;
  if (use_time)
  {
    // Seperate out the time data in the last column from the joint position data
    pos_mat = traj_array.leftCols(traj_array.cols());
    time_mat = traj_array.rightCols(1);
  }
  else
  {
    pos_mat = traj_array;
  }

  ros::Duration time_from_start(0);
  for (int ind = 0; ind < traj_array.rows(); ind++)
  {
    // Create trajectory point
    trajectory_msgs::JointTrajectoryPoint traj_point;

    // Set the position for this time step
    auto mat = pos_mat.row(ind);
    std::vector<double> vec(mat.data(), mat.data() + mat.rows() * mat.cols());
    traj_point.positions = vec;

    // Add the current dt to the time_from_start
    if (use_time)
    {
      time_from_start += ros::Duration(time_mat(ind, time_mat.cols() - 1));
    }
    else
    {
      time_from_start += time_increment;
    }
    traj_point.time_from_start = time_from_start;

    traj_msg.points.push_back(traj_point);
  }
  return traj_msg;
}
