// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CGDemoCanvas2D.generated.h"

class UProceduralMeshComponent;
class UStaticMeshComponent;
class UMaterialInterface;

USTRUCT(BlueprintType)
struct CGDEMOSETTINGS_API FVectorArray
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	TArray<FVector> Array;
};

USTRUCT(BlueprintType)
struct CGDEMOSETTINGS_API FLayerConfig
{
	GENERATED_BODY()
public:
	static constexpr float DefaultLayerOffset = 1e-2;
	static constexpr int32 DefaultMaxLayerCount = 32;

public:

	UPROPERTY(EditAnywhere)
	float StartLayerOffset = DefaultLayerOffset;

	UPROPERTY(EditAnywhere)
	float LayerOffsetStep = DefaultLayerOffset;

	UPROPERTY(EditAnywhere)
	int32 MaxLayerCount = DefaultMaxLayerCount;
	
	UPROPERTY(EditAnywhere)
	TArray<FLinearColor> LayerColors;
};

UCLASS(BlueprintType)
class CGDEMOSETTINGS_API ACGDemoCanvas2D : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACGDemoCanvas2D();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	bool TraceRay(float& Distance, FVector& HitPoint, const FVector& Pos, const FVector& Dir);

	void CreateBackground();

	void DrawPoints(int32 Layer = 0);

	void DrawLines(int32 Layer = 0);

	void DrawPolygons(int32 Layer = 0);

	void ClearDrawing();

	FVector ToCanvasPoint(FVector2D P2D);

	FVector2D FromCanvasPoint(FVector CanvasPoint);

public:
	UPROPERTY(EditAnywhere, Category = "Canvas2D")
	float CanvasPlaneSize = 300.;

	UPROPERTY(EditAnywhere, Category = "Canvas2D")
	float CanvasPlaneX = 0.;

	UPROPERTY(VisibleAnywhere, Category = "Canvas2D")
	FBox2D CanvasBoxYZ;

	UPROPERTY(EditAnywhere, Category = "Canvas2D")
	FLinearColor BackgroundColor = FLinearColor::Black;

	UPROPERTY(EditAnywhere, Category = "Canvas2D")
	FLayerConfig PointLayerConfig {
		FLayerConfig::DefaultLayerOffset * (2 * FLayerConfig::DefaultMaxLayerCount + 1),
		FLayerConfig::DefaultLayerOffset,
		FLayerConfig::DefaultMaxLayerCount,
		{ 
			FLinearColor::White,
		}
	};

	UPROPERTY(EditAnywhere, Category = "Canvas2D")
	FLayerConfig LineLayerConfig {
		FLayerConfig::DefaultLayerOffset * (1 * FLayerConfig::DefaultMaxLayerCount + 1),
		FLayerConfig::DefaultLayerOffset,
		FLayerConfig::DefaultMaxLayerCount,
		{
			FLinearColor::Red,
			FLinearColor::Green,
			FLinearColor::Blue,
			FLinearColor::Yellow,
			FLinearColor(1, 0, 1),
			FLinearColor(0, 1, 1)
		}
	};

	UPROPERTY(EditAnywhere, Category = "Canvas2D")
	FLayerConfig PolygonLayerConfig {
		FLayerConfig::DefaultLayerOffset,
		FLayerConfig::DefaultLayerOffset,
		FLayerConfig::DefaultMaxLayerCount,
		{ 
			FLinearColor(0.5, 0.5, 0.5, 0.5),
			FLinearColor(1.0, 1.0, 0.5, 0.5),
		}
	};

	UPROPERTY(EditAnywhere, Category = "Canvas2D")
	float PointSize = 10.;

	UPROPERTY(EditAnywhere, Category = "Canvas2D")
	float LineSize = 5.;

	UPROPERTY(VisibleAnywhere, Category = "Canvas2D")
	TArray<FVectorArray> DisplayPoints;

	UPROPERTY(VisibleAnywhere, Category = "Canvas2D")
	TArray<FVectorArray> DisplayLines;

	UPROPERTY(VisibleAnywhere, Category = "Canvas2D")
	TArray<FVectorArray> DisplayPolygons;

	UPROPERTY(VisibleAnywhere, Category = "Canvas2D")
	USceneComponent* EditableRootComponent;

	UPROPERTY(VisibleAnywhere, Category = "Canvas2D")
	UProceduralMeshComponent* DrawPointsPMC;

	UPROPERTY(VisibleAnywhere, Category = "Canvas2D")
	UProceduralMeshComponent* DrawLinesPMC;

	UPROPERTY(VisibleAnywhere, Category = "Canvas2D")
	UProceduralMeshComponent* DrawPolygonsPMC;

	UPROPERTY(VisibleAnywhere, Category = "Canvas2D")
	UProceduralMeshComponent* BackgroundPMC;

	UPROPERTY(VisibleAnywhere, Category = "Canvas2D")
	UMaterialInterface* VertexColorMaterial;

	FVector Tangent = FVector::UpVector;

	FVector Normal = FVector::BackwardVector;

	//UPROPERTY(VisibleAnywhere)
	//UStaticMeshComponent* BackgroundSMC;
};
