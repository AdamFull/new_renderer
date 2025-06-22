#pragma once

#include <array>
#include <algorithm>
#include <vector>
#include <random>

#include <edge/util/Serialization.hpp>

#include "Util.h"

namespace e2d::audio {
    struct AllPassFilter {
        float delay_{ 0.0f };
        float feedback_{ 0.0f };
        float a1_{ 0.0f };
        float x1_{ 0.0f }, y1_{ 0.0f };

        AllPassFilter() = default;

        auto set_delay(float delay_samples) noexcept -> void {
            delay_ = delay_samples;
            a1_ = (1.0f - delay_) / (1.0f + delay_);
        }

        auto process(float input) noexcept -> float {
            float output = -a1_ * input + x1_ + a1_ * y1_;
            x1_ = input;
            y1_ = output;
            return output;
        }

        void reset() {
            x1_ = y1_ = 0.0f;
        }
	};

    inline auto to_json(nlohmann::json& j, const AllPassFilter& self) -> void {
        j["delay"] = self.delay_;
        j["feedback"] = self.feedback_;
    }

    inline auto from_json(const nlohmann::json& j, AllPassFilter& self) -> void {
        edge::parse_from("delay", j, self.delay_);
        edge::parse_from("feedback", j, self.feedback_);
    }

    struct BiquadFilter {
        enum class Mode {
            eLowPass,
            eBandPass,
            eHighPass,
            eAllPass,
            eNotch,
            eBell,
            eLowShelf,
            eHighShelf
        };

        // feedforward coefficients
        float b0_{ 1.0f }, b1_{ 0.0f }, b2_{ 0.0f };
        // feedback coefficients (a0 is normalized to 1)
        float a1_{ 0.0f }, a2_{ 0.0f };

        // input history
        float x1_{ 0.0f }, x2_{ 0.0f };
        // output history
        float y1_{ 0.0f }, y2_{ 0.0f };

        float cutoff_freq_{ 1000.0f };
        float q_factor_{ 0.707f };
        float gain_db_{ 1.0f };
        uint32_t last_sample_rate_{ 44100 };
        Mode mode_{ Mode::eLowPass };

        BiquadFilter() {
            update_coefficients();
        }

        BiquadFilter(float cutoff, float gain_db, float q, uint32_t sample_rate, Mode mode)
            : cutoff_freq_(cutoff)
            , gain_db_(gain_db)
            , q_factor_(q)
            , last_sample_rate_(sample_rate)
            , mode_(mode) {}

        auto process(float input) -> float {
            update_coefficients();

            const float output = b0_ * input + b1_ * x1_ + b2_ * x2_ - a1_ * y1_ - a2_ * y2_;

            x2_ = x1_;
            x1_ = input;
            y2_ = y1_;
            y1_ = output;

            return output;
        }

        auto set_mode(Mode mode) {
            if (mode != mode_) {
                mode_ = mode;
                dirty_ = true;
            }
        }

        auto set_sample_rate(uint32_t sample_rate) -> void {
            if (sample_rate != last_sample_rate_) {
                last_sample_rate_ = sample_rate;
                dirty_ = true;
            }
        }

        auto set_gain(float gain_db) -> void {
            if (gain_db != gain_db_) {
                gain_db_ = gain_db;
                dirty_ = true;
            }
        }

        auto set_frequency(float frequency) -> void {
            if (frequency != cutoff_freq_) {
                cutoff_freq_ = frequency;
                dirty_ = true;
            }
        }

        auto set_q(float q) -> void {
            if (q != q_factor_) {
                q_factor_ = q;
                dirty_ = true;
            }
        }

        void reset() {
            x1_ = x2_ = y1_ = y2_ = 0.0f;
        }
    private:
        bool dirty_{ true };
        auto update_coefficients() -> void {
            if (!dirty_) {
                return;
            }
            dirty_ = false;

            const float omega = math::two_pi * cutoff_freq_ / static_cast<float>(last_sample_rate_);
            const float cos_omega = std::cos(omega);
            const float sin_omega = std::sin(omega);
            const float A = std::pow(10.0f, gain_db_ / 40.0f); // Linear gain
            const float alpha = sin_omega / (2.0f * q_factor_);
            const float beta = std::sqrt(A) / q_factor_;

            float a0, a1, a2, b0, b1, b2;

            switch (mode_) {
            case Mode::eLowPass:
                b1 = 1.0f - cos_omega;
                b0 = b2 = b1 * 0.5f;
                a0 = 1.0f + alpha;
                a1 = -2.0f * cos_omega;
                a2 = 1.0f - alpha;
                break;

            case Mode::eHighPass:
                b0 = b2 = (1.0f + cos_omega) * 0.5f;
                b1 = -(1.0f + cos_omega);
                a0 = 1.0f + alpha;
                a1 = -2.0f * cos_omega;
                a2 = 1.0f - alpha;
                break;

            case Mode::eBandPass:
                b0 = sin_omega * 0.5f;
                b1 = 0.0f;
                b2 = -sin_omega * 0.5f;
                a0 = 1.0f + alpha;
                a1 = -2.0f * cos_omega;
                a2 = 1.0f - alpha;
                break;

            case Mode::eNotch:
                b0 = 1.0f;
                b1 = -2.0f * cos_omega;
                b2 = 1.0f;
                a0 = 1.0f + alpha;
                a1 = -2.0f * cos_omega;
                a2 = 1.0f - alpha;
                break;

            case Mode::eAllPass:
                b0 = 1.0f - alpha;
                b1 = -2.0f * cos_omega;
                b2 = 1.0f + alpha;
                a0 = 1.0f + alpha;
                a1 = -2.0f * cos_omega;
                a2 = 1.0f - alpha;
                break;

            case Mode::eBell:
                b0 = 1.0f + alpha * A;
                b1 = -2.0f * cos_omega;
                b2 = 1.0f - alpha * A;
                a0 = 1.0f + alpha / A;
                a1 = -2.0f * cos_omega;
                a2 = 1.0f - alpha / A;
                break;

            case Mode::eLowShelf:
                b0 = A * ((A + 1.0f) - (A - 1.0f) * cos_omega + beta * sin_omega);
                b1 = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cos_omega);
                b2 = A * ((A + 1.0f) - (A - 1.0f) * cos_omega - beta * sin_omega);
                a0 = (A + 1.0f) + (A - 1.0f) * cos_omega + beta * sin_omega;
                a1 = -2.0f * ((A - 1.0f) + (A + 1.0f) * cos_omega);
                a2 = (A + 1.0f) + (A - 1.0f) * cos_omega - beta * sin_omega;
                break;

            case Mode::eHighShelf:
                b0 = A * ((A + 1.0f) + (A - 1.0f) * cos_omega + beta * sin_omega);
                b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cos_omega);
                b2 = A * ((A + 1.0f) + (A - 1.0f) * cos_omega - beta * sin_omega);
                a0 = (A + 1.0f) - (A - 1.0f) * cos_omega + beta * sin_omega;
                a1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * cos_omega);
                a2 = (A + 1.0f) - (A - 1.0f) * cos_omega - beta * sin_omega;
                break;
            }

            // Normalize coefficients by a0
            b0_ = b0 / a0;
            b1_ = b1 / a0;
            b2_ = b2 / a0;
            a1_ = a1 / a0;
            a2_ = a2 / a0;
        }
    };

    inline auto to_json(nlohmann::json& j, const BiquadFilter& self) -> void {
        j["cutoff_frequency"] = self.cutoff_freq_;
        j["q"] = self.q_factor_;
        j["gain_db"] = self.gain_db_;
        j["sample_rate"] = self.last_sample_rate_;
        j["mode"] = static_cast<int32_t>(self.mode_);
    }

    inline auto from_json(const nlohmann::json& j, BiquadFilter& self) -> void {
        edge::parse_from("cutoff_frequency", j, self.cutoff_freq_);
        edge::parse_from("q", j, self.q_factor_);
        edge::parse_from("gain_db", j, self.gain_db_);
        edge::parse_from("sample_rate", j, self.last_sample_rate_);

        int32_t mode{ 0 };
        edge::parse_from("mode", j, mode);
        self.mode_ = static_cast<BiquadFilter::Mode>(mode);
    }

    struct PhaserFilter {
        static constexpr size_t NUM_STAGES = 12;
        std::array<AllPassFilter, NUM_STAGES> all_pass_filters_;

        // Phaser parameters
        float rate_{ 0.5f };            // LFO rate in Hz
        float depth_{ 0.75f };          // Modulation depth (0.0 to 1.0)
        float feedback_{ 0.3f };        // Amount of output fed back to input
        float mix_{ 0.5f };             // Dry/wet mix (0.0 = dry, 1.0 = wet)
        float center_freq_{ 1000.0f };  // Center frequency for modulation
        float freq_range_{ 800.0f };    // Frequency range for modulation

        // LFO state
        float lfo_phase_{ 0.0f };
        uint32_t last_sample_rate_{ 44100u };
        float lfo_phase_increment_{ 0.0f };

        bool dirty_{ true };

        PhaserFilter(float rate = 0.5f, float depth = 0.75f, float feedback = 0.3f, float mix = 0.5f, uint32_t sample_rate = 44100u)
            : rate_(rate)
            , depth_(depth)
            , feedback_(feedback)
            , mix_(mix)
            , last_sample_rate_(sample_rate) {}

        auto process(float input) -> float {
            update_lfo();

            // Generate LFO (sine wave)
            float lfo_value = std::sin(lfo_phase_);
            lfo_phase_ += lfo_phase_increment_;
            if (lfo_phase_ >= 2.0f * math::pi) {
                lfo_phase_ -= 2.0f * math::pi;
            }

            // Calculate modulated delay
            float modulated_freq = center_freq_ + lfo_value * depth_ * freq_range_;
            float delay_samples = static_cast<float>(last_sample_rate_) / (modulated_freq * 2.0f);
            delay_samples = std::clamp(delay_samples, 0.1f, 10.0f);

            // Update all-pass filters with modulated delay
            for (auto& filter : all_pass_filters_) {
                filter.set_delay(delay_samples);
            }

            // Process through all-pass chain
            float processed = input;
            for (auto& filter : all_pass_filters_) {
                processed = filter.process(processed);
            }

            // Apply feedback
            processed = input + feedback_ * processed;

            // Mix dry and wet signals
            return (1.0f - mix_) * input + mix_ * processed;
        }

        auto set_sample_rate(uint32_t sample_rate) {
            if (sample_rate != last_sample_rate_) {
                last_sample_rate_ = sample_rate;
                dirty_ = true;
            }
        }

        auto set_rate(float rate) -> void  { 
            if (rate != rate_) {
                rate_ = rate;
                dirty_ = true;
            }
        }

        auto set_depth(float depth) -> void { depth_ = std::clamp(depth, 0.0f, 1.0f); }
        auto set_feedback(float feedback) -> void { feedback_ = std::clamp(feedback, -0.95f, 0.95f); }
        auto set_mix(float mix) -> void { mix_ = std::clamp(mix, 0.0f, 1.0f); }
        auto set_center_frequency(float freq) -> void { center_freq_ = freq; }
        auto set_frequency_range(float range) -> void { freq_range_ = range; }

        auto reset() -> void {
            lfo_phase_ = 0.0f;
            for (auto& filter : all_pass_filters_) {
                filter.reset();
            }
        }
    private:
        auto update_lfo() -> void {
            if (!dirty_) {
                return;
            }
            dirty_ = false;

            lfo_phase_increment_ = math::two_pi * rate_ / static_cast<float>(last_sample_rate_);
        }
    };

    inline auto to_json(nlohmann::json& j, const PhaserFilter& self) -> void {
        j["rate"] = self.rate_;
        j["depth"] = self.depth_;
        j["feedback"] = self.feedback_;
        j["mix"] = self.mix_;
        j["center_frequency"] = self.center_freq_;
        j["frequency_range"] = self.freq_range_;
        j["sample_rate"] = self.last_sample_rate_;
    }

    inline auto from_json(const nlohmann::json& j, PhaserFilter& self) -> void {
        edge::parse_from("rate", j, self.rate_);
        edge::parse_from("depth", j, self.depth_);
        edge::parse_from("feedback", j, self.feedback_);
        edge::parse_from("mix", j, self.mix_);
        edge::parse_from("center_frequency", j, self.center_freq_);
        edge::parse_from("frequency_range", j, self.freq_range_);
        edge::parse_from("sample_rate", j, self.last_sample_rate_);
    }

    struct EqualizerFilter {
        enum class Type {
            e5Band,
            e10Band
        };

        Type type_{ Type::e10Band };
        uint32_t last_sample_rate_{ 44100u };
        std::vector<BiquadFilter> eq_bands_;

        EqualizerFilter(Type type = Type::e10Band, uint32_t sample_rate = 44100u) 
            : type_{ type }
            , last_sample_rate_{ sample_rate } {
            create_bands();
        }

        auto process(float input) -> float {
            for (auto& band : eq_bands_) {
                input = band.process(input);
            }
            return input;
        }

        auto set_band_gain(size_t band_index, float gain_db) -> void {
            if (band_index < eq_bands_.size()) {
                eq_bands_[band_index].set_gain(gain_db);
            }
        }

        auto set_band_frequency(size_t band_index, float frequency) -> void {
            if (band_index < eq_bands_.size()) {
                eq_bands_[band_index].set_frequency(frequency);
            }
        }

        auto set_band_q(size_t band_index, float q) -> void {
            if (band_index < eq_bands_.size()) {
                eq_bands_[band_index].set_q(q);
            }
        }

        auto get_band_count() const -> size_t {
            return eq_bands_.size();
        }

        auto set_sample_rate(uint32_t sample_rate) -> void {
            if (sample_rate != last_sample_rate_) {
                last_sample_rate_ = sample_rate;

                for (auto& band : eq_bands_) {
                    band.set_sample_rate(last_sample_rate_);
                }
            }
        }

        auto reset() -> void {
            for (auto& band : eq_bands_) {
                band.reset();
            }
        }

        auto create_bands() -> void {
            eq_bands_.clear();

            switch (type_)
            {
            case Type::e5Band: {
                eq_bands_.reserve(5);
                eq_bands_.emplace_back(80.0f, 0.0f, 0.707f, last_sample_rate_, BiquadFilter::Mode::eLowShelf);
                eq_bands_.emplace_back(250.0f, 0.0f, 1.0f, last_sample_rate_, BiquadFilter::Mode::eBell);
                eq_bands_.emplace_back(1000.0f, 0.0f, 1.0f, last_sample_rate_, BiquadFilter::Mode::eBell);
                eq_bands_.emplace_back(4000.0f, 0.0f, 1.0f, last_sample_rate_, BiquadFilter::Mode::eBell);
                eq_bands_.emplace_back(12000.0f, 0.0f, 0.707f, last_sample_rate_, BiquadFilter::Mode::eHighShelf);
                break;
            }
            case Type::e10Band: {
                eq_bands_.reserve(10);
                eq_bands_.emplace_back(31.5f, 0.0f, 1.414f, last_sample_rate_, BiquadFilter::Mode::eLowShelf);
                eq_bands_.emplace_back(63.0f, 0.0f, 1.414f, last_sample_rate_, BiquadFilter::Mode::eBell);
                eq_bands_.emplace_back(125.0f, 0.0f, 1.414f, last_sample_rate_, BiquadFilter::Mode::eBell);
                eq_bands_.emplace_back(250.0f, 0.0f, 1.414f, last_sample_rate_, BiquadFilter::Mode::eBell);
                eq_bands_.emplace_back(500.0f, 0.0f, 1.414f, last_sample_rate_, BiquadFilter::Mode::eBell);
                eq_bands_.emplace_back(1000.0f, 0.0f, 1.414f, last_sample_rate_, BiquadFilter::Mode::eBell);
                eq_bands_.emplace_back(2000.0f, 0.0f, 1.414f, last_sample_rate_, BiquadFilter::Mode::eBell);
                eq_bands_.emplace_back(4000.0f, 0.0f, 1.414f, last_sample_rate_, BiquadFilter::Mode::eBell);
                eq_bands_.emplace_back(8000.0f, 0.0f, 1.414f, last_sample_rate_, BiquadFilter::Mode::eBell);
                eq_bands_.emplace_back(16000.0f, 0.0f, 1.414f, last_sample_rate_, BiquadFilter::Mode::eHighShelf);
                break;
            }
            default:
                break;
            }
        }
    };

    inline auto to_json(nlohmann::json& j, const EqualizerFilter& self) -> void {
        j["type"] = static_cast<int32_t>(self.type_);
        j["sample_rate"] = static_cast<int32_t>(self.last_sample_rate_);
        j["bands"] = self.eq_bands_;
    }

    inline auto from_json(const nlohmann::json& j, EqualizerFilter& self) -> void {
        int32_t type{ 0 };
        edge::parse_from("type", j, type);
        self.type_ = static_cast<EqualizerFilter::Type>(type);

        edge::parse_from("sample_rate", j, self.last_sample_rate_);
        edge::parse_from("bands", j, self.eq_bands_);
        self.create_bands();
    }


    struct DelayLine {
        std::vector<float> buffer_;
        size_t write_index_{ 0 };
        size_t delay_samples_{ 0 };
        float feedback_{ 0.0f };
        float mix_{ 0.5f };

        DelayLine(size_t max_delay_samples = 88200) : buffer_(max_delay_samples, 0.0f) {}

        auto set_delay_time(float delay_seconds, uint32_t sample_rate) -> void {
            delay_samples_ = static_cast<size_t>(delay_seconds * sample_rate);
            delay_samples_ = std::min(delay_samples_, buffer_.size() - 1);
        }

        auto process(float input) -> float {
            size_t read_index = (write_index_ + buffer_.size() - delay_samples_) % buffer_.size();
            float delayed = buffer_[read_index];

            buffer_[write_index_] = input + delayed * feedback_;
            write_index_ = (write_index_ + 1) % buffer_.size();

            return input * (1.0f - mix_) + delayed * mix_;
        }

        auto reset() -> void {
            std::fill(buffer_.begin(), buffer_.end(), 0.0f);
            write_index_ = 0;
        }
    };

    struct ReverbEffect {
        static constexpr size_t NUM_COMBS = 4;
        static constexpr size_t NUM_ALLPASS = 2;

        std::array<DelayLine, NUM_COMBS> comb_filters_;
        std::array<AllPassFilter, NUM_ALLPASS> allpass_filters_;

        float room_size_{ 0.5f };
        float damping_{ 0.5f };
        float wet_level_{ 0.3f };
        float dry_level_{ 0.7f };

        ReverbEffect(uint32_t sample_rate = 44100) {
            // Initialize comb filter delays (in samples)
            std::array<size_t, NUM_COMBS> comb_delays = {
                static_cast<size_t>(0.030f * sample_rate),  // 30ms
                static_cast<size_t>(0.035f * sample_rate),  // 35ms
                static_cast<size_t>(0.040f * sample_rate),  // 40ms
                static_cast<size_t>(0.045f * sample_rate)   // 45ms
            };

            for (size_t i = 0; i < NUM_COMBS; ++i) {
                comb_filters_[i] = DelayLine(comb_delays[i] + 100);
                comb_filters_[i].delay_samples_ = comb_delays[i];
                comb_filters_[i].feedback_ = 0.7f;
            }

            // Initialize allpass filters
            allpass_filters_[0].set_delay(0.005f * sample_rate);  // 5ms
            allpass_filters_[1].set_delay(0.010f * sample_rate);  // 10ms
        }

        auto process(float input) -> float {
            float comb_out = 0.0f;

            // Process through comb filters
            for (auto& comb : comb_filters_) {
                comb.feedback_ = room_size_ * 0.8f;
                comb_out += comb.process(input);
            }
            comb_out *= 0.25f; // Average the combs

            // Process through allpass filters
            float allpass_out = comb_out;
            for (auto& ap : allpass_filters_) {
                allpass_out = ap.process(allpass_out);
            }

            return input * dry_level_ + allpass_out * wet_level_;
        }

        auto reset() -> void {
            for (auto& comb : comb_filters_) {
                comb.reset();
            }
            for (auto& ap : allpass_filters_) {
                ap.reset();
            }
        }
    };

    struct CompressorEffect {
        float threshold_{ -20.0f };  // dB
        float ratio_{ 4.0f };
        float attack_time_{ 0.003f }; // seconds
        float release_time_{ 0.1f };  // seconds
        float makeup_gain_{ 0.0f };   // dB

        float envelope_{ 0.0f };
        float attack_coeff_{ 0.0f };
        float release_coeff_{ 0.0f };
        uint32_t sample_rate_{ 44100 };

        CompressorEffect(uint32_t sample_rate = 44100) : sample_rate_(sample_rate) {
            update_coefficients();
        }

        auto set_sample_rate(uint32_t sample_rate) -> void {
            sample_rate_ = sample_rate;
            update_coefficients();
        }

        auto update_coefficients() -> void {
            attack_coeff_ = std::exp(-1.0f / (attack_time_ * sample_rate_));
            release_coeff_ = std::exp(-1.0f / (release_time_ * sample_rate_));
        }

        auto process(float input) -> float {
            // Convert to dB
            float input_level = 20.0f * std::log10(std::abs(input) + 1e-6f);

            // Update envelope
            float target_gain = 0.0f;
            if (input_level > threshold_) {
                float over_threshold = input_level - threshold_;
                float compressed_over = over_threshold / ratio_;
                target_gain = -(over_threshold - compressed_over);
            }

            // Smooth gain changes
            float gain_coeff = (target_gain < envelope_) ? attack_coeff_ : release_coeff_;
            envelope_ = target_gain + (envelope_ - target_gain) * gain_coeff;

            // Apply gain
            float gain_linear = std::pow(10.0f, (envelope_ + makeup_gain_) / 20.0f);
            return input * gain_linear;
        }

        auto reset() -> void {
            envelope_ = 0.0f;
        }
    };

    // Noise Generator
    struct NoiseGenerator {
        enum class Type {
            eWhite,
            ePink,
            eBrown
        };

        Type type_{ Type::eWhite };
        float amplitude_{ 1.0f };
        std::mt19937 rng_;
        std::uniform_real_distribution<float> dist_{ -1.0f, 1.0f };

        // For pink/brown noise
        std::array<float, 7> pink_state_{ 0.0f };
        float brown_state_{ 0.0f };

        NoiseGenerator() : rng_(std::random_device{}()) {}

        auto process() -> float {
            switch (type_) {
            case Type::eWhite:
                return dist_(rng_) * amplitude_;

            case Type::ePink: {
                float white = dist_(rng_);
                pink_state_[0] = 0.99886f * pink_state_[0] + white * 0.0555179f;
                pink_state_[1] = 0.99332f * pink_state_[1] + white * 0.0750759f;
                pink_state_[2] = 0.96900f * pink_state_[2] + white * 0.1538520f;
                pink_state_[3] = 0.86650f * pink_state_[3] + white * 0.3104856f;
                pink_state_[4] = 0.55000f * pink_state_[4] + white * 0.5329522f;
                pink_state_[5] = -0.7616f * pink_state_[5] - white * 0.0168980f;
                float pink = pink_state_[0] + pink_state_[1] + pink_state_[2] +
                    pink_state_[3] + pink_state_[4] + pink_state_[5] +
                    pink_state_[6] + white * 0.5362f;
                pink_state_[6] = white * 0.115926f;
                return pink * amplitude_ * 0.11f;
            }

            case Type::eBrown: {
                float white = dist_(rng_);
                brown_state_ = (brown_state_ + white * 0.02f);
                brown_state_ = std::clamp(brown_state_, -8.0f, 8.0f); // Prevent drift
                return brown_state_ * amplitude_ * 0.125f;
            }
            }
            return 0.0f;
        }

        auto reset() -> void {
            std::fill(pink_state_.begin(), pink_state_.end(), 0.0f);
            brown_state_ = 0.0f;
        }
    };
}