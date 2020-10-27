// Fill out your copyright notice in the Description page of Project Settings.


#include "CGDemoPlayerController2D.h"
#include "GameFramework/Pawn.h"
#include "CGDemoCanvas2D.h"


ACGDemoPlayerController2D::ACGDemoPlayerController2D(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ACGDemoPlayerController2D::BeginPlay()
{
	Super::BeginPlay();
	APawn* PossessedPawn = GetPawn();
	FVector NewSpawnLocation = FVector(-500, 0, 0);
	if (PossessedPawn) {
		PossessedPawn->SetActorLocation(NewSpawnLocation, false, nullptr, ETeleportType::None);
	}
	else {
		SetSpawnLocation(NewSpawnLocation);
	}
	UWorld* World = GetWorld();
	if (!World) {
		return;
	}
	Canvas2D = World->SpawnActor<ACGDemoCanvas2D>();
	Canvas2D->Rename(TEXT("CGDemoCanvas2D_0"));
}

void ACGDemoPlayerController2D::Canvas3DTo2D(TArray<FVector2D>& Out, const TArray<FVector>& In, bool bAlreadyEmpty)
{
	if (!bAlreadyEmpty) {
		Out.Empty(In.Num());
	}
	for (const FVector& P : In) {
		Out.Emplace(Canvas2D->FromCanvasPoint(P));
	}
}

void ACGDemoPlayerController2D::Canvas3DTo2D(TArray<TTuple<double, double> >& Out, const TArray<FVector>& In, bool bAlreadyEmpty)
{
	if (!bAlreadyEmpty) {
		Out.Empty(In.Num());
	}
	for (const FVector& P : In) {
		FVector2D V = Canvas2D->FromCanvasPoint(P);
		Out.Emplace(V.X, V.Y);
	}
}

void ACGDemoPlayerController2D::Canvas2DTo3D(TArray<FVector>& Out, const TArray<FVector2D>& In, bool bAlreadyEmpty)
{
	if (!bAlreadyEmpty) {
		Out.Empty(In.Num());
	}
	for (const FVector2D& P : In) {
		Out.Emplace(Canvas2D->ToCanvasPoint(P));
	}
}

void ACGDemoPlayerController2D::Canvas2DTo3D(TArray<FVector>& Out, const TArray<TTuple<double, double> >& In, bool bAlreadyEmpty)
{
	if (!bAlreadyEmpty) {
		Out.Empty(In.Num());
	}
	for (const TTuple<double, double>& P : In) {
		FVector2D V(P.Get<0>(), P.Get<1>());
		Out.Emplace(Canvas2D->ToCanvasPoint(V));
	}
}

bool ACGDemoPlayerController2D::TraceToCanvas(float& Distance, FVector& HitPoint, const FVector& Pos, const FVector& Dir)
{
	if (!Canvas2D) {
		return false;
	}
	return Canvas2D->TraceRay(Distance, HitPoint, Pos, Dir);
}
