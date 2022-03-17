#include <rclcpp/rclcpp.hpp>
#include <fake_ar_publisher/msg/ar_marker.hpp>
#include <myworkcell_core/srv/localize_part.hpp>
#include <tf2_ros/buffer.h>				//3.2
#include <tf2_ros/transform_listener.h>			//3.2
#include <geometry_msgs/msg/pose_stamped.hpp>		//3.2
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>	//3.2

class Localizer : public rclcpp::Node
{
public:
//  Localizer() : Node("vision_node"), last_msg_{nullptr} //3.2
  Localizer() : Node("vision_node"), buffer_(this->get_clock()), listener_(buffer_) //3.2.4
  {
    ar_sub_ = this->create_subscription<fake_ar_publisher::msg::ARMarker>(
        "ar_pose_marker",
        rclcpp::QoS(1),
        std::bind(&Localizer::visionCallback, this, std::placeholders::_1));
  server_ = this->create_service<myworkcell_core::srv::LocalizePart>(
  "localize_part",
  std::bind(&Localizer::localizePart, this, std::placeholders::_1, std::placeholders::_2));
  }

  void visionCallback(fake_ar_publisher::msg::ARMarker::SharedPtr msg)
  {
    last_msg_ = msg;
//    RCLCPP_INFO(get_logger(), "Received pose: x=%f, y=%f, z=%f",
//        msg->pose.pose.position.x,
//        msg->pose.pose.position.y,
//        msg->pose.pose.position.z);
  }

  void localizePart(myworkcell_core::srv::LocalizePart::Request::SharedPtr req,
                    myworkcell_core::srv::LocalizePart::Response::SharedPtr res)
  {
    // Read last message
    fake_ar_publisher::msg::ARMarker::SharedPtr p = last_msg_;

    if (!p)
    {
      RCLCPP_ERROR(this->get_logger(), "no data");
      res->success = false;
      return;
    }

//  res->pose = p->pose.pose; //3.2.5.1
    geometry_msgs::msg::PoseStamped target_pose_from_cam; //3.2.5.2
    target_pose_from_cam.header = p->header; //3.2.5.2
    target_pose_from_cam.pose = p->pose.pose; //3.2.5.2

    geometry_msgs::msg::PoseStamped target_pose_from_req = buffer_.transform(
    target_pose_from_cam, req->base_frame); //3.2.5.3
    res->pose = target_pose_from_req.pose; //3.2.5.3
    res->success = true;
  }

  rclcpp::Subscription<fake_ar_publisher::msg::ARMarker>::SharedPtr ar_sub_;
  fake_ar_publisher::msg::ARMarker::SharedPtr last_msg_;
  rclcpp::Service<myworkcell_core::srv::LocalizePart>::SharedPtr server_;

  tf2_ros::Buffer buffer_;			//3.2.2
  tf2_ros::TransformListener listener_;		//3.2.2

};


int main(int argc, char* argv[])
{
  // This must be called before anything else ROS-related
  rclcpp::init(argc, argv);

  // Create a ROS node
//  auto node = std::make_shared<rclcpp::Node>("vision_node");

//  RCLCPP_INFO(node->get_logger(), "Hello, World!");

    // The Localizer class provides this node's ROS interfaces
  auto node = std::make_shared<Localizer>();

  RCLCPP_INFO(node->get_logger(), "Vision node starting");

  // Don't exit the program.
  rclcpp::spin(node);
}

