// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define ECC_SelectableUnit ECC_GameTraceChannel1
#define ECC_Selectable ECC_GameTraceChannel2

UENUM(BlueprintType)
enum class EUILayer : uint8
{
	Background	= 0 UMETA(DisplayName = "Background", ToolTip = "뒷배경 및 가장 아래"),
	Game		= 1 UMETA(DisplayName = "Game", ToolTip = "체력바, 맵, 게임 요소"),
	Window		= 2 UMETA(DisplayName = "Window", ToolTip = "인벤토리, 로비"),
	Popup		= 3 UMETA(DisplayName = "Popup", ToolTip = "상점 팝업, 알림 팝업"),
	System		= 4 UMETA(DisplayName = "System", ToolTip = "로딩 화면, 네트워크 끊김"),
	Top			= 5 UMETA(DisplayName = "Top", ToolTip = "가장 최상위 화면")
};