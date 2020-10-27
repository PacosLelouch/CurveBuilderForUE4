// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CGDemoPlayerController.h"
#include "CGDemoPlayerController2D.generated.h"

class UStaticMesh;
class ACGDemoCanvas2D;

/**
 * 
 */
UCLASS()
class CGDEMOSETTINGS_API ACGDemoPlayerController2D : public ACGDemoPlayerController
{
	GENERATED_BODY()
public:
	ACGDemoPlayerController2D(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

	void Canvas3DTo2D(TArray<FVector2D>& Out, const TArray<FVector>& In, bool bAlreadyEmpty = false);
	void Canvas3DTo2D(TArray<TTuple<double, double> >& Out, const TArray<FVector>& In, bool bAlreadyEmpty = false);

	void Canvas2DTo3D(TArray<FVector>& Out, const TArray<FVector2D>& In, bool bAlreadyEmpty = false);
	void Canvas2DTo3D(TArray<FVector>& Out, const TArray<TTuple<double, double> >& In, bool bAlreadyEmpty = false);
    
public:
	ACGDemoCanvas2D* Canvas2D;
protected:
	bool TraceToCanvas(float& Distance, FVector& HitPoint, const FVector& Pos, const FVector& Dir);
};
