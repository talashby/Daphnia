// Fill out your copyright notice in the Description page of Project Settings.


#include "MyHudWidget.h"
#include "DaphniaPawn.h"

void UMyHudWidget::SwitchCameraView()
{
	ADaphniaPawn::Instance()->SwitchView();
}
