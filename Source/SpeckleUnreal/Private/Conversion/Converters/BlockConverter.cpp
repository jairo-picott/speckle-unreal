
#include "Conversion/Converters/BlockConverter.h"

#include "Objects/Other/BlockInstance.h"
#include "Objects/Utils/SpeckleObjectUtils.h"
#include "Engine/World.h"
#include "Objects/Other/RevitInstance.h"
#include "SpeckleAssetUserData.h"

UBlockConverter::UBlockConverter()
{
	SpeckleTypes.Add(UBlockInstance::StaticClass());
	SpeckleTypes.Add(URevitInstance::StaticClass());
	
	BlockInstanceActorType = AActor::StaticClass();
	ActorMobility = EComponentMobility::Static;
}

UObject* UBlockConverter::ConvertToNative_Implementation(const UBase* SpeckleBase, UWorld* World, TScriptInterface<ISpeckleConverter>&)
{
	const UInstance* Block = Cast<UInstance>(SpeckleBase);
	if(Block == nullptr) return nullptr;

	return BlockToNative(Block, World);
}

AActor* UBlockConverter::BlockToNative(const UInstance* Block, UWorld* World)
{
	AActor* BlockActor = CreateEmptyActor(World, USpeckleObjectUtils::CreateTransform(Block->Transform));

	// start @jairo-picott (GitHub)
	// Added by Jairo B. Picott
	// To try to collect from the UInstance the parameters mapped if URevitInstance	
	if (BlockActor)
	{
		const URevitInstance* ri = Cast<URevitInstance>(Block);
		if (ri && ri->Parameters.Num() > 0)
		{
			USpeckleAssetUserData* Aud = NewObject<USpeckleAssetUserData>();
			Aud->Parameters = ri->Parameters;
			UActorComponent* FoundComponent = nullptr;
			for (UActorComponent* Component : BlockActor->GetComponents())
			{
				if (Component->GetName() == TEXT("UserDataComponent"))
				{
					Component->AddAssetUserData(Aud);
				}
			}
		}
	}
	// end @jairo-picott (GitHub)

	//Return the block actor as is,
	//Other converter logic will convert child geometries because UBlockInstance intentionally left them as dynamic properties
	return BlockActor;
}

AActor* UBlockConverter::CreateEmptyActor(UWorld* World, const FTransform& Transform, const FActorSpawnParameters& SpawnParameters)
{
	AActor* Actor = World->SpawnActor<AActor>(BlockInstanceActorType, Transform, SpawnParameters);

	if(!Actor->HasValidRootComponent())
	{
		USceneComponent* Scene = NewObject<USceneComponent>(Actor, "Root");
		Scene->SetRelativeTransform(Transform);
		Actor->SetRootComponent(Scene);
		Scene->RegisterComponent();
	}
	USceneComponent* RootComponent = Actor->GetRootComponent();
	
	RootComponent->SetMobility(ActorMobility);

	// start @jairo-picott (GitHub)
	// Added by Jairo B. Picott
	// Create a custom component to Hold AssetUserData
	// Where the parameters will be saved
	TObjectPtr<USceneComponent> UserDataComponent = NewObject<USceneComponent>(Actor, USceneComponent::StaticClass());
	UserDataComponent->RegisterComponent();
	Actor->AddInstanceComponent(UserDataComponent);
	UserDataComponent->Rename(TEXT("UserDataComponent"));
	// end @jairo-picott (GitHub)
	
	return Actor;
}

UBase* UBlockConverter::ConvertToSpeckle_Implementation(const UObject* Object)
{
	unimplemented();
	return nullptr; //TODO implement
}
