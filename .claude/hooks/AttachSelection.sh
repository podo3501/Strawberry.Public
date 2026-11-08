#!/bin/bash
# UserPromptSubmit 훅: UIVisualizer에서 고른 위젯이 있으면 선택 정보를 이 메시지에 붙이고 파일을 지운다.
# 파일이 없어진 것을 보고 UIVisualizer가 선택을 비우므로, 번호는 메시지마다 1번부터 다시 시작한다.
# 선택이 없거나 파일이 없으면 아무것도 출력하지 않는다.

root="${CLAUDE_PROJECT_DIR:-.}"
file="$root/Testing/Resource/.visualizer/Selection.txt"

[ -f "$file" ] || exit 0
grep -q '(선택 없음)' "$file" && exit 0

echo "UIVisualizer에서 사람이 고른 위젯이다. 메시지의 번호(1번, 2번...)는 아래 표의 번호이고, 경로의 위젯은 layout에 적힌 JSON 안에 있다."
cat "$file"
rm -f "$file"
