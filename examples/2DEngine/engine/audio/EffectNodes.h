#pragma once

#include "EffectGraph.h"
#include "DSP.h"
#include "IAudioFile.h"
#include "Common.h"

#include <memory>
#include <vector>

namespace e2d::audio {
    class OutputGraphNode : public EffectGraphNode {
    public:
        OutputGraphNode();
        ~OutputGraphNode() override = default;

        auto get_type_name() const -> std::string_view override { return "OutputNode"; }

        auto process(std::span<uint8_t> out, const AudioFormat& format,
            const std::vector<std::span<const uint8_t>>& inputs) -> size_t override;

        auto render() -> bool override;

        auto to_json() const->nlohmann::json override;
        auto from_json(const nlohmann::json& json) -> void override;

        auto set_master_volume(float volume) -> void { master_volume_ = volume; }
        auto get_master_volume() const -> float { return master_volume_; }

    private:
        float master_volume_{ 1.0f };        
    };



    class OscillatorGraphNode : public EffectGraphNode {
    public:
        OscillatorGraphNode();
        ~OscillatorGraphNode() override = default;

        auto get_type_name() const -> std::string_view override { return "OscillatorNode"; }

        auto process(std::span<uint8_t> out, const AudioFormat& format,
            const std::vector<std::span<const uint8_t>>& inputs) -> size_t override;

        auto render() -> bool override;

        auto to_json() const->nlohmann::json override;
        auto from_json(const nlohmann::json& json) -> void override;

        // Parameters
        auto set_wave_type(WaveType type) noexcept -> void { wave_type_ = type; }
        auto set_frequency(float freq) noexcept -> void { frequency_ = freq; }
        auto set_amplitude(float amp) noexcept -> void { amplitude_ = amp; }

        auto get_wave_type() const noexcept -> WaveType { return wave_type_; }
        auto get_frequency() const noexcept -> float { return frequency_; }
        auto get_amplitude() const noexcept -> float { return amplitude_; }

    private:
        WaveType wave_type_{ WaveType::eSine };
        float frequency_{ 440.0f };
        float amplitude_{ 1.0f };
        float phase_{ 0.0f };
        float start_phase_{ 0.0f };
    };



    class FileInputGraphNode : public EffectGraphNode {
    public:
        FileInputGraphNode();
        ~FileInputGraphNode() override = default;

        auto get_type_name() const -> std::string_view override { return "FileInputNode"; }

        auto process(std::span<uint8_t> out, const AudioFormat& format,
            const std::vector<std::span<const uint8_t>>& inputs) -> size_t override;

        auto render() -> bool override;

        auto to_json() const->nlohmann::json override;
        auto from_json(const nlohmann::json& json) -> void override;

        auto load_file(const std::string& path) -> bool;
        auto set_loop(bool loop) -> void { loop_ = loop; }
        auto get_file_path() const -> const std::string& { return file_path_; }
        auto is_looping() const -> bool { return loop_; }

    private:
        std::unique_ptr<IAudioFile> file_;
        std::vector<uint8_t> file_bytes_;
        std::string file_path_;
        bool loop_{ false };
    };



    class BiquadFilterGraphNode : public EffectGraphNode {
    public:
        BiquadFilterGraphNode();
        ~BiquadFilterGraphNode() override = default;

        auto get_type_name() const -> std::string_view override { return "BiquadFilterNode"; }

        auto process(std::span<uint8_t> out, const AudioFormat& format,
            const std::vector<std::span<const uint8_t>>& inputs) -> size_t override;

        auto render() -> bool override;

        auto to_json() const->nlohmann::json override;
        auto from_json(const nlohmann::json& json) -> void override;

        auto set_filter_type(BiquadFilter::Mode type) -> void { filter_.set_mode(type); }
        auto set_frequency(float freq) -> void { filter_.set_frequency(freq); }
        auto set_q(float q) -> void { filter_.set_q(q); }
        auto set_gain(float gain) -> void { filter_.set_gain(gain); }

        auto get_filter() -> BiquadFilter& { return filter_; }
        auto get_filter() const -> const BiquadFilter& { return filter_; }

    private:
        BiquadFilter filter_;
    };



    class EqualizerGraphNode : public EffectGraphNode {
    public:
        EqualizerGraphNode();
        ~EqualizerGraphNode() override = default;

        auto get_type_name() const -> std::string_view override { return "EqualizerNode"; }

        auto process(std::span<uint8_t> out, const AudioFormat& format,
            const std::vector<std::span<const uint8_t>>& inputs) -> size_t override;

        auto render() -> bool override;

        auto to_json() const->nlohmann::json override;
        auto from_json(const nlohmann::json& json) -> void override;

        auto get_filter() -> EqualizerFilter& { return filter_; }
        auto get_filter() const -> const EqualizerFilter& { return filter_; }

    private:
        EqualizerFilter filter_;
    };

    class GainGraphNode : public EffectGraphNode {
    public:
        GainGraphNode();
        ~GainGraphNode() override = default;

        auto get_type_name() const -> std::string_view override { return "GainNode"; }

        auto process(std::span<uint8_t> out, const AudioFormat& format,
            const std::vector<std::span<const uint8_t>>& inputs) -> size_t override;

        auto render() -> bool override;

        auto to_json() const->nlohmann::json override;
        auto from_json(const nlohmann::json& json) -> void override;

        auto set_gain(float gain) -> void { gain_ = gain; }
        auto get_gain() const -> float { return gain_; }

    private:
        float gain_{ 1.0f };
    };

    class DelayGraphNode : public EffectGraphNode {
    public:
        DelayGraphNode();
        ~DelayGraphNode() override = default;

        auto get_type_name() const -> std::string_view override { return "DelayNode"; }

        auto process(std::span<uint8_t> out, const AudioFormat& format,
            const std::vector<std::span<const uint8_t>>& inputs) -> size_t override;

        auto render() -> bool override;

        auto to_json() const->nlohmann::json override;
        auto from_json(const nlohmann::json& json) -> void override;

        auto set_delay_time(float seconds) -> void { delay_time_ = seconds; }
        auto set_feedback(float feedback) -> void { delay_.feedback_ = feedback; }
        auto set_mix(float mix) -> void { delay_.mix_ = mix; }

        auto get_delay_time() const -> float { return delay_time_; }
        auto get_feedback() const -> float { return delay_.feedback_; }
        auto get_mix() const -> float { return delay_.mix_; }

    private:
        DelayLine delay_;
        float delay_time_{ 0.25f }; // seconds
        uint32_t last_sample_rate_{ 44100 };
    };

    class ReverbGraphNode : public EffectGraphNode {
    public:
        ReverbGraphNode();
        ~ReverbGraphNode() override = default;

        auto get_type_name() const -> std::string_view override { return "ReverbNode"; }

        auto process(std::span<uint8_t> out, const AudioFormat& format,
            const std::vector<std::span<const uint8_t>>& inputs) -> size_t override;

        auto render() -> bool override;

        auto to_json() const->nlohmann::json override;
        auto from_json(const nlohmann::json& json) -> void override;

        auto get_reverb() -> ReverbEffect& { return reverb_; }
        auto get_reverb() const -> const ReverbEffect& { return reverb_; }

    private:
        ReverbEffect reverb_;
    };

    class CompressorGraphNode : public EffectGraphNode {
    public:
        CompressorGraphNode();
        ~CompressorGraphNode() override = default;

        auto get_type_name() const -> std::string_view override { return "CompressorNode"; }

        auto process(std::span<uint8_t> out, const AudioFormat& format,
            const std::vector<std::span<const uint8_t>>& inputs) -> size_t override;

        auto render() -> bool override;

        auto to_json() const->nlohmann::json override;
        auto from_json(const nlohmann::json& json) -> void override;

        auto get_compressor() -> CompressorEffect& { return compressor_; }
        auto get_compressor() const -> const CompressorEffect& { return compressor_; }

    private:
        CompressorEffect compressor_;
    };

    class NoiseGraphNode : public EffectGraphNode {
    public:
        NoiseGraphNode();
        ~NoiseGraphNode() override = default;

        auto get_type_name() const -> std::string_view override { return "NoiseNode"; }

        auto process(std::span<uint8_t> out, const AudioFormat& format,
            const std::vector<std::span<const uint8_t>>& inputs) -> size_t override;

        auto render() -> bool override;

        auto to_json() const->nlohmann::json override;
        auto from_json(const nlohmann::json& json) -> void override;

        auto set_noise_type(NoiseGenerator::Type type) -> void { generator_.type_ = type; }
        auto set_amplitude(float amplitude) -> void { generator_.amplitude_ = amplitude; }

        auto get_noise_type() const -> NoiseGenerator::Type { return generator_.type_; }
        auto get_amplitude() const -> float { return generator_.amplitude_; }

    private:
        NoiseGenerator generator_;
    };

    class LFOGraphNode : public EffectGraphNode {
    public:
        LFOGraphNode();
        ~LFOGraphNode() override = default;

        auto get_type_name() const -> std::string_view override { return "LFONode"; }

        auto process(std::span<uint8_t> out, const AudioFormat& format,
            const std::vector<std::span<const uint8_t>>& inputs) -> size_t override;

        auto render() -> bool override;

        auto to_json() const->nlohmann::json override;
        auto from_json(const nlohmann::json& json) -> void override;

        auto set_frequency(float freq) -> void { frequency_ = freq; }
        auto set_amplitude(float amp) -> void { amplitude_ = amp; }
        auto set_wave_type(WaveType type) -> void { wave_type_ = type; }

        auto get_frequency() const -> float { return frequency_; }
        auto get_amplitude() const -> float { return amplitude_; }
        auto get_wave_type() const -> WaveType { return wave_type_; }

    private:
        float frequency_{ 1.0f }; // Hz - much lower than audio oscillator
        float amplitude_{ 1.0f };
        WaveType wave_type_{ WaveType::eSine };
        float phase_{ 0.0f };
    };

    class EnvelopeGraphNode : public EffectGraphNode {
    public:
        enum class Stage {
            eIdle,
            eAttack,
            eDecay,
            eSustain,
            eRelease
        };

        EnvelopeGraphNode();
        ~EnvelopeGraphNode() override = default;

        auto get_type_name() const -> std::string_view override { return "EnvelopeNode"; }

        auto process(std::span<uint8_t> out, const AudioFormat& format,
            const std::vector<std::span<const uint8_t>>& inputs) -> size_t override;

        auto render() -> bool override;

        auto to_json() const->nlohmann::json override;
        auto from_json(const nlohmann::json& json) -> void override;

        auto trigger() -> void;
        auto release() -> void;

        auto set_attack(float seconds) -> void { attack_time_ = seconds; }
        auto set_decay(float seconds) -> void { decay_time_ = seconds; }
        auto set_sustain(float level) -> void { sustain_level_ = level; }
        auto set_release(float seconds) -> void { release_time_ = seconds; }

        auto get_attack() const -> float { return attack_time_; }
        auto get_decay() const -> float { return decay_time_; }
        auto get_sustain() const -> float { return sustain_level_; }
        auto get_release() const -> float { return release_time_; }

    private:
        float attack_time_{ 0.1f };
        float decay_time_{ 0.2f };
        float sustain_level_{ 0.7f };
        float release_time_{ 0.5f };

        Stage current_stage_{ Stage::eIdle };
        float current_level_{ 0.0f };
        float stage_progress_{ 0.0f };
        uint32_t sample_rate_{ 44100 };
    };

    class MixerGraphNode : public EffectGraphNode {
    public:
        MixerGraphNode();
        ~MixerGraphNode() override = default;

        auto get_type_name() const -> std::string_view override { return "MixerNode"; }

        auto process(std::span<uint8_t> out, const AudioFormat& format,
            const std::vector<std::span<const uint8_t>>& inputs) -> size_t override;

        auto render() -> bool override;

        auto to_json() const->nlohmann::json override;
        auto from_json(const nlohmann::json& json) -> void override;

        auto set_input_gain(size_t input_index, float gain) -> void;
        auto get_input_gain(size_t input_index) const -> float;
        auto set_master_gain(float gain) -> void { master_gain_ = gain; }
        auto get_master_gain() const -> float { return master_gain_; }

    private:
        std::vector<float> input_gains_;
        float master_gain_{ 1.0f };
    };
}