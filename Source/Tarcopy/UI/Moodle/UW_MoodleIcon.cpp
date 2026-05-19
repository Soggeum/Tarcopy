// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Moodle/UW_MoodleIcon.h"

#include "Components/ProgressBar.h"

void UUW_MoodleIcon::SetRatio(float InRatio)
{
	BackgroundFill->SetPercent(InRatio);
	BackgroundFill->SetFillColorAndOpacity(GetMoodColor(InRatio));
}

FLinearColor UUW_MoodleIcon::GetMoodColor(float Ratio)
{
	if (Ratio > 0.66f) 
	{
		return FLinearColor(0.26f, 0.74f, 0.45f, 0.90f);
	}
	if (Ratio > 0.33f)
	{
		return FLinearColor(0.96f, 0.74f, 0.22f, 0.90f);
	}
	return FLinearColor(0.90f, 0.36f, 0.36f, 0.90f);
}