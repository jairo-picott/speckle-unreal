
#include "Objects/DisplayValueElement.h"

#include "API/SpeckleSerializer.h"
#include "Objects/Geometry/Mesh.h"


TArray<FString> UDisplayValueElement::DisplayValueAliasStrings = {
	"displayValue",
	"@displayValue",
};


bool UDisplayValueElement::AddDisplayValue(const TSharedPtr<FJsonObject> Obj, const TScriptInterface<ITransport> ReadTransport)
{
	UMesh* DisplayMesh = Cast<UMesh>(USpeckleSerializer::DeserializeBase(Obj, ReadTransport));
	const bool Valid = IsValid(DisplayMesh);
	if(Valid)
		this->DisplayValue.Add(DisplayMesh);
	return Valid;
}

bool UDisplayValueElement::Parse(const TSharedPtr<FJsonObject> Obj, const TScriptInterface<ITransport> ReadTransport)
{
	if(!Super::Parse(Obj, ReadTransport)) return false;

	//Find display values
	for(const FString& Alias : DisplayValueAliasStrings)
	{
		const TSharedPtr<FJsonObject>* SubObjectPtr;
		if (Obj->TryGetObjectField(Alias, SubObjectPtr))
		{
			AddDisplayValue(*SubObjectPtr, ReadTransport);
			DynamicProperties.Remove(Alias);
			continue;
		}

		const TArray<TSharedPtr<FJsonValue>>* SubArrayPtr;
		if (Obj->TryGetArrayField(Alias, SubArrayPtr))
		{
			for (const auto& ArrayElement : *SubArrayPtr)
			{
				const TSharedPtr<FJsonObject>* ArraySubObjPtr;
				if (ArrayElement->TryGetObject(ArraySubObjPtr))
				{
					AddDisplayValue(*ArraySubObjPtr, ReadTransport);
				}
			}
			DynamicProperties.Remove(Alias);
		}


		//Modifications made by Jairo B. Picott to save Object Element in static meshes
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
		
	}

	return DisplayValue.Num() > 0;
}
