export module FileAsset.AudioProcessing;

import std;
import Contract.Audio.Assets;
import Core.Logger;

namespace
{
	constexpr float CubicHermite(float prev, float curr, float next, float next2, float t) noexcept
	{
		float a = (-prev + 3.0f * curr - 3.0f * next + next2) * 0.5f;
		float b = (2.0f * prev - 5.0f * curr + 4.0f * next - next2) * 0.5f;
		float c = (-prev + next) * 0.5f;
		float d = curr;

		return ((a * t + b) * t + c) * t + d;
	}
}

// 레이트를 바꾼다. 인자가 올바르지 않으면 Error를 남기고 빈 결과를 돌려준다
export std::vector<float> ResampleCubic(const float* input, int inFrames, int channels, int inRate, int outRate)
{
	if (inFrames < 0 || channels <= 0 || inRate <= 0 || outRate <= 0 || (inFrames > 0 && !input))
	{
		Core::Logger::Error("ResampleCubic: 올바르지 않은 인자 (frames {}, channels {}, rate {} -> {})", inFrames, channels, inRate, outRate);
		return {};
	}

	double step = static_cast<double>(inRate) / outRate;
	int outFrames = static_cast<int>(std::ceil(inFrames / step));

	std::vector<float> output(outFrames * channels);

	for (int i = 0; i < outFrames; ++i)
	{
		double srcPos = i * step;
		int idx = static_cast<int>(srcPos);
		float t = static_cast<float>(srcPos - idx);

		for (int ch = 0; ch < channels; ++ch)
		{
			auto sample = [&](int frame) {
				if (frame < 0)
					frame = 0;
				else if (frame >= inFrames)
					frame = inFrames - 1;

				return input[frame * channels + ch];
				};

			float p0 = sample(idx - 1);
			float p1 = sample(idx);
			float p2 = sample(idx + 1);
			float p3 = sample(idx + 2);

			output[i * channels + ch] = CubicHermite(p0, p1, p2, p3, t);
		}
	}

	return output;
}

// [-1, 1]을 Int16으로 바꾼다 (반올림, 범위 밖은 잘림, NaN은 무음). 인자가 올바르지 않으면 Error를 남기고 outData를 비운다
export void ConvertFloatToInt16(const float* input, int sampleCount, std::vector<std::uint8_t>& outData)
{
	if (sampleCount < 0 || (sampleCount > 0 && !input))
	{
		Core::Logger::Error("ConvertFloatToInt16: 올바르지 않은 인자 (samples {})", sampleCount);
		outData.clear();
		return;
	}

	outData.resize(sampleCount * sizeof(std::int16_t));

	std::int16_t* out = reinterpret_cast<std::int16_t*>(outData.data());
	for (int i = 0; i < sampleCount; ++i)
	{
		const float v = std::isnan(input[i]) ? 0.0f : std::clamp(input[i], -1.0f, 1.0f);
		out[i] = static_cast<std::int16_t>(std::lround(v * 32767.0f));
	}
}

// 정적 소리는 모두 이 레이트로 맞춰서 만든다
export constexpr int kStaticSoundRate = 48000;

// 디코딩한 float PCM(프레임 수 x 채널 수)을 kStaticSoundRate의 Int16 정적 소리로 만든다. 만들 수 없으면 Error를 남기고 nullptr
export std::shared_ptr<StaticSoundAsset> MakeStaticSound(std::vector<float> pcm, int frames, int channels, int srcRate)
{
	if (frames <= 0 || channels <= 0 || srcRate <= 0 || pcm.size() != static_cast<std::size_t>(frames) * channels)
	{
		Core::Logger::Error("MakeStaticSound: 올바르지 않은 인자 (frames {}, channels {}, rate {}, samples {})", frames, channels, srcRate, pcm.size());
		return nullptr;
	}

	if (srcRate != kStaticSoundRate)
	{
		pcm = ResampleCubic(pcm.data(), frames, channels, srcRate, kStaticSoundRate);
		if (pcm.empty())
			return nullptr;
	}

	auto asset = std::make_shared<StaticSoundAsset>();
	ConvertFloatToInt16(pcm.data(), static_cast<int>(pcm.size()), asset->data);
	asset->channels = channels;
	asset->sampleRate = kStaticSoundRate;
	asset->format = SampleFormat::Int16;
	return asset;
}
