// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
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
	
public:
	UNGGameInstance();
	
	virtual void Init() override;
	
	UFUNCTION(BlueprintCallable, Category = "NGGameInstance", meta = (ToolTip = "사용자가 직접 방을 만듭니다."))
	void CreateSession();
	
	UFUNCTION(BlueprintCallable, Category = "NGGameInstance")
	void JoinSession();
	
protected:
	// 세션 인터페이스
	TWeakPtr<IOnlineSession> SessionInterface;
	
	// 세션 검색 결과를 저장할 공유 포인터
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	
	// 매치 메이킹 콜백
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccess);
	void OnFindSessionsComplete(bool bWasSuccess);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
};
