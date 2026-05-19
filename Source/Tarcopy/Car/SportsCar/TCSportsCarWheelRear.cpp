// Fill out your copyright notice in the Description page of Project Settings.


#include "Car/SportsCar/TCSportsCarWheelRear.h"

UTCSportsCarWheelRear::UTCSportsCarWheelRear()
{
	WheelRadius = 40.f; //바퀴 반지름
	WheelWidth = 40.0f; //바퀴 폭
	FrictionForceMultiplier = 4.0f; //타이어 마찰력
	SlipThreshold = 100.0f; //슬립 임계값
	SkidThreshold = 100.0f; //타이어 판단 임계값
	MaxSteerAngle = 0.0f; //타이어 꺾일 수 있는 각도
	MaxHandBrakeTorque = 6000.0f; //사이드브레이크 토크단위 높을수록 빠르게멈춤
}
