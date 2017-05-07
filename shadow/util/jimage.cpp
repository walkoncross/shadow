#include "jimage.hpp"
#include "util.hpp"

//#define USE_STB
#if defined(USE_STB)
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#endif

void JImage::Read(const std::string &im_path) {
  if (data_ != nullptr) {
    delete[] data_;
    data_ = nullptr;
  }
#if defined(USE_OpenCV)
  FromMat(cv::imread(im_path));

#elif defined(USE_STB)
  data_ = stbi_load(im_path.c_str(), &w_, &h_, &c_, 3);
  CHECK_NOTNULL(data_);
  order_ = kRGB;

#else
  LOG(FATAL) << "Not compiled with either OpenCV or STB, could not read image "
             << im_path;
#endif
}

void JImage::Write(const std::string &im_path) const {
  CHECK_NOTNULL(data_);

#if defined(USE_OpenCV)
  cv::imwrite(im_path, ToMat());

#elif defined(USE_STB)
  int is_ok = -1;
  int step = w_ * c_;
  const auto &path = Util::change_extension(im_path, ".png");
  if (order_ == kRGB) {
    is_ok = stbi_write_png(path.c_str(), w_, h_, c_, data_, step);
  } else if (order_ == kBGR) {
    unsigned char *data_inv = new unsigned char[c_ * h_ * w_];
    GetInv(data_inv);
    is_ok = stbi_write_png(path.c_str(), w_, h_, c_, data_inv, step);
    delete[] data_inv;
  } else {
    LOG(FATAL) << "Unsupported format to disk!";
  }
  CHECK(is_ok) << "Failed to write image to " + im_path;

#else
  LOG(FATAL) << "Not compiled with either OpenCV or STB, could not write image "
             << im_path;
#endif
}

void JImage::Show(const std::string &show_name, int wait_time) const {
  CHECK_NOTNULL(data_);

#if defined(USE_OpenCV)
  cv::namedWindow(show_name, cv::WINDOW_NORMAL);
  cv::imshow(show_name, ToMat());
  cv::waitKey(wait_time);

#else
  LOG(WARNING) << "Not compiled with OpenCV, saving image to " << show_name
               << ".png";
  Write(show_name + ".png");
#endif
}

void JImage::CopyTo(JImage *im_copy) const {
  CHECK_NOTNULL(data_);

  im_copy->Reshape(c_, h_, w_, order_);
  memcpy(im_copy->data_, data_, c_ * h_ * w_ * sizeof(unsigned char));
}

#if defined(USE_OpenCV)
void JImage::FromMat(const cv::Mat &im_mat, bool shared) {
  CHECK(!im_mat.empty()) << "Mat data is empty!";

  if (shared) {
    ShareData(im_mat.data);
    c_ = im_mat.channels(), h_ = im_mat.rows, w_ = im_mat.cols, order_ = kBGR;
  } else {
    Reshape(im_mat.channels(), im_mat.rows, im_mat.cols, kBGR);
    memcpy(data_, im_mat.data, c_ * h_ * w_ * sizeof(unsigned char));
  }
}

cv::Mat JImage::ToMat() const {
  if (order_ == kGray) {
    return cv::Mat(h_, w_, CV_8UC1, data_);
  } else if (order_ == kRGB) {
    cv::Mat im_rgb(h_, w_, CV_8UC3, data_), im_bgr;
    cv::cvtColor(im_rgb, im_bgr, CV_RGB2BGR);
    return im_bgr;
  } else if (order_ == kBGR) {
    return cv::Mat(h_, w_, CV_8UC3, data_);
  } else {
    LOG(FATAL) << "Unsupported format to convert to cv mat!";
  }
  return cv::Mat();
}
#endif

#if defined(USE_ArcSoft)
#define CLIP(x) (unsigned char)((x) & (~255) ? ((-x) >> 31) : (x))
#define fix(x, n) static_cast<int>((x) * (1 << (n)) + 0.5)
#define yuvYr fix(0.299, 10)
#define yuvYg fix(0.587, 10)
#define yuvYb fix(0.114, 10)
#define yuvCr fix(0.713, 10)
#define yuvCb fix(0.564, 10)

inline void RGB2I420(unsigned char *src_bgr, int src_h, int src_w, int src_step,
                     Order order, unsigned char *dst_i420) {
  int loc_r = 0, loc_g = 1, loc_b = 2;
  if (order == kRGB) {
    loc_r = 0;
    loc_g = 1;
    loc_b = 2;
  } else if (order == kBGR) {
    loc_r = 2;
    loc_g = 1;
    loc_b = 0;
  } else {
    LOG(FATAL) << "Unsupported order to convert to I420!";
  }
  int r, g, b;
  int dst_h = src_h >> 1, dst_w = src_w >> 1;
  int uv_offset = src_h * src_w, uv_step = dst_h * dst_w;
  int s_h, s_w, y, h_off, w_off, src_offset, y_offset;
  int cb, cb_0, cb_1, cb_2, cb_3, cr, cr_0, cr_1, cr_2, cr_3;
  for (int h = 0; h < dst_h; ++h) {
    for (int w = 0; w < dst_w; ++w) {
      s_h = h << 1, s_w = w << 1;

      h_off = 0, w_off = 0;
      src_offset = (s_h + h_off) * src_step + (s_w + w_off) * 3;
      y_offset = (s_h + h_off) * src_w + s_w + w_off;
      r = src_bgr[src_offset + loc_r];
      g = src_bgr[src_offset + loc_g];
      b = src_bgr[src_offset + loc_b];
      y = (b * yuvYb + g * yuvYg + r * yuvYr) >> 10;
      cb_0 = ((b - y) * yuvCb + (128 << 10)) >> 10;
      cr_0 = ((r - y) * yuvCr + (128 << 10)) >> 10;
      dst_i420[y_offset] = (unsigned char)y;

      h_off = 0, w_off = 1;
      src_offset = (s_h + h_off) * src_step + (s_w + w_off) * 3;
      y_offset = (s_h + h_off) * src_w + s_w + w_off;
      r = src_bgr[src_offset + loc_r];
      g = src_bgr[src_offset + loc_g];
      b = src_bgr[src_offset + loc_b];
      y = (b * yuvYb + g * yuvYg + r * yuvYr) >> 10;
      cb_1 = ((b - y) * yuvCb + (128 << 10)) >> 10;
      cr_1 = ((r - y) * yuvCr + (128 << 10)) >> 10;
      dst_i420[y_offset] = (unsigned char)y;

      h_off = 1, w_off = 0;
      src_offset = (s_h + h_off) * src_step + (s_w + w_off) * 3;
      y_offset = (s_h + h_off) * src_w + s_w + w_off;
      r = src_bgr[src_offset + loc_r];
      g = src_bgr[src_offset + loc_g];
      b = src_bgr[src_offset + loc_b];
      y = (b * yuvYb + g * yuvYg + r * yuvYr) >> 10;
      cb_2 = ((b - y) * yuvCb + (128 << 10)) >> 10;
      cr_2 = ((r - y) * yuvCr + (128 << 10)) >> 10;
      dst_i420[y_offset] = (unsigned char)y;

      h_off = 1, w_off = 1;
      src_offset = (s_h + h_off) * src_step + (s_w + w_off) * 3;
      y_offset = (s_h + h_off) * src_w + s_w + w_off;
      r = src_bgr[src_offset + loc_r];
      g = src_bgr[src_offset + loc_g];
      b = src_bgr[src_offset + loc_b];
      y = (b * yuvYb + g * yuvYg + r * yuvYr) >> 10;
      cb_3 = ((b - y) * yuvCb + (128 << 10)) >> 10;
      cr_3 = ((r - y) * yuvCr + (128 << 10)) >> 10;
      dst_i420[y_offset] = (unsigned char)y;

      cb = CLIP(((cb_0 + cb_1 + cb_2 + cb_3) >> 2));
      cr = CLIP(((cr_0 + cr_1 + cr_2 + cr_3) >> 2));
      int offset = uv_offset + h * dst_w + w;
      dst_i420[offset] = (unsigned char)cb;
      dst_i420[offset + uv_step] = (unsigned char)cr;
    }
  }
}

inline void I4202RGB(unsigned char *src_y, unsigned char *src_u,
                     unsigned char *src_v, int src_h, int src_w, int src_step,
                     unsigned char *dst_rgb, Order order) {
  for (int h = 0; h < src_h; ++h) {
    for (int w = 0; w < src_w; ++w) {
      int y = src_y[h * src_step + w];
      int u = src_u[(h >> 1) * (src_step >> 1) + (w >> 1)];
      int v = src_v[(h >> 1) * (src_step >> 1) + (w >> 1)];
      u -= 128;
      v -= 128;
      int r = y + v + ((v * 103) >> 8);
      int g = y - ((u * 88) >> 8) - ((v * 183) >> 8);
      int b = y + u + ((u * 198) >> 8);

      int offset = (src_w * h + w) * 3;
      dst_rgb[offset + 1] = (unsigned char)Util::constrain(0, 255, g);
      if (order == kRGB) {
        dst_rgb[offset + 0] = (unsigned char)Util::constrain(0, 255, r);
        dst_rgb[offset + 2] = (unsigned char)Util::constrain(0, 255, b);
      } else if (order == kBGR) {
        dst_rgb[offset + 0] = (unsigned char)Util::constrain(0, 255, b);
        dst_rgb[offset + 2] = (unsigned char)Util::constrain(0, 255, r);
      } else {
        LOG(FATAL) << "Unsupported format to convert i420 to rgb!";
      }
    }
  }
}

void JImage::FromArcImage(const ASVLOFFSCREEN &im_arc) {
  int src_h = static_cast<int>(im_arc.i32Height);
  int src_w = static_cast<int>(im_arc.i32Width);
  int src_step = static_cast<int>(im_arc.pi32Pitch[0]);
  this->Reshape(3, src_h, src_w, kRGB);
  switch (im_arc.u32PixelArrayFormat) {
    case ASVL_PAF_I420: {
      I4202RGB(im_arc.ppu8Plane[0], im_arc.ppu8Plane[1], im_arc.ppu8Plane[2],
               src_h, src_w, src_step, data_, order_);
      break;
    }
    default: {
      LOG(FATAL) << "Unsupported format to convert ArcImage to JImage!";
      break;
    }
  }
}

void JImage::ToArcImage(int arc_format) {
  CHECK_NOTNULL(data_);

  if (order_ == kArc) return;
  if (order_ != kRGB && order_ != kBGR) {
    LOG(FATAL) << "Unsupported format to convert JImage to ArcImage!";
  }
  switch (arc_format) {
    case ASVL_PAF_I420: {
      int src_h = (h_ >> 1) << 1;
      int src_w = (w_ >> 1) << 1;
      int src_step = w_ * c_;
      if (arc_data_ == nullptr) {
        arc_data_ = new unsigned char[src_h * src_w * 3 / 2];
      }
      RGB2I420(data_, src_h, src_w, src_step, order_, arc_data_);
      arc_image_.i32Height = src_h;
      arc_image_.i32Width = src_w;
      arc_image_.ppu8Plane[0] = arc_data_;
      arc_image_.ppu8Plane[1] = arc_data_ + src_h * src_w;
      arc_image_.ppu8Plane[2] = arc_data_ + src_h * src_w * 5 / 4;
      arc_image_.pi32Pitch[0] = src_w;
      arc_image_.pi32Pitch[1] = src_w >> 1;
      arc_image_.pi32Pitch[2] = src_w >> 1;
      arc_image_.u32PixelArrayFormat = ASVL_PAF_I420;
      break;
    }
    default: {
      LOG(FATAL) << "Unsupported ArcImage format!";
      break;
    }
  }
}
#endif

void JImage::Color2Gray() {
  CHECK_NOTNULL(data_);

  if (order_ == kRGB || order_ == kBGR) {
    unsigned char *gray_data = new unsigned char[h_ * w_];
    unsigned char *gray_index = gray_data;
    int index;
    for (int h = 0; h < h_; ++h) {
      for (int w = 0; w < w_; ++w) {
        index = (w_ * h + w) * c_;
        float sum = data_[index + 0] + data_[index + 1] + data_[index + 2];
        *gray_index++ = static_cast<unsigned char>(sum / 3.f);
      }
    }
    memcpy(data_, gray_data, h_ * w_ * sizeof(unsigned char));
    delete[] gray_data;
    c_ = 1;
    order_ = kGray;
  } else if (order_ == kGray) {
    return;
  } else {
    LOG(FATAL) << "Unsupported format to convert color to gray!";
  }
}

void JImage::ColorInv() {
  CHECK_NOTNULL(data_);

  if (order_ == kRGB) {
    order_ = kBGR;
  } else if (order_ == kBGR) {
    order_ = kRGB;
  } else {
    LOG(FATAL) << "Unsupported format to do inverse!";
  }

  int spatial_dim = h_ * w_;
  unsigned char *data_index = data_, temp;
  for (int i = 0; i < spatial_dim; ++i) {
    temp = *(data_index + c_ - 1);
    *(data_index + c_ - 1) = *data_index;
    *data_index = temp;
    data_index += c_;
  }
}

void JImage::Release() {
  if (data_ != nullptr && !shared_) {
    delete[] data_;
    data_ = nullptr;
  }

#if defined(USE_ArcSoft)
  if (arc_data_ != nullptr) {
    delete[] arc_data_;
    arc_data_ = nullptr;
  }
#endif
}

void JImage::GetInv(unsigned char *im_inv) const {
  CHECK_NOTNULL(data_);

  if (order_ == kRGB || order_ == kBGR) {
    int spatial_dim = h_ * w_;
    unsigned char *data_index = data_;
    for (int i = 0; i < spatial_dim; ++i) {
      *(im_inv++) = *(data_index + c_ - 1);
      *(im_inv++) = *(data_index + 1);
      *(im_inv++) = *data_index;
      data_index += c_;
    }
  } else {
    LOG(FATAL) << "Unsupported format to get inverse!";
  }
}