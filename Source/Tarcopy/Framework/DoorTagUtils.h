// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"

inline FName GetDoorTagName()
{
	static const FName DoorTag(TEXT("Door"));
	return DoorTag;
}

inline bool ActorHasDoorTagOrDoorMesh(const AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return false;
	}

	const FName DoorTag = GetDoorTagName();
	if (Actor->ActorHasTag(DoorTag))
	{
		return true;
	}

	TInlineComponentArray<UStaticMeshComponent*> MeshComponents(Actor);
	for (const UStaticMeshComponent* MeshComp : MeshComponents)
	{
		if (IsValid(MeshComp) && MeshComp->ComponentHasTag(DoorTag))
		{
			return true;
		}
	}

	return false;
}
