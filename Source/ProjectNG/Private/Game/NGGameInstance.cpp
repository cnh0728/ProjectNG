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
}

// Host mode
void UNGGameInstance::CreateSession()
{
	UE_LOG(LogTemp, Warning, TEXT("Call CreateSession()"));
	
	if (!SessionInterface.IsValid()) return;
	
	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = true; // LAN 을 이용한 서버 매칭 (로컬 매칭이 아닌 경우 false)
	SessionSettings.NumPublicConnections = 2;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bShouldAdvertise = true; // 다른 유저에게 방을 홍보할 것인지.
	SessionSettings.bUsesPresence = true; // 사용자 상태 표시
	
	SessionInterface.Pin()->CreateSession(0, FName("TestSession1"), SessionSettings);
}

void UNGGameInstance::JoinSession()
{
	UE_LOG(LogTemp, Warning, TEXT("Call JoinSession()"));
	
	if (!SessionInterface.IsValid()) return;
	
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = true;
	SessionSearch->MaxSearchResults = 10000;
	SessionSearch->QuerySettings.Set(SEARCH_EMPTY_SERVERS_ONLY, true, EOnlineComparisonOp::Equals);
	
	SessionInterface.Pin()->FindSessions(0, SessionSearch.ToSharedRef());
}

void UNGGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccess)
{
	if (bWasSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Session Created Success !"));
		
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
			SessionInterface.Pin()->JoinSession(0, FName("TestSession1"), SessionSearch->SearchResults[0]);
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
