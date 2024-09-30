
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"


#include "SpeckleSerializer.generated.h"

class UBase;
class ITransport;
class FJsonObject;

// start @jairo-picott (GitHub)
// Added by Jairo B. Picott
// Make it available in SPECKLEUNREAL_API

UCLASS()
class SPECKLEUNREAL_API USpeckleSerializer : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintPure, Category="Speckle|Serialization")
	static UBase* DeserializeBaseById(const FString& ObjectId, const TScriptInterface<ITransport> ReadTransport);

	static UBase* DeserializeBase(const TSharedPtr<FJsonObject> Obj, const TScriptInterface<ITransport> ReadTransport);
	
};
