// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetUserData.h"
#include "SpeckleAssetUserData.generated.h"

/**
 * 
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
};
