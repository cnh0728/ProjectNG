// Copyright (c) 2025 TeamNG. All Rights Reserved.

#include "Components/NGPathFindingComponent.h"

#include "Combat/Grid/Grid.h"
#include "Pawn/NGPawnBase.h"

UNGPathFindingComponent::UNGPathFindingComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

TArray<FIntVector2> UNGPathFindingComponent::FindPath(const FGridAddress& StartAddress, const FGridAddress& TargetAddress)
{
    TArray<FIntVector2> Path;

    // 그리드 맵 유효성 검사
    FGridMapBase* GridMap = UGridMapHelper::GetGridMap(StartAddress);
    if (!GridMap || StartAddress.GridType != TargetAddress.GridType)
    {
        return Path;
    }

    FIntVector2 StartIdx = StartAddress.GridIndex;
    FIntVector2 TargetIdx = TargetAddress.GridIndex;

    // A* 알고리즘용 컨테이너
    TArray<FIntVector2> OpenSet;
    TSet<FIntVector2> ClosedSet;
    TMap<FIntVector2, FIntVector2> CameFrom; // 경로 역추적용
    TMap<FIntVector2, int32> GScore; // 시작점으로부터의 실제 비용
    TMap<FIntVector2, int32> FScore; // GScore + 휴리스틱(H) 비용

    OpenSet.Add(StartIdx);
    GScore.Add(StartIdx, 0);
    FScore.Add(StartIdx, GetHeuristicCost(StartIdx, TargetIdx));

    while (OpenSet.Num() > 0)
    {
        // 1. OpenSet에서 FScore가 가장 낮은 노드 찾기
        FIntVector2 CurrentIdx = OpenSet[0];
        int32 LowestF = FScore.Contains(CurrentIdx) ? FScore[CurrentIdx] : MAX_int32;
        int32 CurrentOpenIndex = 0;

        for (int32 i = 1; i < OpenSet.Num(); ++i)
        {
            FIntVector2 TestIdx = OpenSet[i];
            int32 TestF = FScore.Contains(TestIdx) ? FScore[TestIdx] : MAX_int32;
            if (TestF < LowestF)
            {
                LowestF = TestF;
                CurrentIdx = TestIdx;
                CurrentOpenIndex = i;
            }
        }

        // 2. 목적지 도달 시 경로 역추적(Backtracking)
        if (CurrentIdx == TargetIdx)
        {
            FIntVector2 TraceIdx = TargetIdx;
            while (TraceIdx != StartIdx)
            {
                Path.Add(TraceIdx); // 배열의 맨 앞에 삽입하여 순서 맞춤
                TraceIdx = CameFrom[TraceIdx];
            }
            
            Algo::Reverse(Path);
            return Path; // 최종 경로 반환
        }

        // 3. 현재 노드를 Open에서 빼고 Closed에 넣음
        OpenSet.RemoveAt(CurrentOpenIndex);
        ClosedSet.Add(CurrentIdx);

        // 4. 인접한 이웃 노드(6방향) 검사
        TArray<FIntVector2> Neighbors;
        UGridMapHelper::GetHexNeighborNodesInRange(CurrentIdx, 1, Neighbors);

        for (const FIntVector2& Neighbor : Neighbors)
        {
            // 유효한 인덱스가 아니거나, 이미 닫힌 노드면 무시
            if (!GridMap->IsValidIndex(Neighbor) || ClosedSet.Contains(Neighbor))
                continue;

            // 목적지가 아닌데 타일이 비어있지 않다면(장애물/다른 폰) 갈 수 없음
            if (Neighbor != TargetIdx && !GridMap->IsGridIndexEmpty(Neighbor))
                continue;

            // 이동 비용은 인접 타일이므로 무조건 1
            int32 TentativeGScore = GScore[CurrentIdx] + 1;
            int32 NeighborGScore = GScore.Contains(Neighbor) ? GScore[Neighbor] : MAX_int32;

            // 더 빠른 길을 찾았거나 처음 방문하는 노드라면 업데이트
            if (TentativeGScore < NeighborGScore)
            {
                CameFrom.Add(Neighbor, CurrentIdx);
                GScore.Add(Neighbor, TentativeGScore);
                FScore.Add(Neighbor, TentativeGScore + GetHeuristicCost(Neighbor, TargetIdx));

                if (!OpenSet.Contains(Neighbor))
                {
                    OpenSet.Add(Neighbor);
                }
            }
        }
    }

    // 길을 찾지 못함
    return Path;
}

TArray<FIntVector2> UNGPathFindingComponent::FindPathToClosestEnemy(const FGridAddress& StartAddress, uint32 OwnerIndex)
{
    TArray<FIntVector2> Path;
    FGridMapBase* GridMap = UGridMapHelper::GetGridMap(StartAddress);
    if (!GridMap) return Path;

    FIntVector2 StartIdx = StartAddress.GridIndex;

    TArray<FIntVector2> Queue;
    TSet<FIntVector2> Visited;
    TMap<FIntVector2, FIntVector2> CameFrom;
    
    Queue.Add(StartIdx);
    Visited.Add(StartIdx);

    int32 HeadIndex = 0; 

    // UE_LOG(LogTemp, Warning, TEXT("--------------------------------"));
    
    while (HeadIndex < Queue.Num())
    {
        // 큐에서 꺼낸 현재 위치 (이 위치는 무조건 내가 서 있거나, '빈 공간'임이 보장됨)
        FIntVector2 CurrentIdx = Queue[HeadIndex++]; 

        // 이웃으로 뻗어나가기
        TArray<FIntVector2> Neighbors;
        UGridMapHelper::GetHexNeighborNodesInRange(CurrentIdx, 1, Neighbors);
        for (const FIntVector2& Neighbor : Neighbors)
        {
            // 유효하지 않거나 이미 방문했다면 스킵
            if (!GridMap->IsValidIndex(Neighbor) || Visited.Contains(Neighbor))
                continue;

            // if (!GridMap->IsValidIndex(Neighbor))
            // {
            //     UE_LOG(LogTemp, Error, TEXT("InValidIndex Index: %s"), *Neighbor.ToString());
            //     
            //     continue;
            // }
            //     
            // if (Visited.Contains(Neighbor))
            // {
            //     UE_LOG(LogTemp, Error, TEXT("Visited Index: %s"), *Neighbor.ToString());
            //     
            //     continue;
            // }
            // UE_LOG(LogTemp, Log, TEXT("Neighbor Index: %s"), *Neighbor.ToString());
            
            int32 NeighborDataIdx = GridMap->ConvertPointToIndex(Neighbor);
            ANGPawnBase* PlacedPawn = GridMap->GridInfo[NeighborDataIdx].PlacedPawn;
            
            
            if (PlacedPawn != nullptr)
            {
                // 누군가 서 있을때
                bool bIsEnemy = !PlacedPawn->IsSameTeam(OwnerIndex); 

                if (bIsEnemy)
                {
                    // 큐에 넣기 전에 적을 발견했으므로 즉시 탐색 종료
                    FIntVector2 TraceIdx = CurrentIdx;

                    while (TraceIdx != StartIdx)
                    {
                        Path.Add(TraceIdx);
                        TraceIdx = CameFrom[TraceIdx];
                    }
                    
                    Algo::Reverse(Path);
                    return Path; 
                }
                else
                {
                    // 아군이거나 통과할 수 없는 장애물인 경우
                    // 방문 처리만 하고 큐에는 절대 넣지 않음
                    Visited.Add(Neighbor);
                    continue; 
                }
            }
            else
            {
                // 완벽한 빈 공간일 경우에만 다음 탐색을 위해 큐에 추가
                Visited.Add(Neighbor);
                CameFrom.Add(Neighbor, CurrentIdx);
                Queue.Add(Neighbor);
            }
        }
    }

    return Path; // 도달할 수 있는 적이 없음 (모두 벽이나 아군으로 둘러싸여 막힌 상태)
}

int32 UNGPathFindingComponent::GetHeuristicCost(const FIntVector2& A, const FIntVector2& B) const
{
    return UGridMapHelper::GetDistance(A, B);
}