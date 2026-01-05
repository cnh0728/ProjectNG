// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Game/NGGameInstance.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Online/OnlineSessionNames.h"

UNGGameInstance::UNGGameInstance()
{
	
}

void UNGGameInstance::Init()
{
	Super::Init();

	if (IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
	{
		SessionInterface = Subsystem->GetSessionInterface();
		
		// Bind delegate.
		if (SessionInterface.IsValid())
		{
			SessionInterface.Pin()->OnCreateSessionCompleteDelegates.AddUObject(this, &UNGGameInstance::OnCreateSessionComplete);
			SessionInterface.Pin()->OnFindSessionsCompleteDelegates.AddUObject(this, &UNGGameInstance::OnFindSessionsComplete);
			SessionInterface.Pin()->OnJoinSessionCompleteDelegates.AddUObject(this, &UNGGameInstance::OnJoinSessionComplete);
		}
	}
	
	PlayerName = FGuid::NewGuid().ToString().Left(8);
}

// 세션 생성 요청
void UNGGameInstance::CreateSession()
{
	UE_LOG(LogTemp, Warning, TEXT("Call CreateSession()"));
	
	if (!SessionInterface.IsValid()) return;
	
	// 아래 링크에서 변수 설정 값 확인 가능. 또는 OnlineSessionSetting.h 파일에서 확인 가능.
	// https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Plugins/OnlineSubsystem/FOnlineSessionSettings#variables
	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = true; // LAN 을 이용한 서버 매칭 (로컬 매칭이 아닌 경우 false)
	SessionSettings.NumPublicConnections = 2;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bShouldAdvertise = true; // 다른 유저에게 방을 홍보할 것인지.
	SessionSettings.bUsesPresence = true; // 사용자 상태 표시
	
	SessionInterface.Pin()->CreateSession(0, NAME_GameSession, SessionSettings);
}

// 세션 참가 요청
void UNGGameInstance::JoinSession()
{
	UE_LOG(LogTemp, Warning, TEXT("Call JoinSession()"));
	
	if (!SessionInterface.IsValid()) return;
	
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = true; // CreateSession의 설정과 맞추기.
	SessionSearch->MaxSearchResults = 10000;
	SessionSearch->QuerySettings.Set(SEARCH_EMPTY_SERVERS_ONLY, true, EOnlineComparisonOp::Equals);
	
	SessionInterface.Pin()->FindSessions(0, SessionSearch.ToSharedRef());
}

void UNGGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccess)
{
	if (bWasSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Session Created Success !"));
		
		// 방장이 Listen Server로 맵을 열기
		UGameplayStatics::OpenLevel(GetWorld(), "/Game/Maps/LobbyMap", true, "listen");
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Session Created Fail !"));
	}
}

void UNGGameInstance::OnFindSessionsComplete(bool bWasSuccess)
{
	if (bWasSuccess && SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Session Search Success !"));
		
		if (SessionSearch->SearchResults.Num() > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Join first Success !"));
			SessionInterface.Pin()->JoinSession(0, NAME_GameSession, SessionSearch->SearchResults[0]);
		}
	}
}

void UNGGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("Session Joined Success !"));
		
		FString ConnectionInfo;
		if (SessionInterface.Pin()->GetResolvedConnectString(SessionName, ConnectionInfo))
		{
			APlayerController* PC = GetFirstLocalPlayerController();
			if (PC)
			{
				PC->ClientTravel(ConnectionInfo, TRAVEL_Absolute);
			}
		}
	}
}
