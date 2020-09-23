#include "mono_slam/system.h"

namespace mono_slam {

System::System(const string& config_file) : config_file_(config_file) {}

bool System::Init() {
  LOG(INFO) << "System is initializing ...";
  {
    //   // Read settings from configuration file.
    //   if (!Config::SetConfigFile(config_file_)) return false;

    //   // Initialize dataset.
    // dataset_ = make_shared<Dataset>(Config::Get<string>("dataset_path"),
    // Config::Get<string>("image_file_name_fmt")); if (!dataset_->Init())
    // return false;

    //   // Initialize camera.
    //   const double& fx = Config::Get<double>("fx");
    //   const double& fy = Config::Get<double>("fy");
    //   const double& cx = Config::Get<double>("cx");
    //   const double& cy = Config::Get<double>("cy");
    //   const double& k1 = Config::Get<double>("k1");
    //   const double& k2 = Config::Get<double>("k2");
    //   const double& p1 = Config::Get<double>("p1");
    //   const double& p2 = Config::Get<double>("p2");
    //   const Vec4 dist_coeffs{k1, k2, p1, p2};
    //   cam_ = make_shared<Camera>(fx, fy, cx, cy, dist_coeffs);

    //   // Initialize initializer.
    //   initializer_ =
    //       make_shared<Initializer>(Config::Get<int>("min_num_features_init"),
    //                                Config::Get<int>("min_num_matched_features"),
    //                                Config::Get<int>("min_num_inlier_matches"));

    //   // Load vocabulary.
    //   voc_ = make_shared<Vocabulary>(Config::Get<string>("vocabulary_file"));

    //   // Load timestamps.
    //   const string& timestamp_file = Config::Get<string>("timestamp_file");
    //   arma::mat timestamps_mat;
    //   if (timestamp_file.empty())
    //     LOG(WARNING) << "No timestamp file.";
    //   else
    //     timestamps_mat.load(Config::Get<string>("timestamp_file"),
    //                         arma::file_type::auto_detect, true);
    //   timestamps_ = arma::conv_to<vector<double>>::from(timestamps_mat);
  }

  // Prepare and link components.
  tracker_ = make_shared<Tracking>();
  local_mapper_ = make_shared<LocalMapping>();
  map_ = make_shared<Map>();
  viewer_ = make_shared<Viewer>();

  System::Ptr this_system(this);

  tracker_->SetSystem(this_system);
  tracker_->SetLocalMapper(local_mapper_);
  tracker_->SetMap(map_);
  tracker_->SetViewer(viewer_);
  tracker_->SetInitializer(initializer_);
  tracker_->SetVocabulary(voc_);

  local_mapper_->SetSystem(this_system);
  local_mapper_->SetTracker(tracker_);
  local_mapper_->SetMap(map_);
  local_mapper_->SetVocabulary(voc_);

  viewer_->SetSystem(this_system);
  viewer_->SetMap(map_);

  return true;
}

void System::Run() {
  LOG(INFO) << "System is running ...";
  if (timestamp_.empty())
    for (;;) tracker_->AddImage(dataset_->NextImage());
  else {
    const int num_images = timestamp_.size();
    // Simply discard the last image.
    for (int i = 0; i < num_images - 1; ++i) {
      const steady_clock::time_point t1 = steady_clock::now();

      // Track one image.
      tracker_->AddImage(dataset_->NextImage());

      const steady_clock::time_point t2 = steady_clock::now();
      const double consumed_time =
          duration_cast<duration<double>>(t2 - t1).count();

      // (Only) pause the tracking thread to align the time.
      const double delta_t = timestamp_[i + 1] - timestamp_[i];
      if (consumed_time < delta_t)
        std::this_thread::sleep_for(delta_t - consumed_time);
    }
  }
  LOG(INFO) << "Exit system.";
}

void System::Reset() {
  tracker_->Reset();
  local_mapper_->Reset();
  map_->Clear();
  viewer_->Reset();
}

}  // namespace mono_slam
