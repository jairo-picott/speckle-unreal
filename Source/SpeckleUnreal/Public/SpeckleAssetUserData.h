// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetUserData.h"
#include "Engine/StaticMeshActor.h"
#include "SpeckleAssetUserData.generated.h"

/**
 * Added by @jairo-picott (GitHub)
 * To store the parameters saved on each element
 */
UCLASS(Blueprintable)
class SPECKLEUNREAL_API USpeckleAssetUserData : public UAssetUserData
{
	GENERATED_BODY()
	
public:

	USpeckleAssetUserData(const FObjectInitializer& ObjectInitializer) :
		Super(ObjectInitializer){}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Speckle|AssetUserData")
	TMap<FString, FString> Parameters;

	UFUNCTION(BlueprintCallable, Category = "Speckle|AssetUserData")
	static USpeckleAssetUserData* GetData(UObject* Object)
	{
		if (AActor* Actor = Cast<AActor>(Object))
		{
			Object = Actor->GetRootComponent();
		}

		if (IInterface_AssetUserData* AssetUserData = Cast<IInterface_AssetUserData>(Object))
		{
			return Cast<USpeckleAssetUserData>(AssetUserData->GetAssetUserDataOfClass(USpeckleAssetUserData::StaticClass()));
		}

		return nullptr;
	}

	UFUNCTION(BlueprintCallable, Category = "Speckle|AssetUserData")
	static void CopyMissingParameters(AStaticMeshActor* SmActor, AActor* Source)
	{
		if (!SmActor || !Source)
			return;

		// Get the StaticMeshComponent's UserData
		//UStaticMeshComponent* StaticMeshComponent = SmActor->GetStaticMeshComponent();
		USpeckleAssetUserData* SmUserData = GetData(SmActor);

		// Get the SourceActor's UserData from "UserDataComponent"
		USpeckleAssetUserData* SourceUserData = nullptr;
		for (UActorComponent* Component : Source->GetComponents())
		{
			if (Component->GetName() == TEXT("UserDataComponent"))
			{
				SourceUserData = Component->GetAssetUserData<USpeckleAssetUserData>();
			}
		}

		if (!SmUserData || !SourceUserData)
			return;

		for (const TPair<FString, FString>& SourceParam : SourceUserData->Parameters)
		{
			if (!SmUserData->Parameters.Contains(SourceParam.Key))
				SmUserData->Parameters.Add(SourceParam.Key, SourceParam.Value);
		}

	}
};
