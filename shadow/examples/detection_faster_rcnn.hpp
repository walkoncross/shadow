#ifndef SHADOW_EXAMPLES_DETECTION_FASTER_RCNN_HPP
#define SHADOW_EXAMPLES_DETECTION_FASTER_RCNN_HPP

#include "method.hpp"

namespace Shadow {

class DetectionFasterRCNN final : public Method {
 public:
  DetectionFasterRCNN() = default;
  ~DetectionFasterRCNN() override { Release(); }

  void Setup(const VecString &model_files, const VecInt &in_shape) override;

  void Predict(const JImage &im_src, const VecRectF &rois,
               std::vector<VecBoxF> *Gboxes,
               std::vector<std::vector<VecPointF>> *Gpoints) override;
#if defined(USE_OpenCV)
  void Predict(const cv::Mat &im_mat, const VecRectF &rois,
               std::vector<VecBoxF> *Gboxes,
               std::vector<std::vector<VecPointF>> *Gpoints) override;
#endif

  void Release() override;

 private:
  void Process(const VecFloat &in_data, const VecInt &in_shape,
               const VecFloat &im_info, float height, float width,
               VecBoxF *boxes);

  void CalculateScales(float height, float width, float max_side,
                       const VecFloat &min_side, VecFloat *scales);

  Network net_;
  VecFloat in_data_, min_side_, scales_, im_info_;
  VecInt in_shape_;
  std::string rois_str_, bbox_pred_str_, cls_prob_str_;
  int num_classes_;
  float max_side_, threshold_, nms_threshold_;
  bool is_bgr_, class_agnostic_;
};

}  // namespace Shadow

#endif  // SHADOW_EXAMPLES_DETECTION_FASTER_RCNN_HPP
