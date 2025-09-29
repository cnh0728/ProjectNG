// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "NGGameInstance.generated.h"

/**
 * ProjectNG에서 사용되는 GameInstance Class
 *
 * 주로 여러 게임 판에 걸쳐 유지되어야 하는 데이터를 관리
 * 온라인 서버에 접속하고, 매치메이킹을 요청하는 등 세션 관련 기능을 총괄
 */
UCLASS()
class PROJECTNG_API UNGGameInstance : public UGameInstance
{
	GENERATED_BODY()
};
