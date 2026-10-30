export module Service.Render:TextRuns;

import std;
import Contract.Render.Shared.Types;
import Core.Utils;

export std::vector<TextRun> BuildTextRuns(std::span<const TextSpan> spans)
{
	std::vector<TextRun> runs;
	std::uint32_t lineIndex = 0;
	for (auto& span : spans)
	{
		if (span.text.empty()) continue;
		std::vector<char32_t> codepoints = Core::UTF8ToUTF32(span.text);
		std::size_t segStart = 0;
		for (std::size_t i = 0; i <= codepoints.size(); ++i)
		{
			bool isNewline = (i < codepoints.size()) && (codepoints[i] == U'\n');
			bool isEnd = (i == codepoints.size());
			if (!isNewline && !isEnd)
				continue;
			if (i > segStart) // 빈 세그먼트(연속 \n)는 run을 만들지 않음
			{
				runs.push_back({
					std::vector<char32_t>(codepoints.begin() + segStart, codepoints.begin() + i),
					span.style,
					lineIndex
					});
			}
			if (isNewline)
				++lineIndex; // 내용이 있든 없든 줄 번호는 증가
			segStart = i + 1;
		}
	}
	return runs;
}