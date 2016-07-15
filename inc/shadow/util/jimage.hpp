#ifndef SHADOW_UTIL_JIMAGE_HPP
#define SHADOW_UTIL_JIMAGE_HPP

#include "shadow/util/boxes.hpp"
#include "shadow/util/util.hpp"

#include <string>

#if defined(USE_OpenCV)
#include <opencv2/opencv.hpp>
#endif

class Scalar {
 public:
  Scalar() {}
  Scalar(int r_t, int g_t, int b_t) {
    r = (unsigned char)Util::constrain(0, 255, r_t);
    g = (unsigned char)Util::constrain(0, 255, g_t);
    b = (unsigned char)Util::constrain(0, 255, b_t);
  }
  unsigned char r, g, b;
};

enum Order { kGray, kRGB, kBGR };

class JImage {
 public:
  JImage() : data_(nullptr) {}
  explicit JImage(const std::string im_path) : data_(nullptr) { Read(im_path); }
  JImage(int channel, int height, int width, Order order = kRGB)
      : c_(channel), h_(height), w_(width), order_(order) {
    data_ = new unsigned char[c_ * h_ * w_];
  }
  ~JImage() { Release(); }

  const unsigned char *data() const { return data_; }
  unsigned char *data() { return data_; }

  const unsigned char operator()(int c, int h, int w) const {
    if (c >= c_ || h >= h_ || w >= w_) Fatal("Index out of range!");
    return data_[(c * h_ + h) * w_ + w];
  }
  unsigned char &operator()(int c, int h, int w) {
    if (c >= c_ || h >= h_ || w >= w_) Fatal("Index out of range!");
    return data_[(c * h_ + h) * w_ + w];
  }

  void SetZero() { memset(data_, 0, sizeof(unsigned char) * c_ * h_ * w_); }

  void Read(const std::string im_path);
  void Write(const std::string im_path);
  void Show(const std::string show_name, int wait_time = 0);
  void CopyTo(JImage *im_copy);
  void Resize(JImage *im_res, int height, int width);
  void Crop(JImage *im_crop, RectF crop);
  void CropWithResize(JImage *im_res, RectF crop, int height, int width);
  void Filter2D(const float *kernel, int height, int width);

#if defined(USE_OpenCV)
  void FromMat(const cv::Mat &im_mat);
  void FromMatWithCropResize(const cv::Mat &im_mat, const RectF crop,
                             int resize_h, int resize_w, float *batch_data);
#endif

  void Rectangle(const Box &box, const Scalar scalar = Scalar(0, 255, 0),
                 bool console_show = true);
  void Rectangle(const VecBox &boxes, const Scalar scalar = Scalar(0, 255, 0),
                 bool console_show = true);
  void GetBatchData(float *batch_data);

  void Release();

  int c_, h_, w_;

 private:
  void GetInv(unsigned char *im_inv);

  unsigned char *data_;
  Order order_;
};

#endif  // SHADOW_UTIL_JIMAGE_HPP