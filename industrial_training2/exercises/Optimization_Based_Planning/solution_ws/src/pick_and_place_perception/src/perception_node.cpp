#include <ros/ros.h>
#include <tf/transform_datatypes.h>
#include <tf/transform_listener.h>
#include <tf/transform_broadcaster.h>
#include <sensor_msgs/PointCloud2.h>  //hydro

// PCL specific includes
#include <pcl_conversions/pcl_conversions.h>  //hydro
#include "pcl_ros/transforms.h"

#include <pcl/filters/voxel_grid.h>
#include <pcl/filters/passthrough.h>
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/model_types.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/segmentation/extract_clusters.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/common/common.h>

#include <visualization_msgs/Marker.h>
#include <pcl/filters/crop_box.h>
#include <pcl/filters/statistical_outlier_removal.h>

// polygonal segmentation
#include <pcl/segmentation/extract_polygonal_prism_data.h>
#include <tf_conversions/tf_eigen.h>
#include <pcl/surface/convex_hull.h>

// Needed to return a Pose message
#include <geometry_msgs/Pose.h>
#include <geometry_msgs/Point.h>
// Needed for ROS Service
#include <pick_and_place_perception/GetTargetPose.h>

class PerceptionPipeline
{
public:
  PerceptionPipeline(ros::NodeHandle& nh)
  {
    nh_ = nh;
    ROS_INFO("Perception service running");

    nh.getParam("processing_node/cloud_topic1", cloud_topic1);
    nh.getParam("processing_node/cloud_topic2", cloud_topic2);
    nh.getParam("processing_node/cloud_topic3", cloud_topic3);
    nh.getParam("processing_node/world_frame", world_frame);
    nh.getParam("processing_node/camera_frame", camera_frame);
    nh.getParam("processing_node/voxel_leaf_size", voxel_leaf_size);
    nh.getParam("processing_node/x_filter_min", x_filter_min);
    nh.getParam("processing_node/x_filter_max", x_filter_max);
    nh.getParam("processing_node/y_filter_min", y_filter_min);
    nh.getParam("processing_node/y_filter_max", y_filter_max);
    nh.getParam("processing_node/z_filter_min", z_filter_min);
    nh.getParam("processing_node/z_filter_max", z_filter_max);
    nh.getParamCached("processing_node/plane_max_iterations", plane_max_iter);
    nh.getParamCached("processing_node/plane_distance_threshold", plane_dist_thresh);
    nh.getParam("processing_node/cluster_tolerance", cluster_tol);
    nh.getParam("processing_node/cluster_min_size", cluster_min_size);
    nh.getParam("processing_node/cluster_max_size", cluster_max_size);
    nh.getParam("cloud_debug", debug_);

    // TODO (for SwRI. Not for the student) Add check parameters
    //    if (!nh.getParam("stuff", data_)) throw std::runtime_error("Must provide parameter 'stuff'");

    // Create ROS interfaces
    server_ = nh.advertiseService("find_pick", &PerceptionPipeline::findPickPose, this);
    if (debug_)
    {
      ROS_INFO("Perception Debug Enabled: Intermediate clouds are being published");
      cropped_pub_ = nh.advertise<sensor_msgs::PointCloud2>("cropped_cloud", 1, true);
      object_pub_ = nh.advertise<sensor_msgs::PointCloud2>("object_cluster", 1, true);
      cluster_pub_ = nh.advertise<sensor_msgs::PointCloud2>("primary_cluster", 1, true);
      pick_surface_pub_ = nh.advertise<sensor_msgs::PointCloud2>("pick_surface", 1, true);
    }
  }

  bool findPickPose(pick_and_place_perception::GetTargetPoseRequest& req,
                    pick_and_place_perception::GetTargetPoseResponse& res)
  {
    res.succeeded = true;
    ROS_INFO("Perception service running");

    // LISTEN FOR POINTCLOUD
    std::string topic1 = nh_.resolveName(cloud_topic1);
    std::string topic2 = nh_.resolveName(cloud_topic2);
    std::string topic3 = nh_.resolveName(cloud_topic3);
    ROS_INFO_STREAM("Cloud service called; waiting for a PointCloud2 on topic " << topic1);
    sensor_msgs::PointCloud2::ConstPtr recent_cloud1 =
        ros::topic::waitForMessage<sensor_msgs::PointCloud2>(topic1, nh_);
    ROS_INFO_STREAM("Waiting for a PointCloud2 on topic " << topic2);
    sensor_msgs::PointCloud2::ConstPtr recent_cloud2 =
        ros::topic::waitForMessage<sensor_msgs::PointCloud2>(topic2, nh_, ros::Duration(10));
    ROS_INFO_STREAM("Waiting for a PointCloud2 on topic " << topic3);
    sensor_msgs::PointCloud2::ConstPtr recent_cloud3 =
        ros::topic::waitForMessage<sensor_msgs::PointCloud2>(topic3, nh_, ros::Duration(2));

    // TRANSFORM POINTCLOUDS FROM CAMERA FRAME TO WORLD FRAME
    tf::TransformListener listener;
    tf::StampedTransform stransform1, stransform2, stransform3;
    try
    {
      listener.waitForTransform(world_frame, recent_cloud1->header.frame_id, ros::Time::now(), ros::Duration(6.0));
      listener.lookupTransform(world_frame, recent_cloud1->header.frame_id, ros::Time(0), stransform1);
      listener.waitForTransform(world_frame, recent_cloud2->header.frame_id, ros::Time::now(), ros::Duration(6.0));
      listener.lookupTransform(world_frame, recent_cloud2->header.frame_id, ros::Time(0), stransform2);
      listener.waitForTransform(world_frame, recent_cloud3->header.frame_id, ros::Time::now(), ros::Duration(6.0));
      listener.lookupTransform(world_frame, recent_cloud3->header.frame_id, ros::Time(0), stransform3);
    }
    catch (tf::TransformException ex)
    {
      ROS_ERROR("%s", ex.what());
    }
    sensor_msgs::PointCloud2 transformed_cloud1, transformed_cloud2, transformed_cloud3;
    pcl_ros::transformPointCloud(world_frame, stransform1, *recent_cloud1, transformed_cloud1);
    pcl_ros::transformPointCloud(world_frame, stransform2, *recent_cloud2, transformed_cloud2);
    pcl_ros::transformPointCloud(world_frame, stransform3, *recent_cloud3, transformed_cloud3);

    // COMBINE POINT CLOUDS
    sensor_msgs::PointCloud2 temp, transformed_cloud;
    pcl::concatenatePointCloud(transformed_cloud1, transformed_cloud2, temp);
    pcl::concatenatePointCloud(temp, transformed_cloud3, transformed_cloud);

    // CONVERT POINTCLOUD ROS->PCL
    pcl::PointCloud<pcl::PointXYZ> cloud1, cloud2, cloud3, cloud;
    pcl::fromROSMsg(transformed_cloud, cloud);
    //    pcl::fromROSMsg(transformed_cloud1, cloud);

    // MAKE TIMERS FOR PROCESS (OPTIONAL)
    ros::Time start_init = ros::Time::now();

    /* ========================================
     * Fill Code: VOXEL GRID
     * ========================================*/
    // input cloud must be a pointer, so we make a new cloud_ptr from the cloud object
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_ptr(new pcl::PointCloud<pcl::PointXYZ>(cloud));
    // output cloud - set up as pointer to ease transition into further processing
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_voxel_filtered(new pcl::PointCloud<pcl::PointXYZ>(cloud));
    // create an instance of the pcl VoxelGrid
    pcl::VoxelGrid<pcl::PointXYZ> voxel_filter;
    voxel_filter.setInputCloud(cloud_ptr);
    voxel_filter.setLeafSize(float(voxel_leaf_size), float(voxel_leaf_size), float(voxel_leaf_size));
    voxel_filter.filter(*cloud_voxel_filtered);

    ROS_INFO_STREAM("Original cloud  had " << cloud_voxel_filtered->size() << " points");
    ROS_INFO_STREAM("Downsampled cloud  with " << cloud_voxel_filtered->size() << " points");

    /* ========================================
     * Fill Code: PASSTHROUGH FILTER(S)
     * ======================*/
    // step 1- filter in x
    pcl::PointCloud<pcl::PointXYZ> xf_cloud = cloud;
    pcl::PointCloud<pcl::PointXYZ> yf_cloud = cloud;
    pcl::PointCloud<pcl::PointXYZ> zf_cloud = cloud;
    pcl::PassThrough<pcl::PointXYZ> pass_x;
    pass_x.setInputCloud(cloud_voxel_filtered);
    pass_x.setFilterFieldName("x");
    pass_x.setFilterLimits(x_filter_min, x_filter_max);
    pass_x.filter(xf_cloud);

    // pass to filter in y
    pcl::PointCloud<pcl::PointXYZ>::Ptr xf_cloud_ptr(new pcl::PointCloud<pcl::PointXYZ>(xf_cloud));
    pcl::PassThrough<pcl::PointXYZ> pass_y;
    pass_y.setInputCloud(xf_cloud_ptr);
    pass_y.setFilterFieldName("y");
    pass_y.setFilterLimits(y_filter_min, y_filter_max);
    pass_y.filter(yf_cloud);

    // pass to filter in z
    pcl::PointCloud<pcl::PointXYZ>::Ptr yf_cloud_ptr(new pcl::PointCloud<pcl::PointXYZ>(yf_cloud));
    pcl::PassThrough<pcl::PointXYZ> pass_z;
    pass_z.setInputCloud(yf_cloud_ptr);
    pass_z.setFilterFieldName("z");
    pass_z.setFilterLimits(z_filter_min, z_filter_max);
    pass_z.filter(zf_cloud);

    /* ========================================
     * Fill Code: CROPBOX (OPTIONAL)
     * Instead of three passthrough filters, the cropbox filter can be used
     * The user should choose one or the other method
     * ========================================*/
    pcl::PointCloud<pcl::PointXYZ> xyz_filtered_cloud;
    pcl::CropBox<pcl::PointXYZ> crop;
    crop.setInputCloud(cloud_voxel_filtered);
    Eigen::Vector4f min_point = Eigen::Vector4f(x_filter_min, y_filter_min, z_filter_min, 0);
    Eigen::Vector4f max_point = Eigen::Vector4f(x_filter_max, y_filter_max, z_filter_max, 0);
    crop.setMin(min_point);
    crop.setMax(max_point);
    crop.filter(xyz_filtered_cloud);

    ROS_INFO_STREAM("Cropped point cloud: " << zf_cloud.points.size() << " data points.");
    // Publish cropped cloud
    if (debug_)
    {
      sensor_msgs::PointCloud2::Ptr cropped(new sensor_msgs::PointCloud2);
      pcl::toROSMsg(zf_cloud, *cropped);
      //    pcl::toROSMsg(xyz_filtered_cloud, *cropped);
      cropped->header.frame_id = world_frame;
      cropped->header.stamp = ros::Time::now();
      cropped_pub_.publish(*cropped);
    }

    /* ========================================
     * Fill Code: PLANE SEGEMENTATION - REMOVE THE WORKTABLE
     * ========================================*/
    pcl::PointCloud<pcl::PointXYZ>::Ptr cropped_cloud(
        new pcl::PointCloud<pcl::PointXYZ>(zf_cloud));  // this passes in either passthrough or crop filtered cloud.
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_f(new pcl::PointCloud<pcl::PointXYZ>);
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_filtered(new pcl::PointCloud<pcl::PointXYZ>);
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_plane(new pcl::PointCloud<pcl::PointXYZ>());
    // Create the segmentation object for the planar model and set all the parameters
    pcl::SACSegmentation<pcl::PointXYZ> seg;
    pcl::PointIndices::Ptr inliers(new pcl::PointIndices);
    pcl::ModelCoefficients::Ptr coefficients(new pcl::ModelCoefficients);
    seg.setOptimizeCoefficients(true);
    seg.setModelType(pcl::SACMODEL_PLANE);
    seg.setMethodType(pcl::SAC_RANSAC);
    seg.setMaxIterations(plane_max_iter);
    seg.setDistanceThreshold(plane_dist_thresh);
    // Segment the largest planar component from the cropped cloud
    seg.setInputCloud(cropped_cloud);
    seg.segment(*inliers, *coefficients);
    if (inliers->indices.size() > 0)
    {
      // Extract the planar inliers from the input cloud
      pcl::ExtractIndices<pcl::PointXYZ> extract;
      extract.setInputCloud(cropped_cloud);
      extract.setIndices(inliers);
      extract.setNegative(false);

      // Get the points associated with the planar surface
      extract.filter(*cloud_plane);
      ROS_INFO_STREAM("PointCloud representing the planar component: " << cloud_plane->points.size()
                                                                       << " data points.");

      // Remove the planar inliers, extract the rest
      extract.setNegative(true);
      extract.filter(*cloud_f);
    }
    else
    {
      ROS_WARN_STREAM("Could not estimate a planar model for the given dataset. Proceeding with full point cloud.");
      *cloud_f = *cropped_cloud;
      res.succeeded = false;
    }
    /* ========================================
     * Fill Code: EUCLIDEAN CLUSTER EXTRACTION
     * ========================================*/
    // Creating the KdTree object for the search method of the extraction
    pcl::search::KdTree<pcl::PointXYZ>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZ>);
    *cloud_filtered = *cloud_f;
    tree->setInputCloud(cloud_filtered);

    std::vector<pcl::PointIndices> cluster_indices;
    pcl::EuclideanClusterExtraction<pcl::PointXYZ> ec;
    ec.setClusterTolerance(cluster_tol);  // 2cm
    ec.setMinClusterSize(cluster_min_size);
    ec.setMaxClusterSize(cluster_max_size);
    ec.setSearchMethod(tree);
    ec.setInputCloud(cloud_filtered);
    ec.extract(cluster_indices);

    std::vector<sensor_msgs::PointCloud2::Ptr> pc2_clusters;
    std::vector<pcl::PointCloud<pcl::PointXYZ>::Ptr> clusters;
    for (std::vector<pcl::PointIndices>::const_iterator it = cluster_indices.begin(); it != cluster_indices.end(); ++it)
    {
      pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_cluster(new pcl::PointCloud<pcl::PointXYZ>);
      for (std::vector<int>::const_iterator pit = it->indices.begin(); pit != it->indices.end(); pit++)
        cloud_cluster->points.push_back(cloud_filtered->points[*pit]);
      cloud_cluster->width = cloud_cluster->points.size();
      cloud_cluster->height = 1;
      cloud_cluster->is_dense = true;
      std::cout << "Cluster has " << cloud_cluster->points.size() << " points.\n";
      clusters.push_back(cloud_cluster);
      sensor_msgs::PointCloud2::Ptr tempROSMsg(new sensor_msgs::PointCloud2);
      pcl::toROSMsg(*cloud_cluster, *tempROSMsg);
      pc2_clusters.push_back(tempROSMsg);
    }
    if (pc2_clusters.size() > 0)
    {
      pc2_clusters.at(0)->header.frame_id = world_frame;
      pc2_clusters.at(0)->header.stamp = ros::Time::now();
      if (debug_)
      {
        cluster_pub_.publish(pc2_clusters.at(0));
      }
    }
    else
    {
      ROS_WARN_STREAM("Clustering failed. Proceeding with full point cloud.");
      clusters.push_back(cloud_filtered);
      res.succeeded = false;
    }
    /* ========================================
     * Fill Code: STATISTICAL OUTLIER REMOVAL (OPTIONAL)
     * ========================================*/
    pcl::PointCloud<pcl::PointXYZ>::Ptr cluster_cloud_ptr = clusters.at(0);
    pcl::PointCloud<pcl::PointXYZ>::Ptr sor_cloud_filtered(new pcl::PointCloud<pcl::PointXYZ>);
    pcl::StatisticalOutlierRemoval<pcl::PointXYZ> sor;
    sor.setInputCloud(cluster_cloud_ptr);
    sor.setMeanK(50);
    sor.setStddevMulThresh(1.0);
    sor.filter(*sor_cloud_filtered);

    // Publish object as point cloud
    if (debug_)
    {
      sensor_msgs::PointCloud2::Ptr pc2_cloud(new sensor_msgs::PointCloud2);
      pcl::toROSMsg(*sor_cloud_filtered, *pc2_cloud);
      pc2_cloud->header.frame_id = world_frame;
      pc2_cloud->header.stamp = ros::Time::now();

      object_pub_.publish(*pc2_cloud);
    }

    /* ========================================
     * Fill Code: PLANE SEGEMENTATION OF PICK SURFACE - FIND TOP OF BOX
     * ========================================*/
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_planeTop(new pcl::PointCloud<pcl::PointXYZ>());
    // Create the segmentation object for the planar model and set all the parameters
    pcl::SACSegmentation<pcl::PointXYZ> segTop;
    pcl::PointIndices::Ptr inliers2(new pcl::PointIndices);
    pcl::ModelCoefficients::Ptr coefficients2(new pcl::ModelCoefficients);
    Eigen::Vector3f axis = Eigen::Vector3f(0.0, 0.0, 1.0);

    segTop.setOptimizeCoefficients(true);
    segTop.setModelType(pcl::SACMODEL_PERPENDICULAR_PLANE);
    segTop.setMethodType(pcl::SAC_RANSAC);
    segTop.setAxis(axis);
    segTop.setEpsAngle(30.0f * (M_PI / 180.0f));
    segTop.setMaxIterations(plane_max_iter);
    segTop.setDistanceThreshold(plane_dist_thresh);
    // Segment the largest planar component perpendicular to the world z axis from the cropped cloud
    segTop.setInputCloud(cluster_cloud_ptr);
    segTop.segment(*inliers2, *coefficients2);
    if (inliers2->indices.size() > 0)
    {
      // Extract the planar inliers from the input cloud
      pcl::ExtractIndices<pcl::PointXYZ> extractTop;
      extractTop.setInputCloud(cluster_cloud_ptr);
      extractTop.setIndices(inliers2);
      extractTop.setNegative(false);

      // Get the points associated with the planar surface
      extractTop.filter(*cloud_planeTop);
      ROS_INFO_STREAM("PointCloud representing the top planar component: " << cloud_planeTop->points.size()
                                                                           << " data points.");
    }
    else
    {
      ROS_WARN_STREAM("Could not estimate a planar model for the given dataset. Proceeding with full point cloud.");
      *cloud_planeTop = *cluster_cloud_ptr;
      res.succeeded = false;
    }

    // OPTIONAL TIMERS
    ros::Time finish_process = ros::Time::now();
    ros::Duration total_process = finish_process - start_init;
    ROS_INFO_STREAM("Point Cloud processing took " << total_process << " s");

    /* ========================================
     * CONVERT POINTCLOUD PCL->ROS
     * PUBLISH CLOUD
     * Fill Code: UPDATE AS NECESSARY
     * ========================================*/
    sensor_msgs::PointCloud2::Ptr pc2_cloud_top(new sensor_msgs::PointCloud2);
    pcl::toROSMsg(*cloud_planeTop, *pc2_cloud_top);
    pc2_cloud_top->header.frame_id = world_frame;
    pc2_cloud_top->header.stamp = ros::Time::now();
    if (debug_)
    {
      pick_surface_pub_.publish(*pc2_cloud_top);
    }

    /* ========================================
     * Fill Code: BROADCAST PART TRANSFORM
     * ========================================*/
    Eigen::Vector4f origin;
    // Compute centroid of the top of the pick plane
    pcl::compute3DCentroid(*cloud_planeTop, origin);
    geometry_msgs::Pose part_pose;

    part_pose.position.x = origin[0];
    part_pose.position.y = origin[1];
    part_pose.position.z = origin[2];
    part_pose.orientation.x = 0;
    part_pose.orientation.y = 0;
    part_pose.orientation.z = 0;
    part_pose.orientation.w = 1;

    /* ========================================
     * FIND BOUNDING BOX
     * ========================================*/
    Eigen::Vector4f min_pt, max_pt;
    pcl::getMinMax3D(*cloud_planeTop, min_pt, max_pt);
    res.min_pt.x = min_pt[0];
    res.min_pt.y = min_pt[1];
    res.min_pt.z = min_pt[2];
    res.max_pt.x = max_pt[0];
    res.max_pt.y = max_pt[1];
    res.max_pt.z = max_pt[2];

    // Store the returned values of the service as defined in the .srv file
    res.target_pose = part_pose;

    ROS_INFO("Perception service returning");

    // Publish the pick point as a TF for visualization
    if (debug_)
    {
      tf::Transform part_transform;
      part_transform.setOrigin(tf::Vector3(origin[0], origin[1], origin[2]));
      tf::Quaternion q;
      q.setRPY(0, 0, 0);
      part_transform.setRotation(q);
      br_.sendTransform(tf::StampedTransform(part_transform, ros::Time::now(), world_frame, "part"));
    }

    return true;
  }

private:
  ros::ServiceServer server_;
  ros::Publisher cropped_pub_, object_pub_, cluster_pub_, pick_surface_pub_;
  ros::NodeHandle nh_;
  tf::TransformBroadcaster br_;

  // Configuration data
  std::string cloud_topic1, cloud_topic2, cloud_topic3, world_frame, camera_frame;
  double voxel_leaf_size;
  double x_filter_min, x_filter_max, y_filter_min, y_filter_max, z_filter_min, z_filter_max;
  double plane_max_iter, plane_dist_thresh;
  double cluster_tol;
  int cluster_min_size, cluster_max_size;
  bool debug_ = false;
};

int main(int argc, char* argv[])
{
  ros::init(argc, argv, "processing_node");
  ros::NodeHandle nh;

  PerceptionPipeline pipeline(nh);
  ROS_INFO("Perception server ready to find pick targets");
  ros::spin();

  return 0;
}
