// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "CurveBuilderTestPlayerController.h"
#include "BezierConcatPlayerController.generated.h"

//USTRUCT(BlueprintType)
//struct CURVEBUILDERTEST_API FCurveBuilderTestParamsInput
//{
//	GENERATED_BODY()
//public:
//	UPROPERTY(BlueprintReadWrite)
//		float CurrentWeight = 1.;
//
//};

UENUM(BlueprintType)
enum class EConcatType : uint8
{
	ToPoint,
	ToCurve,
};

/**
 *
 */
UCLASS()
class CURVEBUILDERTEST_API ABezierConcatPlayerController : public ACurveBuilderTestPlayerController
{
	GENERATED_BODY()
public:
	ABezierConcatPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

	virtual void BindOnRightMouseButtonReleased() override;
	virtual void BindOnCtrlAndKey1Released() override;
	virtual void BindOnCtrlAndKey2Released() override;
	virtual void BindOnCtrlAndKey3Released() override;
	virtual void BindOnCtrlAndKey4Released() override;
	virtual void BindOnCtrlAndKey5Released() override;
	virtual void BindOnCtrlAndKey0Released() override;

public:
	UFUNCTION(BlueprintCallable)
	void ChangeConcatType(EConcatType Type);

	//UFUNCTION(BlueprintCallable)
	//void OnParamsInputChanged();

public:

	UPROPERTY(BlueprintReadOnly)
	EConcatType ConcatType = EConcatType::ToPoint;
public:

protected:
	virtual void AddControlPoint(const FVector& HitPoint) override;
	
private:

	UFUNCTION()
	void ChangeConcatTypeToPoint(FKey Key, EInputEvent Event, APlayerController* Ctrl);

	UFUNCTION()
	void ChangeConcatTypeToCurve(FKey Key, EInputEvent Event, APlayerController* Ctrl);

};
