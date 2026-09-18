module;

// 아직 모듈로 바뀌지 않은 기존 헤더들은 Global Module Fragment에 include
#include "Assert.h"
#include "Macros.h"
#include "Types.h"

export module Foundation;

export import Logger;
export import EnumUtils;
