
#include "Objects/Other/RevitInstance.h"
#include "LogSpeckle.h"

#include "API/SpeckleSerializer.h"
#include "Objects/Utils/SpeckleObjectUtils.h"
#include "Transports/Transport.h"

bool URevitInstance::Parse(const TSharedPtr<FJsonObject> Obj,  const TScriptInterface<ITransport> ReadTransport)
{
	if(!Super::Parse(Obj, ReadTransport)) return false;
	
	const float ScaleFactor = USpeckleObjectUtils::ParseScaleFactor(Units);

	//Transform
	if(!USpeckleObjectUtils::TryParseTransform(Obj, Transform)) return false;
	Transform.ScaleTranslation(FVector(ScaleFactor));
	DynamicProperties.Remove(TEXT("Transform"));
	

	//Geometries
	//NOTE: This logic differs greatly from sharp/py implementations
	const TSharedPtr<FJsonObject>* DefPtr;
	if(!Obj->TryGetObjectField(TEXT("definition"), DefPtr)) return false;
	
	const FString RefID = DefPtr->operator->()->GetStringField(TEXT("referencedId"));
	const TSharedPtr<FJsonObject> Definition = ReadTransport->GetSpeckleObject(RefID);

	const FString NameKey = TEXT("name");
	if(!Obj->TryGetStringField(NameKey, Name))
	{
		if(Definition->TryGetStringField(NameKey, Name))
		{
			//The instance has no name, so we'll steal it from the definition
			DynamicProperties.Add(NameKey, Definition->TryGetField(NameKey));
		}
	}
	auto Geometries = TArray<TSharedPtr<FJsonValue>>();
	const TArray<TSharedPtr<FJsonValue>>* ElementsPtr;
	if(Definition->TryGetArrayField(TEXT("elements"), ElementsPtr))
		Geometries.Append(*ElementsPtr);
	const TArray<TSharedPtr<FJsonValue>>* DisplayValuePtr;
	if(Definition->TryGetArrayField(TEXT("displayValue"), DisplayValuePtr))
		Geometries.Append(*DisplayValuePtr);
	

	if(Geometries.Num() <= 0)
	{
		UE_LOG(LogSpeckle, Warning, TEXT("Instance definition has no geometry. id: %s"), *RefID)
		return false;
	}
	
	for(const auto& Geo : Geometries)
	{
		const TSharedPtr<FJsonObject> MeshReference = Geo->AsObject();
		const FString ChildId = MeshReference->GetStringField(TEXT("referencedId"));

		if(ReadTransport->HasObject(ChildId))
		{
			UBase* Child = USpeckleSerializer::DeserializeBase(ReadTransport->GetSpeckleObject(ChildId), ReadTransport);
			if(IsValid(Child))
				Geometry.Add(Child);
		}
		else UE_LOG(LogSpeckle, Warning, TEXT("Block definition references an unknown object id: %s"), *ChildId)
	}
	DynamicProperties.Remove(TEXT("geometry"));
	
	// start @jairo-picott (GitHub)
	// Added by Jairo B. Picott
	// To save Object Element in RevitInstance Elements
	for (const auto& value : Obj->Values)
	{
		if (!(value.Value->Type == EJson::Object || value.Value->Type == EJson::Array))
		{
			switch (value.Value->Type)
			{
			case EJson::String:
				Parameters.Add(value.Key, value.Value->AsString());
				break;
			case EJson::Boolean:
				Parameters.Add(value.Key, value.Value->AsBool() ? TEXT("true") : TEXT("false"));
				break;
			case EJson::Number:
				Parameters.Add(value.Key, FString::SanitizeFloat(value.Value->AsNumber()));
				break;
			default:
				break;
			}
		}

		if (value.Key == TEXT("parameters"))
		{
			for (const auto& v : value.Value->AsObject()->Values)
			{
				if (v.Value->Type == EJson::Object)
				{
					FString vName;
					v.Value->AsObject()->TryGetStringField(TEXT("name"), vName);

					if (Parameters.Contains(vName)) { continue; }

					TSharedPtr<FJsonValue> vField = v.Value->AsObject()->TryGetField("value");

					switch (vField->Type)
					{
					case EJson::String:
						Parameters.Add(vName, vField->AsString());
						break;
					case EJson::Boolean:
						Parameters.Add(vName, vField->AsBool() ? TEXT("true") : TEXT("false"));
					case EJson::Number:
						Parameters.Add(vName, FString::SanitizeFloat(vField->AsNumber()));
					default:
						break;
					}
				}
			}
		}
	}
	// end @jairo-picott (GitHub)

	// Intentionally don't remove blockDefinition from dynamic properties,
	// because we want the converter to create the child geometries for us
	//DynamicProperties.Remove("blockDefinition");
	
	return true;
	
}
