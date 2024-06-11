
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

		if (Obj && Obj->HasField(TEXT("parameters")))
		{
			const TSharedPtr<FJsonObject>* ParametersJson;
			UE_LOG(LogTemp, Warning, TEXT("Here"));
			if (Obj->TryGetObjectField(TEXT("parameters"), ParametersJson))
			{
				for (const auto& Entry : ParametersJson->Get()->Values)
				{
					if (Entry.Value.IsValid() && Entry.Value->Type == EJson::Object)
					{
						TSharedPtr<FJsonObject> FieldObj = Entry.Value->AsObject();
						FString Name;
						FieldObj->TryGetStringField(TEXT("name"), Name);
						Name.ReplaceInline(TEXT(" "), TEXT("")); //Remove Spaces in the paramters' names

						if (FieldObj->HasField(TEXT("value")))
						{
							TSharedPtr<FJsonValue> ValueField = FieldObj->TryGetField(TEXT("value"));
							if (ValueField.IsValid())
							{
								FString ParameterLine;
								if (ValueField->Type == EJson::String)
								{
									FString Value;
									if (FieldObj->TryGetStringField("value", Value))
									{
										ParameterLine = FString::Printf(TEXT("%s.%s"), *Name, *Value);
										Parameters.Add(Name, Value);
									}
								}
								else if (ValueField->Type == EJson::Number)
								{
									float FieldValueNumber;
									if (ValueField->TryGetNumber(FieldValueNumber))
									{
										FString Value = FString::SanitizeFloat(FieldValueNumber);
										Parameters.Add(Name, Value);
									}
								}
								else if (ValueField->Type == EJson::Boolean)
								{
									bool bFieldValueBool;
									if (ValueField->TryGetBool(bFieldValueBool))
									{
										FString ValueBool = bFieldValueBool ? TEXT("true") : TEXT("false");
										Parameters.Add(Name, ValueBool);
									}

								}
							}
						}
					}
				}
			}
		
		}
		
	}

	return DisplayValue.Num() > 0;
}
