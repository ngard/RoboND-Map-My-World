#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

enum Location {
  LEFT,
  CENTER,
  RIGHT,
  NOT_FOUND
};

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
  // TODO: Request a service and pass the velocities to it to drive the robot
  ball_chaser::DriveToTarget srv;
  srv.request.linear_x = lin_x;
  srv.request.angular_z = ang_z;

  if (!client.call(srv))
    ROS_ERROR("Failed to call service command_robot");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

  int white_pixel = 255;

  enum Location location = NOT_FOUND;
  
  bool found = false;
  for (int row=0; row<img.height && location == NOT_FOUND; ++row) {
    for (int col=0; col<img.width && location == NOT_FOUND; ++col) {
  // TODO: Loop through each pixel in the image and check if there's a bright white one
      bool is_white = true;
      int num_channels = img.step/img.width;
      for (int channel=0; is_white && channel<num_channels; ++channel)
	if (img.data[row*img.step+col*num_channels] != white_pixel)
	  is_white = false;

      if (!is_white)
	continue;
  // Then, identify if this pixel falls in the left, mid, or right side of the image

      if (col < img.width/3)
	location = LEFT;
      else if (col > img.width*2/3)
	location = RIGHT;
      else
	location = CENTER;
    }
  }
  // Depending on the white ball position, call the drive_bot function and pass velocities to it
  // Request a stop when there's no white ball seen by the camera

  switch (location) {
  case LEFT:
    drive_robot(0.1,0.1);
    break;
  case CENTER:
    drive_robot(0.1,0);
    break;
  case RIGHT:
    drive_robot(0.1,-0.1);
    break;
  default:
    drive_robot(0,0);
    break;
  }
  
  return;
}

int main(int argc, char** argv)
{
  // Initialize the process_image node and create a handle to it
  ros::init(argc, argv, "process_image");
  ros::NodeHandle n;

  // Define a client service capable of requesting services from command_robot
  client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

  // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
  ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

  // Handle ROS communication events
  ros::spin();

  return 0;
}
