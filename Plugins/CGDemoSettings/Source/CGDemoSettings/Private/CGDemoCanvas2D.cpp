// Fill out your copyright notice in the Description page of Project Settings.


#include "CGDemoCanvas2D.h"
#include "UObject/ConstructorHelpers.h"
#include "ProceduralMeshComponent.h"
#include "Materials/Material.h"
//#include "Engine/StaticMesh.h"
//#include "Components/StaticMeshComponent.h"

//#include "Kismet/GameplayStatics.h"

// Sets default values
ACGDemoCanvas2D::ACGDemoCanvas2D()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ConstructorHelpers::FObjectFinder<UMaterial>
		MaterialFinder(TEXT("/Engine/EngineDebugMaterials/VertexColorMaterial.VertexColorMaterial"));
	if (MaterialFinder.Succeeded()) {
		VertexColorMaterial = MaterialFinder.Object;
	}
	EditableRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = EditableRootComponent;
	BackgroundPMC = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Background"));
	BackgroundPMC->AttachToComponent(EditableRootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	BackgroundPMC->SetMaterial(0, VertexColorMaterial);

	DrawPointsPMC = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("DrawPoints"));
	DrawPointsPMC->AttachToComponent(EditableRootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	DrawLinesPMC = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("DrawLines"));
	DrawLinesPMC->AttachToComponent(EditableRootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	DrawPolygonsPMC = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("DrawPolygons"));
	DrawPolygonsPMC->AttachToComponent(EditableRootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

// Called when the game starts or when spawned
void ACGDemoCanvas2D::BeginPlay()
{
	Super::BeginPlay();

	DisplayPoints.SetNum(PointLayerConfig.MaxLayerCount);
	DisplayLines.SetNum(LineLayerConfig.MaxLayerCount);
	DisplayPolygons.SetNum(PolygonLayerConfig.MaxLayerCount);

	CanvasBoxYZ = FBox2D(
		FVector2D(-CanvasPlaneSize, -CanvasPlaneSize),
		FVector2D(CanvasPlaneSize, CanvasPlaneSize));
	CreateBackground();
}

// Called every frame
void ACGDemoCanvas2D::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ACGDemoCanvas2D::TraceRay(float& Distance, FVector& HitPoint, const FVector& Pos, const FVector& Dir)
{
	auto InvWeightWithCheck = [](auto W) { return (FMath::IsNearlyZero(W) ? 1 : 1 / W); };
	const FTransform& CanvasToWorld = ActorToWorld();
	const FTransform& WorldToCanvas = CanvasToWorld.Inverse();
	FVector4 LocalPos4 = WorldToCanvas.TransformFVector4(FVector4(Pos, 1));
	FVector4 LocalDir4 = WorldToCanvas.TransformFVector4(FVector4(Dir, 0));
	FVector LocalPos = LocalPos4 * InvWeightWithCheck(LocalPos4.W);
	FVector LocalDir = LocalDir4 * InvWeightWithCheck(LocalDir4.W);
	if (LocalDir.X <= 0) {
		return false;
	}
	Distance = (CanvasPlaneX - LocalPos.X) / LocalDir.X;
	if (Distance <= 0) {
		return false;
	}
	const FVector& LocalHitPoint = LocalPos + LocalDir * Distance;
	const FVector4& HitPoint4 = CanvasToWorld.TransformFVector4(FVector4(LocalHitPoint, 1));
	HitPoint = HitPoint4 * InvWeightWithCheck(HitPoint4.W);
	FVector2D HitPoint2D(HitPoint.Y, HitPoint.Z);
	if (CanvasBoxYZ.Min.X > HitPoint2D.X ||
		CanvasBoxYZ.Min.Y > HitPoint2D.Y ||
		CanvasBoxYZ.Max.X < HitPoint2D.X ||
		CanvasBoxYZ.Max.Y < HitPoint2D.Y) {
		return false;
	}
	return true;
}

void ACGDemoCanvas2D::CreateBackground()
{
	FProcMeshSection Section;
	Section.bEnableCollision = false;
	Section.bSectionVisible = true;
	Section.ProcVertexBuffer.SetNum(4);
	Section.ProcVertexBuffer[0].Position = FVector(CanvasPlaneX, CanvasBoxYZ.Min.X, CanvasBoxYZ.Min.Y); // LB
	Section.ProcVertexBuffer[1].Position = FVector(CanvasPlaneX, CanvasBoxYZ.Max.X, CanvasBoxYZ.Min.Y); // RB
	Section.ProcVertexBuffer[2].Position = FVector(CanvasPlaneX, CanvasBoxYZ.Max.X, CanvasBoxYZ.Max.Y); // RT
	Section.ProcVertexBuffer[3].Position = FVector(CanvasPlaneX, CanvasBoxYZ.Min.X, CanvasBoxYZ.Max.Y); // LT
	Section.SectionLocalBox = FBox(
		Section.ProcVertexBuffer[0].Position,
		Section.ProcVertexBuffer[2].Position);
	for (int32 i = 0; i < Section.ProcVertexBuffer.Num(); ++i) {
		Section.ProcVertexBuffer[i].Normal = Normal;
		Section.ProcVertexBuffer[i].Tangent = { Tangent, false };
		Section.ProcVertexBuffer[i].Color = BackgroundColor.ToFColor(false);
	}

	Section.ProcIndexBuffer = {
		// Forward
		0, 1, 2,
		0, 2, 3,
		// Backward
		0, 2, 1,
		0, 3, 2,
	};

	BackgroundPMC->SetProcMeshSection(0, Section);
	//BackgroundPMC->CreateMeshSection(
	//	Vertices, Triangles, Normals,
	//	UV0, UV1, UV2, UV3, VertexColors, Tangents, bCreateCollision);
}

static constexpr float LayerOffset = 1e-2;

void ACGDemoCanvas2D::DrawPoints(int32 Layer)
{
	Layer = Layer % PointLayerConfig.MaxLayerCount;
	const TArray<FLinearColor>& PointColors = PointLayerConfig.LayerColors;
	DrawPointsPMC->SetMaterial(Layer, VertexColorMaterial);
	float PointExtent = PointSize * 0.5;
	float PointOffset = PointLayerConfig.StartLayerOffset + PointLayerConfig.LayerOffsetStep * Layer;
	FProcMeshSection Section;
	Section.bEnableCollision = false;
	Section.bSectionVisible = true;
	Section.ProcVertexBuffer.SetNum(DisplayPoints[Layer].Array.Num() * 4);
	Section.ProcIndexBuffer.SetNum(DisplayPoints[Layer].Array.Num() * 6);
	for (int32 i = 0; i < DisplayPoints[Layer].Array.Num(); ++i) {
		int32 StartV = i * 4;
		const FVector& CenterPos = DisplayPoints[Layer].Array[i];
		Section.ProcVertexBuffer[StartV + 0].Position = 
			CenterPos + FVector(-PointOffset, -PointExtent, -PointExtent); // LB
		Section.ProcVertexBuffer[StartV + 1].Position = 
			CenterPos + FVector(-PointOffset, PointExtent, -PointExtent); // RB
		Section.ProcVertexBuffer[StartV + 2].Position = 
			CenterPos + FVector(-PointOffset, PointExtent, PointExtent); // RT
		Section.ProcVertexBuffer[StartV + 3].Position = 
			CenterPos + FVector(-PointOffset, -PointExtent, PointExtent); // LT
		Section.SectionLocalBox += FBox(
			Section.ProcVertexBuffer[StartV + 0].Position,
			Section.ProcVertexBuffer[StartV + 2].Position);

		int32 StartI = i * 6;
		Section.ProcIndexBuffer[StartI + 0] = StartV + 0;
		Section.ProcIndexBuffer[StartI + 1] = StartV + 1;
		Section.ProcIndexBuffer[StartI + 2] = StartV + 2;
		Section.ProcIndexBuffer[StartI + 3] = StartV + 0;
		Section.ProcIndexBuffer[StartI + 4] = StartV + 2;
		Section.ProcIndexBuffer[StartI + 5] = StartV + 3;
	}
	for (int32 i = 0; i < Section.ProcVertexBuffer.Num(); ++i) {
		Section.ProcVertexBuffer[i].Normal = Normal;
		Section.ProcVertexBuffer[i].Tangent = { Tangent, false };
		Section.ProcVertexBuffer[i].Color = (Layer < PointColors.Num() ? PointColors[Layer] : PointColors.Last()).ToFColor(false);
	}

	DrawPointsPMC->SetProcMeshSection(Layer, Section);
}

void ACGDemoCanvas2D::DrawLines(int32 Layer)
{
	Layer = Layer % LineLayerConfig.MaxLayerCount;
	if (DisplayLines[Layer].Array.Num() < 2) {
		DrawLinesPMC->SetProcMeshSection(Layer, FProcMeshSection());
		return;
	}
	const TArray<FLinearColor>& LineColors = LineLayerConfig.LayerColors;
	DrawLinesPMC->SetMaterial(Layer, VertexColorMaterial);
	float LineExtent = LineSize * 0.5;
	float LineOffset = LineLayerConfig.StartLayerOffset + LineLayerConfig.LayerOffsetStep * Layer;
	FProcMeshSection Section;
	Section.bEnableCollision = false;
	Section.bSectionVisible = true;
	int32 SegmentNum = DisplayLines[Layer].Array.Num() - 1;
	Section.ProcVertexBuffer.SetNum(SegmentNum * 4);
	Section.ProcIndexBuffer.SetNum(SegmentNum * 6);
	for (int32 i = 0; i < SegmentNum; ++i) {
		int32 StartV = i * 4;
		const FVector& LeftCenterPos = DisplayLines[Layer].Array[i], RightCenterPos = DisplayLines[Layer].Array[i + 1];
		FVector Direction = (RightCenterPos - LeftCenterPos).GetSafeNormal();
		FVector Lateral = Direction ^ Normal;
		Section.ProcVertexBuffer[StartV + 0].Position =
			LeftCenterPos + Normal * LineOffset - Lateral * LineExtent; // LB
		Section.ProcVertexBuffer[StartV + 1].Position =
			RightCenterPos + Normal * LineOffset - Lateral * LineExtent; // RB
		Section.ProcVertexBuffer[StartV + 2].Position =
			RightCenterPos + Normal * LineOffset + Lateral * LineExtent; // RT
		Section.ProcVertexBuffer[StartV + 3].Position =
			LeftCenterPos + Normal * LineOffset + Lateral * LineExtent; // LT
		Section.SectionLocalBox += Section.ProcVertexBuffer[StartV + 0].Position;
		Section.SectionLocalBox += Section.ProcVertexBuffer[StartV + 1].Position;
		Section.SectionLocalBox += Section.ProcVertexBuffer[StartV + 2].Position;
		Section.SectionLocalBox += Section.ProcVertexBuffer[StartV + 3].Position;

		int32 StartI = i * 6;
		Section.ProcIndexBuffer[StartI + 0] = StartV + 0;
		Section.ProcIndexBuffer[StartI + 1] = StartV + 1;
		Section.ProcIndexBuffer[StartI + 2] = StartV + 2;
		Section.ProcIndexBuffer[StartI + 3] = StartV + 0;
		Section.ProcIndexBuffer[StartI + 4] = StartV + 2;
		Section.ProcIndexBuffer[StartI + 5] = StartV + 3;
	}
	for (int32 i = 0; i < Section.ProcVertexBuffer.Num(); ++i) {
		Section.ProcVertexBuffer[i].Normal = Normal;
		Section.ProcVertexBuffer[i].Tangent = { Tangent, false };
		Section.ProcVertexBuffer[i].Color = (Layer < LineColors.Num() ? LineColors[Layer] : LineColors.Last()).ToFColor(false);
	}

	DrawLinesPMC->SetProcMeshSection(Layer, Section);
}

void ACGDemoCanvas2D::DrawPolygons(int32 Layer)
{
	Layer = Layer % PolygonLayerConfig.MaxLayerCount;
	if (DisplayPolygons[Layer].Array.Num() < 3) {
		DrawPolygonsPMC->SetProcMeshSection(Layer, FProcMeshSection());
		return;
	}
	DrawPolygonsPMC->SetMaterial(Layer, VertexColorMaterial);
	const TArray<FLinearColor>& PolygonColors = PolygonLayerConfig.LayerColors;
	if (DisplayLines[Layer].Array.Num() < 2) {
		return;
	}
	float PolygonOffset = PolygonLayerConfig.StartLayerOffset + PolygonLayerConfig.LayerOffsetStep * Layer;

	FProcMeshSection Section;
	Section.bEnableCollision = false;
	Section.bSectionVisible = true;
	int32 FanNum = DisplayPolygons[Layer].Array.Num() - 2;
	Section.ProcVertexBuffer.SetNum(DisplayPolygons[Layer].Array.Num());
	Section.ProcIndexBuffer.SetNum(FanNum * 3);
	for (int32 i = 0; i < DisplayPolygons[Layer].Array.Num(); ++i) {
		Section.ProcVertexBuffer[i].Position = DisplayPolygons[Layer].Array[i] + Normal * PolygonOffset;
		Section.SectionLocalBox += Section.ProcVertexBuffer[i].Position;
	}
	for (int32 i = 0; i < FanNum; ++i) {
		int32 I0 = 0;
		int32 I1 = i + 1;
		int32 I2 = i + 2;

		int32 StartI = i * 3;

		Section.ProcIndexBuffer[StartI] = I0;
		Section.ProcIndexBuffer[StartI + 1] = I1;
		Section.ProcIndexBuffer[StartI + 2] = I2;
		Section.ProcIndexBuffer[StartI] = I0;
		Section.ProcIndexBuffer[StartI + 1] = I2;
		Section.ProcIndexBuffer[StartI + 2] = I1;
	}
	for (int32 i = 0; i < Section.ProcVertexBuffer.Num(); ++i) {
		Section.ProcVertexBuffer[i].Normal = Normal;
		Section.ProcVertexBuffer[i].Tangent = { Tangent, false };
		Section.ProcVertexBuffer[i].Color = (Layer < PolygonColors.Num() ? PolygonColors[Layer] : PolygonColors.Last()).ToFColor(false);
	}

	DrawPolygonsPMC->SetProcMeshSection(Layer, Section);
}

void ACGDemoCanvas2D::ClearDrawing()
{
	DrawPointsPMC->ClearAllMeshSections();
	DrawLinesPMC->ClearAllMeshSections();
	DrawPolygonsPMC->ClearAllMeshSections();
}

FVector ACGDemoCanvas2D::ToCanvasPoint(FVector2D P2D)
{
	return FVector(CanvasPlaneX, P2D.X, P2D.Y);
}

FVector2D ACGDemoCanvas2D::FromCanvasPoint(FVector CanvasPoint)
{
	return FVector2D(CanvasPoint.Y, CanvasPoint.Z);
}


