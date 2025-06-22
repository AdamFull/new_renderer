#include "EffectNodes.h"
#include "AudioFile.h"
#include "Util.h"

#include <edge/util/Logger.h>

#include <edge/fs/fs_virtual_filesystem.h>
#include <edge/fs/fs_helper.h>
#include <edge/util/Logger.h>

#include <imgui-knobs.h>

namespace e2d::audio {
	OutputGraphNode::OutputGraphNode() {
		input_sockets_.emplace_back(id_, 0, NodeSocket::Type::eInput, "Audio Input");
	}

    auto OutputGraphNode::process(std::span<uint8_t> out, const AudioFormat& format,
        const std::vector<std::span<const uint8_t>>& inputs) -> size_t {
        if (inputs.empty()) {
            std::fill(out.begin(), out.end(), 0);
            return out.size();
        }

        const auto& input = inputs[0];
        std::copy_n(input.begin(), std::min(out.size(), input.size()), out.begin());

        auto float_out = std::span<float>((float*)out.data(), out.size() / sizeof(float));
        for (auto& sample : float_out) {
            sample *= master_volume_;
        }

        return out.size();
    }

    auto OutputGraphNode::render() -> bool {
        return ImGuiKnobs::Knob("Volume", &master_volume_, 0.0f, 5.0f);
    }

    auto OutputGraphNode::to_json() const -> nlohmann::json {
        nlohmann::json j;
        j["master_volume"] = master_volume_;
        return j;
    }

    auto OutputGraphNode::from_json(const nlohmann::json& json) -> void {
        if (json.contains("master_volume")) {
            master_volume_ = json["master_volume"];
        }
    }



    OscillatorGraphNode::OscillatorGraphNode() {
        input_sockets_.emplace_back(id_, 0, NodeSocket::Type::eInput, "Frequency");
        input_sockets_.emplace_back(id_, 1, NodeSocket::Type::eInput, "Amplitude");

        output_sockets_.emplace_back(id_, 0, NodeSocket::Type::eOutput, "Output");
    }

    auto OscillatorGraphNode::process(std::span<uint8_t> out, const AudioFormat& format,
        const std::vector<std::span<const uint8_t>>& inputs) -> size_t {
        auto delta_phase = math::two_pi * frequency_ / static_cast<float>(format.sample_rate);
        auto float_out = std::span<float>((float*)out.data(), out.size() / sizeof(float));

        for (int32_t i = 0; i < float_out.size(); ++i) {
            float sample = 0.0f;

            switch (wave_type_) {
            case WaveType::eSine:
                sample = std::sinf(phase_);
                break;
            case WaveType::eSquare:
                sample = (phase_ < math::pi) ? 1.0f : -1.0f;
                break;
            case WaveType::eSaw:
                sample = (2.0f * phase_ / math::two_pi) - 1.0f;
                break;
            case WaveType::eTriangle: {
                const float normalized = phase_ / math::two_pi;
                sample = (normalized < 0.5f) ? (4.0f * normalized - 1.0f) : (3.0f - 4.0f * normalized);
                break;
            }
            }

            float_out[i] = sample * amplitude_;

            phase_ += delta_phase;
            if (phase_ >= math::two_pi) {
                phase_ -= math::two_pi;
            }
        }

        return out.size();
    }

    auto OscillatorGraphNode::render() -> bool {
        bool changed = false;

        const char* items[] = { "Sine", "Square", "Saw", "Triangle" };
        int current_item = static_cast<int>(wave_type_);

        if (ImGui::Combo("Wave", &current_item, items, IM_ARRAYSIZE(items))) {
            wave_type_ = static_cast<WaveType>(current_item);
            changed = true;
        }

        if (ImGuiKnobs::Knob("Freq", &frequency_, 0.0f, 20000.0f, 1.0f, "%.0f")) {
            changed = true;
        }

        ImGui::SameLine();

        if (ImGuiKnobs::Knob("Amp", &amplitude_, 0.0f, 1.0f)) {
            changed = true;
        }

        ImGui::SameLine();

        if (ImGuiKnobs::Knob("Start phase", &start_phase_, -math::two_pi, math::two_pi)) {
            phase_ = start_phase_;
            changed = true;
        }

        return changed;
    }

    auto OscillatorGraphNode::to_json() const -> nlohmann::json {
        nlohmann::json j;
        j["wave_type"] = static_cast<int>(wave_type_);
        j["frequency"] = frequency_;
        j["amplitude"] = amplitude_;
        j["phase"] = phase_;
        return j;
    }

    auto OscillatorGraphNode::from_json(const nlohmann::json& json) -> void {
        if (json.contains("wave_type")) {
            wave_type_ = static_cast<WaveType>(json["wave_type"].get<int>());
        }
        if (json.contains("frequency")) {
            frequency_ = json["frequency"];
        }
        if (json.contains("amplitude")) {
            amplitude_ = json["amplitude"];
        }
        if (json.contains("phase")) {
            phase_ = json["phase"];
        }
    }



    FileInputGraphNode::FileInputGraphNode() {
        output_sockets_.emplace_back(id_, 0, NodeSocket::Type::eOutput, "Audio");
    }

    auto FileInputGraphNode::process(std::span<uint8_t> out, const AudioFormat& format,
        const std::vector<std::span<const uint8_t>>& inputs) -> size_t {
        if (!file_) {
            std::fill(out.begin(), out.end(), 0);
            return 0;
        }

        return file_->read_samples(out, format);
    }

    auto FileInputGraphNode::render() -> bool {
        bool changed = false;

        ImGui::Text("File: %s", file_path_.c_str());
        if (ImGui::Button("Browse...")) {
            const char* path = "/internal_res/assets/iamtheprotectorofthissystem.mp3";
            if (load_file(path)) {
                changed = true;
            }
        }

        if (ImGui::Checkbox("Loop", &loop_)) {
            changed = true;
        }

        return changed;
    }

    auto FileInputGraphNode::to_json() const -> nlohmann::json {
        nlohmann::json j;
        j["file_path"] = file_path_;
        j["loop"] = loop_;
        return j;
    }

    auto FileInputGraphNode::from_json(const nlohmann::json& json) -> void {
        if (json.contains("file_path")) {
            file_path_ = json["file_path"];
            load_file(file_path_);
        }
        if (json.contains("loop")) {
            loop_ = json["loop"];
        }
    }

    auto FileInputGraphNode::load_file(const std::string& path) -> bool {
        using namespace edge;

        file_path_ = path;

        if (!edge::fs::exists(path)) {
            EDGE_LOGE("[FileInputGraphNode] File not found: {}", path);
            return false;
        }

        if (!edge::fs::read_file(path, file_bytes_)) {
            EDGE_LOGE("[FileInputGraphNode] Failed to load file: {}", path);
            return false;
        }

        auto file_ext = edge::fs::get_ext(path);
        auto ext_hash = edge::murmur_hash(file_ext);

        constexpr auto wav = ".wav"_hash32;
        constexpr auto mp3 = ".mp3"_hash32;
        constexpr auto flac = ".flac"_hash32;

        switch (ext_hash) {
        case wav:
            file_ = WavFile::construct(file_bytes_);
            break;
        case mp3:
            file_ = Mp3File::construct(file_bytes_);
            break;
        case flac:
            file_ = FlacFile::construct(file_bytes_);
            break;
        default:
            EDGE_LOGE("[FileInputGraphNode] Unsupported file format: {}", file_ext);
            return false;
        }

        return file_ != nullptr;
    }



    BiquadFilterGraphNode::BiquadFilterGraphNode() {
        filter_.set_sample_rate(44100);
        input_sockets_.emplace_back(id_, 0, NodeSocket::Type::eInput, "Input");
        output_sockets_.emplace_back(id_, 0, NodeSocket::Type::eOutput, "Filtered");
    }

    auto BiquadFilterGraphNode::process(std::span<uint8_t> out, const AudioFormat& format,
        const std::vector<std::span<const uint8_t>>& inputs) -> size_t {
        filter_.set_sample_rate(format.sample_rate);

        if (inputs.empty()) {
            std::fill(out.begin(), out.end(), 0);
            return out.size();
        }

        // Copy input to output first
        const auto& input = inputs[0];
        std::copy_n(input.begin(), std::min(out.size(), input.size()), out.begin());

        // Apply filter
        auto float_out = std::span<float>((float*)out.data(), out.size() / sizeof(float));
        for (auto& sample : float_out) {
            sample = filter_.process(sample);
        }

        return out.size();
    }

    auto BiquadFilterGraphNode::render() -> bool {
        bool changed = false;

        const char* items[] = {
                "Low Pass", "Band Pass", "High Pass", "All Pass",
                "Notch", "Bell", "Low Shelf", "High Shelf"
        };
        int current_item = static_cast<int>(filter_.mode_);

        if (ImGui::Combo("Type", &current_item, items, IM_ARRAYSIZE(items))) {
            filter_.set_mode(static_cast<BiquadFilter::Mode>(current_item));
            changed = true;
        }

        float frequency = filter_.cutoff_freq_;
        if (ImGuiKnobs::Knob("Freq", &frequency, 0.0f, 20000.0f, 1.0f, "%.0f")) {
            filter_.set_frequency(frequency);
            changed = true;
        }

        ImGui::SameLine();

        float q_factor = filter_.q_factor_;
        if (ImGuiKnobs::Knob("Q", &q_factor, 0.1f, 10.0f)) {
            filter_.set_q(q_factor);
            changed = true;
        }

        ImGui::SameLine();

        float gain = filter_.gain_db_;
        if (ImGuiKnobs::Knob("Gain", &gain, 1.0f, 10.0f)) {
            filter_.set_gain(gain);
            changed = true;
        }

        return changed;
    }

    auto BiquadFilterGraphNode::to_json() const -> nlohmann::json {
        return nlohmann::json(filter_);
    }

    auto BiquadFilterGraphNode::from_json(const nlohmann::json& json) -> void {
        filter_ = json;
    }



    EqualizerGraphNode::EqualizerGraphNode() : 
        filter_(EqualizerFilter::Type::e10Band, 44100) {
        input_sockets_.emplace_back(id_, 0, NodeSocket::Type::eInput, "Input");
        output_sockets_.emplace_back(id_, 0, NodeSocket::Type::eOutput, "Equalized");
    }

    auto EqualizerGraphNode::process(std::span<uint8_t> out, const AudioFormat& format,
        const std::vector<std::span<const uint8_t>>& inputs) -> size_t {
        filter_.set_sample_rate(format.sample_rate);

        if (inputs.empty()) {
            std::fill(out.begin(), out.end(), 0);
            return out.size();
        }

        const auto& input = inputs[0];
        std::copy_n(input.begin(), std::min(out.size(), input.size()), out.begin());

        auto float_out = std::span<float>((float*)out.data(), out.size() / sizeof(float));
        for (auto& sample : float_out) {
            sample = filter_.process(sample);
        }

        return out.size();
    }

    auto EqualizerGraphNode::render() -> bool {
        bool changed = false;

        ImGui::Text("EQ Bands: %zu", filter_.get_band_count());

        return changed;
    }

    auto EqualizerGraphNode::to_json() const -> nlohmann::json {
        return nlohmann::json(filter_);
    }

    auto EqualizerGraphNode::from_json(const nlohmann::json& json) -> void {
        filter_ = json;
    }



    GainGraphNode::GainGraphNode() {
        input_sockets_.emplace_back(id_, 0, NodeSocket::Type::eInput, "Input");
        output_sockets_.emplace_back(id_, 0, NodeSocket::Type::eOutput, "Output");
    }

    auto GainGraphNode::process(std::span<uint8_t> out, const AudioFormat& format,
        const std::vector<std::span<const uint8_t>>& inputs) -> size_t {
        if (inputs.empty()) {
            std::fill(out.begin(), out.end(), 0);
            return out.size();
        }

        const auto& input = inputs[0];
        std::copy_n(input.begin(), std::min(out.size(), input.size()), out.begin());

        auto float_out = std::span<float>((float*)out.data(), out.size() / sizeof(float));
        for (auto& sample : float_out) {
            sample *= gain_;
        }

        return out.size();
    }

    auto GainGraphNode::render() -> bool {
        bool changed = false;

        if (ImGuiKnobs::Knob("Gain", &gain_, 0.0f, 2.0f)) {
            changed = true;
        }

        return changed;
    }

    auto GainGraphNode::to_json() const -> nlohmann::json {
        nlohmann::json j;
        j["gain"] = gain_;
        return j;
    }

    auto GainGraphNode::from_json(const nlohmann::json& json) -> void {
        if (json.contains("gain")) {
            gain_ = json["gain"];
        }
    }



    // 2 seconds max delay at 44.1kHz
    DelayGraphNode::DelayGraphNode() 
        : delay_(88200) { 
        input_sockets_.emplace_back(id_, 0, NodeSocket::Type::eInput, "Input");
        output_sockets_.emplace_back(id_, 0, NodeSocket::Type::eOutput, "Delayed");
        delay_.set_delay_time(delay_time_, last_sample_rate_);
    }

    auto DelayGraphNode::process(std::span<uint8_t> out, const AudioFormat& format,
        const std::vector<std::span<const uint8_t>>& inputs) -> size_t {
        if (inputs.empty() || inputs[0].empty()) {
            std::fill(out.begin(), out.end(), 0);
            return out.size();
        }

        // Update delay time if sample rate changed
        if (format.sample_rate != last_sample_rate_) {
            last_sample_rate_ = format.sample_rate;
            delay_.set_delay_time(delay_time_, format.sample_rate);
        }

        const auto& input = inputs[0];
        std::copy_n(input.begin(), std::min(out.size(), input.size()), out.begin());

        auto float_out = std::span<float>((float*)out.data(), out.size() / sizeof(float));
        for (auto& sample : float_out) {
            sample = delay_.process(sample);
        }

        return out.size();
    }

    auto DelayGraphNode::render() -> bool {
        bool changed = false;

        if (ImGuiKnobs::Knob("Feedback", &delay_.feedback_, 0.0f, 5.0f)) {
            changed = true;
        }

        ImGui::SameLine();

        if (ImGuiKnobs::Knob("Mix", &delay_.mix_, 0.0f, 1.0f)) {
            changed = true;
        }

        ImGui::SameLine();

        if (ImGuiKnobs::Knob("Time", &delay_time_, 0.0f, 1.0f)) {
            delay_.set_delay_time(delay_time_, last_sample_rate_);
            changed = true;
        }

        return changed;
    }

    auto DelayGraphNode::to_json() const -> nlohmann::json {
        nlohmann::json j;
        j["delay_time"] = delay_time_;
        j["feedback"] = delay_.feedback_;
        j["mix"] = delay_.mix_;
        return j;
    }

    auto DelayGraphNode::from_json(const nlohmann::json& json) -> void {
        if (json.contains("delay_time")) {
            delay_time_ = json["delay_time"];
            delay_.set_delay_time(delay_time_, last_sample_rate_);
        }
        if (json.contains("feedback")) {
            delay_.feedback_ = json["feedback"];
        }
        if (json.contains("mix")) {
            delay_.mix_ = json["mix"];
        }
    }



    ReverbGraphNode::ReverbGraphNode() 
        : reverb_(44100) {
        input_sockets_.emplace_back(id_, 0, NodeSocket::Type::eInput, "Input");
        output_sockets_.emplace_back(id_, 0, NodeSocket::Type::eOutput, "Reverb");
    }

    auto ReverbGraphNode::process(std::span<uint8_t> out, const AudioFormat& format,
        const std::vector<std::span<const uint8_t>>& inputs) -> size_t {
        if (inputs.empty() || inputs[0].empty()) {
            std::fill(out.begin(), out.end(), 0);
            return out.size();
        }

        const auto& input = inputs[0];
        std::copy_n(input.begin(), std::min(out.size(), input.size()), out.begin());

        auto float_out = std::span<float>((float*)out.data(), out.size() / sizeof(float));
        for (auto& sample : float_out) {
            sample = reverb_.process(sample);
        }

        return out.size();
    }

    auto ReverbGraphNode::render() -> bool {
        bool changed = false;

        if (ImGuiKnobs::Knob("Room size", &reverb_.room_size_, 0.1f, 10.0f)) {
            changed = true;
        }

        ImGui::SameLine();

        if (ImGuiKnobs::Knob("Damping", &reverb_.damping_, 0.01f, 1.0f)) {
            changed = true;
        }

        ImGui::SameLine();

        if (ImGuiKnobs::Knob("Wet", &reverb_.wet_level_, 0.0f, 1.0f)) {
            changed = true;
        }

        if (ImGuiKnobs::Knob("Dry", &reverb_.dry_level_, 0.0f, 1.0f)) {
            changed = true;
        }

        return changed;
    }

    auto ReverbGraphNode::to_json() const -> nlohmann::json {
        nlohmann::json j;
        j["room_size"] = reverb_.room_size_;
        j["damping"] = reverb_.damping_;
        j["wet_level"] = reverb_.wet_level_;
        j["dry_level"] = reverb_.dry_level_;
        return j;
    }

    auto ReverbGraphNode::from_json(const nlohmann::json& json) -> void {
        if (json.contains("room_size")) {
            reverb_.room_size_ = json["room_size"];
        }
        if (json.contains("damping")) {
            reverb_.damping_ = json["damping"];
        }
        if (json.contains("wet_level")) {
            reverb_.wet_level_ = json["wet_level"];
        }
        if (json.contains("dry_level")) {
            reverb_.dry_level_ = json["dry_level"];
        }
    }



    CompressorGraphNode::CompressorGraphNode() {
        input_sockets_.emplace_back(id_, 0, NodeSocket::Type::eInput, "Input");
        output_sockets_.emplace_back(id_, 0, NodeSocket::Type::eOutput, "Compressed");
        compressor_.set_sample_rate(44100);
    }

    auto CompressorGraphNode::process(std::span<uint8_t> out, const AudioFormat& format,
        const std::vector<std::span<const uint8_t>>& inputs) -> size_t {
        if (inputs.empty() || inputs[0].empty()) {
            std::fill(out.begin(), out.end(), 0);
            return out.size();
        }

        compressor_.set_sample_rate(format.sample_rate);

        const auto& input = inputs[0];
        std::copy_n(input.begin(), std::min(out.size(), input.size()), out.begin());

        auto float_out = std::span<float>((float*)out.data(), out.size() / sizeof(float));
        for (auto& sample : float_out) {
            sample = compressor_.process(sample);
        }

        return out.size();
    }

    auto CompressorGraphNode::render() -> bool {
        bool changed = false;

        if (ImGuiKnobs::Knob("Threshold", &compressor_.threshold_, -120.0f, 120.0f, 1.0f, "%.0fDb")) {
            changed = true;
        }

        ImGui::SameLine();

        if (ImGuiKnobs::Knob("Ratio", &compressor_.ratio_, 1.0f, 10.0f)) {
            changed = true;
        }

        ImGui::SameLine();

        if (ImGuiKnobs::Knob("Attack", &compressor_.attack_time_, 0.0f, 2.0f, 0.01f, "%.3fs")) {
            compressor_.update_coefficients();
            changed = true;
        }

        if (ImGuiKnobs::Knob("Release", &compressor_.release_time_, 0.0f, 2.0f, 0.01f, "%.3fs")) {
            compressor_.update_coefficients();
            changed = true;
        }

        ImGui::SameLine();

        if (ImGuiKnobs::Knob("Makeup gain", &compressor_.makeup_gain_, -120.0f, 120.0f, 1.0f, "%.0fDb")) {
            changed = true;
        }

        return changed;
    }

    auto CompressorGraphNode::to_json() const -> nlohmann::json {
        nlohmann::json j;
        j["threshold"] = compressor_.threshold_;
        j["ratio"] = compressor_.ratio_;
        j["attack_time"] = compressor_.attack_time_;
        j["release_time"] = compressor_.release_time_;
        j["makeup_gain"] = compressor_.makeup_gain_;
        return j;
    }

    auto CompressorGraphNode::from_json(const nlohmann::json& json) -> void {
        if (json.contains("threshold")) {
            compressor_.threshold_ = json["threshold"];
        }
        if (json.contains("ratio")) {
            compressor_.ratio_ = json["ratio"];
        }
        if (json.contains("attack_time")) {
            compressor_.attack_time_ = json["attack_time"];
            compressor_.update_coefficients();
        }
        if (json.contains("release_time")) {
            compressor_.release_time_ = json["release_time"];
            compressor_.update_coefficients();
        }
        if (json.contains("makeup_gain")) {
            compressor_.makeup_gain_ = json["makeup_gain"];
        }
    }



    NoiseGraphNode::NoiseGraphNode() {
        output_sockets_.emplace_back(id_, 0, NodeSocket::Type::eOutput, "Noise");
    }

    auto NoiseGraphNode::process(std::span<uint8_t> out, const AudioFormat& format,
        const std::vector<std::span<const uint8_t>>& inputs) -> size_t {
        auto float_out = std::span<float>((float*)out.data(), out.size() / sizeof(float));

        for (auto& sample : float_out) {
            sample = generator_.process();
        }

        return out.size();
    }

    auto NoiseGraphNode::render() -> bool {
        bool changed = false;

        const char* items[] = {
                "White", "Pink", "Brown"
        };
        int current_item = static_cast<int>(generator_.type_);

        if (ImGui::Combo("Type", &current_item, items, IM_ARRAYSIZE(items))) {
            generator_.type_ = static_cast<NoiseGenerator::Type>(current_item);
            changed = true;
        }

        if (ImGuiKnobs::Knob("Amplitude", &generator_.amplitude_, 0.01f, 2.0f)) {
            changed = true;
        }

        return changed;
    }

    auto NoiseGraphNode::to_json() const -> nlohmann::json {
        nlohmann::json j;
        j["noise_type"] = static_cast<int>(generator_.type_);
        j["amplitude"] = generator_.amplitude_;
        return j;
    }

    auto NoiseGraphNode::from_json(const nlohmann::json& json) -> void {
        if (json.contains("noise_type")) {
            generator_.type_ = static_cast<NoiseGenerator::Type>(json["noise_type"].get<int>());
        }
        if (json.contains("amplitude")) {
            generator_.amplitude_ = json["amplitude"];
        }
    }


    LFOGraphNode::LFOGraphNode() {
        input_sockets_.emplace_back(id_, 0, NodeSocket::Type::eInput, "Frequency");
        input_sockets_.emplace_back(id_, 1, NodeSocket::Type::eInput, "Amplitude");
        output_sockets_.emplace_back(id_, 0, NodeSocket::Type::eOutput, "LFO Out");
    }

    auto LFOGraphNode::process(std::span<uint8_t> out, const AudioFormat& format,
        const std::vector<std::span<const uint8_t>>& inputs) -> size_t {
        auto delta_phase = math::two_pi * frequency_ / static_cast<float>(format.sample_rate);
        auto float_out = std::span<float>((float*)out.data(), out.size() / sizeof(float));

        for (int32_t i = 0; i < float_out.size(); ++i) {
            float sample = 0.0f;

            switch (wave_type_) {
            case WaveType::eSine:
                sample = std::sinf(phase_);
                break;
            case WaveType::eSquare:
                sample = (phase_ < math::pi) ? 1.0f : -1.0f;
                break;
            case WaveType::eSaw:
                sample = (2.0f * phase_ / math::two_pi) - 1.0f;
                break;
            case WaveType::eTriangle: {
                const float normalized = phase_ / math::two_pi;
                sample = (normalized < 0.5f) ? (4.0f * normalized - 1.0f) : (3.0f - 4.0f * normalized);
                break;
            }
            }

            float_out[i] = sample * amplitude_;

            phase_ += delta_phase;
            if (phase_ >= math::two_pi) {
                phase_ -= math::two_pi;
            }
        }

        return out.size();
    }

    auto LFOGraphNode::render() -> bool {
        bool changed = false;

        const char* items[] = { "Sine", "Square", "Saw", "Triangle" };
        int current_item = static_cast<int>(wave_type_);

        if (ImGui::Combo("Wave", &current_item, items, IM_ARRAYSIZE(items))) {
            wave_type_ = static_cast<WaveType>(current_item);
            changed = true;
        }

        if (ImGuiKnobs::Knob("Freq", &frequency_, 0.0f, 20000.0f, 1.0f, "%.0f")) {
            changed = true;
        }

        ImGui::SameLine();

        if (ImGuiKnobs::Knob("Amp", &amplitude_, 0.0f, 1.0f)) {
            changed = true;
        }

        return changed;
    }

    auto LFOGraphNode::to_json() const -> nlohmann::json {
        nlohmann::json j;
        j["frequency"] = frequency_;
        j["amplitude"] = amplitude_;
        j["wave_type"] = static_cast<int>(wave_type_);
        j["phase"] = phase_;
        return j;
    }

    auto LFOGraphNode::from_json(const nlohmann::json& json) -> void {
        if (json.contains("frequency")) {
            frequency_ = json["frequency"];
        }
        if (json.contains("amplitude")) {
            amplitude_ = json["amplitude"];
        }
        if (json.contains("wave_type")) {
            wave_type_ = static_cast<WaveType>(json["wave_type"].get<int>());
        }
        if (json.contains("phase")) {
            phase_ = json["phase"];
        }
    }



    EnvelopeGraphNode::EnvelopeGraphNode() {
        input_sockets_.emplace_back(id_, 0, NodeSocket::Type::eInput, "Trigger");
        input_sockets_.emplace_back(id_, 1, NodeSocket::Type::eInput, "Release");
        output_sockets_.emplace_back(id_, 0, NodeSocket::Type::eOutput, "Envelope");
    }

    auto EnvelopeGraphNode::process(std::span<uint8_t> out, const AudioFormat& format,
        const std::vector<std::span<const uint8_t>>& inputs) -> size_t {
        sample_rate_ = format.sample_rate;
        auto float_out = std::span<float>((float*)out.data(), out.size() / sizeof(float));

        for (auto& sample : float_out) {
            switch (current_stage_) {
            case Stage::eIdle:
                current_level_ = 0.0f;
                break;

            case Stage::eAttack: {
                float attack_samples = attack_time_ * sample_rate_;
                stage_progress_ += 1.0f;
                current_level_ = stage_progress_ / attack_samples;

                if (current_level_ >= 1.0f) {
                    current_level_ = 1.0f;
                    current_stage_ = Stage::eDecay;
                    stage_progress_ = 0.0f;
                }
                break;
            }

            case Stage::eDecay: {
                float decay_samples = decay_time_ * sample_rate_;
                stage_progress_ += 1.0f;
                float decay_progress = stage_progress_ / decay_samples;
                current_level_ = 1.0f - decay_progress * (1.0f - sustain_level_);

                if (decay_progress >= 1.0f) {
                    current_level_ = sustain_level_;
                    current_stage_ = Stage::eSustain;
                    stage_progress_ = 0.0f;
                }
                break;
            }

            case Stage::eSustain:
                current_level_ = sustain_level_;
                break;

            case Stage::eRelease: {
                float release_samples = release_time_ * sample_rate_;
                stage_progress_ += 1.0f;
                float release_progress = stage_progress_ / release_samples;
                current_level_ = sustain_level_ * (1.0f - release_progress);

                if (release_progress >= 1.0f) {
                    current_level_ = 0.0f;
                    current_stage_ = Stage::eIdle;
                    stage_progress_ = 0.0f;
                }
                break;
            }
            }

            sample = current_level_;
        }

        return out.size();
    }

    auto EnvelopeGraphNode::render() -> bool {
        bool changed = false;

        if (ImGuiKnobs::Knob("Attack", &attack_time_, 0.0f, 2.0f, 2.0f, "%.3fs")) {
            changed = true;
        }

        ImGui::SameLine();

        if (ImGuiKnobs::Knob("Decay", &decay_time_, 0.0f, 2.0f, 0.0f, "%.3fs")) {
            changed = true;
        }

        ImGui::SameLine();

        if (ImGuiKnobs::Knob("Sustain", &sustain_level_, 0.0f, 1.0f)) {
            changed = true;
        }

        if (ImGuiKnobs::Knob("Release", &release_time_, 0.0f, 2.0f, 0.0f, "%.3fs")) {
            changed = true;
        }

        return changed;
    }

    auto EnvelopeGraphNode::trigger() -> void {
        current_stage_ = Stage::eAttack;
        stage_progress_ = 0.0f;
    }

    auto EnvelopeGraphNode::release() -> void {
        if (current_stage_ != Stage::eIdle && current_stage_ != Stage::eRelease) {
            current_stage_ = Stage::eRelease;
            stage_progress_ = 0.0f;
        }
    }

    auto EnvelopeGraphNode::to_json() const -> nlohmann::json {
        nlohmann::json j;
        j["attack_time"] = attack_time_;
        j["decay_time"] = decay_time_;
        j["sustain_level"] = sustain_level_;
        j["release_time"] = release_time_;
        j["current_stage"] = static_cast<int>(current_stage_);
        j["current_level"] = current_level_;
        return j;
    }

    auto EnvelopeGraphNode::from_json(const nlohmann::json& json) -> void {
        if (json.contains("attack_time")) {
            attack_time_ = json["attack_time"];
        }
        if (json.contains("decay_time")) {
            decay_time_ = json["decay_time"];
        }
        if (json.contains("sustain_level")) {
            sustain_level_ = json["sustain_level"];
        }
        if (json.contains("release_time")) {
            release_time_ = json["release_time"];
        }
        if (json.contains("current_stage")) {
            current_stage_ = static_cast<Stage>(json["current_stage"].get<int>());
        }
        if (json.contains("current_level")) {
            current_level_ = json["current_level"];
        }
    }


    MixerGraphNode::MixerGraphNode() {
        input_sockets_.emplace_back(id_, 0, NodeSocket::Type::eInput, "Input 0");
        input_gains_.emplace_back(1.0f);
        input_sockets_.emplace_back(id_, 1, NodeSocket::Type::eInput, "Input 1");
        input_gains_.emplace_back(1.0f);
        output_sockets_.emplace_back(id_, 0, NodeSocket::Type::eOutput, "Mixed");
    }

    auto MixerGraphNode::process(std::span<uint8_t> out, const AudioFormat& format,
        const std::vector<std::span<const uint8_t>>& inputs) -> size_t {
        std::fill(out.begin(), out.end(), 0);

        auto float_out = std::span<float>((float*)out.data(), out.size() / sizeof(float));

        // Process each input that's connected
        for (size_t input_idx = 0; input_idx < inputs.size() && input_idx < input_gains_.size(); ++input_idx) {
            auto input_span = inputs[input_idx];

            // Skip empty inputs
            if (input_span.empty()) {
                continue;
            }

            auto float_input = std::span<const float>((const float*)input_span.data(),
                input_span.size() / sizeof(float));

            float gain = input_gains_[input_idx];

            for (size_t i = 0; i < std::min(float_out.size(), float_input.size()); ++i) {
                float_out[i] += float_input[i] * gain;
            }
        }

        // Apply master gain
        for (auto& sample : float_out) {
            sample *= master_gain_;
        }

        return out.size();
    }

    auto MixerGraphNode::render() -> bool {
        bool changed = false;

        if (ImGuiKnobs::Knob("Master", &master_gain_, 0.0f, 5.0f)) {
            changed = true;
        }

        for (int32_t i = 0; i < input_gains_.size(); ++i) {
            auto formatted = std::format("Channel {}", i);
            if (ImGui::DragFloat(formatted.c_str(), input_gains_.data() + i, 0.1f, 0.0f, 5.0f)) {
                changed = true;
            }
        }

        if (ImGui::Button("Add")) {
            input_sockets_.emplace_back(id_, input_gains_.size(), NodeSocket::Type::eInput, std::format("Input {}", input_gains_.size()));
            input_gains_.emplace_back(1.0f);
        }

        if (input_gains_.size() > 2) {
            ImGui::SameLine();
            if (ImGui::Button("Remove")) {
                input_gains_.pop_back();
                input_sockets_.pop_back();
            }
        }

        return changed;
    }

    auto MixerGraphNode::to_json() const -> nlohmann::json {
        nlohmann::json j;
        j["input_gains"] = input_gains_;
        j["master_gain"] = master_gain_;
        return j;
    }

    auto MixerGraphNode::from_json(const nlohmann::json& json) -> void {
        if (json.contains("input_gains")) {
            //input_gains_ = json["input_gains"];
        }
        if (json.contains("master_gain")) {
            master_gain_ = json["master_gain"];
        }
    }

    auto MixerGraphNode::set_input_gain(size_t input_index, float gain) -> void {
        if (input_index < input_gains_.size()) {
            input_gains_[input_index] = gain;
        }
    }

    auto MixerGraphNode::get_input_gain(size_t input_index) const -> float {
        return (input_index < input_gains_.size()) ? input_gains_[input_index] : 0.0f;
    }

    // Register all node types
    //REGISTER_AUDIO_NODE(OutputGraphNode);
    REGISTER_AUDIO_NODE(OscillatorGraphNode);
    REGISTER_AUDIO_NODE(FileInputGraphNode);
    REGISTER_AUDIO_NODE(BiquadFilterGraphNode);
    REGISTER_AUDIO_NODE(EqualizerGraphNode);
    REGISTER_AUDIO_NODE(GainGraphNode);
    REGISTER_AUDIO_NODE(DelayGraphNode);
    REGISTER_AUDIO_NODE(ReverbGraphNode);
    REGISTER_AUDIO_NODE(CompressorGraphNode);
    REGISTER_AUDIO_NODE(NoiseGraphNode);
    REGISTER_AUDIO_NODE(LFOGraphNode);
    REGISTER_AUDIO_NODE(EnvelopeGraphNode);
    REGISTER_AUDIO_NODE(MixerGraphNode);
}