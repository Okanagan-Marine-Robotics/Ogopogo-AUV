#include <chrono>
#include <memory>
#include <string>
#include <iostream>
#include <fstream>

#include "rclcpp/rclcpp.hpp"
#include "okmr_msgs/msg/movement_command.hpp"
#include "okmr_msgs/srv/find_object.hpp"
#include "okmr_msgs/srv/status.hpp"
#include "okmr_msgs/msg/goal_pose.hpp"
#include "okmr_msgs/msg/status.hpp"
#include "okmr_msgs/msg/classes.hpp"
#include "geometry_msgs/msg/pose.hpp"
#include "geometry_msgs/msg/quaternion.hpp"
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>


using std::placeholders::_1;
using namespace std::chrono_literals;

class NavigatorNode : public rclcpp::Node
{
    public:
        NavigatorNode() 
        : Node("navigator_node"){ 
            command_subscription = this->create_subscription<okmr_msgs::msg::MovementCommand>("/movement_command", 10, std::bind(&NavigatorNode::movement_command_callback, this, _1));
            pose_subscription = this->create_subscription<geometry_msgs::msg::PoseStamped>("/pose", 10, std::bind(&NavigatorNode::pose_callback, this, _1));

            pose_publisher = this->create_publisher<okmr_msgs::msg::GoalPose>("/end_goal_pose", 10);

            status_service=this->create_service<okmr_msgs::srv::Status>("navigator_status", std::bind(&NavigatorNode::status_callback, this, std::placeholders::_1, std::placeholders::_2));

            subNode = rclcpp::Node::make_shared("_navigator_client");
            object_finder_client=subNode->create_client<okmr_msgs::srv::FindObject>("find_object");
            status_client=subNode->create_client<okmr_msgs::srv::Status>("motor_cortex_status");
            //sendObjectRequest();
        }
    private:
        void movement_command_callback(okmr_msgs::msg::MovementCommand msg){
            RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "received movement command");
            switch(msg.command){
                case(okmr_msgs::msg::MovementCommand::GO_TO_GATE):
                    go_to_gate(msg.data0);
                    break;
                case(okmr_msgs::msg::MovementCommand::MOVE_RELATIVE):
                    auto end_pose = calculateDesiredPose(current_pose,msg.data0,msg.data1);
                    okmr_msgs::msg::GoalPose msg;
                    msg.pose=end_pose;
                    msg.copy_orientation=true;
                    msg.header.stamp=this->now();
                    pose_publisher->publish(msg);
                    break;
            }
        }
        void status_callback(const std::shared_ptr<okmr_msgs::srv::Status::Request> request,
                                        std::shared_ptr<okmr_msgs::srv::Status::Response>      response)
        {
            auto status = sendStatusRequest();
            if(status.ongoing==false){
                response->ongoing=false;
                response->success=status.success;
            }
            else
                response->ongoing=true;
        }

        
        void pose_callback(geometry_msgs::msg::PoseStamped msg){
            current_pose=msg.pose;
        }

        geometry_msgs::msg::Pose calculateDesiredPose(const geometry_msgs::msg::Pose& current_pose,
                                              const geometry_msgs::msg::Vector3& relative_translation,
                                              const geometry_msgs::msg::Vector3& relative_rotation)
        {
            // Convert current pose to tf2::Transform
            tf2::Transform tf_current;
            tf2::fromMsg(current_pose, tf_current);

            // Create a tf2::Transform for the relative translation and rotation
            tf2::Transform tf_relative;
            tf_relative.setOrigin(tf2::Vector3(relative_translation.x, relative_translation.y, relative_translation.z));
            tf2::Quaternion relative_quat;
            relative_quat.setRPY(relative_rotation.x*0.01745329251, relative_rotation.y*0.01745329251, relative_rotation.z*0.01745329251);
            tf_relative.setRotation(relative_quat);

            // Apply the relative transformation to the current pose
            tf2::Transform tf_desired = tf_current * tf_relative;

            // Convert the result back to geometry_msgs::msg::Pose
            geometry_msgs::msg::Pose desired_pose;
            tf2::toMsg(tf_desired, desired_pose);

            return desired_pose;
        }

        void go_to_gate(geometry_msgs::msg::Vector3& relative_translation){
            auto location = sendObjectLocationRequest(okmr_msgs::msg::Classes::GATE);
            if(!location.exists)return;
            okmr_msgs::msg::GoalPose msg;
            msg.pose=calculateDesiredPose(location.pose, relative_translation, geometry_msgs::msg::Vector3());
            msg.copy_orientation=false;
            msg.header.stamp=this->now();
            pose_publisher->publish(msg);
        }

        okmr_msgs::srv::FindObject::Response sendObjectLocationRequest(int object_type){
            auto request = std::make_shared<okmr_msgs::srv::FindObject::Request>();
            request->object_type=object_type;

            while (!object_finder_client->wait_for_service(1s)) {
                if (!rclcpp::ok()) {
                    RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Interrupted while waiting for the service. Exiting.");
                }
                RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "service not available, waiting again...");
            }

            auto result = object_finder_client->async_send_request(request);
            // Wait for the result.
            if (rclcpp::spin_until_future_complete(subNode, result) ==
                rclcpp::FutureReturnCode::SUCCESS)
            {
                //RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "got object location");
            } else {
                RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Failed to call service find_object");
            }
            return *result.get();
        }

        okmr_msgs::srv::Status::Response sendStatusRequest(){            
            auto request = std::make_shared<okmr_msgs::srv::Status::Request>();

            while (!status_client->wait_for_service(1s)) {
                if (!rclcpp::ok()) {
                    RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Interrupted while waiting for the service. Exiting.");
                }
                RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "service not available, waiting again...");
            }

            auto result = status_client->async_send_request(request);
            // Wait for the result.
            if (rclcpp::spin_until_future_complete(subNode, result) ==
                rclcpp::FutureReturnCode::SUCCESS)
            {
                //RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "got object location");
            } else {
                RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Failed to call service find_object");
            }
            return *result.get();
        }

        geometry_msgs::msg::Quaternion quaternion_from_rpy(float roll, float pitch, float yaw) {
            // Convert roll, pitch, yaw angles to quaternion
            tf2::Quaternion quat;
            quat.setRPY(roll, pitch, yaw);

            // Convert tf2 Quaternion to ROS 2 Quaternion message
            geometry_msgs::msg::Quaternion quat_msg;
            tf2::convert(quat, quat_msg);

            return quat_msg;
        }

        rclcpp::Subscription<okmr_msgs::msg::MovementCommand>::SharedPtr command_subscription;
        rclcpp::Subscription<okmr_msgs::msg::Status>::SharedPtr status_subscription;
        rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr pose_subscription;
        rclcpp::Publisher<okmr_msgs::msg::Status>::SharedPtr status_publisher;
        rclcpp::Publisher<okmr_msgs::msg::GoalPose>::SharedPtr pose_publisher;
        rclcpp::Service<okmr_msgs::srv::Status>::SharedPtr status_service;
        geometry_msgs::msg::Pose current_pose;
        std::shared_ptr<rclcpp::Node> subNode;
        rclcpp::Client<okmr_msgs::srv::FindObject>::SharedPtr object_finder_client;
        rclcpp::Client<okmr_msgs::srv::Status>::SharedPtr status_client;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<NavigatorNode>());
  rclcpp::shutdown();
  return 0;
}
