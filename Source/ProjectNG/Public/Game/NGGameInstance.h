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
 * 
 * OnlineSubsystem에 관한 Docs.
 * https://dev.epicgames.com/documentation/ko-kr/unreal-engine/online-subsystem-session-interface-in-unreal-engine
 */
UCLASS()
class PROJECTNG_API UNGGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UNGGameInstance();
	
	virtual void Init() override;
	
	/** 세션 생성 */
	UFUNCTION(BlueprintCallable, Category = "NGGameInstance", meta = (ToolTip = "사용자가 직접 방을 만듭니다."))
	void CreateSession();
	
	/** 세션 검색 및 입장 */
	UFUNCTION(BlueprintCallable, Category = "NGGameInstance")
	void JoinSession();
	
	UPROPERTY(BlueprintReadOnly)
	FString PlayerName; // Todo: Init 함수에서 GUID로 생성중.
	
protected:
	/** 
	 * 세션 인터페이스
	 * - 방 만들기, 찾기, 종료 기능을 담당하는 인터페이스
	 */
	TWeakPtr<IOnlineSession> SessionInterface;

	/**
	 * 세션 검색 설정
	 * - 방 검색 조건과 결과를 담는 컨테이너
	 */
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	
	// 매치 메이킹 콜백
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccess); // 생성시
	void OnFindSessionsComplete(bool bWasSuccess); // 검색 완료시
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result); // 입장 완료 시
};
