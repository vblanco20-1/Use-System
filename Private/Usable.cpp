// Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
     
#include "MyProject.h"

#include "Usable.h"

UUsable::UUsable(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


//This is required for compiling, would also let you know if somehow you called
//the base event/function rather than the over-rided version
void IUsable::OnUsed(AController * user)
{
	BTOnUsed(user);
}

void IUsable::OnFocus(AController * user)
{
	BTOnFocus(user);
}


FString IUsable::GetNameToDisplay()
{
	return FString("");
}