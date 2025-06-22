#pragma once

#include <array>
#include <algorithm>

#include "Util.h"

namespace e2d::audio {
    struct AllPassFilter {
		float delay_;
		float feedback_;
		float a1_;
		float x1_, y1_;

		AllPassFilter() : delay_(0.0f), feedback_(0.0f), a1_(0.0f), x1_(0.0f), y1_(0.0f) {}

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
        float b0_, b1_, b2_;
        // feedback coefficients (a0 is normalized to 1)
        float a1_, a2_;

        // input history
        float x1_, x2_;
        // output history
        float y1_, y2_;

        float cutoff_freq_;
        float q_factor_;
        float gain_db_;
        uint32_t last_sample_rate_;
        Mode mode_;

        BiquadFilter()
            : b0_(1.0f), b1_(0.0f), b2_(0.0f)
            , a1_(0.0f), a2_(0.0f)
            , x1_(0.0f), x2_(0.0f)
            , y1_(0.0f), y2_(0.0f)
            , cutoff_freq_(1000.0f)
            , gain_db_(0.0f)
            , q_factor_(0.707f)
            , last_sample_rate_(0)
            , mode_(Mode::eLowPass) {
            dirty_ = true;
            update_coefficients();
        }

        BiquadFilter(float cutoff, float gain_db, float q, uint32_t sample_rate, Mode mode)
            : b0_(1.0f), b1_(0.0f), b2_(0.0f)
            , a1_(0.0f), a2_(0.0f)
            , x1_(0.0f), x2_(0.0f)
            , y1_(0.0f), y2_(0.0f)
            , cutoff_freq_(cutoff)
            , gain_db_(gain_db)
            , q_factor_(q)
            , last_sample_rate_(sample_rate)
            , mode_(mode) {
            dirty_ = true;
            update_coefficients();
        }

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
        bool dirty_{ false };
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

    template<size_t NUM_STAGES = 6>
    struct PhaserFilter {
        std::array<AllPassFilter, NUM_STAGES> all_pass_filters_;

        // Phaser parameters
        float rate_;           // LFO rate in Hz
        float depth_;          // Modulation depth (0.0 to 1.0)
        float feedback_;       // Amount of output fed back to input
        float mix_;            // Dry/wet mix (0.0 = dry, 1.0 = wet)
        float center_freq_;    // Center frequency for modulation
        float freq_range_;     // Frequency range for modulation

        // LFO state
        float lfo_phase_;
        uint32_t last_sample_rate_;
        float lfo_phase_increment_;

        PhaserFilter(float rate = 0.5f, float depth = 0.75f, float feedback = 0.3f, float mix = 0.5f, uint32_t sample_rate = 44100u)
            : rate_(rate)
            , depth_(depth)
            , feedback_(feedback)
            , mix_(mix)
            , center_freq_(1000.0f)
            , freq_range_(800.0f)
            , lfo_phase_(0.0f)
            , last_sample_rate_(0)
            , lfo_phase_increment_(0.0f) {
            set_sample_rate(sample_rate);
        }

        auto process(float input) -> float {
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
                lfo_phase_increment_ = 2.0f * std::numbers::pi_v<float> *rate_ / static_cast<float>(sample_rate);
            }
        }

        auto set_rate(float rate) -> void { rate_ = rate; last_sample_rate_ = 0; }
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
    };

    template<size_t N_BANDS>
    struct EqualizerFilter {
        std::array<BiquadFilter, N_BANDS> eq_bands_;

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

        auto reset() -> void {
            for (auto& band : eq_bands_) {
                band.reset();
            }
        }
    };

    struct EqualizerFilter5Band : EqualizerFilter<5> {

        EqualizerFilter5Band(uint32_t sample_rate = 44100u) {
            eq_bands_[0] = { 80.0f, 0.0f, 0.707f, sample_rate, BiquadFilter::Mode::eLowShelf };
            eq_bands_[1] = { 250.0f, 0.0f, 1.0f, sample_rate, BiquadFilter::Mode::eBell };
            eq_bands_[2] = { 1000.0f, 0.0f, 1.0f, sample_rate, BiquadFilter::Mode::eBell };
            eq_bands_[3] = { 4000.0f, 0.0f, 1.0f, sample_rate, BiquadFilter::Mode::eBell };
            eq_bands_[4] = { 12000.0f, 0.0f, 0.707f, sample_rate, BiquadFilter::Mode::eHighShelf };
        }
    };

    struct EqualizerFilter10Band : EqualizerFilter<10> {

        EqualizerFilter10Band(uint32_t sample_rate = 44100u) {
            eq_bands_[0] = { 31.5f, 0.0f, 1.414f, sample_rate, BiquadFilter::Mode::eLowShelf };
            eq_bands_[1] = { 63.0f, 0.0f, 1.414f, sample_rate, BiquadFilter::Mode::eBell };
            eq_bands_[2] = { 125.0f, 0.0f, 1.414f, sample_rate, BiquadFilter::Mode::eBell };
            eq_bands_[3] = { 250.0f, 0.0f, 1.414f, sample_rate, BiquadFilter::Mode::eBell };
            eq_bands_[4] = { 500.0f, 0.0f, 1.414f, sample_rate, BiquadFilter::Mode::eBell };
            eq_bands_[5] = { 1000.0f, 0.0f, 1.414f, sample_rate, BiquadFilter::Mode::eBell };
            eq_bands_[6] = { 2000.0f, 0.0f, 1.414f, sample_rate, BiquadFilter::Mode::eBell };
            eq_bands_[7] = { 4000.0f, 0.0f, 1.414f, sample_rate, BiquadFilter::Mode::eBell };
            eq_bands_[8] = { 8000.0f, 0.0f, 1.414f, sample_rate, BiquadFilter::Mode::eBell };
            eq_bands_[9] = { 16000.0f, 0.0f, 1.414f, sample_rate, BiquadFilter::Mode::eHighShelf };
        }
    };
}