// Copyright 2017 Google Inc. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "packager/media/base/media_handler_test_base.h"

#include "packager/media/base/audio_stream_info.h"
#include "packager/media/base/video_stream_info.h"
#include "packager/status_test_util.h"

namespace {

const int kTrackId = 1;
const uint64_t kDuration = 10000;
const char kCodecString[] = "codec string";
const uint8_t kSampleBits = 1;
const uint8_t kNumChannels = 2;
const uint32_t kSamplingFrequency = 48000;
const uint64_t kSeekPrerollNs = 12345;
const uint64_t kCodecDelayNs = 56789;
const uint32_t kMaxBitrate = 13579;
const uint32_t kAvgBitrate = 13000;
const char kLanguage[] = "eng";
const uint16_t kWidth = 10u;
const uint16_t kHeight = 20u;
const uint32_t kPixelWidth = 2u;
const uint32_t kPixelHeight = 3u;
const int16_t kTrickPlayFactor = 0;
const uint8_t kNaluLengthSize = 1u;
const bool kEncrypted = true;

// Use H264 code config.
const uint8_t kCodecConfig[]{
    // Header
    0x01, 0x64, 0x00, 0x1e, 0xff,
    // SPS count (ignore top three bits)
    0xe1,
    // SPS
    0x00, 0x19,  // Size
    0x67, 0x64, 0x00, 0x1e, 0xac, 0xd9, 0x40, 0xa0, 0x2f, 0xf9, 0x70, 0x11,
    0x00, 0x00, 0x03, 0x03, 0xe9, 0x00, 0x00, 0xea, 0x60, 0x0f, 0x16, 0x2d,
    0x96,
    // PPS count
    0x01,
    // PPS
    0x00, 0x06,  // Size
    0x68, 0xeb, 0xe3, 0xcb, 0x22, 0xc0,
};

// Mock data, we don't really care about what is inside.
const uint8_t kData[]{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
};

}  // namespace

namespace shaka {
namespace media {

bool FakeInputMediaHandler::ValidateOutputStreamIndex(size_t index) const {
  return true;
}

Status FakeInputMediaHandler::InitializeInternal() {
  return Status::OK;
}

Status FakeInputMediaHandler::Process(std::unique_ptr<StreamData> stream_data) {
  return Status::OK;
}

Status MockOutputMediaHandler::InitializeInternal() {
  return Status::OK;
}

Status MockOutputMediaHandler::Process(
    std::unique_ptr<StreamData> stream_data) {
  OnProcess(stream_data.get());
  return Status::OK;
}

Status MockOutputMediaHandler::OnFlushRequest(size_t index) {
  OnFlush(index);
  return Status::OK;
}

Status FakeMediaHandler::InitializeInternal() {
  return Status::OK;
}

Status FakeMediaHandler::Process(std::unique_ptr<StreamData> stream_data) {
  stream_data_vector_.push_back(std::move(stream_data));
  return Status::OK;
}

Status FakeMediaHandler::OnFlushRequest(size_t input_stream_index) {
  return Status::OK;
}

bool FakeMediaHandler::ValidateOutputStreamIndex(size_t stream_index) const {
  return true;
}

MediaHandlerTestBase::MediaHandlerTestBase()
    : next_handler_(new FakeMediaHandler),
      some_handler_(new FakeMediaHandler) {}

bool MediaHandlerTestBase::IsVideoCodec(Codec codec) const {
  return codec >= kCodecVideo && codec < kCodecVideoMaxPlusOne;
}

std::unique_ptr<StreamInfo> MediaHandlerTestBase::GetVideoStreamInfo(
    uint32_t time_scale) const {
  return GetVideoStreamInfo(time_scale, kCodecVP9, kWidth, kHeight);
}

std::unique_ptr<StreamInfo> MediaHandlerTestBase::GetVideoStreamInfo(
    uint32_t time_scale,
    uint32_t width,
    uint64_t height) const {
  return GetVideoStreamInfo(time_scale, kCodecVP9, width, height);
}

std::unique_ptr<StreamInfo> MediaHandlerTestBase::GetVideoStreamInfo(
    uint32_t time_scale,
    Codec codec) const {
  return GetVideoStreamInfo(time_scale, codec, kWidth, kHeight);
}

std::unique_ptr<StreamInfo> MediaHandlerTestBase::GetVideoStreamInfo(
    uint32_t time_scale,
    Codec codec,
    uint32_t width,
    uint64_t height) const {
  return std::unique_ptr<VideoStreamInfo>(new VideoStreamInfo(
      kTrackId, time_scale, kDuration, codec, H26xStreamFormat::kUnSpecified,
      kCodecString, kCodecConfig, sizeof(kCodecConfig), width, height,
      kPixelWidth, kPixelHeight, kTrickPlayFactor, kNaluLengthSize, kLanguage,
      !kEncrypted));
}

std::unique_ptr<StreamInfo> MediaHandlerTestBase::GetAudioStreamInfo(
    uint32_t time_scale) const {
  return GetAudioStreamInfo(time_scale, kCodecAAC);
}

std::unique_ptr<StreamInfo> MediaHandlerTestBase::GetAudioStreamInfo(
    uint32_t time_scale,
    Codec codec) const {
  return std::unique_ptr<AudioStreamInfo>(new AudioStreamInfo(
      kTrackId, time_scale, kDuration, codec, kCodecString, kCodecConfig,
      sizeof(kCodecConfig), kSampleBits, kNumChannels, kSamplingFrequency,
      kSeekPrerollNs, kCodecDelayNs, kMaxBitrate, kAvgBitrate, kLanguage,
      !kEncrypted));
}

std::unique_ptr<MediaSample> MediaHandlerTestBase::GetMediaSample(
    int64_t timestamp,
    int64_t duration,
    bool is_keyframe) const {
  return GetMediaSample(timestamp, duration, is_keyframe, kData, sizeof(kData));
}

std::unique_ptr<MediaSample> MediaHandlerTestBase::GetMediaSample(
    int64_t timestamp,
    int64_t duration,
    bool is_keyframe,
    const uint8_t* data,
    size_t data_length) const {
  std::unique_ptr<MediaSample> sample(
      new MediaSample(data, data_length, nullptr, 0, is_keyframe));
  sample->set_dts(timestamp);
  sample->set_duration(duration);

  return sample;
}

std::unique_ptr<SegmentInfo> MediaHandlerTestBase::GetSegmentInfo(
    int64_t start_timestamp,
    int64_t duration,
    bool is_subsegment) const {
  std::unique_ptr<SegmentInfo> info(new SegmentInfo);
  info->start_timestamp = start_timestamp;
  info->duration = duration;
  info->is_subsegment = is_subsegment;

  return info;
}

void MediaHandlerTestBase::SetUpGraph(size_t num_inputs,
                                      size_t num_outputs,
                                      std::shared_ptr<MediaHandler> handler) {
  // Input handler is not really used anywhere but just to satisfy one input
  // one output restriction for the encryption handler.
  auto input_handler = std::make_shared<FakeMediaHandler>();
  for (size_t i = 0; i < num_inputs; ++i)
    ASSERT_OK(input_handler->SetHandler(i, handler));
  // All outputs are routed to |next_handler_|.
  for (size_t i = 0; i < num_outputs; ++i)
    ASSERT_OK(handler->SetHandler(i, next_handler_));
}

const std::vector<std::unique_ptr<StreamData>>&
MediaHandlerTestBase::GetOutputStreamDataVector() const {
  return next_handler_->stream_data_vector();
}

void MediaHandlerTestBase::ClearOutputStreamDataVector() {
  next_handler_->clear_stream_data_vector();
}

}  // namespace media
}  // namespace shaka