#pragma once
#include "Usable.generated.h"

/**This interface has to be added to every Actor that can be used, you have to implement the OnUsed function */
UINTERFACE(MinimalAPI,Blueprintable)
class UUsable :public UInterface
{
	GENERATED_UINTERFACE_BODY()
	
};

class IUsable
{
	GENERATED_IINTERFACE_BODY()

	
	virtual void OnUsed(AController * user);

	virtual void  OnFocus(AController * user);

	// This will be called by the HUD if you want to display a name in the USE message, leave it empty if you dont want any message
	virtual FString GetNameToDisplay();

	//This event will be called when the character gets close to an object and presses the Use key.
	UFUNCTION(BlueprintImplementableEvent, meta = (FriendlyName = "On Used"))
	void BTOnUsed(AController * user);

	//This event will be called every frame if the object is being watched by the Use System in the HUD
	UFUNCTION(BlueprintImplementableEvent, meta = (FriendlyName = " On Focus"))
	void  BTOnFocus( AController * user);
};